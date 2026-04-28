#ifndef VISION_IPC_CORE1_H
#define VISION_IPC_CORE1_H

#include "pvc_vision.h"
#include "vision/vision_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

void VisionIpc_Core1_Init(void);
void VisionIpc_Core1_Update_2ms(void);
void VisionIpc_Core1_PollCommand(void);
uint8 VisionIpc_Core1_ShouldRunPvc(void);
uint8 VisionIpc_Core1_TakePvcResetRequest(void);
void VisionIpc_Core1_PublishPvc(const volatile pvc_vision_output_t *pvc_output);
void VisionIpc_Core1_PublishIdle(void);

#ifdef __cplusplus
}
#endif

#endif
