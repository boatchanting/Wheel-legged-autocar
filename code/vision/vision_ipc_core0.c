/*
 * =================================================================================
 * 文件: vision_ipc_core0.c
 * 作用: 0 核 (Core 0) 进程间通信 (IPC) 驱动的具体实现。
 * 说明: 这里是 0 核与 1 核通信的“发报机”和“收报机”。
 *       发报：把控制层的命令打包后写入共享内存。
 *       收报：通过 IPC Notify 中断接收 1 核的新结果，并原子更新本地最新快照。
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

/* 0 核自己用的“最新战报”，其他模块都来读这个变量 */
volatile vision_ipc_packet_t g_vision_ipc_latest = {0};

/* --- 内部静态变量 --- */
static vision_ipc_command_t g_core0_command_shadow;
static uint32 g_core0_last_result_seq = 0U;
static volatile uint8 g_core0_command_dirty = 0U;
static volatile uint8 g_core0_result_pending = 0U;

static void vision_ipc_core0_ipc_isr(void);

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

static void vision_ipc_core0_init_rx(void)
{
    cy_stc_sysint_irq_t irq_cfg;
    volatile stc_IPC_INTR_STRUCT_t *intr;

    irq_cfg.sysIntSrc = (cy_en_intr_t)(cpuss_interrupts_ipc_0_IRQn + IPC_VISION_INTR);
    irq_cfg.intIdx = IPC_VISION_INTR_IDX;
    irq_cfg.isEnabled = true;
    (void)Cy_SysInt_InitIRQ(&irq_cfg);
    Cy_SysInt_SetSystemIrqVector(irq_cfg.sysIntSrc, vision_ipc_core0_ipc_isr);

    intr = Cy_IPC_Drv_GetIntrBaseAddr(IPC_VISION_INTR);
    Cy_IPC_Drv_SetInterruptMask(intr, 0U, (uint32_t)(1UL << IPC_VISION_CHAN));

    NVIC_ClearPendingIRQ(IPC_VISION_INTR_IDX);
    NVIC_SetPriority(IPC_VISION_INTR_IDX, IPC_VISION_INTR_PRIORITY);
    NVIC_EnableIRQ(IPC_VISION_INTR_IDX);
}

/**
 * @brief 把草稿纸上的新命令真正发送出去
 * @note  打上包头、算好校验码，然后写进共享内存。
 */
static void vision_ipc_core0_flush_command(void)
{
    g_core0_command_shadow.magic = VISION_IPC_CMD_MAGIC;
    g_core0_command_shadow.version = VISION_IPC_VERSION;
    g_core0_command_shadow.size = (uint16)sizeof(vision_ipc_command_t);

    g_core0_command_shadow.crc = 0U;
    g_core0_command_shadow.crc = vision_ipc_command_crc(&g_core0_command_shadow);

    g_vision_ipc_shared.command = g_core0_command_shadow;
}

static void vision_ipc_core0_ipc_isr(void)
{
    const uint32 notify_mask = (uint32)(1UL << IPC_VISION_CHAN);
    const uint32 notify_status_mask = (notify_mask << 16U);
    volatile stc_IPC_INTR_STRUCT_t *intr = Cy_IPC_Drv_GetIntrBaseAddr(IPC_VISION_INTR);
    volatile stc_IPC_STRUCT_t *ipc = Cy_IPC_Drv_GetIpcBaseAddress(IPC_VISION_CHAN);
    vision_ipc_packet_t packet;
    uint32 msg_word = 0U;

    if ((Cy_IPC_Drv_GetInterruptStatusMasked(intr) & notify_status_mask) == 0U)
    {
        return;
    }

    Cy_IPC_Drv_ClearInterrupt(intr, 0U, notify_mask);
    (void)Cy_IPC_Drv_GetInterruptStatus(intr);

    if (Cy_IPC_Drv_ReadMsgWord(ipc, &msg_word) != CY_IPC_DRV_SUCCESS)
    {
        return;
    }

    packet = g_vision_ipc_shared.result;

    if ((vision_ipc_packet_is_valid(&packet) != 0U) &&
        (packet.seq != g_core0_last_result_seq))
    {
        uint32 primask = __get_PRIMASK();

        g_core0_last_result_seq = packet.seq;

        __disable_irq();
        g_vision_ipc_latest = packet;
        g_core0_result_pending = 1U;
        if (primask == 0U)
        {
            __enable_irq();
        }
    }

    (void)msg_word;
    (void)Cy_IPC_Drv_LockRelease(ipc, IPC_VISION_INTR_MASK);
}

void VisionIpc_Core0_Init(void)
{
    memset(&g_core0_command_shadow, 0, sizeof(g_core0_command_shadow));
    memset((void *)&g_vision_ipc_latest, 0, sizeof(g_vision_ipc_latest));
    g_core0_last_result_seq = 0U;
    g_core0_result_pending = 0U;

    vision_ipc_core0_init_mpu();
    vision_ipc_core0_init_rx();

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

void VisionIpc_Core0_SetBridgeLineEnable(uint8 enable)
{
    if (enable)
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
    if (enable)
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
    if (g_core0_command_dirty)
    {
        vision_ipc_core0_flush_command();
        g_core0_command_dirty = 0U;
    }
}

uint8 VisionIpc_Core0_PollResult(void)
{
    uint32 primask = __get_PRIMASK();
    uint8 pending;

    __disable_irq();
    pending = g_core0_result_pending;
    g_core0_result_pending = 0U;
    if (primask == 0U)
    {
        __enable_irq();
    }

    return pending;
}

const volatile vision_ipc_packet_t *VisionIpc_Core0_GetLatest(void)
{
    return &g_vision_ipc_latest;
}
