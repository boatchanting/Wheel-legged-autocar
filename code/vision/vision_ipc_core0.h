/*
 * 文件: vision_ipc_core0.h
 * 作用: 0 核 IPC 接口声明。
 * 说明: 上层控制模块通过此接口选择视觉任务并读取最新结果。
 */
#ifndef VISION_IPC_CORE0_H
#define VISION_IPC_CORE0_H

#include "vision/vision_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

extern volatile vision_ipc_packet_t g_vision_ipc_latest;

void VisionIpc_Core0_Init(void);
void VisionIpc_Core0_SetTask(uint8 active_target, uint16 enable_mask);
void VisionIpc_Core0_SetPvcEnable(uint8 enable);
void VisionIpc_Core0_SetBridgeLineEnable(uint8 enable);
void VisionIpc_Core0_Update_2ms(void);
uint8 VisionIpc_Core0_PollResult(void);
const volatile vision_ipc_packet_t *VisionIpc_Core0_GetLatest(void);

#ifdef __cplusplus
}
#endif

#endif
