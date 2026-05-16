#include "fused_nav.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define FUSED_NAV_DT                    0.01f
#define FUSED_NAV_VEL_BLEND_ALPHA       0.90f
#define FUSED_NAV_LATERAL_ACC_DEADZONE  200.0f

FusedNav_t fused_nav = {0};
extern uint32_t loop_counter;

static float FusedNav_NormalizeAngle180(float angle)
{
    while (angle > 180.0f)  angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

static float FusedNav_NormalizeAngle360(float angle)
{
    while (angle >= 360.0f) angle -= 360.0f;
    while (angle < 0.0f) angle += 360.0f;
    return angle;
}

static float FusedNav_CurrentAbsoluteHeadingDeg(void)
{
#if IMU_CATEGORY == 3
    return FusedNav_NormalizeAngle360(heading);
#else
    return FusedNav_NormalizeAngle360(gnss.direction);
#endif
}

/* fused_yaw always follows IMU attitude output rather than wheel-diff yaw integration. */
static void FusedNav_PushGpsDelta(float dx, float dy)
{
    fused_nav.gps_dx_hist[fused_nav.gps_hist_head] = dx;
    fused_nav.gps_dy_hist[fused_nav.gps_hist_head] = dy;
    fused_nav.gps_hist_head = (uint8_t)((fused_nav.gps_hist_head + 1U) % FUSED_NAV_GPS_HISTORY_LEN);

    if (fused_nav.gps_hist_count < FUSED_NAV_GPS_HISTORY_LEN)
    {
        fused_nav.gps_hist_count++;
    }
}

static float FusedNav_ComputeGpsStepSigma(void)
{
    if (fused_nav.gps_hist_count < 2U)
    {
        return 0.0f;
    }

    float mean_dx = 0.0f;
    float mean_dy = 0.0f;
    float var_dx = 0.0f;
    float var_dy = 0.0f;
    uint8_t count = fused_nav.gps_hist_count;

    for (uint8_t i = 0U; i < count; i++)
    {
        mean_dx += fused_nav.gps_dx_hist[i];
        mean_dy += fused_nav.gps_dy_hist[i];
    }

    mean_dx /= (float)count;
    mean_dy /= (float)count;

    for (uint8_t i = 0U; i < count; i++)
    {
        float ddx = fused_nav.gps_dx_hist[i] - mean_dx;
        float ddy = fused_nav.gps_dy_hist[i] - mean_dy;
        var_dx += ddx * ddx;
        var_dy += ddy * ddy;
    }

    var_dx /= (float)count;
    var_dy /= (float)count;
    return sqrtf(var_dx + var_dy);
}

static void FusedNav_GlobalToLocal(float east_mm, float north_mm, float *out_x, float *out_y)
{
    float heading_rad = fused_nav.start_heading_deg * (M_PI / 180.0f);
    float cos_h = cosf(heading_rad);
    float sin_h = sinf(heading_rad);

    *out_x = -(east_mm * cos_h + north_mm * sin_h);
    *out_y = east_mm * sin_h - north_mm * cos_h;
}

void FusedNav_Init(void)
{
    memset(&fused_nav, 0, sizeof(fused_nav));
    fused_nav.beta = 0.95f;
}

void FusedNav_ResetSession(void)
{
    FusedNav_Init();
}

void FusedNav_FreezeVelocity(void)
{
    fused_nav.vx_body = 0.0f;
    fused_nav.vy_body = 0.0f;
    fused_nav.has_last_wheel_speed = 0U;
}

void FusedNav_UpdateOdom_100Hz(float current_yaw_deg,
                               float acc_lat_left_mm_s2,
                               float acc_lon_forward_mm_s2,
                               float speed_l,
                               float speed_r,
                               float imu_gyro_z_rad_s)
{
    float v_l_mm = -speed_l * SPEED_TO_MM_S;
    float v_r_mm = speed_r * SPEED_TO_MM_S;
    float v_wheel_avg = (v_l_mm + v_r_mm) * 0.5f;
    float wheel_yaw_rate = (v_r_mm - v_l_mm) / WHEEL_BASE_MM;
    float wheel_acc = 0.0f;
    float v_pred = 0.0f;
    float yaw_deg = FusedNav_NormalizeAngle180(current_yaw_deg);
    float yaw_rad = yaw_deg * (M_PI / 180.0f);
    float long_slip = 0.0f;
    float yaw_slip = 0.0f;

    if (fused_nav.has_last_wheel_speed)
    {
        wheel_acc = (v_wheel_avg - fused_nav.last_wheel_speed_mm_s) / FUSED_NAV_DT;
    }
    fused_nav.last_wheel_speed_mm_s = v_wheel_avg;
    fused_nav.has_last_wheel_speed = 1U;

    if ((fabsf(v_wheel_avg) > 100.0f) &&
        (fabsf(wheel_acc - acc_lon_forward_mm_s2) > FUSED_NAV_LONG_SLIP_ACC_THRES_MM_S2))
    {
        long_slip = 1.0f;
    }

    if ((fabsf(v_wheel_avg) > 100.0f) &&
        (fabsf(wheel_yaw_rate - imu_gyro_z_rad_s) > FUSED_NAV_YAW_SLIP_THRES_RAD_S))
    {
        yaw_slip = 1.0f;
    }

    if (fabsf(v_wheel_avg) < 5.0f)
    {
        fused_nav.vx_body = 0.0f;
    }

    if (fabsf(speed_l + speed_r) < 5.0f)
    {
        acc_lat_left_mm_s2 = 0.0f;
        acc_lon_forward_mm_s2 = 0.0f;
    }

    v_pred = fused_nav.vx_body + acc_lon_forward_mm_s2 * FUSED_NAV_DT;
    if (long_slip > 0.5f)
    {
        fused_nav.vx_body = v_pred;
    }
    else
    {
        fused_nav.vx_body = FUSED_NAV_VEL_BLEND_ALPHA * v_wheel_avg +
                            (1.0f - FUSED_NAV_VEL_BLEND_ALPHA) * v_pred;
    }

    if ((fabsf(acc_lat_left_mm_s2) < FUSED_NAV_LATERAL_ACC_DEADZONE) || (yaw_slip > 0.5f))
    {
        fused_nav.vy_body *= 0.8f;
    }
    else
    {
        fused_nav.vy_body = fused_nav.vy_body * 0.95f + acc_lat_left_mm_s2 * FUSED_NAV_DT;
    }

    fused_nav.fused_yaw = yaw_deg;

    {
        float cos_theta = cosf(yaw_rad);
        float sin_theta = sinf(yaw_rad);
        float vx_world = fused_nav.vx_body * cos_theta - fused_nav.vy_body * sin_theta;
        float vy_world = fused_nav.vx_body * sin_theta + fused_nav.vy_body * cos_theta;

        fused_nav.x_odom += vx_world * FUSED_NAV_DT;
        fused_nav.y_odom += vy_world * FUSED_NAV_DT;
    }
}

uint8_t FusedNav_UpdateGpsIfFresh(void)
{
    float gps_east_mm = 0.0f;
    float gps_north_mm = 0.0f;
    float gps_x_mm = 0.0f;
    float gps_y_mm = 0.0f;
    float err_x = 0.0f;
    float err_y = 0.0f;
    float err_dist = 0.0f;
    float sigma_mm = 0.0f;
    float innovation_limit_mm = FUSED_NAV_MAX_ALLOWED_INNOVATION_MM;
    int32 sat_drop = 0;
    uint32_t curr_tick_ms = loop_counter;
    float dt_s = 0.0f;
    uint8_t had_prev_gps_sample = fused_nav.has_last_gps_sample;
    float prev_gps_sample_x_mm = fused_nav.last_gps_sample_x_mm;
    float prev_gps_sample_y_mm = fused_nav.last_gps_sample_y_mm;

    if (!gnss_trans.is_valid || !gnss_trans.is_origin_set || (gnss.state != 1U) || (gnss.satellite_used < 4U))
    {
        fused_nav.gps_is_valid = 0U;
        return 0U;
    }

    if (gnss_trans.update_count == fused_nav.last_gnss_update_count)
    {
        return 0U;
    }
    fused_nav.last_gnss_update_count = gnss_trans.update_count;
    fused_nav.gps_is_valid = 1U;
    fused_nav.latest_satellite_used = gnss.satellite_used;

    gps_east_mm = gnss_trans.x * 1000.0f;
    gps_north_mm = gnss_trans.y * 1000.0f;

    if (!fused_nav.start_heading_locked)
    {
        fused_nav.start_heading_deg = FusedNav_CurrentAbsoluteHeadingDeg();
        fused_nav.start_heading_locked = 1U;
    }

    FusedNav_GlobalToLocal(gps_east_mm, gps_north_mm, &gps_x_mm, &gps_y_mm);

    if (had_prev_gps_sample)
    {
        float step_dx = gps_x_mm - prev_gps_sample_x_mm;
        float step_dy = gps_y_mm - prev_gps_sample_y_mm;
        float step_dist = sqrtf(step_dx * step_dx + step_dy * step_dy);
        uint32_t dt_ms = curr_tick_ms - fused_nav.last_gps_sample_tick_ms;

        if (dt_ms > 0U)
        {
            dt_s = (float)dt_ms / 1000.0f;
        }

        if ((dt_s > 0.0f) &&
            ((step_dist / 1000.0f / dt_s) > FUSED_NAV_MAX_GPS_SPEED_M_S))
        {
            return 0U;
        }
    }

    fused_nav.last_gps_sample_x_mm = gps_x_mm;
    fused_nav.last_gps_sample_y_mm = gps_y_mm;
    fused_nav.last_gps_sample_tick_ms = curr_tick_ms;
    fused_nav.has_last_gps_sample = 1U;

    if (gnss.satellite_used < 10U)
    {
        fused_nav.beta = 0.999f;
    }
    else if (gnss.satellite_used <= 18U)
    {
        fused_nav.beta = 0.98f;
    }
    else
    {
        fused_nav.beta = 0.95f;
    }

    sat_drop = (int32)fused_nav.last_satellite_used - (int32)gnss.satellite_used;
    fused_nav.last_satellite_used = gnss.satellite_used;
    if (sat_drop >= 4)
    {
        fused_nav.gps_block_countdown = FUSED_NAV_GPS_BLOCK_SAMPLES;
        return 0U;
    }

    if (fused_nav.gps_block_countdown > 0U)
    {
        fused_nav.gps_block_countdown--;
        return 0U;
    }

    sigma_mm = FusedNav_ComputeGpsStepSigma();
    if ((fused_nav.gps_hist_count >= 4U) && (sigma_mm > FUSED_NAV_GPS_RANDOM_WALK_SIGMA_MM))
    {
        return 0U;
    }

    err_x = gps_x_mm - fused_nav.x_odom;
    err_y = gps_y_mm - fused_nav.y_odom;
    err_dist = sqrtf((gps_x_mm - fused_nav.fused_x) * (gps_x_mm - fused_nav.fused_x) +
                     (gps_y_mm - fused_nav.fused_y) * (gps_y_mm - fused_nav.fused_y));
    if ((fused_nav.gps_hist_count >= 4U) && ((3.0f * sigma_mm) > innovation_limit_mm))
    {
        innovation_limit_mm = 3.0f * sigma_mm;
    }
    if (err_dist > innovation_limit_mm)
    {
        if (fused_nav.gps_reject_count < 255U)
        {
            fused_nav.gps_reject_count++;
        }
        if (fused_nav.gps_reject_count > FUSED_NAV_FORCE_RECOVER_REJECT_COUNT)
        {
            fused_nav.offset_x = gps_x_mm - fused_nav.x_odom;
            fused_nav.offset_y = gps_y_mm - fused_nav.y_odom;
            fused_nav.gps_reject_count = 0U;
            FusedNav_CalcOutput();
            return 1U;
        }
        return 0U;
    }
    fused_nav.gps_reject_count = 0U;

    fused_nav.offset_x = fused_nav.beta * fused_nav.offset_x + (1.0f - fused_nav.beta) * err_x;
    fused_nav.offset_y = fused_nav.beta * fused_nav.offset_y + (1.0f - fused_nav.beta) * err_y;

    if (had_prev_gps_sample)
    {
        FusedNav_PushGpsDelta(gps_x_mm - prev_gps_sample_x_mm,
                              gps_y_mm - prev_gps_sample_y_mm);
    }

    FusedNav_CalcOutput();
    return 1U;
}

void FusedNav_CalcOutput(void)
{
    fused_nav.fused_x = fused_nav.x_odom + fused_nav.offset_x;
    fused_nav.fused_y = fused_nav.y_odom + fused_nav.offset_y;
    fused_nav.fused_yaw = FusedNav_NormalizeAngle180(fused_nav.fused_yaw);
}
