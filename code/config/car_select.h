#ifndef __CAR_SELECT_H__
#define __CAR_SELECT_H__

#define CAR_SELECT 4 // 【全局配置】选择哪个小车，这里改动之后会全局改变对应的引脚，重新编译会自动选择

//【请在下方撰写小车具体包含什么硬件及对应的配置在哪里】

/* 
0代表 【学习板小车】 对应板子 【学习板 v1.2】
学习板小车内容
1.四个舵机的引脚及占90度时候的占空比，极性，每次拆装都需要重新测试，来自servo/servo.h
2.屏幕的引脚配置，来自zf_device/zf_device_ips200.h
3.无线模块配置，来自zf_device/zf_device_wifi_spi.h
4.按钮模块配置，来自zf_device/zf_device_key.h，需要更改按钮和结构体
5.pid参数配置，来自code/calculate/pid-new.h
6.GNSS的配置：zf_driver_uart.c末尾的检测不动，参考语雀文档，来自zf_driver/zf_driver_uart.c;zf_driver_uart.h更改;zf_device_gnss.h中引脚和uart配置

*/

/* 
1代表【我们板小车1】 对应板子 【2026/01 队名还未定】【此板子没有wifi，暂时弃用】 不要开启这个小车
学习板小车内容


*/

/* 
2代表 【2026/1/31新车】 对应板子 【2026/01/16 锦鲤跃龙门】
我们的板子1小车内容
1.四个舵机的引脚及占90度时候的占空比，极性，每次拆装都需要重新测试，来自servo/servo.h
2.屏幕的引脚配置，来自zf_device/zf_device_ips200.h
3.无线模块配置，来自zf_device/zf_device_wifi_spi.h
4.按钮模块配置，来自zf_device/zf_device_key.h，需要更改按钮和结构体
5.pid参数配置，来自code/calculate/pid-new.h
6.GNSS的配置：zf_driver_uart.c末尾的检测要注释掉，参考语雀文档，来自zf_driver/zf_driver_uart.c;zf_driver_uart.h更改;zf_device_gnss.h中引脚和uart配置
*/

/*
3代表 【2026/3/30新车】 对应板子 【2026/03/24 最后的舵机v腿】
1.四个舵机的引脚及占90度时候的占空比，极性，每次拆装都需要重新测试，来自servo/servo.h
2.屏幕的引脚配置，来自zf_device/zf_device_ips200.h
3.无线模块配置，来自zf_device/zf_device_wifi_spi.h
4.按钮模块配置，来自zf_device/zf_device_key.h，需要更改按钮和结构体
5.pid参数配置，来自code/calculate/pid-new.h
6.GNSS的配置：zf_driver_uart.c末尾的检测要注释掉，参考语雀文档，来自zf_driver/zf_driver_uart.c;zf_driver_uart.h更改;zf_device_gnss.h中引脚和uart配置
7.摄像头的配置：来自zf_device_mt9v03x.h
*/

/*
4代表 【小车4】 初版参数与小车3相同，待实车标定。
需要重新确认：舵机中位/极性、PID、IMU安装方向、轮径/轮距、
GNSS、WiFi、屏幕、按键、摄像头和串口引脚配置。
*/

#endif // __CAR_SELECT_H__
