#ifndef __SYS_OPTIONS_H__
#define __SYS_OPTIONS_H__

// WIFI_USE mode:
// 0  -> use core0 default WiFi
// 1  -> use core1 WiFi
// 10 -> use core0 custom WiFi
// others -> disable WiFi
#define WIFI_USE 255// 【全局开关】选择WIFI使用模式，0:core0默认模式，1:core1模式，10:core0自定义模式，其他值:关闭wifi功能
#define WIFI_IMAGE_SEND 0 // 【全局开关】1:开启图像发送功能 0:关闭图像发送功能，开启后会占用较多性能，比赛时候建议关闭
#define WIFI_USE_CORE0_DEFAULT   (0 == WIFI_USE)
#define WIFI_USE_CORE1           (1 == WIFI_USE)
#define WIFI_USE_CORE0_CUSTOM    (10 == WIFI_USE)
#define WIFI_USE_CORE0_ANY       (WIFI_USE_CORE0_DEFAULT || WIFI_USE_CORE0_CUSTOM)
#define WIFI_USE_ANY             (WIFI_USE_CORE0_ANY || WIFI_USE_CORE1)
#define DEBUG_DISPLAY 1                  // 【全局开关】1:开启屏幕调试显示  0:关闭
#define REMOTE_CONTROL 1                 //【全局开关】1：开启遥控器 0:关闭
#define DEBUG_LOG_ENABLE 0 // 【全局开关】1开启串口调试日志，0关闭串口调试日志，【提醒！！！】比赛时候编译烧录代码前，请务必关闭，其影响性能
#define IMU_CATEGORY 3//【全局开关】1:imu660ra  2:imu660rb 3:imu963ra 注：imu660ra被赛事禁用
#define SUBS_CATEGORY 1  //【遥控器选择】1.旧遥控器2.新遥控器。选反会导致前进后退相反
// ---------------- plan 配置 ----------------
#define CURRENT_NAV_PLAN   1   // 【全局开关】在这里切换科目几，科目一为1，科目二2，科目三3，每个科目的主要逻辑会单独优化，上层控制参数层不共享，现阶段尽量做到互不干扰(0429)，后面做到各自独立优化
/*
【科目一优化与拆分】
1.1. 惯导条件下，去除了任务点搜索 code/navigation/nav_replay.c
1.2. 惯导模式下，注释了所有状态机代码

【科目二优化与拆分】
2.1. 惯导条件下，状态机触发不用先角度对准 code/navigation/nav_replay.c
2.2. 惯导条件下，状态机里保留了雷区状态机 code/navigation/nav_replay.c
*/

#endif // __SYS_OPTIONS_H__
