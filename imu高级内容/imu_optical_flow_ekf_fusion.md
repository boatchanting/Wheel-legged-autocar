# IMU/ToF/光流/视觉多传感器融合 EKF 与互补算法 — 极致细节技术文档

> **工程**: Seekfree CYT4BB 飞控 (CM7_0)  
> **生成日期**: 2026-07-22  
> **适用平台**: Cortex-M7 双核, 单精度浮点, 无 DSP 加速  
> **编码**: GB2312

---

## 目录

1. [系统融合架构总览](#1-系统融合架构总览)
2. [四元数 EKF 姿态估计](#2-四元数-ekf-姿态估计)
3. [Height KF 高度/垂速估计](#3-height-kf-高度垂速估计)
4. [光流二维速度估计器](#4-光流二维速度估计器)
5. [车灯视觉 CF3 互补速度观测器](#5-车灯视觉-cf3-互补速度观测器)
6. [双 ToF VL53L8CX 4×4 网格融合](#6-双-tof-vl53l8cx-4x4-网格融合)
7. [Gyro 历史环形缓冲区 (去旋)](#7-gyro-历史环形缓冲区-去旋)
8. [加速度链 (光流预测输入)](#8-加速度链-光流预测输入)
9. [速度源切换策略](#9-速度源切换策略)
10. [坐标系变换与杆臂补偿全览](#10-坐标系变换与杆臂补偿全览)
11. [工程移植指南](#11-工程移植指南)

---

## 1. 系统融合架构总览

### 1.1 融合拓扑

```
                          ┌─────────────────────────┐
                          │    IMU 双路 (2kHz/416Hz) │
                          │    ICM-42688 兼容        │
                          └───────────┬─────────────┘
                                      │
                    ┌─────────────────┼─────────────────┐
                    ▼                 ▼                  ▼
            ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
            │ 四元数 EKF    │  │ Gyro 历史环   │  │ Acc 链        │
            │ (500us+1ms)  │  │ (500us push)  │  │ (1ms push)    │
            │ 姿态估计      │  │ 256槽×128ms  │  │ notch+BW2     │
            └──────┬───────┘  └──────┬───────┘  └──────┬───────┘
                   │                 │                  │
      ┌────────────┼─────────────────┼──────────────────┼──────────────┐
      │            ▼                 ▼                  ▼              │
      │   ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
      │   │ 姿态投影缓存  │  │ 光流去旋      │  │ 光流估计器    │       │
      │   │ cos_tilt     │  │ gyro补偿      │  │ 2D KF 预测    │       │
      │   │ world_z_acc  │  │ 交叉轴+尖峰   │  │ (4ms 250Hz)  │       │
      │   └──────┬───────┘  └──────┬───────┘  └──────┬───────┘       │
      │          │                 │                  │               │
      │          ▼                 ▼                  ▼               │
      │   ┌──────────────────────────────────────────────┐           │
      │   │           Height KF (1ms 1kHz)                │           │
      │   │           2 状态: height + vz                 │           │
      │   │           预测: world_z_accel 输入            │           │
      │   │           校正: ToF 双源融合观测              │           │
      │   └──────────────────────┬───────────────────────┘           │
      │                          │                                    │
      └──────────────────────────┼────────────────────────────────────┘
                                 │
                    ┌────────────┴────────────┐
                    ▼                         ▼
            ┌──────────────┐          ┌──────────────┐
            │ 双 ToF 网格    │          │ 车灯视觉 CF3  │
            │ VL53L8CX 4×4 │          │ 互补观测器    │
            │ 中值+MAD+簇   │          │ 位置+速度+偏置│
            │ 双源 NIS 融合 │          │ 源切换策略    │
            └──────────────┘          └──────────────┘
```

### 1.2 调度时序

```
时间轴 (每 1ms):  
  [0us]    500us PIT: IMU kick #1
  [500us]  500us PIT: 四元数预测 + 速率PID + IMU kick #2
  [1000us] 1ms PIT:   EKF acc校正 + 高度KF预测/校正 + 姿态PID
                       + 光流acc链推进(仅416Hz fresh帧)
  [2000us] 1ms PIT (cycle 2): 同上
  [3000us] 1ms PIT (cycle 3): 同上
  [4000us] 4ms PIT:   光流传感器更新 + 去旋 + 速度估计器KF
                       + 高度位置PID + 光流速度PID
                       + 车灯视觉CF3观测器
```

---

## 2. 四元数 EKF 姿态估计

### 2.1 数学模型

**状态向量** (4维):
$$ \mathbf{q} = [q_w, q_x, q_y, q_z]^T $$

**预测模型** (500us 一步积分):
$$ \mathbf{q}_{k+1} = \mathbf{q}_k + \frac{\Delta t}{2} \boldsymbol{\Omega}(\boldsymbol{\omega}) \mathbf{q}_k $$

其中 $\boldsymbol{\Omega}(\boldsymbol{\omega})$ 为 gyro 四元数乘法矩阵:
$$ \boldsymbol{\Omega}(\boldsymbol{\omega}) = \begin{bmatrix}
0 & -\omega_x & -\omega_y & -\omega_z \\
\omega_x & 0 & \omega_z & -\omega_y \\
\omega_y & -\omega_z & 0 & \omega_x \\
\omega_z & \omega_y & -\omega_x & 0
\end{bmatrix} $$

**归一化**: 预测后立即归一化 $\mathbf{q} = \mathbf{q} / \|\mathbf{q}\|$

**协方差预测** (1ms，合并 2 个 500us 步):
$$ \mathbf{P}_{k+1} = \mathbf{F} \mathbf{P}_k \mathbf{F}^T + \mathbf{Q} \cdot \Delta t $$

**过程噪声**:
$$ \mathbf{Q} = 2 \times 10^{-5} \cdot \Delta t \cdot \mathbf{I}_{4\times4} $$

**观测模型** (acc = 重力方向):
$$ \mathbf{z}_{pred} = \mathbf{R}(\mathbf{q}) \cdot [0, 0, 1]^T $$
$$ \mathbf{H} = \frac{\partial \mathbf{z}_{pred}}{\partial \mathbf{q}} $$

**校正更新** (Joseph 形式):
$$ \mathbf{S} = \mathbf{H}\mathbf{P}\mathbf{H}^T + \mathbf{R} $$
$$ \mathbf{K} = \mathbf{P}\mathbf{H}^T \mathbf{S}^{-1} $$
$$ \mathbf{q} = \mathbf{q} + \mathbf{K} \cdot \mathbf{innov} $$
$$ \mathbf{P} = (\mathbf{I} - \mathbf{K}\mathbf{H}) \mathbf{P} (\mathbf{I} - \mathbf{K}\mathbf{H})^T + \mathbf{K}\mathbf{R}\mathbf{K}^T $$

### 2.2 完整参数表

| 参数宏 | 值 | 单位 | 说明 |
|--------|-----|------|------|
| `IMU_EKF_P_INIT` | 0.02 | — | 初始协方差对角值 |
| `IMU_EKF_P_DIAG_MIN` | 1.0e-7 | — | 协方差下界 |
| `IMU_EKF_P_DIAG_MAX` | 1.0 | — | 协方差上界 (超限触发恢复) |
| `IMU_EKF_Q_QUAT_CONT` | 2.0e-5 | — | 连续过程噪声强度 |
| `IMU_EKF_R0` | 0.004 | — | 基础观测噪声方差 |
| `IMU_EKF_R_MIN` | 0.001 | — | 自适应 R 下限 |
| `IMU_EKF_R_MAX_NORMAL` | 0.08 | — | 正常模式 R 上限 |
| `IMU_EKF_R_MAX_DYNAMIC` | 0.80 | — | 动态低trust R 上限 |
| `IMU_EKF_R_ALPHA` | 30.0 | — | 模长偏离对 R 的放大系数 |
| `IMU_EKF_CHI2_GATE` | 9.0 | — | Chi2 门限 (~3σ) |
| `IMU_EKF_REJECT_R_INFLATE` | 50.0 | — | Reject R 膨胀倍数 |
| `IMU_EKF_SOLVE_EPS` | 1.0e-8 | — | Cholesky pivot 下限 |

### 2.3 Acc Trust 三维门控

```c
// 三个独立 trust 分量相乘得到最终信任度
trust_norm = ramp_down(|acc_norm - 1g|,  0.03g,  0.18g);  // 模长信任
trust_innov= ramp_down(innov_norm,        0.035,  0.22);   // 三轴创新信任
trust_xy   = ramp_down(innov_xy,          0.030,  0.20);   // 水平创新信任

acc_trust = trust_norm * trust_innov * trust_xy;  // 范围 [0, 1]

if (acc_trust < 0.08) skip_correct();  // 跳过低信任帧
```

**ramp_down 函数**:
```c
float ramp_down(value, soft, hard) {
    if (value <= soft) return 1.0;
    if (value >= hard) return 0.0;
    return 1.0 - (value - soft) / (hard - soft);
}
```

### 2.4 自适应 R 策略

```c
R_eff = R0 * (1.0 + 30.0 * |acc_norm - 1g|);
R_eff = clip(R_eff, 0.001, 0.08);  // 正常模式
R_eff = clip(R_eff, 0.001, 0.80);  // 动态低trust模式

// R 也可以由 trust 反推缩放
R_eff = max(R_eff, R0 / max(trust, 0.005));
```

### 2.5 动态保持策略

| 级别 | 保持时间 | R 上限 | 触发条件 |
|------|---------|--------|---------|
| **Soft** | 0ms | 0.08 | trust 略低，只通过 R 自适应降权 |
| **Hard** | 30ms | 0.80 | acc 模长或创新进入 hard 区 |
| **Severe** | 90ms | 0.80 | acc_trust 极低或连续 hard 保持 |

```c
// 动态保持计数器 (acc_hold_ms)
if (trust >= 0.08) {
    hold_ms = 0;  // 正常校正
} else if (进入 hard 区) {
    hold_ms = 30;  // 短保持
} else {
    hold_ms = 90;  // 长保持, 允许弱校正
}
```

### 2.6 数值保护

```c
// 四元数范数检查
if (q_norm < 0.5 || q_norm > 2.0) → 触发恢复, 重置为单位四元数

// 协方差对称化
p01 = p10 = 0.5*(p01 + p10);

// 交叉项限幅
max_cross = sqrt(p00 * p11);
p01 = clip(p01, -max_cross, max_cross);

// Cholesky 求解保护
pivot > 1e-8 → 正常求解; 否则 → 跳过 correct, 计数 solve_error
```

### 2.7 投影缓存 (供下游模块使用)

```c
// 每 1ms 更新一次，避免各模块重复计算
world_z_from_body_x = 2*(q.x*q.z - q.w*q.y);  // body X → world Z 方向余弦
world_z_from_body_y = 2*(q.y*q.z + q.w*q.x);  // body Y → world Z 方向余弦
world_z_from_body_z = 1 - 2*(q.x² + q.y²);    // body Z → world Z (= cos_tilt)
cos_tilt = world_z_from_body_z;

// 世界 Z 线加速度 (已去重力)
world_z_accel = acc_x*proj_x + acc_y*proj_y + acc_z*proj_z - 9.81;
```

---

## 3. Height KF 高度/垂速估计

### 3.1 数学模型

**状态向量** (2维):
$$ \mathbf{x} = [h, v_z]^T $$
其中 $h$ 为相对起飞点的高度 (m)，$v_z$ 为世界 Z 轴垂向速度 (m/s, 向下为正)。

**预测模型** (1ms, 匀加速):
$$ h_{k+1} = h_k + v_{z,k} \cdot \Delta t + \frac{1}{2} a_{z,k} \cdot \Delta t^2 $$
$$ v_{z,k+1} = v_{z,k} + a_{z,k} \cdot \Delta t $$
$$ \mathbf{P}_{k+1} = \mathbf{F} \mathbf{P}_k \mathbf{F}^T + \mathbf{Q}_k $$

其中:
$$ \mathbf{F} = \begin{bmatrix} 1 & \Delta t \\ 0 & 1 \end{bmatrix} $$
$$ \mathbf{Q}_k = \begin{bmatrix} Q_h \cdot \Delta t & 0 \\ 0 & Q_{vz} \cdot \Delta t \end{bmatrix} $$

### 3.2 完整参数表

| 参数宏 | 值 | 单位 | 说明 |
|--------|-----|------|------|
| `HEIGHT_KF_Q_H` | 0.04 | m²/s | 高度过程噪声谱密度 |
| `HEIGHT_KF_Q_VZ` | 0.10 | m²/s³ | 速度过程噪声谱密度 |
| `HEIGHT_KF_GATE_REDUCE` | 12.25 | — | 3.5σ 降权门限 |
| `HEIGHT_KF_GATE_REJECT` | 25.0 | — | 5σ 拒绝门限 |
| `HEIGHT_KF_R_INFLATE` | 10.0 | — | 降权 R 放大倍数 |
| `HEIGHT_KF_HARD_INNOV_BASE_M` | 0.15 | m | 硬创新基础门限 |
| `HEIGHT_KF_HARD_INNOV_MAX_M` | 0.40 | m | 硬创新最大门限 (随时间增大) |
| `HEIGHT_KF_FRESH_TIMEOUT_MS` | 50 | ms | 无校正超时 |
| `HEIGHT_KF_H_MIN_M` | -0.02 | m | 高度下界 |
| `HEIGHT_KF_H_MAX_M` | 4.00 | m | 高度上界 |
| `HEIGHT_KF_VZ_MAX_M_S` | 3.0 | m/s | 垂速上界 |
| `HEIGHT_KF_P_MIN` | 1.0e-6 | — | 协方差下界 |
| `HEIGHT_KF_P00_MAX` | 1.0 | m² | 高度方差上界 |
| `HEIGHT_KF_P11_MAX` | 2.0 | (m/s)² | 速度方差上界 |

### 3.3 完整预测函数

```c
void tof_height_kf_predict_step_1ms(float world_z_accel_m_s2, float dt_s, uint8 acc_trusted)
{
    // ---- 前置安保 ----
    if (contract_fault_latched) { healthy=0; return; }
    if (dt_s <= 0 || dt_s > 0.01) { healthy=0; return; }  // dt 合同违规
    if (!acc_trusted) { healthy=0; return; }               // IMU 资格丢失
    if (|world_z_accel| > 20.0) { healthy=0; return; }     // 数值异常

    // ---- 状态预测 (匀加速模型) ----
    dt2 = dt_s * dt_s;
    height_m    += velocity_m_s * dt_s + 0.5 * world_z_accel * dt2;
    velocity_m_s += world_z_accel * dt_s;

    // ---- 协方差预测 (标量展开, 避免 2×2 矩阵乘法) ----
    p00 = state.p00 + dt_s*(state.p01+state.p10) + dt2*state.p11 + Q_H*dt_s;
    p01 = state.p01 + dt_s * state.p11;
    p10 = state.p10 + dt_s * state.p11;
    p11 = state.p11 + Q_VZ * dt_s;

    // ---- 绝对对地高度(不停更新, 即使无校正) ----
    range_vertical_abs_m = height_m + zero_height_m;

    // ---- 无更新计时 ----
    no_update_ms++;
    if (no_update_ms > 50) healthy = 0;

    // ---- 数值边界保护 ----
    tof_height_state_bound();
}
```

### 3.4 完整校正函数

```c
uint8 tof_height_kf_correct_handle(void)
{
    // ---- 步骤 1: 读取双缓冲观测 ----
    if (!tof_observation_snapshot_try_read_latest(&obs, &last_gen)) return 0;

    // ---- 步骤 2: 观测可用性检查 ----
    if (obs.age > STALE_LIMIT) return 0;
    if (!(obs.flags & MEASUREMENT_USABLE)) return 0;
    if (!obs.healthy) return 0;

    // ---- 步骤 3: 首次初始化 (重捕获) ----
    measurement_m = obs.height_m - zero_height_m;
    if (!state.initialized) {
        height_m = measurement_m;
        velocity_m_s = 0;
        p00 = 0.25; p11 = 1.0; p01 = p10 = 0;
        initialized = 1;
        return 2;  // 2=正常校正
    }

    // ---- 步骤 4: 长时间无校正 → 锁定 ----
    if (no_update_ms > 50) { healthy=0; contract_fault_latched=1; return 0; }

    // ---- 步骤 5: 创新计算与硬门限 ----
    r = clip(obs.measurement_var_m2, 9e-4, 9e-2);
    innovation = measurement_m - height_m;
    s = p00 + r;

    // 硬创新门限随时间增长 (防止长时间无校正后突然接受大跳变)
    hard_limit = clip(0.15 + 0.003 * no_update_ms, 0.15, 0.40);
    if (|innovation| > hard_limit) { reject_count++; return 0; }

    // ---- 步骤 6: NIS 门控 ----
    nis = innovation² / s;
    if (nis > 25.0) { reject_count++; return 0; }       // 5σ 拒绝
    if (nis > 12.25) { r *= 10.0; s = p00 + r; }        // 3.5σ 降权

    // ---- 步骤 7: Kalman 校正 (增益限幅版) ----
    k0 = clip(p00/s, 0.0, 0.85);
    k1 = clip(p10/s, -5.0, 5.0);

    height_m    += k0 * innovation;
    velocity_m_s += k1 * innovation;

    p00 = (1-k0)*p00;
    p01 = (1-k0)*p01;
    p10 = p10 - k1*p00_old;
    p11 = p11 - k1*p01_old;

    // ---- 步骤 8: 后处理 ----
    reject_count = 0;
    no_update_ms = 0;
    healthy = 1;
    return reduced ? 1 : 2;  // 1=降权校正, 2=正常校正
}
```

### 3.5 数值保护函数

```c
static void tof_height_state_bound(state) {
    // NaN/Inf 检测 → 重置
    if (!finite(height) || !finite(vel) || !finite(p00) || !finite(p11)) {
        reset_to_safe();
    }
    // 范围限幅
    height_m = clip(height_m, -0.02, 4.00);
    velocity_m_s = clip(velocity_m_s, -3.0, 3.0);
    // 协方差限幅
    p00 = clip(p00, 1e-6, 1.0);
    p01 = clip(p01, -sqrt(p00*p11), sqrt(p00*p11));
    p11 = clip(p11, 1e-6, 2.0);
    // 对称化
    p10 = p01;
}
```

---

## 4. 光流二维速度估计器

### 4.1 数学模型

**每轴独立 2 状态 KF** (X 和 Y 各一个):
$$ \mathbf{x}_{axis} = [v, a_{bias}]^T $$
其中 $v$ 为机体系速度 (m/s)，$a_{bias}$ 为加速度偏置 (m/s²)。

**预测模型** (4ms):
$$ v_{k+1} = v_k + (a_{meas} - a_{bias,k}) \cdot \Delta t $$
$$ a_{bias,k+1} = a_{bias,k} $$
$$ \mathbf{F} = \begin{bmatrix} 1 & -\Delta t \\ 0 & 1 \end{bmatrix} $$

### 4.2 完整参数表

| 参数宏 | 值 | 单位 | 说明 |
|--------|-----|------|------|
| `OPTFLOW_EST_ACC_PROC_NOISE` | 1.60 | m²/s³ | 速度过程噪声谱密度 |
| `OPTFLOW_EST_BIAS_PROC_NOISE` | 0.30 | m²/s⁵ | 偏置过程噪声谱密度 |
| `OPTFLOW_EST_MEAS_NOISE_MPS` | 0.16 | m/s | 量测基准标准差 |
| `OPTFLOW_EST_LOWQ_R_SCALE` | 1.4 | — | LOW_QUALITY R 放大 |
| `OPTFLOW_EST_GATE_SIGMA` | 4.0 | — | 创新门控 σ |
| `OPTFLOW_EST_INIT_VEL_STD` | 0.40 | m/s | 初始速度标准差 |
| `OPTFLOW_EST_INIT_BIAS_STD` | 0.35 | m/s² | 初始偏置标准差 |
| `OPTFLOW_EST_BIAS_LIMIT` | 0.80 | m/s² | 偏置状态限幅 |
| `OPTFLOW_EST_COV_MIN` | 1.0e-5 | — | 协方差下界 |
| `OPTFLOW_EST_COV_MAX` | 4.0 | — | 协方差上界 |
| `OPTFLOW_EST_MEAS_VAR_MAX` | 4.0 | m²/s² | 量测方差上限 |
| `OPTFLOW_EST_REJECT_INFLATE` | 1.8 | — | 拒绝后 P 膨胀倍数 |
| `OPTFLOW_EST_HARD_REJECT_COUNT` | 6 | 次 | 连续拒绝→INVALID |
| `OPTFLOW_EST_HARD_INVALID_AGE_S` | 0.180 | s | 无校正→INVALID |
| `OPTFLOW_EST_INVALID_DECAY_AGE_S` | 0.060 | s | 无校正→COAST |

### 4.3 单轴预测函数

```c
static void optical_estimator_axis_predict(axis, accel_m_s2, dt_s) {
    if (!axis->initialized) return;

    // 状态预测 (加速度补偿偏置后积分)
    axis->velocity_m_s += (accel_m_s2 - axis->accel_bias_m_s2) * dt_s;

    // 协方差预测 (标量展开)
    p00_new = p00 - dt_s*(p01+p10) + dt_s²*p11;
    p01_new = p01 - dt_s*p11;
    p10_new = p10 - dt_s*p11;
    p11_new = p11;

    // 运动自适应 Q: 加速度越大，过程噪声越大
    motion_factor = clip(|accel|, 0.15, 3.0);
    q_scale = 1.0 + motion_factor;
    p00 = p00_new + 1.60 * dt_s² * q_scale;
    p01 = p01_new;
    p10 = p10_new;
    p11 = p11_new + 0.30 * dt_s;

    optical_estimator_axis_bound(axis);
}
```

### 4.4 单轴校正函数

```c
static uint8 optical_estimator_axis_correct(axis, meas_m_s, quality, status, &innov_out, &gain_out) {
    if (!axis->initialized || status == INVALID) return 0;

    // R 自适应 (质量越低 R 越大)
    r_std = 0.16 / clip(quality, 0.10, 1.0);
    r_var = r_std²;
    if (status == LOW_QUALITY) r_var *= 1.4;
    r_var = clip(r_var, 1e-5, 4.0);

    // 创新计算与门控
    innovation = meas - velocity;
    innovation_var = p00 + r_var;
    nis_sq = innovation² / innovation_var;

    if (nis_sq > 4.0²) {  // 4σ 门控
        p00 *= 1.8;  // 拒绝后膨胀协方差
        p11 *= 1.8;
        reject_count++;
        return 0;
    }

    // 标准 KF 校正 (使用 kf2_scalar_correct)
    k0 = p00 / innovation_var;
    k1 = p10 / innovation_var;

    velocity     += k0 * innovation;
    accel_bias   += k1 * innovation;
    accel_bias    = clip(accel_bias, -0.80, 0.80);

    // Joseph 更新
    p00 = (1-k0)*(1-k0)*p00_old + k0*k0*r_var;
    p01 = (1-k0)*p01_old - k1*(1-k0)*p00_old + k0*k1*r_var;
    ...

    reject_count = 0;
    return 1;
}
```

### 4.5 估计器状态机

```
INVALID ──(有可信量测)──→ VALID
VALID   ──(>180ms无校正)──→ INVALID
VALID   ──(>60ms无校正)──→ COAST
COAST   ──(有可信量测)──→ VALID
COAST   ──(>120ms无校正)──→ INVALID

COAST 速度衰减:  velocity *= 0.996/拍 (1.0/s, 4ms因子)
INVALID 速度衰减: velocity *= 0.921/拍 (19.75/s, 4ms因子)
```

### 4.6 分轴退化策略 (P1-6)

```c
// 单轴连续拒绝 ≥ 12 次 (~48ms) → 强衰减该轴速度
if (!corrected_x && reject_count_x >= 12) {
    axis_x.velocity_m_s *= 0.85;
}
if (!corrected_y && reject_count_y >= 12) {
    axis_y.velocity_m_s *= 0.85;
}

// 任一轴成功校正 → 清零该轴 reject 计数
```

---

## 5. 车灯视觉 CF3 互补速度观测器

### 5.1 数学模型

**CF3 = 三阶互补滤波器** (Complementary Filter 3rd order)，融合：
- 视觉位置差分 → 高频噪声大但无漂移
- IMU 加速度积分 → 低频漂移但高频响应好

**每轴状态** (3 个):
$$ \mathbf{x} = [pos, vel, a_{bias}]^T $$

**预测** (IMU 加速度驱动):
$$ pos_{k+1} = pos_k + vel_k \cdot \Delta t + 0.5 \cdot (a_{meas} - a_{bias}) \cdot \Delta t^2 $$
$$ vel_{k+1} = vel_k + (a_{meas} - a_{bias}) \cdot \Delta t $$

**校正** (视觉位置到达时):
$$ err_{pos} = pos_{vision} - pos_{pred} $$

$$ pos = pos + K_{pos} \cdot err_{pos} $$
$$ vel = vel + K_{vel} \cdot err_{pos} / \Delta t_{vision} $$

$$ vel = (1 - blend) \cdot vel + blend \cdot vel_{vision\_filtered} $$

$$ a_{bias} = a_{bias} - K_{bias} \cdot err_{pos} / \Delta t_{vision}^2 $$

### 5.2 完整参数表

| 参数宏 | 值 | 说明 |
|--------|-----|------|
| `CARLIGHT_VISION_CF3_K_POS` | 0.15 | 位置误差校正增益 |
| `CARLIGHT_VISION_CF3_K_VEL` | 0.08 | 速度误差校正增益 |
| `CARLIGHT_VISION_CF3_K_BIAS` | 0.003 | 偏置校正增益 (极慢) |
| `CARLIGHT_VISION_VEL_BLEND` | 0.30 | 速度混合比例 (30%视觉差分) |
| `CARLIGHT_VISION_VEL_LIMIT_MPS` | 3.0 | 速度输出限幅 |
| `CARLIGHT_VISION_ACCEL_LIMIT_MPS2` | 5.0 | 加速度输入限幅 |
| `CARLIGHT_VISION_ACCEL_BIAS_LIMIT_MPS2` | 1.5 | 偏置限幅 |
| `CARLIGHT_VISION_RAW_VEL_REJECT_MPS` | 5.0 | 裸差分拒绝阈值 |
| `CARLIGHT_VISION_DT_MIN_S` | 0.02 | 最小视觉帧间隔 |
| `CARLIGHT_VISION_DT_MAX_S` | 0.50 | 最大视觉帧间隔 |

### 5.3 视觉速度预处理

```c
// 裸差分: 相对位置差分取反 (机体速度 = -相对目标位移/时间)
raw_vel = -(vision_pos - last_vision_pos) / vision_dt;

// 拒绝异常
if (|raw_vel| > 5.0) { reject; return; }

// PT3 低通: 3 级 PT1 级联
filtered_vel = PT3(raw_vel);
// PT3 截止频率由 CARLIGHT_VISION_VEL_PT3_HZ 控制
```

### 5.4 速度源切换策略

```
┌─────────────────────────────────────────────────────────┐
│                    速度源切换逻辑                         │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  默认源: CARLIGHT_VEL_SOURCE_OPTICAL_IMU                │
│          (光流 + IMU 二维 KF 估计器)                     │
│                                                          │
│  切换到视觉 CF3 的条件:                                  │
│  1. 光流低质量 > 80ms  →  BAD_OPTICAL                   │
│  2. 车灯位置无进展 > 800ms → NO_PROGRESS                │
│                                                          │
│  切回光流的条件:                                        │
│  1. 视觉丢失 (样本过期/不健康) → VISION_LOST            │
│  2. 光流恢复稳定 > 500ms → OPTICAL_RECOVER              │
│  3. 退出车灯模式 → MODE_EXIT                            │
│                                                          │
│  光流坏判据: quality < 0.25 或 estimator_state != VALID │
│  光流恢复判据: quality > 0.55 且 estimator_state=VALID  │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

---

## 6. 双 ToF VL53L8CX 4×4 网格融合

### 6.1 传感器参数

| 参数 | 值 | 说明 |
|------|-----|------|
| 传感器型号 | VL53L8CX | ToF 阵列 |
| 分辨率 | 4×4 = 16 区 | 多区测距 |
| 有效范围 | 20-4000mm | 单区 |
| 最小有效区数 | 4 区 | 形成观测 |
| 中心区最小数 | 2 区 | 主量测 |
| 双源配对超时 | 20000us | 配对同步 |

### 6.2 单源网格降维算法

```
输入: 16 区距离数组 distance_mm[16]

步骤 1: 有效性筛选
  valid_zones = {zone | 20 ≤ distance_mm[zone] ≤ 4000}
  if (count < 4) → INVALID

步骤 2: 中值 + MAD 鲁棒估计
  median_mm = median(valid_zones)
  deviations[i] = |valid_zones[i] - median_mm|
  mad_mm = median(deviations)  // 若 < 20 → 钳位到 20

步骤 3: 离群剔除
  threshold = 3 * mad_mm + 50mm
  center_inliers = {center_zones 中 |deviation| ≤ threshold}
  if (center_inliers ≥ 2):
    primary_mm = median(center_inliers)
  else:
    primary_mm = median_mm  (回退, 置 GRID_FALLBACK 标志)

步骤 4: 近距簇检测 (遮挡判断)
  for each zone:
    if (distance < median - 150mm):
      检查相邻 4 邻域是否也有近距读数
  if (存在相邻近距簇) → 置 OBSTRUCTION 标志 → valid=0

步骤 5: 姿态补偿
  height_mm = primary_mm * cos(tilt) + offset_x * sin(tilt_comp)
  // offset_x: ToF1=+100mm, ToF2=-100mm

步骤 6: 方差估计
  sigma = 1.4826 * mad_mm  // MAD→标准差
  variance = sigma² (限幅 900~90000 mm²)
  if (valid_count < 8) variance *= 2.0
  if (GRID_FALLBACK)   variance *= 1.5

步骤 7: 质量评分
  score = clip(valid_count*6.0 - mad_mm*0.10, 1.0, 100.0)
```

### 6.3 双源 NIS 一致性融合

```c
// 仅当双源均有效且时间差 ≤ 20000us 时融合
if (s0.valid && s1.valid && skew_us ≤ 20000) {
    diff_mm = s0.height_mm - s1.height_mm;
    nis = diff_mm² / (s0.variance_mm2 + s1.variance_mm2);

    if (nis ≤ 9.0) {  // 3σ 一致性
        // 逆方差加权 (限幅 20%~80%)
        weight0 = s1.variance / (s0.variance + s1.variance);
        weight0 = clip(weight0, 0.20, 0.80);

        fused_height = weight0 * s0.height + (1-weight0) * s1.height;
        fused_variance = (s0.variance * s1.variance) / (s0.variance + s1.variance);
        source_mask = 0x03;  // 双源
    } else {
        // 两源不一致 → 锁存歧义故障
        ambiguous_latched = 1;
        contract_fault_latched = 1;
    }
}
// 仅单源有效 → 方差放大 2.5 倍
else if (s0.valid || s1.valid) {
    fused_height = valid_source.height;
    fused_variance = valid_source.variance * 2.5;
    source_mask = valid_source_bit;
}
```

---

## 7. Gyro 历史环形缓冲区 (去旋)

### 7.1 设计目的

光流模块输出的是**包含自身旋转**的像素位移，需要扣除积分窗口内的 gyro 平均值才能得到纯粹的平移角速度。

### 7.2 数据结构

```c
#define OPTICAL_GYRO_HISTORY_SIZE 256  // 256 槽 (2 的幂，支持序号回绕)
// 覆盖时间: 256 × 500us = 128ms

typedef struct {
    volatile uint32 publish_sequence;  // 奇偶代际: 奇数=写入中
    uint32 sample_sequence;
    uint32 timestamp_us;
    imu_gyro_rad_s_struct gyro_rad_s;
} optical_gyro_history_item_struct;

// 全局控制
volatile uint32 gyro_history_write_sequence;   // 500us ISR 已写入总序号
volatile uint16 gyro_history_count;            // 已填充样本数 [0, 256]
```

### 7.3 写入 (500us ISR)

```c
void optical_gyro_history_push(timestamp_us, gyro_rad_s) {
    // 去重: 同一物理时间戳不重复入环
    if (timestamp_us == last_gyro_timestamp_us) return;

    next_seq = write_sequence + 1;
    slot = (next_seq - 1) % 256;

    // 发布: 先写数据，再原子提交序号
    publish_sequence |= 1;  // 奇数=写入中
    DMB();
    history[slot].gyro = *gyro;
    history[slot].timestamp = timestamp_us;
    history[slot].sample_sequence = next_seq;
    DMB();
    publish_sequence = next_seq;  // 偶数=稳定

    write_sequence = next_seq;
    if (count < 256) count++;
}
```

### 7.4 去旋读取 (4ms ISR)

```c
// 步骤 1: 获取边界快照 (两次复核)
optical_gyro_history_boundary_snapshot(&write_seq, &count);

// 步骤 2: 根据光流积分窗口中心时间回找 gyro
item_seq = write_seq - (now_us - frame_center_us + jitter) / 500us;

// 步骤 3: 复核读取 (两次确认)
optical_gyro_history_item_snapshot(item_seq, &time, &gyro);

// 步骤 4: 窗口平均 (以积分窗中心为参考)
gyro_avg = 窗口内 gyro 均值;
derot_rate = raw_rate - gyro_avg;
```

### 7.5 去旋补偿系数

```c
// 前光流 X(前向): 主要由 pitch gyro 补偿，含交叉轴
derot_x = 1.36 * gyro_pitch + 0.02 * gyro_roll + 0.08 * gyro_yaw;

// 前光流 Y(右向): 主要由 roll gyro 补偿 (取反)，含交叉轴  
derot_y = -1.39 * gyro_roll + 0.03 * gyro_pitch - 0.10 * gyro_yaw;
```

去旋后残差保护 (3 种):
1. **反向过补偿**: derot 推到反向 → 限幅并降质量
2. **高角速瞬态**: gyro/光流窗口错相 → 瞬态窗口限幅
3. **孤立尖峰**: raw 弱但 derot 单帧突跳 → 限幅

---

## 8. 加速度链 (光流预测输入)

### 8.1 信号流

```
IMU Acc (416Hz fresh)
    │
    ├── 去重力 + 世界投影
    │   accel_world_x = acc_body·world_x_vec
    │   accel_world_y = acc_body·world_y_vec
    │
    ├── Notch (固定 120Hz, 可选)
    │   filter_notch_apply(&notch_x, accel_x)
    │
    ├── BW2 LPF (30Hz)
    │   filter_bw2_lpf_apply(&lpf_x, notch_out)
    │
    ├── 死区 (0.25 m/s²)
    │   if (|accel| < 0.25) accel = 0;
    │
    └── 限幅 (±1.8 m/s²)
        accel = clip(accel, -1.8, 1.8);
```

### 8.2 参数表

| 参数宏 | 值 | 说明 |
|--------|-----|------|
| `OPTFLOW_IMU_PREDICT_ACC_CHAIN_FS_HZ` | 416 | Acc 链路更新频率 |
| `OPTFLOW_IMU_PREDICT_NOTCH_CENTER_HZ` | 120 | Notch 中心频率 |
| `OPTFLOW_IMU_PREDICT_NOTCH_BW_HZ` | 50 | Notch 带宽 |
| `OPTFLOW_IMU_PREDICT_LPF_HZ` | 30 | 二阶低通截止频率 |
| `OPTFLOW_IMU_PREDICT_ACC_STALE_S` | 0.010 | Acc 过期阈值 |
| `OPTFLOW_IMU_PREDICT_ACC_DEADBAND` | 0.25 | 死区 m/s² |
| `OPTFLOW_IMU_PREDICT_ACC_LIMIT` | 1.8 | 限幅 m/s² |

---

## 9. 速度源切换策略

### 9.1 源枚举

```c
typedef enum {
    CARLIGHT_VEL_SOURCE_OPTICAL_IMU = 0,    // 光流+IMU 二维KF估计器
    CARLIGHT_VEL_SOURCE_VISION_IMU_CF3 = 1  // 车灯视觉差分+IMU CF3互补
} carlight_velocity_source_t;
```

### 9.2 切换原因枚举

```c
typedef enum {
    NONE = 0,               // 无切换
    BAD_OPTICAL = 1,        // 光流低质量/失效 → 切到视觉
    NO_PROGRESS = 2,        // 位置无进展 → 切到视觉
    VISION_LOST = 3,        // 视觉丢失 → 切回光流
    OPTICAL_RECOVER = 4,    // 光流恢复稳定 → 切回光流
    MODE_EXIT = 5,          // 退出车灯模式
    MODE_SELECT = 6         // 用户速度环子模式选择
} carlight_velocity_switch_reason_t;
```

### 9.3 切换时序参数

| 参数宏 | 值 | 说明 |
|--------|-----|------|
| `CARLIGHT_OPTICAL_BAD_QUALITY_SCALE` | 0.25 | 光流质量低于此→坏 |
| `CARLIGHT_OPTICAL_RECOVER_QUALITY_SCALE` | 0.55 | 光流质量高于此→恢复 |
| `CARLIGHT_OPTICAL_LOWQ_HOLD_MS` | 80 | 低质量确认时间 |
| `CARLIGHT_OPTICAL_COAST_HOLD_MS` | 120 | COAST 确认时间 |
| `CARLIGHT_OPTICAL_RECOVER_HOLD_MS` | 500 | 恢复确认时间 |
| `CARLIGHT_OPTICAL_INNOV_BAD_MPS` | 0.60 | 创新异常阈值 |
| `CARLIGHT_PROGRESS_TIMEOUT_MS` | 800 | 位置无进展触发时间 |

---

## 10. 坐标系变换与杆臂补偿全览

### 10.1 关键坐标系

| 坐标系 | 说明 |
|--------|------|
| **传感器原始系** | IMU660RB: X右/Y前/Z上 |
| **机体坐标系 (Body)** | X前/Y右/Z下 (NED) |
| **世界坐标系 (World)** | X北/Y东/Z下 (NED) |
| **光流像素系** | 模块原始 DX/DY count |

### 10.2 IMU 轴映射

```c
// Gyro: 传感器 → 机体
body_gyro_x = +sensor_gyro_y;   // 传感器 Y(前) = 机体 X(前) 
body_gyro_y = +sensor_gyro_x;   // 传感器 X(右) = 机体 Y(右)
body_gyro_z = -sensor_gyro_z;   // 传感器 Z(上) = -机体 Z(下)

// Acc (重力观测符号): 传感器 → 机体
body_acc_x = -sensor_acc_y;     // 取反
body_acc_y = -sensor_acc_x;
body_acc_z = +sensor_acc_z;
```

### 10.3 双 IMU 杆臂位置

```c
IMU1: Y = -0.020m (中心左侧)
IMU2: Y = +0.020m (中心右侧)
```

### 10.4 光流安装偏移

```c
前光流 U23: X = +0.100m (前方),  Y = 0.000m
后光流 U24: X = -0.100m (后方),  Y = 0.000m
```

### 10.5 ToF 安装偏移

```c
ToF1 (U9):  X = +0.100m (前方)
ToF2 (U10): X = -0.100m (后方)
```

### 10.6 光流传感器速度 → 机体中心速度

```c
// 传感器安装点速度
sensor_vel_x = derot_rate_x * height / cos(tilt);  // 角速度 → 线速度
sensor_vel_y = derot_rate_y * height / cos(tilt);

// 转换到机体中心 (考虑传感器偏移和 yaw rate)
center_vel_x = sensor_vel_x - offset_y * gyro_z;  // 简化: offset_y=0
center_vel_y = sensor_vel_y + offset_x * gyro_z;
```

### 10.7 ToF 姿态补偿

```c
// 斜距投影到竖直 + X 杆臂补偿
height_mm = primary_mm * cos(tilt) + offset_x_mm * sin_component;
// cos(tilt) = world_z_from_body_z (投影缓存提供)
// sin_component = world_z_from_body_x (body X→world Z 投影)
```

---

## 11. 工程移植指南

### 11.1 最小依赖

要将本融合系统移植到新平台，需要：

1. **IMU 驱动**: 提供 gyro(rad/s) + acc(m/s²) 三轴数据，含时间戳
2. **ToF 驱动**: 提供距离(mm) + 有效标志
3. **光流驱动**: 提供像素计数 + 积分时间
4. **定时器**: 微秒级时间戳 `timing_get_us()`
5. **基础数学**: `sinf/cosf/sqrtf/atan2f`、`finite_f32`、`fclip`
6. **滤波器库**: `common_filter.c/h` (notch/BW2/PT1/PT2/PT3)

### 11.2 调度集成

```c
// 500us ISR
void fast_ctrl_500us() {
    imu_spi_kick();             // 触发 SPI 采集
    imu_snapshot_try_read();    // 读取最新快照
    imu_attitude_predict_500us(snapshot);  // 四元数预测
    optical_gyro_history_push(snapshot);   // Gyro 历史入环
    motor_rate_pid();           // 速率环 PID
}

// 1ms ISR
void estimator_1ms() {
    imu_ekf_acc_correct();      // EKF acc 校正
    imu_euler_update();         // 欧拉角更新
    tof_kf_predict(world_z_accel, 0.001, acc_trusted);  // 高度KF预测
    tof_kf_correct();           // 高度KF校正 (如有新观测)
    optical_accel_chain_push(); // 加速度链推进 (仅 fresh 帧)
    motor_attitude_pid();       // 姿态PID
    motor_height_speed_pid();   // 高度速度PID
}

// 4ms ISR
void outer_ctrl_4ms() {
    optical_sensor_update();    // 光流去旋 + 传感器速度
    optical_estimator_step();   // 二维速度KF
    carlight_velocity_update(); // 车灯CF3观测器
    motor_height_pos_pid();     // 高度位置PID
    motor_optflow_vel_pid();    // 光流速度PID
}
```

### 11.3 关键配置检查清单

| 检查项 | 宏/变量 | 需要确认 |
|--------|---------|---------|
| IMU 采样率 | `IMU_GYRO_SAMPLE_FREQ_HZ` | 必须匹配硬件 ODR |
| Acc 采样率 | `IMU_ACCEL_SAMPLE_FREQ_HZ` | 必须匹配硬件 ODR |
| EKF 过程噪声 | `IMU_EKF_Q_QUAT_CONT` | 根据振动环境调整 |
| EKF 观测噪声 | `IMU_EKF_R0` | 根据 acc 噪声水平调整 |
| 高度KF噪声 | `HEIGHT_KF_Q_H/VZ` | 根据飞行机动性调整 |
| 光流噪声 | `OPTFLOW_EST_MEAS_NOISE_MPS` | 根据光流精度调整 |
| 动态notch模型 | `IMU_GYRO_DYNAMIC_NOTCH_MODEL_*` | 必须本机 FFT 标定 |
| 去旋系数 | `FLOW_RATE_PER_RAD_S_*` | 必须本机标定 |
| ToF 杆臂 | `TOF_SENSOR*_OFFSET_X_MM` | 必须按安装位置填写 |
| 光流杆臂 | `FLOW_*_SENSOR_OFFSET_*_M` | 必须按安装位置填写 |

### 11.4 与 common_kf2.h 的关系

`common_kf2.h` 提供了两个可复用的纯数学原语：

```c
// 1. 协方差边界保护 (ToF和Optical共用)
void kf2_covariance_bound(p00, p01, p10, p11, diag_min, p00_max, p11_max);

// 2. 标量量测校正 H=[1,0] (Joseph形式, 支持增益限幅)
void kf2_scalar_correct(state0, state1, p00, p01, p10, p11,
                         innovation, innovation_var, meas_r,
                         k0_limit, k1_limit, &k0_out, &k1_out);
```

ToF 高度 KF 使用自己的内联校正（因为需要 hard innovation gate），
Optical 速度 KF 也使用自己的校正（因为需要质量缩放和状态机），
但两者都调用 `kf2_covariance_bound` 做协方差数值保护。

---

> **文档版本**: v1.0  
> **关联文件**: `sensor/sensor_imu.c`, `sensor/sensor_tof.c`, `sensor/sensor_optical.c`, `sensor/sensor_carlight_velocity_observer.c`, `common/common_kf2.h`
