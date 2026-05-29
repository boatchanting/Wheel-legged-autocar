#include "bumpy_road.h"
#include "vision/vision_ipc_core0.h"
#include "vision/vision_bumpy_control.h"
#include "tools/sbus.h"

/* ========================= 参数区（可按实车调参） ========================= */
#define BUMPY_ROAD_LOCK_SPEED_SET        (-300.0f)      // 正常行驶时的锁定速度(转速)，负值表示前进
#define BUMPY_ROAD_TARGET_DISTANCE_MM    (4000.0f)      // 目标行驶距离(mm)，超过此距离自动结束任务
#define BUMPY_ROAD_SAMPLE_DIV_1MS        (10U)          // 距离采样分频系数，每10ms(10个1ms周期)更新一次距离

#define BUMPY_ROAD_STALL_SPEED_ABS_TH    (50.0f)        // 卡顿检测速度阈值(mm/s)，低于此值认为可能卡住
#define BUMPY_ROAD_STALL_PITCH_TH        (2.6f)         // 卡顿检测俯仰角阈值(°)，大于此值且速度低时认为卡住
#define BUMPY_ROAD_STALL_MS              (100U)         // 卡顿持续时间阈值(ms)，持续满足卡顿条件此时间才判定为卡住

#define BUMPY_ROAD_JUMP_MIN_GAP_MS       (1000U)        // 两次跳跃动作最小间隔(ms)，防止频繁跳跃
#define BUMPY_ROAD_NO_JUMP_TICK          (0xFFFFFFFFU)  // 表示从未执行过跳跃的特殊时间戳

#define BUMPY_ROAD_BACK_SPEED_SET        (100.0f)       // 后退脱困速度(mm/s)，正值表示后退
#define BUMPY_ROAD_BACK_DURATION_MS      (800U)         // 后退持续时间(ms)

#define BUMPY_ROAD_APPROACH_SPEED_SET    (-200.0f)      // 接近障碍物时的速度(mm/s)，负值表示前进
#define BUMPY_ROAD_APPROACH_DURATION_MS  (200U)         // 接近持续时间(ms)，之后触发跳跃


typedef struct
{
    BumpyRoadState_e state;
    float start_x_mm;
    float start_y_mm;
    float traveled_mm;
    uint16_t sample_div_cnt;
    uint32_t last_jump_tick_ms;
    uint32_t stall_counter_ms;
    uint32_t backing_start_tick_ms;
    uint32_t approach_start_tick_ms;
} BumpyRoadContext_t;

static BumpyRoadContext_t s_bumpy_ctx =
{
    BUMPY_ROAD_STATE_IDLE,
    0.0f,
    0.0f,
    0.0f,
    0U,
    BUMPY_ROAD_NO_JUMP_TICK,
    0U,
    0U,
    0U
};

/**
 * @brief 基于惯导坐标计算当前位置到起点的平面距离
 *
 * @return 距离（mm）
 */
static float BumpyRoad_CalcDistanceMm(void)
{
    const float dx = inertial_nav.x - s_bumpy_ctx.start_x_mm;
    const float dy = inertial_nav.y - s_bumpy_ctx.start_y_mm;
    return sqrtf(dx * dx + dy * dy);
}

static void BumpyRoad_ApplyVisionSteer(void)
{
    /* 方向由视觉模块统一给出；若视觉暂时无效则输出0，避免随机摆动。 */
    // if (VisionBumpyControl_IsEnabled())
    // {
    //     err_degree = VisionBumpyControl_GetErrDegreeCmd();
    // }
    // else
    // {
        err_degree = 0.0f;
    // }
}

static void BumpyRoad_Cleanup(uint8_t stop_car)
{
    VisionBumpyControl_SetEnable(0U);
    VisionIpc_Core0_SetBumpyEnable(0U);

    if (stop_car)
    {
        target_speed_set = 0.0f;
    }
    err_degree = 0.0f;

    g_special_action_trigger = 0U;
    s_bumpy_ctx.state = BUMPY_ROAD_STATE_IDLE;
}

void BumpyRoad_Init(void)
{
    s_bumpy_ctx.state = BUMPY_ROAD_STATE_IDLE;
    s_bumpy_ctx.start_x_mm = 0.0f;
    s_bumpy_ctx.start_y_mm = 0.0f;
    s_bumpy_ctx.traveled_mm = 0.0f;
    s_bumpy_ctx.sample_div_cnt = 0U;
    s_bumpy_ctx.last_jump_tick_ms = BUMPY_ROAD_NO_JUMP_TICK;
    s_bumpy_ctx.stall_counter_ms = 0U;
    s_bumpy_ctx.backing_start_tick_ms = 0U;
    s_bumpy_ctx.approach_start_tick_ms = 0U;

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
    s_bumpy_ctx.last_jump_tick_ms = BUMPY_ROAD_NO_JUMP_TICK;
    s_bumpy_ctx.stall_counter_ms = 0U;
    s_bumpy_ctx.backing_start_tick_ms = 0U;
    s_bumpy_ctx.approach_start_tick_ms = 0U;

    /* 进入任务时独占控制权：开启1核颠簸视觉，并启用0核方向控制器。 */
    g_special_action_trigger = 1U;
    VisionIpc_Core0_SetBumpyEnable(1U);
    VisionBumpyControl_SetEnable(1U);

    s_bumpy_ctx.state = BUMPY_ROAD_STATE_RUNNING;
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
        target_speed_set = BUMPY_ROAD_LOCK_SPEED_SET;
        BumpyRoad_ApplyVisionSteer();

        if (jump_flag == 0U)
        {
            const float left_speed_abs = fabsf((float)motor_value.receive_left_speed_data);
            const float right_speed_abs = fabsf((float)motor_value.receive_right_speed_data);
            const float pitch_deg = euler_angle.pitch;
            uint8_t jump_gap_ok = 0U;
            uint8_t stall_condition_met = 0U;

            if (s_bumpy_ctx.last_jump_tick_ms == BUMPY_ROAD_NO_JUMP_TICK)
            {
                jump_gap_ok = 1U;
            }
            else if ((uint32_t)(loop_counter - s_bumpy_ctx.last_jump_tick_ms) >= BUMPY_ROAD_JUMP_MIN_GAP_MS)
            {
                jump_gap_ok = 1U;
            }

            if ((left_speed_abs < BUMPY_ROAD_STALL_SPEED_ABS_TH) &&
                (right_speed_abs < BUMPY_ROAD_STALL_SPEED_ABS_TH) &&
                (pitch_deg > BUMPY_ROAD_STALL_PITCH_TH) &&
                (pitch_deg < 60.0f))
            {
                stall_condition_met = 1U;
            }

            if (stall_condition_met)
            {
                s_bumpy_ctx.stall_counter_ms++;
            }
            else
            {
                s_bumpy_ctx.stall_counter_ms = 0U;
            }

            if ((stall_condition_met == 1U) &&
                (s_bumpy_ctx.stall_counter_ms >= BUMPY_ROAD_STALL_MS) &&
                (jump_gap_ok == 1U))
            {
                s_bumpy_ctx.state = BUMPY_ROAD_STATE_BACKING;
                s_bumpy_ctx.backing_start_tick_ms = loop_counter;
                s_bumpy_ctx.stall_counter_ms = 0U;
            }
        }

        s_bumpy_ctx.sample_div_cnt++;
        if (s_bumpy_ctx.sample_div_cnt >= BUMPY_ROAD_SAMPLE_DIV_1MS)
        {
            s_bumpy_ctx.sample_div_cnt = 0U;
            s_bumpy_ctx.traveled_mm = BumpyRoad_CalcDistanceMm();

            if (s_bumpy_ctx.traveled_mm >= BUMPY_ROAD_TARGET_DISTANCE_MM)
            {
                s_bumpy_ctx.state = BUMPY_ROAD_STATE_FINISH;
            }
        }
    }

    if (s_bumpy_ctx.state == BUMPY_ROAD_STATE_BACKING)
    {
        target_speed_set = BUMPY_ROAD_BACK_SPEED_SET;
        BumpyRoad_ApplyVisionSteer();

        if ((loop_counter - s_bumpy_ctx.backing_start_tick_ms) >= BUMPY_ROAD_BACK_DURATION_MS)
        {
            s_bumpy_ctx.state = BUMPY_ROAD_STATE_APPROACHING;
            s_bumpy_ctx.approach_start_tick_ms = loop_counter;
        }
    }

    // 接近状态处理（新增）
    if (s_bumpy_ctx.state == BUMPY_ROAD_STATE_APPROACHING)
    {
        // 设置接近速度（前进加速）
        target_speed_set = BUMPY_ROAD_APPROACH_SPEED_SET;
        BumpyRoad_ApplyVisionSteer();

        // 检查接近时间是否达到200ms
        if ((loop_counter - s_bumpy_ctx.approach_start_tick_ms) >= BUMPY_ROAD_APPROACH_DURATION_MS)
        {
            // 接近完成，触发跳跃
            vision_detected_jump_point = true;
            s_bumpy_ctx.last_jump_tick_ms = loop_counter;
            s_bumpy_ctx.state = BUMPY_ROAD_STATE_RUNNING;  // 返回运行状态
        }
    }

    if (s_bumpy_ctx.state == BUMPY_ROAD_STATE_FINISH)
    {
        BumpyRoad_Cleanup(1U);
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
