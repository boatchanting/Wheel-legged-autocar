# tools/wifi 模块文档

## 1. 模块作用

`wifi` 模块负责 WiFi 模块初始化、TCP 建链、摄像头图像发送，以及与上位机联调参数更新。  
对应源码：`code/tools/wifi.h`、`code/tools/wifi.c`。

## 2. 常用配置宏

- `WIFI_SSID_TEST` / `WIFI_PASSWORD_TEST`：接入点账号密码
- `TCP_TARGET_IP` / `TCP_TARGET_PORT`：目标 TCP Server
- `WIFI_LOCAL_PORT`：本地端口
- `INCLUDE_BOUNDARY_TYPE`：图像边界数据打包类型

## 3. 关键接口

- `wifi_init()`：初始化 WiFi 模块
- `wifi_connect_tcp_server()`：连接 TCP 服务器
- `wifi_camera_init()`：初始化摄像头图像发送
- `wifi_update_pid_params()`：处理上位机下发参数并更新本地
- `encode_double_to_two_floats()`：辅助拆分双精度数据

## 4. 推荐初始化顺序

1. `wifi_init()`
2. `wifi_connect_tcp_server()`
3. `wifi_camera_init()`

## 5. 注意事项

- 先确认 `zf_device_wifi_spi.h` 中目标 IP/端口配置一致。
- 建议保留调试串口日志，便于定位连接失败原因。
- 网络环境复杂时帧率可能下降，优先排查热点干扰与供电稳定性。
