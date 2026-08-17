/*
 * =================================================================================
 * 文件: vision_entry_lqr.c
 * 作用: 0 核 (Core 0) 单边桥/坡道「进入段」方向控制器实现（双模式）。
 * 说明: 纯方向算法，无副作用、无 target_speed_set 引用。
 *       ENTRY_CTRL_MODE_PD : PD + 视觉扰动（默认）
 *       ENTRY_CTRL_MODE_LQR: 原 LQR（f87b18b 已验证修复版）
 * 依据: docs/任务规划/进入段方向控制LQR改PD结构-详细规划.md（v1）
 * =================================================================================
 */
#include "vision/vision_entry_lqr.h"

#include <math.h>
#include <string.h>

/* --- 内部状态（仅本文件可用） --- */
static vision_entry_lqr_state_t s_lqr;

#if (ENTRY_CTRL_MODE == ENTRY_CTRL_MODE_PD)
/* PD 模式航向微分状态（2ms 差分 + 一阶低通） */
static float s_psi_err_prev_rad = 0.0f;   /* 上一拍航向误差（rad） */
static float s_psi_err_dot_f    = 0.0f;   /* 滤波后航向误差微分（rad/s） */
static uint8 s_first_valid      = 0U;     /* 首个有效视觉帧标志 */
#endif

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
#if (ENTRY_CTRL_MODE == ENTRY_CTRL_MODE_PD)
    s_psi_err_prev_rad = 0.0f;
    s_psi_err_dot_f    = 0.0f;
    s_first_valid      = 0U;
#endif
}

uint8 VisionEntryLqr_UpdateVision(int16 phy_x_mm, int16 phy_y_mm, float yaw_deg, float v_mps)
{
    const float deg2rad = 0.0174532925f;
    float fx;
    float fy;
    float beta_rad;
    float dist_m;
    float psi_err_deg;
    float psi_err_rad;
    float e_m;
    float omega;
#if (ENTRY_CTRL_MODE == ENTRY_CTRL_MODE_LQR)
    float k1;
    float k2;
#endif

    /* 物理坐标无效 → 本拍无视觉，调用方回退 */
    if ((phy_x_mm == PD_PHY_INVALID_MM) || (phy_y_mm == PD_PHY_INVALID_MM))
    {
        s_lqr.valid = 0U;
        return 0U;
    }

    fx = (float)phy_x_mm;
    fy = (float)phy_y_mm;
    dist_m = sqrtf(fx * fx + fy * fy) / 1000.0f;

    /* 超出检测距离 → 盲区段，调用方回退锁角 */
    if (dist_m > PD_DETECT_RANGE_M)
    {
        s_lqr.valid = 0U;
        return 0U;
    }

    beta_rad = atan2f(fx, fy);
    psi_err_deg = vision_entry_lqr_normalize_angle(s_lqr.entry_yaw_deg - yaw_deg);
    psi_err_rad = psi_err_deg * deg2rad;

#if (ENTRY_CTRL_MODE == ENTRY_CTRL_MODE_PD)
    /* ========== 模式 B：PD + 视觉扰动（默认） ========== */
    /* 横向偏差重建（投影至入口基准系：e = D·sin(β − ψ_err)） */
    e_m = dist_m * sinf(beta_rad - psi_err_rad);

    /* 航向微分（2ms 差分 + 一阶低通，防 IMU 噪声放大） */
    if (s_first_valid != 0U)
    {
        const float psi_err_dot = (psi_err_rad - s_psi_err_prev_rad) * PD_CTRL_HZ;
        const float alpha = (1.0f / PD_CTRL_HZ) / (PD_D_TAU_S + 1.0f / PD_CTRL_HZ);
        s_psi_err_dot_f += alpha * (psi_err_dot - s_psi_err_dot_f);
    }
    s_psi_err_prev_rad = psi_err_rad;
    s_first_valid = 1U;

    /* PD + 视觉扰动合成 */
    omega = PD_P_PSI * psi_err_rad + PD_D_PSI * s_psi_err_dot_f + PD_K_E * e_m;
#else
    /* ========== 模式 A：LQR（f87b18b 已验证修复版） ========== */
    e_m = dist_m * sinf(beta_rad - psi_err_rad);

    /* CARE 闭式增益：k1=√Qy，k2=√(2·max(v,0.3)·k1+Qψ) */
    k1 = sqrtf(LQR_QY);
    k2 = sqrtf(2.0f * fmaxf(v_mps, 0.3f) * k1 + LQR_QPSI);

    omega = k1 * e_m + k2 * psi_err_rad;
#endif

    /* 共用钳位（两种模式同一物理限制） */
    omega = vision_entry_lqr_constrain_f(omega, -PD_W_MAX_RADPS, PD_W_MAX_RADPS);

    s_lqr.beta_rad = beta_rad;
    s_lqr.dist_m = dist_m;
    s_lqr.e_m = e_m;
    s_lqr.psi_err_rad = psi_err_rad;
#if (ENTRY_CTRL_MODE == ENTRY_CTRL_MODE_PD)
    s_lqr.psi_err_dot_radps = s_psi_err_dot_f;
#endif
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
    return vision_entry_lqr_constrain_f(err, -PD_ERR_MAX_DEG, PD_ERR_MAX_DEG);
}

const vision_entry_lqr_state_t *VisionEntryLqr_GetState(void)
{
    return &s_lqr;
}
