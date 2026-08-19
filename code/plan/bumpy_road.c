#include "bumpy_road.h"
#include "vision/vision_ipc_core0.h"
#include "vision/vision_bumpy_control.h"
#include "navigation/inertial_nav.h"
#include "calculate/ekf.h"
#include "tools/sbus.h"
#include <math.h>

extern volatile uint8 exit_beep_request;

/* ========================= 参数区 ========================= */
#define BUMPY_ROAD_INIT_SPEED_SET        (-400.0f)      // 接近时的初始速度  //7段颠簸给-400.0f // 4段给-800.0f
#define BUMPY_ROAD_LOCK_SPEED_SET        (-800.0f)      // 颠簸段目标速度
#define BUMPY_ROAD_SPEED_INC_STEP        (1.0f)         // 每1ms速度增量 (斜率加速)

#define BUMPY_ROAD_VISUAL_EXIT_MIN_DISTANCE_MM (1000.0f) // 累计满 1m 后才允许由视觉确认出口
#define BUMPY_ROAD_POST_EXIT_DISTANCE_MM     (1500.0f)      // 视觉确认出口后再行驶的缓冲距离(mm)，随后结束任务
#define BUMPY_ROAD_TARGET_DISTANCE_MM    (4000.0f)      // 目标行驶距离(mm)，视觉始终未确认出口时超过此距离自动结束任务 // 
#define BUMPY_ROAD_SAMPLE_DIV_1MS        (10U)          // 距离采样分频系数，每10ms更新一次距离

/* 中线对正（2026-08-19 恢复供移植，惰性禁用）：出口修正参数原样保留；
   对正变量 lateral_mm 恒 0 → BumpyRoad_ApplyExitCorrection 整体不执行，不干扰任何内容。 */
#define BUMPY_LATERAL_OVERLAY_SIGN       (1.0f)         // 出口叠加方向：车往右偏时融合坐标应往右移=+1；方向反了改成-1
#define BUMPY_ROAD_POST_CORRECTION_DISTANCE_MM (1500.0f) // 中线修正后行驶距离(mm)，修正完成后行驶该距离结束任务

#define BUMPY_ROAD_GYRO_WIN_SIZE         (200U)         // (改小窗口) 1000Hz下200ms的帧数，减小延迟
#define BUMPY_ROAD_ENTRY_STD_TH          (0.3f)         // (降低阈值) 进入特征标准差阈值 (rad/s)
#define BUMPY_ROAD_ENTRY_CONFIRM_FRAMES  (50U)          // (减少确认时间) 连续满足进入条件的帧数(50ms)

/* 垂直冲击滞回比较器（2026-08-19，参考 d27e0b1 滞回鸣叫逻辑）：
   g_vert_acc_world_g（世界系竖直动态加速度，g，向上为正，已扣除 1g）
     > BUMPY_ROAD_VERT_ACC_TAKEOFF_TH_G(5g) → impact_active=1（冲击中）；
     < BUMPY_ROAD_VERT_ACC_RELEASE_TH_G(2g) → impact_active=0（无冲击）；
     2g~5g 之间保持上一状态（施密特触发器，避免阈值附近抖动/信号粘连）。
   上升沿（0→1）即一次高冲击：第 1 次=起飞，第 2 次=落地；
   转向锁：impact_count>=1（第 1 次冲击/起飞之后）起全程锁死（物理事件锁）。 */
#define BUMPY_ROAD_VERT_ACC_TAKEOFF_TH_G    (5.0f)
#define BUMPY_ROAD_VERT_ACC_RELEASE_TH_G    (2.0f)
typedef struct
{
    BumpyRoadState_e state;
    float start_x_mm;
    float start_y_mm;
        float locked_yaw_deg;
    float traveled_mm;
    
    uint16_t sample_div_cnt;

    BumpyRoadExitReason_e exit_reason;
    /* —— 中线对正（2026-08-19 恢复供移植，惰性禁用：对正变量 lateral_mm 恒 0）——
       出口锚点/修正计距/修正标志/修正时刻：结构原样保留，修正函数整体不执行 */
    float exit_anchor_x_mm;
    float exit_anchor_y_mm;
    float correction_start_x_mm;
    float correction_start_y_mm;
    uint8_t exit_anchor_valid;
    uint8_t correction_applied;
    BumpyRoadCorrectionMoment_e correction_moment;
    uint8_t entry_confirmed;
    uint8_t visual_exit_armed;
    uint8_t exit_beep_sent;                /* 视觉确认出口后只响一次蜂鸣 */
    float exit_confirmed_traveled_mm;      /* 视觉确认出口时刻的已行驶距离(mm)：缓冲距离计距起点 */

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
    /* 垂直冲击滞回比较器（2026-08-19）：
       impact_active = 滞回输出（>5g=1，<2g=0，2~5g 保持），仅用于上升沿检测；
       impact_prev   = 上一帧输出（上升沿检测）；
       impact_count  = 冲击上升沿计数（1=起飞，2=落地，3+ 后续颠簸不再提示）；
       转向物理锁 = impact_count>=1（起飞后全程锁死）；
       takeoff_detected / landing_detected = 起飞/落地已检测标志 */
    uint8_t impact_active;
    uint8_t impact_prev;
    uint8_t impact_count;
    uint8_t takeoff_detected;
    uint8_t landing_detected;
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

/* 中线对正（恢复供移植，惰性禁用）：修正后距离计距（correction_applied 恒 0 → 不触发） */
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
       2026-08-19：中线对正逻辑已恢复但惰性禁用（对正变量 lateral_mm 恒 0），
       角度路径是唯一实际生效的控制量，直接送 err_degree。
       起飞锁（impact_count>=1）期间由调用方强制归零。 */
    err_degree = VisionBumpyControl_GetErrDegreeCmd();
}

/* 中线对正（2026-08-19 恢复供移植，惰性禁用）：一次性出口修正。
   总闸 = 对正变量 lateral_mm != 0：0 核从不写入该变量（恒 0），
   故本函数整体（含蜂鸣/钉出口锚点/横向叠加/修正收尾）绝不执行，绝不干扰任何内容；
   后续移植：在 vision_bumpy_control.c 恢复写入 lateral_mm 后本函数即完整生效。 */
static void BumpyRoad_ApplyExitCorrection(void)
{
    if (g_vision_bumpy_control_status.lateral_mm != 0)
    {
        exit_beep_request = 1U;
        if (s_bumpy_ctx.exit_anchor_valid != 0U)
        {
            nav_vision_fusion_x = s_bumpy_ctx.exit_anchor_x_mm;
            nav_vision_fusion_y = s_bumpy_ctx.exit_anchor_y_mm;
            /* 出口修正叠加（2026-08-18 方案 v5 §3.4）：lat 取 1 核稳定滤波值
               （IPC lateral_mm → g_vision_bumpy_control_status.lateral_mm），
               叠加门=meas_valid；车身右侧单位向量 r=(sinθ,-cosθ)，符号方向实车必测。 */
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
    /* 中线对正字段复位（恢复供移植，惰性禁用） */
    s_bumpy_ctx.exit_anchor_x_mm = 0.0f;
    s_bumpy_ctx.exit_anchor_y_mm = 0.0f;
    s_bumpy_ctx.correction_start_x_mm = 0.0f;
    s_bumpy_ctx.correction_start_y_mm = 0.0f;
    s_bumpy_ctx.exit_anchor_valid = 0U;
    s_bumpy_ctx.correction_applied = 0U;
    s_bumpy_ctx.correction_moment = BUMPY_ROAD_CORRECTION_MOMENT_TAKEOFF;
    s_bumpy_ctx.impact_active = 0U;      // 任务结束清除滞回/起飞/落地检测
    s_bumpy_ctx.impact_prev = 0U;
    s_bumpy_ctx.impact_count = 0U;
    s_bumpy_ctx.takeoff_detected = 0U;
    s_bumpy_ctx.landing_detected = 0U;
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
    /* 中线对正字段复位（恢复供移植，惰性禁用） */
    s_bumpy_ctx.exit_anchor_x_mm = 0.0f;
    s_bumpy_ctx.exit_anchor_y_mm = 0.0f;
    s_bumpy_ctx.correction_start_x_mm = 0.0f;
    s_bumpy_ctx.correction_start_y_mm = 0.0f;
    s_bumpy_ctx.exit_anchor_valid = 0U;
    s_bumpy_ctx.correction_applied = 0U;
    s_bumpy_ctx.correction_moment = BUMPY_ROAD_CORRECTION_MOMENT_TAKEOFF;
    s_bumpy_ctx.entry_confirmed = 0U;
    s_bumpy_ctx.visual_exit_armed = 0U;
    s_bumpy_ctx.exit_beep_sent = 0U;
    s_bumpy_ctx.exit_confirmed_traveled_mm = 0.0f;

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
    s_bumpy_ctx.impact_active = 0U;      // 初始化清除滞回/起飞/落地检测
    s_bumpy_ctx.impact_prev = 0U;
    s_bumpy_ctx.impact_count = 0U;
    s_bumpy_ctx.takeoff_detected = 0U;
    s_bumpy_ctx.landing_detected = 0U;

    if (was_active != 0U)
    {
        BumpyRoad_PublishEvent(BUMPY_ROAD_EVENT_ENDED);
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
    /* 中线对正字段复位（恢复供移植，惰性禁用） */
    s_bumpy_ctx.exit_anchor_x_mm = 0.0f;
    s_bumpy_ctx.exit_anchor_y_mm = 0.0f;
    s_bumpy_ctx.correction_start_x_mm = 0.0f;
    s_bumpy_ctx.correction_start_y_mm = 0.0f;
    s_bumpy_ctx.exit_anchor_valid = 0U;
    s_bumpy_ctx.correction_applied = 0U;
    s_bumpy_ctx.correction_moment = BUMPY_ROAD_CORRECTION_MOMENT_TAKEOFF;
    s_bumpy_ctx.entry_confirmed = 0U;
    s_bumpy_ctx.visual_exit_armed = 0U;
    s_bumpy_ctx.exit_beep_sent = 0U;
    s_bumpy_ctx.exit_confirmed_traveled_mm = 0.0f;
    
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
    s_bumpy_ctx.impact_active = 0U;      // 重新触发清除滞回/起飞/落地检测
    s_bumpy_ctx.impact_prev = 0U;
    s_bumpy_ctx.impact_count = 0U;
    s_bumpy_ctx.takeoff_detected = 0U;
    s_bumpy_ctx.landing_detected = 0U;

    /* 进入任务时独占控制权：开启1核颠簸视觉，并启用0核方向控制器。 */
    g_special_action_trigger = 1U;
    VisionIpc_Core0_SetBumpyEnable(1U);
    VisionBumpyControl_SetEnable(1U);
    VisionBumpyControl_ResetExitDetection();

    s_bumpy_ctx.state = BUMPY_ROAD_STATE_RUNNING;
    BumpyRoad_PublishEvent(BUMPY_ROAD_EVENT_STARTED);
}

void BumpyRoad_SetExitAnchor(float x_mm, float y_mm)
{
    /* 中线对正（恢复供移植，惰性禁用）：仅记录出口锚点；修正函数整体不执行
       （lateral_mm 恒 0），故设置锚点不影响任何内容。 */
    if (s_bumpy_ctx.state == BUMPY_ROAD_STATE_IDLE)
    {
        s_bumpy_ctx.exit_anchor_x_mm = x_mm;
        s_bumpy_ctx.exit_anchor_y_mm = y_mm;
        s_bumpy_ctx.exit_anchor_valid = 1U;
    }
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
         * 0. 垂直冲击滞回比较器 + 起飞/落地检测（1ms）
         *    g_vert_acc_world_g（世界系竖直动态加速度，g，向上为正，已扣除 1g）
         *    做滞回比较（施密特触发器）：
         *      > 5g（BUMPY_ROAD_VERT_ACC_TAKEOFF_TH_G）→ impact_active=1（冲击中）
         *      < 2g（BUMPY_ROAD_VERT_ACC_RELEASE_TH_G）→ impact_active=0（无冲击）
         *      2g~5g 保持上一状态（避免阈值附近抖动/信号粘连）。
         *    上升沿（0→1）即一次高冲击：第 1 次=起飞，第 2 次=落地，各自响一次
         *    提示蜂鸣；后续冲击不再提示（用户指示：落地后的叫声不在乎）。
         *    转向锁：impact_count>=1（第 1 次冲击/起飞之后）起全程锁死
         *    （err_degree=0，物理事件锁），视觉不再接入转向；
         *    视觉数据管线（左右偏差/出入口检测）照常更新。
         * ========================================================== */
        if (g_vert_acc_world_g > BUMPY_ROAD_VERT_ACC_TAKEOFF_TH_G)
        {
            s_bumpy_ctx.impact_active = 1U;
        }
        else if (g_vert_acc_world_g < BUMPY_ROAD_VERT_ACC_RELEASE_TH_G)
        {
            s_bumpy_ctx.impact_active = 0U;
        }
        /* 2g~5g：保持上一状态（滞回） */

        if ((s_bumpy_ctx.impact_active != 0U) && (s_bumpy_ctx.impact_prev == 0U))
        {
            /* 上升沿：一次高冲击 */
            s_bumpy_ctx.impact_count++;
            if (s_bumpy_ctx.impact_count == 1U)
            {
                s_bumpy_ctx.takeoff_detected = 1U;   /* 第 1 次=起飞 */
                exit_beep_request = 1U;
                /* 中线对正（恢复供移植，惰性禁用）：起飞时刻一次性修正
                   （lateral_mm 恒 0 → ApplyExitCorrection 无任何效果） */
                if ((s_bumpy_ctx.correction_moment == BUMPY_ROAD_CORRECTION_MOMENT_TAKEOFF) &&
                    (s_bumpy_ctx.correction_applied == 0U))
                {
                    BumpyRoad_ApplyExitCorrection();
                }
            }
            else if (s_bumpy_ctx.impact_count == 2U)
            {
                s_bumpy_ctx.landing_detected = 1U;   /* 第 2 次=落地 */
                exit_beep_request = 1U;
            }
            /* 第 3 次及以上：后续颠簸冲击，不再提示 */
        }
        s_bumpy_ctx.impact_prev = s_bumpy_ctx.impact_active;

        if (s_bumpy_ctx.impact_count >= 1U)
        {
            /* 已检测到第 1 次冲击（起飞）后：全程锁转向，视觉不再接入（物理事件锁） */
            err_degree = 0.0f;
        }
        else
        {
            /* 视觉角度提案无条件直送 err_degree（2026-08-18 方案 v5 §1；
               2026-08-19 拒绝中线对正后，角度是唯一控制量，直通不变） */
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
                (s_bumpy_ctx.exit_beep_sent == 0U) &&
                VisionBumpyControl_IsExitConfirmed())
            {
                /* 视觉确认出口（1 核角度方差门控：bumpy_detected/hdg_valid=0 连续 3 帧）：
                   记录出口确认时刻 + 只响一次提示蜂鸣。 */
                s_bumpy_ctx.exit_beep_sent = 1U;
                s_bumpy_ctx.exit_confirmed_traveled_mm = s_bumpy_ctx.traveled_mm;
                exit_beep_request = 1U;
                /* 中线对正（恢复供移植，惰性禁用）：视觉脱出时刻一次性修正
                   （lateral_mm 恒 0 → ApplyExitCorrection 无任何效果） */
                if (s_bumpy_ctx.correction_applied == 0U)
                {
                    BumpyRoad_ApplyExitCorrection();
                }
            }

            /* 中线修正后收尾（恢复供移植，惰性禁用）：correction_applied 恒 0
               （lateral_mm 恒 0），此路径永不触发 */
            if ((s_bumpy_ctx.correction_applied != 0U) &&
                (BumpyRoad_CalcCorrectionDistanceMm() >= BUMPY_ROAD_POST_CORRECTION_DISTANCE_MM))
            {
                exit_beep_request = 1U;
                s_bumpy_ctx.exit_reason = BUMPY_ROAD_EXIT_POST_CORRECTION_COMPLETE;
                s_bumpy_ctx.state = BUMPY_ROAD_STATE_FINISH;
            }
            else if (s_bumpy_ctx.exit_beep_sent != 0U)
            {
                /* 视觉确认出口后：再行驶缓冲距离，随后结束任务交还导航（当前生效路径） */
                if ((s_bumpy_ctx.traveled_mm - s_bumpy_ctx.exit_confirmed_traveled_mm) >=
                    BUMPY_ROAD_POST_EXIT_DISTANCE_MM)
                {
                    s_bumpy_ctx.exit_reason = BUMPY_ROAD_EXIT_VISUAL_CONFIRMED;
                    s_bumpy_ctx.state = BUMPY_ROAD_STATE_FINISH;
                }
            }
            else if (s_bumpy_ctx.traveled_mm >= BUMPY_ROAD_TARGET_DISTANCE_MM)
            {
                /* 视觉始终未确认出口：满目标距离自动结束（兜底） */
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
    /* 外部显式清除起飞/落地检测：复位滞回比较器与冲击计数（检测重新武装） */
    s_bumpy_ctx.impact_active = 0U;
    s_bumpy_ctx.impact_prev = 0U;
    s_bumpy_ctx.impact_count = 0U;
    s_bumpy_ctx.takeoff_detected = 0U;
    s_bumpy_ctx.landing_detected = 0U;
}

uint8_t BumpyRoad_IsTakeoff(void)
{
    return s_bumpy_ctx.takeoff_detected;
}

uint8_t BumpyRoad_IsLanding(void)
{
    return s_bumpy_ctx.landing_detected;
}

void BumpyRoad_SetCorrectionMoment(BumpyRoadCorrectionMoment_e moment)
{
    /* 中线对正（恢复供移植，惰性禁用）：修正时刻设置，当前不影响任何内容 */
    s_bumpy_ctx.correction_moment = moment;
}

BumpyRoadCorrectionMoment_e BumpyRoad_GetCorrectionMoment(void)
{
    return s_bumpy_ctx.correction_moment;
}
