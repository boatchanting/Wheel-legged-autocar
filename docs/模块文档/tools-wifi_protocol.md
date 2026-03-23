# tools/wifi_protocol 模块文档

## 1. 模块作用

`wifi_protocol` 模块定义并发送自定义 WiFi 数据帧，用于把惯导 + GNSS 组合数据上传到上位机。  
对应源码：`code/tools/wifi_protocol.h`、`code/tools/wifi_protocol.c`。

## 2. 帧格式常量

- 帧头：`WIFI_FRAME_HEAD1 = 0x5A`、`WIFI_FRAME_HEAD2 = 0xA5`
- 帧尾：`WIFI_FRAME_TAIL = 0xED`
- 命令字：`WIFI_CMD_DATA_PACKET = 0x01`（组合数据包）
- 缓冲区：`WIFI_TX_BUFFER_SIZE = 256`

## 3. 关键接口

- `wifi_protocol_send_data()`
  - 自动读取全局导航/GNSS数据并按协议打包发送。

## 4. 发送内容（概要）

当前协议包含：

- 系统时间戳（`loop_counter`）
- 惯导位置（`nav.x/nav.y`）
- GNSS 时间、状态、经纬度（含 double 精度值）
- 速度、方向、天线测向、卫星数、高度等

## 5. 接入建议

- 在固定周期任务中调用 `wifi_protocol_send_data()`（常见 10ms/20ms/50ms）。
- 上位机解析应严格按字节序和字段长度实现。
- 新增字段时建议同步升级命令字或版本号，避免解析歧义。
