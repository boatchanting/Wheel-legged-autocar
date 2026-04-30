/*
 * 文件: vision_ipc_core1.h
 * 作用: 1 核 IPC 接口声明。
 * 说明: 负责读取 0 核命令并发布 PVC/直线检测结果到共享内存。
 */
#ifndef VISION_IPC_CORE1_H
#define VISION_IPC_CORE1_H

#include "pvc_vision.h"
#include "line_vision.h"
#include "vision/vision_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

void VisionIpc_Core1_Init(void);
void VisionIpc_Core1_Update_2ms(void);
void VisionIpc_Core1_PollCommand(void);
uint8 VisionIpc_Core1_ShouldRunPvc(void);
uint8 VisionIpc_Core1_TakePvcResetRequest(void);
uint8 VisionIpc_Core1_ShouldRunBridgeLine(void);
uint8 VisionIpc_Core1_TakeLineResetRequest(void);
void VisionIpc_Core1_PublishPvc(const volatile pvc_vision_output_t *pvc_output);
void VisionIpc_Core1_PublishCurrent(void);
void VisionIpc_Core1_PublishIdle(void);

#ifdef __cplusplus
}
#endif

#endif
