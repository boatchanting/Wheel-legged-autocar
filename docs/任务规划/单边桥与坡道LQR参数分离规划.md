# 单边桥 / 坡道 LQR 参数分离规划（待审批）

> 状态：**待审批**（未获批准前，严禁改动任何代码）
> 前提：当前 `vision_entry_lqr` 两路（单边桥 / 坡道）**共用同一套编译期宏参数，不支持独立配置**。
> 修订日期：2026-08-17

---

## 0. 结论

| 问题 | 现状 |
|---|---|
| 两路 LQR 参数能否不同？ | **不能**。`vision_entry_lqr.h` 用 `#define LQR_QY 150` 等宏，全模块静态单实例 `s_lqr`，`Reset` 只传航向不传参数 → bridge 与 slope 参数完全相同 |

本规划将参数由「编译期宏」改为「运行时结构体」，使 bridge / slope 各自持有一套独立参数集。

---

## 1. 现状（已核实）

- `vision_entry_lqr.h` 宏：`LQR_QY 150.0f`、`LQR_QPSI 24.0f`、`LQR_DETECT_RANGE_M 1.5f`、`LQR_W_MAX_RADPS 2.2f`、`LQR_ERR_MAX_DEG`（由 W_MAX 推导）、`LQR_V_FLOOR_MPS 0.3f`、`LQR_KLOCK 1.8f`（实现中未使用，冗余）。
- `VisionEntryLqr_Reset(float entry_yaw_deg)` 只锁航向。
- bridge（`vision_bridge_control.c`）与 slope（`vision_slope_control.c`）调同一模块 → 参数一致。

---

## 2. 目标

bridge、slope 各自独立配置 LQR 参数（Qy / Qψ / 检测距离 / W_MAX / v 下限）。

---

## 3. 改造方案（参数结构体化）

### 3.1 新增参数结构体（`vision_entry_lqr.h`）

```c
typedef struct
{
    float qy;             /* 横向权重 → k1 = √qy */
    float qpsi;           /* 航向权重 */
    float detect_range_m; /* 视觉段检测距离 */
    float w_max_radps;    /* ω 输出钳位（执行器极限） */
    float v_floor_mps;    /* k2 内 max(v, v_floor) 下限 */
} vision_entry_lqr_param_t;
```

- `err_degree` 钳位不再单独存：由 `w_max_radps` 推导 `err_max = w_max_radps · 57.29578f / 8.0f`。
- 删除冗余宏 `LQR_KLOCK`（盲区锁角由 bridge 侧 `g_vision_bridge_tune_defaults.yaw_hold_kp` 负责，slope 盲区回退 0，本模块不输出锁角）。
- 保留通用常量宏：`LQR_PHY_INVALID_MM 32767`。

### 3.2 状态结构增参数副本

```c
typedef struct
{
    vision_entry_lqr_param_t param;   /* 参数副本（Reset 时拷贝） */
    float entry_yaw_deg;
    uint8 valid;
    float beta_rad, dist_m, e_m, psi_err_rad, omega_radps;
} vision_entry_lqr_state_t;
```

### 3.3 接口变更

- `void VisionEntryLqr_Reset(float entry_yaw_deg, const vision_entry_lqr_param_t *param);` —— 签名加参数。
- `VisionEntryLqr_UpdateVision(...)` 内部用 `s_lqr.param.*` 替代 `LQR_QY/LQR_QPSI/LQR_DETECT_RANGE_M/LQR_W_MAX_RADPS/LQR_V_FLOOR_MPS`。
- `VisionEntryLqr_GetErrDegree()` 钳位改用 `s_lqr.param.w_max_radps · 57.29578f / 8.0f`。

---

## 4. 逐文件精确变更清单

### 4.1 `code/vision/vision_entry_lqr.h`
- 新增 `vision_entry_lqr_param_t`。
- 删除宏：`LQR_QY / LQR_QPSI / LQR_DETECT_RANGE_M / LQR_W_MAX_RADPS / LQR_ERR_MAX_DEG / LQR_V_FLOOR_MPS / LQR_KLOCK`。
- 保留宏：`LQR_PHY_INVALID_MM`。
- `vision_entry_lqr_state_t` 增 `param` 字段。
- `VisionEntryLqr_Reset` 签名加 `const vision_entry_lqr_param_t *param`。

### 4.2 `code/vision/vision_entry_lqr.c`
- `Reset` 拷贝参数；`UpdateVision` / `GetErrDegree` 改读 `s_lqr.param.*`。

### 4.3 `code/vision/vision_bridge_control.h`（新增 bridge 参数宏）
```c
#define VISION_BRIDGE_LQR_QY             150.0f
#define VISION_BRIDGE_LQR_QPSI           24.0f
#define VISION_BRIDGE_LQR_DETECT_RANGE_M 1.5f
#define VISION_BRIDGE_LQR_W_MAX_RADPS    2.2f
#define VISION_BRIDGE_LQR_V_FLOOR_MPS    0.3f
```

### 4.4 `code/vision/vision_bridge_control.c`
- 定义 `static const vision_entry_lqr_param_t s_bridge_lqr_param = { ...bridge 宏... };`
- `vision_bridge_enter_task()`：`VisionEntryLqr_Reset(entry_yaw_deg, &s_bridge_lqr_param);`

### 4.5 `code/vision/vision_slope_control.h`（新增 slope 参数宏）
```c
#define VISION_SLOPE_LQR_QY             ???   /* 待审批给定 */
#define VISION_SLOPE_LQR_QPSI           ???
#define VISION_SLOPE_LQR_DETECT_RANGE_M ???
#define VISION_SLOPE_LQR_W_MAX_RADPS    2.2f
#define VISION_SLOPE_LQR_V_FLOOR_MPS    0.3f
```

### 4.6 `code/vision/vision_slope_control.c`
- 定义 `static const vision_entry_lqr_param_t s_slope_lqr_param = { ...slope 宏... };`
- `vision_slope_enter_task()`：`VisionEntryLqr_Reset(entry_yaw_deg, &s_slope_lqr_param);`

---

## 5. 默认参数（待确认）

| 参数 | bridge | slope |
|---|---|---|
| Qy | 150.0f | **待审批给定**（暂缺） |
| Qψ | 24.0f | **待审批给定**（暂缺） |
| 检测距离 | 1.5f m | **待审批给定**（暂缺） |
| W_MAX | 2.2f | 2.2f |
| v 下限 | 0.3f | 0.3f |

> 调参文档《无降速版调参文档.md》只给了单边桥参数（Qy150/Qψ24/1.5m）。**坡道（slope）的 Qy/Qψ/检测距离需你给定**；若暂缺，可先沿用 bridge 同值（结构上已支持独立，后续只改 slope 宏即可）。

---

## 6. 编译验证

- 双核 iarbuild（cm7_0 / cm7_1）0 错误。

---

## 7. 回退方案

- L1：slope 参数宏改回与 bridge 同值（等效旧行为）。
- L2：`git checkout` 回退本次参数分离改动。
