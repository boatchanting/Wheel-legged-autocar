#ifndef __NAV_FUSION_H__
#define __NAV_FUSION_H__

#include "zf_common_headfile.h"

typedef struct
{
    float fused_vx_body_mm_s;
    float fused_x_mm;
    float fused_y_mm;
    uint8_t is_initialized;
    uint32_t predict_count;
    uint32_t gnss_update_count;
} nav_fusion_t;

extern nav_fusion_t g_nav_fusion;

void NavFusion_Init(void);
void NavFusion_Predict_10ms(void);
void NavFusion_UpdateGnss_100ms(void);

#endif
