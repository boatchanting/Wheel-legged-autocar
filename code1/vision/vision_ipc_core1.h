/*
 * =================================================================================
 * 文件: vision_ipc_core1.h
 * 作用: 1 核 (Core 1) 进程间通信 (IPC) 接口声明。
 * 说明: 这个头文件暴露了 1 核用来和 0 核通信的公共函数。
 *       主要功能包括：读取 0 核下发的命令、将 PVC（入口）或直线/桥梁检测的结果
 *       发布到共享内存中供 0 核读取。
 * =================================================================================
 */
#ifndef VISION_IPC_CORE1_H
#define VISION_IPC_CORE1_H

#include "pvc_vision.h"
#include "line_vision.h"
#include "bumpy_vision.h"
#include "vision/vision_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- 核心生命周期与更新接口 --- */

/**
 * @brief 初始化 1 核的 IPC 状态
 * @note  在系统启动时调用，重置所有命令缓存和状态。
 */
void VisionIpc_Core1_Init(void);

/**
 * @brief 1 核 IPC 定时更新函数
 * @note  建议放在 1 核的 2ms 定时中断中调用，负责拉取命令并发送最新视觉结果。
 */
void VisionIpc_Core1_Update_2ms(void);

/**
 * @brief 主动拉取 0 核命令
 * @note  从共享内存读取并解析 0 核发来的控制指令。
 */
void VisionIpc_Core1_PollCommand(void);

/* --- 视觉模块状态查询接口 --- */

/**
 * @brief 检查当前是否需要运行 PVC 检测
 * @return 1: 开启, 0: 关闭
 */
uint8 VisionIpc_Core1_ShouldRunPvc(void);

/**
 * @brief 检查是否需要重置 PVC 检测状态，读取后自动清除请求
 * @return 1: 需要重置, 0: 不需要
 */
uint8 VisionIpc_Core1_TakePvcResetRequest(void);

/**
 * @brief 检查当前是否需要运行 直线/桥梁 检测
 * @return 1: 开启, 0: 关闭
 */
uint8 VisionIpc_Core1_ShouldRunBridgeLine(void);

/**
 * @brief 检查是否需要重置 直线/桥梁 检测状态，读取后自动清除请求
 * @return 1: 需要重置, 0: 不需要
 */
uint8 VisionIpc_Core1_TakeLineResetRequest(void);
uint8 VisionIpc_Core1_ShouldRunBumpy(void);
uint8 VisionIpc_Core1_TakeBumpyResetRequest(void);

/* --- 数据发布接口 --- */

/**
 * @brief 专门发布一次 PVC 检测结果到共享内存
 * @param pvc_output 指向当前最新的 PVC 结果
 */
uint8 VisionIpc_Core1_GetActiveTarget(void);
uint16 VisionIpc_Core1_GetEnableMask(void);
void VisionIpc_Core1_PublishPvc(const volatile pvc_vision_output_t *pvc_output);

/**
 * @brief 整合并发布当前所有激活模块的检测结果
 */
void VisionIpc_Core1_PublishCurrent(void);

/**
 * @brief 发布空闲数据包（告诉 0 核当前没在做任何检测）
 */
void VisionIpc_Core1_PublishIdle(void);

#ifdef __cplusplus
}
#endif

#endif
