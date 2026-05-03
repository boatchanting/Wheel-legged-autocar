#ifndef TELEMETRY_IPC_CORE1_H
#define TELEMETRY_IPC_CORE1_H

#include "../../code/tools/telemetry_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8 TelemetryIpc_Core1_ReadLatest(telemetry_ipc_packet_t *out_packet);

#ifdef __cplusplus
}
#endif

#endif
