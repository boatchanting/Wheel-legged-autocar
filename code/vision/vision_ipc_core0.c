#include "vision/vision_ipc_core0.h"

#if defined(__ICCARM__)
#pragma data_alignment = 32
#pragma location = VISION_IPC_COMMAND_ADDR
__no_init volatile vision_ipc_command_t g_vision_ipc_command;

#pragma data_alignment = 32
#pragma location = VISION_IPC_RESULT_ADDR
__no_init volatile vision_ipc_packet_t g_vision_ipc_result;
#else
volatile vision_ipc_command_t g_vision_ipc_command;
volatile vision_ipc_packet_t g_vision_ipc_result;
#endif

volatile vision_ipc_packet_t g_vision_ipc_latest = {0};

static vision_ipc_command_t g_core0_command_shadow;
static uint32 g_core0_last_result_seq = 0U;
static volatile uint8 g_core0_command_dirty = 0U;

static void vision_ipc_core0_flush_command(void)
{
    g_core0_command_shadow.magic = VISION_IPC_CMD_MAGIC;
    g_core0_command_shadow.version = VISION_IPC_VERSION;
    g_core0_command_shadow.size = (uint16)sizeof(vision_ipc_command_t);
    g_core0_command_shadow.crc = 0U;
    g_core0_command_shadow.crc = vision_ipc_command_crc(&g_core0_command_shadow);

    g_vision_ipc_command = g_core0_command_shadow;
    SCB_CleanInvalidateDCache_by_Addr((void *)&g_vision_ipc_command, sizeof(g_vision_ipc_command));
}

void VisionIpc_Core0_Init(void)
{
    memset(&g_core0_command_shadow, 0, sizeof(g_core0_command_shadow));
    memset((void *)&g_vision_ipc_latest, 0, sizeof(g_vision_ipc_latest));
    g_core0_last_result_seq = 0U;

    g_core0_command_shadow.active_target = VISION_TARGET_NONE;
    g_core0_command_shadow.enable_mask = 0U;
    g_core0_command_shadow.pvc_min_score_u16 = 580U;
    g_core0_command_shadow.seq = 1U;
    g_core0_command_dirty = 0U;
    vision_ipc_core0_flush_command();
}

void VisionIpc_Core0_SetTask(uint8 active_target, uint16 enable_mask)
{
    g_core0_command_shadow.active_target = active_target;
    g_core0_command_shadow.enable_mask = enable_mask;
    g_core0_command_shadow.seq++;
    g_core0_command_dirty = 1U;
}

void VisionIpc_Core0_SetPvcEnable(uint8 enable)
{
    if (enable)
    {
        VisionIpc_Core0_SetTask(VISION_TARGET_PVC_ENTRY, VISION_MASK_PVC_ENTRY);
    }
    else
    {
        VisionIpc_Core0_SetTask(VISION_TARGET_NONE, 0U);
    }
}

void VisionIpc_Core0_Update_2ms(void)
{
    if (g_core0_command_dirty)
    {
        vision_ipc_core0_flush_command();
        g_core0_command_dirty = 0U;
    }

    (void)VisionIpc_Core0_PollResult();
}

uint8 VisionIpc_Core0_PollResult(void)
{
    vision_ipc_packet_t packet;

    SCB_CleanInvalidateDCache_by_Addr((void *)&g_vision_ipc_result, sizeof(g_vision_ipc_result));
    packet = g_vision_ipc_result;

    if (vision_ipc_packet_is_valid(&packet) == 0U)
    {
        return 0U;
    }
    if (packet.seq == g_core0_last_result_seq)
    {
        return 0U;
    }

    g_core0_last_result_seq = packet.seq;
    g_vision_ipc_latest = packet;
    return 1U;
}

const volatile vision_ipc_packet_t *VisionIpc_Core0_GetLatest(void)
{
    return &g_vision_ipc_latest;
}
