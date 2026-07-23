# IMU 陀螺仪温漂标定全链路技术文档 (完整版)

> **工程**: Seekfree CYT4BB 飞控 (CM7_0)  
> **传感器**: 双 IMU660RB (ICM-42688-P 兼容), SPI 30MHz DMA  
> **MCU**: Infineon CYT4BB7 Cortex-M7 @ 250MHz  
> **生成日期**: 2026-07-23  
> **编码**: GB2312  
> **覆盖范围**: 从芯片上电初始化、SPI 原始温度寄存器读取、到在线补偿落地、离线拟合全流程

---

## 核心结论: 温度传感器无需额外寄存器配置

**ICM-42688-P 的温度传感器在芯片进入工作模式后自动启动, 无需写入任何专用寄存器来使能。**

`imu660rb_init_instance()` 写入的 9 个寄存器中没有与温度使能相关的位——`CTRL1_XL=0x6C` (加速度计 416Hz ODR) 使芯片进入 Low Noise 模式后, 温度传感器同步自动运行。17 字节 SPI Burst 的第 2-3 字节天然就是 `TEMP_OUT`, 这是芯片原生行为。

---

## 目录

1. [芯片初始化: 寄存器写入全貌](#1-芯片初始化-寄存器写入全貌)
2. [IMU 模块初始化: 运行时结构体与温度初值](#2-imu-模块初始化-运行时结构体与温度初值)
3. [底层数据读取: SPI Burst 温度解析 (完整代码)](#3-底层数据读取-spi-burst-温度解析)
4. [温度数据在运行时的完整流转](#4-温度数据在运行时的完整流转)
5. [温漂离线数据采集管道](#5-温漂离线数据采集管道)
6. [VOFA 遥测通道定义](#6-vofa-遥测通道定义)
7. [离线拟合方法与结果](#7-离线拟合方法与结果)
8. [在线温漂补偿实现 (完整代码)](#8-在线温漂补偿实现)
9. [双 IMU 实例级温漂采集](#9-双-imu-实例级温漂采集)
10. [静态零偏标定与温漂参考点的自动耦合](#10-静态零偏标定与温漂参考点的自动耦合)
11. [诊断状态导出](#11-诊断状态导出)
12. [安全保护机制 (6层)](#12-安全保护机制)
13. [Gyro 数据处理完整时序 (含温漂位置)](#13-gyro-数据处理完整时序)
14. [验证方法与后续工作](#14-验证方法与后续工作)
15. [变更记录索引](#15-变更记录索引)
16. [附录: 宏定义/函数/结构体字段速查](#16-附录)

---

## 1. 芯片初始化: 寄存器写入全貌

### 1.1 关键事实

`imu660rb_init_instance()` 向每颗 IMU660RB 写入以下 9 个寄存器。**全都没有温度使能位。**

> 文件: `libraries/zf_device/zf_device_imu660rb.c:613`

```c
uint8 imu660rb_init_instance(const imu660rb_spi_config_struct *config)
{
    uint8 return_state = 0;
    if (config == 0) { return 1u; }

    system_delay_ms(20);   // 上电稳定等待

    // 硬件 SPI 初始化
    spi_init(config->spi, SPI_MODE0, IMU660RB_SPI_SPEED,
             config->spc_pin, config->sdi_pin, config->sdo_pin, SPI_CS_NULL);
    gpio_init(config->cs_pin, GPO, GPIO_HIGH, GPO_PUSH_PULL);

    do {
        // WHO_AM_I 自检
        if (imu660rb_self_check_instance(config)) {
            zf_log(0, "imu660rb instance self check error.");
            return_state = 1; break;
        }

        // ===== 9 个寄存器写入 (无一与温度使能相关) =====
        if ((imu660rb_write_register_instance_raw(config, IMU660RB_INT1_CTRL, 0x03) == 0u) ||  // INT1 中断配置
            (imu660rb_write_register_instance_raw(config, IMU660RB_CTRL1_XL,  0x6C) == 0u) ||  // Acc 416Hz ±8G ← 使芯片进入active, 温度自动启动
            (imu660rb_write_register_instance_raw(config, IMU660RB_CTRL2_G,   0x9C) == 0u) ||  // Gyro 3.33kHz ±2000dps
            (imu660rb_write_register_instance_raw(config, IMU660RB_CTRL3_C,  0x44) == 0u) ||  // BDU + 数字低通
            (imu660rb_write_register_instance_raw(config, IMU660RB_CTRL4_C,  0x02) == 0u) ||  // Gyro 低通使能
            (imu660rb_write_register_instance_raw(config, IMU660RB_CTRL5_C,  0x00) == 0u) ||  // Round 配置
            (imu660rb_write_register_instance_raw(config, IMU660RB_CTRL6_C,  0x00) == 0u) ||  // Acc 高性能
            (imu660rb_write_register_instance_raw(config, IMU660RB_CTRL7_G,  0x00) == 0u) ||  // Gyro 高性能
            (imu660rb_write_register_instance_raw(config, IMU660RB_CTRL9_XL, 0x02) == 0u))    // I3C disable
        {
            zf_log(0, "imu660rb instance config write timeout.");
            return_state = 1u; break;
        }

        // ===== 回读验证 9 个寄存器 =====
        if ((imu660rb_register_matches_instance(config, IMU660RB_CHIP_ID,   0x6B) == 0u) ||
            (imu660rb_register_matches_instance(config, IMU660RB_INT1_CTRL, 0x03) == 0u) ||
            (imu660rb_register_matches_instance(config, IMU660RB_CTRL1_XL,  0x6C) == 0u) ||
            (imu660rb_register_matches_instance(config, IMU660RB_CTRL2_G,   0x9C) == 0u) ||
            (imu660rb_register_matches_instance(config, IMU660RB_CTRL3_C,   0x44) == 0u) ||
            (imu660rb_register_matches_instance(config, IMU660RB_CTRL4_C,   0x02) == 0u) ||
            (imu660rb_register_matches_instance(config, IMU660RB_CTRL5_C,   0x00) == 0u) ||
            (imu660rb_register_matches_instance(config, IMU660RB_CTRL6_C,   0x00) == 0u) ||
            (imu660rb_register_matches_instance(config, IMU660RB_CTRL7_G,   0x00) == 0u) ||
            (imu660rb_register_matches_instance(config, IMU660RB_CTRL9_XL,  0x02) == 0u))
        {
            zf_log(0, "imu660rb instance config readback error.");
            return_state = 1; break;
        }
    } while (0);

    return return_state;
}
```

### 1.2 为什么不需要 PWR_MGMT0?

ICM-42688-P 的 `PWR_MGMT0` (0x4E, Bank 0) 控制 `ACCEL_MODE` 和 `GYRO_MODE`。当 `CTRL1_XL` 写入 0x6C (ODR≠0) 时, 内部状态机自动将加速度计从 SLEEP 切换到 LN (Low Noise) 模式, 温度传感器同步自动运行。不需要手动写 `PWR_MGMT0`。

### 1.3 温度换算公式

```c
// sensor_imu.c:64-65
#define IMU660RB_TEMP_OFFSET_DEGC     (25.0f)    // 0 LSB → 25°C
#define IMU660RB_TEMP_LSB_PER_DEGC    (256.0f)   // 256 LSB/°C

// 换算: temp_degc = (int16)raw_temp / 256.0 + 25.0
// 例: raw=288 → 288/256.0+25.0 = 26.125°C
```

---

## 2. IMU 模块初始化: 运行时结构体与温度初值

> 文件: `project/code/sensor/sensor_imu.c:4369`, `imu_init()`

```c
void imu_init(void)
{
    uint8 index;
    uint8 present_count = 0u;
    uint8 context_count = 0u;

    // ===== 步骤 1: 全结构体清零 =====
    memset(&imu_runtime, 0, sizeof(imu_runtime));

    // ===== 步骤 2: 双 IMU 实例绑定 (SPI/INT/坐标/温度初值) =====
    imu_dual_instance_runtime_configure();
    // 内部为每颗 IMU 设置:
    //   instance->device       = &imu_device_config[index];
    //   instance->axis         = &imu_axis_mapping_default;
    //   instance->int_pin      = IMU1_INT_PIN / IMU2_INT_PIN;
    //   instance->position_body_m = {0, ±0.020m, 0};
    //   instance->temp_degc    = IMU_TEMP_COMP_REF_DEGC;  // = 25.0f ← TEMP

    // ===== 步骤 3: 姿态/滤波/FFT 初始化 =====
    imu_quat_reset_identity(&imu_runtime.attitude.quat);
    imu_ekf_covariance_reset();
    imu_filters_init();
    imu_fft_debug_init();

    // ===== 步骤 4: 融合级温度字段初始化为 25°C ← TEMP =====
    imu_runtime.imu_temp_degc             = IMU_TEMP_COMP_REF_DEGC;
    imu_runtime.temp_drift_temp_mean_degc = IMU_TEMP_COMP_REF_DEGC;
    imu_runtime.zero.state.gyro_bias_temp_degc = IMU_TEMP_COMP_REF_DEGC;

    // ===== 步骤 5: 在线 bias 学习初始授权 =====
    imu_runtime.online_bias_motor_inactive = 1u;
    imu_runtime.online_bias_disarmed = 1u;

    // ===== 步骤 6: 逐颗 IMU 芯片初始化 =====
    for (index = 0u; index < IMU_INSTANCE_COUNT; index++)
    {
        imu_instance_runtime_struct *instance = &imu_runtime.instance[index];

        // ★ 芯片寄存器写入——温度传感器自此自动运行
        if (imu660rb_init_instance(instance->device) == 0u)
        {
            instance->present = 1u;
            instance->healthy = 1u;
            present_count++;
            imu_spi_async_context_init(index);
            if (instance->spi.context_ready != 0u) { context_count++; }
            exti_init(instance->int_pin, EXTI_TRIGGER_RISING);
        }
        else
        {
            instance->present = 0u;
            instance->healthy = 0u;
            imu_runtime.dual.health_fault_flags |= imu_dual_instance_fault_flag(index);
        }
    }

    // ===== 步骤 7: 双 IMU 比赛构型检查 =====
    imu_runtime.device_ready = ((present_count == IMU_INSTANCE_COUNT) &&
                                (context_count == IMU_INSTANCE_COUNT)) ? 1u : 0u;
    imu_runtime.healthy = (imu_runtime.device_ready != 0u) ? 1u : 0u;
    imu_dual_refresh_health_flags();
}
```

**为何初始化为 25°C?** 25°C 是 `TEMP_OUT=0` 的标称温度。在首次 SPI Burst 完成前, VOFA 显示 25°C 而非 0°C, 避免误导。

---

## 3. 底层数据读取: SPI Burst 温度解析

### 3.1 SPI Burst 17 字节布局

```
Byte 0:  0x9E         (读 0x1E 连续模式)
Byte 1:  STATUS_REG   (XLDA, GDA)
Byte 2:  TEMP_OUT[7:0]
Byte 3:  TEMP_OUT[15:8]
Byte 4-5: GYRO_X      (int16)
Byte 6-7: GYRO_Y      (int16)
Byte 8-9: GYRO_Z      (int16)
Byte 10-11: ACC_X     (int16)
Byte 12-13: ACC_Y     (int16)
Byte 14-15: ACC_Z     (int16)
Byte 16: 保留
```

### 3.2 SPI Complete ISR 中的温度解析 (完整代码)

> 文件: `project/code/sensor/sensor_imu.c`, `imu_parse_burst_and_update_instance()`

```c
static uint8 imu_parse_burst_and_update_instance(uint8 index)
{
    imu_instance_runtime_struct *instance;
    const uint8 *payload;
    int16 gyro_count[3];
    int16 accel_count[3];
    int16 temp_count;               // ← TEMP
    imu_gyro_rad_s_struct raw_gyro;
    imu_accel_m_s2_struct raw_accel;
    uint32 sample_timestamp_us;
    uint16 structure_fault;

    if (index >= IMU_INSTANCE_COUNT) { return 0u; }
    instance = &imu_runtime.instance[index];

    // 硬故障锁: 运行期故障后不再解析该 IMU
    if (instance->runtime_fault_latched) { return 0u; }

    // rx_buffer[0] 是命令字节期间的无效应答, payload 从 [1] 开始
    payload = &instance->spi.rx_buffer[1];

    // === 步骤 1: 结构检查 (全00/FF, 保留位, 冻结检测) ===
    structure_fault = imu_payload_structure_fault_get(instance, payload);
    if (structure_fault != 0u)
    {
        instance->spi.parse_error_count++;
        instance->raw_stuck_count++;
        instance->recovery_valid_count = 0u;
        instance->healthy = 0u;
        instance->raw_valid = 0u;
        instance->runtime_fault_latched = 1u;
        imu_health_latch_error(structure_fault);
        imu_runtime.dual.health_fault_flags |= imu_dual_instance_fault_flag(index);
        return 0u;
    }

    // === 步骤 2: 解析 int16 原始值 (小端序) ===
    temp_count   = read_i16_le(&payload[2]);   // ← TEMP: payload[2:3]
    gyro_count[0] = read_i16_le(&payload[4]);
    gyro_count[1] = read_i16_le(&payload[6]);
    gyro_count[2] = read_i16_le(&payload[8]);
    accel_count[0] = read_i16_le(&payload[10]);
    accel_count[1] = read_i16_le(&payload[12]);
    accel_count[2] = read_i16_le(&payload[14]);

    // === 步骤 3: 饱和检查 ===
    if (imu_raw_counts_are_saturated(gyro_count, accel_count))
    {
        instance->spi.parse_error_count++;
        instance->raw_saturation_count++;
        instance->recovery_valid_count = 0u;
        instance->healthy = 0u;
        instance->raw_valid = 0u;
        instance->runtime_fault_latched = 1u;
        imu_health_latch_error(IMU_HEALTH_FAULT_RAW_SATURATION);
        imu_runtime.dual.health_fault_flags |= imu_dual_instance_fault_flag(index);
        return 0u;
    }

    // === 步骤 4: int16→物理单位 + 轴映射 ===
    imu_raw_to_body_units(gyro_count, accel_count, instance->axis, &raw_gyro, &raw_accel);
    // 内部: gyro: LSB→deg/s→rad/s, acc: LSB→g→m/s², 然后传感器→机体 NED 映射

    // === 步骤 5: 物理量有效性检查 (NaN/Inf/粗限幅) ===
    if (!imu_raw_units_are_valid(&raw_gyro, &raw_accel))
    {
        instance->spi.parse_error_count++;
        instance->recovery_valid_count = 0u;
        instance->healthy = 0u;
        instance->raw_valid = 0u;
        instance->runtime_fault_latched = 1u;
        imu_health_latch_error(IMU_HEALTH_FAULT_PARSE);
        imu_runtime.dual.health_fault_flags |= imu_dual_instance_fault_flag(index);
        return 0u;
    }

    // === 步骤 6: 采样时间戳 (优先 INT1 边沿) ===
    sample_timestamp_us = instance->spi.kick_timestamp_us + IMU_SPI_SAMPLE_OFFSET_US;
    if ((instance->int_diag.last_edge_us != 0u) &&
        ((uint32)(instance->spi.complete_timestamp_us - instance->int_diag.last_edge_us)
         <= IMU_INT_TIMESTAMP_MAX_AGE_US))
    {
        sample_timestamp_us = instance->int_diag.last_edge_us;
    }

    // === 步骤 7: 写入实例缓存 ← TEMP ===
    instance->status_reg   = payload[0];
    instance->temp_degc    = ((float)temp_count / IMU660RB_TEMP_LSB_PER_DEGC)
                           + IMU660RB_TEMP_OFFSET_DEGC;   // raw/256+25
    instance->raw_gyro_rad_s  = raw_gyro;
    instance->raw_accel_m_s2  = raw_accel;
    instance->gyro_fresh  = (payload[0] & IMU660RB_STATUS_GDA_MASK)  ? 1u : 0u;
    instance->accel_fresh = (payload[0] & IMU660RB_STATUS_XLDA_MASK) ? 1u : 0u;
    instance->last_sample_timestamp_us = sample_timestamp_us;
    if (instance->gyro_fresh)  { instance->last_gyro_timestamp_us  = sample_timestamp_us; }
    if (instance->accel_fresh) { instance->last_accel_timestamp_us = sample_timestamp_us;
                                 instance->accel_sample_seq++; }

    // === 步骤 8: 单实例级温漂采集 ← TEMP ===
    imu_instance_temp_drift_observe(instance, &raw_gyro, &raw_accel,
                                    instance->gyro_fresh, instance->accel_fresh);

    instance->last_complete_timestamp_us = instance->spi.complete_timestamp_us;
    instance->raw_valid = 1u;

    // === 步骤 9: 冷启动连续有效帧资格 ===
    if (instance->gyro_fresh != 0u)
    {
        if (instance->recovery_valid_count < IMU_INSTANCE_STARTUP_VALID_FRAMES)
            instance->recovery_valid_count++;
        if (instance->recovery_valid_count >= IMU_INSTANCE_STARTUP_VALID_FRAMES)
            instance->healthy = 1u;
    }
    return 1u;
}
```

---

## 4. 温度数据在运行时的完整流转

```
SPI Complete ISR (per instance)
  payload[2:3] → temp_count (int16)
  instance->temp_degc = (float)temp_count/256.0 + 25.0
  imu_instance_temp_drift_observe() [单实例级温漂采集]
        │
        ▼
双 IMU Voter (imu_dual_try_publish)
  imu_runtime.imu_temp_degc = 0.5*(IMU1.temp_degc + IMU2.temp_degc)  ← 双路均值
  gyro/acc 融合 → 杆臂补偿 → imu_build_and_publish_snapshot()
        │
        ▼
imu_build_and_publish_snapshot() [gyro fresh 分支]
  ① imu_temp_drift_observe(raw_gyro, raw_acc)  → 静止+256样本→窗口+1
  ② imu_zero_accumulate(raw_gyro, raw_acc)     → 零偏完成: gyro_bias_temp_degc = imu_temp_degc
  ③ imu_zero_apply_gyro(raw → gyro_zeroed)     → 扣静态 bias
  ④ imu_temp_comp_apply_gyro(&gyro_zeroed)     → 扣温漂: slope×(T-T_ref) ← TEMP
  ⑤ imu_online_bias_update / apply
  ⑥ imu_filters_apply_gyro (notch → 4路LPF)
        │
        ▼
VOFA 遥测: CH40-49(温漂窗口), CH54(实时温度)
```

---

## 5. 温漂离线数据采集管道

### 5.1 参数

```c
#define IMU_TEMP_DRIFT_WINDOW_SAMPLES (256u)
// 256个 gyro+acc 同时 fresh 的静止样本 → 约 256/416Hz ≈ 0.615 秒/窗口
```

### 5.2 静止门控 (与零偏标定共用)

```c
static uint8 imu_zero_sample_is_static(
    const imu_gyro_rad_s_struct *gyro_rad_s,
    const imu_accel_m_s2_struct *accel_m_s2)
{
    float acc_x_g = accel_m_s2->x_m_s2 / 9.80665f;
    float acc_y_g = accel_m_s2->y_m_s2 / 9.80665f;
    float acc_z_g = accel_m_s2->z_m_s2 / 9.80665f;
    float acc_norm_g = imu_accel_norm_m_s2(accel_m_s2) / 9.80665f;

    if (fabsf(acc_x_g) >= 0.08f)          return 0u;  // 水平运动
    if (fabsf(acc_y_g) >= 0.08f)          return 0u;  // 水平运动
    if (fabsf(acc_norm_g - 1.0f) >= 0.05f) return 0u; // 振动/加速
    if (acc_z_g <= 0.5f)                 return 0u;  // 倒置/大倾角
    if (imu_gyro_norm_rad_s(gyro_rad_s) >= 0.08f) return 0u; // 转动

    return 1u;
}
```

### 5.3 融合级采集函数

```c
static void imu_temp_drift_window_reset(void)
{
    imu_runtime.temp_drift_temp_sum_degc = 0.0f;
    imu_runtime.temp_drift_gyro_sum_rad_s.x_rad_s = 0.0f;
    imu_runtime.temp_drift_gyro_sum_rad_s.y_rad_s = 0.0f;
    imu_runtime.temp_drift_gyro_sum_rad_s.z_rad_s = 0.0f;
    imu_runtime.temp_drift_sample_count = 0u;
}

static void imu_temp_drift_observe(
    const imu_gyro_rad_s_struct *raw_gyro_rad_s,
    const imu_accel_m_s2_struct *raw_accel_m_s2,
    uint8 gyro_ready, uint8 accel_ready)
{
    float inv_count;

    // 第1关: gyro+acc 必须同时 fresh
    if ((!gyro_ready) || (!accel_ready)) { return; }

    // 第2关: 温度无效或机体不静止 → 清空窗口 (防止运动角速度拟合成温漂!)
    if ((!finite_f32(imu_runtime.imu_temp_degc, 1.0e20f)) ||
        (!imu_zero_sample_is_static(raw_gyro_rad_s, raw_accel_m_s2)))
    {
        imu_runtime.temp_drift_static = 0u;
        imu_temp_drift_window_reset();
        return;
    }

    // 第3关: 累计静止样本
    imu_runtime.temp_drift_static = 1u;
    imu_runtime.temp_drift_temp_sum_degc += imu_runtime.imu_temp_degc;
    imu_runtime.temp_drift_gyro_sum_rad_s.x_rad_s += raw_gyro_rad_s->x_rad_s;
    imu_runtime.temp_drift_gyro_sum_rad_s.y_rad_s += raw_gyro_rad_s->y_rad_s;
    imu_runtime.temp_drift_gyro_sum_rad_s.z_rad_s += raw_gyro_rad_s->z_rad_s;
    imu_runtime.temp_drift_sample_count++;

    if (imu_runtime.temp_drift_sample_count < 256u) { return; }

    // 第4关: 满256样本 → 形成拟合点, 窗口+1, 重置
    inv_count = 1.0f / (float)imu_runtime.temp_drift_sample_count;
    imu_runtime.temp_drift_temp_mean_degc =
        imu_runtime.temp_drift_temp_sum_degc * inv_count;
    imu_runtime.temp_drift_gyro_mean_rad_s.x_rad_s =
        imu_runtime.temp_drift_gyro_sum_rad_s.x_rad_s * inv_count;
    imu_runtime.temp_drift_gyro_mean_rad_s.y_rad_s =
        imu_runtime.temp_drift_gyro_sum_rad_s.y_rad_s * inv_count;
    imu_runtime.temp_drift_gyro_mean_rad_s.z_rad_s =
        imu_runtime.temp_drift_gyro_sum_rad_s.z_rad_s * inv_count;
    imu_runtime.temp_drift_window_count++;
    imu_temp_drift_window_reset();
}
```

### 5.4 调用位置 (在 `imu_build_and_publish_snapshot` 中)

```c
if (gyro_fresh)
{
    // ① 温漂采集 ← 使用原始 gyro/acc, 未扣任何 bias
    imu_temp_drift_observe(raw_gyro_rad_s, raw_accel_m_s2, gyro_fresh, accel_fresh);

    // ② 零点标定 (完成时记录 gyro_bias_temp_degc)
    imu_zero_accumulate(raw_gyro_rad_s, raw_accel_m_s2, gyro_fresh, accel_fresh);

    // ③ 扣静态零偏
    imu_zero_apply_gyro(raw_gyro_rad_s, &gyro_zeroed);

    // ④ 扣温漂补偿
    imu_temp_comp_apply_gyro(&gyro_zeroed);

    // ⑤ 在线bias / 滤波...
}
```

**关键设计**: 温漂采集使用原始 gyro, 后续补偿链的修改不影响采集到的温漂模型。

---

## 6. VOFA 遥测通道定义

> 文件: `project/code/communication/comm_vofa_debug.c`, `vofa_debug_fill_imu()`

| 通道 | 名称 | 来源字段 | 离线拟合用途 |
|------|------|---------|-------------|
| CH34-36 | gyro_bias X/Y/Z | `zero.state.gyro_bias_rad_s` | 确认零偏完成 |
| **CH40** | 窗口温度 | `temp_drift_temp_mean_degc` | **拟合自变量 X** |
| **CH41** | 窗口 gyro X | `temp_drift_gyro_mean_rad_s.x` | **X轴拟合目标 Y** |
| **CH42** | 窗口 gyro Y | `temp_drift_gyro_mean_rad_s.y` | **Y轴拟合目标 Y** |
| **CH43** | 窗口 gyro Z | `temp_drift_gyro_mean_rad_s.z` | **Z轴拟合目标 Y** |
| CH44 | 窗口计数 | `temp_drift_window_count` | 去重 (变化=新点) |
| CH45 | 样本数 | `temp_drift_sample_count` | 判断采集推进 |
| CH46 | 静止门控 | `temp_drift_static` | 筛选=1且CH44变化 |
| CH47-49 | 温补 X/Y/Z | `temp_comp_correction_rad_s` | 启用后验证模型 |
| CH54 | IMU温度 | `imu_temp_degc` | 实时温度观测 |

```c
// VOFA 填充代码:
vofa_debug_set_channel(40u, context->imu_diag.temp_drift_temp_degc);
vofa_debug_set_channel(41u, context->imu_diag.temp_drift_gyro_mean_rad_s.x_rad_s);
vofa_debug_set_channel(42u, context->imu_diag.temp_drift_gyro_mean_rad_s.y_rad_s);
vofa_debug_set_channel(43u, context->imu_diag.temp_drift_gyro_mean_rad_s.z_rad_s);
vofa_debug_set_channel(44u, (float)context->imu_diag.temp_drift_window_count);
vofa_debug_set_channel(45u, (float)context->imu_diag.temp_drift_sample_count);
vofa_debug_set_channel(46u, (float)context->imu_diag.temp_drift_static);
vofa_debug_set_channel(47u, context->imu_diag.temp_comp_correction_rad_s.x_rad_s);
vofa_debug_set_channel(48u, context->imu_diag.temp_comp_correction_rad_s.y_rad_s);
vofa_debug_set_channel(49u, context->imu_diag.temp_comp_correction_rad_s.z_rad_s);
vofa_debug_set_channel(54u, context->imu_diag.imu_temp_degc);
```

---

## 7. 离线拟合方法与结果

### 7.1 拟合流程

```
上电静置 → 温度自然升高 → VOFA+ CSV 导出
  → 按 CH44 去重 (每个窗口只取第一条)
  → 过滤 CH46≠1 或无效值
  → 线性回归: gyro_mean = a + b × temp_mean
  → 3×RMSE 残差裁剪 → 重拟合
  → 只取斜率 b 写入代码
  (截距 a 不需要: 零偏标定时自动记录参考温度)
```

### 7.2 当前代码中使用的斜率 (2026-06-27, 温区 24.57→31.09°C)

```c
#define IMU_TEMP_COMP_GYRO_X_SLOPE_RAD_S_PER_DEGC (0.00111208f)
#define IMU_TEMP_COMP_GYRO_Y_SLOPE_RAD_S_PER_DEGC (-0.00017554f)
#define IMU_TEMP_COMP_GYRO_Z_SLOPE_RAD_S_PER_DEGC (-0.00004176f)
```

### 7.3 初次拟合 (2026-06-17, 温区 26.1→29.0°C, 643窗口)

| 轴 | 斜率 | R² | 评价 |
|---|------|-----|------|
| X | +0.00067336 | 0.838 | 温漂明显 |
| Y | -0.00008942 | 0.201 | 接近噪声 |
| Z | -0.00007213 | 0.175 | 接近噪声 |

---

## 8. 在线温漂补偿实现

### 8.1 宏定义

```c
// sensor_imu.c:70-75
#define IMU_TEMP_COMP_ENABLE          (1u)            // 总开关
#define IMU_TEMP_COMP_REF_DEGC        (25.0f)         // 默认参考温度
#define IMU_TEMP_COMP_GYRO_LIMIT_RAD_S (0.05f)        // 补偿绝对限幅

#define IMU_TEMP_COMP_GYRO_X_SLOPE_RAD_S_PER_DEGC (0.00111208f)
#define IMU_TEMP_COMP_GYRO_Y_SLOPE_RAD_S_PER_DEGC (-0.00017554f)
#define IMU_TEMP_COMP_GYRO_Z_SLOPE_RAD_S_PER_DEGC (-0.00004176f)
```

### 8.2 补偿函数 (完整实现)

```c
static void imu_temp_comp_apply_gyro(imu_gyro_rad_s_struct *gyro_zeroed_rad_s)
{
    // === 步骤 1: 清零诊断输出 ===
    imu_runtime.temp_comp_correction_rad_s.x_rad_s = 0.0f;
    imu_runtime.temp_comp_correction_rad_s.y_rad_s = 0.0f;
    imu_runtime.temp_comp_correction_rad_s.z_rad_s = 0.0f;

#if IMU_TEMP_COMP_ENABLE  // ← 编译时开关
    {
        // === 步骤 2: 获取参考温度 (零偏标定时自动记录) ===
        float reference_temp_degc = imu_runtime.zero.state.gyro_bias_temp_degc;
        float delta_temp_degc;

        // === 步骤 3: 三重安全门控 ===
        if ((!imu_runtime.zero.state.valid) ||               // 零偏未完成
            (!finite_f32(imu_runtime.imu_temp_degc, 1e20f)) ||  // 温度无效
            (!finite_f32(reference_temp_degc, 1e20f)))           // 参考温度无效
        {
            return;  // → 零补偿
        }

        // === 步骤 4: 温差 ===
        delta_temp_degc = imu_runtime.imu_temp_degc - reference_temp_degc;

        // === 步骤 5: 逐轴线性补偿 + 双向限幅 ===
        imu_runtime.temp_comp_correction_rad_s.x_rad_s =
            fclip(0.00111208f * delta_temp_degc, -0.05f, 0.05f);
        imu_runtime.temp_comp_correction_rad_s.y_rad_s =
            fclip(-0.00017554f * delta_temp_degc, -0.05f, 0.05f);
        imu_runtime.temp_comp_correction_rad_s.z_rad_s =
            fclip(-0.00004176f * delta_temp_degc, -0.05f, 0.05f);

        // === 步骤 6: 从 gyro 中扣除 ===
        gyro_zeroed_rad_s->x_rad_s -= imu_runtime.temp_comp_correction_rad_s.x_rad_s;
        gyro_zeroed_rad_s->y_rad_s -= imu_runtime.temp_comp_correction_rad_s.y_rad_s;
        gyro_zeroed_rad_s->z_rad_s -= imu_runtime.temp_comp_correction_rad_s.z_rad_s;
    }
#else
    (void)gyro_zeroed_rad_s;
#endif
}
```

### 8.3 补偿公式

$$ \text{correction}_i = \operatorname{clip}(\text{slope}_i \times (T_{\text{curr}} - T_{\text{ref}}),\; \pm 0.05) $$

$$ \hat{\omega}_i = \omega_{\text{raw}} - \text{bias}_{\text{static}} - \text{correction} - \text{bias}_{\text{online}} $$

---

## 9. 双 IMU 实例级温漂采集

```c
// 在 SPI Complete ISR 中, 解析完单颗 IMU 后:
instance->temp_degc = ((float)temp_count / 256.0f) + 25.0f;

// 单实例级温漂采集 — 使用单颗 IMU 温度 (非融合温度)
imu_instance_temp_drift_observe(instance, &raw_gyro, &raw_accel,
                                instance->gyro_fresh, instance->accel_fresh);

// 逻辑与融合级一致: 256样本/窗口, 静止门控, 运动清空
// 数据输出到 instance->temp_drift_* 字段
```

---

## 10. 静态零偏标定与温漂参考点的自动耦合

### 10.1 零偏完成时自动记录温度

```c
// 在 imu_zero_accumulate() 中, bias 第一阶段完成时:
zero->state.gyro_bias_temp_degc =
    finite_f32(imu_runtime.imu_temp_degc, 1e20f)
    ? imu_runtime.imu_temp_degc            // ← 使用当前实际温度
    : IMU_TEMP_COMP_REF_DEGC;              // ← 温度无效时回退到 25°C
```

### 10.2 为什么不需要固定截距?

```
correction = slope × (T_current − T_reference)
                              ↑
                    零偏标定时自动记录的实际温度

每次开机零偏标定 → T_reference 自动更新 → 适应不同环境温度
```

### 10.3 耦合时序

```
开机
  ├── imu_init()               → gyro_bias_temp_degc = 25°C (占位)
  ├── SPI 采样开始              → imu_temp_degc 实时更新
  ├── imu_zero_calibration_start() → 500样本静止标定
  ├── 零偏完成                  → gyro_bias_temp_degc = imu_temp_degc ← ★
  └── 此后:
        ├── 温漂采集: 使用 imu_temp_degc 继续累计窗口
        └── 温漂补偿: ΔT = imu_temp_degc - gyro_bias_temp_degc
```

---

## 11. 诊断状态导出

> 文件: `sensor_imu.h`, `imu_diagnostic_state_struct`; `sensor_imu.c`, `imu_diagnostic_get_state()`

```c
// 融合级:
state->imu_temp_degc         = imu_runtime.imu_temp_degc;
state->gyro_bias_temp_degc   = imu_runtime.zero.state.gyro_bias_temp_degc;
state->temp_drift_temp_degc  = imu_runtime.temp_drift_temp_mean_degc;
state->temp_drift_gyro_mean_rad_s = imu_runtime.temp_drift_gyro_mean_rad_s;
state->temp_comp_correction_rad_s = imu_runtime.temp_comp_correction_rad_s;
state->temp_drift_window_count    = imu_runtime.temp_drift_window_count;
state->temp_drift_sample_count    = imu_runtime.temp_drift_sample_count;
state->temp_drift_static          = imu_runtime.temp_drift_static;
state->temp_comp_enabled          = IMU_TEMP_COMP_ENABLE ? 1u : 0u;

// 实例级 (字段名相同, 通过 imu_dual_calibration_state_struct 导出):
state->temperature_degc[0..1]
state->temp_drift_temperature_degc[0..1]
state->temp_drift_gyro_mean_rad_s[0..1]
// ...
```

---

## 12. 安全保护机制 (6层)

| 层级 | 机制 | 条件 | 效果 |
|------|------|------|------|
| 1 | 编译开关 | `IMU_TEMP_COMP_ENABLE=0` | 全代码被 `#if` 排除 |
| 2 | 斜率默认0 | slope=0.0f (未拟合时) | 补偿恒为0 |
| 3 | 零偏有效性 | `!zero.state.valid` | → 零补偿 |
| 4 | 温度有限性 | `!finite_f32(temp, 1e20)` | → 零补偿 |
| 5 | 参考温度检查 | `!finite_f32(ref_temp, 1e20)` | → 零补偿 |
| 6 | 绝对值限幅 | `fclip(value, ±0.05)` | 补偿量双向钳位 |

---

## 13. Gyro 数据处理完整时序

```
原始 gyro (SPI burst)
  │
  ├── [温漂采集]  ← raw gyro, 不受后续补偿影响
  ├── [零偏标定]  ← 完成时记录 gyro_bias_temp_degc
  │
  ├── 减静态 bias         → gyro_zeroed
  ├── 减温漂补偿           → correction = slope×(T−T_ref)   ← 温漂在此
  ├── 减在线 bias          → gyro_corrected
  │
  ├── 动态 Notch (1-3槽)
  ├── 静态 Notch
  │
  ├── 分流: gyro_ekf          (直通, 给EKF预测)
  ├── 分流: gyro_rate_pi      (BW2 130Hz, 给速率P/I)
  ├── 分流: gyro_rate_d       (PT2 50-70Hz动态, 给速率D)
  └── 分流: gyro_attitude     (PT1 60Hz, 给姿态外环)
```

---

## 14. 验证方法与后续工作

### 已验证

| 项目 | 状态 | 说明 |
|------|------|------|
| 采集管道 | ✅ | 643窗口, CH44: 2→644 |
| 静止门控 | ✅ | CH46 全程=1 |
| 两次拟合 | ✅ | X R²=0.838 → 更宽温区更新 |
| 温补启用 | ✅ | `IMU_TEMP_COMP_ENABLE=1` |
| IAR编译 | ✅ | 0 error, 0 warning |

### 待验证 (P1优先)

| 优先级 | 项目 | 说明 |
|--------|------|------|
| P1 | 更宽温区 | 当前~6.5°C, 需20→45°C |
| P1 | 冷却回程 | 判断斜率有无迟滞 |
| P1 | CH47-49确认 | 补偿量随温度变化 |
| P2 | Y/Z复核 | R²低, 不稳定可置0 |
| P3 | Acc温补 | 需六面数据, 暂不做 |

---

## 15. 变更记录索引

| 日期 | 文档 |
|------|------|
| 2026-06-16 | `变更记录/FUNCTION_VAR_CHANGE_RECORD_20260616_IMU_TEMP_DRIFT_DIAG.md` |
| 2026-06-17 | `变更记录/FUNCTION_VAR_CHANGE_RECORD_20260617_IMU_TEMP_COMP_ENABLE.md` |
| 2026-06-17 | `IMU_TEMP_DRIFT_CALIBRATION_AND_CHAIN_REVIEW_20260617.md` |
| 2026-06-27 | `sensor_imu.c` 斜率更新 (更宽温区重拟合) |

---

## 16. 附录: 宏定义/函数/结构体快速索引

### A. 温度宏定义 (sensor_imu.c)

| 宏 | 值 | 行号 |
|----|-----|------|
| `IMU660RB_TEMP_OFFSET_DEGC` | 25.0f | 64 |
| `IMU660RB_TEMP_LSB_PER_DEGC` | 256.0f | 65 |
| `IMU_TEMP_DRIFT_WINDOW_SAMPLES` | 256u | 66 |
| `IMU_TEMP_COMP_ENABLE` | 1u | 70 |
| `IMU_TEMP_COMP_REF_DEGC` | 25.0f | 71 |
| `IMU_TEMP_COMP_GYRO_LIMIT_RAD_S` | 0.05f | 72 |
| `IMU_TEMP_COMP_GYRO_X_SLOPE_RAD_S_PER_DEGC` | 0.00111208f | 73 |
| `IMU_TEMP_COMP_GYRO_Y_SLOPE_RAD_S_PER_DEGC` | -0.00017554f | 74 |
| `IMU_TEMP_COMP_GYRO_Z_SLOPE_RAD_S_PER_DEGC` | -0.00004176f | 75 |

### B. 温度函数

| 函数 | 文件 | 行号 | 功能 |
|------|------|------|------|
| `imu660rb_init_instance` | `zf_device_imu660rb.c` | 613 | 芯片初始化(温度自此自动运行) |
| `imu_init` | `sensor_imu.c` | 4369 | IMU模块初始化(温度初值=25°C) |
| `imu_parse_burst_and_update_instance` | `sensor_imu.c` | ~3920 | SPI解析(temp_degc更新) |
| `imu_temp_drift_window_reset` | `sensor_imu.c` | 1589 | 清空采集窗口 |
| `imu_temp_drift_observe` | `sensor_imu.c` | 1609 | 融合级温漂采集 |
| `imu_instance_temp_drift_observe` | `sensor_imu.c` | ~1660 | 单实例温漂采集 |
| `imu_zero_sample_is_static` | `sensor_imu.c` | ~1575 | 静止门控(温漂+零偏共用) |
| `imu_zero_accumulate` | `sensor_imu.c` | 1714 | 零偏标定(记录gyro_bias_temp_degc) |
| `imu_temp_comp_apply_gyro` | `sensor_imu.c` | 1832 | 在线温漂补偿扣除 |
| `imu_build_and_publish_snapshot` | `sensor_imu.c` | 3593 | 快照发布(串联全链路) |
| `imu_diagnostic_get_state` | `sensor_imu.c` | 5048 | 诊断导出(含温漂字段) |
| `vofa_debug_fill_imu` | `comm_vofa_debug.c` | 1159 | VOFA CH40-49,54 填充 |

### C. 温度结构体字段

| 结构体 | 字段 | 说明 |
|--------|------|------|
| `imu_instance_runtime_struct` | `temp_degc` | 单颗芯片温度 |
| `imu_instance_runtime_struct` | `temp_drift_*` | 单颗温漂窗口 |
| `imu_runtime_struct` | `imu_temp_degc` | 融合温度(双路平均) |
| `imu_runtime_struct` | `temp_drift_*` | 融合级温漂窗口 |
| `imu_runtime_struct` | `temp_comp_correction_rad_s` | 在线补偿量 |
| `imu_zero_calibration_state_struct` | `gyro_bias_temp_degc` | 零偏完成时温度(温漂参考点) |
| `imu_diagnostic_state_struct` | `imu_temp_degc` | 对外诊断: 实时温度 |
| `imu_diagnostic_state_struct` | `gyro_bias_temp_degc` | 对外诊断: 参考温度 |
| `imu_diagnostic_state_struct` | `temp_drift_*` | 对外诊断: 温漂窗口 |
| `imu_diagnostic_state_struct` | `temp_comp_correction_rad_s` | 对外诊断: 补偿量 |
| `imu_diagnostic_state_struct` | `temp_comp_enabled` | 对外诊断: 开关状态 |
| `imu_dual_calibration_state_struct` | `temperature_degc[2]` | 两颗IMU各自温度 |
