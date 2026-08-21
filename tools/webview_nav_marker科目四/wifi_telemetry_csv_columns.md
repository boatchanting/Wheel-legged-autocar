# WiFi 遥测 CSV 列说明

本文档对应 `nav_marker_host.py` 生成的 `wifi_telemetry_*.csv`，以及下位机
`code/tools/wifi_protocol.c` 的遥测协议 V3。

- 遥测 payload 固定为 **108 字节**，采用**小端序**。
- CSV 使用 UTF-8 with BOM 编码，可直接用 Excel 打开。
- `payload_hex` 保存收到的完整原始 payload，便于离线复核协议。
- 位置单位均为 mm；角度单位均为 °；速度单位均为 mm/s（除非字段另有说明）。

## CSV 文件公共列

|列名|类型|单位|说明|
|---|---|---|---|
|`received_at_unix_ms`|整数|ms|上位机接收该帧时的 Unix 时间戳（毫秒）。|
|`received_at`|字符串|-|上位机本地接收时间，格式 `YYYY-MM-DD HH:MM:SS`。|
|`payload_hex`|十六进制字符串|-|完整遥测 payload 的十六进制文本；当前协议应为 216 个十六进制字符。|
|`payload_size`|整数|byte|遥测 payload 长度；协议 V3 固定为 `108`。|

## 车辆遥测列（按 payload 发送顺序）

|序号|列名|类型|单位|说明|
|---:|---|---|---|---|
|1|`loop`|`uint32`|ms|小车 `loop_counter`，用于帧顺序、周期与时间差分析。
|2|`nav_x`|`float32`|mm|视觉融合导航世界坐标 X，即 `nav_vision_fusion_x`。(需要注意回传的是融合轨迹会被视觉更新的，所以可能会有突变)|
|3|`nav_y`|`float32`|mm|视觉融合导航世界坐标 Y，即 `nav_vision_fusion_y`。(需要注意回传的是融合轨迹会被视觉更新的，所以可能会有突变)|
|4|`vx_body`|`float32`|mm/s|车体坐标系 X 向速度，即 `inertial_nav.vx_body`。|
|5|`vy_body`|`float32`|mm/s|车体坐标系 Y 向速度，即 `inertial_nav.vy_body`。|
|6|`heading`|`float32`|°|现在这个量没有调试好，别用|
|7|`relative_yaw`|`float32`|°|相对偏航角，复现轨迹开始时为0，即 `inertial_nav.relative_yaw`。|
|8|`mark_trigger`|`uint8`|-|设备打点触发标志；为 `1` 时上位机将当前坐标作为标记点加入列表。发送后下位机会清零。|
|9|`point_type`|`uint8`|-|上位机打点所用点类型，来源为 `robot_ctrl.point_type`；**不要与** `nav_replay_point_type` 混淆。|
|10|`pid_mode`|`uint8`|-|当前已应用的控制模式 `g_control_mode_applied`。|
|11|`slip_flag`|`uint8`|-|惯导滑移标志，是通过雷区状态机给的，其他地方没用 `inertial_nav.slip_flag`；`0` 表示未标记滑移，非 0 表示存在滑移/特殊工况。|
|12|`nav_replay_point_type`|`uint8`|-|导航回放状态机当前点类型，即 `g_current_point_type`。与 `point_type` 分别表示“回放执行点”和“上位机打点点”。|
|13|`g_replay_state`|`uint8`|-|导航回放状态：`0=REPLAY_IDLE`、`1=REPLAY_RUNNING`、`2=REPLAY_FINISHED`。重要！！！通过这个数据去筛选小车自己跑的片段！|
|14|`err_degree`|`float32`|°|导航/特殊任务给底盘的预期转向控制量。|
|15|`minefield_is_active`|`uint8`|-|雷区状态机是否运行，`Minefield_Is_Active()` 返回值。|
|16|`g_special_action_trigger`|`uint8`|-|导航特殊动作接管开关；非 0 表示特殊任务接管导航。|
|17|`bumpy_road_is_active`|`uint8`|-|颠簸路段状态机是否运行，`BumpyRoad_Is_Active()` 返回值。|
|18|`vision_bridge_task_is_active`|`uint8`|-|单边桥视觉状态机是否运行，`VisionBridgeTask_IsActive()` 返回值。|
|19|`vision_slope_task_is_active`|`uint8`|-|斜坡视觉状态机是否运行，`VisionSlopeTask_IsActive()` 返回值。|
|20|`vision_three_stage_control_is_active`|`uint8`|-|三级跳视觉状态机是否运行，`VisionThreeStageControl_IsActive()` 返回值。|
|21|`euler_roll`|`float32`|°|EKF 欧拉横滚角 `euler_angle.roll`。|
|22|`euler_pitch`|`float32`|°|EKF 欧拉俯仰角 `euler_angle.pitch`。正负方向遵循当前固件 EKF 坐标定义。|
|23|`euler_yaw`|`float32`|°|EKF 欧拉偏航角 `euler_angle.yaw`。|
|24|`imu_acc_x`|`float32`|÷ 4098 → 单位 g|EKF IMU 加速度计 X 轴数据 `imu_data.acc_x`。|
|25|`imu_acc_y`|`float32`|÷ 4098 → 单位 g|EKF IMU 加速度计 Y 轴数据 `imu_data.acc_y`。|
|26|`imu_acc_z`|`float32`|÷ 4098 → 单位 g|EKF IMU 加速度计 Z 轴数据 `imu_data.acc_z`。|
|27|`imu_gyro_x`|`float32`|÷ 14.3 → 单位 °/s|EKF IMU 陀螺仪 X 轴角速度 `imu_data.gyro_x`。|
|28|`imu_gyro_y`|`float32`|÷ 14.3 → 单位 °/s|EKF IMU 陀螺仪 Y 轴角速度 `imu_data.gyro_y`。|
|29|`imu_gyro_z`|`float32`|÷ 14.3 → 单位 °/s|EKF IMU 陀螺仪 Z 轴角速度 `imu_data.gyro_z`。|
|30|`imu_grav_x`|`float32`|暂时不要用|EKF 估计的重力单位向量 X 分量 `imu_data.grav_x`。|
|31|`imu_grav_y`|`float32`|暂时不要用|EKF 估计的重力单位向量 Y 分量 `imu_data.grav_y`。|
|32|`imu_grav_z`|`float32`|暂时不要用|EKF 估计的重力单位向量 Z 分量 `imu_data.grav_z`。|
|33|`servo_angle_rf`|`float32`|°|右前（RF）舵机当前缓存角度。|
|34|`servo_angle_rr`|`float32`|°|右后（RR）舵机当前缓存角度。|
|35|`servo_angle_lf`|`float32`|°|左前（LF）舵机当前缓存角度。|
|36|`servo_angle_lr`|`float32`|°|左后（LR）舵机当前缓存角度。|
|37|`target_speed_set`|`float32`|转/分|小车车机实时计算目标转速|
|38|`speed_L`|`float32`|转/分|乘以4.79得到左轮速度mm/s，向前为正|
|39|`speed_R`|`float32`|转/分|乘以4.79得到右轮速度mm/s，向前为负|
|40|`pwm_left`|`float32`|-|左轮 PWM 占空比|
|41|`pwm_right`|`float32`|-|右轮 PWM 占空比|


## 点类型取值

`point_type` 和 `nav_replay_point_type` 使用同一组路线点枚举值，但来源和用途不同。

|值|含义|
|---:|---|
|0|普通路径点（PATH）|
|1 / 10|雷区进入 / 退出|
|2 / 20|斜坡进入 / 退出|
|3 / 30|三级跳进入 / 退出|
|4 / 40|单边桥进入 / 退出|
|5 / 50|颠簸路段进入 / 退出|

> 舵机角度来自软件最近一次 PWM 占空比换算得到的缓存值，不是外部角度传感器的直接测量值。
