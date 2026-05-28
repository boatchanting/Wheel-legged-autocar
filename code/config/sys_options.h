#ifndef __SYS_OPTIONS_H__
#define __SYS_OPTIONS_H__

#define WIFI_USE 1 // 【WIFI总开关】选择是否使用WIFI模块，0表示不使用，1表示使用
#define WIFI_CORE_SELECT 0 // 【WIFI核心选择】0表示0核使用WIFI，1表示1核使用WIFI
#define WIFI_PROTOCOL_SELECT 2 // 【WIFI协议选择】1表示逐飞助手，2表示我们的自定义协议
#define G_MOTOR_ENABLE_INIT 1 // 【电机使能初值】控制g_motor_enable上电默认状态，1为使能，0为关机
#define DEBUG_DISPLAY 1                  // 【全局开关】1:开启屏幕调试显示  0:关闭
#define DEBUG_DISPLAY_CORE_SELECT 0      // 【显示核心选择】0: 0核独占屏幕  1: 1核视觉屏幕
#define CAMERA_MENU_REFRESH_DIV 4U       // 【图像刷新分频】1核屏幕分频，每N帧刷新一次图像画面（降低刷新频率以减少CPU占用）
#define CAMERA_MENU_DEBUG_LOG_ENABLE 0U  // 【图像调试日志】1: 打开1核 camera_menu 串口调试打印  0: 关闭
#define CAMERA_MENU_DEBUG_LOG_DIV 20U    // 【日志打印分频】1核 camera_menu 串口打印分频，每N帧打印一次

#define REMOTE_CONTROL 1                 //【全局开关】1：开启遥控器 0:关闭
#define DEBUG_LOG_ENABLE 0 // 【全局开关】1开启串口调试日志，0关闭串口调试日志，【提醒！！！】比赛时候编译烧录代码前，请务必关闭，其影响性能
#define ACCEL_FF_ENABLE 1U // 【加速前馈总开关】1:启用复刻起步/急加速前馈  0:完全关闭加速前馈
#define ACCEL_FF_MODE 1U // 【加速前馈模式】0:关闭  1:直接叠加 PWM 前馈  2:加速时临时增强舵机速度环 Kp
#define ACCEL_FF_BUZZER_ENABLE 0U // 【全局开关】1:大幅加速前馈触发时蜂鸣提示  0:关闭
#define ROLL_BALANCE_ENABLE_INIT 0U //【全局开关】主动侧倾平衡环初始使能状态，1为上电默认使能，0为上电默认关闭
#define IMU_CATEGORY 3//【全局开关】1:imu660ra  2:imu660rb 3:imu963ra 注：imu660ra被赛事禁用
#define IMU_REFRESH_TEST_ENABLE 0 // 1: 上电后测试IMU刷新频率，运行10秒后串口打印一次结果
#define SUBS_CATEGORY 1  //【遥控器选择】1.旧遥控器2.新遥控器。选反会导致前进后退相反
// ---------------- plan 配置 ----------------
#define GNSS_NAV 0 // 【全局开关】gps寻迹还是惯导寻迹，现阶段暂时还没联合(date0511)，联合后考虑去除该开关，1表示使用gnss寻迹，0表示不使用gnss寻迹，惯导开关常开
#define CURRENT_NAV_PLAN   2   // 【全局开关】在这里切换科目几，科目一为1，科目二2，科目三3，nav_replay模版函数99，每个科目的主要逻辑会单独优化，上层控制参数层不共享，互不干扰，后面做到各自独立优化，这个开关现在对惯导寻迹和gps方案均有效(date0520)
/*
【科目一优化与拆分】
1.1.惯导条件下，去除了任务点搜索code/navigation/nav_replay.c
1.2.惯导模式下，注释了所有状态机代码

【科目二优化与拆分】
2.1.惯导条件下，状态机触发不用先角度对准code/navigation/nav_replay.c
2.2.惯导条件下，状态机进保留了雷区状态机code/navigation/nav_replay.c

*/
















// ==================== 屏幕与图像调试配置，用户不要关心这段代码 ====================
#if (DEBUG_DISPLAY && (DEBUG_DISPLAY_CORE_SELECT != 0) && (DEBUG_DISPLAY_CORE_SELECT != 1))
#error "DEBUG display config error: invalid DEBUG_DISPLAY_CORE_SELECT."
#endif

#if (CAMERA_MENU_REFRESH_DIV == 0U)
#error "DEBUG display config error: CAMERA_MENU_REFRESH_DIV must be greater than 0."
#endif

#if (CAMERA_MENU_DEBUG_LOG_DIV == 0U)
#error "DEBUG display config error: CAMERA_MENU_DEBUG_LOG_DIV must be greater than 0."
#endif

#define DEBUG_DISPLAY_CORE0 (DEBUG_DISPLAY && (DEBUG_DISPLAY_CORE_SELECT == 0))
#define DEBUG_DISPLAY_CORE1 (DEBUG_DISPLAY && (DEBUG_DISPLAY_CORE_SELECT == 1))

#endif // __SYS_OPTIONS_H__
