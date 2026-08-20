#ifndef _COMMON_H_
#define _COMMON_H_

#include "stdint.h"
#include "stdbool.h"

//==========================================
// 宏定义
//==========================================
#define EXTI_PORT20_0              (P20_0) // 外部中断端口定义,用于惯性导航录制
#define EXTI_PORT20_1              (P20_1) // 外部中断端口定义,用于惯性导航停止录制,停止录制即开启ram转flash的数据压缩储存
#define EXTI_PORT20_2              (P20_2) // 外部中断端口定义,用于惯性导航开始复现
#define EXTI_PORT20_3              (P20_3) // 外部中断端口定义,用于惯性导航停止复现

//==========================================
// 全局变量声明
//=========================================

extern int g_motor_enable;       // 1: 电机安全使能, 0: 车端保护/急停关闭
extern volatile bool g_fallen;   // true: 主动倒下/保持倒下, false: 主动起立

// 【nav】导航记录控制标志位
extern volatile uint8_t g_nav_start_recording; // 1: 开始录制, 0: 清除开始录制标记
extern volatile uint8_t g_nav_recording;       // 1: 正在记录 RAM, 0: 停止记录
extern volatile uint8_t g_save_flash_request;  // 1: 请求将 RAM 数据存入 Flash
extern volatile uint8_t g_load_flash_request;  // 1: 请求将 Flash 数据存入 RAM
extern volatile uint8_t g_replay_start_request;
extern volatile uint8_t g_replay_stop_request;

#include "config/sys_options.h"

#if (LAUNCH_STRATEGY_SELECT == 1)
// 【直立发车 / 航向校准】标志位与主循环蜂鸣器请求
extern volatile bool g_turn_loop_disabled;              // 1: 禁用转向环(手动对准航向), 0: 正常转向环
extern volatile uint8_t g_upright_long_short_long_request; // 1: 请求主循环播放长-短-长提示音
extern volatile uint8_t g_upright_single_beep_request;     // 1: 请求主循环播放单声短鸣
extern volatile uint8_t g_upright_beep_done;               // 1: 主循环长-短-长提示音播放完毕
#endif

// ... 声明其他你项目中需要全局访问的变量 ...

#endif // _COMMON_H_
