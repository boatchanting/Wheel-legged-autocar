# IMU 滤波技术报告与开发文档

> **工程**: Seekfree CYT4BB 飞控 (CM7_0)  
> **硬件**: 双 IMU660RB (ICM42688兼容), SPI 30MHz DMA  
> **传感器频率**: Gyro 2kHz / Acc 416Hz  
> **生成日期**: 2026-07-22  
> **编码**: GB2312

---

## 目录

1. [系统概述](#1-系统概述)
2. [IMU 硬件架构](#2-imu-硬件架构)
3. [零点标定系统](#3-零点标定系统)
4. [Gyro 多路径滤波器链](#4-gyro-多路径滤波器链)
5. [Acc 滤波器链](#5-acc-滤波器链)
6. [动态 Notch 系统](#6-动态-notch-系统)
7. [FFT 频谱分析与后台服务](#7-fft-频谱分析与后台服务)
8. [温度漂移补偿](#8-温度漂移补偿)
9. [在线 Gyro Bias 学习](#9-在线-gyro-bias-学习)
10. [四元数 EKF 姿态估计](#10-四元数-ekf-姿态估计)
11. [滤波器库完整清单](#11-滤波器库完整清单)
12. [开发指南与调优建议](#12-开发指南与调优建议)

---

## 1. 系统概述

### 1.1 设计哲学

本 IMU 滤波系统遵循以下核心原则：

1. **多路径分工**: Gyro 经过同一 notch 链后分流为 4 条独立路径（EKF预测、速率P/I、速率D、姿态阻尼），各路径使用不同截止频率的低通滤波器，以平衡响应速度与噪声抑制。
2. **噪声前置切除**: Notch（陷波）放在滤波器链最前端，先把最窄带的高能电机噪声切掉，后续低通只需处理残余宽带噪声。
3. **动态自适应**: 动态 Notch 中心频率随油门实时调整，FFT 后台确认谐波槽，温漂补偿随温度变化。
4. **安全第一**: 所有浮点操作经过 `finite_f32` 检查，所有输出经过 `fclip` 限幅，零偏学习有严格静止门控。

### 1.2 坐标系约定

```
机体坐标系 (NED):
  X: 机头正前方 (Forward)
  Y: 机体右侧   (Right)
  Z: 机体正下方 (Down)

IMU 安装: 传感器 X→机体右, 传感器 Y→机体前, 传感器 Z→机体上
Gyro 映射: body X=+sensor Y, body Y=+sensor X, body Z=-sensor Z
Acc 映射:  body X=-sensor Y, body Y=-sensor X, body Z=+sensor Z (重力观测符号)
```

### 1.3 关键宏参数速查

| 参数 | 值 | 说明 |
|------|-----|------|
| `IMU_GYRO_SAMPLE_FREQ_HZ` | 2000 | Gyro 采样率 |
| `IMU_ACCEL_SAMPLE_FREQ_HZ` | 416 | Acc 采样率 |
| `IMU_SPI_TIMEOUT_US` | 350 | SPI 超时保护 |
| `IMU_STALE_LIMIT_US` | 2500 | Gyro 过期阈值 |
| `IMU_DUAL_ENABLE` | 1 | 双 IMU 融合开关 |

---

## 2. IMU 硬件架构

### 2.1 双 IMU 配置

```
IMU1 (P15/SCB9, INT1=P19_2): 机体中心左侧 0.020m
IMU2 (P10/SCB4, INT1=P10_4): 机体中心右侧 0.020m
```

双 IMU 融合模式 (`imu_dual_runtime_struct.fusion_mode`):
- `IMU_DUAL_FUSION_MODE_NONE` (0): 无融合
- `IMU_DUAL_FUSION_MODE_FRONT` (1): 仅 IMU1
- `IMU_DUAL_FUSION_MODE_REAR` (2): 仅 IMU2
- `IMU_DUAL_FUSION_MODE_DUAL` (3): 双路融合（默认）

### 2.2 异步 SPI 采集

采用异步 SPI Burst 模式，17 字节传输（1 地址 + 16 payload）：

```
PIT 500us 触发 kick → SPI 异步传输 → 完成 ISR 解析 → 发布快照
```

关键保护机制：
- **超时保护**: 350us 超时后 abort 并释放 CS
- **Overrun 检测**: BUSY 状态下重复 kick 计数
- **完成 ISR 耗时统计**: 追踪 SPI 完成中断处理耗时
- **传输错误分类**: ACTIVE / IRQ_MISS / ERROR_STATUS 三级分类
- **温度读取**: 从 payload 解析 IMU660RB 温度寄存器（25°C 基准, 256 LSB/°C）

### 2.3 快照双缓冲交换

```c
typedef struct {
    imu_snapshot_struct slot[2];        // 双缓冲
    volatile uint8 active_index;        // 当前可读槽
    volatile uint32 generation;         // 发布代数
} imu_snapshot_exchange_struct;
```

- 生产者（SPI 完成 ISR）写 inactive 槽，DMB 后切换 active_index
- 消费者（500us/1ms ISR）读 active 槽，通过 generation 检测一致性
- `imu_publish_work_snapshot` 放在静态区以降低完成 ISR 栈占用

### 2.4 健康故障位图

| 故障位 | 含义 |
|--------|------|
| `IMU_HEALTH_FAULT_TIMEOUT` | SPI busy 超时 |
| `IMU_HEALTH_FAULT_TRANSFER` | SPI SDK 传输错误 |
| `IMU_HEALTH_FAULT_PARSE` | Payload 解析异常 |
| `IMU_HEALTH_FAULT_NO_FRESH_GYRO` | 无 fresh gyro |
| `IMU_HEALTH_FAULT_STALE` | Gyro 超龄 |
| `IMU_HEALTH_FAULT_SAMPLE_INTERVAL` | 采样间隔越界 |
| `IMU_HEALTH_FAULT_DUAL_PRIMARY` | 主 IMU 不可用 |
| `IMU_HEALTH_FAULT_DUAL_SECONDARY` | 副 IMU 不可用 |
| `IMU_HEALTH_FAULT_DUAL_DISAGREE` | 双 IMU 不一致 |
| `IMU_HEALTH_FAULT_FIFO_OVERRUN` | FIFO 溢出 |
| `IMU_HEALTH_FAULT_RAW_STUCK` | 数据冻结 |
| `IMU_HEALTH_FAULT_RAW_SATURATION` | 量程饱和 |
| `IMU_HEALTH_FAULT_ATTITUDE_INVALID` | 四元数损坏 |

---

## 3. 零点标定系统

### 3.1 两阶段标定流程

```
阶段 1: Gyro/Acc Bias 均值
  ├── 静止门控: |acc_xy|<0.08g, |acc_norm-1g|<0.05g, acc_z>0.5g, gyro<0.08rad/s
  ├── 连续静止满 500 样本 (IMU_ZERO_CALIBRATION_SAMPLES_DEFAULT)
  ├── 取平均 → gyro_bias / acc_bias (acc Z 轴 -1g)
  ├── 双 IMU 分别记录与融合中心的 delta
  └── 锁存 gyro_bias_temp_degc (供温漂补偿参考)

阶段 2: 姿态角零点 (bias 完成后)
  ├── 等待 500 样本 EKF 收敛 (IMU_ZERO_ATTITUDE_SETTLE_SAMPLES ≈ 1.2s)
  ├── 圆周平均 (sin/cos 累计, atan2 恢复) 避免 ±180° 边界跳变
  └── 输出 attitude_zero (roll/pitch/yaw)
```

### 3.2 静止门控条件

```c
static uint8 imu_zero_sample_is_static(gyro, accel) {
    if (|acc_x_g| >= 0.08g) return 0;
    if (|acc_y_g| >= 0.08g) return 0;
    if (|acc_norm - 1g| >= 0.05g) return 0;
    if (acc_z_g <= 0.5g) return 0;  // 排除倒置
    if (gyro_norm >= 0.08rad/s) return 0;
    return 1;
}
```

### 3.3 运动检测与重置策略

- **阶段 1 运动**: 清空 gyro/acc bias 窗口，要求重新连续静止满窗口
- **阶段 2 运动**: 保留已完成的 bias，只重置姿态角均值，等重新静止后重新收敛

### 3.4 双 IMU 零偏差分

标定完成时保存每颗 IMU 相对融合中心 bias 的差值，供单路退化时扣除：

```c
imu_runtime.instance[i].zero.gyro_delta_rad_s  // 单颗 gyro delta
imu_runtime.instance[i].zero.accel_delta_m_s2  // 单颗 acc delta
```

---

## 4. Gyro 多路径滤波器链

### 4.1 滤波器链架构

```
原始 Gyro (2kHz)
    │
    ├── [减静态bias] ──→ gyro_raw_rad_s (诊断用)
    │
    ├── [减在线bias] ──→ gyro_unfiltered_rad_s (诊断对照)
    │
    ├── [温漂补偿]     (IMU_TEMP_COMP_ENABLE=1)
    │
    ▼
┌─────────────────────────────────────────────────────┐
│ 动态 Notch (TPT-SVF) — 2kHz                          │
│   Slot 0: 基频 (油门快前馈, 55-280Hz)                │
│   Slot 1: 2次谐波 (FFT 确认后启用, ~12-35Hz BW)      │
│   Slot 2: 3次谐波 (FFT 确认后启用, ~10-35Hz BW)      │
│   后台重建时: gyro_dynamic_notch_rebuild_active=1    │
│   → ISR 临时旁路动态 notch                           │
├─────────────────────────────────────────────────────┤
│ 静态 Notch (直接型 IIR) — 2kHz                       │
│   默认关闭 (IMU_GYRO_STATIC_NOTCH_ENABLE=0)          │
│   默认中心 135Hz, 带宽 35Hz                          │
└────────────────────┬────────────────────────────────┘
                     │
          gyro_ekf_rad_s (EKF 预测用, 无额外低通)
                     │
         ┌───────────┼───────────┬──────────────────┐
         ▼           ▼           ▼                  ▼
    ┌─────────┐ ┌─────────┐ ┌─────────┐    ┌──────────┐
    │BW2 LPF  │ │PT2 LPF  │ │PT1 LPF  │    │gyro_ekf  │
    │130Hz    │ │50-70Hz  │ │60Hz     │    │(直通)    │
    │固定     │ │动态油门 │ │固定     │    │          │
    └────┬────┘ └────┬────┘ └────┬────┘    └────┬─────┘
         ▼           ▼           ▼              ▼
    rate_pi     rate_d      attitude         EKF预测
    (P/I项)     (D项)       (姿态阻尼)
```

### 4.2 滤波器链实现细节

```c
static void imu_filters_apply_gyro(gyro_input, snapshot) {
    for (axis = 0; axis < 3; axis++) {
        // Step 1: 动态 Notch (级联 3 槽 TPT-SVF)
        dynamic_out = input[axis];
        if (rebuild_active == 0) {  // 后台重建时旁路
            for (slot = 0; slot < 3; slot++)
                dynamic_out = filter_svf_notch_apply(&notch[slot][axis], dynamic_out);
        }

        // Step 2: 静态 Notch (直接型 IIR)
        static_out = filter_notch_apply(&gyro_static_notch[axis], dynamic_out);

        // Step 3: 四路分流
        gyro_ekf[axis]     = static_out;                           // EKF 预测 (无低通)
        gyro_rate_pi[axis] = filter_bw2_lpf_apply(&lpf[axis], static_out);  // P/I 130Hz
        gyro_rate_d[axis]  = filter_pt2_apply(&d_lpf[axis], static_out);     // D 50-70Hz
        gyro_attitude[axis]= filter_pt1_apply(&att_lpf[axis], static_out);   // 姿态 60Hz
    }
}
```

### 4.3 各路径参数配置

| 路径 | 滤波器类型 | 截止频率 | 采样率 | 用途 |
|------|-----------|---------|--------|------|
| gyro_ekf | 无低通 (仅notch) | — | 2kHz | EKF 四元数预测 |
| gyro_rate_pi | BW2 LPF | 130Hz | 2kHz | 速率环 P/I 项 |
| gyro_rate_d | PT2 LPF | 50-70Hz 动态 | 2kHz | 速率环 D 项 |
| gyro_attitude | PT1 LPF | 60Hz | 2kHz | 姿态外环阻尼 |

### 4.4 D 链动态低通

D 项低通截止频率随油门动态调整，使用 BF 风格曲线：

```c
// 油门越高 → 截止频率越高 (噪声容忍度提高)
shaped_throttle = x*(1-x)+x;  // BF 风格曲线
target_cutoff = min_cutoff + (max_cutoff - min_cutoff) * shaped_throttle;
// 范围: 50Hz (低油门) → 70Hz (高油门)
```

更新死区 0.5Hz，避免频繁重算 PT2 增益。

### 4.5 Notch 残差诊断

```c
// 供 VOFA 判断 notch 是否实际作用
gyro_notch_residual = input - gyro_ekf;     // notch 链滤除分量
gyro_lpf_residual   = gyro_ekf - gyro_rate_pi; // 低通滤除分量
```

---

## 5. Acc 滤波器链

### 5.1 滤波器链架构

```
原始 Acc (416Hz, 仅 fresh 帧)
    │
    ├── [减静态bias]
    │
    ▼
┌──────────────────────────────┐
│ Acc Notch (直接型 IIR)       │
│   中心 180Hz, 带宽 45Hz      │
│   默认开启                    │
├──────────────────────────────┤
│ BW2 LPF                      │
│   20Hz                        │
└──────────────┬───────────────┘
               │
          accel_ekf_m_s2 (EKF correct 用)
```

### 5.2 Acc 推进策略

Acc 滤波器链**只在 fresh acc 样本到达时推进**（416Hz），旧样本不重复推进滤波状态。这避免了：
- 滤波状态因重复样本而过阻尼
- EKF correct 使用过时 acc 数据

### 5.3 Acc Trust 门控

EKF correct 前计算综合信任度：

```c
trust = trust_norm * trust_innov * trust_xy

trust_norm: |acc_norm-1g| 映射, 软 0.03g → 硬 0.18g
trust_innov: 三轴创新模长映射, 软 0.035 → 硬 0.22
trust_xy:    水平创新模长映射, 软 0.030 → 硬 0.20
```

trust < 0.08 时跳过本帧 correct。

---

## 6. 动态 Notch 系统

### 6.1 总体架构

```
油门归一化 (motor.c 提供)
    │
    ▼
┌──────────────────────────────────────┐
│ 油门平滑 (alpha=0.20, 1ms更新)       │
│ throttle_smooth += 0.20*(raw-smooth)  │
└──────────────┬───────────────────────┘
               │
    ┌──────────┴──────────┐
    ▼                     ▼
┌──────────────┐   ┌──────────────────┐
│ 油门前馈模型  │   │ FFT 谐波确认      │
│ 基频目标      │   │ 2x/3x 谐波槽      │
│ 35+247*油门  │   │ 连续2帧确认/丢失  │
│ 55-280Hz限幅 │   │ 容差 8Hz          │
└──────┬───────┘   └────────┬─────────┘
       │                    │
       ▼                    ▼
┌──────────────────────────────────────┐
│ 4ms 高频路径: 只写目标频率            │
│ 后台慢路径: 限速重建 IIR 系数         │
│   - 最短间隔 20ms                     │
│   - 最大步进 8Hz/次                   │
│   - preserve_state=1 (不清历史)       │
│   - rebuild_active 标志保护 ISR       │
└──────────────────────────────────────┘
```

### 6.2 油门模型

来自 2026-06-27 VOFA FFT 地面油门扫描和飞行悬停复核：

```c
target_hz = 35.0 + 247.0 * throttle_norm  // Hz
// 限幅: [55, 280] Hz
```

### 6.3 FFT 基频修正

FFT 后台估计油门模型残差，慢速修正基频：

```c
#if IMU_FFT_BASE_CORRECTION_ENABLE
    target_hz += imu_fft_base_correction_hz;  // 慢速残差叠加
    // 修正限幅: ±18Hz
    // 更新系数: alpha=0.25
    // 衰减系数: 0.90 (无匹配时回退)
    // 死区: 0.25Hz
#endif
```

### 6.4 谐波槽管理

| 槽位 | 谐波 | 启用条件 | 带宽策略 |
|------|------|---------|---------|
| Slot 0 | 1x 基频 | 始终启用（油门模型） | 低频宽45Hz → 高频窄36Hz |
| Slot 1 | 2x 谐波 | FFT 连续2帧确认 | center*8%, [12,35]Hz |
| Slot 2 | 3x 谐波 | FFT 连续2帧确认 | center*6%, [10,35]Hz |

谐波确认/丢失逻辑：
```c
#define IMU_FFT_HARMONIC_CONFIRM_FRAMES 2  // 连续确认帧数
#define IMU_FFT_HARMONIC_MISS_FRAMES    2  // 连续丢失帧数
#define IMU_FFT_HARMONIC_TOLERANCE_HZ   8  // 频点容差
#define IMU_FFT_HARMONIC_MIN_MAG_RATIO  0.18 // 最小相对幅值
```

### 6.5 后台重建保护

动态 notch 系数重建在后台执行，使用 `rebuild_active` 标志保护 ISR：

```
后台: rebuild_active = 1; DMB; 修改 notch 对象; DMB; rebuild_active = 0;
ISR:  if (rebuild_active == 0) { 执行动态 notch; } else { 旁路; }
```

### 6.6 带宽自适应

基频槽带宽随频率动态调整：
```c
freq_norm = (center_hz - 55) / (280 - 55);  // 归一化
bw = 45 + (36 - 45) * freq_norm;           // 低频45Hz → 高频36Hz
```

---

## 7. FFT 频谱分析与后台服务

### 7.1 FFT 参数

| 参数 | 值 | 说明 |
|------|-----|------|
| FFT 点数 | 2048 | 实数 FFT |
| 采样率 | 1000Hz | 降采样采集 |
| 频率分辨率 | ~0.49Hz/bin | |
| 峰值搜索范围 | 50-450Hz | `IMU_FFT_PEAK_MIN/MAX_HZ` |
| 采集源 | Gyro 或 Accel | `g_flight_mode_config.imu_fft_mode` |

### 7.2 采集模式

- `IMU_FFT_MODE_GYRO` (0): 采集 gyro 数据
- `IMU_FFT_MODE_ACCEL` (1): 采集 accel 数据

通过 `IMU_FFT_RUNTIME_ENABLE_DEFAULT` 控制默认使能。

### 7.3 后台服务调度

FFT 计算在后台主循环执行（非 ISR），每次传入时间预算：

```c
// SPI Complete ISR → 压入 FFT 样本缓冲区
// 后台: imu_fft_service_background(budget_us)
//   预算内完成采集和分析
//   峰值搜索 → 谐波确认 → 基频修正
```

诊断统计：
- `fft_service_last_us`: 最近耗时
- `fft_service_max_us`: 最大耗时
- `fft_service_avg_us`: 平均耗时 (IIR, shift=6)
- `fft_service_count`: 执行次数
- `fft_service_over_budget_count`: 超预算次数

---

## 8. 温度漂移补偿

### 8.1 离线拟合流程

```c
#define IMU_TEMP_DRIFT_WINDOW_SAMPLES 256  // 静止窗口样本数

// 每个完整静止窗口形成一个拟合样本点:
//   - 平均温度 (degC)
//   - 平均原始 gyro (rad/s, 三轴)
// VOFA CSV 导出后离线拟合线性模型
```

### 8.2 在线补偿

```c
#define IMU_TEMP_COMP_ENABLE 1

// 参考温度: 零点标定时的 IMU 温度
delta_temp = current_temp - bias_temp;
correction = slope * delta_temp;  // 限幅 ±0.05 rad/s
gyro_zeroed -= correction;
```

### 8.3 当前拟合系数

来自 24.57~31.09°C 静置窗口鲁棒拟合：

| 轴 | 斜率 (rad/s/°C) |
|-----|-----------------|
| X | +0.00111208 |
| Y | -0.00017554 |
| Z | -0.00004176 |

补偿绝对限幅: ±0.05 rad/s（防止错误系数污染控制链）。

---

## 9. 在线 Gyro Bias 学习

### 9.1 静止学习（默认开启）

```c
#define IMU_ONLINE_BIAS_TAU_S       8.0f    // 时间常数 8s
#define IMU_ONLINE_BIAS_LIMIT_RAD_S 0.08f   // 绝对限幅

// 条件:
//   - 电机停转 (online_bias_motor_inactive=1)
//   - 飞控锁定或安全停机 (online_bias_disarmed=1)
//   - |acc_norm - 1g| < 0.02g (静止)
//   - 学习三轴 (含 yaw)
```

### 9.2 飞行学习（默认关闭）

```c
#define IMU_FLIGHT_GYRO_BIAS_ENABLE 0       // 默认关闭
#define IMU_FLIGHT_GYRO_BIAS_TAU_S  60.0f    // 时间常数 60s
#define IMU_FLIGHT_GYRO_BIAS_LIMIT_RAD_S 0.035f

// 条件:
//   - acc trust > 0.95
//   - gyro 模长 < 0.10 rad/s
//   - 油门变化率 < 0.015/拍
//   - 连续满足 1000ms
//   - 只学习 roll/pitch (不含 yaw)
```

### 9.3 学习算法

```c
static void imu_gyro_bias_learn_step(bias, target, dt_s, tau_s, limit, learn_z) {
    gain = dt / (tau + dt);  // 无指数一阶步长
    bias += gain * (target - bias);
    bias = clip(bias, -limit, limit);
}
```

---

## 10. 四元数 EKF 姿态估计

### 10.1 EKF 架构

```
状态: 四元数 [w, x, y, z] (4 维固定)
协方差: 4×4 上三角存储 (10 个元素)

预测 (500us):
  q_pred = q + 0.5 * dt * Ω(gyro) * q
  归一化

协方差预测 (1ms, 合并 2 个 500us 步):
  P = F * P * F' + Q*dt
  Q = 2e-5 * dt (IMU_EKF_Q_QUAT_CONT)
  F 由 gyro 步长线性化

校正 (1ms, 仅 fresh acc):
  观测模型: z = [0, 0, 1]' (重力方向)
  创新: innov = z_pred - z_meas
  S = H*P*H' + R
  K = P*H' / S
  q = q + K * innov
  P = (I-KH) * P * (I-KH)' + K*R*K' (Joseph 形式)
```

### 10.2 Acc Trust 门控

```c
trust = ramp_down(|acc-1g|, 0.03g, 0.18g)    // 模长信任
      * ramp_down(innov_norm, 0.035, 0.22)    // 三轴创新信任
      * ramp_down(innov_xy, 0.030, 0.20);     // 水平创新信任

if (trust < 0.08) skip_correct();             // 跳过低信任帧
```

### 10.3 自适应 R

```c
R_eff = R0 * (1 + 30 * |acc_norm - 1g|);  // R0=0.004, 放大系数 30
R_eff = clip(R_eff, 0.001, 0.08);         // (正常模式)
R_eff = clip(R_eff, 0.001, 0.80);         // (动态模式)
```

### 10.4 Chi2 门控

```c
#define IMU_EKF_CHI2_GATE 9.0  // ~3σ

if (innov²/S > 9.0) {
    R *= 50;  // 膨胀 R 降低该轴权重
    reject_count[axis]++;
}
```

### 10.5 动态保持策略

| 模式 | 保持时间 | 触发条件 |
|------|---------|---------|
| Soft | 0ms | trust 略低，只通过 R 降权 |
| Hard | 30ms | trust 显著降低 |
| Severe | 90ms | trust 极低或连续拒绝 |

### 10.6 数值保护

- 四元数范数检查: [0.5, 2.0]，越界触发恢复
- 协方差对角线: [1e-7, 1.0]，越界触发恢复
- Cholesky 求解 pivot 下限: 1e-8
- 协方差对称化 + 交叉项限幅
- Joseph 形式更新（对增益限幅鲁棒）

### 10.7 投影缓存 (1ms 更新)

```c
// 预计算三角函数，避免控制链重复计算
world_z_from_body_x = 2*(q.x*q.z - q.w*q.y)  // body X → world Z
world_z_from_body_y = 2*(q.y*q.z + q.w*q.x)  // body Y → world Z
world_z_from_body_z = 1-2*(q.x²+q.y²)        // = cos_tilt
world_z_accel = acc·proj - 9.81              // 世界 Z 线加速度
```

---

## 11. 滤波器库完整清单

`common_filter.c/h` 提供以下滤波器类型：

### 11.1 一阶滤波器

| 滤波器 | 结构体 | 特点 | 典型用途 |
|--------|--------|------|---------|
| **LPF1** | `filter_lpf1_t` | 一阶低通, 1次乘加/拍 | 电池电压, 目标平滑 |
| **HPF1** | `filter_hpf1_t` | 一阶高通, 去直流 | 提取变化量 |
| **PT1** | `filter_pt1_t` | Betaflight 风格 PT1 | 姿态阻尼 gyro |
| **Simple LPF (i32)** | `filter_simple_lpf_i32_t` | 定点低通, 无浮点 | 低速变量 |
| **Mean Acc** | `filter_mean_acc_t` | 整数均值累加器 | 简单均值 |

### 11.2 级联滤波器

| 滤波器 | 结构体 | 特点 | 典型用途 |
|--------|--------|------|---------|
| **PT2** | `filter_pt2_t` | 两级 PT1 级联 (-3dB 修正) | Rate D gyro |
| **PT3** | `filter_pt3_t` | 三级 PT1 级联 | 需要更强抑制 |

### 11.3 二阶 IIR 滤波器

| 滤波器 | 结构体 | 离散化 | 特点 |
|--------|--------|--------|------|
| **Biquad** | `filter_biquad_t` | 通用直接型 | 共享延迟线推进 |
| **Biquad LPF** | `filter_biquad_lpf_t` | 双二阶 + Butterworth Q | Q=0.707 |
| **BW2 LPF** | `filter_bw2_lpf_t` | 双线性变换 | 兼容旧工程 |
| **Notch** | `filter_notch_t` | 直接型陷波 | 固定频点抑制 |

### 11.4 Betaflight TPT-SVF 滤波器

| 滤波器 | 结构体 | 特点 | 典型用途 |
|--------|--------|------|---------|
| **SVF LPF** | `filter_svf_lpf_t` | 数值稳定性好 | 替代直接型 |
| **SVF Notch** | `filter_svf_notch_t` | 支持在线调谐 | 动态 notch |

### 11.5 其他滤波器

| 滤波器 | 结构体 | 特点 |
|--------|--------|------|
| **Phase Comp** | `filter_phase_comp_t` | 固定频段相位补偿 |
| **Moving Avg** | `filter_mavg_t` | O(1) 滑动平均 |

### 11.6 核心 API 模式

所有滤波器遵循统一模式：

```c
// 初始化 (一次性计算系数)
void filter_xxx_init(filter_xxx_t *f, float sample_freq, float cutoff);

// 高频推进 (只做状态更新)
float filter_xxx_apply(filter_xxx_t *f, float input);

// 状态管理
void filter_xxx_reset(filter_xxx_t *f);
void filter_xxx_state_set(filter_xxx_t *f, float value);
```

### 11.7 截止频率安全限幅

```c
// 所有滤波器初始化时统一限幅: 最高为 Nyquist * 0.475
static float filter_limit_cutoff(sample_freq_hz, cutoff_hz) {
    nyquist_safe = sample_freq_hz * 0.475f;
    return min(cutoff_hz, nyquist_safe);
}
```

---

## 12. 开发指南与调优建议

### 12.1 添加新滤波器路径

1. 在 `imu_filter_bank_struct` 中添加新滤波器实例
2. 在 `imu_filters_init()` 中初始化
3. 在 `imu_filters_apply_gyro/accel()` 中推进
4. 在 `imu_snapshot_struct` 中发布输出（可选）
5. 在 `imu_filters_prime_gyro/accel()` 中支持状态灌入

### 12.2 修改动态 Notch 策略

关键配置宏（`sensor_imu.c` 顶部）：

```c
IMU_GYRO_DYNAMIC_NOTCH_ENABLE       // 总开关
IMU_GYRO_DYNAMIC_NOTCH_MIN_HZ       // 最低频率 (保护低频带宽)
IMU_GYRO_DYNAMIC_NOTCH_MAX_HZ       // 最高频率
IMU_GYRO_DYNAMIC_NOTCH_MODEL_BASE_HZ // 油门模型截距
IMU_GYRO_DYNAMIC_NOTCH_MODEL_GAIN_HZ // 油门模型斜率
IMU_GYRO_DYNAMIC_NOTCH_ALPHA         // 油门平滑系数
IMU_DYNAMIC_NOTCH_REBUILD_PERIOD_MS  // 后台重建间隔
IMU_DYNAMIC_NOTCH_MAX_STEP_HZ        // 单次最大步进
IMU_FFT_BASE_CORRECTION_ENABLE       // FFT 基频修正开关
```

### 12.3 调优建议

1. **Notch 带宽**: 带宽越宽抑制越强但相位损失越大。悬停区可适度放宽（45Hz），高油门区应收窄（36Hz）。
2. **D 链低通**: 50-70Hz 动态范围，D 项对噪声敏感但需要相位响应。如果 D 项抖动大，降低 max_cutoff。
3. **EKF Q/R**: 
   - Q=2e-5: 增大可加快姿态收敛但更易受振动影响
   - R0=0.004: 减小更信任 acc 但动态时姿态漂移
4. **Acc Trust**: 当前门控较保守 (trust<0.08 跳过)，如需更激进 correct 可降低 `IMU_ACC_TRUST_MIN_FOR_CORRECT`
5. **温漂补偿**: 当前斜率来自窄温区拟合 (24.6-31.1°C)，宽温区飞行前需重新拟合
6. **FFT 谐波**: `IMU_FFT_HARMONIC_MIN_MAG_RATIO=0.18`，噪声环境中可适当提高以避免误检

### 12.4 VOFA 调试入口

| VOFA 模式 | 内容 |
|-----------|------|
| VOFA_MODE_IMU | IMU 原始/滤波对照、EKF 状态 |
| VOFA_MODE_FFT | FFT 频谱瀑布图 |
| VOFA_MODE_TIMING | 各 ISR/后台耗时统计 |
| VOFA_MODE_TUNING_CASCADE | 级联 PID 在线调参 |

诊断通道关键字段：
- `gyro_notch_residual_rad_s`: notch 实际滤除量
- `gyro_lpf_residual_rad_s`: 低通实际滤除量
- `gyro_dynamic_notch_center_hz`: 基频 notch 当前中心频率
- `last_acc_trust`: 最近 acc 信任度
- `last_ekf_r_acc`: 最近 EKF 实际 R 值

---

> **文档版本**: v1.0  
> **适用范围**: Seekfree CYT4BB CM7_0 飞控固件  
> **关联文件**: `sensor/sensor_imu.c`, `sensor/sensor_imu.h`, `common/common_filter.c`, `common/common_filter.h`
