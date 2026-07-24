#include "wifi_protocol.h"
#include "menu.h"
#include "sbus.h"
#include "../common.h"
#include "../navigation/gnss_transform.h"
#include "../calculate/pid-new.h"
#include "../calculate/ekf.h"
#include "../calculate/matrix.h"
#include "../navigation/nav_replay/nav_replay.h"
#include "../servo/servo_executor.h"

// ------------------------------------------------------------------
// TX and RX buffers
// ------------------------------------------------------------------
static uint8_t tx_buf[WIFI_TX_BUFFER_SIZE];
static uint16_t tx_idx = 0;

uint8_t g_manual_log_enabled = 0;
volatile uint8_t g_wifi_host_beep_request = 0U;
#define WIFI_RX_READ_CHUNK   128U
#define WIFI_RX_STREAM_SIZE  512U
#define WIFI_FRAME_MIN_SIZE  6U
#define WIFI_ACK_PAYLOAD_LEN 2U
#define WIFI_SPEED_CMD_ZERO_EPS 0.001f
#define WIFI_CONTROL_DEBUG_FLOAT_COUNT 12U
#define WIFI_CONTROL_DEBUG_PAYLOAD_LEN (WIFI_CONTROL_DEBUG_FLOAT_COUNT * 4U)

static uint8_t rx_stream[WIFI_RX_STREAM_SIZE];
static uint16_t rx_len = 0;
static uint8_t s_host_speed_test_active = 0U;
static float s_host_speed_test_cmd = 0.0f;
static uint8_t s_host_speed_test_mode = (uint8_t)CONTROL_MODE_NORMAL;
static float s_last_host_speed_cmd = 0.0f;
static uint8_t s_last_host_speed_mode = (uint8_t)CONTROL_MODE_NORMAL;
static uint8_t s_last_host_speed_valid = 0U;

extern volatile float err_degree;
extern volatile float g_debug_pwm_left;
extern volatile float g_debug_pwm_right;

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

static float read_float_le(const uint8_t *src)
{
    union {
        uint8_t b[4];
        float f;
    } value;
    value.b[0] = src[0];
    value.b[1] = src[1];
    value.b[2] = src[2];
    value.b[3] = src[3];
    return value.f;
}

static float wifi_protocol_absf(float value)
{
    return (value < 0.0f) ? -value : value;
}

static uint8_t wifi_protocol_speed_cmd_is_zero(float speed_cmd)
{
    return (uint8_t)(wifi_protocol_absf(speed_cmd) <= WIFI_SPEED_CMD_ZERO_EPS);
}

static uint8_t wifi_protocol_target_changed(float speed_cmd, uint8_t mode)
{
    if (s_last_host_speed_valid == 0U)
    {
        return 1U;
    }
    if (s_last_host_speed_mode != mode)
    {
        return 1U;
    }
    return (uint8_t)(wifi_protocol_absf(s_last_host_speed_cmd - speed_cmd) > WIFI_SPEED_CMD_ZERO_EPS);
}

static void wifi_protocol_request_host_beep(void)
{
    g_wifi_host_beep_request = 1U;
}

static void wifi_protocol_force_straight_steering(void)
{
    err_degree = 0.0f;
    turn_angle_loop_out = 0.0f;
    turn_gyro_loop_out = 0.0f;
}

uint8_t WifiHostSpeedTest_IsActive(void)
{
    return s_host_speed_test_active;
}

uint8_t WifiHostSpeedTest_AccelFeedforwardActive(void)
{
    return (uint8_t)((s_host_speed_test_active != 0U) &&
                     (s_host_speed_test_mode == (uint8_t)CONTROL_MODE_ACCEL) &&
                     (wifi_protocol_speed_cmd_is_zero(s_host_speed_test_cmd) == 0U));
}

uint8_t WifiHostSpeedTest_Arm(void)
{
    if ((g_motor_enable == 0) || (g_brake_active != 0U) || (g_reverse_brake_active != 0U))
    {
        return 0U;
    }

    g_fallen = false;
    g_load_flash_request = 0U;
    g_replay_start_request = 0U;
    g_replay_stop_request = 0U;
    g_nav_start_recording = 0U;
    g_nav_recording = 0U;
    NavReplay_Stop();
#if GNSS_NAV == 1
    GpsNavReplay_Stop();
#endif

    s_host_speed_test_active = 1U;
    s_host_speed_test_cmd = 0.0f;
    s_host_speed_test_mode = (uint8_t)CONTROL_MODE_NORMAL;
    s_last_host_speed_valid = 0U;
    target_speed_set = 0.0f;
    wifi_protocol_force_straight_steering();
    Turn_Control_Reset();
    Control_Profile_RequestMode(CONTROL_MODE_NORMAL);
    wifi_protocol_request_host_beep();
    return 1U;
}

uint8_t WifiHostSpeedTest_SetTarget(float speed_cmd, uint8_t mode)
{
    const uint8_t zero_cmd = wifi_protocol_speed_cmd_is_zero(speed_cmd);

    if (mode > (uint8_t)CONTROL_MODE_BRAKE)
    {
        return 0U;
    }
    if (s_host_speed_test_active == 0U)
    {
        return 0U;
    }
    if ((zero_cmd == 0U) && ((g_motor_enable == 0) || (g_brake_active != 0U) || (g_reverse_brake_active != 0U)))
    {
        return 0U;
    }

    s_host_speed_test_cmd = speed_cmd;
    s_host_speed_test_mode = mode;
    target_speed_set = speed_cmd;
    wifi_protocol_force_straight_steering();
    Turn_Control_Reset();
    Control_Profile_RequestMode((ControlMode_e)mode);

    if (wifi_protocol_target_changed(speed_cmd, mode) != 0U)
    {
        s_last_host_speed_cmd = speed_cmd;
        s_last_host_speed_mode = mode;
        s_last_host_speed_valid = 1U;
        wifi_protocol_request_host_beep();
    }
    return 1U;
}

void WifiHostSpeedTest_Stop(void)
{
    s_host_speed_test_active = 0U;
    s_host_speed_test_cmd = 0.0f;
    s_host_speed_test_mode = (uint8_t)CONTROL_MODE_NORMAL;
    s_last_host_speed_valid = 0U;
    target_speed_set = 0.0f;
    wifi_protocol_force_straight_steering();
    Turn_Control_Reset();
    Control_Profile_RequestMode(CONTROL_MODE_NORMAL);
    wifi_protocol_request_host_beep();
}

void WifiHostSpeedTest_Update1ms(void)
{
    if (s_host_speed_test_active == 0U)
    {
        return;
    }

    if ((g_motor_enable == 0) || (g_fallen) || (g_reverse_brake_active != 0U))
    {
        WifiHostSpeedTest_Stop();
        return;
    }

    if (g_brake_active != 0U)
    {
        s_host_speed_test_cmd = 0.0f;
        s_host_speed_test_mode = (uint8_t)CONTROL_MODE_BRAKE;
        target_speed_set = 0.0f;
        Control_Profile_RequestMode(CONTROL_MODE_BRAKE);
        wifi_protocol_force_straight_steering();
        return;
    }

    target_speed_set = s_host_speed_test_cmd;
    wifi_protocol_force_straight_steering();
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
static void wifi_protocol_apply_host_control(uint8_t control_id, const uint8_t *payload, uint8_t payload_len)
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

    case WIFI_HOST_CTRL_START_LOG:
    {
        g_manual_log_enabled = 1;
        ack_status = WIFI_HOST_ACK_ACCEPTED;
        break;
    }

    case WIFI_HOST_CTRL_STOP_LOG:
    {
        g_manual_log_enabled = 0;
        ack_status = WIFI_HOST_ACK_ACCEPTED;
        break;
    }

    case WIFI_HOST_CTRL_ARM_DIRECT_SPEED:
    {
        if (WifiHostSpeedTest_Arm() != 0U)
        {
            ack_status = WIFI_HOST_ACK_ACCEPTED;
#if DEBUG_LOG_ENABLE
            printf("[WIFI] Host cmd ARM_DIRECT_SPEED accepted.\r\n");
#endif
        }
        else
        {
            ack_status = WIFI_HOST_ACK_REJECTED;
#if DEBUG_LOG_ENABLE
            printf("[WIFI] Host cmd ARM_DIRECT_SPEED rejected.\r\n");
#endif
        }
        break;
    }

    case WIFI_HOST_CTRL_STOP_DIRECT_SPEED:
    {
        WifiHostSpeedTest_Stop();
        ack_status = WIFI_HOST_ACK_ACCEPTED;
#if DEBUG_LOG_ENABLE
        printf("[WIFI] Host cmd STOP_DIRECT_SPEED accepted.\r\n");
#endif
        break;
    }

    case WIFI_HOST_CTRL_SET_TARGET_SPEED:
    {
        if (payload_len < 7U)
        {
            ack_status = WIFI_HOST_ACK_INVALID_PAYLOAD;
            break;
        }

        const float speed_cmd = read_float_le(&payload[1]);
        const uint8_t mode = payload[5];
        const uint8_t zero_cmd = wifi_protocol_speed_cmd_is_zero(speed_cmd);
        if (mode > (uint8_t)CONTROL_MODE_BRAKE)
        {
            ack_status = WIFI_HOST_ACK_INVALID_PAYLOAD;
            break;
        }

        if (WifiHostSpeedTest_IsActive() != 0U)
        {
            if (WifiHostSpeedTest_SetTarget(speed_cmd, mode) != 0U)
            {
                ack_status = WIFI_HOST_ACK_ACCEPTED;
            }
            else
            {
                ack_status = WIFI_HOST_ACK_REJECTED;
            }
        }
        else if (zero_cmd != 0U)
        {
            target_speed_set = 0.0f;
            Control_Profile_RequestMode((ControlMode_e)mode);
            ack_status = WIFI_HOST_ACK_ACCEPTED;
        }
        else
        {
            ack_status = WIFI_HOST_ACK_REJECTED;
        }
#if DEBUG_LOG_ENABLE
        printf("[WIFI] Host cmd SET_TARGET_SPEED speed=%.2f mode=%u status=%u.\r\n", speed_cmd, mode, ack_status);
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
            wifi_protocol_apply_host_control(payload[0], payload, payload_len);
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

static void wifi_protocol_send_control_debug_frame(void)
{
    tx_idx = 0;

    write_u8(WIFI_FRAME_HEAD1);
    write_u8(WIFI_FRAME_HEAD2);
    write_u8(WIFI_CMD_CONTROL_DEBUG);

    const uint16_t len_pos = tx_idx;
    write_u8(0x00);

    float servo_speed_target = (float)target_speed_set;
    float servo_speed_actual = (float)current_actual_speed;
    float servo_speed_output = (float)pid_servo_speed.output;
    float servo_speed_error = (float)pid_servo_speed.error;
    float servo_pwm_speed_adj = (float)g_target_pwm_speed_adj;
    float servo_pwm_high = (float)g_target_pwm_high;
    float servo_pwm_angle_adj = (float)g_target_pwm_angle_adj;
    float turn_err_degree = (float)err_degree;
    float turn_angle_out = (float)turn_angle_loop_out;
    float turn_gyro_out = (float)turn_gyro_loop_out;
    float final_pwm_left = (float)g_debug_pwm_left;
    float final_pwm_right = (float)g_debug_pwm_right;

    write_u32_or_float(&servo_speed_target);
    write_u32_or_float(&servo_speed_actual);
    write_u32_or_float(&servo_speed_output);
    write_u32_or_float(&servo_speed_error);
    write_u32_or_float(&servo_pwm_speed_adj);
    write_u32_or_float(&servo_pwm_high);
    write_u32_or_float(&servo_pwm_angle_adj);
    write_u32_or_float(&turn_err_degree);
    write_u32_or_float(&turn_angle_out);
    write_u32_or_float(&turn_gyro_out);
    write_u32_or_float(&final_pwm_left);
    write_u32_or_float(&final_pwm_right);

    const uint16_t payload_len_u16 = tx_idx - (len_pos + 1U);
    if (payload_len_u16 > 255U)
    {
        return;
    }
    tx_buf[len_pos] = (uint8_t)payload_len_u16;

    uint8_t check_sum = 0U;
    for (uint16_t i = 0U; i < tx_idx; i++)
    {
        check_sum = (uint8_t)(check_sum + tx_buf[i]);
    }
    write_u8(check_sum);
    write_u8(WIFI_FRAME_TAIL);

    wifi_spi_send_buffer(tx_buf, tx_idx);
}

// ------------------------------------------------------------------
// Telemetry TX (MCU -> host)
// ------------------------------------------------------------------
void wifi_protocol_send_data(void)
{
    // Poll host command first. This runs in the same 10 ms cycle as telemetry.
    wifi_protocol_poll_rx();
    wifi_protocol_send_control_debug_frame();

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

    // F. reserved fusion trace (complement filter removed in this branch)
    write_float_value(0.0f);
    write_float_value(0.0f);
    write_u8(0U);

    // G. PID Control Mode
    write_u8((uint8_t)g_control_mode_applied);
    
    // H. Slip Flag
    write_u8((uint8_t)inertial_nav.slip_flag);

    // I. Base control debug values (20 bytes). Always send these so a replay
    // that has already stopped can still be diagnosed from its CSV tail.
    float t_speed = (float)target_speed_set;
    write_u32_or_float(&t_speed);
    write_u32_or_float(&inertial_nav.current_speed_L);
    write_u32_or_float(&inertial_nav.current_speed_R);
    write_u32_or_float(&inertial_nav.theoretical_yaw_rate);
    write_u32_or_float(&inertial_nav.actual_yaw_rate);

    // J. Plan-2 special-point diagnostic block (19 floats, 76 bytes).
    // Keep this fixed-width and append-only; the host accepts older packets
    // without it and records these fields only when this block is present.
    {
        float nav_replay_state = (float)g_replay_state;
        float nav_special_action_trigger = (float)g_special_action_trigger;
        float nav_current_point_type = (float)g_current_point_type;
        float nav_special_target_idx = 0.0f;
        float nav_special_target_x = 0.0f;
        float nav_special_target_y = 0.0f;
        float nav_special_dist_mm = 0.0f;
        float nav_special_brake_radius_mm = 0.0f;
        float nav_special_speed_ref_mm_s = 0.0f;
        float nav_special_zero_brake_issued = 0.0f;
        float nav_special_zero_brake_active = 0.0f;
        float nav_special_crawl_active = 0.0f;
        float nav_special_prep_zero_latched = 0.0f;
        float brake_ff_pwm = Brake_Feedforward_GetPwm();
        float accel_ff_pwm = Accel_Feedforward_GetPwm();
        float motor_enable = (float)g_motor_enable;
        float fallen = g_fallen ? 1.0f : 0.0f;
        float remote_brake_active = (float)g_brake_active;
        float remote_reverse_brake_active = (float)g_reverse_brake_active;

#if (CURRENT_NAV_PLAN == 2) && (NAV_PLAN2_METHOD == PLAN2_POINT_SPEED_PLANNING)
        nav_special_target_idx = (float)g_nav_point_special_debug_target_idx;
        nav_special_target_x = g_nav_point_special_debug_target_x;
        nav_special_target_y = g_nav_point_special_debug_target_y;
        nav_special_dist_mm = g_nav_point_special_debug_dist_mm;
        nav_special_brake_radius_mm = g_nav_point_special_debug_brake_radius_mm;
        nav_special_speed_ref_mm_s = g_nav_point_special_debug_speed_ref_mm_s;
        nav_special_zero_brake_issued = (float)g_nav_point_special_debug_zero_brake_issued;
        nav_special_zero_brake_active = (float)NavReplay_SpecialPointZeroBrakeActive();
        nav_special_crawl_active = (float)NavReplay_SpecialPointCrawlActive();
        nav_special_prep_zero_latched = (float)NavReplay_SpecialPointPrepZeroBrakeLatched();
#endif

        write_u32_or_float(&nav_replay_state);
        write_u32_or_float(&nav_special_action_trigger);
        write_u32_or_float(&nav_current_point_type);
        write_u32_or_float(&nav_special_target_idx);
        write_u32_or_float(&nav_special_target_x);
        write_u32_or_float(&nav_special_target_y);
        write_u32_or_float(&nav_special_dist_mm);
        write_u32_or_float(&nav_special_brake_radius_mm);
        write_u32_or_float(&nav_special_speed_ref_mm_s);
        write_u32_or_float(&nav_special_zero_brake_issued);
        write_u32_or_float(&nav_special_zero_brake_active);
        write_u32_or_float(&nav_special_crawl_active);
        write_u32_or_float(&nav_special_prep_zero_latched);
        write_u32_or_float(&brake_ff_pwm);
        write_u32_or_float(&accel_ff_pwm);
        write_u32_or_float(&motor_enable);
        write_u32_or_float(&fallen);
        write_u32_or_float(&remote_brake_active);
        write_u32_or_float(&remote_reverse_brake_active);
    }

    // K. Motion/attitude diagnostic block (12 floats, 48 bytes).
    // Append-only for the accel/brake collector; older hosts ignore these bytes.
    write_u32_or_float(&euler_angle.roll);
    write_u32_or_float(&euler_angle.pitch);
    write_u32_or_float(&euler_angle.yaw);
    write_u32_or_float(&imu_data.gyro_x);
    write_u32_or_float(&imu_data.gyro_y);
    write_u32_or_float(&imu_data.gyro_z);
    write_u32_or_float(&imu_data.acc_x);
    write_u32_or_float(&imu_data.acc_y);
    write_u32_or_float(&imu_data.acc_z);
    write_u32_or_float(&imu_data.grav_x);
    write_u32_or_float(&imu_data.grav_y);
    write_u32_or_float(&imu_data.grav_z);

    const uint16_t payload_len_u16 = tx_idx - (len_pos + 1U);
    if (payload_len_u16 > 255U)
    {
        return;
    }
    tx_buf[len_pos] = (uint8_t)payload_len_u16;

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
