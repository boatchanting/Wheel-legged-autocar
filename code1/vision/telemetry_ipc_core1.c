#include "telemetry_ipc_core1.h"

#if defined(__ICCARM__)
#pragma data_alignment = 32
#pragma location = TELEMETRY_IPC_ADDR
__no_init volatile telemetry_ipc_packet_t g_telemetry_ipc_packet;
#else
volatile telemetry_ipc_packet_t g_telemetry_ipc_packet;
#endif

uint8 TelemetryIpc_Core1_ReadLatest(telemetry_ipc_packet_t *out_packet)
{
    telemetry_ipc_packet_t packet;

    if (out_packet == NULL)
    {
        return 0U;
    }

    SCB_CleanInvalidateDCache_by_Addr((void *)&g_telemetry_ipc_packet, sizeof(g_telemetry_ipc_packet));
    packet = g_telemetry_ipc_packet;
    if (telemetry_ipc_packet_is_valid(&packet) == 0U)
    {
        return 0U;
    }

    *out_packet = packet;
    return 1U;
}
