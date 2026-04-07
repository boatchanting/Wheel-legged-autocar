#include "bumpy_road.h"

// ========================= 参数区（可按实车调参） =========================
#define BUMPY_ROAD_LOCK_SPEED_SET      (-150.0f)   // 状态机执行期间强制锁定目标速度
#define BUMPY_ROAD_TARGET_DISTANCE_MM  (3000.0f)   // 目标直行距离（mm）
#define BUMPY_ROAD_SAMPLE_DIV_1MS      (10U)       // 1ms任务分频：每10ms计算一次里程

// ========================= 内部运行时上下文 =========================
typedef struct
{
    BumpyRoadState_e state;      // 当前状态
    float start_x_mm;            // 触发时惯导起点 x（mm）
    float start_y_mm;            // 触发时惯导起点 y（mm）
    float traveled_mm;           // 已累计行驶距离（mm）
    uint16_t sample_div_cnt;     // 1ms 分频计数器
} BumpyRoadContext_t;

static BumpyRoadContext_t s_bumpy_ctx =
{
    BUMPY_ROAD_STATE_IDLE,
    0.0f,
    0.0f,
    0.0f,
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

void BumpyRoad_Init(void)
{
    s_bumpy_ctx.state = BUMPY_ROAD_STATE_IDLE;
    s_bumpy_ctx.start_x_mm = 0.0f;
    s_bumpy_ctx.start_y_mm = 0.0f;
    s_bumpy_ctx.traveled_mm = 0.0f;
    s_bumpy_ctx.sample_div_cnt = 0U;
}

void BumpyRoad_Trigger(void)
{
    // 仅允许从空闲态触发，避免重复触发打断流程
    if (s_bumpy_ctx.state != BUMPY_ROAD_STATE_IDLE)
    {
        return;
    }

    // 记录触发点作为“1000mm直行”的起点
    s_bumpy_ctx.start_x_mm = inertial_nav.x;
    s_bumpy_ctx.start_y_mm = inertial_nav.y;
    s_bumpy_ctx.traveled_mm = 0.0f;
    s_bumpy_ctx.sample_div_cnt = 0U;
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
        // 1) 强制锁速：target_speed_set = -150.0f
        // 2) 直行约束：err_degree = 0，抑制外部转向指令干扰
        target_speed_set = BUMPY_ROAD_LOCK_SPEED_SET;
        err_degree = 0.0f;

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

float BumpyRoad_GetDistanceMm(void)
{
    return s_bumpy_ctx.traveled_mm;
}
