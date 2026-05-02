#include "tools/telemetry_ipc_core0.h"
#include "vision/vision_ipc_core0.h"
#include "small_driver_uart_control.h"
#include "calculate/pid-new.h"
#include <string.h>

#if defined(__ICCARM__)
#pragma data_alignment = 32
#pragma location = TELEMETRY_IPC_ADDR
__no_init volatile telemetry_ipc_packet_t g_telemetry_ipc_packet;
#else
volatile telemetry_ipc_packet_t g_telemetry_ipc_packet;
#endif

static telemetry_ipc_packet_t g_telemetry_shadow;
static uint32 g_telemetry_seq = 0U;

void TelemetryIpc_Core0_Init(void)
{
    memset(&g_telemetry_shadow, 0, sizeof(g_telemetry_shadow));
    g_telemetry_shadow.magic = TELEMETRY_IPC_MAGIC;
    g_telemetry_shadow.size = (uint16)sizeof(telemetry_ipc_packet_t);
    g_telemetry_shadow.version = TELEMETRY_IPC_VERSION;
    g_telemetry_shadow.channel_count = TELEMETRY_IPC_CHANNELS;
    g_telemetry_shadow.seq = 0U;
    g_telemetry_shadow.crc = 0U;
    g_telemetry_shadow.crc = telemetry_ipc_packet_crc(&g_telemetry_shadow);
    g_telemetry_ipc_packet = g_telemetry_shadow;
    SCB_CleanInvalidateDCache_by_Addr((void *)&g_telemetry_ipc_packet, sizeof(g_telemetry_ipc_packet));
}

void TelemetryIpc_Core0_PublishPvcDefault(void)
{
    const volatile vision_ipc_packet_t *packet = VisionIpc_Core0_GetLatest();

    g_telemetry_shadow.magic = TELEMETRY_IPC_MAGIC;
    g_telemetry_shadow.size = (uint16)sizeof(telemetry_ipc_packet_t);
    g_telemetry_shadow.version = TELEMETRY_IPC_VERSION;
    g_telemetry_shadow.channel_count = TELEMETRY_IPC_CHANNELS;
    g_telemetry_shadow.seq = ++g_telemetry_seq;

    g_telemetry_shadow.data[0] = (float)motor_value.receive_left_speed_data;
    g_telemetry_shadow.data[1] = (float)motor_value.receive_right_speed_data;
    g_telemetry_shadow.data[2] = (float)packet->pvc_yaw_error_deg_x100 / 100.0f;
    g_telemetry_shadow.data[3] = (float)packet->pvc_lateral_mm;
    g_telemetry_shadow.data[4] = (float)target_speed_set;
    g_telemetry_shadow.data[5] = (float)packet->pvc_confidence_u16;
    g_telemetry_shadow.data[6] = (float)packet->pvc_entry_bottom_y;
    g_telemetry_shadow.data[7] = (float)packet->pvc_entry_top_y;

    g_telemetry_shadow.crc = 0U;
    g_telemetry_shadow.crc = telemetry_ipc_packet_crc(&g_telemetry_shadow);
    g_telemetry_ipc_packet = g_telemetry_shadow;
    SCB_CleanInvalidateDCache_by_Addr((void *)&g_telemetry_ipc_packet, sizeof(g_telemetry_ipc_packet));
}
