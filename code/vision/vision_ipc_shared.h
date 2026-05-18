#ifndef VISION_IPC_SHARED_H
#define VISION_IPC_SHARED_H

#include <stddef.h>
#include "vision/vision_ipc.h"
#include "tools/telemetry_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHARED_PAYLOAD_BASE_ADDR           (0x28001000U)
#define SHARED_PAYLOAD_SIZE_BYTES          (0x00000800U)
#define SHARED_PAYLOAD_MPU_REGION          (6U)
#define SHARED_CACHE_LINE_SIZE             (32U)

#define VISION_EVTGEN_SYS_INT_SRC          (evtgen_0_interrupt_IRQn)
#define VISION_EVTGEN_CPU_INT_IDX          (CPUIntIdx4_IRQn)
#define VISION_EVTGEN_PRIORITY             (7U)
#define VISION_EVTGEN_RESULT_BIT           (0U)
#define VISION_EVTGEN_COMMAND_BIT          (1U)
#define VISION_EVTGEN_RESULT_MASK          (1UL << VISION_EVTGEN_RESULT_BIT)
#define VISION_EVTGEN_COMMAND_MASK         (1UL << VISION_EVTGEN_COMMAND_BIT)
#define VISION_EVTGEN_USED_MASK            (VISION_EVTGEN_RESULT_MASK | VISION_EVTGEN_COMMAND_MASK)

#define VISION_CHANNEL_FLAG_VALID          (0x00000001UL)

#define VISION_IPC_COMMAND_OFFSET          (0x00000200U)
#define VISION_IPC_RESULT_OFFSET           (0x00000400U)
#define TELEMETRY_IPC_OFFSET               (0x00000600U)

#define VISION_IPC_COMMAND_SLOT_SIZE       (VISION_IPC_RESULT_OFFSET - VISION_IPC_COMMAND_OFFSET)
#define VISION_IPC_RESULT_SLOT_SIZE        (TELEMETRY_IPC_OFFSET - VISION_IPC_RESULT_OFFSET)
#define TELEMETRY_IPC_SLOT_SIZE            (SHARED_PAYLOAD_SIZE_BYTES - TELEMETRY_IPC_OFFSET)

typedef struct
{
    volatile vision_ipc_command_t payload;
    volatile uint32 frame_seq;
    volatile uint32 publish_us;
    volatile uint32 flags;
    volatile uint32 timestamp;
    volatile uint8 reserved[VISION_IPC_COMMAND_SLOT_SIZE
                            - sizeof(vision_ipc_command_t)
                            - (4U * sizeof(uint32))];
} vision_ipc_command_channel_t;

typedef struct
{
    volatile vision_ipc_packet_t payload;
    volatile uint32 frame_seq;
    volatile uint32 publish_us;
    volatile uint32 flags;
    volatile uint32 timestamp;
    volatile uint8 reserved[VISION_IPC_RESULT_SLOT_SIZE
                            - sizeof(vision_ipc_packet_t)
                            - (4U * sizeof(uint32))];
} vision_ipc_result_channel_t;

typedef struct
{
    volatile telemetry_ipc_packet_t payload;
    volatile uint32 frame_seq;
    volatile uint32 publish_us;
    volatile uint32 flags;
    volatile uint32 timestamp;
    volatile uint8 reserved[TELEMETRY_IPC_SLOT_SIZE
                            - sizeof(telemetry_ipc_packet_t)
                            - (4U * sizeof(uint32))];
} vision_ipc_telemetry_channel_t;

typedef char vision_ipc_command_slot_check[
    (sizeof(vision_ipc_command_channel_t) == VISION_IPC_COMMAND_SLOT_SIZE) ? 1 : -1];
typedef char vision_ipc_result_slot_check[
    (sizeof(vision_ipc_result_channel_t) == VISION_IPC_RESULT_SLOT_SIZE) ? 1 : -1];
typedef char vision_ipc_telemetry_slot_check[
    (sizeof(vision_ipc_telemetry_channel_t) == TELEMETRY_IPC_SLOT_SIZE) ? 1 : -1];

typedef struct
{
    volatile uint8 reserved0[VISION_IPC_COMMAND_OFFSET];
    volatile vision_ipc_command_channel_t command_channel;
    volatile vision_ipc_result_channel_t result_channel;
    volatile vision_ipc_telemetry_channel_t telemetry_channel;
} vision_ipc_shared_layout_t;

typedef char vision_ipc_command_addr_check[
    (offsetof(vision_ipc_shared_layout_t, command_channel.payload) == VISION_IPC_COMMAND_OFFSET) ? 1 : -1];
typedef char vision_ipc_result_addr_check[
    (offsetof(vision_ipc_shared_layout_t, result_channel.payload) == VISION_IPC_RESULT_OFFSET) ? 1 : -1];
typedef char vision_ipc_telemetry_addr_check[
    (offsetof(vision_ipc_shared_layout_t, telemetry_channel.payload) == TELEMETRY_IPC_OFFSET) ? 1 : -1];
typedef char vision_ipc_legacy_command_addr_check[
    (VISION_IPC_COMMAND_ADDR == (SHARED_PAYLOAD_BASE_ADDR + VISION_IPC_COMMAND_OFFSET)) ? 1 : -1];
typedef char vision_ipc_legacy_result_addr_check[
    (VISION_IPC_RESULT_ADDR == (SHARED_PAYLOAD_BASE_ADDR + VISION_IPC_RESULT_OFFSET)) ? 1 : -1];
typedef char vision_ipc_legacy_telemetry_addr_check[
    (TELEMETRY_IPC_ADDR == (SHARED_PAYLOAD_BASE_ADDR + TELEMETRY_IPC_OFFSET)) ? 1 : -1];
typedef char vision_ipc_shared_layout_size_check[
    (sizeof(vision_ipc_shared_layout_t) == SHARED_PAYLOAD_SIZE_BYTES) ? 1 : -1];

extern volatile vision_ipc_shared_layout_t g_vision_ipc_shared;

#ifdef __cplusplus
}
#endif

#endif
