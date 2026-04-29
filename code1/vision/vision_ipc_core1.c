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
static volatile uint8 g_core1_pvc_enabled = 0U;
static volatile uint8 g_core1_line_enabled = 0U;
static volatile uint8 g_core1_pvc_reset_request = 0U;
static volatile uint8 g_core1_line_reset_request = 0U;
static uint32 g_core1_last_published_pvc_frame_id = 0U;
static uint32 g_core1_last_published_line_frame_id = 0U;
static uint32 g_core1_last_published_command_seq = 0U;
static uint16 g_core1_last_published_enable_mask = 0U;

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

static int16 vision_float_to_i16_x100(float value)
{
    if (value > 327.67f)
    {
        return 32767;
    }
    if (value < -327.68f)
    {
        return -32768;
    }
    return (int16)(value * 100.0f);
}

static uint32 vision_max_u32(uint32 a, uint32 b)
{
    return (a > b) ? a : b;
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

static uint8 vision_ipc_core1_command_wants_pvc(const vision_ipc_command_t *cmd)
{
    const uint8 active_is_pvc = (uint8)(cmd->active_target == VISION_TARGET_PVC_ENTRY);
    const uint8 mask_has_pvc = (uint8)((cmd->enable_mask & VISION_MASK_PVC_ENTRY) != 0U);
    return (uint8)(active_is_pvc || mask_has_pvc);
}

static uint8 vision_ipc_core1_command_wants_line(const vision_ipc_command_t *cmd)
{
    const uint8 active_is_bridge = (uint8)(cmd->active_target == VISION_TARGET_BRIDGE);
    const uint8 mask_has_bridge = (uint8)((cmd->enable_mask & VISION_MASK_BRIDGE) != 0U);
    return (uint8)(active_is_bridge || mask_has_bridge);
}

static void vision_ipc_core1_fill_pvc(vision_ipc_packet_t *packet,
                                      const volatile pvc_vision_output_t *pvc_output)
{
    pvc_vision_output_t pvc;
    const pvc_vision_frame_result_t *ctrl;

    if ((pvc_output == NULL) || (pvc_output->frame_id == 0U))
    {
        return;
    }

    pvc = *pvc_output;
    ctrl = pvc.stable_detected ? &pvc.stable : &pvc.raw;

    packet->valid_mask = (uint16)(packet->valid_mask | VISION_VALID_PVC | VISION_VALID_PROFILE);
    packet->frame_id = vision_max_u32(packet->frame_id, pvc.frame_id);
    packet->frame_dt_us = (uint16)g_pvc_vision_frame_profiler.last_us;
    packet->cost_us = (uint16)g_pvc_vision_cost_profiler.last_us;

    packet->pvc_detected = pvc.raw.detected;
    packet->pvc_stable_detected = pvc.stable_detected;
    packet->pvc_confidence_u16 = vision_confidence_to_u16(ctrl->confidence);
    packet->pvc_forward_mm = ctrl->forward_mm;
    packet->pvc_lateral_mm = ctrl->lateral_mm;
    packet->pvc_yaw_error_deg_x100 = ctrl->yaw_error_deg_x100;
    packet->pvc_entry_bottom_y = ctrl->entry_bottom_y;
    packet->pvc_entry_top_y = ctrl->entry_top_y;
    packet->pvc_bbox_xmin = ctrl->bbox_xmin;
    packet->pvc_bbox_ymin = ctrl->bbox_ymin;
    packet->pvc_bbox_xmax = ctrl->bbox_xmax;
    packet->pvc_bbox_ymax = ctrl->bbox_ymax;
    packet->pvc_area = ctrl->area;
    packet->pvc_component_count = ctrl->component_count;
    packet->pvc_candidate_count = ctrl->candidate_count;
}

static void vision_ipc_core1_fill_line(vision_ipc_packet_t *packet,
                                       const volatile line_vision_output_t *line_output)
{
    line_vision_output_t line;
    const line_vision_frame_result_t *ctrl;

    if ((line_output == NULL) || (line_output->frame_id == 0U))
    {
        return;
    }

    line = *line_output;
    ctrl = (line.stable_detected || line.bridge_stable_detected) ? &line.stable : &line.raw;

    packet->valid_mask = (uint16)(packet->valid_mask | VISION_VALID_BRIDGE | VISION_VALID_PROFILE);
    packet->frame_id = vision_max_u32(packet->frame_id, line.frame_id);
    packet->frame_dt_us = (uint16)g_line_vision_frame_profiler.last_us;
    packet->cost_us = (uint16)g_line_vision_cost_profiler.last_us;

    packet->line_detected = line.raw_detected;
    packet->line_stable_detected = line.stable_detected;
    packet->line_confidence_u16 = vision_confidence_to_u16(ctrl->confidence);
    packet->line_lateral_px_x100 = vision_float_to_i16_x100(ctrl->lateral_error_px);
    packet->line_yaw_error_deg_x100 = vision_float_to_i16_x100(ctrl->yaw_error_deg);
    packet->line_x_bottom_x100 = vision_float_to_i16_x100(ctrl->line_x_bottom);
    packet->line_x_lookahead_x100 = vision_float_to_i16_x100(ctrl->line_x_lookahead);
    packet->line_points_used = ctrl->points_used;
    packet->line_y_span = ctrl->y_span;
    packet->line_rmse_px_x100 = (uint16)(ctrl->fit_rmse * 100.0f);
    packet->line_roi_white_ratio_u16 = vision_confidence_to_u16(ctrl->roi_white_ratio);
    packet->line_speed_hint = (int16)ctrl->target_speed_hint;

    packet->line_bridge_detected = line.bridge_raw_detected;
    packet->line_bridge_stable_detected = line.bridge_stable_detected;
    packet->line_bridge_confidence_u16 = vision_confidence_to_u16(ctrl->bridge_confidence);
    packet->line_bridge_component_count = ctrl->bridge_component_count;
    packet->line_bridge_bbox_xmin = ctrl->bridge_bbox_xmin;
    packet->line_bridge_bbox_ymin = ctrl->bridge_bbox_ymin;
    packet->line_bridge_bbox_xmax = ctrl->bridge_bbox_xmax;
    packet->line_bridge_bbox_ymax = ctrl->bridge_bbox_ymax;

    packet->bridge_detected = line.bridge_stable_detected;
    packet->bridge_count = ctrl->bridge_component_count;
    packet->bridge_side = 0;
    packet->bridge_exit_seen = 0U;
    packet->bridge_center_err = packet->line_lateral_px_x100;
}

void VisionIpc_Core1_Init(void)
{
    memset(&g_core1_command_shadow, 0, sizeof(g_core1_command_shadow));
    g_core1_command_shadow.active_target = VISION_TARGET_NONE;
    g_core1_command_shadow.enable_mask = 0U;
    g_core1_command_shadow.pvc_min_score_u16 = 580U;
    g_core1_result_seq = 0U;
    g_core1_pvc_enabled = 0U;
    g_core1_line_enabled = 0U;
    g_core1_pvc_reset_request = 0U;
    g_core1_line_reset_request = 0U;
    g_core1_last_published_pvc_frame_id = 0U;
    g_core1_last_published_line_frame_id = 0U;
    g_core1_last_published_command_seq = 0U;
    g_core1_last_published_enable_mask = 0U;
    VisionIpc_Core1_PublishIdle();
}

void VisionIpc_Core1_Update_2ms(void)
{
    uint32 pvc_frame_id = 0U;
    uint32 line_frame_id = 0U;
    uint8 should_publish = 0U;

    VisionIpc_Core1_PollCommand();

    if ((g_core1_pvc_enabled == 0U) && (g_core1_line_enabled == 0U))
    {
        if (g_core1_last_published_enable_mask != 0U)
        {
            VisionIpc_Core1_PublishIdle();
            g_core1_last_published_enable_mask = 0U;
        }
        return;
    }

    if ((g_core1_pvc_enabled != 0U) && (g_pvc_vision_output_write_busy == 0U))
    {
        pvc_frame_id = pvc_vision_get_output()->frame_id;
    }
    if ((g_core1_line_enabled != 0U) && (g_line_vision_output_write_busy == 0U))
    {
        line_frame_id = line_vision_get_output()->frame_id;
    }

    if ((pvc_frame_id != 0U) && (pvc_frame_id != g_core1_last_published_pvc_frame_id))
    {
        should_publish = 1U;
    }
    if ((line_frame_id != 0U) && (line_frame_id != g_core1_last_published_line_frame_id))
    {
        should_publish = 1U;
    }
    if (g_core1_command_shadow.seq != g_core1_last_published_command_seq)
    {
        should_publish = 1U;
    }

    if (should_publish)
    {
        VisionIpc_Core1_PublishCurrent();
        g_core1_last_published_pvc_frame_id = pvc_frame_id;
        g_core1_last_published_line_frame_id = line_frame_id;
        g_core1_last_published_command_seq = g_core1_command_shadow.seq;
        g_core1_last_published_enable_mask = g_core1_command_shadow.enable_mask;
    }
}

void VisionIpc_Core1_PollCommand(void)
{
    vision_ipc_command_t cmd;
    uint8 next_pvc_enabled;
    uint8 next_line_enabled;

    SCB_CleanInvalidateDCache_by_Addr((void *)&g_vision_ipc_command, sizeof(g_vision_ipc_command));
    cmd = g_vision_ipc_command;

    if (vision_ipc_command_is_valid(&cmd))
    {
        if (cmd.seq == g_core1_command_shadow.seq)
        {
            return;
        }

        g_core1_command_shadow = cmd;
        next_pvc_enabled = vision_ipc_core1_command_wants_pvc(&g_core1_command_shadow);
        next_line_enabled = vision_ipc_core1_command_wants_line(&g_core1_command_shadow);

        if (next_pvc_enabled != g_core1_pvc_enabled)
        {
            g_core1_pvc_reset_request = 1U;
            g_core1_pvc_enabled = next_pvc_enabled;
            g_core1_last_published_pvc_frame_id = 0U;
        }
        if (next_line_enabled != g_core1_line_enabled)
        {
            g_core1_line_reset_request = 1U;
            g_core1_line_enabled = next_line_enabled;
            g_core1_last_published_line_frame_id = 0U;
        }
    }
}

uint8 VisionIpc_Core1_ShouldRunPvc(void)
{
    return g_core1_pvc_enabled;
}

uint8 VisionIpc_Core1_TakePvcResetRequest(void)
{
    if (g_core1_pvc_reset_request)
    {
        g_core1_pvc_reset_request = 0U;
        return 1U;
    }
    return 0U;
}

uint8 VisionIpc_Core1_ShouldRunBridgeLine(void)
{
    return g_core1_line_enabled;
}

uint8 VisionIpc_Core1_TakeLineResetRequest(void)
{
    if (g_core1_line_reset_request)
    {
        g_core1_line_reset_request = 0U;
        return 1U;
    }
    return 0U;
}

void VisionIpc_Core1_PublishPvc(const volatile pvc_vision_output_t *pvc_output)
{
    vision_ipc_packet_t packet;

    memset(&packet, 0, sizeof(packet));
    packet.active_target = VISION_TARGET_PVC_ENTRY;
    packet.stable_target = VISION_TARGET_NONE;
    packet.valid_mask = VISION_VALID_COMMON;
    vision_ipc_core1_fill_pvc(&packet, pvc_output);
    if (packet.pvc_stable_detected)
    {
        packet.stable_target = VISION_TARGET_PVC_ENTRY;
    }
    packet.detected = packet.pvc_stable_detected ? 1U : packet.pvc_detected;
    packet.raw_detected = packet.pvc_detected;
    packet.confidence_u16 = packet.pvc_confidence_u16;
    packet.forward_mm = packet.pvc_forward_mm;
    packet.lateral_mm = packet.pvc_lateral_mm;
    packet.yaw_error_deg_x100 = packet.pvc_yaw_error_deg_x100;
    vision_ipc_core1_write_packet(&packet);
}

void VisionIpc_Core1_PublishCurrent(void)
{
    vision_ipc_packet_t packet;

    memset(&packet, 0, sizeof(packet));
    packet.active_target = g_core1_command_shadow.active_target;
    packet.stable_target = VISION_TARGET_NONE;
    packet.valid_mask = VISION_VALID_COMMON;

    if ((g_core1_pvc_enabled != 0U) && (g_pvc_vision_output_write_busy == 0U))
    {
        vision_ipc_core1_fill_pvc(&packet, pvc_vision_get_output());
    }
    if ((g_core1_line_enabled != 0U) && (g_line_vision_output_write_busy == 0U))
    {
        vision_ipc_core1_fill_line(&packet, line_vision_get_output());
    }

    if (packet.active_target == VISION_TARGET_BRIDGE)
    {
        packet.stable_detected = (uint8)(packet.line_stable_detected || packet.line_bridge_stable_detected);
        packet.detected = (uint8)(packet.line_stable_detected || packet.line_bridge_stable_detected);
        packet.raw_detected = (uint8)(packet.line_detected || packet.line_bridge_detected);
        packet.confidence_u16 = packet.line_bridge_stable_detected ?
                                packet.line_bridge_confidence_u16 :
                                packet.line_confidence_u16;
        packet.lateral_mm = packet.line_lateral_px_x100;
        packet.yaw_error_deg_x100 = packet.line_yaw_error_deg_x100;
        if (packet.stable_detected)
        {
            packet.stable_target = VISION_TARGET_BRIDGE;
        }
    }
    else
    {
        packet.stable_detected = packet.pvc_stable_detected;
        packet.detected = packet.pvc_stable_detected ? 1U : packet.pvc_detected;
        packet.raw_detected = packet.pvc_detected;
        packet.confidence_u16 = packet.pvc_confidence_u16;
        packet.forward_mm = packet.pvc_forward_mm;
        packet.lateral_mm = packet.pvc_lateral_mm;
        packet.yaw_error_deg_x100 = packet.pvc_yaw_error_deg_x100;
        if (packet.pvc_stable_detected)
        {
            packet.stable_target = VISION_TARGET_PVC_ENTRY;
        }
    }

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
