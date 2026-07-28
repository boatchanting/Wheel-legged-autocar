#include "bumpy_road.h"
#include "vision/vision_ipc_core0.h"
#include "vision/vision_bumpy_control.h"
#include "navigation/inertial_nav.h"
#include "calculate/ekf.h"
#include "tools/sbus.h"
#include <math.h>

extern volatile uint8 exit_beep_request;

/* ========================= 参数区 ========================= */
#define BUMPY_ROAD_POST_CORRECTION_DISTANCE_MM (1500.0f)

#define BUMPY_ROAD_INIT_SPEED_SET        (-800.0f)      // 接近时的初始速度
#define BUMPY_ROAD_LOCK_SPEED_SET        (-800.0f)      // 颠簸段目标速度
#define BUMPY_ROAD_SPEED_INC_STEP        (1.0f)         // 每1ms速度增量 (斜率加速)

#define BUMPY_ROAD_VISUAL_EXIT_MIN_DISTANCE_MM (1000.0f) // 累计满 1m 后才允许由视觉确认出口
#define BUMPY_ROAD_TARGET_DISTANCE_MM    (4000.0f)      // 目标行驶距离(mm)，超过此距离自动结束任务
#define BUMPY_ROAD_SAMPLE_DIV_1MS        (10U)          // 距离采样分频系数，每10ms更新一次距离

#define BUMPY_ROAD_STEER_FILTER_ALPHA    (0.05f)        // 方向偏差轻度低通滤波系数 (0~1，越小越平滑)

#define BUMPY_ROAD_GYRO_WIN_SIZE         (200U)         // (改小窗口) 1000Hz下200ms的帧数，减小延迟
#define BUMPY_ROAD_ENTRY_STD_TH          (0.3f)         // (降低阈值) 进入特征标准差阈值 (rad/s)
#define BUMPY_ROAD_ENTRY_CONFIRM_FRAMES  (50U)          // (减少确认时间) 连续满足进入条件的帧数(50ms)

typedef struct
{
    BumpyRoadState_e state;
    float start_x_mm;
    float start_y_mm;
    float traveled_mm;
    
    uint16_t sample_div_cnt;

    BumpyRoadExitReason_e exit_reason;
    float exit_anchor_x_mm;
    float exit_anchor_y_mm;
    float correction_start_x_mm;
    float correction_start_y_mm;
    uint8_t exit_anchor_valid;
    uint8_t entry_confirmed;
    uint8_t visual_exit_armed;
    uint8_t correction_applied;

    BumpyRoadEvent_e last_event;
    uint32_t event_sequence;

    /* Gyro O(1) stddev 变量 */
    float gyro_z_buffer[BUMPY_ROAD_GYRO_WIN_SIZE];
    uint16_t gyro_z_idx;
    uint8_t gyro_buffer_full;
    float gyro_sum;
    float gyro_sum_sq;

    uint16_t bump_entry_cnt;
    uint8_t on_bump;
    float current_speed_set;
    float filtered_err_degree;

} BumpyRoadContext_t;

static BumpyRoadContext_t s_bumpy_ctx = {BUMPY_ROAD_STATE_IDLE};

static void BumpyRoad_PublishEvent(BumpyRoadEvent_e event)
{
    s_bumpy_ctx.last_event = event;
    s_bumpy_ctx.event_sequence++;
}

static float BumpyRoad_CalcDistanceMm(void)
{
    const float dx = inertial_nav.x - s_bumpy_ctx.start_x_mm;
    const float dy = inertial_nav.y - s_bumpy_ctx.start_y_mm;
    return sqrtf(dx * dx + dy * dy);
}

static float BumpyRoad_CalcCorrectionDistanceMm(void)
{
    const float dx = inertial_nav.x - s_bumpy_ctx.correction_start_x_mm;
    const float dy = inertial_nav.y - s_bumpy_ctx.correction_start_y_mm;
    return sqrtf(dx * dx + dy * dy);
}

static void BumpyRoad_ApplyVisionSteer(void)
{
    float target_err = 0.0f;
    if (VisionBumpyControl_IsEnabled())
    {
        target_err = VisionBumpyControl_GetErrDegreeCmd();
    }
    
    // 一阶低通滤波 (EMA)，减少视觉识别跳变带来的左右扭动
    s_bumpy_ctx.filtered_err_degree = (target_err * BUMPY_ROAD_STEER_FILTER_ALPHA) + 
                                      (s_bumpy_ctx.filtered_err_degree * (1.0f - BUMPY_ROAD_STEER_FILTER_ALPHA));
    err_degree = s_bumpy_ctx.filtered_err_degree;
}

static void BumpyRoad_Cleanup(uint8_t stop_car)
{
    const uint8_t was_active = (s_bumpy_ctx.state != BUMPY_ROAD_STATE_IDLE) ? 1U : 0U;

    VisionBumpyControl_SetEnable(0U);
    VisionIpc_Core0_SetBumpyEnable(0U);

    if (stop_car)
    {
        target_speed_set = 0.0f;
        err_degree = 0.0f;
    }

    g_special_action_trigger = 0U;
    s_bumpy_ctx.state = BUMPY_ROAD_STATE_IDLE;
    s_bumpy_ctx.exit_anchor_valid = 0U;
    if (was_active != 0U)
    {
        BumpyRoad_PublishEvent(BUMPY_ROAD_EVENT_ENDED);
    }
}

void BumpyRoad_Init(void)
{
    const uint8_t was_active = (s_bumpy_ctx.state != BUMPY_ROAD_STATE_IDLE) ? 1U : 0U;

    s_bumpy_ctx.state = BUMPY_ROAD_STATE_IDLE;
    s_bumpy_ctx.start_x_mm = 0.0f;
    s_bumpy_ctx.start_y_mm = 0.0f;
    s_bumpy_ctx.traveled_mm = 0.0f;
    s_bumpy_ctx.sample_div_cnt = 0U;
    
    s_bumpy_ctx.exit_reason = BUMPY_ROAD_EXIT_NONE;
    s_bumpy_ctx.exit_anchor_x_mm = 0.0f;
    s_bumpy_ctx.exit_anchor_y_mm = 0.0f;
    s_bumpy_ctx.correction_start_x_mm = 0.0f;
    s_bumpy_ctx.correction_start_y_mm = 0.0f;

    s_bumpy_ctx.exit_anchor_valid = 0U;
    s_bumpy_ctx.entry_confirmed = 0U;
    s_bumpy_ctx.visual_exit_armed = 0U;
    s_bumpy_ctx.correction_applied = 0U;

    s_bumpy_ctx.last_event = BUMPY_ROAD_EVENT_NONE;
    
    s_bumpy_ctx.gyro_z_idx = 0;
    s_bumpy_ctx.gyro_buffer_full = 0;
    s_bumpy_ctx.gyro_sum = 0.0f;
    s_bumpy_ctx.gyro_sum_sq = 0.0f;
    for (int i = 0; i < BUMPY_ROAD_GYRO_WIN_SIZE; i++) {
        s_bumpy_ctx.gyro_z_buffer[i] = 0.0f;
    }

    s_bumpy_ctx.bump_entry_cnt = 0;
    s_bumpy_ctx.on_bump = 0;
    s_bumpy_ctx.current_speed_set = BUMPY_ROAD_INIT_SPEED_SET;
    s_bumpy_ctx.filtered_err_degree = 0.0f;

    if (was_active != 0U)
    {
        BumpyRoad_PublishEvent(BUMPY_ROAD_EVENT_ENDED);
    }
}

void BumpyRoad_SetExitAnchor(float x_mm, float y_mm)
{
    if (s_bumpy_ctx.state == BUMPY_ROAD_STATE_IDLE)
    {
        s_bumpy_ctx.exit_anchor_x_mm = x_mm;
        s_bumpy_ctx.exit_anchor_y_mm = y_mm;
        s_bumpy_ctx.exit_anchor_valid = 1U;
    }
}

void BumpyRoad_Trigger(void)
{
    if (s_bumpy_ctx.state != BUMPY_ROAD_STATE_IDLE)
    {
        return;
    }

    s_bumpy_ctx.start_x_mm = inertial_nav.x;
    s_bumpy_ctx.start_y_mm = inertial_nav.y;
    s_bumpy_ctx.traveled_mm = 0.0f;
    s_bumpy_ctx.sample_div_cnt = 0U;
    
    s_bumpy_ctx.exit_reason = BUMPY_ROAD_EXIT_NONE;
    s_bumpy_ctx.correction_start_x_mm = 0.0f;
    s_bumpy_ctx.correction_start_y_mm = 0.0f;
    s_bumpy_ctx.entry_confirmed = 0U;
    s_bumpy_ctx.visual_exit_armed = 0U;
    s_bumpy_ctx.correction_applied = 0U;
    
    s_bumpy_ctx.gyro_z_idx = 0;
    s_bumpy_ctx.gyro_buffer_full = 0;
    s_bumpy_ctx.gyro_sum = 0.0f;
    s_bumpy_ctx.gyro_sum_sq = 0.0f;
    for (int i = 0; i < BUMPY_ROAD_GYRO_WIN_SIZE; i++) {
        s_bumpy_ctx.gyro_z_buffer[i] = 0.0f;
    }

    s_bumpy_ctx.bump_entry_cnt = 0;
    s_bumpy_ctx.on_bump = 0;
    s_bumpy_ctx.current_speed_set = BUMPY_ROAD_INIT_SPEED_SET;
    s_bumpy_ctx.filtered_err_degree = 0.0f;

    /* 进入任务时独占控制权：开启1核颠簸视觉，并启用0核方向控制器。 */
    g_special_action_trigger = 1U;
    VisionIpc_Core0_SetBumpyEnable(1U);
    VisionBumpyControl_SetEnable(1U);
    VisionBumpyControl_ResetExitDetection();

    s_bumpy_ctx.state = BUMPY_ROAD_STATE_RUNNING;
    BumpyRoad_PublishEvent(BUMPY_ROAD_EVENT_STARTED);
}

void BumpyRoad_Update_1ms(void)
{
    #if REMOTE_CONTROL == 1
    if (s_bumpy_ctx.state == BUMPY_ROAD_STATE_IDLE || robot_ctrl.brake_active == 1U)
    {
        if (robot_ctrl.brake_active == 1U)//遥控器可以停控制器
        {
            BumpyRoad_Init();
            return;
        }
        return;
    }
    #endif
    #if REMOTE_CONTROL == 0
    if (s_bumpy_ctx.state == BUMPY_ROAD_STATE_IDLE)
    {
        return;
    }
    #endif

    if (s_bumpy_ctx.state == BUMPY_ROAD_STATE_RUNNING)
    {
        if (s_bumpy_ctx.correction_applied != 0U)
        {
            err_degree = 0.0f;
        }
        else
        {
            BumpyRoad_ApplyVisionSteer();
        }
        
        // 1. Gyro Z 滑动窗口标准差计算
        float new_val = imu_data.gyro_z;
        float old_val = s_bumpy_ctx.gyro_z_buffer[s_bumpy_ctx.gyro_z_idx];
        
        s_bumpy_ctx.gyro_sum += (new_val - old_val);
        s_bumpy_ctx.gyro_sum_sq += (new_val * new_val - old_val * old_val);
        s_bumpy_ctx.gyro_z_buffer[s_bumpy_ctx.gyro_z_idx] = new_val;
        
        s_bumpy_ctx.gyro_z_idx++;
        if (s_bumpy_ctx.gyro_z_idx >= BUMPY_ROAD_GYRO_WIN_SIZE) {
            s_bumpy_ctx.gyro_z_idx = 0;
            s_bumpy_ctx.gyro_buffer_full = 1;
        }
        
        float stddev = 0.0f;
        if (s_bumpy_ctx.gyro_buffer_full) {
            float mean = s_bumpy_ctx.gyro_sum / BUMPY_ROAD_GYRO_WIN_SIZE;
            float variance = (s_bumpy_ctx.gyro_sum_sq - s_bumpy_ctx.gyro_sum * mean) / BUMPY_ROAD_GYRO_WIN_SIZE;
            if (variance > 0.0f) {
                stddev = sqrtf(variance);
            }
        }
        
        // 2. 状态判断与速度控制
        if (s_bumpy_ctx.on_bump == 0)
        {
            s_bumpy_ctx.current_speed_set = BUMPY_ROAD_INIT_SPEED_SET;
            
            if (s_bumpy_ctx.gyro_buffer_full && stddev > BUMPY_ROAD_ENTRY_STD_TH) {
                s_bumpy_ctx.bump_entry_cnt++;
            } else {
                s_bumpy_ctx.bump_entry_cnt = 0;
            }
            
            if (s_bumpy_ctx.bump_entry_cnt >= BUMPY_ROAD_ENTRY_CONFIRM_FRAMES) {
                s_bumpy_ctx.on_bump = 1;
                exit_beep_request = 1U; // 触发进入路肩斜率加速蜂鸣器提示
            }
        }
        else
        {
            // 斜率加速 (速度是负值，所以是做减法)
            s_bumpy_ctx.current_speed_set -= BUMPY_ROAD_SPEED_INC_STEP;
            if (s_bumpy_ctx.current_speed_set < BUMPY_ROAD_LOCK_SPEED_SET) {
                s_bumpy_ctx.current_speed_set = BUMPY_ROAD_LOCK_SPEED_SET;
            }
        }
        
        target_speed_set = s_bumpy_ctx.current_speed_set;

        // 3. 视觉里程计脱出逻辑 (恢复原本逻辑)
        s_bumpy_ctx.sample_div_cnt++;
        if (s_bumpy_ctx.sample_div_cnt >= BUMPY_ROAD_SAMPLE_DIV_1MS)
        {
            s_bumpy_ctx.sample_div_cnt = 0U;
            s_bumpy_ctx.traveled_mm = BumpyRoad_CalcDistanceMm();

            if ((s_bumpy_ctx.entry_confirmed == 0U) &&
                VisionBumpyControl_IsEntryConfirmed())
            {
                s_bumpy_ctx.entry_confirmed = 1U;
            }

            if ((s_bumpy_ctx.visual_exit_armed == 0U) &&
                (s_bumpy_ctx.entry_confirmed != 0U) &&
                (s_bumpy_ctx.traveled_mm >= BUMPY_ROAD_VISUAL_EXIT_MIN_DISTANCE_MM))
            {
                s_bumpy_ctx.visual_exit_armed = 1U;
                VisionBumpyControl_RearmExitDetection();
            }

            if ((s_bumpy_ctx.visual_exit_armed != 0U) &&
                (s_bumpy_ctx.correction_applied == 0U) &&
                VisionBumpyControl_IsExitConfirmed())
            {
                if (s_bumpy_ctx.exit_anchor_valid != 0U)
                {
                    nav_vision_fusion_x = s_bumpy_ctx.exit_anchor_x_mm;
                    nav_vision_fusion_y = s_bumpy_ctx.exit_anchor_y_mm;
                    exit_beep_request = 1U;
                }
                s_bumpy_ctx.correction_start_x_mm = inertial_nav.x;
                s_bumpy_ctx.correction_start_y_mm = inertial_nav.y;
                s_bumpy_ctx.correction_applied = 1U;
                s_bumpy_ctx.state = BUMPY_ROAD_STATE_RUNNING;
                err_degree = 0.0f;
            }

            if ((s_bumpy_ctx.correction_applied != 0U) &&
                (BumpyRoad_CalcCorrectionDistanceMm() >= BUMPY_ROAD_POST_CORRECTION_DISTANCE_MM))
            {
                exit_beep_request = 1U;
                s_bumpy_ctx.exit_reason = BUMPY_ROAD_EXIT_POST_CORRECTION_COMPLETE;
                s_bumpy_ctx.state = BUMPY_ROAD_STATE_FINISH;
            }
            else if ((s_bumpy_ctx.correction_applied == 0U) &&
                     (s_bumpy_ctx.traveled_mm >= BUMPY_ROAD_TARGET_DISTANCE_MM))
            {
                // 视觉始终未确认出口时自动继续，Plan3 不会把融合坐标重定位到 50。
                s_bumpy_ctx.exit_reason = BUMPY_ROAD_EXIT_AUTO_DISTANCE;
                s_bumpy_ctx.state = BUMPY_ROAD_STATE_FINISH;
            }
        }
    }
    
    if (s_bumpy_ctx.state == BUMPY_ROAD_STATE_FINISH)
    {
        BumpyRoad_Cleanup(0U);
    }
}

uint8_t BumpyRoad_Is_Active(void)
{
    return (s_bumpy_ctx.state != BUMPY_ROAD_STATE_IDLE) ? 1U : 0U;
}

BumpyRoadState_e BumpyRoad_GetState(void)
{
    return s_bumpy_ctx.state;
}

float BumpyRoad_GetDistanceMm(void)
{
    return s_bumpy_ctx.traveled_mm; 
}

BumpyRoadExitReason_e BumpyRoad_GetExitReason(void)
{
    return s_bumpy_ctx.exit_reason;
}

BumpyRoadEvent_e BumpyRoad_GetLastEvent(void)
{
    return s_bumpy_ctx.last_event;
}

uint32_t BumpyRoad_GetEventSequence(void)
{
    return s_bumpy_ctx.event_sequence;
}
