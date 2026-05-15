#include "cf_fusion.h"
#include "gnss_transform.h"
#include "inertial_nav.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define CF_FUSION_DEFAULT_ALPHA       0.98f
#define CF_FUSION_GNSS_REJECT_DIST_MM 5000.0f
#define DEG2RAD_F(x)                  ((x) * (M_PI / 180.0f))

CF_Fusion_t cf_fusion = {0};

static float s_last_inertial_x = 0.0f;
static float s_last_inertial_y = 0.0f;
static float s_local_heading_rad = 0.0f;

static float CF_Fusion_ComputeGnssLocalXmm(void)
{
    float de = gnss_trans.x * 1000.0f;
    float dn = gnss_trans.y * 1000.0f;
    float sin_h = sinf(s_local_heading_rad);
    float cos_h = cosf(s_local_heading_rad);

    return -(de * sin_h + dn * cos_h);
}

static float CF_Fusion_ComputeGnssLocalYmm(void)
{
    float de = gnss_trans.x * 1000.0f;
    float dn = gnss_trans.y * 1000.0f;
    float sin_h = sinf(s_local_heading_rad);
    float cos_h = cosf(s_local_heading_rad);

    return de * cos_h - dn * sin_h;
}

void CF_Fusion_Init(void)
{
    cf_fusion.x = 0.0f;
    cf_fusion.y = 0.0f;
    cf_fusion.yaw = 0.0f;
    cf_fusion.alpha = CF_FUSION_DEFAULT_ALPHA;
    cf_fusion.ready = 0U;
    cf_fusion.gnss_ready = 0U;
    cf_fusion.last_gnss_update_count = 0U;

    s_last_inertial_x = 0.0f;
    s_last_inertial_y = 0.0f;
    s_local_heading_rad = 0.0f;
}

void CF_Fusion_ResetLocalFrame(float start_heading_deg)
{
    s_local_heading_rad = DEG2RAD_F(start_heading_deg);

    cf_fusion.x = 0.0f;
    cf_fusion.y = 0.0f;
    cf_fusion.yaw = inertial_nav.relative_yaw;
    cf_fusion.alpha = CF_FUSION_DEFAULT_ALPHA;
    cf_fusion.ready = 1U;
    cf_fusion.gnss_ready = 0U;
    cf_fusion.last_gnss_update_count = gnss_trans.update_count;

    s_last_inertial_x = inertial_nav.x;
    s_last_inertial_y = inertial_nav.y;
}

void CF_Fusion_Update(void)
{
    float pred_x = 0.0f;
    float pred_y = 0.0f;
    float gnss_local_x = 0.0f;
    float gnss_local_y = 0.0f;
    float err_x = 0.0f;
    float err_y = 0.0f;
    float err_dist = 0.0f;

    if (!cf_fusion.ready)
    {
        return;
    }

    cf_fusion.x += inertial_nav.x - s_last_inertial_x;
    cf_fusion.y += inertial_nav.y - s_last_inertial_y;
    cf_fusion.yaw = inertial_nav.relative_yaw;

    s_last_inertial_x = inertial_nav.x;
    s_last_inertial_y = inertial_nav.y;

    if (!(gnss_trans.is_valid && gnss_trans.is_origin_set))
    {
        return;
    }

    if (gnss_trans.update_count == cf_fusion.last_gnss_update_count)
    {
        return;
    }

    cf_fusion.last_gnss_update_count = gnss_trans.update_count;
    cf_fusion.gnss_ready = 1U;

    pred_x = cf_fusion.x;
    pred_y = cf_fusion.y;
    gnss_local_x = CF_Fusion_ComputeGnssLocalXmm();
    gnss_local_y = CF_Fusion_ComputeGnssLocalYmm();

    err_x = gnss_local_x - pred_x;
    err_y = gnss_local_y - pred_y;
    err_dist = sqrtf(err_x * err_x + err_y * err_y);

    if (err_dist > CF_FUSION_GNSS_REJECT_DIST_MM)
    {
        return;
    }

    cf_fusion.x = cf_fusion.alpha * pred_x + (1.0f - cf_fusion.alpha) * gnss_local_x;
    cf_fusion.y = cf_fusion.alpha * pred_y + (1.0f - cf_fusion.alpha) * gnss_local_y;
}
