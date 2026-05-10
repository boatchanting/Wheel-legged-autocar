#ifndef _NAV_FUSION_H_
#define _NAV_FUSION_H_

#include "zf_common_headfile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float x_mm;
    float y_mm;
    float vx_mm_s;
    float vy_mm_s;
    uint8_t gnss_valid;
    uint32_t update_imu_count;
    uint32_t update_gnss_count;
} NavFusionState_t;

extern NavFusionState_t nav_fusion;

void NavFusion_Init(void);
void NavFusion_UpdateImu10ms(float world_heading_deg);
void NavFusion_UpdateGnss100ms(float gnss_x_m, float gnss_y_m, uint8_t is_valid, float world_heading_deg);

#ifdef __cplusplus
}
#endif

#endif
