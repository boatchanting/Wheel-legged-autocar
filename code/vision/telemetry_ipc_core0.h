#ifndef TELEMETRY_IPC_CORE0_H
#define TELEMETRY_IPC_CORE0_H

#include "vision/telemetry_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

void TelemetryIpc_Core0_Init(void);
void TelemetryIpc_Core0_PublishPvcDefault(void);

#ifdef __cplusplus
}
#endif

#endif
