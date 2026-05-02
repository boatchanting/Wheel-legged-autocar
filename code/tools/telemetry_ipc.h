#ifndef TELEMETRY_IPC_H
#define TELEMETRY_IPC_H

#include <stddef.h>
#include "zf_common_headfile.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TELEMETRY_IPC_ADDR        (0x28001600U)
#define TELEMETRY_IPC_MAGIC       (0x544DU)
#define TELEMETRY_IPC_VERSION     (1U)
#define TELEMETRY_IPC_CHANNELS    (8U)

typedef struct
{
    uint16 magic;
    uint16 size;
    uint8 version;
    uint8 channel_count;
    uint16 reserved0;
    uint32 seq;
    float data[TELEMETRY_IPC_CHANNELS];
    uint16 crc;
} telemetry_ipc_packet_t;

static inline uint16 telemetry_ipc_checksum16(const void *data, uint16 size_without_crc)
{
    const uint8 *bytes = (const uint8 *)data;
    uint32 sum = 0U;
    for (uint16 i = 0U; i < size_without_crc; i++)
    {
        sum += bytes[i];
    }
    return (uint16)((sum & 0xFFFFU) + (sum >> 16));
}

static inline uint16 telemetry_ipc_packet_crc(const telemetry_ipc_packet_t *packet)
{
    return telemetry_ipc_checksum16(packet, (uint16)offsetof(telemetry_ipc_packet_t, crc));
}

static inline uint8 telemetry_ipc_packet_is_valid(const telemetry_ipc_packet_t *packet)
{
    return (uint8)((packet->magic == TELEMETRY_IPC_MAGIC) &&
                   (packet->version == TELEMETRY_IPC_VERSION) &&
                   (packet->size == sizeof(telemetry_ipc_packet_t)) &&
                   (packet->channel_count <= TELEMETRY_IPC_CHANNELS) &&
                   (packet->crc == telemetry_ipc_packet_crc(packet)));
}

#ifdef __cplusplus
}
#endif

#endif
