/*
 * =================================================================================
 * 文件: vision_ipc_core0.c
 * 作用: 0 核 (Core 0) 视觉双核通信实现。
 * 说明: 0 核负责下发视觉任务命令，并通过 EVTGEN 中断接收 1 核发布的视觉结果。
 *       共享区使用 MPU Non-Cacheable，结果 ISR 只做清中断 + shadow 拷贝 + 置新标志。
 * =================================================================================
 */
#include "vision/vision_ipc_core0.h"
#include "vision/vision_ipc_shared.h"
#include "mpu/cy_mpu.h"
#include "sysint/cy_sysint.h"

#if defined(__ICCARM__)
#pragma data_alignment = 32
#pragma location = ".global_ram_data"
__no_init volatile vision_ipc_shared_layout_t g_vision_ipc_shared;
#else
volatile vision_ipc_shared_layout_t g_vision_ipc_shared;
#endif

volatile vision_ipc_packet_t g_vision_ipc_latest = {0};

static vision_ipc_command_t g_core0_command_shadow;
static volatile uint8 g_vision_result_new = 0U;
static uint32 g_core0_last_result_seq = 0U;
static uint32 g_core0_last_result_timestamp = 0U;
static uint32 g_core0_command_timestamp = 0U;

static void vision_ipc_core0_evtgen_isr(void);

static void vision_ipc_core0_init_mpu(void)
{
    cy_stc_mpu_region_cfg_t region_cfg;
    cy_stc_mpu_global_ctrl_bits_t ctrl_bits;

    region_cfg.addr = SHARED_PAYLOAD_BASE_ADDR;
    region_cfg.size = CY_MPU_SIZE_2KB;
    region_cfg.permission = CY_MPU_ACCESS_P_FULL_ACCESS;
    region_cfg.attribute = CY_MPU_ATTR_NORM_SHR_MEM_NC;
    region_cfg.execute = CY_MPU_INST_ACCESS_DIS;
    region_cfg.srd = 0x00U;
    region_cfg.enable = CY_MPU_ENABLE;
    (void)Cy_MPU_SetRegion(&region_cfg, SHARED_PAYLOAD_MPU_REGION);

    Cy_MPU_GetGlobalControlBits(&ctrl_bits);
    if (ctrl_bits.mpuGlobalEnable == CY_MPU_GLOBAL_DISABLE)
    {
        ctrl_bits.mpuGlobalEnable = CY_MPU_GLOBAL_ENABLE;
        ctrl_bits.privDefMapEn = CY_MPU_USE_DEFAULT_MAP_AS_BG;
        ctrl_bits.faultNmiEn = CY_MPU_ENABLED_DURING_FAULT_NMI;
        Cy_MPU_SetGlobalControlBits(&ctrl_bits);
    }

    __DSB();
    __ISB();
}

static void vision_ipc_core0_init_evtgen_common(void)
{
    EVTGEN0->unCTL.u32Register |= EVTGEN_CTL_ENABLED_Msk;
    EVTGEN0->unINTR_MASK.u32Register |= VISION_EVTGEN_USED_MASK;
    EVTGEN0->unINTR.u32Register = VISION_EVTGEN_USED_MASK;
    (void)EVTGEN0->unINTR.u32Register;
}

static void vision_ipc_core0_init_evtgen_rx(void)
{
    cy_stc_sysint_irq_t irq_cfg;

    irq_cfg.sysIntSrc = (cy_en_intr_t)VISION_EVTGEN_SYS_INT_SRC;
    irq_cfg.intIdx = VISION_EVTGEN_CPU_INT_IDX;
    irq_cfg.isEnabled = true;
    (void)Cy_SysInt_InitIRQ(&irq_cfg);
    Cy_SysInt_SetSystemIrqVector(irq_cfg.sysIntSrc, vision_ipc_core0_evtgen_isr);

    NVIC_ClearPendingIRQ(VISION_EVTGEN_CPU_INT_IDX);
    NVIC_SetPriority(VISION_EVTGEN_CPU_INT_IDX, VISION_EVTGEN_PRIORITY);
    NVIC_EnableIRQ(VISION_EVTGEN_CPU_INT_IDX);
}

static void vision_ipc_core0_publish_command(uint8 notify_peer)
{
    volatile vision_ipc_command_channel_t *channel = &g_vision_ipc_shared.command_channel;

    g_core0_command_shadow.magic = VISION_IPC_CMD_MAGIC;
    g_core0_command_shadow.version = VISION_IPC_VERSION;
    g_core0_command_shadow.size = (uint16)sizeof(vision_ipc_command_t);
    g_core0_command_shadow.crc = 0U;
    g_core0_command_shadow.crc = vision_ipc_command_crc(&g_core0_command_shadow);

    channel->payload = g_core0_command_shadow;
    channel->frame_seq = g_core0_command_shadow.seq;
    channel->publish_us = 0U;
    channel->flags = VISION_CHANNEL_FLAG_VALID;
    channel->timestamp = ++g_core0_command_timestamp;

    if (notify_peer != 0U)
    {
        EVTGEN0->unINTR_SET.u32Register = VISION_EVTGEN_COMMAND_MASK;
    }
}

static void vision_ipc_core0_evtgen_isr(void)
{
    const uint32 masked = EVTGEN0->unINTR_MASKED.u32Register;
    vision_ipc_packet_t packet;

    if ((masked & VISION_EVTGEN_RESULT_MASK) == 0U)
    {
        return;
    }

    EVTGEN0->unINTR.u32Register = VISION_EVTGEN_RESULT_MASK;
    (void)EVTGEN0->unINTR.u32Register;

    if ((g_vision_ipc_shared.result_channel.flags & VISION_CHANNEL_FLAG_VALID) == 0U)
    {
        return;
    }
    if (g_vision_ipc_shared.result_channel.timestamp == g_core0_last_result_timestamp)
    {
        return;
    }

    memcpy(&packet,
           (const void *)&g_vision_ipc_shared.result_channel.payload,
           sizeof(packet));

    if (vision_ipc_packet_is_valid(&packet) == 0U)
    {
        return;
    }
    if (g_vision_ipc_shared.result_channel.frame_seq != packet.seq)
    {
        return;
    }
    if (packet.seq == g_core0_last_result_seq)
    {
        return;
    }

    g_core0_last_result_seq = packet.seq;
    g_core0_last_result_timestamp = g_vision_ipc_shared.result_channel.timestamp;

    memcpy((void *)&g_vision_ipc_latest, (const void *)&packet, sizeof(packet));
    g_vision_result_new = 1U;
}

void VisionIpc_Core0_Init(void)
{
    memset(&g_core0_command_shadow, 0, sizeof(g_core0_command_shadow));
    memset((void *)&g_vision_ipc_latest, 0, sizeof(g_vision_ipc_latest));
    g_vision_result_new = 0U;
    g_core0_last_result_seq = 0U;
    g_core0_last_result_timestamp = 0U;
    g_core0_command_timestamp = 0U;

    vision_ipc_core0_init_mpu();
    vision_ipc_core0_init_evtgen_common();
    vision_ipc_core0_init_evtgen_rx();

    g_core0_command_shadow.active_target = VISION_TARGET_NONE;
    g_core0_command_shadow.enable_mask = 0U;
    g_core0_command_shadow.pvc_min_score_u16 = 580U;
    g_core0_command_shadow.seq = 1U;

    vision_ipc_core0_publish_command(0U);
}

void VisionIpc_Core0_SetTask(uint8 active_target, uint16 enable_mask)
{
    g_core0_command_shadow.active_target = active_target;
    g_core0_command_shadow.enable_mask = enable_mask;
    g_core0_command_shadow.seq++;

    vision_ipc_core0_publish_command(1U);
}

void VisionIpc_Core0_SetPvcEnable(uint8 enable)
{
    if (enable != 0U)
    {
        VisionIpc_Core0_SetTask(VISION_TARGET_PVC_ENTRY, VISION_MASK_PVC_ENTRY);
    }
    else
    {
        VisionIpc_Core0_SetTask(VISION_TARGET_NONE, 0U);
    }
}

void VisionIpc_Core0_SetBridgeLineEnable(uint8 enable)
{
    if (enable != 0U)
    {
        VisionIpc_Core0_SetTask(VISION_TARGET_BRIDGE, VISION_MASK_BRIDGE);
    }
    else
    {
        VisionIpc_Core0_SetTask(VISION_TARGET_PVC_ENTRY, VISION_MASK_PVC_ENTRY);
    }
}

void VisionIpc_Core0_SetBumpyEnable(uint8 enable)
{
    if (enable != 0U)
    {
        VisionIpc_Core0_SetTask(VISION_TARGET_BUMPY, VISION_MASK_BUMPY);
    }
    else
    {
        VisionIpc_Core0_SetTask(VISION_TARGET_NONE, 0U);
    }
}

void VisionIpc_Core0_Update_2ms(void)
{
}

uint8 VisionIpc_Core0_PollResult(void)
{
    uint8 pending = g_vision_result_new;
    g_vision_result_new = 0U;
    return pending;
}

const volatile vision_ipc_packet_t *VisionIpc_Core0_GetLatest(void)
{
    return &g_vision_ipc_latest;
}
