# 0701 GPS+惯导互补滤波架构说明

## 一、架构概述

废弃纯 GPS 寻迹，改用**基于 Offset 的双频互补滤波**：

```
惯导 100Hz ─→ INS 独立积分 ─→ ins_x/y/yaw ─┐
                                              ├─ fuse = ins + offset → Pure Pursuit
GPS 10Hz ──→ 高斯投影 → 旋转到赛道系 → 杆臂补偿 ──→ 缓慢更新 offset ─┘
              + 延时前馈    + 跃变剔除
              + ZUPT 零速挂起
              + 打滑权重转移
航向：100% 陀螺仪积分，GPS 不修正（单天线无 THS）
```

## 二、工作流

```
上电 → GPS 冷启动收敛（约1分钟，卫星≥8）
  → 摆好车头朝向 → 上位机点击"清空轨迹"
    → 锁定原点 + 磁力计记发车角 + 惯导清零
  → 上位机点击"开始GPS复刻"（或菜单按键）
    → Pure Pursuit 使用 fuse_x/fuse_y（赛道坐标系，mm）
```

## 三、文件清单

| 文件 | 职责 |
|------|------|
| `code/navigation/gnss_transform.h` | GPS 物理参数宏、坐标转换 API |
| `code/navigation/gnss_transform.c` | 高斯投影、手动锁原点 |
| `code/navigation/gnss_ins_fusion.h` | 融合状态结构体、API 声明 |
| `code/navigation/gnss_ins_fusion.c` | 100Hz INS 推算 + 10Hz GPS 纠偏 + 手动锁原点 |
| `code/navigation/nav_replay/plan1/plan1_gnss.c` | Pure Pursuit 消费端（读取 fuse 坐标） |
| `user/cm7_0_isr.c` | ISR 调度（Fusion_Ins_Update + Fusion_Gps_Correct） |
| `code/tools/wifi_protocol.c` | 上位机"清空轨迹"触发手动锁原点 |

## 四、需实车测量的参数

> 全部在 `code/navigation/gnss_transform.h` 中修改

| 宏 | 当前值 | 怎么测 |
|----|--------|--------|
| `ANTENNA_HEIGHT_Z` | 0.15f m | 卷尺量 GPS 天线中心到两轮轴心连线中点的**垂直高度** |
| `ANTENNA_OFFSET_X` | 0.04f m | 卷尺量天线中心到两轮轴心所在垂直平面的**前后距离**（车头方向为正） |
| `ANTENNA_OFFSET_Y` | 0.04f m | 卷尺量天线中心到车身纵向中轴线的**左右距离**（车身左侧为正） |
| `GPS_DELAY_SEC` | 0.12f s | 默认值可先用；若高速急刹时坐标冲过头，微调增大 |

## 五、需调试的参数

### 5.1 融合权重 `K_pos`（战术级调度）

> 位置：`gnss_ins_fusion.c` → `Fusion_Gps_Correct()` 战术级 K_pos 调度器

K_pos 由以下规则**依次判定**，后者可覆盖前者：

| 优先级 | 条件 | K_pos | 原因 |
|--------|------|-------|------|
| 1 (默认) | 卫星 ≥ 15 且不在特殊元素 | **0.02** | 正常行驶 |
| 2 | 卫星 10~14 | 0.01 | 卫星数一般，拉扯减半 |
| 3 | 卫星 < 10 | 0.0 | 卫星严重不足，弃用 GPS |
| 4 (最高) | 当前路点为雷区/桥/颠簸路 | 0.0 | 特殊元素内 100% 信任惯导 |
| — | 零速 (轮速<10 且角速度<2°/s) | 0.0 | ZUPT 防原地漂移 |
| — | 打滑 (slip_flag=1) | 0.10 | 惯导失真，被迫信任 GPS |

**调优建议**：默认 0.02 偏惯导信任，偏离路线调大至 0.05，有抖动调小至 0.01。

### 5.2 跃变剔除阈值

> 位置：`gnss_ins_fusion.c` 第 179 行，`delta_gps > 1500.0f`（即 1.5m/0.1s）

正常行驶不会触发。金属盆遮挡 GPS 测试时验证：遮挡→移开后应平滑归位，无急刹。

### 5.3 零速挂起阈值

> 位置：`gnss_ins_fusion.c` 第 193 行

- `wheel_speed < 10.0f`：轮速死区（原始值，非 mm/s）
- `gyro_z_deg_s < 2.0f`：角速度死区（°/s）

停车时 GPS 散布不应导致车身晃动。若停车时仍抖，调小 `K_pos` 或调大轮速死区。

### 5.4 发车角 `g_track_base_yaw`

> 位置：`gnss_ins_fusion.c` → `Fusion_Manual_Lock_Origin()` 自动从 `heading`（磁力计）读取

**需确认**：磁力计 `heading` 的 0 度定义。逐飞通常定义正北=0°，顺时针为正。如果实际偏差大，需校准磁力计（见 `tools/04_传感器标定与测试/` 下的标定工具）。

### 5.5 Pure Pursuit 参数

> 位置：`code/navigation/nav_replay/plan1/plan1_gnss.h`

| 宏 | 当前值 | 作用 | 调法 |
|----|--------|------|------|
| `GPS_NAV_LOOKAHEAD_DIST` | 2500.0f mm | 前瞻距离 | 调大走线更平滑，调小转弯更灵敏 |
| `GPS_NAV_SPEED_FAST` | -600.0f | 直道速度 | 负数=前进 |
| `GPS_NAV_SPEED_SLOW` | -80.0f | 弯道/近点速度 | |

## 六、遥测三轨迹验证

WiFi 遥测帧已包含三组轨迹数据（单位 mm），可在上位机中画出比对：

| 轨迹 | 变量 | 特征 |
|------|------|------|
| 纯惯导 (ins) | `ins_x`, `ins_y` | 极度平滑，但长时间会漂移 |
| GPS 地面真值 (gps) | `ground_x*1000`, `ground_y*1000` | 绝对位置准，但有锯齿/跳变 |
| 融合输出 (fusion) | `fuse_x`, `fuse_y` | 平滑 + 长期贴合 GPS |

**验证要点**：
1. fusion 线必须和 ins 线一样平滑，不能出现 gps 线的锯齿
2. fusion 线在长距离下必须慢慢贴合 gps 线，不能越拉越远
3. 过桥/雷区时 fusion 线应完全跟随 ins（此时 K=0，GPS 被挂起）

## 七、Checklist

- [ ] 卷尺测量三个天线偏移量，填入 `gnss_transform.h`
- [ ] 上电等 GPS 收敛（卫星≥8，`gnss_trans.is_valid==1`）
- [ ] 摆好车头，上位机点"清空轨迹"，确认串口输出 `Origin LOCKED. base_yaw=xxx`
- [ ] **直推测试**：往前推车 → `fuse_x` 应越来越小（负数增大），`fuse_y` 基本不变
- [ ] **横推测试**：往右推车 → `fuse_y` 应越来越大（正数），`fuse_x` 基本不变
- [ ] 若直推/横推方向反了：检查 `Fusion_Gps_Correct()` 中旋转矩阵正负号
- [ ] 原地猛推车身：`ground_x/y` 不应剧烈反向变化（若反了则 `sinf(pitch)` 加负号）
- [ ] 金属盆遮挡测试：遮挡 3 秒 → 移开后平滑归位，无抽搐
- [ ] 实际跑一圈调 `K_pos`：偏离路线调大，有抖动调小
