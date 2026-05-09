/**
 * @file    vision_bumpy_control.h
 * @brief   视觉凹凸路面控制模块头文件
 * @details 定义了凹凸路面控制相关的数据结构、常量和函数接口
 *          该模块负责处理视觉系统检测到的凹凸路面信息，并生成相应的转向控制指令
 */

#ifndef VISION_BUMPY_CONTROL_H
#define VISION_BUMPY_CONTROL_H

/* 头文件包含区 */
#include "zf_common_headfile.h"      // 通用头文件
#include "tools/runtime_profiler.h"  // 运行时性能分析器
#include "vision/vision_ipc.h"       // 视觉IPC通信接口

#ifdef __cplusplus
extern "C" {
#endif

/* 功能开关宏定义区 */
#define VISION_BUMPY_CONTROL_ENABLE            (1)    // 凹凸路面控制功能使能(1:使能, 0:禁用)
#define VISION_BUMPY_CONTROL_DEFAULT_ACTIVE    (1)    // 默认激活状态(1:激活, 0:不激活)
#define VISION_BUMPY_CONTROL_PROFILE_ENABLE    (0)    // 性能分析功能使能(1:使能, 0:禁用)
#define VISION_BUMPY_CONTROL_PROFILE_TIMER     (TC_TIME2_CH0)  // 性能分析使用的定时器通道

/* 控制参数宏定义区 */
#define VISION_BUMPY_STALE_TIMEOUT_TICKS       (120U)  // 数据包过期超时时间(2ms周期计数, 120=240ms)
#define VISION_BUMPY_K_STEER_DEG_PER_PX        (-0.30f) // 像素到角度的转换系数(度/像素)
#define VISION_BUMPY_MAX_ERR_DEG               (18.0f) // 最大转向误差角度限制(度)
#define VISION_BUMPY_DEADBAND_DEG              (180.0f) // 转向误差死区(度), 小于此值的误差将被忽略

/* 枚举类型定义区 */
/**
 * @brief   凹凸路面控制状态枚举
 */
typedef enum
{
    VISION_BUMPY_CTRL_IDLE = 0,    // 空闲状态(未使能或未检测到凹凸路面)
    VISION_BUMPY_CTRL_SEARCH,      // 搜索状态(正在寻找凹凸路面)
    VISION_BUMPY_CTRL_TRACK,       // 跟踪状态(稳定跟踪凹凸路面)
    VISION_BUMPY_CTRL_STALE,       // 过期状态(数据包过期或无效)
} vision_bumpy_control_state_e;

/* 结构体类型定义区 */
/**
 * @brief   凹凸路面控制状态结构体
 * @details 包含了凹凸路面控制的所有状态信息和控制参数
 */
typedef struct
{
    uint8 enabled;                         // 控制使能标志(1:使能, 0:禁用)
    uint8 has_new_packet;                   // 新数据包标志(1:有新数据, 0:无新数据)
    uint8 stable_detected;                  // 稳定检测标志(1:稳定检测到凹凸路面, 0:未稳定检测)
    uint8 raw_detected;                    // 原始检测标志(1:检测到凹凸路面, 0:未检测)
    uint8 phase;                           // 凹凸路面阶段(对应bumpy_vision.h中的BumpyPhase)
    uint8 mode;                            // 控制模式(对应bumpy_vision.h中的BumpyControllerMode)
    uint16 confidence_u16;                 // 检测置信度(0-65535)
    uint16 stale_ticks;                    // 数据包过期计数器(2ms周期计数)
    uint32 last_seq;                       // 上次接收到的数据包序列号
    vision_bumpy_control_state_e state;    // 当前控制状态
    int16 steer_error_px_x100;             // 转向误差像素值(放大100倍)
    float err_degree_cmd;                  // 转向误差角度指令(度)
} vision_bumpy_control_status_t;

/* 全局变量声明区 */
extern volatile vision_bumpy_control_status_t g_vision_bumpy_control_status;  // 凹凸路面控制状态全局变量
extern volatile runtime_profiler_t g_vision_bumpy_control_profiler;          // 运行时性能分析器
extern volatile uint8 g_bumpy_control_enable;                                 // 凹凸路面控制使能标志

/* 函数声明区 */
/**
 * @brief   初始化凹凸路面控制模块
 * @details 1. 清零影子变量
 *          2. 设置默认使能状态
 *          3. 重置性能分析器(如果启用)
 */
void VisionBumpyControl_Init(void);

/**
 * @brief   设置凹凸路面控制使能状态
 * @param   enable 使能标志(1:使能, 0:禁用)
 * @details 如果禁用控制，将应用空闲输出状态
 */
void VisionBumpyControl_SetEnable(uint8 enable);

/**
 * @brief   获取凹凸路面控制使能状态
 * @return  当前使能状态(1:使能, 0:禁用)
 */
uint8 VisionBumpyControl_IsEnabled(void);

/**
 * @brief   2ms周期调用的控制更新函数
 * @details 1. 检查使能状态
 *          2. 获取最新视觉数据包
 *          3. 处理数据包有效性
 *          4. 根据检测状态计算控制指令
 */
void VisionBumpyControl_Update_2ms(void);

/**
 * @brief   获取转向误差指令
 * @return  当前转向误差指令(度)
 */
float VisionBumpyControl_GetErrDegreeCmd(void);

#ifdef __cplusplus
}
#endif

#endif
