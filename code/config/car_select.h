#ifndef __CAR_SELECT_H__
#define __CAR_SELECT_H__

#define CAR_SELECT 0 // 【全局配置】选择哪个小车，这里改动之后会全局改变对应的引脚，重新编译会自动选择

//【请在下方撰写小车具体包含什么硬件及对应的配置在哪里】

/* 
0代表 【学习板小车】 对应板子 【学习板 v1.2】
学习板小车内容
1.四个舵机的引脚及占90度时候的占空比，极性，每次拆装都需要重新测试，来自servo/servo.h
2.屏幕的引脚配置，来自zf_device/zf_device_ips200.h
3.无线模块配置，来自zf_device/zf_device_wifi_spi.h
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
*/

#endif // __CAR_SELECT_H__