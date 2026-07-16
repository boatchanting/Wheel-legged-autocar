我要基于当前惯导的纵向速度融合，在上面融入 GPS 速度，也就是互补滤波。

但是，这个方案更像是用 GPS 的相对变化量修正惯导速度与轨迹，而不是取 GPS 的绝对坐标。

（原因：低速遥控打点记录轨迹的流程中，通过惯导记录我可以得到一个精确的发车点位和发车角度，这个精度远高于GPS，也可以绕开GPS坐标系原点和惯导坐标系原点的重合难题；只有高速复刻轨迹的过程中，我才需要引入GPS来修正惯导。也就是说，只有开始发车或者说开始复刻轨迹，也就是START_CAR触发时，才启用GPS的权重）

首先我需要一个车头的地理航向角heading，用于对齐GPS坐标系与车身坐标系（但是当前原点可能没有重合。没关系，既然不需要绝对GPS坐标，就不需要GPS系原点）；

然后我需要从gnss底层库直接调用解析出来的gnss.speed；

接下来我要把这个GPS速度融入惯导中。当前惯导权重是0.9的轮速计+0.1的陀螺仪融合，我需要把这个融合一次的惯导二次融入gps（这样获得一个新的量），给GPS赋一个权重融进去；

最后，考虑到轮速计不打滑的情况下，轮速精准度要高于GPS和IMU。因此，为了应对轮腾空/打滑等惯导失效的场景，我需要动态调整这个权重。考虑到赛道固定、打滑场景固定（掉头、绕桩），采用基于曲率的动态权重：
读取当前参考点的前瞻点的 curvature。
进行简单的分段映射或线性映射：直道（曲率趋近0）时设置低 Kp，弯道（曲率大）时设置高 Kp。注意这里说的曲率大是指相对轨迹的其他地方而言的。

补充：

1. GPS 航向角 (Heading) 的获取与对齐

对于这个heading的获取，采用两种方式，在sys_options里切换：

①直接写死。相当于我发车之前测量好，然后烧录代码。

②如下：
不要用位置相减算位移，直接用 GPS 的多普勒速度！ GPS 模块会直接输出对地速度 (Speed Over Ground) 和 对地航向 (Course Over Ground, COG)。当车速大于 0.5m/s 时，GPS 的 COG 是极其精准的地理航向。这里它的作用不是去算绝对位移，而是用来估计车辆的地理行驶方向，并把 gnss.speed 投影到车身坐标系。

为了更加精确，又考虑到小车发车后必定有一段直线轨迹（赛道特性），可以用一段时间内的 COG 做均值或者滤波，把处理后对应实际直线轨迹的 COG 作为航向角 heading。这样的话，可以设置：发车三秒内（不一定几秒）不启用 GPS，直到获取了稳定的 COG 才把 GPS 权重打开。

当前使用的是 逐飞科技 TAU1201 / GN42A 双频 GPS 模块（见 user/main0/init_main0.c:174），驱动层已经完整解析了这两个字段：

数据结构定义（zf_device_gnss.h）

数据来源：NMEA RMC 语句（zf_device_gnss.c:263-265）

RMC 语句的第 7 个字段是 Speed Over Ground（节），第 8 个字段是 Course Over Ground（度）。驱动已自动将速度从"节"转换为"km/h"。

2. 频率不匹配与“阶跃突变”

问题：轮速计和 IMU 是高频的（例如 500Hz/1000Hz），而 GPS 是低频的（5Hz/10Hz）。如果在 GPS 更新的那一帧突然把 Δv_gps 融进去，会导致速度估计出现阶跃，并进一步传导到位移上。

对策：必须将低频的 GPS 速度平滑分配到高频的控制周期中，或者对融合后的速度进行低通滤波后再积分。

3. 动态权重切换的平滑性

问题：从“直道（例如GPS权重0.05）”进入“U型弯（例如GPS权重0.6）”时，如果权重瞬间切换，同样会导致轨迹跳变。
对策：权重必须是连续渐变的（例如使用线性过渡或一阶低通滤波）。

4. 关于车身坐标系与gnss坐标系

保留 `inertial_nav` 作为原始惯导，在它后面增加一个 `nav_pose_fusion` 或类似模块，专门输出：

```c
fused_x_mm
fused_y_mm
fused_yaw_deg
gps_weight
gps_valid
heading_lock
```

推荐的速度版融合方式如下：

在 `START_CAR` 时锁定三个基准：

```c
ins0 = 当前 inertial_nav.x/y
heading0 = 固定写死值，或发车直线段 COG 平均值
fused = ins0
```

之后每次 GPS 更新时，直接取 `gnss.speed`，并结合 `heading0` 或 COG 将其转换成车身坐标系下的纵向速度，而不是使用 GPS 绝对坐标：

```text
v_gps = gnss.speed
v_gps_body = v_gps * cos(cog_deg - heading0_deg)
gps_vx_ins = -v_gps_body
```

这里 `gps_vx_ins` 前面取负，是因为你的惯导 `X` 轴“向后为正”。如果只做纵向速度互补，GPS 只需要参与 `vx_body` 的估计；位置仍由惯导积分得到。

然后不要让 GPS 一帧跳进融合坐标，而是做速度残差修正：

```text
v_err = gps_vx_ins - fused_vx_body
fused_vx_body += gps_weight * lowpass(v_err)
fused_pose += fused_vx_body * dt + inertial_lateral_delta
```


以上就是一个惯导优先的互补滤波方案。评估这个方案。

## 📋 需要阅读的文件清单

### 一、惯导核心模块（必须精读）

| 文件 | 路径 | 阅读重点 |
|------|------|----------|
| **inertial_nav.h** | inertial_nav.h | 融合参数 `NAV_ALPHA_VEL`(0.9)、数据结构 `InertialNav_t`、坐标系定义 |
| **inertial_nav.c** | inertial_nav.c | 当前融合逻辑（第150-160行）、打滑检测机制、速度积分与坐标变换 |

### 二、GPS模块（必须精读）

| 文件 | 路径 | 阅读重点 |
|------|------|----------|
| **gnss_transform.h** | gnss_transform.h | 输出结构体 `gnss_transform_struct`（x/y相对坐标、有效性标志） |
| **gnss_transform.c** | gnss_transform.c | 高斯投影算法、原点机制、相对坐标计算（第130-180行） |

### 三、GPS导航实现（参考）

| 文件 | 路径 | 阅读重点 |
|------|------|----------|
| **plan1_gnss.c** | plan1_gnss.c | GPS航向角计算 `GpsNav_GetCurrentHeadingDeg()`、GPS速度读取方式 |
| **gps_nav_replay_route_table.h** | gps_nav_replay_route_table.h | GPS路线表格式 |

### 四、系统配置（理解开关逻辑）

| 文件 | 路径 | 阅读重点 |
|------|------|----------|
| **sys_options.h** | sys_options.h | `GNSS_NAV`开关、`SLIP_DETECTION_ENABLE`开关、`IMU_CATEGORY` |
| **car_select.h** | car_select.h | 轮距 `WHEEL_BASE_MM`、轮速系数 `SPEED_TO_MM_S` |

### 五、中断调用（理解调用时机）

| 文件 | 路径 | 阅读重点 |
|------|------|----------|
| **cm7_0_isr.c** | cm7_0_isr.c | 第340-390行：`InertialNav_Update()` 调用位置，10ms周期，输入参数来源 |

### 六、打点与回放（理解数据流）

| 文件 | 路径 | 阅读重点 |
|------|------|----------|
| **nav_ram.h** | nav_ram.h | `NavRamPoint_t` 结构体（x/y/heading/target_speed） |
| **nav_ram.c** | nav_ram.c | 打点时读取 `inertial_nav.x/y` 的方式 |
| **nav_options.h** | nav_options.h | 科目方案选择宏定义 |

### 七、EKF模块（了解角度来源）

| 文件 | 路径 | 阅读重点 |
|------|------|----------|
| **ekf.h** | ekf.h | `euler_angle.yaw` 偏航角来源、`heading` 磁力计航向角 |

### 八、WiFi协议（数据上报）

| 文件 | 路径 | 阅读重点 |
|------|------|----------|
| **wifi_protocol.h/c** | `code/tools/wifi_protocol.*` | 现有数据上报格式，便于后续添加GPS融合调试数据 |

### 九、参考文档

| 文件 | 路径 | 阅读重点 |
|------|------|----------|
| **GPS循迹与惯导循迹实现说明.md** | 项目根目录 | GPS与惯导现有实现思路 |
| **双轮足并联腿机器人结构与控制架构说明.md** | 项目根目录 | 整体控制架构 |
| **双轮足打滑检测机制说明.md** | docs | 打滑检测详细逻辑 |

---

## 🔑 关键代码段定位

**1. 当前融合公式**（需修改）：
```c
// inertial_nav.c 第150-160行
float alpha = (inertial_nav.slip_flag == 1) ? 0.3f : NAV_ALPHA_VEL;
float v_pred = inertial_nav.vx_body + acc_lon_forward * NAV_DT;
inertial_nav.vx_body = alpha * v_wheel_avg + (1.0f - alpha) * v_pred;
```

**2. GPS速度读取方式**（参考）：
```c
// plan1_gnss.c
float GpsNavCurrentSpeedMps(void) { return gnss.speed; }
```

**3. GPS航向角计算**（参考）：
```c
// plan1_gnss.c 第90-110行
GpsNav_GetCurrentHeadingDeg()  // 使用陀螺仪+初始GPS航向角偏移
```

---

## ⚠️ 执行前需确认的问题

1. **坐标系对齐**：GPS 的地理航向与惯导的 x(后)/y(右)如何转换？（本版方案主要通过 heading 将 gnss.speed 投影到车身纵向）
2. **打滑场景识别**：掉头、绕桩的具体触发条件是什么？用现有 `slip_flag` 还是新增标志？（这个坚决不能用slip_flag，现有判断打滑逻辑完全不可靠，用上面提到的曲率前瞻）
3. **GPS更新频率**：GPS数据更新频率是多少？是否需要插值？（具体上面提到了）
4. **权重动态调整策略**：具体权重数值如何设定？（目前采用上面提到的权重）