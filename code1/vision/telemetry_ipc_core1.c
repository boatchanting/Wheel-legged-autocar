#include "telemetry_ipc_core1.h"
#include "vision/vision_ipc_shared.h"

uint8 TelemetryIpc_Core1_ReadLatest(telemetry_ipc_packet_t *out_packet)
{
    telemetry_ipc_packet_t packet;

    if (out_packet == NULL)
    {
        return 0U;
    }

    packet = g_vision_ipc_shared.telemetry;
    if (telemetry_ipc_packet_is_valid(&packet) == 0U)
    {
        return 0U;
    }

    *out_packet = packet;
    return 1U;
}
