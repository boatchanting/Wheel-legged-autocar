/*
 * =================================================================================
 * 文件: vision_ipc_core1.c
 * 作用: 1 核 (Core 1) 进程间通信 (IPC) 驱动的核心实现。
 * 说明: 在多核处理器中，不同核心需要相互通信。这个文件负责 1 核这边的通信逻辑：
 *       接收来自 0 核的同步命令，调度各个视觉模块（如 PVC 和直线检测），
 *       最后将检测结果打包发布回共享内存，供 0 核读取。
 * =================================================================================
 */
#include "vision_ipc_core1.h"

/* --- 1. 全局变量与内存对齐定义部分 --- */
/* 这里定义了两个核心之间共享内存的地址。使用特定的对齐方式是为了确保内存读写的高效和安全。 */

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

/* --- 2. 内部静态变量定义部分 --- */
/* 这些变量只在当前文件中使用，用于记录上一帧的状态、请求等，防止其他文件误修改 */
static vision_ipc_command_t g_core1_command_shadow;         /* 缓存的 0 核命令，用于比对是否有新命令 */
static uint32 g_core1_result_seq = 0U;                      /* 发送给 0 核的结果序号，每次发送加 1 */
static volatile uint8 g_core1_pvc_enabled = 0U;             /* 标记当前是否开启了 PVC（入口）检测 */
static volatile uint8 g_core1_line_enabled = 0U;            /* 标记当前是否开启了直线（桥）检测 */
static volatile uint8 g_core1_pvc_reset_request = 0U;       /* 标记是否需要重置 PVC 检测状态 */
static volatile uint8 g_core1_line_reset_request = 0U;      /* 标记是否需要重置直线检测状态 */
static uint32 g_core1_last_published_pvc_frame_id = 0U;     /* 上次发布的 PVC 数据帧 ID，用来判断是否有新数据 */
static uint32 g_core1_last_published_line_frame_id = 0U;    /* 上次发布的直线数据帧 ID */
static uint32 g_core1_last_published_command_seq = 0U;      /* 上次响应的命令序号 */
static uint16 g_core1_last_published_enable_mask = 0U;      /* 上次发布时启用的掩码，用于状态变化检测 */

/* --- 3. 基础工具函数部分 --- */

/**
 * @brief 将浮点型的置信度 (0.0 ~ 1.0) 转换为整数 (0 ~ 1000)
 * 
 * @param confidence 浮点型的置信度，表示对识别结果的确信程度
 * @return uint16 转换后的整数置信度
 * 
 * @note 为什么要转换？因为在 IPC（进程间通信）中，传输整数通常比浮点数更稳定、占用带宽更小。
 */
static uint16 vision_confidence_to_u16(float confidence)
{
    /* 如果置信度小于等于 0，直接返回 0，防止负数异常 */
    if (confidence <= 0.0f)
    {
        return 0U;
    }
    /* 如果置信度大于等于 1，说明非常确信，返回最大值 1000 */
    if (confidence >= 1.0f)
    {
        return 1000U;
    }
    /* 正常范围内，乘以 1000 并转为整数，比如 0.85 变成 850 */
    return (uint16)(confidence * 1000.0f);
}

/**
 * @brief 将浮点数放大 100 倍并转换为 16 位有符号整数
 * 
 * @param value 需要转换的浮点数（例如角度、偏差等）
 * @return int16 放大 100 倍后的整数，保留了两位小数的精度
 * 
 * @note 这样做可以避免浮点数传输，同时保留足够的小数精度（如 1.23 -> 123）
 */
static int16 vision_float_to_i16_x100(float value)
{
    /* 防止放大后超过 int16 的最大值 (32767) 导致溢出 */
    if (value > 327.67f)
    {
        return 32767;
    }
    /* 防止放大后小于 int16 的最小值 (-32768) 导致溢出 */
    if (value < -327.68f)
    {
        return -32768;
    }
    return (int16)(value * 100.0f);
}

/**
 * @brief 比较两个无符号 32 位整数，返回较大的那个
 * 
 * @param a 数字 a
 * @param b 数字 b
 * @return uint32 较大的数字
 */
static uint32 vision_max_u32(uint32 a, uint32 b)
{
    return (a > b) ? a : b;
}

/* --- 4. 核心通信与数据打包函数 --- */

/**
 * @brief 将视觉结果数据包写入共享内存，供 0 核读取
 * 
 * @param packet 包含最新视觉处理结果的数据包指针
 * 
 * @note 此函数会将包头、版本、校验码等元数据补齐，并刷新数据缓存(DCache)
 *       确保 0 核能够读到最新的物理内存数据。
 */
static void vision_ipc_core1_write_packet(vision_ipc_packet_t *packet)
{
    /* 设置固定的魔数，就像是对暗号，0 核检查这个就知道数据没乱 */
    packet->magic = VISION_IPC_RESULT_MAGIC;
    packet->version = VISION_IPC_VERSION; /* 版本号同步 */
    packet->size = (uint16)sizeof(vision_ipc_packet_t); /* 记录数据包的字节大小 */
    packet->seq = ++g_core1_result_seq; /* 结果序号，每次发送加 1，告诉 0 核这是新数据 */
    packet->command_seq_echo = g_core1_command_shadow.seq; /* 回显 0 核的命令序号，表示 "你的命令我收到了" */
    
    /* 计算并填入 CRC 校验码，防止数据在传输中损坏 */
    packet->crc = 0U;
    packet->crc = vision_ipc_packet_crc(packet);

    /* 将数据包写入共享内存变量 */
    g_vision_ipc_result = *packet;
    /* 刷新数据缓存，强制将 Cache 中的数据写回物理内存，确保 0 核能马上看到 */
    SCB_CleanInvalidateDCache_by_Addr((void *)&g_vision_ipc_result, sizeof(g_vision_ipc_result));
}

/**
 * @brief 判断当前的 0 核命令是否要求开启 PVC (入口) 检测
 * 
 * @param cmd 指向当前命令的指针
 * @return uint8 如果需要开启返回 1，否则返回 0
 */
static uint8 vision_ipc_core1_command_wants_pvc(const vision_ipc_command_t *cmd)
{
    /* 如果当前活跃目标是 PVC，或者掩码中包含 PVC 的标志位，都说明需要开启 */
    const uint8 active_is_pvc = (uint8)(cmd->active_target == VISION_TARGET_PVC_ENTRY);
    const uint8 mask_has_pvc = (uint8)((cmd->enable_mask & VISION_MASK_PVC_ENTRY) != 0U);
    return (uint8)(active_is_pvc || mask_has_pvc);
}

/**
 * @brief 判断当前的 0 核命令是否要求开启 直线(桥) 检测
 * 
 * @param cmd 指向当前命令的指针
 * @return uint8 如果需要开启返回 1，否则返回 0
 */
static uint8 vision_ipc_core1_command_wants_line(const vision_ipc_command_t *cmd)
{
    /* 同样地，如果活跃目标是桥梁，或者掩码中包含桥梁的标志位，则说明需要开启 */
    const uint8 active_is_bridge = (uint8)(cmd->active_target == VISION_TARGET_BRIDGE);
    const uint8 mask_has_bridge = (uint8)((cmd->enable_mask & VISION_MASK_BRIDGE) != 0U);
    return (uint8)(active_is_bridge || mask_has_bridge);
}

/**
 * @brief 将 PVC 的视觉检测结果提取并填充到 IPC 数据包中
 * 
 * @param packet 将要发送给 0 核的数据包
 * @param pvc_output 当前最新的 PVC 检测输出结果
 */
static void vision_ipc_core1_fill_pvc(vision_ipc_packet_t *packet,
                                      const volatile pvc_vision_output_t *pvc_output)
{
    pvc_vision_output_t pvc;
    const pvc_vision_frame_result_t *ctrl;

    /* 如果输出为空或者还没有处理出有效帧 (frame_id 为 0)，直接返回，不填充 */
    if ((pvc_output == NULL) || (pvc_output->frame_id == 0U))
    {
        return;
    }

    /* 拷贝当前结果，防止在中途被其他中断修改 */
    pvc = *pvc_output;
    /* 
     * 判断选用哪个结果：
     * 如果检测稳定 (stable_detected)，就用稳定结果；否则用原始结果 (raw) 
     */
    ctrl = pvc.stable_detected ? &pvc.stable : &pvc.raw;

    /* 更新数据包中的有效掩码，标记里面包含 PVC 数据和性能分析数据 */
    packet->valid_mask = (uint16)(packet->valid_mask | VISION_VALID_PVC | VISION_VALID_PROFILE);
    /* 记录当前最大的帧 ID */
    packet->frame_id = vision_max_u32(packet->frame_id, pvc.frame_id);
    /* 记录上一帧的用时（微秒），用于性能分析 */
    packet->frame_dt_us = (uint16)g_pvc_vision_frame_profiler.last_us;
    packet->cost_us = (uint16)g_pvc_vision_cost_profiler.last_us;

    /* 将具体的 PVC 参数如实填充进数据包，例如前向距离、横向偏差、包围框等 */
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

/**
 * @brief 将 直线(桥梁) 的视觉检测结果提取并填充到 IPC 数据包中
 * 
 * @param packet 将要发送给 0 核的数据包
 * @param line_output 当前最新的直线检测输出结果
 */
static void vision_ipc_core1_fill_line(vision_ipc_packet_t *packet,
                                       const volatile line_vision_output_t *line_output)
{
    line_vision_output_t line;
    const line_vision_frame_result_t *ctrl;

    /* 如果没有有效数据，直接退出 */
    if ((line_output == NULL) || (line_output->frame_id == 0U))
    {
        return;
    }

    /* 拷贝数据，防止并发修改 */
    line = *line_output;
    /* 
     * 如果直线或者桥梁的检测是稳定的，就使用稳定结果 (stable)；
     * 否则使用单帧的原始结果 (raw)
     */
    ctrl = (line.stable_detected || line.bridge_stable_detected) ? &line.stable : &line.raw;

    /* 标记数据包中包含了直线/桥梁数据 */
    packet->valid_mask = (uint16)(packet->valid_mask | VISION_VALID_BRIDGE | VISION_VALID_PROFILE);
    packet->frame_id = vision_max_u32(packet->frame_id, line.frame_id);
    packet->frame_dt_us = (uint16)g_line_vision_frame_profiler.last_us;
    packet->cost_us = (uint16)g_line_vision_cost_profiler.last_us;

    /* 填充直线的具体信息：检测状态、置信度、横偏、航向角误差等 */
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

    /* 填充桥梁的具体信息：桥梁是否检测到、置信度、包围框等 */
    packet->line_bridge_detected = line.bridge_raw_detected;
    packet->line_bridge_stable_detected = line.bridge_stable_detected;
    packet->line_bridge_confidence_u16 = vision_confidence_to_u16(ctrl->bridge_confidence);
    packet->line_bridge_component_count = ctrl->bridge_component_count;
    packet->line_bridge_bbox_xmin = ctrl->bridge_bbox_xmin;
    packet->line_bridge_bbox_ymin = ctrl->bridge_bbox_ymin;
    packet->line_bridge_bbox_xmax = ctrl->bridge_bbox_xmax;
    packet->line_bridge_bbox_ymax = ctrl->bridge_bbox_ymax;

    /* 为 0 核控制层整理桥梁的最终结果 */
    packet->bridge_detected = line.bridge_stable_detected;
    packet->bridge_count = ctrl->bridge_component_count;
    packet->bridge_side = 0; /* 默认未定 */
    packet->bridge_exit_seen = 0U;
    packet->bridge_center_err = packet->line_lateral_px_x100;
}

/* --- 5. 外部调用的接口函数 --- */

/**
 * @brief 1 核 IPC 初始化函数
 * 
 * @note 在系统启动时被调用。它会将所有相关的状态变量清零，并发布一次“空闲”状态的数据包，
 *       让 0 核知道 1 核已经准备就绪。
 */
void VisionIpc_Core1_Init(void)
{
    /* 清空并初始化命令缓存 */
    memset(&g_core1_command_shadow, 0, sizeof(g_core1_command_shadow));
    g_core1_command_shadow.active_target = VISION_TARGET_NONE;
    g_core1_command_shadow.enable_mask = 0U;
    g_core1_command_shadow.pvc_min_score_u16 = 580U;
    
    /* 清零所有状态变量 */
    g_core1_result_seq = 0U;
    g_core1_pvc_enabled = 0U;
    g_core1_line_enabled = 0U;
    g_core1_pvc_reset_request = 0U;
    g_core1_line_reset_request = 0U;
    g_core1_last_published_pvc_frame_id = 0U;
    g_core1_last_published_line_frame_id = 0U;
    g_core1_last_published_command_seq = 0U;
    g_core1_last_published_enable_mask = 0U;
    
    /* 发布一次空闲状态包 */
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
    uint8 should_publish = 0U; /* 标记是否需要向 0 核发布数据 */

    /* 1. 检查并拉取 0 核的最新命令 */
    VisionIpc_Core1_PollCommand();

    /* 2. 如果当前不需要做任何视觉检测 */
    if ((g_core1_pvc_enabled == 0U) && (g_core1_line_enabled == 0U))
    {
        /* 并且上次发布时是开启状态，说明刚刚被关闭，需要发一个空包通知 0 核 */
        if (g_core1_last_published_enable_mask != 0U)
        {
            VisionIpc_Core1_PublishIdle();
            g_core1_last_published_enable_mask = 0U;
        }
        return;
    }

    /* 3. 获取各视觉模块最新产生的数据帧 ID */
    /* 获取 PVC 的帧 ID (前提是没在被写入) */
    if ((g_core1_pvc_enabled != 0U) && (g_pvc_vision_output_write_busy == 0U))
    {
        pvc_frame_id = pvc_vision_get_output()->frame_id;
    }
    /* 获取直线的帧 ID (前提是没在被写入) */
    if ((g_core1_line_enabled != 0U) && (g_line_vision_output_write_busy == 0U))
    {
        line_frame_id = line_vision_get_output()->frame_id;
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
    /* 如果 0 核发来了新命令（即使视觉数据没更新，也要回显确认收到） */
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

    /* 刷新数据缓存，确保从物理内存读到 0 核最新写入的命令 */
    SCB_CleanInvalidateDCache_by_Addr((void *)&g_vision_ipc_command, sizeof(g_vision_ipc_command));
    cmd = g_vision_ipc_command;

    /* 检查命令是否有效（例如版本匹配、魔数正确） */
    if (vision_ipc_command_is_valid(&cmd))
    {
        /* 如果序号没变，说明是旧命令，直接返回 */
        if (cmd.seq == g_core1_command_shadow.seq)
        {
            return;
        }

        /* 发现新命令，更新缓存 */
        g_core1_command_shadow = cmd;
        
        /* 根据新命令解析需要开启的模块 */
        next_pvc_enabled = vision_ipc_core1_command_wants_pvc(&g_core1_command_shadow);
        next_line_enabled = vision_ipc_core1_command_wants_line(&g_core1_command_shadow);

        /* 如果 PVC 的使能状态发生变化，请求重置 PVC，以防使用历史错误数据 */
        if (next_pvc_enabled != g_core1_pvc_enabled)
        {
            g_core1_pvc_reset_request = 1U;
            g_core1_pvc_enabled = next_pvc_enabled;
            g_core1_last_published_pvc_frame_id = 0U; /* 清零已发布帧号，强制下次发新包 */
        }
        /* 如果直线的使能状态发生变化，请求重置直线检测 */
        if (next_line_enabled != g_core1_line_enabled)
        {
            g_core1_line_reset_request = 1U;
            g_core1_line_enabled = next_line_enabled;
            g_core1_last_published_line_frame_id = 0U;
        }
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

/**
 * @brief 专门发布 PVC 检测结果
 * 
 * @param pvc_output 传入的 PVC 输出数据
 * @note 组装一个只含 PVC 结果的数据包发给 0 核。
 */
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
    if ((g_core1_line_enabled != 0U) && (g_line_vision_output_write_busy == 0U))
    {
        vision_ipc_core1_fill_line(&packet, line_vision_get_output());
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
