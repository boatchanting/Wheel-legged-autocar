#ifndef __SYS_OPTIONS_H__
#define __SYS_OPTIONS_H__

#define WIFI_USE 1 // 【WIFI总开关】选择是否使用WIFI模块，0表示不使用，1表示使用
#define WIFI_CORE_SELECT 1 // 【WIFI核心选择】0表示0核使用WIFI，1表示1核使用WIFI
#define WIFI_PROTOCOL_SELECT 2 // 【WIFI协议选择】1表示逐飞助手，2表示我们的自定义协议
#define G_MOTOR_ENABLE_INIT 1 // 【电机使能初值】控制g_motor_enable上电默认状态，1为使能，0为关机
#define DEBUG_DISPLAY 1                  // 【全局开关】1:开启屏幕调试显示  0:关闭
#define REMOTE_CONTROL 1                 //【全局开关】1：开启遥控器 0:关闭
#define DEBUG_LOG_ENABLE 0 // 【全局开关】1开启串口调试日志，0关闭串口调试日志，【提醒！！！】比赛时候编译烧录代码前，请务必关闭，其影响性能
#define ACCEL_FF_ENABLE 1U // 【全局开关】1:启用复刻起步/急加速PWM前馈  0:关闭
#define ACCEL_FF_BUZZER_ENABLE 1U // 【全局开关】1:大幅加速前馈触发时蜂鸣提示  0:关闭
#define ROLL_BALANCE_ENABLE_INIT 1U // Rolling balance loop initial enable: 1 on, 0 off
#define IMU_CATEGORY 3//【全局开关】1:imu660ra  2:imu660rb 3:imu963ra 注：imu660ra被赛事禁用
#define IMU_REFRESH_TEST_ENABLE 0 // 1: 上电后测试IMU刷新频率，运行10秒后串口打印一次结果
#define SUBS_CATEGORY 1  //【遥控器选择】1.旧遥控器2.新遥控器。选反会导致前进后退相反
// ---------------- plan 配置 ----------------
#define GNSS_NAV 0 // 【全局开关】gps寻迹还是惯导寻迹，现阶段暂时还没联合(date0511)，联合后考虑去除该开关，1表示使用gnss寻迹，0表示不使用gnss寻迹，惯导开关常开
#define CURRENT_NAV_PLAN   1   // 【全局开关】在这里切换科目几，科目一为1，科目二2，科目三3，nav_replay模版函数99，每个科目的主要逻辑会单独优化，上层控制参数层不共享，互不干扰，后面做到各自独立优化，这个开关现在对惯导寻迹和gps方案均有效(date0520)
/*
【科目一优化与拆分】
1.1.惯导条件下，去除了任务点搜索code/navigation/nav_replay.c
1.2.惯导模式下，注释了所有状态机代码

【科目二优化与拆分】
2.1.惯导条件下，状态机触发不用先角度对准code/navigation/nav_replay.c
2.2.惯导条件下，状态机进保留了雷区状态机code/navigation/nav_replay.c

*/

#endif // __SYS_OPTIONS_H__
