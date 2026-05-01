/*
 * =================================================================================
 * 文件: vision_ipc_core0.c
 * 作用: 0 核 (Core 0) 进程间通信 (IPC) 驱动的具体实现。
 * 说明: 这里是 0 核与 1 核通信的“发报机”和“收报机”。
 *       发报：把控制层的命令打包，算好校验码，塞进共享内存。
 *       收报：去共享内存里看 1 核有没有回复，如果有，就把数据抄出来供 0 核用。
 * =================================================================================
 */
#include "vision/vision_ipc_core0.h"

/* --- 1. 全局变量与内存对齐定义部分 --- */
/* 这里定义了两个核心之间共享内存的物理地址。必须和 1 核那边一模一样。 */

#if defined(__ICCARM__)
#pragma data_alignment = 32
#pragma location = VISION_IPC_COMMAND_ADDR
/* g_vision_ipc_command 用来往里写 0 核的命令，发给 1 核 */
__no_init volatile vision_ipc_command_t g_vision_ipc_command;

#pragma data_alignment = 32
#pragma location = VISION_IPC_RESULT_ADDR
/* g_vision_ipc_result 用来读取 1 核算出来的结果 */
__no_init volatile vision_ipc_packet_t g_vision_ipc_result;
#else
volatile vision_ipc_command_t g_vision_ipc_command;
volatile vision_ipc_packet_t g_vision_ipc_result;
#endif

/* 0 核自己用的“最新战报”，其他模块都来读这个变量 */
volatile vision_ipc_packet_t g_vision_ipc_latest = {0};

/* --- 2. 内部静态变量 --- */
static vision_ipc_command_t g_core0_command_shadow; /* 发报机的草稿纸，写好了再一次性发出去 */
static uint32 g_core0_last_result_seq = 0U;         /* 记下上一次收到的战报编号，防止重复读 */
static volatile uint8 g_core0_command_dirty = 0U;   /* 标记：是不是有新命令还没发出去？(1=有，0=没有) */

/* --- 3. 核心发报/收报函数 --- */

/**
 * @brief 把草稿纸上的新命令真正发送出去
 * @note  打上包头、算好校验码，然后写进共享内存，并强制刷新 Cache。
 */
static void vision_ipc_core0_flush_command(void)
{
    /* 填入暗号（魔数）和版本号，让 1 核知道这不是乱码 */
    g_core0_command_shadow.magic = VISION_IPC_CMD_MAGIC;
    g_core0_command_shadow.version = VISION_IPC_VERSION;
    g_core0_command_shadow.size = (uint16)sizeof(vision_ipc_command_t);
    
    /* 计算并填入 CRC 校验码，防止数据在传输中损坏 */
    g_core0_command_shadow.crc = 0U;
    g_core0_command_shadow.crc = vision_ipc_command_crc(&g_core0_command_shadow);

    /* 真正写入共享内存 */
    g_vision_ipc_command = g_core0_command_shadow;
    /* 刷新数据缓存 (DCache)，把数据从高速缓存逼进物理内存，确保 1 核能看到 */
    SCB_CleanInvalidateDCache_by_Addr((void *)&g_vision_ipc_command, sizeof(g_vision_ipc_command));
}

/**
 * @brief 初始化 0 核的通信机
 * @note  开机时调用，把所有的编号清零，并发送一个默认的空指令。
 */
void VisionIpc_Core0_Init(void)
{
    memset(&g_core0_command_shadow, 0, sizeof(g_core0_command_shadow));
    memset((void *)&g_vision_ipc_latest, 0, sizeof(g_vision_ipc_latest));
    g_core0_last_result_seq = 0U;

    /* 默认不让 1 核做任何事 */
    g_core0_command_shadow.active_target = VISION_TARGET_NONE;
    g_core0_command_shadow.enable_mask = 0U;
    g_core0_command_shadow.pvc_min_score_u16 = 580U;
    g_core0_command_shadow.playgroud_max_lost = 30U;
    g_core0_command_shadow.playgroud_smooth_alpha_u16 = 450U;
    g_core0_command_shadow.playgroud_min_temporal_score_u16 = 200U;
    g_core0_command_shadow.seq = 1U; /* 命令编号从 1 开始 */
    g_core0_command_dirty = 0U;
    
    /* 发送初始命令 */
    vision_ipc_core0_flush_command();
}

/**
 * @brief 设置要让 1 核干什么活
 * 
 * @param active_target 主要目标
 * @param enable_mask 辅助目标
 */
void VisionIpc_Core0_SetTask(uint8 active_target, uint16 enable_mask)
{
    g_core0_command_shadow.active_target = active_target;
    g_core0_command_shadow.enable_mask = enable_mask;
    g_core0_command_shadow.seq++; /* 命令编号加 1，告诉 1 核这是新命令 */
    g_core0_command_dirty = 1U;   /* 标记有新命令待发 */
}

/**
 * @brief 快捷指令：开启或关闭 PVC 检测
 */
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

/**
 * @brief 快捷指令：开启或关闭桥梁/直线检测
 */
void VisionIpc_Core0_SetBridgeLineEnable(uint8 enable)
{
    if (enable)
    {
        VisionIpc_Core0_SetTask(VISION_TARGET_BRIDGE, VISION_MASK_BRIDGE);
    }
    else
    {
        /* 桥梁任务关闭后，默认退回到找 PVC（可以根据实际项目流程修改） */
        VisionIpc_Core0_SetTask(VISION_TARGET_PVC_ENTRY, VISION_MASK_PVC_ENTRY);
    }
}

/**
 * @brief 0 核的通信定时任务（每 2 毫秒调用一次）
 * @note  如果有新命令没发，就发出去。然后去看看有没有新数据收回来。
 */
void VisionIpc_Core0_Update_2ms(void)
{
    /* 如果有新命令被下达了，发送出去 */
    if (g_core0_command_dirty)
    {
        vision_ipc_core0_flush_command();
        g_core0_command_dirty = 0U; /* 标记为已发 */
    }

    /* 去看看 1 核有没有回信 */
    (void)VisionIpc_Core0_PollResult();
}

/**
 * @brief 收报机：去共享内存里读取 1 核的新数据
 * @return 1: 读到了新数据; 0: 没新数据或者数据坏了
 */
uint8 VisionIpc_Core0_PollResult(void)
{
    vision_ipc_packet_t packet;

    /* 刷新缓存，确保读到的是 1 核刚写进物理内存的热乎数据 */
    SCB_CleanInvalidateDCache_by_Addr((void *)&g_vision_ipc_result, sizeof(g_vision_ipc_result));
    packet = g_vision_ipc_result;

    /* 检查包裹上的暗号和校验码对不对 */
    if (vision_ipc_packet_is_valid(&packet) == 0U)
    {
        return 0U; /* 坏包，不要 */
    }
    /* 检查包裹编号是不是和上次一样 */
    if (packet.seq == g_core0_last_result_seq)
    {
        return 0U; /* 旧包，不要 */
    }

    /* 是个全新的好包裹，收下！ */
    g_core0_last_result_seq = packet.seq;
    g_vision_ipc_latest = packet; /* 更新今日战报 */
    return 1U;
}

/**
 * @brief 获取最新的视觉战报
 * @return 战报的指针
 */
const volatile vision_ipc_packet_t *VisionIpc_Core0_GetLatest(void)
{
    return &g_vision_ipc_latest;
}
