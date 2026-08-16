/*
 * =================================================================================
 * 文件: vision_entry_lqr.c
 * 作用: 0 核 (Core 0) 单边桥/坡道「进入段」LQR 方向控制器实现。
 * 说明: 纯方向算法，无副作用、无 target_speed_set 引用。
 * 依据: docs/任务规划/单边桥与坡道进入段LQR方向控制接入规划.md（v5）§3.1/§3.2
 * =================================================================================
 */
#include "vision/vision_entry_lqr.h"

#include <math.h>
#include <string.h>

/* --- 内部状态（仅本文件可用） --- */
static vision_entry_lqr_state_t s_lqr;

/* --- 基础数学工具 --- */

static float vision_entry_lqr_normalize_angle(float angle_deg)
{
    while (angle_deg > 180.0f)
    {
        angle_deg -= 360.0f;
    }
    while (angle_deg < -180.0f)
    {
        angle_deg += 360.0f;
    }
    return angle_deg;
}

static float vision_entry_lqr_constrain_f(float value, float min_value, float max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

/* --- 对外接口 --- */

void VisionEntryLqr_Reset(float entry_yaw_deg)
{
    memset(&s_lqr, 0, sizeof(s_lqr));
    s_lqr.entry_yaw_deg = entry_yaw_deg;
}

uint8 VisionEntryLqr_UpdateVision(int16 phy_x_mm, int16 phy_y_mm, float yaw_deg, float v_mps)
{
    const float deg2rad = 0.0174532925f;
    float fx;
    float fy;
    float beta_rad;
    float dist_m;
    float yaw_rad;
    float psi_err_deg;
    float psi_err_rad;
    float e_m;
    float k1;
    float k2;
    float omega;

    /* 物理坐标无效 → 本拍无视觉，调用方回退 */
    if ((phy_x_mm == LQR_PHY_INVALID_MM) || (phy_y_mm == LQR_PHY_INVALID_MM))
    {
        s_lqr.valid = 0U;
        return 0U;
    }

    fx = (float)phy_x_mm;
    fy = (float)phy_y_mm;
    dist_m = sqrtf(fx * fx + fy * fy) / 1000.0f;

    /* 超出检测距离 → 盲区段，调用方回退锁角 */
    if (dist_m > LQR_DETECT_RANGE_M)
    {
        s_lqr.valid = 0U;
        return 0U;
    }

    beta_rad = atan2f(fx, fy);
    yaw_rad = yaw_deg * deg2rad;
    psi_err_deg = vision_entry_lqr_normalize_angle(s_lqr.entry_yaw_deg - yaw_deg);
    psi_err_rad = psi_err_deg * deg2rad;

    /* 横向偏差重建（与仿真 e = D·sin(β+ψ) 同式） */
    e_m = dist_m * sinf(beta_rad + yaw_rad);

    /* CARE 闭式增益：k1=√Qy，k2=√(2·max(v,0.3)·k1+Qψ) */
    k1 = sqrtf(LQR_QY);
    k2 = sqrtf(2.0f * fmaxf(v_mps, LQR_V_FLOOR_MPS) * k1 + LQR_QPSI);

    omega = k1 * e_m + k2 * psi_err_rad;
    omega = vision_entry_lqr_constrain_f(omega, -LQR_W_MAX_RADPS, LQR_W_MAX_RADPS);

    s_lqr.beta_rad = beta_rad;
    s_lqr.dist_m = dist_m;
    s_lqr.e_m = e_m;
    s_lqr.psi_err_rad = psi_err_rad;
    s_lqr.omega_radps = omega;
    s_lqr.valid = 1U;
    return 1U;
}

float VisionEntryLqr_GetErrDegree(void)
{
    float err;

    if (s_lqr.valid == 0U)
    {
        return 0.0f;
    }

    /* err_degree = ω·(180/π) / TURN_ANG_KP，TURN_ANG_KP = -8（pid-new.h） */
    err = s_lqr.omega_radps * 57.29578f / TURN_ANG_KP;
    return vision_entry_lqr_constrain_f(err, -LQR_ERR_MAX_DEG, LQR_ERR_MAX_DEG);
}

const vision_entry_lqr_state_t *VisionEntryLqr_GetState(void)
{
    return &s_lqr;
}
