#include "nav_fusion.h"

#include "inertial_nav.h"
#include "gnss_transform.h"

#ifndef M_PI
#define M_PI 3.1415926535f
#endif
#define DEG2RAD(x) ((x) * M_PI / 180.0f)

#define NAV_FUSION_PREDICT_DT_S       (0.01f)
#define NAV_FUSION_GNSS_DT_S          (0.10f)
#define NAV_FUSION_VEL_BLEND_ALPHA    (0.65f)
#define NAV_FUSION_POS_CORR_GAIN      (0.12f)
#define NAV_FUSION_MAX_GNSS_SPEED     (7000.0f)

nav_fusion_t g_nav_fusion;

static float s_last_gnss_x_m = 0.0f;
static float s_last_gnss_y_m = 0.0f;
static uint8_t s_has_last_gnss = 0U;

void NavFusion_Init(void)
{
    g_nav_fusion.fused_vx_body_mm_s = 0.0f;
    g_nav_fusion.fused_x_mm = 0.0f;
    g_nav_fusion.fused_y_mm = 0.0f;
    g_nav_fusion.is_initialized = 1U;
    g_nav_fusion.predict_count = 0U;
    g_nav_fusion.gnss_update_count = 0U;

    s_last_gnss_x_m = 0.0f;
    s_last_gnss_y_m = 0.0f;
    s_has_last_gnss = 0U;
}

void NavFusion_Predict_10ms(void)
{
    if (g_nav_fusion.is_initialized == 0U)
    {
        NavFusion_Init();
    }

    g_nav_fusion.predict_count++;

    g_nav_fusion.fused_vx_body_mm_s = inertial_nav.vx_body;

    g_nav_fusion.fused_x_mm += inertial_nav.vx_body * NAV_FUSION_PREDICT_DT_S;
}

void NavFusion_UpdateGnss_100ms(void)
{
    if (gnss_trans.is_valid == 0U || gnss_trans.is_origin_set == 0U)
    {
        return;
    }

    if (s_has_last_gnss == 0U)
    {
        s_last_gnss_x_m = gnss_trans.x;
        s_last_gnss_y_m = gnss_trans.y;
        s_has_last_gnss = 1U;

        g_nav_fusion.fused_x_mm = gnss_trans.x * 1000.0f;
        g_nav_fusion.fused_y_mm = gnss_trans.y * 1000.0f;
        return;
    }

    float dx_mm = (gnss_trans.x - s_last_gnss_x_m) * 1000.0f;
    float dy_mm = (gnss_trans.y - s_last_gnss_y_m) * 1000.0f;

    s_last_gnss_x_m = gnss_trans.x;
    s_last_gnss_y_m = gnss_trans.y;

    float yaw_rad = DEG2RAD(inertial_nav.relative_yaw);
    float cos_yaw = cosf(yaw_rad);
    float sin_yaw = sinf(yaw_rad);

    float gnss_vx_world = dx_mm / NAV_FUSION_GNSS_DT_S;
    float gnss_vy_world = dy_mm / NAV_FUSION_GNSS_DT_S;

    float gnss_vx_body = gnss_vx_world * cos_yaw + gnss_vy_world * sin_yaw;
    if (fabsf(gnss_vx_body) < NAV_FUSION_MAX_GNSS_SPEED)
    {
        g_nav_fusion.fused_vx_body_mm_s =
            NAV_FUSION_VEL_BLEND_ALPHA * inertial_nav.vx_body +
            (1.0f - NAV_FUSION_VEL_BLEND_ALPHA) * gnss_vx_body;

        inertial_nav.vx_body = g_nav_fusion.fused_vx_body_mm_s;
    }

    float gnss_abs_x_mm = gnss_trans.x * 1000.0f;
    float gnss_abs_y_mm = gnss_trans.y * 1000.0f;

    g_nav_fusion.fused_x_mm += NAV_FUSION_POS_CORR_GAIN * (gnss_abs_x_mm - g_nav_fusion.fused_x_mm);
    g_nav_fusion.fused_y_mm += NAV_FUSION_POS_CORR_GAIN * (gnss_abs_y_mm - g_nav_fusion.fused_y_mm);

    inertial_nav.x = g_nav_fusion.fused_x_mm;
    inertial_nav.y = g_nav_fusion.fused_y_mm;

    g_nav_fusion.gnss_update_count++;
}
