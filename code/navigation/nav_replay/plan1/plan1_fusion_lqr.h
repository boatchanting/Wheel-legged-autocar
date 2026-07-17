#ifndef _PLAN1_FUSION_LQR_H_
#define _PLAN1_FUSION_LQR_H_

/*
 * Plan1 GNSS + inertial fusion local-projection LQR-style tracking.
 *
 * target_speed_set: chassis speed command from route target_speed, with slew.
 * err_degree: steering/yaw command = local feedback + curvature/yaw-rate feedforward.
 *
 * Control structure:
 *   u = u_ff(kappa_preview, speed)
 *     + K_y * e_y
 *     + K_psi * e_psi
 *     + K_r * (r_ref - r_actual)
 *
 * e_y/e_psi are calculated from the closest local path segment projection.
 * x/y come from plan1_fusion_pose; control logic stays aligned with Plan1 LQR.
 * Preview is used only for curvature feedforward. Negative target_speed means forward.
 */

#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE   1

#define NAV_DIST_ARRIVE                    20.0f
#define NAV_START_HEADING_TOLERANCE         0.3f
#define NAV_SPEED_STOP                      0.0f

/* 融合定位更新周期，必须和惯导 10ms 更新周期一致。 */
#define PLAN1_FUSION_DT_S                   0.01f

/* GPS 融合质量与跳变抑制参数。除特别说明外，单位均为 mm。 */
#define PLAN1_FUSION_MIN_SAT_USED           4U      /* GPS 最低可用卫星数，低于该值完全不融合。 */
#define PLAN1_FUSION_GOOD_SAT_USED          7U      /* GPS 正常融合卫星数，低于该值只做弱融合。 */
#define PLAN1_FUSION_GPS_NORMAL_GAIN        0.08f   /* GPS 可信时的残差融合比例，越大越快贴近 GPS。 */
#define PLAN1_FUSION_GPS_WEAK_GAIN          0.02f   /* GPS 可疑时的残差融合比例，用于慢慢拉回漂移。 */
#define PLAN1_FUSION_GPS_NORMAL_STEP_MM     80.0f   /* GPS 可信时单次最大位置修正量，防止控制突跳。 */
#define PLAN1_FUSION_GPS_WEAK_STEP_MM       25.0f   /* GPS 可疑时单次最大位置修正量，比正常融合更保守。 */
#define PLAN1_FUSION_GPS_WEAK_RESIDUAL_MM   800.0f  /* GPS 与预测残差超过该值后降级为弱融合。 */
#define PLAN1_FUSION_GPS_REJECT_RESIDUAL_MM 2500.0f /* GPS 与预测残差超过该值直接丢弃本帧。 */
#define PLAN1_FUSION_GPS_MAX_FRAME_STEP_MM  700.0f  /* GPS 相邻两帧位移超过该值认为发生跳点。 */
#define PLAN1_FUSION_MAX_SPEED_MM_S         3500.0f /* 当前车速超过该值时降低 GPS 修正权重。 */
#define PLAN1_FUSION_GPS_TO_PATH_YAW_DEG    0.0f    /* GPS东/北坐标旋转到科目一路径坐标的初始兜底角度；正常发车后会用起步直线自动估算覆盖。 */

/* 起步直线自动对齐参数：先用惯导跑前几米，只采样 GPS，不修正融合位置；对齐成功后才允许 GPS 渐进修正。 */
#define PLAN1_FUSION_ALIGN_MIN_INS_DIST_MM  5000.0f /* 惯导起步位移达到该距离后，才开始计算 GPS->路径坐标旋转角；建议 4000~6000。 */
#define PLAN1_FUSION_ALIGN_MIN_GPS_DIST_MM  3500.0f /* GPS 起步位移达到该距离后才参与对齐；普通 GPS 抖动大时可适当加大。 */
#define PLAN1_FUSION_ALIGN_TIMEOUT_TICKS    1200U   /* 对齐最长等待时间，单位 10ms；1200 表示 12s，超时直接停车。 */
#define PLAN1_FUSION_ALIGN_MIN_SAT_USED        6U   /* 对齐阶段最低卫星数；低于该值只等待，不用于估算旋转角。 */
#define PLAN1_FUSION_ALIGN_MAX_FRAME_STEP_MM 900.0f /* 对齐阶段 GPS 相邻两帧最大允许位移；超过认为 GPS 跳点并停车。 */
#define PLAN1_FUSION_ALIGN_MAX_DIR_ERR_DEG   25.0f  /* 连续两次直线方向估算最大允许差；越小越严格，普通 GPS 建议 20~35 度。 */

/* 轮速不可信保护：掉头/打滑/原地转向时，轮速计可能虚高，融合位置不再直接跟随轮速积分。 */
#define PLAN1_FUSION_UNRELIABLE_PREDICT_SCALE   0.0f    /* 轮速不可信时惯导位置增量缩放；0 表示冻结轮速位置预测，只保留 IMU 航向。 */
#define PLAN1_FUSION_UNRELIABLE_GPS_GAIN        0.18f   /* 轮速不可信时 GPS 残差融合比例；比正常融合更愿意把位置拉回。 */
#define PLAN1_FUSION_UNRELIABLE_GPS_STEP_MM     160.0f  /* 轮速不可信时单次 GPS 最大修正量；仍限幅，避免普通 GPS 跳点直接打控制。 */
#define PLAN1_FUSION_UNRELIABLE_REJECT_RESIDUAL_MM 4500.0f /* 轮速不可信时 GPS 残差拒绝阈值；冻结预测后允许更大的可恢复残差。 */
#define PLAN1_FUSION_UNRELIABLE_CURVATURE_TH    0.0010f /* 普通掉头/大曲率路径入口触发阈值；只保护刚进入掉头区的一小段。 */
#define PLAN1_FUSION_UNRELIABLE_YAW_RATE_RAD_S  0.85f   /* 实际 IMU 角速度触发阈值；原地转向或急掉头时进入轮速不可信模式。 */
#define PLAN1_FUSION_UNRELIABLE_HOLD_TICKS        40U   /* 触发后保持时间，单位 10ms；防止掉头边界状态频繁抖动。 */
#define PLAN1_FUSION_TURN_ENTRY_UNRELIABLE_TICKS  25U   /* 从直线进入掉头/大曲率区域后的轮速保护窗口，单位 10ms；默认只保护前 250ms。 */

/* 融合状态蜂鸣请求：由融合模块置位，0 核 ISR 非阻塞消费。 */
#define PLAN1_FUSION_BEEP_NONE                 0U /* 无蜂鸣请求。 */
#define PLAN1_FUSION_BEEP_ALIGN_READY          1U /* GPS/惯导坐标旋转对齐成功：短鸣 1 声。 */
#define PLAN1_FUSION_BEEP_GPS_CORRECT_START    2U /* 第一次真正 GPS 位置修正开始：短鸣 2 声。 */
#define PLAN1_FUSION_BEEP_ALIGN_FAILED         3U /* 对齐失败停车：长鸣 1 声。 */

/* Curvature preview only. Feedback never uses preview point yaw. */
#define LQR_PREVIEW_POINTS                  3U
#define LQR_SHARP_CURVATURE_TH              0.0010f
#define LQR_SHARP_PREVIEW_POINTS            4U

#define LQR_SEARCH_RANGE_NORMAL            80U
#define LQR_SEARCH_RANGE_RECOVER          300U
#define LQR_MAX_TRACK_DIST_MM             800.0f
#define LQR_PROJECTION_MIN_SEG_LEN_MM       1.0f

/* Global steering sign. Flip this first if real-car left/right is reversed. */
#define LQR_SIGN                            1.0f

/* Feedback and feedforward gains. */
#define LQR_K_YAW_RATE_FF                   8.0f
#define LQR_K_LATERAL                       0.025f
#define LQR_K_HEADING                       0.75f

/*
 * Optional yaw-rate feedback. Default is off: inertial_nav.actual_yaw_rate exists,
 * but enable this only after sensor sign/noise are verified on the real car.
 */
#define LQR_USE_ACTUAL_YAW_RATE_FB          0
#define LQR_K_YAW_RATE                      0.0f

#define LQR_CURVATURE_SPEED_SIGN_ENABLE     1
#define LQR_FORWARD_SPEED_IS_NEGATIVE       1
#define LQR_SPEED_TO_MM_S                   4.936f

/* Low speed keeps authority; high speed tightens stability envelopes. */
#define LQR_LOW_SPEED_MM_S                500.0f
#define LQR_HIGH_SPEED_MM_S              2500.0f
#define LQR_LOW_SPEED_ERR_MAX_DEG          45.0f
#define LQR_HIGH_SPEED_ERR_MAX_DEG         28.0f
#define LQR_LOW_SPEED_ERR_SLEW_DEG         24.0f
#define LQR_HIGH_SPEED_ERR_SLEW_DEG        10.0f
#define LQR_LOW_SPEED_FILTER_ALPHA          0.80f
#define LQR_HIGH_SPEED_FILTER_ALPHA         0.45f
#define LQR_LOW_SPEED_YAW_RATE_LIMIT_RAD_S  3.20f
#define LQR_HIGH_SPEED_YAW_RATE_LIMIT_RAD_S 1.80f
#define LQR_LOW_SPEED_YAW_ACCEL_LIMIT_RAD_S2 80.0f
#define LQR_HIGH_SPEED_YAW_ACCEL_LIMIT_RAD_S2 35.0f
#define LQR_LOW_SPEED_LATERAL_ACCEL_MM_S2  4200.0f
#define LQR_HIGH_SPEED_LATERAL_ACCEL_MM_S2 2600.0f
#define LQR_CONTROL_PERIOD_S                0.01f
#define LQR_OUTPUT_DEG_PER_RADPS           30.0f
#define LQR_LATERAL_ERR_LIMIT_MM          600.0f

/* Route speed command slew, kept compatible with the existing replay style. */
#define NAV_SPEED_SLEW_EPS                  50.0f
#define NAV_SPEED_SLEW_LOW_SPEED_TH        80.0f
#define NAV_SPEED_SLEW_FAST_DECEL_TH      220.0f
#define NAV_SPEED_SLEW_UP_LOW              30.0f
#define NAV_SPEED_SLEW_UP_NORMAL           45.0f
#define NAV_SPEED_SLEW_DOWN_NORMAL        400.0f
#define NAV_SPEED_SLEW_DOWN_FAST          800.0f
#define NAV_SPEED_SLEW_DOWN_CROSS_ZERO    120.0f
#define NAV_STOP_LOCK_SPEED_EPS             1.0f

typedef enum
{
    PLAN1_FUSION_MODE_INS_ONLY = 0,
    PLAN1_FUSION_MODE_GPS_WEAK,
    PLAN1_FUSION_MODE_GPS_NORMAL,
    PLAN1_FUSION_MODE_GPS_REJECT,
    PLAN1_FUSION_MODE_ALIGNING,
    PLAN1_FUSION_MODE_ALIGN_FAILED,
    PLAN1_FUSION_MODE_WHEEL_UNRELIABLE
} Plan1FusionMode_e;

typedef enum
{
    PLAN1_FUSION_GPS_QUALITY_INVALID = 0,
    PLAN1_FUSION_GPS_QUALITY_WEAK,
    PLAN1_FUSION_GPS_QUALITY_NORMAL
} Plan1FusionGpsQuality_e;

typedef struct
{
    float x_mm;
    float y_mm;
    float yaw_deg;
    float vx_mm_s;
    float vy_mm_s;
    uint8 gps_used;
    uint8 gps_quality;
    float gps_residual_mm;
    uint8 fusion_mode;
} Plan1FusionPose_t;

extern volatile float target_speed_set;
extern volatile float err_degree;
extern volatile float roll_degree;

extern NavReplayState_e g_replay_state;
extern uint16 g_target_idx;
extern uint8 g_current_point_type;
extern uint8 g_special_action_trigger;
extern uint8 g_plan1_fast_uturn_state;
extern uint8 g_plan1_fast_uturn_lead;
extern NavReplayState_e g_gps_replay_state;
extern uint8 g_gps_current_point_type;
extern uint8 g_gps_special_action_trigger;

extern Plan1FusionPose_t plan1_fusion_pose;
extern volatile uint8 plan1_fusion_beep_request;

void NavReplay_Start(void);
void NavReplay_Stop(void);
void NavReplay_Process(void);
uint16 NavReplay_LoadStaticRouteToRam(void);

void Plan1FusionLqr_Start(void);
void Plan1FusionLqr_Stop(void);
void Plan1FusionLqr_Process(void);
void Plan1FusionLqr_UpdateFusion10ms(void);
void Plan1FusionLqr_CorrectWithGnss(void);

#endif
