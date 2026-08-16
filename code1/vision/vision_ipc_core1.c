#include "vision_ipc_core1.h"
#include "bridge_v2_arbiter.h"
#include "bridge_output_filter.h"
#include "bridge_pvc_vision.h"
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
static volatile uint8 g_core1_bridge_enabled = 0U;
static volatile uint8 g_core1_bumpy_enabled = 0U;

static volatile uint8 g_core1_pvc_reset_request = 0U;
static volatile uint8 g_core1_bridge_reset_request = 0U;
static volatile uint8 g_core1_bumpy_reset_request = 0U;

static uint32 g_core1_last_published_pvc_frame_id = 0U;
static uint32 g_core1_last_published_bridge_frame_id = 0U;
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

static uint8 vision_ipc_core1_command_wants_bridge(const vision_ipc_command_t *cmd)
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

/* 新单边桥管线计时 (main_cm7_1.c 定义) */
extern volatile runtime_profiler_t g_bridge_v2_cost_profiler;

/**
 * @brief 填充新单边桥管线 (bridge_detect) 的仲裁输出 b2_* 字段 (C04/C21)
 * @note  数据来自 bridge_output_filter (中值滤波层, 2026-08-14 起仲裁输出
 *        必须经滤波后才发布给 0核); 写忙时跳过 (防撕裂)。
 */
static void vision_ipc_core1_fill_bridge_v2(vision_ipc_packet_t *packet)
{
    const bridge_v2_arb_t *arb;

    if (bridge_output_filter_is_busy())
    {
        return;
    }
    arb = bridge_output_filter_get();

    packet->valid_mask = (uint16)(packet->valid_mask | VISION_VALID_BRIDGE | VISION_VALID_BRIDGE_V2);
    packet->frame_id = vision_max_u32(packet->frame_id, bridge_output_filter_get_frame_id());
    packet->cost_us = (uint16)g_bridge_v2_cost_profiler.last_us;

    packet->b2_valid        = arb->valid;
    packet->b2_source       = arb->source;
    packet->b2_mode         = arb->mode;
    packet->b2_gate         = arb->gate;
    packet->b2_has_top      = arb->has_top;
    packet->b2_line_u_lo    = arb->u_lo;
    packet->b2_line_u_hi    = arb->u_hi;
    packet->b2_line_a_x1000 = arb->line_a_x1000;
    packet->b2_line_b_x100  = arb->line_b_x100;
    packet->b2_top_a_x1000  = arb->top_a_x1000;
    packet->b2_top_b_x100   = arb->top_b_x100;
    packet->b2_spacing_x100 = arb->spacing_x100;
    packet->b2_mid_ratio_x1000 = arb->mid_ratio_x1000;

    /* 旁路透传: 单边桥专用 PVC 的 IPM 物理坐标 (LQR 方向控制用)。
       bridge_pvc_vision 自带连续3帧确认+平滑, 直接读 stable 即可。 */
    {
        bridge_pvc_vision_output_t pvc_local;
        pvc_local = *bridge_pvc_vision_get_output();
        if (pvc_local.stable_detected)
        {
            packet->pvc_phy_x_mm = pvc_local.stable.phy_x_mm;
            packet->pvc_phy_y_mm = pvc_local.stable.phy_y_mm;
        }
    }
}

static void vision_ipc_core1_fill_bumpy(vision_ipc_packet_t *packet,
                                        const volatile bumpy_vision_output_t *bumpy_output)
{
    bumpy_vision_output_t bumpy;

    if ((bumpy_output == NULL) || (bumpy_output->frame_id == 0U))
    {
        return;
    }

    bumpy = *bumpy_output;

    packet->valid_mask = (uint16)(packet->valid_mask | VISION_VALID_BUMPY | VISION_VALID_PROFILE);
    packet->frame_id = vision_max_u32(packet->frame_id, bumpy.frame_id);
    packet->frame_dt_us = (uint16)g_bumpy_vision_frame_profiler.last_us;
    packet->cost_us = (uint16)g_bumpy_vision_cost_profiler.last_us;

    packet->bumpy_detected = bumpy.bumpy_detected;
    packet->bumpy_direction_x = bumpy.direction_x;
    packet->bumpy_direction_y = bumpy.direction_y;
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
    g_core1_bridge_enabled = 0U;
    g_core1_bumpy_enabled = 0U;

    g_core1_pvc_reset_request = 0U;
    g_core1_bridge_reset_request = 0U;
    g_core1_bumpy_reset_request = 0U;

    g_core1_last_published_pvc_frame_id = 0U;
    g_core1_last_published_bridge_frame_id = 0U;
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
    uint32 bridge_frame_id = 0U;
    uint32 bumpy_frame_id = 0U;
    uint8 should_publish = 0U;

    VisionIpc_Core1_PollCommand();

    if ((g_core1_pvc_enabled == 0U) &&
        (g_core1_bridge_enabled == 0U) &&
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
    if (g_core1_bridge_enabled != 0U)
    {
        /* 桥检测源 = 新管线中值滤波层 (仲裁输出的唯一发布口径) */
        if (bridge_output_filter_is_busy() == 0U)
        {
            bridge_frame_id = bridge_output_filter_get_frame_id();
        }
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
    if ((bridge_frame_id != 0U) && (bridge_frame_id != g_core1_last_published_bridge_frame_id))
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
        g_core1_last_published_bridge_frame_id = bridge_frame_id;
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
    uint8 next_bridge_enabled;
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
    next_bridge_enabled = vision_ipc_core1_command_wants_bridge(&g_core1_command_shadow);
    next_bumpy_enabled = vision_ipc_core1_command_wants_bumpy(&g_core1_command_shadow);

    if (next_pvc_enabled != g_core1_pvc_enabled)
    {
        g_core1_pvc_reset_request = 1U;
        g_core1_pvc_enabled = next_pvc_enabled;
        g_core1_last_published_pvc_frame_id = 0U;
    }
    if (next_bridge_enabled != g_core1_bridge_enabled)
    {
        g_core1_bridge_reset_request = 1U;
        g_core1_bridge_enabled = next_bridge_enabled;
        g_core1_last_published_bridge_frame_id = 0U;
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
uint8 VisionIpc_Core1_ShouldRunBridge(void)
{
    return g_core1_bridge_enabled;
}

/**
 * @brief 获取并清除 直线/桥梁 重置请求
 * 
 * @return uint8 1: 有重置请求; 0: 没有
 */
uint8 VisionIpc_Core1_TakeBridgeResetRequest(void)
{
    if (g_core1_bridge_reset_request)
    {
        g_core1_bridge_reset_request = 0U;
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
    if (g_core1_bridge_enabled != 0U)
    {
        /* 发布 b2_* (中值滤波层输出, 2026-08-14) */
        if (bridge_output_filter_is_busy() == 0U)
        {
            vision_ipc_core1_fill_bridge_v2(&packet);
        }
    }
    if ((g_core1_bumpy_enabled != 0U) && (g_bumpy_vision_output_write_busy == 0U))
    {
        vision_ipc_core1_fill_bumpy(&packet, bumpy_vision_get_output());
    }

    /* 根据当前的活跃目标，将相应模块的数据提取到包的通用字段中供 0 核直接使用 */
    if (packet.active_target == VISION_TARGET_BRIDGE)
    {
        /* 桥梁模式下, 用新管线仲裁输出 (b2_valid) 填通用字段 */
        packet.stable_detected = packet.b2_valid;
        packet.detected = packet.b2_valid ? 1U : 0U;
        packet.raw_detected = packet.b2_valid;
        packet.confidence_u16 = packet.b2_valid ? 1000U : 0U;

        if (packet.stable_detected)
        {
            packet.stable_target = VISION_TARGET_BRIDGE;
        }
    }
    else if (packet.active_target == VISION_TARGET_BUMPY)
    {
        packet.stable_detected = packet.bumpy_detected;
        packet.detected = packet.bumpy_detected;
        packet.raw_detected = packet.bumpy_detected;
        packet.confidence_u16 = packet.bumpy_detected ? 1000U : 0U;
        packet.forward_mm = 0;
        packet.lateral_mm = 0;
        packet.yaw_error_deg_x100 = 0;

        if (packet.bumpy_detected)
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
