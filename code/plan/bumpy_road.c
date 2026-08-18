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

#define BUMPY_ROAD_INIT_SPEED_SET        (-400.0f)      // 接近时的初始速度  //7段颠簸给-400.0f // 4段给-800.0f
#define BUMPY_ROAD_LOCK_SPEED_SET        (-800.0f)      // 颠簸段目标速度
#define BUMPY_ROAD_SPEED_INC_STEP        (1.0f)         // 每1ms速度增量 (斜率加速)

#define BUMPY_ROAD_VISUAL_EXIT_MIN_DISTANCE_MM (1000.0f) // 累计满 1m 后才允许由视觉确认出口
#define BUMPY_ROAD_TARGET_DISTANCE_MM    (4000.0f)      // 目标行驶距离(mm)，超过此距离自动结束任务 // 
#define BUMPY_ROAD_SAMPLE_DIV_1MS        (10U)          // 距离采样分频系数，每10ms更新一次距离

/* 出口修正（2026-08-18 方案 v5 §3.4）：脱出状态机确认时刻，用 1 核稳定滤波 lat_stable 一次性修正惯导；
   方向车往右偏时融合坐标应往右移=+1；方向反了改成 -1 */
#define BUMPY_LATERAL_OVERLAY_SIGN       (1.0f)         // 出口叠加方向：车往右偏时融合坐标应往右移=+1；方向反了改成-1

#define BUMPY_ROAD_GYRO_WIN_SIZE         (200U)         // (改小窗口) 1000Hz下200ms的帧数，减小延迟
#define BUMPY_ROAD_ENTRY_STD_TH          (0.3f)         // (降低阈值) 进入特征标准差阈值 (rad/s)
#define BUMPY_ROAD_ENTRY_CONFIRM_FRAMES  (50U)          // (减少确认时间) 连续满足进入条件的帧数(50ms)

/* 垂直加速度“起飞”锁存阈值（g，向上为正，可调）：g_vert_acc_world_g（原始加速度计
   数据 + EKF 重力向量，已扣除 1g）超过该值（>5g 极短突发冲击，车体腾空）即锁存
   “起飞”标志；锁存期间仅锁住转向控制量（err_degree 归零），视觉数据管线照常更新 */
#define BUMPY_ROAD_VERT_ACC_TAKEOFF_TH_G    (5.0f)

typedef struct
{
    BumpyRoadState_e state;
    float start_x_mm;
    float start_y_mm;
        float locked_yaw_deg;
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
    uint8_t takeoff_latched;    // 垂直加速度“起飞”锁存：超过阈值后置位，锁住转向控制量
    BumpyRoadCorrectionMoment_e correction_moment;  // 中线修正执行时刻：默认起飞时刻(0)，可运行时切换
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

static void BumpyRoad_ApplyYawHold(void)
{
    /* 2026-08-18 重构后：1 核已完成“按角度大小整形+EMA”并输出稳定提案
       （无条纹报 0）；0 核无条件直送 err_degree，不做任何因视觉可信度带来的锁。
       仅起飞锁存/出口修正完成后强制归零（物理事件锁，非视觉可信度）。 */
    err_degree = VisionBumpyControl_GetErrDegreeCmd();
}

/* 一次性中线修正（起飞时刻或视觉脱出时刻调用）：
   钉出口锚点 + 横向叠加 1 核 lat_stable；correction_applied=1 后不再做任何中线修正 */
static void BumpyRoad_ApplyExitCorrection(void)
{
    exit_beep_request = 1U;
    if (s_bumpy_ctx.exit_anchor_valid != 0U)
    {
        nav_vision_fusion_x = s_bumpy_ctx.exit_anchor_x_mm;
        nav_vision_fusion_y = s_bumpy_ctx.exit_anchor_y_mm;

        /* 出口修正叠加（2026-08-18 方案 v5 §3.4）：修正时刻一次性修正惯导。
         * lat 取 1 核稳定滤波值 lat_stable（3帧满窗中值+门控+保持状态机输出，
         * 即 IPC lateral_mm → g_vision_bumpy_control_status.lateral_mm；读取时刻
         * 正处 HOLD 保持/FREEZE 冻结的最近可信值）。
         * 叠加门 = 1 核横向置信度（meas_valid=VISION_VALID_BUMPY_MEAS）：
         *   只要可信度未耗尽，哪怕修正瞬间边线未测出也修正惯导；
         *   可信度耗尽（conf==0）或数据过旧才跳过（安全降级）。
         * lat_stable 值本身与置信度无关（只在有边线时更新），过旧仅控制侧不接。
         * 车身右侧单位向量 r=(sinθ,-cosθ) 由 inertial_nav 车体→世界变换推导；
         * 符号方向实车必测，反了翻转 BUMPY_LATERAL_OVERLAY_SIGN。 */
        if (g_vision_bumpy_control_status.meas_valid != 0U)
        {
            const float lat = (float)g_vision_bumpy_control_status.lateral_mm;
            const float theta = inertial_nav.relative_yaw * (3.14159265358979f / 180.0f);
            nav_vision_fusion_x += BUMPY_LATERAL_OVERLAY_SIGN * lat * sinf(theta);
            nav_vision_fusion_y -= BUMPY_LATERAL_OVERLAY_SIGN * lat * cosf(theta);
        }
    }
    s_bumpy_ctx.correction_start_x_mm = inertial_nav.x;
    s_bumpy_ctx.correction_start_y_mm = inertial_nav.y;
    s_bumpy_ctx.correction_applied = 1U;
    err_degree = 0.0f;
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
    s_bumpy_ctx.takeoff_latched = 0U;   // 任务结束清除“起飞”锁存
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
    s_bumpy_ctx.locked_yaw_deg = 0.0f;
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
    s_bumpy_ctx.takeoff_latched = 0U;   // 初始化清除“起飞”锁存

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
    s_bumpy_ctx.locked_yaw_deg = inertial_nav.relative_yaw;
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
    s_bumpy_ctx.takeoff_latched = 0U;   // 重新触发清除“起飞”锁存

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
        /* ==========================================================
         * 0. 垂直加速度“起飞”锁存监控（1ms）
         *    g_vert_acc_world_g（原始加速度计数据 + EKF 重力向量，单位 g，向上为正，
         *    已扣除 1g）超过阈值（>5g 的极短突发冲击，车体腾空）即置位“起飞”锁存。
         *    锁存期间仅锁住转向控制量（err_degree 强制归零，视觉不再接入转向）；
         *    视觉数据管线（左右偏差/出入口检测）照常更新——出口确认仍依赖视觉。
         *    锁存不自动释放，需要时由外部显式调用 BumpyRoad_ClearTakeoffLatch() 解除。
         *    【中线修正时刻=起飞】时，在起飞瞬间执行一次性中线修正；
         *    修正后 correction_applied=1，视觉脱出路径自然跳过，无需专门处理。
         * ========================================================== */
        if (g_vert_acc_world_g > BUMPY_ROAD_VERT_ACC_TAKEOFF_TH_G)
        {
            if (s_bumpy_ctx.takeoff_latched == 0U)
            {
                s_bumpy_ctx.takeoff_latched = 1U;

                if ((s_bumpy_ctx.correction_moment == BUMPY_ROAD_CORRECTION_MOMENT_TAKEOFF) &&
                    (s_bumpy_ctx.correction_applied == 0U))
                {
                    BumpyRoad_ApplyExitCorrection();   /* 起飞时刻：立即一次性中线修正 */
                }
            }
        }

        if ((s_bumpy_ctx.correction_applied != 0U) ||
            (s_bumpy_ctx.takeoff_latched != 0U))
        {
            /* 出口修正完成后 或 起飞锁存期间：视觉不再接入转向，保持直行 */
            err_degree = 0.0f;
        }
        else
        {
            /* 视觉提案无条件直送 err_degree（2026-08-18 方案 v5 §1） */
            BumpyRoad_ApplyYawHold();
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
                /* 视觉确认出口即提示；遥控触发时也可能没有导航出口锚点。
                   起飞时刻已修正时（correction_applied=1）本分支不会进入。 */
                BumpyRoad_ApplyExitCorrection();
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

void BumpyRoad_ClearTakeoffLatch(void)
{
    /* 外部显式解除“起飞”锁存：恢复视觉转向接入（当前不自动释放） */
    s_bumpy_ctx.takeoff_latched = 0U;
}

uint8_t BumpyRoad_IsTakeoff(void)
{
    return s_bumpy_ctx.takeoff_latched;
}

void BumpyRoad_SetCorrectionMoment(BumpyRoadCorrectionMoment_e moment)
{
    s_bumpy_ctx.correction_moment = moment;
}

BumpyRoadCorrectionMoment_e BumpyRoad_GetCorrectionMoment(void)
{
    return s_bumpy_ctx.correction_moment;
}
