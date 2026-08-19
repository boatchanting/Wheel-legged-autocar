/*
 * =================================================================================
 * 文件: vision_entry_lqr.h
 * 作用: 0 核 (Core 0) 单边桥/坡道「进入段」方向控制器（共享模块，双模式）。
 * 说明: 只输出方向 err_degree，绝不读写 target_speed_set（速度归路径/导航管理）。
 *       双模式（ENTRY_CTRL_MODE 一键切换）：
 *         - ENTRY_CTRL_MODE_PD : PD + 视觉扰动（默认）
 *             ω = P·ψ_err + D·ψ_err' + K_E·e，e = D·sin(β−ψ_err)
 *         - ENTRY_CTRL_MODE_LQR: 原 LQR（f87b18b 已验证修复版）
 *             ω = k1·e + k2(v)·ψ_err，CARE 闭式增益
 *       盲区段由调用方回退锁角/直行（本模块不处理）。
 * 依据: docs/任务规划/进入段方向控制LQR改PD结构-详细规划.md（v1）
 * =================================================================================
 */
#ifndef VISION_ENTRY_LQR_H
#define VISION_ENTRY_LQR_H

#include "zf_common_headfile.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 双模式切换（一键 A/B，现场只改 ENTRY_CTRL_MODE 一行）
 *   ENTRY_CTRL_MODE_PD  —— 新 PD 结构（航向 PD + 视觉扰动）【默认】
 *   ENTRY_CTRL_MODE_LQR —— 原 LQR（f87b18b 已验证修复版，保留用于对比/回退）
 * 两套参数各自独立，切换互不影响。
 * ============================================================================ */
#define ENTRY_CTRL_MODE_PD      (1U)
#define ENTRY_CTRL_MODE_LQR     (0U)
#define ENTRY_CTRL_MODE         (ENTRY_CTRL_MODE_PD)   /* ← 切换点 */

/* ============================================================================
 * 模式 A：LQR（f87b18b 已验证修复版，勿再手动改回 bug 版）
 *   控制律: ω = k1·e + k2(v)·ψ_err，k1=√Qy、k2=√(2·max(v,0.3)·√Qy+Qψ)。
 *   旋钮只有 Qy、Qψ 两个。
 * ============================================================================ */
/*
 * LQR_QY —— 横向权重（f87b18b 已验证值 120）
 *   k1 = √Qy ≈ 10.95。Qy 越大横向收敛越快，但过大在高速会把 ω 需求推过 W_MAX/slew
 *   造成饱和振荡（Qy≥150 在 v≥3.5 饱和失稳）。
 *   边界：上限自觉线 ≈200；下限别低于 100（再低 1.5m 窗内横向消不完）。
 */
#define LQR_QY                  (120.0f)

/*
 * LQR_QPSI —— 航向权重（f87b18b 已验证值 32）
 *   短窗内 ψ 误差没有时间自然沉降，必须靠硬航向通道第一拍压住。
 */
#define LQR_QPSI                (32.0f)

/* ============================================================================
 * 模式 B：PD + 视觉扰动（默认，精修参数 K_E=15 / P=9.5 / D=0.25）
 *   控制律: ω = P·ψ_err + D·ψ_err' + K_E·e
 *     - 航向 PD：P 维持"自适应回归正确角度"（entry_yaw）；D 提供阻尼
 *     - K_E·e：视觉横向偏差 e = D·sin(β−ψ_err) 作为扰动注入
 *   旋钮 3 个：K_E / P / D。
 *   【2026-08 路面约束寻优】验收=上桥瞬间 |x|≤11.5cm 且 |ψ|≤20°（30cm 内拉回）：
 *     精修 15/9.5/0.25 在 18 工况通过 9/18、裕度 5.66，与 LQR(120/32) 等效（9/18, 5.68）；
 *     现役 50/50/3.5 仅 1/18（裕度 0.31）——K_E/P 过大导致全程饱和、过旋转甩出桥口。
 *     可修能力（平行偏差）：70cm@1m/s、25cm@1.5、15cm@2.0；方位角 15°@1m/s。
 *     注意：方位角≥25°、平行≥40cm@1.5 为几何极限，调参不可修（需上游对齐/提前检测）。
 *   调参（症状 → 动作）：
 *     - 横向收敛慢、拉不回来            → K_E +1~2（先动它）
 *     - 横向过冲 / S 形摆动 / |ω| 贴 2.2 → K_E −1~2
 *     - ψ唇 偏大(>3°)但 e 正常          → P +0.5~1
 *     - 航向抖动 / 保向段来回摆          → D +0.1~0.25（甜点 0~0.5，严禁 >0.5×P，
 *                                       否则 ψ_err'=−ω 的代数反馈把增益压缩 (1+D) 倍）
 *     - 仅高速振荡、低速正常             → 先 K_E −2，再考虑 P −1
 *   D 项实现：2ms 差分 + 一阶低通（PD_D_TAU_S），防 IMU 噪声放大。
 * ============================================================================ */
#define PD_K_E                  (15.0f)   /* 视觉扰动放大系数（对应原 k1=√Qy，精修 2026-08） */
#define PD_P_PSI                (9.5f)    /* 航向比例（对应原 k2，精修 2026-08） */
#define PD_D_PSI                (0.25f)   /* 航向微分阻尼（甜点 0~0.5，精修 2026-08） */
#define PD_D_TAU_S              (0.05f)   /* 航向微分一阶低通时间常数(s)，防噪声 */
#define PD_CTRL_HZ              (500.0f)  /* 控制周期 500Hz=2ms，微分分母（若改周期需同步） */

/* ============================================================================
 * 两模式共用的物理常数（勿当旋钮）
 * ============================================================================ */
/* W_MAX —— 极限转向角速度（外部标定）：执行器极限，换车/换硬件才改。 */
#define PD_W_MAX_RADPS          (2.2f)

/* err_degree 钳位（由 W_MAX 推导 ≈15.76°，勿单独调）。 */
#define PD_ERR_MAX_DEG          (PD_W_MAX_RADPS * 57.29578f / 8.0f)

/* IPM 物理坐标无效标记（与 PVC_VISION_PHY_INVALID_MM 一致，固定，勿动）。 */
#define PD_PHY_INVALID_MM       (32767)

/*
 * PD_DETECT_RANGE_M —— 视觉段检测距离（起步 1.5m，现场自行标定）
 *   无降速后修正时间全靠检测窗：1.5m 撑 2.5 m/s；1.0m 只能撑 1.0 m/s。
 */
#define PD_DETECT_RANGE_M       (1.5f)

/**
 * @brief 进入段方向控制器的内部状态（诊断可观测，双模式共用）。
 */
typedef struct
{
    float   entry_yaw_deg;     /* 进入状态机时刻锁存的基准航向（ψ_存储） */
    uint8   valid;             /* 本次更新是否有效（视觉段有效） */
    float   beta_rad;          /* 桥唇/入口方位角（IPM 车体系，右正） */
    float   dist_m;            /* 桥唇距离 D */
    float   e_m;               /* 视觉横向偏差 e = D·sin(β−ψ_err)（扰动源） */
    float   psi_err_rad;       /* 航向偏差 ψ_err = ψ_存储 − ψ */
    float   psi_err_dot_radps; /* 航向偏差微分（滤波后，PD 模式诊断用） */
    float   omega_radps;       /* 期望角速度 ω（已钳 W_MAX） */
} vision_entry_lqr_state_t;

/**
 * @brief 复位控制器，锁存基准航向。
 * @param entry_yaw_deg 进入状态机时刻的惯导航向（度）。
 */
void VisionEntryLqr_Reset(float entry_yaw_deg);

/**
 * @brief 视觉段方向更新（每控制周期调用，内部用 D≤检测距离 + phy 有效判定）。
 *        双模式：ENTRY_CTRL_MODE_PD → PD+视觉扰动；ENTRY_CTRL_MODE_LQR → 原 LQR。
 * @param phy_x_mm 桥唇/入口 IPM 物理 X（mm，向右为正）。
 * @param phy_y_mm 桥唇/入口 IPM 物理 Y（mm，向前为正）。
 * @param yaw_deg  惯导 relative_yaw（度）。
 * @param v_mps    车身速度（m/s，互补滤波 vx_body）；LQR 模式用于 k2 增益调度，
 *                 PD 模式当前不使用（保留参数以兼容调用方）。
 * @return 1=本次有效（视觉段），0=无效（调用方回退锁角/直行）。
 */
uint8 VisionEntryLqr_UpdateVision(int16 phy_x_mm, int16 phy_y_mm, float yaw_deg, float v_mps);

/**
 * @brief 取当前方向指令 err_degree（deg）。
 * @return err_degree（无效时返回 0，由调用方回退）。
 */
float VisionEntryLqr_GetErrDegree(void);

/**
 * @brief 读取当前方向控制器内部状态（诊断/上位机用，只读）。
 */
const vision_entry_lqr_state_t *VisionEntryLqr_GetState(void);

#ifdef __cplusplus
}
#endif

#endif /* VISION_ENTRY_LQR_H */
