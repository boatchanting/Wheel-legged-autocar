#ifndef _FUSED_NAV_H_
#define _FUSED_NAV_H_

#include "zf_common_headfile.h"

#define FUSED_NAV_GPS_HISTORY_LEN                 10U
#define FUSED_NAV_GPS_BLOCK_SAMPLES               20U
#define FUSED_NAV_FORCE_RECOVER_REJECT_COUNT      20U
#define FUSED_NAV_MAX_GPS_SPEED_M_S               8.0f
#define FUSED_NAV_MAX_ALLOWED_INNOVATION_MM       1500.0f
#define FUSED_NAV_GPS_RANDOM_WALK_SIGMA_MM        500.0f
#define FUSED_NAV_LONG_SLIP_ACC_THRES_MM_S2       3500.0f
#define FUSED_NAV_YAW_SLIP_THRES_RAD_S            0.50f

typedef struct
{
    float x_odom;
    float y_odom;
    float offset_x;
    float offset_y;
    float fused_x;
    float fused_y;
    float fused_yaw;
    float beta;

    float vx_body;
    float vy_body;

    float last_wheel_speed_mm_s;
    uint8_t has_last_wheel_speed;

    float last_gps_sample_x_mm;
    float last_gps_sample_y_mm;
    uint32_t last_gps_sample_tick_ms;
    uint8_t has_last_gps_sample;
    uint8_t gps_reject_count;

    float start_heading_deg;
    uint8_t start_heading_locked;

    uint8_t gps_is_valid;
    uint8_t latest_satellite_used;
    uint8_t gps_block_countdown;
    uint8_t last_satellite_used;

    uint32_t last_gnss_update_count;

    float gps_dx_hist[FUSED_NAV_GPS_HISTORY_LEN];
    float gps_dy_hist[FUSED_NAV_GPS_HISTORY_LEN];
    uint8_t gps_hist_count;
    uint8_t gps_hist_head;
} FusedNav_t;

extern FusedNav_t fused_nav;

void FusedNav_Init(void);
void FusedNav_ResetSession(void);
void FusedNav_FreezeVelocity(void);
/* imu_gyro_z_rad_s is only used for yaw-slip detection. */
void FusedNav_UpdateOdom_100Hz(float current_yaw_deg,
                               float acc_lat_left_mm_s2,
                               float acc_lon_forward_mm_s2,
                               float speed_l,
                               float speed_r,
                               float imu_gyro_z_rad_s);
uint8_t FusedNav_UpdateGpsIfFresh(void);
void FusedNav_CalcOutput(void);

#endif
