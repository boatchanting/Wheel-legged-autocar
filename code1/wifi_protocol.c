#include "wifi_protocol.h"
#include "wifi.h"

#define WIFI_RX_READ_CHUNK   128U
#define WIFI_RX_STREAM_SIZE  512U
#define WIFI_FRAME_MIN_SIZE  6U
#define WIFI_ACK_PAYLOAD_LEN 2U

static uint8_t rx_stream[WIFI_RX_STREAM_SIZE];
static uint16_t rx_len = 0U;

static void wifi_protocol_send_simple_frame(uint8_t cmd, const uint8_t *payload, uint8_t payload_len)
{
    uint8_t frame[WIFI_FRAME_MIN_SIZE + 32U];
    uint16_t idx = 0U;
    uint8_t check_sum = 0U;

    if (payload_len > 32U)
    {
        return;
    }

    frame[idx++] = C1_WIFI_FRAME_HEAD1;
    frame[idx++] = C1_WIFI_FRAME_HEAD2;
    frame[idx++] = cmd;
    frame[idx++] = payload_len;

    for (uint8_t i = 0U; i < payload_len; i++)
    {
        frame[idx++] = payload[i];
    }

    for (uint16_t i = 0U; i < idx; i++)
    {
        check_sum = (uint8_t)(check_sum + frame[i]);
    }

    frame[idx++] = check_sum;
    frame[idx++] = C1_WIFI_FRAME_TAIL;
    wifi_spi_send_buffer(frame, idx);
}

static void wifi_protocol_send_host_ack(uint8_t control_id, uint8_t status)
{
    uint8_t payload[WIFI_ACK_PAYLOAD_LEN];
    payload[0] = control_id;
    payload[1] = status;
    wifi_protocol_send_simple_frame(C1_WIFI_CMD_HOST_ACK, payload, WIFI_ACK_PAYLOAD_LEN);
}

static void wifi_protocol_apply_host_control(uint8_t control_id, const uint8_t *payload, uint8_t payload_len)
{
    uint8_t ack_status = C1_WIFI_HOST_ACK_UNKNOWN_CMD;

    switch (control_id)
    {
    case C1_WIFI_HOST_CTRL_SET_TCP_TARGET:
    {
        (void)payload;
        (void)payload_len;
        ack_status = C1_WIFI_HOST_ACK_UNSUPPORTED;
        break;
    }

    case C1_WIFI_HOST_CTRL_SET_LOCAL_PORT:
    {
        (void)payload;
        (void)payload_len;
        ack_status = C1_WIFI_HOST_ACK_UNSUPPORTED;
        break;
    }

    case C1_WIFI_HOST_CTRL_RECONNECT_TCP:
    {
        wifi_connect_tcp_server();
        ack_status = C1_WIFI_HOST_ACK_ACCEPTED;
        break;
    }

    default:
        break;
    }

    wifi_protocol_send_host_ack(control_id, ack_status);
}

static void wifi_protocol_handle_frame(uint8_t cmd, const uint8_t *payload, uint8_t payload_len)
{
    if (cmd == C1_WIFI_CMD_HOST_CONTROL)
    {
        if (payload_len >= 1U)
        {
            wifi_protocol_apply_host_control(payload[0], payload + 1U, (uint8_t)(payload_len - 1U));
        }
        else
        {
            wifi_protocol_send_host_ack(0U, C1_WIFI_HOST_ACK_INVALID_PAYLOAD);
        }
    }
}

static void wifi_protocol_parse_stream(void)
{
    while (rx_len >= WIFI_FRAME_MIN_SIZE)
    {
        if ((rx_stream[0] != C1_WIFI_FRAME_HEAD1) || (rx_stream[1] != C1_WIFI_FRAME_HEAD2))
        {
            if (rx_len > 1U)
            {
                memmove(rx_stream, rx_stream + 1U, rx_len - 1U);
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
                memmove(rx_stream, rx_stream + 1U, rx_len - 1U);
            }
            rx_len -= 1U;
            continue;
        }

        if (rx_len < frame_len)
        {
            break;
        }

        if (rx_stream[frame_len - 1U] != C1_WIFI_FRAME_TAIL)
        {
            if (rx_len > 1U)
            {
                memmove(rx_stream, rx_stream + 1U, rx_len - 1U);
            }
            rx_len -= 1U;
            continue;
        }

        uint8_t check_sum = 0U;
        for (uint16_t i = 0U; i < frame_len - 2U; i++)
        {
            check_sum = (uint8_t)(check_sum + rx_stream[i]);
        }
        if (check_sum != rx_stream[frame_len - 2U])
        {
            if (rx_len > 1U)
            {
                memmove(rx_stream, rx_stream + 1U, rx_len - 1U);
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
