# tools/webview_nav_marker 双向通信思路稿

## 1. 目标

当前新的 WebView 上位机 `tools/webview_nav_marker/nav_marker_host.py` 已经可以作为 TCP Server 接收小车发来的惯导/GNSS/打点状态数据，并在前端页面 `tools/webview_nav_marker/nav_marker.html` 中实时显示轨迹和标记点。

现在希望在这个基础上，再让上位机能够主动向小车发送信息，而不是只接收信息。这个“上位机 -> 小车”的能力，优先服务于以下需求：

- 发送简单调参命令
- 发送状态机触发命令
- 发送打点/路径相关控制命令
- 后续扩展为导航参数、调试开关等

这份文档先给出思路稿，不直接改代码。

## 2. 当前现状

### 2.1 新上位机链路

`tools/webview_nav_marker/nav_marker_host.py` 当前的工作方式：

- 本机作为 TCP Server，监听 `192.168.137.1:8086`
- 小车作为 TCP Client 主动连上来
- 上位机只做 `recv()`，解析 `0x5A 0xA5 ... checksum 0xED` 格式的数据帧
- 解析后通过 `pywebview` 暴露给前端页面

当前 `Api` 里暴露的接口主要是：

- `get_new_data()`
- `get_status()`
- `clear_history()`
- `export_mark_points_csv()`

也就是说，现阶段新的 WebView 上位机还没有“发送到小车”的后端接口，也没有对应的前端按钮/表单。

### 2.2 小车当前 WiFi 侧现状

项目里现在实际并存两套思路：

#### A. 自定义遥测协议

对应文件：

- `code/tools/wifi_protocol.h`
- `code/tools/wifi_protocol.c`

这套代码当前只负责“小车 -> 上位机”发送，核心函数是：

- `wifi_protocol_send_data()`

当前帧格式大致是：

- 帧头：`0x5A 0xA5`
- 功能字：当前只定义了 `WIFI_CMD_DATA_PACKET = 0x01`
- 长度：1 字节 payload 长度
- payload：惯导、GNSS、heading、relative_yaw、mark_trigger、point_type
- 校验：1 字节累加和
- 帧尾：`0xED`

注意：目前这套自定义协议只有发送打包函数，没有对应的“接收并解析 PC 指令帧”的模块入口。

#### B. 逐飞库参数下发方案

对应文件：

- `code/tools/wifi.h`
- `code/tools/wifi.c`

这里已经存在一套“上位机下发参数到 MCU”的能力，核心入口是：

- `wifi_update_pid_params()`

函数内部调用：

- `seekfree_assistant_data_analysis()`

并根据：

- `seekfree_assistant_parameter_update_flag[i]`
- `seekfree_assistant_parameter[i]`

来更新本地变量，例如：

- `target_speed_set`
- `pid_servo_speed.kp`
- `pid_servo_speed.ki`
- `vision_detected_jump_point`
- `vision_detected_bumpy_point`
- `g_motor_enable`

但是从 `user/main_cm7_0.c` 当前代码看，主循环里已经启用了：

- `wifi_protocol_send_data()`

而：

- `wifi_update_pid_params()`

目前还是注释状态，所以现状更偏向“单向上报”，没有真正跑起来“新上位机主动下发”。

## 3. 需求拆分

“上位机给小车发送信息”最好不要一开始就做成一个很大的万能协议，建议先拆成两层：

### 3.1 第一层：先能发简单命令

先让新上位机支持以下最小功能：

- 发送某个参数编号 + 参数值
- 发送某个布尔触发命令
- 发送某个枚举状态切换命令

例如：

- 设置 `target_speed_set`
- 设置 `pid_servo_speed.kp`
- 设置 `pid_servo_speed.ki`
- 触发 `vision_detected_jump_point`
- 触发 `vision_detected_bumpy_point`
- 开关 `g_motor_enable`

这样最容易先跑通链路。

### 3.2 第二层：再扩展为结构化控制

在最小链路打通后，再考虑扩展为：

- 路径下发
- 标记点下发
- 导航模式切换
- 参数批量同步
- 参数读回确认

## 4. 推荐路线

推荐采用：

**保留现有自定义遥测协议不动，同时新增一套简洁的 PC -> MCU 控制帧。**

原因如下：

- `nav_marker_host.py` 现在已经完整跑在自定义协议上，继续沿用成本最低
- `wifi_protocol.c` 的帧头、校验、长度逻辑已经成型，扩展更自然
- 逐飞库参数下发可以继续保留，但它更像“兼容已有逐飞助手生态”的能力
- 新 WebView 上位机后面要做的事情不止 PID 调参，还可能包括打点确认、控制命令、路径操作，因此自定义控制帧更灵活

结论：

- 短期主线：新上位机走自定义双向协议
- 兼容保留：`wifi.c` 的逐飞参数下发能力作为备用/参考，不先删

## 5. 建议的协议扩展方式

### 5.1 保持当前遥测帧不变

当前 `0x01` 数据帧继续用于：

- 小车 -> 上位机 的状态上报

这样不会影响已有轨迹显示和打点逻辑。

### 5.2 新增 PC -> MCU 控制帧类型

建议在 `code/tools/wifi_protocol.h` 中继续扩展功能字，例如：

- `0x01`：遥测数据帧，MCU -> PC
- `0x10`：设置单个参数，PC -> MCU
- `0x11`：触发单个命令，PC -> MCU
- `0x12`：批量参数下发，PC -> MCU
- `0x80`：应答帧，MCU -> PC

这里只是建议，不一定非要一次全上。第一版最小实现只做：

- `0x10`：设置参数
- `0x11`：触发命令
- `0x80`：应答

就够了。

### 5.3 第一版 payload 建议

#### `0x10` 设置参数帧

建议 payload：

- `param_id`：`uint8`
- `value`：`float`

用途：

- 给某个参数编号设置一个浮点值

例如：

- `param_id=0` -> `target_speed_set`
- `param_id=1` -> `pid_servo_speed.kp`
- `param_id=2` -> `pid_servo_speed.ki`
- `param_id=3` -> `g_motor_enable`

说明：

- `g_motor_enable` 虽然本质是布尔量，也可以先按 `float` 传，车端做阈值判断
- 这样第一版协议最简单，解析也统一

#### `0x11` 触发命令帧

建议 payload：

- `cmd_id`：`uint8`
- `arg0`：`float`

用途：

- 用于“触发一次动作”或“切换一次状态”

例如：

- `cmd_id=0` -> 触发跳跃点
- `cmd_id=1` -> 触发颠簸路段状态机
- `cmd_id=2` -> 清空某些运行态缓存

### 5.4 应答帧建议

如果要让调试更稳，建议 MCU 在收到参数/命令后，回一个简短 ACK：

- `ack_cmd`
- `ack_id`
- `status`

例如：

- 成功应用参数
- 参数编号非法
- 数值越界
- 当前状态不允许执行该命令

这样上位机按钮点击后能明确显示“是否生效”。

## 6. 软件结构建议

### 6.1 上位机 `nav_marker_host.py`

建议新增三部分能力：

#### A. 保存当前 TCP 连接

现在 `accept()` 后虽然拿到了 `conn`，但只在接收循环内局部使用。后续需要增加一个全局连接引用，例如：

- `client_conn`
- `client_lock`

这样后端 API 调用时，才能把数据发回当前已连接的小车。

#### B. 新增发送帧打包函数

建议新增类似：

- `build_tx_frame(cmd, payload_bytes)`
- `send_frame(cmd, payload_bytes)`

职责：

- 拼接帧头
- 填长度
- 算校验
- 发送到当前连接

注意点：

- 发送与接收可能并发，需要加锁
- 小车断开后要清空连接句柄
- 发送失败时要给前端返回明确错误

#### C. 在 `Api` 中暴露发送接口

建议新增 `pywebview` API，例如：

- `set_param(param_id, value)`
- `send_command(cmd_id, value=0.0)`

后续前端按钮直接调用这些接口即可。

### 6.2 前端 `nav_marker.html`

第一版不需要把页面做得很复杂，先增加一个“小控制面板”即可，内容可以包括：

- 参数编号输入框
- 参数值输入框
- 发送按钮
- 若干常用快捷按钮

例如：

- 设置目标速度
- 设置舵机 `kp`
- 设置舵机 `ki`
- 电机使能 / 失能
- 触发跳跃点
- 触发颠簸状态机

UI 上建议优先做成“少量常用按钮 + 一个通用调试入口”，这样便于边调边扩展。

### 6.3 小车侧 `wifi_protocol`

建议把 `wifi_protocol` 从“只有发送”扩展成“收发都归这里管”。

建议新增内容：

- 接收缓冲区
- 帧解析状态机
- 命令分发函数

可考虑新增函数：

- `wifi_protocol_poll_rx()`
- `wifi_protocol_parse_byte()`
- `wifi_protocol_handle_frame(cmd, payload, len)`
- `wifi_protocol_apply_param(param_id, value)`
- `wifi_protocol_send_ack(...)`

这样职责会更清晰。

### 6.4 小车侧 `wifi.c`

`wifi.c` 不建议现在硬删，理由：

- 它已经整理好了逐飞参数下发的映射关系
- 有些参数编号映射可以直接参考
- 后面如果仍要兼容逐飞助手，上层还能继续调用

更合适的做法是：

- 把 `wifi.c` 视为“逐飞库兼容层”
- 把新的 WebView 双向协议收发主线放到 `wifi_protocol.c`

## 7. 第一阶段最小可落地方案

建议先做一个最小版本，只实现以下闭环：

### 7.1 上位机侧

- 在 `nav_marker_host.py` 中保存当前 `conn`
- 增加 `send_frame()`
- 在 `Api` 中增加：
  - `set_param(param_id, value)`
  - `send_command(cmd_id, value)`
- 在 `nav_marker.html` 中先做 3~5 个按钮

### 7.2 小车侧

- 在 `wifi_protocol.h/.c` 中新增命令字定义
- 增加接收解析逻辑
- 先只支持少量变量映射：
  - `target_speed_set`
  - `pid_servo_speed.kp`
  - `pid_servo_speed.ki`
  - `g_motor_enable`
  - `vision_detected_jump_point`
  - `vision_detected_bumpy_point`

### 7.3 主循环接入点

在和 `wifi_protocol_send_data()` 同一个周期里，再加一个轮询函数，例如：

- `wifi_protocol_poll_rx()`

这样每个周期既上报数据，也处理上位机下发命令。

## 8. 为什么不建议第一步直接复用 `wifi_update_pid_params()`

`wifi_update_pid_params()` 可以作为参考，但不建议直接把新 WebView 上位机硬接到它上面，原因有几个：

- 它依赖逐飞助手的数据格式和解析函数
- 新上位机不是逐飞助手，前端页面也不是按那套协议设计的
- 目前你们的新上位机主线已经是自定义数据帧，继续混两套 PC 协议会增加理解成本
- 后续如果要下发的不只是 PID，而是路径、打点、命令、状态切换，自定义帧更容易扩展

所以更合适的定位是：

- `wifi_update_pid_params()`：参考现有“参数编号 -> 本地变量”的映射经验
- `wifi_protocol`：承担新系统真正的双向协议

## 9. 参数编号建议草案

第一版可以先固定如下：

- `0`：`target_speed_set`
- `1`：`pid_servo_speed.kp`
- `2`：`pid_servo_speed.ki`
- `3`：`g_motor_enable`
- `4`：`vision_detected_jump_point`
- `5`：`vision_detected_bumpy_point`

命令编号可以先定义：

- `0`：触发跳跃
- `1`：触发颠簸状态机
- `2`：保留

后续真开始写代码时，建议把“参数编号表”单独写进头文件或文档，避免上位机和下位机各写一份魔法数字。

## 10. 调试顺序建议

建议按以下顺序推进：

1. 先让上位机能给当前 TCP 连接发出一帧固定测试命令
2. 再让小车能识别 `0x10` / `0x11`
3. 先只改一个最容易观察的变量，比如 `g_motor_enable`
4. 加 ACK 回包，确认“收到了”和“应用成功了”是两回事
5. 最后再加前端按钮和常用参数面板

这样排查问题最省时间。否则一上来把前端、后端、协议、车端逻辑一起改，出错时不容易定位。

## 11. 这份思路稿对应的推荐改动清单

后续真正开始实现时，优先会改这些文件：

- `tools/webview_nav_marker/nav_marker_host.py`
- `tools/webview_nav_marker/nav_marker.html`
- `code/tools/wifi_protocol.h`
- `code/tools/wifi_protocol.c`
- `user/main_cm7_0.c`

`code/tools/wifi.c` 和 `code/tools/wifi.h` 主要作为参考，不一定第一步就动。

## 12. 当前建议结论

当前最合适的方向不是把新上位机强行套进逐飞助手协议，而是：

- 保留 `wifi.c` 里的逐飞参数下发能力作为参考和备用
- 以 `wifi_protocol.c + nav_marker_host.py` 为主线，补出一套轻量的双向自定义协议
- 第一版先只做“单参数设置 + 单命令触发 + ACK”

这样能最快把“新上位机主动给小车发信息”跑通，而且后面继续扩展时结构也最清楚。

## 13. 本文档的假设

这份思路稿默认以下前提成立：

- 当前 WiFi 模块建立的是稳定的 TCP 单连接
- 小车仍然主动连接到电脑上位机
- 新上位机会继续沿用现有 `0x5A 0xA5 ... 0xED` 的基本帧格式
- 近期的下发需求以“参数和简单命令”为主，不是一次性整包路径下发

如果后面你决定改成：

- 上位机做 TCP Client
- 小车做 TCP Server
- 或者改用 UDP

那这份方案还要再调整一轮。
