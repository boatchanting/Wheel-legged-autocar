# IMU 滤波技术报告与开发文档 (极致细节版)

> **工程**: Seekfree CYT4BB 飞控 (CM7_0)  
> **硬件**: 双 IMU660RB (ICM-42688-P 兼容), SPI 30MHz DMA  
> **MCU**: Infineon CYT4BB7 Cortex-M7 @ 250MHz  
> **传感器频率**: Gyro 2kHz / Acc 416Hz  
> **生成日期**: 2026-07-22  
> **编码**: GB2312

---

## 目录

1. [系统概述与完整参数速查](#1-系统概述与完整参数速查)
2. [IMU 硬件架构与 SPI 异步采集](#2-imu-硬件架构与-spi-异步采集)
3. [原始数据到机体系的完整转换链](#3-原始数据到机体系的完整转换链)
4. [零点标定系统 (两阶段 + 双IMU差分)](#4-零点标定系统)
5. [Gyro 多路径滤波器链 (完整数据流)](#5-gyro-多路径滤波器链)
6. [Acc 滤波器链](#6-acc-滤波器链)
7. [动态 Notch 系统 (完整实现)](#7-动态-notch-系统)
8. [TPT-SVF 陷波滤波器数学推导](#8-tpt-svf-陷波滤波器数学推导)
9. [FFT 频谱分析与后台服务](#9-fft-频谱分析与后台服务)
10. [温度漂移补偿 (离线拟合 + 在线应用)](#10-温度漂移补偿)
11. [在线 Gyro Bias 学习 (静止 + 飞行)](#11-在线-gyro-bias-学习)
12. [四元数 EKF 姿态估计 (完整数学推导)](#12-四元数-ekf-姿态估计)
13. [滤波器库完整实现清单](#13-滤波器库完整实现清单)
14. [健康监控与故障诊断体系](#14-健康监控与故障诊断体系)
15. [VOFA 调试入口与诊断通道](#15-vofa-调试入口与诊断通道)
16. [开发指南与完整调参手册](#16-开发指南与完整调参手册)

---

## 1. 系统概述与完整参数速查

### 1.1 设计哲学

1. **多路径分工**: Gyro 经同一 notch 链后分流为 4 条独立路径 (EKF预测/速率P-I/速率D/姿态阻尼)，各路径用不同截止频率低通，平衡响应速度与噪声抑制
2. **噪声前置切除**: Notch 放在滤波器链最前端，先把最窄带高能电机噪声切掉，后续低通只需处理残余宽带噪声
3. **动态自适应**: 动态 Notch 中心频率随油门实时调整，FFT 后台确认谐波槽，温漂补偿随温度变化
4. **安全第一**: 所有浮点操作经 `finite_f32` 检查，所有输出经 `fclip` 限幅，零偏学习有严格静止门控

### 1.2 坐标系约定

```
机体坐标系 (NED): X前/Y右/Z下
IMU 安装: 传感器 X→机体右, Y→机体前, Z→机体上

Gyro 映射: body X=+sensor Y, body Y=+sensor X, body Z=-sensor Z
Acc 映射:  body X=-sensor Y, body Y=-sensor X, body Z=+sensor Z (重力观测符号)
```

### 1.3 SPI 采集参数

| 宏 | 值 | 说明 |
|----|-----|------|
| `IMU_SPI_BURST_LENGTH` | 17 | SPI burst 字节数 (1地址+16payload) |
| `IMU_SPI_TIMEOUT_US` | 350 | 异步 SPI 最长等待时间 |
| `IMU_SPI_SAMPLE_OFFSET_US` | 8 | 样本时间戳相对 kick 的经验补偿 |
| `IMU_SPI_TRANSFER_AVG_SHIFT` | 6 | SPI 耗时 IIR shift (~32ms 时间常数) |
| `IMU_DUAL_ENABLE` | 1 | 双 IMU 融合开关 |

### 1.4 频率与周期

| 宏 | 值 | 说明 |
|----|-----|------|
| `IMU_GYRO_SAMPLE_FREQ_HZ` | 2000.0 | Gyro 消费频率 |
| `IMU_ACCEL_SAMPLE_FREQ_HZ` | 416.0 | Acc 消费频率 (仅 fresh 帧) |
| `IMU_FAST_STEP_DT_S` | 0.0005 | 500us 快环步长 |
| `IMU_ESTIMATOR_STEP_DT_S` | 0.001 | 1ms 估计器步长 |
| `IMU_STALE_LIMIT_US` | 2500 | Gyro 过期阈值 |
| `IMU_GYRO_INTERVAL_MIN_US` | 250 | Gyro fresh 最小可信间隔 |
| `IMU_GYRO_INTERVAL_MAX_US` | 1000 | Gyro fresh 最大可信间隔 |

### 1.5 零偏标定参数

| 宏 | 值 | 说明 |
|----|-----|------|
| `IMU_ZERO_CALIBRATION_SAMPLES_DEFAULT` | 500 | 默认标定样本数 |
| `IMU_STATIC_ACC_XY_LIMIT_G` | 0.08 | 水平门限 |
| `IMU_STATIC_ACC_NORM_LIMIT_G` | 0.05 | 模长偏差门限 |
| `IMU_STATIC_ACC_Z_MIN_G` | 0.5 | Z 轴最小重力 (排除倒置) |
| `IMU_STATIC_GYRO_LIMIT_RAD_S` | 0.08 | Gyro 模长门限 |
| `IMU_ZERO_ATTITUDE_SETTLE_SAMPLES` | 500 | 姿态收敛等待 (~1.2s@416Hz) |

### 1.6 滤波器参数

| 宏 | 值 | 说明 |
|----|-----|------|
| `IMU_GYRO_STATIC_NOTCH_CENTER_HZ` | 135.0 | 静态 notch 中心 |
| `IMU_GYRO_STATIC_NOTCH_BW_HZ` | 35.0 | 静态 notch 带宽 |
| `IMU_GYRO_STATIC_NOTCH_ENABLE` | 0 | 静态 notch 默认关闭 |
| `IMU_GYRO_DYNAMIC_NOTCH_ENABLE` | 1 | 动态 notch 默认开启 |
| `IMU_GYRO_DYNAMIC_NOTCH_MIN_HZ` | 55.0 | 动态 notch 最低 |
| `IMU_GYRO_DYNAMIC_NOTCH_MAX_HZ` | 280.0 | 动态 notch 最高 |
| `IMU_GYRO_DYNAMIC_NOTCH_MODEL_BASE_HZ` | 35.0 | 油门模型截距 |
| `IMU_GYRO_DYNAMIC_NOTCH_MODEL_GAIN_HZ` | 247.0 | 油门模型斜率 |
| `IMU_GYRO_DYNAMIC_NOTCH_ALPHA` | 0.20 | 油门平滑系数 |
| `IMU_DYNAMIC_NOTCH_REBUILD_PERIOD_MS` | 20 | 后台最短重建间隔 |
| `IMU_DYNAMIC_NOTCH_MAX_STEP_HZ` | 8.0 | 单次最大步进 |
| `IMU_DYNAMIC_NOTCH_BASE_BW_LOW_HZ` | 45.0 | 低油门基频带宽 |
| `IMU_DYNAMIC_NOTCH_BASE_BW_HIGH_HZ` | 36.0 | 高油门基频带宽 |
| `IMU_GYRO_DYNAMIC_NOTCH_SLOT_COUNT` | 3 | 动态 notch 槽数 |
| `IMU_GYRO_RATE_PI_CUTOFF_HZ` | 130.0 | Rate P/I 低通 |
| `IMU_GYRO_RATE_D_CUTOFF_MIN_HZ` | 50.0 | D 链低通最小值 |
| `IMU_GYRO_RATE_D_CUTOFF_MAX_HZ` | 70.0 | D 链低通最大值 |
| `IMU_GYRO_ATT_CUTOFF_HZ` | 60.0 | 姿态阻尼低通 |
| `IMU_ACCEL_NOTCH_CENTER_HZ` | 180.0 | Acc notch 中心 |
| `IMU_ACCEL_NOTCH_BW_HZ` | 45.0 | Acc notch 带宽 |
| `IMU_ACCEL_NOTCH_ENABLE` | 1 | Acc notch 开关 |
| `IMU_ACCEL_EKF_CUTOFF_HZ` | 20.0 | Acc EKF 低通 |

### 1.7 EKF 参数

| 宏 | 值 | 说明 |
|----|-----|------|
| `IMU_EKF_P_INIT` | 0.02 | 初始协方差对角 |
| `IMU_EKF_P_DIAG_MIN` | 1.0e-7 | 协方差下界 |
| `IMU_EKF_P_DIAG_MAX` | 1.0 | 协方差上界 (超限→恢复) |
| `IMU_EKF_Q_QUAT_CONT` | 2.0e-5 | 过程噪声密度 |
| `IMU_EKF_R0` | 0.004 | 基础观测噪声 |
| `IMU_EKF_R_MIN` | 0.001 | 自适应 R 下界 |
| `IMU_EKF_R_MAX_NORMAL` | 0.08 | 正常模式 R 上界 |
| `IMU_EKF_R_MAX_DYNAMIC` | 0.80 | 动态低trust R 上界 |
| `IMU_EKF_R_ALPHA` | 30.0 | R 放大系数 (×acc偏差²) |
| `IMU_EKF_CHI2_GATE` | 9.0 | Chi2 门限 (~3σ) |
| `IMU_EKF_REJECT_R_INFLATE` | 50.0 | Reject 后 R 膨胀倍数 |
| `IMU_EKF_SOLVE_EPS` | 1.0e-8 | Cholesky pivot 下限 |
| `IMU_ACC_TRUST_NORM_SOFT_G` | 0.03 | 模长软门限 |
| `IMU_ACC_TRUST_NORM_HARD_G` | 0.18 | 模长硬门限 |
| `IMU_ACC_TRUST_INNOV_SOFT` | 0.035 | 创新软门限 |
| `IMU_ACC_TRUST_INNOV_HARD` | 0.22 | 创新硬门限 |
| `IMU_ACC_TRUST_INNOV_XY_SOFT` | 0.030 | 水平创新软 |
| `IMU_ACC_TRUST_INNOV_XY_HARD` | 0.20 | 水平创新硬 |
| `IMU_ACC_TRUST_MIN_FOR_CORRECT` | 0.08 | 最低 trust |
| `IMU_ACC_TRUST_R_SCALE_MAX` | 200.0 | R 最大放大倍数 |
| `IMU_EKF_ACC_HOLD_HARD_MS` | 30 | Hard 动态保持 |
| `IMU_EKF_ACC_HOLD_SEVERE_MS` | 90 | Severe 长保持 |

### 1.8 温漂与 Bias 参数

| 宏 | 值 | 说明 |
|----|-----|------|
| `IMU_TEMP_COMP_ENABLE` | 1 | 温漂补偿开关 |
| `IMU_TEMP_COMP_GYRO_LIMIT_RAD_S` | 0.05 | 补偿绝对限幅 |
| `IMU_TEMP_COMP_GYRO_X_SLOPE` | +0.00111208 | X 斜率 (rad/s/°C) |
| `IMU_TEMP_COMP_GYRO_Y_SLOPE` | -0.00017554 | Y 斜率 |
| `IMU_TEMP_COMP_GYRO_Z_SLOPE` | -0.00004176 | Z 斜率 |
| `IMU_ONLINE_BIAS_TAU_S` | 8.0 | 静止学习时间常数 |
| `IMU_ONLINE_BIAS_LIMIT_RAD_S` | 0.08 | 静止学习限幅 |
| `IMU_FLIGHT_GYRO_BIAS_ENABLE` | 0 | 飞行学习 (默认关闭) |
| `IMU_FLIGHT_GYRO_BIAS_TAU_S` | 60.0 | 飞行学习时间常数 |
| `IMU_FLIGHT_GYRO_BIAS_LIMIT_RAD_S` | 0.035 | 飞行学习限幅 |

### 1.9 FFT 参数

| 宏 | 值 | 说明 |
|----|-----|------|
| `IMU_FFT_RUNTIME_ENABLE_DEFAULT` | 1 | FFT 默认后台常开 |
| `IMU_FFT_HARMONIC_CONFIRM_FRAMES` | 2 | 谐波连续确认帧数 |
| `IMU_FFT_HARMONIC_MISS_FRAMES` | 2 | 谐波连续丢失帧数 |
| `IMU_FFT_HARMONIC_TOLERANCE_HZ` | 8.0 | 频点匹配容差 |
| `IMU_FFT_HARMONIC_MIN_MAG_RATIO` | 0.18 | 最小相对幅值 |
| `IMU_FFT_BASE_CORRECTION_ENABLE` | 1 | 基频修正开关 |
| `IMU_FFT_BASE_CORRECTION_MAX_HZ` | 18.0 | 修正绝对限幅 |
| `IMU_FFT_BASE_CORRECTION_ALPHA` | 0.25 | 修正更新系数 |
| `IMU_FFT_BASE_CORRECTION_DECAY` | 0.90 | 无匹配时衰减系数 |

---

## 2. IMU 硬件架构与 SPI 异步采集

### 2.1 双 IMU 物理布局

```
俯视图 (机头朝上):
        前方 (X+)
          ↑
    ┌─────────────┐
    │ IMU1    IMU2 │
    │ (左)    (右) │
    │ Y=-20mm Y=+20mm
    └─────────────┘

IMU1: SCB9 (P15), INT1=P19_2, Y=-0.020m
IMU2: SCB4 (P10), INT1=P10_4, Y=+0.020m
```

### 2.2 SPI Burst 时序

```
17 字节传输 (1 命令 + 16 payload):
  Byte 0:  0x9E (读寄存器 0x1E, 连续模式)
  Byte 1:  STATUS_REG (GDA=gyro fresh, XLDA=acc fresh)
  Byte 2-3: TEMP (int16, 256 LSB/°C, +25°C offset)
  Byte 4-5: GYRO_X (int16)
  Byte 6-7: GYRO_Y (int16)
  Byte 8-9: GYRO_Z (int16)
  Byte 10-11: ACC_X (int16)
  Byte 12-13: ACC_Y (int16)
  Byte 14-15: ACC_Z (int16)
  Byte 16: 保留

完成 ISR 处理顺序:
  1. 记录 complete_timestamp_us
  2. 解析 STATUS (GDA/XLDA)
  3. 温度: degC = RAW/256.0 + 25.0
  4. gyro: deg/s = transition(count); rad/s = deg/s * 0.01745
  5. acc:  g = transition(count); m/s² = g * 9.80665
  6. 轴映射 (传感器→机体)
  7. 双 IMU voter
  8. 饱和/冻结检测
  9. 发布快照
```

### 2.3 SPI 超时三级分类

```c
if (status & ERROR_MASK)        → TIMEOUT_REASON_ERROR_STATUS
else if (status & ACTIVE)       → TIMEOUT_REASON_ACTIVE
else                            → TIMEOUT_REASON_IRQ_MISS
```

### 2.4 双 IMU Voter

```c
融合模式:
  DUAL (3): 两路均有效 → 按新鲜度混合; 单路退化 → fallback
  FRONT (1)/REAR (2): 指定单路

对齐窗口:
  gyro: 1200us (覆盖 2kHz + 抖动)
  acc:  6000us (跨 2-3 个 2kHz tick)

不一致检测:
  |gyro1 - gyro2| > 0.80 rad/s  → DUAL_DISAGREE
  |acc1_residual - acc2_residual| > 4.0 m/s² → DUAL_DISAGREE
```

---

## 3. 原始数据到机体系的完整转换链

```c
// Step 1: int16 → 物理单位
sensor_gyro[i] = imu660rb_gyro_transition(count[i]) * 0.0174533f;  // deg/s→rad/s
sensor_acc[i]  = imu660rb_acc_transition(count[i]) * 9.80665f;     // g→m/s²

// Step 2: 轴映射 (传感器 X右/Y前/Z上 → 机体 X前/Y右/Z下)
// gyro: src={1,0,2}, sign={+1,+1,-1}
body_gyro_x = +sensor_gyro_y;
body_gyro_y = +sensor_gyro_x;
body_gyro_z = -sensor_gyro_z;

// acc: src={1,0,2}, sign={-1,-1,+1}  (重力观测符号, 与specific force相反)
body_acc_x = -sensor_acc_y;
body_acc_y = -sensor_acc_x;
body_acc_z = +sensor_acc_z;

// Step 3: 饱和检测
if (|count| >= 32760) → RAW_SATURATION 故障, 本帧不参与控制融合

// Step 4: 冻结检测
if (连续 16 帧 data payload 完全相同 且 宣称 fresh) → RAW_STUCK 故障
```

---

## 4. 零点标定系统

### 4.1 两阶段完整流程

```
阶段 1: Gyro/Acc Bias 均值
├── 静止门控 (全部满足):
│   ├── |acc_x_g| < 0.08g
│   ├── |acc_y_g| < 0.08g
│   ├── |acc_norm_g - 1.0| < 0.05g
│   ├── acc_z_g > 0.5g (排除倒置)
│   └── gyro_norm < 0.08 rad/s
├── 连续满足 500 样本
├── gyro_bias = Σgyro / 500
├── acc_bias = Σacc / 500 (Z轴-9.81)
├── gyro_bias_temp_degc = 当前IMU温度
├── 双IMU各记录 delta (供单路退化)
└── valid=1, 进入收敛等待

收敛等待: 500 样本 (~1.2s) EKF 姿态稳定

阶段 2: 姿态角零点 (圆周平均)
├── 累计 sin/cos:
│   ├── Σsin(roll), Σcos(roll)
│   ├── Σsin(pitch), Σcos(pitch)
│   └── Σsin(yaw), Σcos(yaw)
├── 满 500 样本后:
│   └── angle = atan2(Σsin, Σcos) * 57.296
├── attitude_zero_valid=1
└── active=0 (标定完成)
```

### 4.2 运动中断策略

```
阶段 1 运动 → 清空 bias 窗口, 重新累计 500 样本
阶段 2 运动 → 保留 bias, 清空姿态角窗口, 重置 settle_count=500
```

### 4.3 双 IMU 零偏差分

```c
// 标定完成时保存差分, 供单路退化扣除
instance[i].gyro_delta  = Σinstance_gyro/N - global_gyro_bias;
instance[i].accel_delta = Σinstance_acc/N  - (global_acc_bias + gravity);
```

---

## 5. Gyro 多路径滤波器链

### 5.1 完整数据流图

```
原始 Gyro (2kHz, 每 500us)
    │
    ├── [减静态bias]
    ├── [减在线bias]
    ├── [温漂补偿]
    │
    ▼  gyro_unfiltered_rad_s
    │
┌───┴────────────────────────────────────────────────┐
│ 动态 Notch (TPT-SVF, 级联 1~3 槽)                    │
│   Slot 0: 基频 (油门前馈, 55-280Hz, 始终启用)        │
│   Slot 1: 2x 谐波 (FFT确认后启用, BW=center*8%)       │
│   Slot 2: 3x 谐波 (FFT确认后启用, BW=center*6%)       │
│                                                      │
│   保护: rebuild_active=1 → ISR 旁路; 后台限速重建     │
├──────────────────────────────────────────────────────┤
│ 静态 Notch (直接型 IIR, 135Hz/35Hz, 默认关闭)         │
└──────────────────────┬───────────────────────────────┘
                       │  gyro_ekf_rad_s
        ┌──────────────┼──────────────┬──────────────────┐
        ▼              ▼              ▼                  ▼
   EKF预测(直通)  BW2 130Hz    PT2 50-70Hz       PT1 60Hz
   gyro_ekf       gyro_rate_pi  gyro_rate_d      gyro_attitude
```

### 5.2 滤波器链实现

```c
static void imu_filters_apply_gyro(gyro_input, snapshot)
{
    float input[3] = {gyro_input->x, gyro_input->y, gyro_input->z};

    // 启动时灌入状态避免阶跃
    if (!gyro_primed) imu_filters_prime_gyro(gyro_input);

    // 动态参数更新 (2kHz→1ms分频, 每4ms更新notch目标)
    imu_filter_dynamic_gyro_update();

    for (axis = 0; axis < 3; axis++) {
        // 级 1-3: 动态 TPT-SVF Notch
        dynamic_out = input[axis];
        if (rebuild_active == 0) {
            for (slot = 0; slot < 3; slot++)
                dynamic_out = filter_svf_notch_apply(&notch[slot][axis], dynamic_out);
        }
        gyro_dynamic_notch[axis] = dynamic_out;

        // 级 4: 静态直接型 Notch
        static_out = filter_notch_apply(&gyro_static_notch[axis], dynamic_out);

        // 四路分流
        gyro_ekf[axis]      = static_out;
        gyro_rate_pi[axis]  = filter_bw2_lpf_apply(&rate_pi_lpf[axis], static_out);
        gyro_rate_d[axis]   = filter_pt2_apply(&rate_d_lpf[axis], static_out);
        gyro_attitude[axis] = filter_pt1_apply(&attitude_lpf[axis], static_out);
    }

    // 诊断: notch 和 LPF 滤除分量
    notch_residual = input - gyro_ekf;
    lpf_residual   = gyro_ekf - gyro_rate_pi;
}
```

### 5.3 各路径滤波配置

| 路径 | 滤波器 | 截止频率 | 采样率 | 用途 |
|------|--------|---------|--------|------|
| gyro_ekf | 无 | — | 2kHz | EKF 四元数预测 |
| gyro_rate_pi | BW2 LPF | 130Hz | 2kHz | 速率环 P/I 测量 |
| gyro_rate_d | PT2 LPF | 50→70Hz 动态 | 2kHz | 速率环 D 测量 |
| gyro_attitude | PT1 LPF | 60Hz | 2kHz | 姿态外环阻尼 |

### 5.4 D 链动态低通

```c
// BF 风格曲线: x*(1-x)+x, 中段更线性
shaped_throttle = x*(1-x)+x;
target_cutoff = 50.0 + (70.0-50.0) * shaped_throttle;  // 50→70Hz

// 死区 0.5Hz
if (|target - current| >= 0.5) {
    gain = filter_pt2_gain(target, 0.0005);
    for (axis=0; axis<3; axis++) rate_d_lpf[axis].k = gain;
}
```

---

## 6. Acc 滤波器链

### 6.1 数据流

```
原始 Acc (416Hz, 仅 fresh 帧)
    │
    ├── [减静态bias]
    ▼  accel_unfiltered_m_s2
    │
┌───┴──────────────────┐
│ Acc Notch (180Hz/45Hz)│  直接型 IIR, 抑制固定机身振动
├──────────────────────┤
│ BW2 LPF (20Hz)       │  大幅衰减非重力加速度
└──────────┬───────────┘
           │  accel_ekf_m_s2
           ├→ EKF correct
           ├→ 投影缓存 world_z_accel
           └→ 高度 KF 预测输入
```

### 6.2 Acc 推进策略

**只在 fresh acc 帧到达时推进滤波状态**。在 1ms ISR 中:
```c
if (snapshot->accel_ready && snapshot->accel_fresh) {
    imu_filters_apply_accel(accel_input, snapshot);
} else {
    // 复用上次滤波结果, 不重复推进状态
    snapshot->accel_ekf_m_s2 = last_accel_ekf;
}
```

---

## 7. 动态 Notch 系统

### 7.1 三路径架构

```
高速路径 (2kHz ISR): 只写目标不重建系数
  油门归一化 → 一阶平滑(α=0.20, 1ms) → 油门模型 → 基频目标
  → 4ms一次写入 notch 目标频率

慢速路径 (后台): 限速重建系数
  → 检查 rebuild_pending → 限速(≥20ms) → 限步进(≤8Hz)
  → 计算带宽 → rebuild_active=1 → 重算TPT-SVF → rebuild_active=0

FFT 路径 (后台): 确认谐波
  → 峰值搜索 → 匹配 base*2/3 ±8Hz → 连续2帧确认/丢失
  → 启用/关闭谐波槽
```

### 7.2 油门模型

```c
target_hz = 35.0 + 247.0 * throttle_norm;  // 来自 2026-06-27 VOFA FFT
target_hz = clip(target_hz, 55, 280);

// FFT 基频慢修正叠加:
#if IMU_FFT_BASE_CORRECTION_ENABLE
target_hz += imu_fft_base_correction_hz;  // ±18Hz限幅
#endif
```

### 7.3 后台限速重建

```c
static void imu_dynamic_notch_service_background(now_ms)
{
    if (!rebuild_pending) return;
    if (now_ms - last_rebuild < 20) return;  // 限速

    for (slot = 0; slot < 3; slot++) {
        if (!slot_enabled[slot]) { center=0; changed=1; continue; }

        // 限步进
        step = clip(target - current, -8.0, 8.0);
        if (|step| >= 1.0) { current += step; changed=1; }

        // 是否需要继续
        if (|target - current| >= 1.0) pending = 1;
    }

    if (changed) {
        rebuild_active = 1; DMB();
        imu_filter_rebuild_gyro_dynamic_notch(1);  // preserve_state
        DMB(); rebuild_active = 0;
    }
    rebuild_pending = pending;
}
```

### 7.4 基频带宽自适应

```c
freq_norm = (center - 55) / (280 - 55);
bw = 45.0 + (36.0 - 45.0) * freq_norm;
// 悬停(低频): 45Hz 宽带宽, 覆盖密集电机噪声
// 满油门(高频): 36Hz 窄带宽, 减少相位损失
```

---

## 8. TPT-SVF 陷波滤波器数学推导

### 8.1 为什么用 TPT-SVF

直接型 IIR 在动态调谐时系数与历史状态耦合，更新会产生毛刺。TPT-SVF 将
状态从系数中解耦，支持在线调谐。

### 8.2 系数计算

```c
f = tan(π * center / sample_freq);
q_inv = 1.0 / q;
inv_denom = 1.0 / (1.0 + f*(f + q_inv));

a1  = inv_denom;
a2q = (f * inv_denom) * q_inv;
fq  = f * q;
```

### 8.3 运行时 Apply

```c
float filter_svf_notch_apply(filter, input) {
    v3 = input - ic2;           // 输入与第二积分误差
    v1 = a1*ic1 + a2q*v3;      // 第一积分器
    v2 = ic2 + fq*v1;          // 第二积分器 (=低通分量)
    notch = input - v2;        // 陷波 = 输入 - 低通

    // TPT 梯形积分 (比欧拉更稳定)
    ic1 = 2*v1 - ic1;
    ic2 = 2*v2 - ic2;
    return notch;
}
```

### 8.4 在线调谐 (preserve_state)

```c
void filter_svf_notch_tune_q_preserve(filter, fs, center, q) {
    // 只重算 a1/a2q/fq, 保留 ic1/ic2
    // ic1q 按新 q 重缩放以保持能量守恒
    filter_svf_notch_calc_coeff(fs, center, q, &a1, &a2q, &fq);
    filter->ic1q = ic1 * (1.0/q);
}
```

---

## 9. FFT 频谱分析与后台服务

### 9.1 参数

| 参数 | 值 | 说明 |
|------|-----|------|
| 点数 | 2048 | 实数 FFT |
| 采样率 | 1000Hz | 降采样 |
| 分辨率 | ~0.49Hz/bin | |
| 搜索范围 | 50-450Hz | |
| 采集源 | Gyro/Accel | 可配置 |

### 9.2 处理流程

```
SPI ISR: 压入降采样缓冲区

后台:
  1. 检查缓冲区满 2048 点
  2. 实数 FFT (在预算内分步)
  3. 幅值谱
  4. 峰值搜索 (50-450Hz, 3~5 最强峰)
  5. 谐波匹配:
     for harmonic in (2x, 3x):
       if peak_near(base*harmonic, ±8Hz) && mag_ratio>0.18:
         confirm++; if (confirm>=2) enable_slot
       else: miss++; if (miss>=2) disable_slot
  6. 基频修正:
     if (找到基频峰):
       correction += 0.25*(fft_peak - model_target), clip(±18)
     else: correction *= 0.90
```

### 9.3 耗时统计

```c
fft_service_last_us, fft_service_max_us, fft_service_avg_us (IIR shift=6)
fft_service_count, fft_service_over_budget_count
```

---

## 10. 温度漂移补偿

### 10.1 离线拟合

```c
// 静止窗口 (仅 gyro+acc 同时 fresh + 静止门控):
#define IMU_TEMP_DRIFT_WINDOW_SAMPLES 256

// 每完成一个窗口形成一个 (温度均值, gyro均值) 数据点
// VOFA 导出 CSV → MATLAB/Python 线性拟合 → slope
```

### 10.2 在线补偿

```c
delta_temp = current_temp - gyro_bias_temp_degc;
correction_x = clip(0.00111208 * delta_temp, ±0.05);
correction_y = clip(-0.00017554 * delta_temp, ±0.05);
correction_z = clip(-0.00004176 * delta_temp, ±0.05);
gyro_zeroed -= correction;
```

---

## 11. 在线 Gyro Bias 学习

### 11.1 一阶学习核心

```c
static void imu_gyro_bias_learn_step(bias, target, dt, tau, limit, learn_z) {
    float gain = dt / (tau + dt);  // 无 expf 的一阶步长
    bias->x += gain * (target->x - bias->x);
    bias->y += gain * (target->y - bias->y);
    if (learn_z) bias->z += gain * (target->z - bias->z);
    bias.x = clip(bias.x, ±limit); // 逐轴限幅
    bias.y = clip(bias.y, ±limit);
    bias.z = clip(bias.z, ±limit);
}
```

### 11.2 静止学习

```
条件: motor_inactive && disarmed && |acc-1g|<0.02 && gyro<0.08
参数: tau=8s, limit=±0.08rad/s, learn_z=1
```

### 11.3 飞行学习 (默认关闭)

```
条件: acc_trust>0.95 && gyro<0.10 && |Δthrottle|<0.015 && stable>1000ms
参数: tau=60s, limit=±0.035rad/s, learn_z=0 (只 roll/pitch)
```

---

## 12. 四元数 EKF 姿态估计

### 12.1 协方差存储 (上三角, 10元素)

```c
typedef struct {
    float p00, p01, p02, p03;
    float p11, p12, p13;
    float p22, p23;
    float p33;
} imu_ekf_covariance_struct;
```

### 12.2 500us 四元数预测

```c
// 一阶积分: q += 0.5*dt * Ω(gyro)*q
half_dt = 0.5 * dt;
q.w += (-q.x*gx - q.y*gy - q.z*gz) * half_dt;
q.x += ( q.w*gx + q.y*gz - q.z*gy) * half_dt;
q.y += ( q.w*gy - q.x*gz + q.z*gx) * half_dt;
q.z += ( q.w*gz + q.x*gy - q.y*gx) * half_dt;
imu_quat_normalize(q);
```

### 12.3 1ms 协方差预测

```c
// 合并 2 个 500us gyro step
// F = I + 0.5*dt * Ω(gyro) (4×4 线性化)
// P = F*P*F' + (2e-5*dt)*I
// 完整 4×4 矩阵乘法, 显式展开
```

### 12.4 完整 EKF Correct

```c
// 步骤 1: 预检查
acc_norm = |acc|; if (<=0.001) { severe_hold(90ms); return; }
if (acc_hold_ms > 0) { skip++; return; }

// 步骤 2: 观测与创新
z = acc / |acc|;  // 归一化观测 = 重力方向
h = R(q)·[0,0,1]';  // 预测重力方向
innov = z - h;

// 步骤 3: Acc Trust
a_err_g = |acc_norm/9.81 - 1.0|;
trust = ramp(a_err_g,0.03,0.18) * ramp(|innov|,0.035,0.22) * ramp(innov_xy,0.030,0.20);
if (trust < 0.08) { hold or skip; return; }

// 步骤 4: 雅可比 H (3×4)
H = ∂h/∂q

// 步骤 5: 自适应 R
r_adapt = 0.004*(1 + 30*a_err_g²);
r_eff = clip(r_adapt/trust², 0.001, 0.80);

// 步骤 6: S = HPH' + R (3×3)

// 步骤 7: Chi2 门控 (逐轴)
for i in 0..2:
    if innov[i]²/S[i][i] > 9.0: reject[i]=1; r[i]*=50;
if all_rejected: return;

// 步骤 8: Cholesky 求解 K = PHT/S
for row in 0..3:
    imu_cholesky3_solve(S, PHT[row], K[row]);  // 3×3 固定维度

// 步骤 9: 状态更新
q += K * innov;  imu_quat_normalize(q);

// 步骤 10: Joseph 协方差
A = I - K*H;
P = A*P*A' + K*R*K';
imu_covariance_from_matrix(P);  // 对称化+钳位+写回10元素
```

### 12.5 3×3 Cholesky 求解器

```c
// 固定维度: S=L*L', 前代+回代, 不显式求逆
static uint8 imu_cholesky3_solve(S[3][3], b[3], x[3])
{
    // 分解 S = L*L'
    l00 = √S00; if(l00≤1e-8) fail;
    l10 = S10/l00; l20 = S20/l00;
    l11 = √(S11-l10²); if(l11≤1e-8) fail;
    l21 = (S21-l20*l10)/l11;
    l22 = √(S22-l20²-l21²); if(l22≤1e-8) fail;

    // 前代 L*y = b
    y0=b0/l00; y1=(b1-l10*y0)/l11; y2=(b2-l20*y0-l21*y1)/l22;

    // 回代 L'*x = y
    x2=y2/l22; x1=(y1-l21*x2)/l11; x0=(y0-l10*x1-l20*x2)/l00;
}
```

### 12.6 欧拉角转换 (Z-Y-X)

```c
// 仅 1ms 调用
sinr_cosp = 2*(q.w*q.x + q.y*q.z);
cosr_cosp = 1 - 2*(q.x² + q.y²);
roll = atan2(sinr_cosp, cosr_cosp);  // [-180,180]

sinp = clip(2*(q.w*q.y - q.z*q.x), -1, 1);
pitch = asin(sinp);  // Z-Y-X 中间轴

siny_cosp = 2*(q.w*q.z + q.x*q.y);
cosy_cosp = 1 - 2*(q.y² + q.z²);
yaw = atan2(siny_cosp, cosy_cosp);
```

### 12.7 姿态恢复

```c
// 四元数/cov 损坏时, 用 acc 重建 roll/pitch, 保留 yaw
if (need_recover && acc_safe) {
    roll  = atan2(acc_y, acc_z);
    pitch = atan2(-acc_x, sqrt(acc_y²+acc_z²));
    yaw   = current_yaw;
    imu_quat_from_euler(roll, pitch, yaw, &q);
    P = 0.02*I;
}
// acc 安全条件: |acc-1g|<0.05 && hold_ms==0 && innov_xy<0.08
```

### 12.8 投影缓存 (1ms 更新)

```c
world_z_body_x = 2*(q.x*q.z - q.w*q.y);
world_z_body_y = 2*(q.w*q.x + q.y*q.z);
world_z_body_z = q.w² - q.x² - q.y² + q.z²;  // = cos(tilt)
cos_tilt = world_z_body_z;
world_z_accel = acc·proj - 9.81;  // 世界Z线加速度
```

---

## 13. 滤波器库完整实现清单

### 13.1 核心引擎: filter_biquad_apply_core

所有二阶 IIR 共用此核心 (5乘+4加+4赋值/拍):

```c
static float filter_biquad_apply_core(b0,b1,b2,a1,a2, x1,x2,y1,y2, input) {
    output = b0*input + b1*x1 + b2*x2 - a1*y1 - a2*y2;
    x2 = x1; x1 = input; y2 = y1; y1 = output;
    return output;
}
```

### 13.2 滤波器类型总表

| 结构体 | 离散化 | 参数 | 用途 |
|--------|--------|------|------|
| `filter_notch_t` | 直接型陷波 | center, bw | 固定频点抑制 |
| `filter_bw2_lpf_t` | 双线性变换 | cutoff | Rate P/I 低通 |
| `filter_biquad_lpf_t` | 双二阶+Q | cutoff | 通用二阶低通 |
| `filter_biquad_t` | 外部写系数 | 5系数 | 通用二阶 |
| `filter_lpf1_t` | 一阶 | cutoff | 电池/目标平滑 |
| `filter_hpf1_t` | 一阶 | cutoff | 去直流 |
| `filter_pt1_t` | PT1 | cutoff | 姿态阻尼 |
| `filter_pt2_t` | 两级PT1 | cutoff (-3dB修正) | D 链低通 |
| `filter_pt3_t` | 三级PT1 | cutoff (-3dB修正) | 视觉速度 |
| `filter_svf_lpf_t` | TPT-SVF | cutoff | 数值稳定替代 |
| `filter_svf_notch_t` | TPT-SVF notch | center, Q | 动态 notch |
| `filter_phase_comp_t` | 一阶补偿 | center | 相位补偿 |
| `filter_mavg_t` | O(1)滑动 | window | 均值平滑 |
| `filter_simple_lpf_i32_t` | 定点低通 | beta | 低速变量 |
| `filter_mean_acc_t` | 累加器 | — | 简单均值 |

### 13.3 截止频率修正系数

```c
#define FILTER_PT2_CUTOFF_CORRECTION 1.553773974f  // PT2 -3dB 修正
#define FILTER_PT3_CUTOFF_CORRECTION 1.961459177f  // PT3 -3dB 修正
#define FILTER_BUTTERWORTH_Q         0.70710678118f // BW Q 值
```

### 13.4 所有滤波器都支持的操作

```c
// 初始化 (一次性计算系数)
void filter_xxx_init(filter, sample_freq, cutoff...);

// 高频推进 (只做状态更新)
float filter_xxx_apply(filter, input);

// 状态管理
void filter_xxx_reset(filter);
void filter_xxx_state_set(filter, value);  // 灌入初值, 避免阶跃
```

---

## 14. 健康监控与故障诊断体系

### 14.1 完整故障位图 (16位)

```c
IMU_HEALTH_FAULT_TIMEOUT           (1<<0)   // SPI 超时
IMU_HEALTH_FAULT_TRANSFER          (1<<1)   // SDK 传输错误
IMU_HEALTH_FAULT_PARSE             (1<<2)   // payload 异常
IMU_HEALTH_FAULT_NO_FRESH_GYRO     (1<<3)   // 无 fresh gyro
IMU_HEALTH_FAULT_STALE             (1<<4)   // gyro 超龄
IMU_HEALTH_FAULT_SAMPLE_INTERVAL   (1<<5)   // 间隔越界
IMU_HEALTH_FAULT_DUAL_PRIMARY      (1<<6)   // IMU1 不可用
IMU_HEALTH_FAULT_DUAL_SECONDARY    (1<<7)   // IMU2 不可用
IMU_HEALTH_FAULT_DUAL_DISAGREE     (1<<8)   // 双路不一致
IMU_HEALTH_FAULT_FIFO_OVERRUN      (1<<9)   // FIFO 溢出
IMU_HEALTH_FAULT_RAW_STUCK         (1<<10)  // 数据冻结
IMU_HEALTH_FAULT_RAW_SATURATION    (1<<11)  // 量程饱和
IMU_HEALTH_FAULT_ATTITUDE_INVALID  (1<<12)  // 四元数/cov 损坏
```

### 14.2 故障分类

```
可自恢复 (health_fault_flags):
  TIMEOUT, TRANSFER, PARSE, NO_FRESH, STALE, INTERVAL,
  DUAL_DISAGREE, FIFO_OVERRUN
  → 后续好帧自动清除

不可自恢复 (runtime_fault_latched_flags):
  ATTITUDE_INVALID, RAW_STUCK, RAW_SATURATION
  → 仅 imu_init() 可清除

硬件故障锁 (单独锁存):
  DUAL_PRIMARY, DUAL_SECONDARY (单路硬件故障)
```

### 14.3 Gyro 超龄后台确认

```c
// 避免单次调度空窗误报:
if (gyro_age > 2500us) {
    if (stale_start == 0) stale_start = now;  // 开始计时
    else if (now - stale_start >= 2500us) mark_error(STALE);  // 确认
} else {
    stale_start = 0;  // 恢复
}
```

---

## 15. VOFA 调试入口与诊断通道

### 15.1 相关 VOFA 模式

| 模式 | 内容 |
|------|------|
| `VOFA_MODE_IMU` | 原始/滤波分级对照 |
| `VOFA_MODE_FFT` | 频谱瀑布图 |
| `VOFA_MODE_TIMING` | ISR/后台耗时统计 |
| `VOFA_MODE_TUNING_CASCADE` | 级联 PID 在线调参 |
| `VOFA_MODE_BEEP_SAFETY` | 蜂鸣/安全总览 |

### 15.2 关键诊断字段

```c
// 滤波器
gyro_dynamic_notch_center_hz / target_hz  // notch 当前/目标
gyro_dynamic_notch_h2/h3_center_hz        // 谐波槽
gyro_notch_residual_rad_s                 // notch 滤除量
gyro_lpf_residual_rad_s                   // 低通滤除量

// EKF
last_acc_trust / last_acc_innov_xy        // acc 信任度
last_ekf_r_acc                            // 实际 R 值
ekf_correct/skip/reset_count              // 计数
ekf_reject_count[3]                       // 逐轴 reject
ekf_solve_error_count                     // Cholesky 失败
cov_asymmetry_count                       // 不对称计数

// 健康
health_fault_flags                        // 故障位图
acc_low_trust_skip/hard_hold/severe_hold_count
acc_innov_too_high_count                  // 高创新次数

// 在线 bias
online_gyro_bias_rad_s                    // 当前零偏
flight_bias_update/skip_count             // 飞行学习统计

// SPI
transfer_count, timeout_count, overrun_count
transfer_last/max/avg_us
timeout_active/irq_miss/error_status_count
```

---

## 16. 开发指南与完整调参手册

### 16.1 添加新滤波器路径

```
1. imu_filter_bank_struct 添加实例
2. imu_filters_init() 初始化
3. imu_filters_apply_gyro/accel() 推进
4. imu_snapshot_struct 发布输出 (可选)
5. imu_filters_prime_gyro/accel() 支持状态灌入
6. VOFA 通道添加诊断
```

### 16.2 动态 Notch 标定步骤

```
1. 地面油门扫描: 从 idle→max, 每隔 100 duty, 记录 VOFA FFT 基频
2. 线性拟合: target = base + gain * throttle
3. 设置 IMU_GYRO_DYNAMIC_NOTCH_MODEL_BASE_HZ / GAIN_HZ
4. 悬停验证: 观察 notch_residual, 确认基频被有效抑制
5. 如果 FFT 能看到 2x/3x 谐波 → 谐波槽会自动启用
6. 调带宽: 悬停时加宽 (45Hz), 高油门收窄 (36Hz)
```

### 16.3 调参优先级

```
第 1: 油门模型标定 (必须本机 FFT)
第 2: Notch 带宽 (VOFA 观察 residual, 平衡抑制 vs 相位)
第 3: D 链低通 (D 噪声大→降低 max, D 响应慢→提高 min)
第 4: EKF Q/R  (Q 增大→快收敛, R 减小→更信 acc)
第 5: Acc Trust (correct 跳过太多→放宽, 振动抖→收紧)
```

### 16.4 常见问题

| 症状 | 可能原因 | 检查 |
|------|---------|------|
| 姿态漂移 | gyro bias 不准 | VOFA online_bias |
| 姿态抖动 | 振动通过 EKF | 观察 acc_trust, 降 R |
| 电机噪声大 | notch 频率不准 | FFT 复核基频 |
| 控制响应慢 | 滤波延迟大 | 复核低通截止频率 |
| SPI 超时增多 | 总线/中断异常 | timeout 分类统计 |
| EKF reset 多 | 数值问题 | ekf_reset_count |

---

> **文档版本**: v2.0 (极致细节版)  
> **关联文件**: `sensor/sensor_imu.c` (~5000行), `sensor/sensor_imu.h`, `common/common_filter.c` (~1100行), `common/common_filter.h`
