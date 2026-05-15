#ifndef _CF_FUSION_H_
#define _CF_FUSION_H_

#include "zf_common_headfile.h"

typedef struct
{
    float x;
    float y;
    float yaw;
    float alpha;
    uint8 ready;
    uint8 gnss_ready;
    uint32 last_gnss_update_count;
} CF_Fusion_t;

extern CF_Fusion_t cf_fusion;

void CF_Fusion_Init(void);
void CF_Fusion_ResetLocalFrame(float start_heading_deg);
void CF_Fusion_Update(void);

#endif // _CF_FUSION_H_
