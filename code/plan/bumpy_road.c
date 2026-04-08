#include "bumpy_road.h"

// ========================= 参数区（可按实车调参） =========================
#define BUMPY_ROAD_LOCK_SPEED_SET      (-100.0f)     // 状态机执行期间强制锁定目标速度
#define BUMPY_ROAD_TARGET_DISTANCE_MM  (3000.0f)   // 目标直行距离（mm）
#define BUMPY_ROAD_SAMPLE_DIV_1MS      (10U)       // 1ms任务分频：每10ms计算一次里程

// 堵转触发跳跃判据：
// 1) 左右电机速度绝对值都小于该阈值
// 2) 俯仰角 pitch 小于该阈值（车头下扎）
// 3) 仅在非跳跃状态下触发（jump_flag == 0）
#define BUMPY_ROAD_STALL_SPEED_ABS_TH  (50.0f)
#define BUMPY_ROAD_STALL_PITCH_TH      (3.0f)

// 连续跳跃保护：
// 使用 loop_counter（1ms计数）限制两次自动跳跃触发间隔，防止疯狂连跳
#define BUMPY_ROAD_JUMP_MIN_GAP_MS     (1000U)
#define BUMPY_ROAD_NO_JUMP_TICK        (0xFFFFFFFFU)

// ========================= 内部运行时上下文 =========================
typedef struct
{
    BumpyRoadState_e state;      // 当前状态
    float start_x_mm;            // 触发时惯导起点 x（mm）
    float start_y_mm;            // 触发时惯导起点 y（mm）
    float traveled_mm;           // 已累计行驶距离（mm）
    uint16_t sample_div_cnt;     // 1ms 分频计数器
    uint32_t last_jump_tick_ms;  // 最近一次自动触发跳跃的时间戳（loop_counter）
} BumpyRoadContext_t;

static BumpyRoadContext_t s_bumpy_ctx =
{
    BUMPY_ROAD_STATE_IDLE,
    0.0f,
    0.0f,
    0.0f,
    0U,
    BUMPY_ROAD_NO_JUMP_TICK
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

void BumpyRoad_Init(void)
{
    s_bumpy_ctx.state = BUMPY_ROAD_STATE_IDLE;
    s_bumpy_ctx.start_x_mm = 0.0f;
    s_bumpy_ctx.start_y_mm = 0.0f;
    s_bumpy_ctx.traveled_mm = 0.0f;
    s_bumpy_ctx.sample_div_cnt = 0U;
    s_bumpy_ctx.last_jump_tick_ms = BUMPY_ROAD_NO_JUMP_TICK;
}

void BumpyRoad_Trigger(void)
{
    // 仅允许从空闲态触发，避免重复触发打断流程
    if (s_bumpy_ctx.state != BUMPY_ROAD_STATE_IDLE)
    {
        return;
    }

    // 记录触发点作为“目标里程直行”的起点
    s_bumpy_ctx.start_x_mm = inertial_nav.x;
    s_bumpy_ctx.start_y_mm = inertial_nav.y;
    s_bumpy_ctx.traveled_mm = 0.0f;
    s_bumpy_ctx.sample_div_cnt = 0U;
    s_bumpy_ctx.last_jump_tick_ms = BUMPY_ROAD_NO_JUMP_TICK;
    s_bumpy_ctx.state = BUMPY_ROAD_STATE_RUNNING;
}

void BumpyRoad_Update_1ms(void)
{
    if (s_bumpy_ctx.state == BUMPY_ROAD_STATE_IDLE)
    {
        return;
    }

    if (s_bumpy_ctx.state == BUMPY_ROAD_STATE_RUNNING)
    {
        // ------------------------- 执行态控制目标 -------------------------
        // 1) 强制锁速：target_speed_set = -75.0f
        // 2) 直行约束：err_degree = 0，抑制外部转向指令干扰
        target_speed_set = BUMPY_ROAD_LOCK_SPEED_SET;
        err_degree = 0.0f;

        // ------------------------- 堵转判据与跳跃触发 -------------------------
        // 条件说明：
        // A. 仅在非跳跃状态下检测（避免跳跃过程中重复触发）
        // B. 左右轮速度都很小（接近堵转/顶死）
        // C. 俯仰角明显下扎（pitch < -7°）
        // D. 满足最小触发间隔（loop_counter 计时）
        // 满足后置位 vision_detected_jump_point，由主循环中的跳跃逻辑消费并清零。
        if (jump_flag == 0U)
        {
            const float left_speed_abs = fabsf((float)motor_value.receive_left_speed_data);
            const float right_speed_abs = fabsf((float)motor_value.receive_right_speed_data);
            const float pitch_deg = euler_angle.pitch;
            uint8_t jump_gap_ok = 0U;

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
                (jump_gap_ok == 1U))
            {
                vision_detected_jump_point = true;
                s_bumpy_ctx.last_jump_tick_ms = loop_counter;
            }
        }

        // 每10ms更新一次距离，降低计算开销并与惯导更新节拍更匹配
        s_bumpy_ctx.sample_div_cnt++;
        if (s_bumpy_ctx.sample_div_cnt >= BUMPY_ROAD_SAMPLE_DIV_1MS)
        {
            s_bumpy_ctx.sample_div_cnt = 0U;
            s_bumpy_ctx.traveled_mm = BumpyRoad_CalcDistanceMm();

            // 达到目标距离：进入收尾态
            if (s_bumpy_ctx.traveled_mm >= BUMPY_ROAD_TARGET_DISTANCE_MM)
            {
                s_bumpy_ctx.state = BUMPY_ROAD_STATE_FINISH;
            }
        }
    }

    if (s_bumpy_ctx.state == BUMPY_ROAD_STATE_FINISH)
    {
        // 收尾：停车一次，然后退出状态机，控制权交还上层逻辑
        target_speed_set = 0.0f;
        err_degree = 0.0f;
        s_bumpy_ctx.state = BUMPY_ROAD_STATE_IDLE;
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

/**
 * @brief 获取颠簸道路已行驶距离(毫米)
 * @return 返回已行驶的距离，单位为毫米
 */
float BumpyRoad_GetDistanceMm(void)
{
    return s_bumpy_ctx.traveled_mm;  // 返回颠簸道路上下文中存储的已行驶距离
}
