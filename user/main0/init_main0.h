#ifndef _INIT_MAIN0_H_
#define _INIT_MAIN0_H_

#include "zf_common_headfile.h"//【提醒！！！】导入了新模块添加到这个文件里
#include "config/config.h"//【提醒】配置请在这里修改
#include "tools/runtime_profiler.h"
#include "vision/vision_ipc_core0.h"
#include "tools/telemetry_ipc_core0.h"
#include "vision/vision_pvc_control.h"
#include "vision/vision_bumpy_control.h"
#include "vision/vision_bridge_control.h"
#include "vision/vision_three_stage_control.h"

// **************************** uart配置区域 **************************** 
#define UART_INDEX              (DEBUG_UART_INDEX    )                           // 默认 UART_0
#define UART_BAUDRATE           (DEBUG_UART_BAUDRATE)                           // 默认 115200
#define UART_TX_PIN             (DEBUG_UART_TX_PIN  )                           // 默认 UART0_TX_P00_1
#define UART_RX_PIN             (DEBUG_UART_RX_PIN  )                           // 默认 UART0_RX_P00_0

extern uint8 uart_get_data[64];                                                        // 串口接收数据缓冲区
extern uint8 fifo_get_data[64];                                                        // fifo 输出读出缓冲区
extern uint8 get_data;                                                                 // 接收数据变量
extern uint32 fifo_data_count;                                                         // fifo 数据个数
extern fifo_struct uart_data_fifo;

// **************************** 无刷电机配置区域 **************************** 
#define MAX_DUTY            (30 )                                               // 最大 MAX_DUTY% 占空比
extern int8 duty;
extern bool dir;

// **************************** ips200屏幕配置区域 ****************************                                    
#define IPS200_TYPE     (IPS200_TYPE_SPI)   // 八位并口两寸屏 这里宏定义填写 IPS200_TYPE_PARALLEL8  定义屏幕接口类型    
// SPI 串口两寸屏 这里宏定义填写 IPS200_TYPE_SPI

// **************************** LED配置区域 ****************************
#define LED1                    (P19_0)                                         // SPI 串口 SPI 两寸屏 这里宏定义填写 IPS200_TYPE_SPI

//  **************************** 中断配置区域 ****************************
#define PIT_NUM         (PIT_CH0) // 使用定时器通道0       用于平衡控制，1ms
#define PIT_NUM_1         (PIT_CH1) // 使用定时器通道1     用于遥控器，10ms
#define PIT_NUM_10         (PIT_CH10) // 使用定时器通道10  用于遥控器，10ms

extern volatile uint8 pit_state;  //通道0中断标志位
extern uint8 pit_state_1;         //通道1中断标志位
extern volatile runtime_profiler_t g_ekf_profiler;

// *************************** EKF中断声明 ***************************
extern void IMU_Calibrate_All_Gyro(void); // 校准陀螺仪声明
extern void EKF_Init(void);
extern EulerAngles euler_angle; // 引用 ekf.c 中计算出的角度

// =================================================================================
// 导航记录控制标志位与视觉标志位 外部声明
extern volatile uint8_t vision_detected_bumpy_point; // 模拟视觉检测到“颠簸入口”

// ---------------------------
// 0核主初始化函数声明
// ---------------------------
void Main0_Init(void);
extern uint8 imu_refresh_test_printed; // IMU刷新率测试结果只打印一次
#endif // _INIT_MAIN0_H_