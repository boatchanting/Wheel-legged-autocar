#include "wifi_protocol.h"
#include "menu.h"
#include "../navigation/gnss_transform.h"
#include "../navigation/gnss_ins_fusion.h"

// ------------------------------------------------------------------
// TX and RX buffers
// ------------------------------------------------------------------
static uint8_t tx_buf[WIFI_TX_BUFFER_SIZE];
static uint16_t tx_idx = 0;

#define WIFI_RX_READ_CHUNK   128U
#define WIFI_RX_STREAM_SIZE  512U
#define WIFI_FRAME_MIN_SIZE  6U
#define WIFI_ACK_PAYLOAD_LEN 2U

static uint8_t rx_stream[WIFI_RX_STREAM_SIZE];
static uint16_t rx_len = 0;

// ------------------------------------------------------------------
// Serialization helpers (little-endian)
// ------------------------------------------------------------------
static void write_u8(uint8_t val)
{
    if (tx_idx < WIFI_TX_BUFFER_SIZE) tx_buf[tx_idx++] = val;
}

static void write_i8(int8_t val)
{
    if (tx_idx < WIFI_TX_BUFFER_SIZE) tx_buf[tx_idx++] = (uint8_t)val;
}

static void write_u16(uint16_t val)
{
    if (tx_idx + 2U <= WIFI_TX_BUFFER_SIZE)
    {
        tx_buf[tx_idx++] = (uint8_t)(val & 0xFFU);
        tx_buf[tx_idx++] = (uint8_t)((val >> 8U) & 0xFFU);
    }
}

static void write_u32_or_float(const void *val_ptr)
{
    if (tx_idx + 4U <= WIFI_TX_BUFFER_SIZE)
    {
        const uint8_t *p = (const uint8_t *)val_ptr;
        tx_buf[tx_idx++] = p[0];
        tx_buf[tx_idx++] = p[1];
        tx_buf[tx_idx++] = p[2];
        tx_buf[tx_idx++] = p[3];
    }
}

static void write_float_value(float val)
{
    write_u32_or_float(&val);
}

static void write_double(const double *val_ptr)
{
    if (tx_idx + 8U <= WIFI_TX_BUFFER_SIZE)
    {
        const uint8_t *p = (const uint8_t *)val_ptr;
        for (uint8_t i = 0; i < 8U; i++)
        {
            tx_buf[tx_idx++] = p[i];
        }
    }
}

static void wifi_protocol_send_simple_frame(uint8_t cmd, const uint8_t *payload, uint8_t payload_len)
{
    uint8_t frame[WIFI_FRAME_MIN_SIZE + 16U];
    uint16_t idx = 0U;

    if (payload_len > 16U)
    {
        return;
    }

    frame[idx++] = WIFI_FRAME_HEAD1;
    frame[idx++] = WIFI_FRAME_HEAD2;
    frame[idx++] = cmd;
    frame[idx++] = payload_len;

    for (uint8_t i = 0U; i < payload_len; i++)
    {
        frame[idx++] = payload[i];
    }

    uint8_t check_sum = 0U;
    for (uint16_t i = 0U; i < idx; i++)
    {
        check_sum = (uint8_t)(check_sum + frame[i]);
    }

    frame[idx++] = check_sum;
    frame[idx++] = WIFI_FRAME_TAIL;

    wifi_spi_send_buffer(frame, idx);
}

static void wifi_protocol_send_host_ack(uint8_t control_id, uint8_t status)
{
    uint8_t payload[WIFI_ACK_PAYLOAD_LEN];
    payload[0] = control_id;
    payload[1] = status;
    wifi_protocol_send_simple_frame(WIFI_CMD_HOST_ACK, payload, WIFI_ACK_PAYLOAD_LEN);
}

// ------------------------------------------------------------------
// Host control command handling
// ------------------------------------------------------------------
static void wifi_protocol_apply_host_control(uint8_t control_id)
{
    uint8_t ack_status = WIFI_HOST_ACK_UNKNOWN_CMD;

    switch (control_id)
    {
    case WIFI_HOST_CTRL_CLEAR_TRAJECTORY:
    {
        const uint8_t accepted = (uint8_t)(g_motor_enable && g_yaw_initialized);
        // Reuse menu action path so host command and key action stay identical.
        Menu_TriggerRecordAction();
        if (accepted)
        {
            // GPS+惯导融合：手动锁定原点与发车角
            #if GNSS_NAV == 1
            Fusion_Manual_Lock_Origin();
            #endif
            ack_status = WIFI_HOST_ACK_ACCEPTED;
#if DEBUG_LOG_ENABLE
            printf("[WIFI] Host cmd CLEAR_TRAJECTORY accepted.\r\n");
#endif
        }
        else
        {
            ack_status = WIFI_HOST_ACK_REJECTED;
#if DEBUG_LOG_ENABLE
            printf("[WIFI] Host cmd CLEAR_TRAJECTORY ignored (motor/yaw not ready).\r\n");
#endif
        }
        break;
    }

    case WIFI_HOST_CTRL_START_CAR:
    {
        const uint8_t accepted = g_motor_enable;
        // Reuse menu action path so host command and key action stay identical.
        Menu_TriggerStartAction();
        if (accepted)
        {
            ack_status = WIFI_HOST_ACK_ACCEPTED;
#if DEBUG_LOG_ENABLE
            printf("[WIFI] Host cmd START_CAR accepted.\r\n");
#endif
        }
        else
        {
            ack_status = WIFI_HOST_ACK_REJECTED;
#if DEBUG_LOG_ENABLE
            printf("[WIFI] Host cmd START_CAR ignored (motor disabled).\r\n");
#endif
        }
        break;
    }

    case WIFI_HOST_CTRL_START_GPS_REPLAY:
    {
        const uint8_t accepted = g_motor_enable;
        if (accepted)
        {
            g_replay_start_request = 1U;
            ack_status = WIFI_HOST_ACK_ACCEPTED;
#if DEBUG_LOG_ENABLE
            printf("[WIFI] Host cmd START_GPS_REPLAY accepted.\r\n");
#endif
        }
        else
        {
            ack_status = WIFI_HOST_ACK_REJECTED;
#if DEBUG_LOG_ENABLE
            printf("[WIFI] Host cmd START_GPS_REPLAY ignored (motor disabled).\r\n");
#endif
        }
        break;
    }

    case WIFI_HOST_CTRL_STOP_GPS_REPLAY:
    {
        g_replay_stop_request = 1U;
        ack_status = WIFI_HOST_ACK_ACCEPTED;
#if DEBUG_LOG_ENABLE
        printf("[WIFI] Host cmd STOP_GPS_REPLAY accepted.\r\n");
#endif
        break;
    }

    default:
#if DEBUG_LOG_ENABLE
        printf("[WIFI] Unknown host control cmd: 0x%02X\r\n", control_id);
#endif
        break;
    }

    wifi_protocol_send_host_ack(control_id, ack_status);
}

static void wifi_protocol_handle_frame(uint8_t cmd, const uint8_t *payload, uint8_t payload_len)
{
    if (cmd == WIFI_CMD_HOST_CONTROL)
    {
        if (payload_len >= 1U)
        {
            wifi_protocol_apply_host_control(payload[0]);
        }
        else
        {
            wifi_protocol_send_host_ack(0U, WIFI_HOST_ACK_INVALID_PAYLOAD);
        }
        return;
    }

    // Ignore unknown command frames from host.
}

static void wifi_protocol_parse_stream(void)
{
    while (rx_len >= WIFI_FRAME_MIN_SIZE)
    {
        if ((rx_stream[0] != WIFI_FRAME_HEAD1) || (rx_stream[1] != WIFI_FRAME_HEAD2))
        {
            if (rx_len > 1U)
            {
                memmove(rx_stream, rx_stream + 1, rx_len - 1U);
            }
            rx_len -= 1U;
            continue;
        }

        const uint8_t payload_len = rx_stream[3];
        const uint16_t frame_len = (uint16_t)payload_len + WIFI_FRAME_MIN_SIZE;

        if ((payload_len == 0U) || (frame_len > WIFI_RX_STREAM_SIZE))
        {
            if (rx_len > 1U)
            {
                memmove(rx_stream, rx_stream + 1, rx_len - 1U);
            }
            rx_len -= 1U;
            continue;
        }

        if (rx_len < frame_len)
        {
            break;
        }

        if (rx_stream[frame_len - 1U] != WIFI_FRAME_TAIL)
        {
            if (rx_len > 1U)
            {
                memmove(rx_stream, rx_stream + 1, rx_len - 1U);
            }
            rx_len -= 1U;
            continue;
        }

        uint8_t check_sum = 0U;
        for (uint16_t i = 0; i < frame_len - 2U; i++)
        {
            check_sum = (uint8_t)(check_sum + rx_stream[i]);
        }

        if (check_sum != rx_stream[frame_len - 2U])
        {
            if (rx_len > 1U)
            {
                memmove(rx_stream, rx_stream + 1, rx_len - 1U);
            }
            rx_len -= 1U;
            continue;
        }

        wifi_protocol_handle_frame(rx_stream[2], &rx_stream[4], payload_len);

        if (rx_len > frame_len)
        {
            memmove(rx_stream, rx_stream + frame_len, rx_len - frame_len);
        }
        rx_len = (uint16_t)(rx_len - frame_len);
    }
}

void wifi_protocol_poll_rx(void)
{
    uint8_t read_buf[WIFI_RX_READ_CHUNK];
    uint32_t read_len = wifi_spi_read_buffer(read_buf, WIFI_RX_READ_CHUNK);
    if (read_len == 0U)
    {
        return;
    }

    if (read_len >= WIFI_RX_STREAM_SIZE)
    {
        memcpy(rx_stream, &read_buf[read_len - WIFI_RX_STREAM_SIZE], WIFI_RX_STREAM_SIZE);
        rx_len = WIFI_RX_STREAM_SIZE;
    }
    else
    {
        if ((uint32_t)rx_len + read_len > WIFI_RX_STREAM_SIZE)
        {
            const uint16_t overflow = (uint16_t)((uint32_t)rx_len + read_len - WIFI_RX_STREAM_SIZE);
            if (overflow < rx_len)
            {
                memmove(rx_stream, rx_stream + overflow, rx_len - overflow);
                rx_len = (uint16_t)(rx_len - overflow);
            }
            else
            {
                rx_len = 0U;
            }
        }

        memcpy(rx_stream + rx_len, read_buf, read_len);
        rx_len = (uint16_t)(rx_len + read_len);
    }

    wifi_protocol_parse_stream();
}

// ------------------------------------------------------------------
// Telemetry TX (MCU -> host)
// ------------------------------------------------------------------
void wifi_protocol_send_data(void)
{
    // Poll host command first. This runs in the same 10 ms cycle as telemetry.
    wifi_protocol_poll_rx();

    tx_idx = 0;

    write_u8(WIFI_FRAME_HEAD1);
    write_u8(WIFI_FRAME_HEAD2);
    write_u8(WIFI_CMD_DATA_PACKET);

    const uint16_t len_pos = tx_idx;
    write_u8(0x00);

    // A. loop counter
    write_u32_or_float(&loop_counter);

    // B. inertial nav
    write_u32_or_float(&inertial_nav.x);
    write_u32_or_float(&inertial_nav.y);
    write_u32_or_float(&inertial_nav.vx_body);
    write_u32_or_float(&inertial_nav.vy_body);

    // C. GNSS fields
    write_u16(gnss.time.year);
    write_u8(gnss.time.month);
    write_u8(gnss.time.day);
    write_u8(gnss.time.hour);
    write_u8(gnss.time.minute);
    write_u8(gnss.time.second);

    write_u8(gnss.state);

    write_u16(gnss.latitude_degree);
    write_u16(gnss.latitude_cent);
    write_u16(gnss.latitude_second);
    write_u16(gnss.longitude_degree);
    write_u16(gnss.longitude_cent);
    write_u16(gnss.longitude_second);

    write_double(&gnss.latitude);
    write_double(&gnss.longitude);

    write_i8(gnss.ns);
    write_i8(gnss.ew);

    write_u32_or_float(&gnss.speed);
    write_u32_or_float(&gnss.direction);

    write_u8(gnss.antenna_direction_state);
    write_u32_or_float(&gnss.antenna_direction);

    write_u8(gnss.satellite_used);
    write_u32_or_float(&gnss.height);

#if IMU_CATEGORY == 3
    float heading_to_send = heading;
#else
    float heading_to_send = 0.0f;
#endif
    write_u32_or_float(&heading_to_send);
    write_u32_or_float(&inertial_nav.relative_yaw);

    // D. host point editor fields
    write_u8(robot_ctrl.mark_trigger);
    write_u8(robot_ctrl.point_type);

    // E. projected GNSS XY for pure GPS marker/replay (unit: mm)
    write_float_value(gnss_trans.x * 1000.0f);
    write_float_value(gnss_trans.y * 1000.0f);
    write_u8(gnss_trans.is_valid);
    write_u8(gnss_trans.is_origin_set);

    // F. GPS+INS fusion state (unit: mm / deg)
    write_float_value(g_fuse_state.fuse_x);
    write_float_value(g_fuse_state.fuse_y);
    write_float_value(g_fuse_state.fuse_yaw);
    write_float_value(g_fuse_state.offset_x);
    write_float_value(g_fuse_state.offset_y);

    // G. three trajectories for offline verification (unit: mm)
    //    1. pure INS: ins_x, ins_y
    //    2. GPS ground truth: ground_x, ground_y
    //    3. fusion output: fuse_x, fuse_y (already sent in section F)
    write_float_value(g_fuse_state.ins_x);
    write_float_value(g_fuse_state.ins_y);
    write_float_value(gnss_trans.ground_x * 1000.0f);
    write_float_value(gnss_trans.ground_y * 1000.0f);

    // H. Status monitoring for fusion 
    write_float_value(g_fuse_state.k_pos);
    write_u8(g_fuse_state.jump_reject_count);
    write_u8(g_fuse_state.zupt_flag);
    write_u8(g_fuse_state.special_element_flag);

    const uint8_t payload_len = (uint8_t)(tx_idx - (len_pos + 1U));
    tx_buf[len_pos] = payload_len;

    uint8_t check_sum = 0U;
    for (uint16_t i = 0; i < tx_idx; i++)
    {
        check_sum = (uint8_t)(check_sum + tx_buf[i]);
    }
    write_u8(check_sum);
    write_u8(WIFI_FRAME_TAIL);

    wifi_spi_send_buffer(tx_buf, tx_idx);

    robot_ctrl.mark_trigger = 0;
}
