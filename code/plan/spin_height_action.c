#include "spin_height_action.h"
#include "minefield.h"
#include "bridge.h"
#include "../servo/servo.h"
#include "../tools/sbus.h"

// ============================================================
// 组合动作：原地自转两圈 + 伸腿收腿伸腿收腿
//
// 4 个高度节点均匀分布在 720° 自转中：
//    0°        180°        360°        540°        720°
//    |--- 伸腿 ---|--- 收腿 ---|--- 伸腿 ---|--- 收腿 ---|
//    ↑ 开始自转                                    ↑ 自转完成
//
// 触发前先清零速度（参考 CH5 刹车逻辑）
// ============================================================

// 外部全局变量
extern uint8 g_special_action_trigger;

// 状态枚举
typedef enum {
    SHA_STATE_IDLE = 0,
    SHA_STATE_SPINNING,        // 自转进行中，根据角度控制升降
    SHA_STATE_DONE             // 完成，过渡到 IDLE
} SpinHeightState_e;

static SpinHeightState_e s_state = SHA_STATE_IDLE;
static uint8_t s_phase = 0;  // 当前阶段 0-3

// 高度参数：复用 bridge
#define SHA_HEIGHT_NORMAL   3.0f
#define SHA_HEIGHT_HIGH     6.0f
#define SHA_HEIGHT_TOL      0.15f

// 伸腿/收腿步长（每个 20ms 周期的增量）
#define SHA_STEP_RISE   0.5f
#define SHA_STEP_DROP   0.8f

// 自转参数
#define SHA_SPIN_ANGLE  721.0f
#define SHA_SPIN_SIGN   1.0f    // CCW

// 4 个阶段的角度分界点（均匀分布 720°）
#define SHA_PHASE_1_BOUNDARY  180.0f
#define SHA_PHASE_2_BOUNDARY  360.0f
#define SHA_PHASE_3_BOUNDARY  540.0f

#ifndef MY_ABS_F
#define MY_ABS_F(x) (((x) < 0.0f) ? (-(x)) : (x))
#endif

void SpinHeightAction_Init(void)
{
    s_state = SHA_STATE_IDLE;
    s_phase = 0;
}

void SpinHeightAction_Trigger(void)
{
    if (s_state != SHA_STATE_IDLE) {
        return;
    }

    // ---- 速度清零（参考 CH5 刹车逻辑） ----
    robot_ctrl.target_speed = 0.0f;
    robot_ctrl.brake_active = 1U;
    g_brake_active = 1U;

    // ---- 配置并启动自转 ----
    Minefield_SetSpinPlan(SHA_SPIN_ANGLE, 0.0f, SHA_SPIN_SIGN);
    minefield_flag = 1U;
    g_special_action_trigger = 1U;

    // ---- 进入状态机，Phase 0：伸腿 ----
    s_phase = 0;
    s_state = SHA_STATE_SPINNING;
}

uint8_t SpinHeightAction_IsActive(void)
{
    return (s_state != SHA_STATE_IDLE) ? 1U : 0U;
}

void SpinHeightAction_Update(void)
{
    float angle;

    switch (s_state) {
    // ----------------------------------------------------------
    case SHA_STATE_SPINNING:
        // 释放刹车（仅第一个周期执行一次）
        robot_ctrl.brake_active = 0U;
        g_brake_active = 0U;

        angle = Minefield_GetAccumulatedAngle();

        // 根据累计角度判断当前阶段
        if (angle >= SHA_PHASE_3_BOUNDARY) {
            s_phase = 3;
        } else if (angle >= SHA_PHASE_2_BOUNDARY) {
            s_phase = 2;
        } else if (angle >= SHA_PHASE_1_BOUNDARY) {
            s_phase = 1;
        }
        // else: s_phase stays 0

        // 根据阶段执行升降
        // Phase 0 (0°-180°):   伸腿
        // Phase 1 (180°-360°): 收腿
        // Phase 2 (360°-540°): 伸腿
        // Phase 3 (540°-720°): 收腿
        if (s_phase == 0 || s_phase == 2) {
            Bridge_Apply_Height_Control(SHA_HEIGHT_HIGH, SHA_STEP_RISE);
        } else {
            Bridge_Apply_Height_Control(SHA_HEIGHT_NORMAL, SHA_STEP_DROP);
        }

        // 自转完成检测
        if (Minefield_Is_Active() == 0U) {
            // 确保最终高度回到正常值
            servo_height = SHA_HEIGHT_NORMAL;
            g_special_action_trigger = 0U;
            s_state = SHA_STATE_DONE;
        }
        break;

    // ----------------------------------------------------------
    case SHA_STATE_DONE:
        s_state = SHA_STATE_IDLE;
        s_phase = 0;
        break;

    // ----------------------------------------------------------
    default:
        s_state = SHA_STATE_IDLE;
        s_phase = 0;
        break;
    }
}
