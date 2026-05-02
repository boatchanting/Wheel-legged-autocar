#include "wifi_diff_stream.h"

#define WIFI_DIFF_SYNC0              (0xA5U)
#define WIFI_DIFF_SYNC1              (0x5AU)
#define WIFI_DIFF_VERSION            (1U)
#define WIFI_DIFF_MAX_W              (188U)
#define WIFI_DIFF_MAX_H              (120U)
#define WIFI_DIFF_HEADER_SIZE        (24U)
#define WIFI_DIFF_MAX_PAYLOAD        (WIFI_DIFF_MAX_W * WIFI_DIFF_MAX_H)
#define WIFI_DIFF_MAX_PACKET         (WIFI_DIFF_HEADER_SIZE + WIFI_DIFF_MAX_PAYLOAD + 1U)

static uint16 s_width = 0U;
static uint16 s_height = 0U;
static uint16 s_key_interval = 30U;
static uint8 s_diff_threshold = 0U;
static uint8 s_prev[WIFI_DIFF_MAX_H][WIFI_DIFF_MAX_W];
static uint8 s_has_prev = 0U;
static uint8 s_packet_buf[WIFI_DIFF_MAX_PACKET];
static wifi_diff_stream_stats_t s_stats;

static void write_u16_le(uint8 *dst, uint16 v)
{
    dst[0] = (uint8)(v & 0xFFU);
    dst[1] = (uint8)((v >> 8U) & 0xFFU);
}

static void write_u32_le(uint8 *dst, uint32 v)
{
    dst[0] = (uint8)(v & 0xFFU);
    dst[1] = (uint8)((v >> 8U) & 0xFFU);
    dst[2] = (uint8)((v >> 16U) & 0xFFU);
    dst[3] = (uint8)((v >> 24U) & 0xFFU);
}

static uint8 calc_sum8(const uint8 *buf, uint32 len)
{
    uint8 sum = 0U;
    for (uint32 i = 0U; i < len; i++)
    {
        sum = (uint8)(sum + buf[i]);
    }
    return sum;
}

static uint32 build_header(uint8 frame_type, uint16 x, uint16 y, uint16 w, uint16 h, uint32 payload_len)
{
    uint8 *p = s_packet_buf;
    p[0] = WIFI_DIFF_SYNC0;
    p[1] = WIFI_DIFF_SYNC1;
    p[2] = WIFI_DIFF_VERSION;
    p[3] = frame_type;
    write_u32_le(&p[4], s_stats.frame_id);
    write_u16_le(&p[8], s_width);
    write_u16_le(&p[10], s_height);
    write_u16_le(&p[12], x);
    write_u16_le(&p[14], y);
    write_u16_le(&p[16], w);
    write_u16_le(&p[18], h);
    write_u32_le(&p[20], payload_len);
    return WIFI_DIFF_HEADER_SIZE;
}

void wifi_diff_stream_init(uint16 width, uint16 height, uint16 keyframe_interval, uint8 diff_threshold)
{
    if ((width == 0U) || (height == 0U) || (width > WIFI_DIFF_MAX_W) || (height > WIFI_DIFF_MAX_H))
    {
        width = 94U;
        height = 60U;
    }
    s_width = width;
    s_height = height;
    s_key_interval = (keyframe_interval == 0U) ? 30U : keyframe_interval;
    s_diff_threshold = diff_threshold;
    memset(s_prev, 0, sizeof(s_prev));
    memset(&s_stats, 0, sizeof(s_stats));
    s_has_prev = 0U;
}

void wifi_diff_stream_send_gray_frame(const uint8 *frame_gray)
{
    if ((frame_gray == NULL) || (s_width == 0U) || (s_height == 0U))
    {
        return;
    }

    uint8 force_key = (uint8)((!s_has_prev) || ((s_stats.frame_id % s_key_interval) == 0U));
    uint32 packet_len = 0U;

    if (force_key)
    {
        const uint32 payload_len = (uint32)s_width * (uint32)s_height;
        packet_len = build_header(WIFI_DIFF_FRAME_KEY, 0U, 0U, s_width, s_height, payload_len);
        memcpy(&s_packet_buf[packet_len], frame_gray, payload_len);
        packet_len += payload_len;
        memcpy(s_prev[0], frame_gray, payload_len);
        s_has_prev = 1U;
        s_stats.keyframes++;
    }
    else
    {
        uint16 min_x = s_width, min_y = s_height, max_x = 0U, max_y = 0U;
        uint8 changed = 0U;
        for (uint16 y = 0U; y < s_height; y++)
        {
            for (uint16 x = 0U; x < s_width; x++)
            {
                uint32 idx = (uint32)y * (uint32)s_width + (uint32)x;
                uint8 cur = frame_gray[idx];
                uint8 prev = s_prev[y][x];
                uint8 d = (cur >= prev) ? (cur - prev) : (prev - cur);
                if (d > s_diff_threshold)
                {
                    if (!changed)
                    {
                        min_x = max_x = x;
                        min_y = max_y = y;
                        changed = 1U;
                    }
                    else
                    {
                        if (x < min_x) min_x = x;
                        if (x > max_x) max_x = x;
                        if (y < min_y) min_y = y;
                        if (y > max_y) max_y = y;
                    }
                }
            }
        }

        if (!changed)
        {
            packet_len = build_header(WIFI_DIFF_FRAME_SKIP, 0U, 0U, 0U, 0U, 0U);
            s_stats.skip_frames++;
        }
        else
        {
            uint16 roi_w = (uint16)(max_x - min_x + 1U);
            uint16 roi_h = (uint16)(max_y - min_y + 1U);
            uint32 payload_len = (uint32)roi_w * (uint32)roi_h;
            packet_len = build_header(WIFI_DIFF_FRAME_DIFF, min_x, min_y, roi_w, roi_h, payload_len);

            uint8 *dst = &s_packet_buf[packet_len];
            uint32 cursor = 0U;
            for (uint16 y = min_y; y <= max_y; y++)
            {
                uint32 src_row = (uint32)y * (uint32)s_width;
                memcpy(&dst[cursor], &frame_gray[src_row + min_x], roi_w);
                memcpy(&s_prev[y][min_x], &frame_gray[src_row + min_x], roi_w);
                cursor += roi_w;
            }
            packet_len += payload_len;
            s_stats.diff_frames++;
        }
    }

    s_packet_buf[packet_len] = calc_sum8(s_packet_buf, packet_len);
    packet_len += 1U;
    wifi_spi_send_buffer(s_packet_buf, packet_len);

    s_stats.total_bytes += packet_len;
    s_stats.frame_id++;
}

const wifi_diff_stream_stats_t *wifi_diff_stream_get_stats(void)
{
    return &s_stats;
}
