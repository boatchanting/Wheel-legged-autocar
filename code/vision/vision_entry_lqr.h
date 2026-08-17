/*
 * =================================================================================
 * 文件: vision_entry_lqr.h
 * 作用: 0 核 (Core 0) 单边桥/坡道「进入段」LQR 方向控制器（共享模块）。
 * 说明: 只输出方向 err_degree，绝不读写 target_speed_set（速度归路径/导航管理）。
 *       视觉段: e = D·sin(β+ψ)，ω = k1·e + k2(v)·ψ_err（CARE 闭式增益）。
 *       盲区段由调用方回退锁角/直行（本模块不处理）。
 * 依据: docs/任务规划/单边桥与坡道进入段LQR方向控制接入规划.md（v5）
 * =================================================================================
 */
#ifndef VISION_ENTRY_LQR_H
#define VISION_ENTRY_LQR_H

#include "zf_common_headfile.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * LQR 参数与调参指南（定稿值以《无降速版调参文档》为准，无指令降速版）
 * ----------------------------------------------------------------------------
 * 控制律: ω = k1·e + k2(v)·ψ_err，CARE 闭式增益 k1=√Qy、k2=√(2·max(v,0.3)·√Qy+Qψ)。
 * 只有 Qy、Qψ 两个调参旋钮；其余为外部标定/物理常数，勿动。
 *
 * 【调参纪律】（必须遵守，违反必翻车）
 *   1. 每次只动一个旋钮，观察唇口 e / ψ 读数（上位机 [BridgeCtrl] 串口 lqr=le/lpsi/lD/lw 字段，
 *      参数集见进入任务时的 [LqrParam] 一次性打印；状态结构体 lqr_* 字段持续可读）。
 *   2. 先 Qψ 后 Qy：短窗架构里航向是稀缺资源，横向其次；e、ψ 都差时先动 Qψ。
 *   3. 在目标最高速度下调：低速裕量天然大，高速通过的参数低速必过（已验单调）。
 *   4. 恶化方向判断：改完若 ω 曲线饱和段变长（|ω| 持续贴 W_MAX）→ 方向反了，立即回退。
 *   5. 不动的参数：K_lock=1.8（盲区锁角，只保向不纠偏）、drop=0.8（转弯掉速系数，
 *      物理测量值非旋钮）、转向内环 PD（W_SLEW 必须 ≥6 rad/s²，命门，现役 9 勿动）。
 * ============================================================================
 */
/*
 * LQR_QY —— 横向权重（起步 150）
 *   k1 = √Qy ≈ 12.25；Qy 越大横向收敛越快，但过大在高速会把 ω 需求推过 W_MAX/slew
 *   造成饱和振荡（Qy≥150 在 v≥3.5 饱和失稳）。
 *   调参（症状 → 动作）：
 *     - 唇口 e 大、横向收敛慢              → Qy +10~25
 *     - 接近段横向过冲 / S 形摆动 /
 *       |ω| 持续贴 2.2 rad/s              → Qy −10~25
 *     - 仅高速(≥2.5 m/s)振荡、低速正常      → 优先 Qy −20（先于降 Qψ）
 *   边界：上限自觉线 ≈200（再高 2.0 m/s 开始掉通过率）；下限别低于 100
 *         （再低 1.5m 窗内横向消不完）。
 */
#define LQR_QY                  (150.0f)

/*
 * LQR_QPSI —— 航向权重（起步 24）
 *   短窗内 ψ 误差没有时间自然沉降，必须靠硬航向通道第一拍压住，故 Qψ 显著大于有降速版(4)。
 *   调参（症状 → 动作）：
 *     - ψ唇 偏大(>3°)但 e 正常           → Qψ +2~4
 *     - 航向抖动 / 过桥后保向段来回摆      → Qψ −2~4
 *     - e、ψ 都差                        → 先动 Qψ（航向消了 e 才有几何收敛条件），再动 Qy
 */
#define LQR_QPSI                (24.0f)

/* W_MAX —— 极限转向角速度（外部标定，勿当旋钮）：执行器极限，换车/换硬件才改。 */
#define LQR_W_MAX_RADPS         (2.2f)

/* err_degree 钳位（由 W_MAX 推导 ≈15.76°，勿单独调）。 */
#define LQR_ERR_MAX_DEG         (LQR_W_MAX_RADPS * 57.29578f / 8.0f)

/* k2 增益调度速度下限（max(v,0.3)），防低速时 k2 过小，一般不调。 */
#define LQR_V_FLOOR_MPS         (0.3f)

/* IPM 物理坐标无效标记（与 PVC_VISION_PHY_INVALID_MM 一致，固定，勿动）。 */
#define LQR_PHY_INVALID_MM      (32767)

/*
 * LQR_DETECT_RANGE_M —— 视觉段检测距离（起步 1.5m，现场自行标定）
 *   无降速后修正时间全靠检测窗：1.5m 撑 2.5 m/s；1.0m 只能撑 1.0 m/s。
 *   - 现场能超过 1.5m：每 +0.5m 可把 Qy 降 10~20（更从容），或巡航提速 +0.5 m/s。
 *   - 现场掉回 1.0m：最大速度降 1.0 m/s，且权重换 Qy=400 / Qψ=32（短窗需更激进增益）。
 */
#define LQR_DETECT_RANGE_M      (1.5f)

/*
 * LQR_KLOCK —— 盲区锁角增益（调用方 bridge 锁角用 yaw_hold_kp 同值 1.8；本模块不输出锁角）
 *   盲区只保向不纠偏：调大无意义、调小浪费进入角精度，勿动。
 */
#define LQR_KLOCK               (1.8f)

/**
 * @brief LQR 进入段控制器的内部状态（诊断可观测）。
 *
 * @note 末尾 6 个 qy/qpsi/detect_range_m/w_max_radps/v_floor_mps/err_max_deg
 *       为「调参诊断副本」：Reset 时从编译期宏同步，只读、不参与控制计算，
 *       供日志/上位机确认当前实际生效参数（2026-08-17 增）。
 */
typedef struct
{
    float   entry_yaw_deg;     /* 进入状态机时刻锁存的基准航向（ψ_存储） */
    uint8   valid;             /* 本次更新是否有效（视觉段有效） */
    float   beta_rad;          /* 桥唇/入口方位角（IPM 车体系，右正） */
    float   dist_m;            /* 桥唇距离 D */
    float   e_m;               /* 横向偏差重建 e = D·sin(β+ψ) */
    float   psi_err_rad;       /* 航向偏差 ψ_err = ψ_存储 − ψ */
    float   omega_radps;       /* 期望角速度 ω（已钳 W_MAX） */
    /* --- 调参诊断副本（Reset 同步自宏，勿在控制路径使用） --- */
    float   qy;                /* 横向权重 Qy（k1=√Qy），LQR_QY */
    float   qpsi;              /* 航向权重 Qψ，LQR_QPSI */
    float   detect_range_m;    /* 视觉段检测距离（m），LQR_DETECT_RANGE_M */
    float   w_max_radps;       /* ω 输出钳位（rad/s），LQR_W_MAX_RADPS */
    float   v_floor_mps;       /* k2 速度下限（m/s），LQR_V_FLOOR_MPS */
    float   err_max_deg;       /* err_degree 钳位（deg），LQR_ERR_MAX_DEG */
} vision_entry_lqr_state_t;

/**
 * @brief 复位控制器，锁存基准航向。
 * @param entry_yaw_deg 进入状态机时刻的惯导航向（度）。
 */
void VisionEntryLqr_Reset(float entry_yaw_deg);

/**
 * @brief 视觉段 LQR 更新（每视觉包调用一次，内部用 D≤检测距离 + phy 有效判定）。
 * @param phy_x_mm 桥唇/入口 IPM 物理 X（mm，向右为正）。
 * @param phy_y_mm 桥唇/入口 IPM 物理 Y（mm，向前为正）。
 * @param yaw_deg  惯导 relative_yaw（度）。
 * @param v_mps    车身速度（m/s，互补滤波 vx_body），仅 k2 增益调度只读。
 * @return 1=本次有效（视觉段），0=无效（调用方回退锁角/直行）。
 */
uint8 VisionEntryLqr_UpdateVision(int16 phy_x_mm, int16 phy_y_mm, float yaw_deg, float v_mps);

/**
 * @brief 取当前方向指令 err_degree（deg）。
 * @return err_degree（无效时返回 0，由调用方回退）。
 */
float VisionEntryLqr_GetErrDegree(void);

/**
 * @brief 读取当前 LQR 内部状态（诊断/上位机用，只读）。
 */
const vision_entry_lqr_state_t *VisionEntryLqr_GetState(void);

#ifdef __cplusplus
}
#endif

#endif /* VISION_ENTRY_LQR_H */
