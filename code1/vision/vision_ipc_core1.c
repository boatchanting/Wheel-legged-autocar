#include "vision_ipc_core1.h"

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

static vision_ipc_command_t g_core1_command_shadow;
static uint32 g_core1_result_seq = 0U;
static uint8 g_core1_pvc_enabled = 0U;

static uint16 vision_confidence_to_u16(float confidence)
{
    if (confidence <= 0.0f)
    {
        return 0U;
    }
    if (confidence >= 1.0f)
    {
        return 1000U;
    }
    return (uint16)(confidence * 1000.0f);
}

static void vision_ipc_core1_write_packet(vision_ipc_packet_t *packet)
{
    packet->magic = VISION_IPC_RESULT_MAGIC;
    packet->version = VISION_IPC_VERSION;
    packet->size = (uint16)sizeof(vision_ipc_packet_t);
    packet->seq = ++g_core1_result_seq;
    packet->command_seq_echo = g_core1_command_shadow.seq;
    packet->crc = 0U;
    packet->crc = vision_ipc_packet_crc(packet);

    g_vision_ipc_result = *packet;
    SCB_CleanInvalidateDCache_by_Addr((void *)&g_vision_ipc_result, sizeof(g_vision_ipc_result));
}

void VisionIpc_Core1_Init(void)
{
    memset(&g_core1_command_shadow, 0, sizeof(g_core1_command_shadow));
    g_core1_command_shadow.active_target = VISION_TARGET_NONE;
    g_core1_command_shadow.enable_mask = 0U;
    g_core1_command_shadow.pvc_min_score_u16 = 580U;
    g_core1_result_seq = 0U;
    g_core1_pvc_enabled = 0U;
    VisionIpc_Core1_PublishIdle();
}

void VisionIpc_Core1_PollCommand(void)
{
    vision_ipc_command_t cmd;
    uint8 next_pvc_enabled;

    SCB_CleanInvalidateDCache_by_Addr((void *)&g_vision_ipc_command, sizeof(g_vision_ipc_command));
    cmd = g_vision_ipc_command;

    if (vision_ipc_command_is_valid(&cmd))
    {
        g_core1_command_shadow = cmd;
        next_pvc_enabled = VisionIpc_Core1_ShouldRunPvc();
        if (next_pvc_enabled != g_core1_pvc_enabled)
        {
            pvc_vision_reset_filter();
            g_core1_pvc_enabled = next_pvc_enabled;
        }
    }
}

uint8 VisionIpc_Core1_ShouldRunPvc(void)
{
    const uint8 active_is_pvc = (uint8)(g_core1_command_shadow.active_target == VISION_TARGET_PVC_ENTRY);
    const uint8 mask_has_pvc = (uint8)((g_core1_command_shadow.enable_mask & VISION_MASK_PVC_ENTRY) != 0U);
    return (uint8)(active_is_pvc || mask_has_pvc);
}

void VisionIpc_Core1_PublishPvc(const volatile pvc_vision_output_t *pvc_output)
{
    vision_ipc_packet_t packet;
    pvc_vision_output_t pvc;
    const pvc_vision_frame_result_t *ctrl;

    if (pvc_output == NULL)
    {
        VisionIpc_Core1_PublishIdle();
        return;
    }

    pvc = *pvc_output;
    ctrl = pvc.stable_detected ? &pvc.stable : &pvc.raw;

    memset(&packet, 0, sizeof(packet));
    packet.active_target = VISION_TARGET_PVC_ENTRY;
    packet.stable_target = pvc.stable_detected ? VISION_TARGET_PVC_ENTRY : VISION_TARGET_NONE;
    packet.stable_detected = pvc.stable_detected;
    packet.valid_mask = (uint16)(VISION_VALID_COMMON | VISION_VALID_PVC | VISION_VALID_PROFILE);
    packet.frame_id = pvc.frame_id;
    packet.frame_dt_us = (uint16)g_pvc_vision_frame_profiler.last_us;
    packet.cost_us = (uint16)g_pvc_vision_cost_profiler.last_us;

    packet.detected = ctrl->detected;
    packet.raw_detected = pvc.raw_detected;
    packet.confidence_u16 = vision_confidence_to_u16(ctrl->confidence);
    packet.forward_mm = ctrl->forward_mm;
    packet.lateral_mm = ctrl->lateral_mm;
    packet.yaw_error_deg_x100 = ctrl->yaw_error_deg_x100;

    packet.pvc_detected = pvc.raw.detected;
    packet.pvc_stable_detected = pvc.stable_detected;
    packet.pvc_confidence_u16 = vision_confidence_to_u16(ctrl->confidence);
    packet.pvc_forward_mm = ctrl->forward_mm;
    packet.pvc_lateral_mm = ctrl->lateral_mm;
    packet.pvc_yaw_error_deg_x100 = ctrl->yaw_error_deg_x100;
    packet.pvc_entry_bottom_y = ctrl->entry_bottom_y;
    packet.pvc_entry_top_y = ctrl->entry_top_y;
    packet.pvc_bbox_xmin = ctrl->bbox_xmin;
    packet.pvc_bbox_ymin = ctrl->bbox_ymin;
    packet.pvc_bbox_xmax = ctrl->bbox_xmax;
    packet.pvc_bbox_ymax = ctrl->bbox_ymax;
    packet.pvc_area = ctrl->area;
    packet.pvc_component_count = ctrl->component_count;
    packet.pvc_candidate_count = ctrl->candidate_count;

    vision_ipc_core1_write_packet(&packet);
}

void VisionIpc_Core1_PublishIdle(void)
{
    vision_ipc_packet_t packet;

    memset(&packet, 0, sizeof(packet));
    packet.active_target = g_core1_command_shadow.active_target;
    packet.stable_target = VISION_TARGET_NONE;
    packet.valid_mask = VISION_VALID_COMMON;
    vision_ipc_core1_write_packet(&packet);
}
