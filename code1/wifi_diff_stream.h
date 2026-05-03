#ifndef __CODE1_WIFI_DIFF_STREAM_H__
#define __CODE1_WIFI_DIFF_STREAM_H__

#include "zf_common_headfile.h"

// Diff-stream protocol frame types
#define WIFI_DIFF_FRAME_KEY   (1U)
#define WIFI_DIFF_FRAME_DIFF  (0U)
#define WIFI_DIFF_FRAME_SKIP  (2U)

typedef struct
{
    uint32 frame_id;
    uint32 keyframes;
    uint32 diff_frames;
    uint32 skip_frames;
    uint32 total_bytes;
} wifi_diff_stream_stats_t;

void wifi_diff_stream_init(uint16 width, uint16 height, uint16 keyframe_interval, uint8 diff_threshold);
void wifi_diff_stream_send_gray_frame(const uint8 *frame_gray);
const wifi_diff_stream_stats_t *wifi_diff_stream_get_stats(void);

#endif
