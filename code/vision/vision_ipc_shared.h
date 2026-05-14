#ifndef VISION_IPC_SHARED_H
#define VISION_IPC_SHARED_H

#include "vision/vision_ipc.h"
#include "tools/telemetry_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHARED_PAYLOAD_BASE_ADDR       (0x28001000U)
#define SHARED_PAYLOAD_SIZE_BYTES      (0x00000800U)
#define SHARED_PAYLOAD_MPU_REGION      (6U)
#define SHARED_CACHE_LINE_SIZE         (32U)

#define IPC_VISION_CHAN                (6U)
#define IPC_VISION_INTR                (6U)
#define IPC_VISION_INTR_MASK           (1UL << IPC_VISION_CHAN)
#define IPC_VISION_INTR_IDX            CPUIntIdx4_IRQn
#define IPC_VISION_INTR_PRIORITY       (7U)

#define VISION_IPC_COMMAND_OFFSET      (0x00000200U)
#define VISION_IPC_RESULT_OFFSET       (0x00000400U)
#define TELEMETRY_IPC_OFFSET           (0x00000600U)

#define VISION_IPC_COMMAND_SLOT_SIZE   (VISION_IPC_RESULT_OFFSET - VISION_IPC_COMMAND_OFFSET)
#define VISION_IPC_RESULT_SLOT_SIZE    (TELEMETRY_IPC_OFFSET - VISION_IPC_RESULT_OFFSET)
#define TELEMETRY_IPC_SLOT_SIZE        (SHARED_PAYLOAD_SIZE_BYTES - TELEMETRY_IPC_OFFSET)

typedef char vision_ipc_command_slot_check[
    (sizeof(vision_ipc_command_t) <= VISION_IPC_COMMAND_SLOT_SIZE) ? 1 : -1];
typedef char vision_ipc_result_slot_check[
    (sizeof(vision_ipc_packet_t) <= VISION_IPC_RESULT_SLOT_SIZE) ? 1 : -1];
typedef char vision_ipc_telemetry_slot_check[
    (sizeof(telemetry_ipc_packet_t) <= TELEMETRY_IPC_SLOT_SIZE) ? 1 : -1];
typedef char vision_ipc_command_addr_check[
    (VISION_IPC_COMMAND_ADDR == (SHARED_PAYLOAD_BASE_ADDR + VISION_IPC_COMMAND_OFFSET)) ? 1 : -1];
typedef char vision_ipc_result_addr_check[
    (VISION_IPC_RESULT_ADDR == (SHARED_PAYLOAD_BASE_ADDR + VISION_IPC_RESULT_OFFSET)) ? 1 : -1];
typedef char vision_ipc_telemetry_addr_check[
    (TELEMETRY_IPC_ADDR == (SHARED_PAYLOAD_BASE_ADDR + TELEMETRY_IPC_OFFSET)) ? 1 : -1];

typedef struct
{
    volatile uint8 reserved0[VISION_IPC_COMMAND_OFFSET];
    volatile vision_ipc_command_t command;
    volatile uint8 reserved1[VISION_IPC_RESULT_OFFSET - VISION_IPC_COMMAND_OFFSET - sizeof(vision_ipc_command_t)];
    volatile vision_ipc_packet_t result;
    volatile uint8 reserved2[TELEMETRY_IPC_OFFSET - VISION_IPC_RESULT_OFFSET - sizeof(vision_ipc_packet_t)];
    volatile telemetry_ipc_packet_t telemetry;
    volatile uint8 reserved3[SHARED_PAYLOAD_SIZE_BYTES - TELEMETRY_IPC_OFFSET - sizeof(telemetry_ipc_packet_t)];
} vision_ipc_shared_layout_t;

typedef char vision_ipc_shared_layout_size_check[
    (sizeof(vision_ipc_shared_layout_t) == SHARED_PAYLOAD_SIZE_BYTES) ? 1 : -1];

extern volatile vision_ipc_shared_layout_t g_vision_ipc_shared;

#ifdef __cplusplus
}
#endif

#endif
