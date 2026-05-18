/*
 * =================================================================================
 * 文件: vision_ipc_core0.h
 * 作用: 0 核 (Core 0) 进程间通信 (IPC) 接口声明。
 * 说明: 这个文件就像是 0 核的“通讯器说明书”。
 *       0 核通过它，向 1 核下达命令（“去帮我找 PVC”、“去帮我找桥”），
 *       并从 1 核那里读取最新的视觉处理结果。
 * =================================================================================
 */
#ifndef VISION_IPC_CORE0_H
#define VISION_IPC_CORE0_H

#include "vision/vision_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 这个变量保存了从 1 核拿到的最新数据，就像是“今日战报” */
extern volatile vision_ipc_packet_t g_vision_ipc_latest;

/* --- 对外公开的函数接口 --- */

/**
 * @brief 初始化 0 核视觉双核通信
 * @note  开机时调用，配置共享区 MPU NC、EVTGEN 接收中断与本核 shadow。
 */
void VisionIpc_Core0_Init(void);

/**
 * @brief 给 1 核下达新的视觉任务
 * @param active_target 当前的主要目标（比如：找 PVC）
 * @param enable_mask   同时还要开哪些辅助目标（比如：同时找桥）
 */
void VisionIpc_Core0_SetTask(uint8 active_target, uint16 enable_mask);

/**
 * @brief 快捷指令：让 1 核开启或关闭 PVC 检测
 */
void VisionIpc_Core0_SetPvcEnable(uint8 enable);

/**
 * @brief 快捷指令：让 1 核开启或关闭桥梁/直线检测
 */
void VisionIpc_Core0_SetBridgeLineEnable(uint8 enable);
void VisionIpc_Core0_SetBumpyEnable(uint8 enable);

/**
 * @brief 0 核视觉通信的兼容周期接口
 * @note  保留给现有调度调用，EVTGEN 方案下无需主动轮询共享区。
 */
void VisionIpc_Core0_Update_2ms(void);

/**
 * @brief 查询是否有新的结果 shadow 尚未被消费
 * @return 1: 有新的结果; 0: 没有
 */
uint8 VisionIpc_Core0_PollResult(void);

/**
 * @brief 获取最新的一包视觉数据
 * @return 指向最新数据的指针
 */
const volatile vision_ipc_packet_t *VisionIpc_Core0_GetLatest(void);

#ifdef __cplusplus
}
#endif

#endif
