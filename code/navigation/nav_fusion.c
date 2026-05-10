#include "nav_fusion.h"
#include "inertial_nav.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define NAV_FUSION_IMU_DT_S           (0.01f)
#define NAV_FUSION_GNSS_DT_S          (0.10f)
#define NAV_FUSION_GNSS_POS_ALPHA     (0.35f)
#define NAV_FUSION_GNSS_VEL_ALPHA     (0.30f)
#define NAV_FUSION_MIN_GNSS_STEP_MM   (5.0f)

#define DEG2RAD_F(x) ((x) * M_PI / 180.0f)

NavFusionState_t nav_fusion;

static uint8_t s_has_gnss_anchor = 0U;
static float s_last_gnss_x_mm = 0.0f;
static float s_last_gnss_y_mm = 0.0f;

void NavFusion_Init(void)
{
    nav_fusion.x_mm = 0.0f;
    nav_fusion.y_mm = 0.0f;
    nav_fusion.vx_mm_s = 0.0f;
    nav_fusion.vy_mm_s = 0.0f;
    nav_fusion.gnss_valid = 0U;
    nav_fusion.update_imu_count = 0U;
    nav_fusion.update_gnss_count = 0U;

    s_has_gnss_anchor = 0U;
    s_last_gnss_x_mm = 0.0f;
    s_last_gnss_y_mm = 0.0f;
}

void NavFusion_UpdateImu10ms(float world_heading_deg)
{
    float yaw_rad = DEG2RAD_F(world_heading_deg);
    float cos_yaw = cosf(yaw_rad);
    float sin_yaw = sinf(yaw_rad);

    float vx_world = inertial_nav.vx_body * cos_yaw - inertial_nav.vy_body * sin_yaw;
    float vy_world = inertial_nav.vx_body * sin_yaw + inertial_nav.vy_body * cos_yaw;

    nav_fusion.vx_mm_s = vx_world;
    nav_fusion.vy_mm_s = vy_world;

    nav_fusion.x_mm += nav_fusion.vx_mm_s * NAV_FUSION_IMU_DT_S;
    nav_fusion.y_mm += nav_fusion.vy_mm_s * NAV_FUSION_IMU_DT_S;
    nav_fusion.update_imu_count++;
}

void NavFusion_UpdateGnss100ms(float gnss_x_m, float gnss_y_m, uint8_t is_valid, float world_heading_deg)
{
    float gnss_x_mm = gnss_x_m * 1000.0f;
    float gnss_y_mm = gnss_y_m * 1000.0f;

    nav_fusion.gnss_valid = is_valid;
    if (!is_valid)
    {
        return;
    }

    if (!s_has_gnss_anchor)
    {
        nav_fusion.x_mm = gnss_x_mm;
        nav_fusion.y_mm = gnss_y_mm;
        s_last_gnss_x_mm = gnss_x_mm;
        s_last_gnss_y_mm = gnss_y_mm;
        s_has_gnss_anchor = 1U;
        nav_fusion.update_gnss_count++;
        return;
    }

    float gnss_dx = gnss_x_mm - s_last_gnss_x_mm;
    float gnss_dy = gnss_y_mm - s_last_gnss_y_mm;

    if ((fabsf(gnss_dx) + fabsf(gnss_dy)) > NAV_FUSION_MIN_GNSS_STEP_MM)
    {
        float gnss_vx = gnss_dx / NAV_FUSION_GNSS_DT_S;
        float gnss_vy = gnss_dy / NAV_FUSION_GNSS_DT_S;

        nav_fusion.vx_mm_s = (1.0f - NAV_FUSION_GNSS_VEL_ALPHA) * nav_fusion.vx_mm_s
                           + NAV_FUSION_GNSS_VEL_ALPHA * gnss_vx;
        nav_fusion.vy_mm_s = (1.0f - NAV_FUSION_GNSS_VEL_ALPHA) * nav_fusion.vy_mm_s
                           + NAV_FUSION_GNSS_VEL_ALPHA * gnss_vy;
    }

    nav_fusion.x_mm = (1.0f - NAV_FUSION_GNSS_POS_ALPHA) * nav_fusion.x_mm
                    + NAV_FUSION_GNSS_POS_ALPHA * gnss_x_mm;
    nav_fusion.y_mm = (1.0f - NAV_FUSION_GNSS_POS_ALPHA) * nav_fusion.y_mm
                    + NAV_FUSION_GNSS_POS_ALPHA * gnss_y_mm;

    s_last_gnss_x_mm = gnss_x_mm;
    s_last_gnss_y_mm = gnss_y_mm;
    nav_fusion.update_gnss_count++;

    (void)world_heading_deg;
}
