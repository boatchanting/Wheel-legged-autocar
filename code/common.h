#ifndef _COMMON_H_
#define _COMMON_H_

//==========================================
// 宏定义
//==========================================
#define EXTI_PORT20_0              (P20_0) // 外部中断端口定义,用于惯性导航录制
#define EXTI_PORT20_1              (P20_1) // 外部中断端口定义,用于惯性导航停止录制,停止录制即开启ram转flash的数据压缩储存
#define EXTI_PORT20_2              (P20_2) // 外部中断端口定义,用于惯性导航开始复现
#define EXTI_PORT20_3              (P20_3) // 外部中断端口定义,用于惯性导航停止复现

//==========================================
// 全局变量声明
//==========================================
#define DEBUG_LOG_ENABLE 1 // 【全局变量】1开启串口调试日志，0关闭串口调试日志

// 导航记录控制标志位
extern volatile uint8_t g_nav_recording;       // 1: 正在记录 RAM, 0: 停止记录
extern volatile uint8_t g_save_flash_request;  // 1: 请求将 RAM 数据存入 Flash
extern volatile uint8_t g_replay_start_request;
extern volatile uint8_t g_replay_stop_request;
extern volatile uint8_t g_read_test_request;     // 读取测试请求标志（新添加）
extern volatile uint8_t g_clear_flash_request;   // 清空Flash请求标志（新添加）
// ... 声明其他你项目中需要全局访问的变量 ...

#endif // _COMMON_H_