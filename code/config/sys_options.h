#ifndef __SYS_OPTIONS_H__
#define __SYS_OPTIONS_H__

<<<<<<< HEAD
#define WIFI_USE 0 // 【全局开关】选择是否使用WIFI模块，0表示不使用，1表示使用
#define WIFI_IMAGE_SEND 0 // 【全局开关】选择是否使用WIFI回传摄像机图像，0表示不使用，1表示使用。只有当WIFI_USE和它均为1时有效
=======
#define WIFI_USE 1 // 【全局开关】选择是否使用WIFI模块，0表示不使用，1表示使用
#define WIFI_IMAGE_SEND 1 // 【全局开关】选择是否使用WIFI回传摄像机图像，0表示不使用，1表示使用。只有当WIFI_USE和它均为1时有效
>>>>>>> 0316新车调试pid分支
#define DEBUG_DISPLAY 1                  // 【全局开关】1:开启屏幕调试显示  0:关闭
#define REMOTE_CONTROL 0                 //【全局开关】1：开启遥控器 0:关闭
#define DEBUG_LOG_ENABLE 0 // 【全局开关】1开启串口调试日志，0关闭串口调试日志，【提醒！！！】比赛时候编译烧录代码前，请务必关闭，其影响性能
#define IMU_CATEGORY 1//【全局开关】1:imu660ra  2:imu660rb 3:imu963ra 注：imu660ra被赛事禁用
#define SUBS_CATEGORY 1  //【遥控器选择】1.旧遥控器2.新遥控器。选反会导致前进后退相反
// ---------------- plan 配置 ----------------
#define CURRENT_NAV_PLAN   NAV_PLAN_1   // 【全局开关】【暂时未使用】在这里切换科目几，科目一为NAV_PLAN_1，科目二NAV_PLAN_2，科目三NAV_PLAN_3

#endif // __SYS_OPTIONS_H__
