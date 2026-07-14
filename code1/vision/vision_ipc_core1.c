#include "vision_ipc_core1.h"
#include <string.h>

#if defined(__ICCARM__)
#pragma data_alignment = 32
#pragma location = VISION_IPC_COMMAND_ADDR
/* g_vision_ipc_command 存放从 0 核发来的命令 */
__no_init volatile vision_ipc_command_t g_vision_ipc_command;

#pragma data_alignment = 32
#pragma location = VISION_IPC_RESULT_ADDR
/* g_vision_ipc_result 存放 1 核计算后要发给 0 核的视觉结果 */
__no_init volatile vision_ipc_packet_t g_vision_ipc_result;
#else
volatile vision_ipc_command_t g_vision_ipc_command;
volatile vision_ipc_packet_t g_vision_ipc_result;
#endif

static vision_ipc_command_t g_core1_command_shadow;
static uint32 g_core1_result_seq = 0U;

static volatile uint8 g_core1_pvc_enabled = 0U;
static volatile uint8 g_core1_line_enabled = 0U;
static volatile uint8 g_core1_bumpy_enabled = 0U;

static volatile uint8 g_core1_pvc_reset_request = 0U;
static volatile uint8 g_core1_line_reset_request = 0U;
static volatile uint8 g_core1_bumpy_reset_request = 0U;

static uint32 g_core1_last_published_pvc_frame_id = 0U;
static uint32 g_core1_last_published_line_frame_id = 0U;
static uint32 g_core1_last_published_bumpy_frame_id = 0U;
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

static uint8 vision_ipc_core1_command_wants_bumpy(const vision_ipc_command_t *cmd)
{
    const uint8 active_is_bumpy = (uint8)(cmd->active_target == VISION_TARGET_BUMPY);
    const uint8 mask_has_bumpy = (uint8)((cmd->enable_mask & VISION_MASK_BUMPY) != 0U);
    return (uint8)(active_is_bumpy || mask_has_bumpy);
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
    packet->pvc_target_x_px_x100 = ctrl->target_x_px_x100;
    packet->pvc_steer_error_px_x100 = ctrl->steer_error_px_x100;
    packet->pvc_forward_mm = ctrl->forward_mm;
    packet->pvc_lateral_mm = ctrl->lateral_mm;
    packet->pvc_phy_x_mm = ctrl->phy_x_mm;
    packet->pvc_phy_y_mm = ctrl->phy_y_mm;
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

/**
 * @brief 将视觉检测结果提取并填充到 IPC 数据包中
 * 
 * @param packet 将要发送给 0 核的数据包
 * @param bridge_output 当前最新的单边桥检测输出结果
 */
static void vision_ipc_core1_fill_bridge(vision_ipc_packet_t *packet,
                                         const volatile bridge_vision_output_t *bridge_output)
{
    bridge_vision_output_t bridge;
    const bridge_vision_frame_result_t *ctrl;

    /* 如果没有有效数据，直接退出 */
    if ((bridge_output == NULL) || (bridge_output->frame_id == 0U))
    {
        return;
    }

    /* 拷贝数据，防止并发修改 */
    bridge = *bridge_output;
    ctrl = (bridge.stable_detected || bridge.bridge_stable_detected) ? &bridge.stable : &bridge.raw;

    packet->valid_mask = (uint16)(packet->valid_mask | VISION_VALID_BRIDGE | VISION_VALID_PROFILE);
    packet->frame_id = vision_max_u32(packet->frame_id, bridge.frame_id);
    packet->frame_dt_us = (uint16)g_bridge_vision_frame_profiler.last_us;
    packet->cost_us = (uint16)g_bridge_vision_cost_profiler.last_us;

    packet->line_detected = bridge.raw_detected;
    packet->line_stable_detected = bridge.stable_detected;
    packet->line_confidence_u16 = vision_confidence_to_u16(ctrl->confidence);
    packet->line_lateral_px_x100 = vision_float_to_i16_x100(ctrl->lateral_error_px);
    packet->line_yaw_error_deg_x100 = vision_float_to_i16_x100(ctrl->yaw_error_deg);
    packet->line_x_bottom_x100 = vision_float_to_i16_x100(ctrl->line_x_bottom);
    packet->line_x_lookahead_x100 = vision_float_to_i16_x100(ctrl->line_x_lookahead);
    packet->line_points_used = (uint8)(ctrl->left_line_visible + ctrl->right_line_visible);
    packet->line_y_span = (ctrl->bbox_ymax >= ctrl->bbox_ymin) ?
                          (uint8)(ctrl->bbox_ymax - ctrl->bbox_ymin + 1U) : 0U;
    packet->line_rmse_px_x100 = 0U;
    packet->line_roi_white_ratio_u16 = 0U;
    packet->line_speed_hint = 0;

    packet->line_bridge_detected = bridge.bridge_raw_detected;
    packet->line_bridge_stable_detected = bridge.bridge_stable_detected;
    packet->line_bridge_confidence_u16 = vision_confidence_to_u16(ctrl->bridge_confidence);
    packet->line_bridge_component_count = ctrl->bridge_detected ? 1U : 0U;
    packet->line_bridge_bbox_xmin = ctrl->bbox_xmin;
    packet->line_bridge_bbox_ymin = ctrl->bbox_ymin;
    packet->line_bridge_bbox_xmax = ctrl->bbox_xmax;
    packet->line_bridge_bbox_ymax = ctrl->bbox_ymax;

    packet->bridge_detected = bridge.bridge_stable_detected;
    packet->bridge_count = ctrl->bridge_detected ? 1U : 0U;
    packet->bridge_side = 0;
    packet->bridge_exit_seen = 0U;
    packet->bridge_center_err = packet->line_lateral_px_x100;
}

static void vision_ipc_core1_fill_bumpy(vision_ipc_packet_t *packet,
                                        const volatile bumpy_vision_output_t *bumpy_output)
{
    bumpy_vision_output_t bumpy;
    const bumpy_vision_frame_result_t *ctrl;

    if ((bumpy_output == NULL) || (bumpy_output->frame_id == 0U))
    {
        return;
    }

    bumpy = *bumpy_output;
    ctrl = bumpy.stable_detected ? &bumpy.stable : &bumpy.raw;

    packet->valid_mask = (uint16)(packet->valid_mask | VISION_VALID_BUMPY | VISION_VALID_PROFILE);
    packet->frame_id = vision_max_u32(packet->frame_id, bumpy.frame_id);
    packet->frame_dt_us = (uint16)g_bumpy_vision_frame_profiler.last_us;
    packet->cost_us = (uint16)g_bumpy_vision_cost_profiler.last_us;

    packet->bumpy_detected = bumpy.raw_detected;
    packet->bumpy_stable_detected = bumpy.stable_detected;
    packet->bumpy_confidence_u16 = ctrl->confidence_u16;
    packet->bumpy_steer_error_px_x100 = ctrl->steer_error_px_x100;
    packet->bumpy_target_x_px_x100 = ctrl->target_x_px_x100;
    packet->bumpy_phase = ctrl->phase;
    packet->bumpy_mode = ctrl->mode;
    packet->bumpy_component_count = ctrl->component_count;
    packet->bumpy_candidate_count = ctrl->candidate_count;
    packet->bumpy_run_count = ctrl->run_count;
    packet->bumpy_rib_count = ctrl->rib_count;
    packet->bumpy_centerline_rows = ctrl->centerline_rows;
    packet->bumpy_centerline_bottom_rows = ctrl->centerline_bottom_rows;
    packet->bumpy_centerline_top_y = ctrl->centerline_top_y;
    packet->bumpy_centerline_bottom_y = ctrl->centerline_bottom_y;
    packet->bumpy_bbox_xmin = ctrl->bbox_xmin;
    packet->bumpy_bbox_ymin = ctrl->bbox_ymin;
    packet->bumpy_bbox_xmax = ctrl->bbox_xmax;
    packet->bumpy_bbox_ymax = ctrl->bbox_ymax;
    packet->bumpy_bbox_area = ctrl->bbox_area;
    packet->bumpy_white_threshold_x10 = ctrl->white_threshold_x10;
    packet->bumpy_dark_threshold_x10 = ctrl->dark_threshold_x10;
    packet->bumpy_target_x_ipm_mm = ctrl->target_x_ipm_mm;
    packet->bumpy_target_y_ipm_mm = ctrl->target_y_ipm_mm;
    packet->bumpy_steer_error_ipm_mm = ctrl->steer_error_ipm_mm;
    packet->bumpy_start_seen = bumpy.start_seen;
    packet->bumpy_end_seen = bumpy.end_seen;
    packet->bumpy_local_s_mm = ctrl->local_s_mm;
}

void VisionIpc_Core1_Init(void)
{
    /* 清空并初始化命令缓存 */
    memset(&g_core1_command_shadow, 0, sizeof(g_core1_command_shadow));
    g_core1_command_shadow.active_target = VISION_TARGET_NONE;
    g_core1_command_shadow.enable_mask = 0U;
    g_core1_command_shadow.pvc_min_score_u16 = 580U;

    g_core1_result_seq = 0U;

    g_core1_pvc_enabled = 0U;
    g_core1_line_enabled = 0U;
    g_core1_bumpy_enabled = 0U;

    g_core1_pvc_reset_request = 0U;
    g_core1_line_reset_request = 0U;
    g_core1_bumpy_reset_request = 0U;

    g_core1_last_published_pvc_frame_id = 0U;
    g_core1_last_published_line_frame_id = 0U;
    g_core1_last_published_bumpy_frame_id = 0U;
    g_core1_last_published_command_seq = 0U;
    g_core1_last_published_enable_mask = 0U;

    VisionIpc_Core1_PublishIdle();
}

/**
 * @brief 1 核 IPC 的定时更新函数 (建议每 2 毫秒调用一次)
 * 
 * @note 此函数是通信的"心脏"：
 *       1. 检查 0 核是否有新命令。
 *       2. 如果没有开启任何检测，且刚刚关闭，则发一个空包。
 *       3. 如果开启了检测，检查视觉模块是否有产生新数据。
 *       4. 如果有新数据或者命令序号发生变化，则打包发布给 0 核。
 */
void VisionIpc_Core1_Update_2ms(void)
{
    uint32 pvc_frame_id = 0U;
    uint32 line_frame_id = 0U;
    uint32 bumpy_frame_id = 0U;
    uint8 should_publish = 0U;

    VisionIpc_Core1_PollCommand();

    if ((g_core1_pvc_enabled == 0U) &&
        (g_core1_line_enabled == 0U) &&
        (g_core1_bumpy_enabled == 0U))
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
    if ((g_core1_line_enabled != 0U) && (g_bridge_vision_output_write_busy == 0U))
    {
        line_frame_id = bridge_vision_get_output()->frame_id;
    }
    if ((g_core1_bumpy_enabled != 0U) && (g_bumpy_vision_output_write_busy == 0U))
    {
        bumpy_frame_id = bumpy_vision_get_output()->frame_id;
    }

    /* 4. 判断是否需要发布数据 */
    /* 如果产生了新的 PVC 帧 */
    if ((pvc_frame_id != 0U) && (pvc_frame_id != g_core1_last_published_pvc_frame_id))
    {
        should_publish = 1U;
    }
    /* 如果产生了新的 直线 帧 */
    if ((line_frame_id != 0U) && (line_frame_id != g_core1_last_published_line_frame_id))
    {
        should_publish = 1U;
    }
    if ((bumpy_frame_id != 0U) && (bumpy_frame_id != g_core1_last_published_bumpy_frame_id))
    {
        should_publish = 1U;
    }
    if (g_core1_command_shadow.seq != g_core1_last_published_command_seq)
    {
        should_publish = 1U;
    }

    /* 5. 执行发布并更新记录 */
    if (should_publish)
    {
        VisionIpc_Core1_PublishCurrent();
        g_core1_last_published_pvc_frame_id = pvc_frame_id;
        g_core1_last_published_line_frame_id = line_frame_id;
        g_core1_last_published_bumpy_frame_id = bumpy_frame_id;
        g_core1_last_published_command_seq = g_core1_command_shadow.seq;
        g_core1_last_published_enable_mask = g_core1_command_shadow.enable_mask;
    }
}

/**
 * @brief 拉取并解析 0 核发来的命令
 * 
 * @note 此函数从共享内存读取命令结构体，判断是否有新命令。
 *       如果有，解析其意图（比如开启或关闭某些检测），并生成重置请求。
 */
void VisionIpc_Core1_PollCommand(void)
{
    vision_ipc_command_t cmd;
    uint8 next_pvc_enabled;
    uint8 next_line_enabled;
    uint8 next_bumpy_enabled;

    /* 刷新数据缓存，确保从物理内存读到 0 核最新写入的命令 */
    SCB_CleanInvalidateDCache_by_Addr((void *)&g_vision_ipc_command, sizeof(g_vision_ipc_command));
    cmd = g_vision_ipc_command;

    if (vision_ipc_command_is_valid(&cmd) == 0U)
    {
        return;
    }
    if (cmd.seq == g_core1_command_shadow.seq)
    {
        return;
    }

    g_core1_command_shadow = cmd;

    next_pvc_enabled = vision_ipc_core1_command_wants_pvc(&g_core1_command_shadow);
    next_line_enabled = vision_ipc_core1_command_wants_line(&g_core1_command_shadow);
    next_bumpy_enabled = vision_ipc_core1_command_wants_bumpy(&g_core1_command_shadow);

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
    if (next_bumpy_enabled != g_core1_bumpy_enabled)
    {
        g_core1_bumpy_reset_request = 1U;
        g_core1_bumpy_enabled = next_bumpy_enabled;
        g_core1_last_published_bumpy_frame_id = 0U;
    }
}

/**
 * @brief 查询当前是否应该运行 PVC 检测算法
 * 
 * @return uint8 1: 需要运行; 0: 不需要
 */
uint8 VisionIpc_Core1_ShouldRunPvc(void)
{
    return g_core1_pvc_enabled;
}

/**
 * @brief 获取并清除 PVC 重置请求
 * 
 * @return uint8 1: 有重置请求; 0: 没有
 * @note "拿走(Take)"的意思是读取后就会将该标志位清零，保证只重置一次。
 */
uint8 VisionIpc_Core1_TakePvcResetRequest(void)
{
    if (g_core1_pvc_reset_request)
    {
        g_core1_pvc_reset_request = 0U;
        return 1U;
    }
    return 0U;
}

/**
 * @brief 查询当前是否应该运行 直线/桥梁 检测算法
 * 
 * @return uint8 1: 需要运行; 0: 不需要
 */
uint8 VisionIpc_Core1_ShouldRunBridgeLine(void)
{
    return g_core1_line_enabled;
}

/**
 * @brief 获取并清除 直线/桥梁 重置请求
 * 
 * @return uint8 1: 有重置请求; 0: 没有
 */
uint8 VisionIpc_Core1_TakeLineResetRequest(void)
{
    if (g_core1_line_reset_request)
    {
        g_core1_line_reset_request = 0U;
        return 1U;
    }
    return 0U;
}

uint8 VisionIpc_Core1_ShouldRunBumpy(void)
{
    return g_core1_bumpy_enabled;
}

uint8 VisionIpc_Core1_TakeBumpyResetRequest(void)
{
    if (g_core1_bumpy_reset_request)
    {
        g_core1_bumpy_reset_request = 0U;
        return 1U;
    }
    return 0U;
}

uint8 VisionIpc_Core1_GetActiveTarget(void)
{
    return g_core1_command_shadow.active_target;
}

uint16 VisionIpc_Core1_GetEnableMask(void)
{
    return g_core1_command_shadow.enable_mask;
}

void VisionIpc_Core1_PublishPvc(const volatile pvc_vision_output_t *pvc_output)
{
    vision_ipc_packet_t packet;

    memset(&packet, 0, sizeof(packet));
    packet.active_target = VISION_TARGET_PVC_ENTRY;
    packet.stable_target = VISION_TARGET_NONE;
    packet.valid_mask = VISION_VALID_COMMON;
    
    /* 填充具体的 PVC 参数 */
    vision_ipc_core1_fill_pvc(&packet, pvc_output);
    
    /* 如果检测稳定，则确认稳定目标为 PVC */
    if (packet.pvc_stable_detected)
    {
        packet.stable_target = VISION_TARGET_PVC_ENTRY;
    }
    /* 同步全局检测状态变量 */
    packet.detected = packet.pvc_stable_detected ? 1U : packet.pvc_detected;
    packet.raw_detected = packet.pvc_detected;
    packet.confidence_u16 = packet.pvc_confidence_u16;
    packet.forward_mm = packet.pvc_forward_mm;
    packet.lateral_mm = packet.pvc_lateral_mm;
    packet.yaw_error_deg_x100 = packet.pvc_yaw_error_deg_x100;
    
    /* 最终写入共享内存 */
    vision_ipc_core1_write_packet(&packet);
}

/**
 * @brief 发布当前的综合检测结果 (包含被开启的所有视觉模块数据)
 * 
 * @note 这个函数会将 PVC 和 直线检测的结果整合到一个数据包里发给 0 核。
 */
void VisionIpc_Core1_PublishCurrent(void)
{
    vision_ipc_packet_t packet;

    memset(&packet, 0, sizeof(packet));
    packet.active_target = g_core1_command_shadow.active_target; /* 当前响应的目标任务 */
    packet.stable_target = VISION_TARGET_NONE;
    packet.valid_mask = VISION_VALID_COMMON;

    /* 分别获取已开启且未被占用的模块数据 */
    if ((g_core1_pvc_enabled != 0U) && (g_pvc_vision_output_write_busy == 0U))
    {
        vision_ipc_core1_fill_pvc(&packet, pvc_vision_get_output());
    }
    if ((g_core1_line_enabled != 0U) && (g_bridge_vision_output_write_busy == 0U))
    {
        vision_ipc_core1_fill_bridge(&packet, bridge_vision_get_output());
    }
    if ((g_core1_bumpy_enabled != 0U) && (g_bumpy_vision_output_write_busy == 0U))
    {
        vision_ipc_core1_fill_bumpy(&packet, bumpy_vision_get_output());
    }

    /* 根据当前的活跃目标，将相应模块的数据提取到包的通用字段中供 0 核直接使用 */
    if (packet.active_target == VISION_TARGET_BRIDGE)
    {
        /* 桥梁模式下，综合直线和桥梁的结果 */
        packet.stable_detected = (uint8)(packet.line_stable_detected || packet.line_bridge_stable_detected);
        packet.detected = (uint8)(packet.line_stable_detected || packet.line_bridge_stable_detected);
        packet.raw_detected = (uint8)(packet.line_detected || packet.line_bridge_detected);
        
        /* 桥梁优先级高于普通直线 */
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
    else if (packet.active_target == VISION_TARGET_BUMPY)
    {
        packet.stable_detected = packet.bumpy_stable_detected;
        packet.detected = packet.bumpy_stable_detected ? 1U : packet.bumpy_detected;
        packet.raw_detected = packet.bumpy_detected;
        packet.confidence_u16 = packet.bumpy_confidence_u16;
        packet.forward_mm = (int16)packet.bumpy_local_s_mm;
        packet.lateral_mm = packet.bumpy_steer_error_px_x100;
        packet.yaw_error_deg_x100 = 0;

        if (packet.bumpy_stable_detected)
        {
            packet.stable_target = VISION_TARGET_BUMPY;
        }
    }
    else
    {
        /* 其他模式 (如 PVC 模式) 下，使用 PVC 结果 */
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

    /* 写入共享内存 */
    vision_ipc_core1_write_packet(&packet);
}

/**
 * @brief 发布空闲数据包
 * 
 * @note 当所有视觉任务关闭时，发一个空包告诉 0 核 "我现在在休息"。
 */
void VisionIpc_Core1_PublishIdle(void)
{
    vision_ipc_packet_t packet;

    memset(&packet, 0, sizeof(packet));
    packet.active_target = g_core1_command_shadow.active_target;
    packet.stable_target = VISION_TARGET_NONE;
    packet.valid_mask = VISION_VALID_COMMON;
    
    /* 写入共享内存 */
    vision_ipc_core1_write_packet(&packet);
}
