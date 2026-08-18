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
/* 角度响应整形/符号参数已于 2026-08-18 上移至 1 核 code1/vision/bumpy_vision.h（VISION_BUMPY_*），
   0 核不再做整形/EMA/锁角，仅直通 1 核稳定提案。 */

/* 横向记录（2026-08-17 规划 §4.3；2026-08-18 起正式横向源为 1 核 lat_stable，recorded 保留为遥测） */
#define VISION_BUMPY_LATERAL_RECORD_ALPHA      (0.50f)  // 记录 EMA 系数：0~1，越小越平滑
#define VISION_BUMPY_LATERAL_RECORD_MAX_MM     (200.0f) // 记录值限幅，防视觉异常导致出口跳变
#define VISION_BUMPY_ENTRY_DETECT_FRAMES        (3U)     // 连续 5 个新视觉帧检测到颠簸，确认进入路段
#define VISION_BUMPY_EXIT_MISS_FRAMES          (3U)     // 连续 5 个新视觉帧未检测到颠簸，确认视觉出口

/* 横向记录（2026-08-17 规划 §4.3） */
#define VISION_BUMPY_LATERAL_RECORD_ALPHA      (0.50f)  // 记录 EMA 系数：0~1，越小越平滑
#define VISION_BUMPY_LATERAL_RECORD_MAX_MM     (200.0f) // 记录值限幅，防视觉异常导致出口跳变
#define VISION_BUMPY_ENTRY_DETECT_FRAMES        (3U)     // 连续 5 个新视觉帧检测到颠簸，确认进入路段
#define VISION_BUMPY_EXIT_MISS_FRAMES          (3U)     // 连续 5 个新视觉帧未检测到颠簸，确认视觉出口

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
    uint8 bumpy_detected;                  // 当前帧颠簸路段检测结果
    uint8 entry_confirmed;                 // 连续检测到颠簸后确认已经进入路段
    uint8 exit_confirmed;                  // 连续未检测达到阈值后的视觉出口确认
    uint8 detect_frame_count;              // 连续检测到颠簸的新视觉帧数量
    uint8 miss_frame_count;                // 连续未检测到颠簸的新视觉帧数量
    uint16 stale_ticks;                    // 数据包过期计数器(2ms周期计数)
    uint32 last_seq;                       // 上次接收到的数据包序列号
    uint32 last_frame_id;                  // 已统计的最后一个视觉帧编号
    vision_bumpy_control_state_e state;    // 当前控制状态
    float direction_x;                     // 视觉方向向量 X 分量
    float direction_y;                     // 视觉方向向量 Y 分量
    /* —— 角度路径（2026-08-18 起：1 核完成“按角度大小整形+EMA”，0 核零锁、纯直通）——
       yaw_error_deg_x100 已是 1 核稳定提案（无条纹报 0）；err_degree_cmd 直通送 err_degree —— */
    int16 yaw_error_deg_x100;              // 1 核整形后偏差角度×100（遥测用）
    float err_degree_cmd;                  // 转向误差角度指令(度)：1 核 yaw_error 直通
    /* 横向记录（lateral_mm → recorded，只记录不修正；正式消费源为 1 核 lat_stable） */
    int16 lateral_mm;                      // 最近一帧可信横向偏差（原始观测，遥测用）
    float recorded_lateral_mm;             // 记录的横向偏差（EMA 滤波，失稳时冻结，遥测用）
    uint8 meas_valid;                      // 最近一帧是否可信（VISION_VALID_BUMPY_MEAS）
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
 * @brief   获取转向误差指令（角度响应整形输出，直接送 err_degree 管线）
 * @return  当前转向误差指令(度)
 */
float VisionBumpyControl_GetErrDegreeCmd(void);

/**
 * @brief   查询最近一帧角度测量是否可信（VISION_VALID_BUMPY_MEAS 直通，含横向 HOLD 透传）
 * @return  1: 本帧角度可信；0: 严重丢失（无条纹），锁当前角度
 */
uint8 VisionBumpyControl_IsMeasurementValid(void);

/**
 * @brief   获取冻结后的横向偏差记录值（新视觉接口 2026-08-17 规划 §4.3；遥测用）
 * @return  记录的横向偏差（mm，正值=车身偏右）
 */
float VisionBumpyControl_GetRecordedLateralMm(void);

/**
 * @brief   清空颠簸视觉出口判定历史（进入颠簸任务时调用）
 */
void VisionBumpyControl_ResetExitDetection(void);

/**
 * @brief   在已经确认进入颠簸路段后，重新开始出口未检测帧统计
 */
void VisionBumpyControl_RearmExitDetection(void);

/**
 * @brief   查询是否已经连续 5 帧检测到颠簸路段
 */
uint8 VisionBumpyControl_IsEntryConfirmed(void);

/**
 * @brief   查询是否已经连续 5 帧未检测到颠簸
 */
uint8 VisionBumpyControl_IsExitConfirmed(void);

#ifdef __cplusplus
}
#endif

#endif
