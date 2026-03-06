#ifndef _COMMON_H_
#define _COMMON_H_

//==========================================
// 宏定义
//==========================================


//==========================================
// 全局变量声明
//=========================================

// 【nav】导航记录控制标志位
extern volatile uint8_t g_nav_start_recording; // 1: 开始录制, 0: 清除开始录制标记
extern volatile uint8_t g_nav_recording;       // 1: 正在记录 RAM, 0: 停止记录
extern volatile uint8_t g_save_flash_request;  // 1: 请求将 RAM 数据存入 Flash
extern volatile uint8_t g_load_flash_request;  // 1: 请求将 Flash 数据存入 RAM
extern volatile uint8_t g_replay_start_request;
extern volatile uint8_t g_replay_stop_request;

//【gnss】导航记录控制标志位
extern volatile uint8_t g_gnss_start_recording; // 1: 开始录制, 0: 清除开始录制标记
extern volatile uint8_t g_gnss_recording;       // 1: 正在记录 RAM, 0: 停止记录
extern volatile uint8_t g_gnss_save_flash_request; // 1: 请求将 RAM 数据存入 Flash
extern volatile uint8_t g_gnss_load_flash_request; // 1: 请求将 Flash 数据存入 RAM

// ... 声明其他你项目中需要全局访问的变量 ...

#endif // _COMMON_H_