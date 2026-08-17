/*
 * 文件: vision_ipc.h
 * 作用: 定义 0 核与 1 核之间视觉共享内存 IPC 协议。
 * 内容: 地址、魔数、版本号、命令包/结果包结构体及 CRC 校验工具函数。
 */
#ifndef VISION_IPC_H
#define VISION_IPC_H

#include <stddef.h>
#include "zf_common_headfile.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VISION_IPC_COMMAND_ADDR      (0x28001200U)
#define VISION_IPC_RESULT_ADDR       (0x28001400U)

#define VISION_IPC_CMD_MAGIC         (0x5631U)
#define VISION_IPC_RESULT_MAGIC      (0x5652U)
#define VISION_IPC_VERSION           (1U)

typedef enum
{
    VISION_TARGET_NONE = 0,
    VISION_TARGET_PVC_ENTRY = 1,
    VISION_TARGET_BRIDGE = 2,
    VISION_TARGET_BUMPY = 3,
    VISION_TARGET_STAIR_UP = 4,
    VISION_TARGET_STAIR_DOWN = 5,
    VISION_TARGET_GRASS = 6,
} vision_target_e;

#define VISION_TARGET_MASK(_target)     ((uint16)(1U << (uint16)(_target)))
#define VISION_MASK_PVC_ENTRY           VISION_TARGET_MASK(VISION_TARGET_PVC_ENTRY)
#define VISION_MASK_BRIDGE              VISION_TARGET_MASK(VISION_TARGET_BRIDGE)
#define VISION_MASK_BUMPY               VISION_TARGET_MASK(VISION_TARGET_BUMPY)
#define VISION_MASK_STAIR_UP            VISION_TARGET_MASK(VISION_TARGET_STAIR_UP)
#define VISION_MASK_STAIR_DOWN          VISION_TARGET_MASK(VISION_TARGET_STAIR_DOWN)
#define VISION_MASK_GRASS               VISION_TARGET_MASK(VISION_TARGET_GRASS)

#define VISION_VALID_COMMON             (1U << 0)
#define VISION_VALID_PVC                (1U << 1)
#define VISION_VALID_BRIDGE             (1U << 2)
#define VISION_VALID_BUMPY              (1U << 3)
#define VISION_VALID_STAIR              (1U << 4)
#define VISION_VALID_GRASS              (1U << 5)
#define VISION_VALID_PROFILE            (1U << 6)
#define VISION_VALID_BRIDGE_V2          (1U << 7)   /* 新单边桥管线(bridge_detect)仲裁+中值滤波输出 */

typedef struct
{
    uint16 magic;
    uint16 size;
    uint8 version;
    uint8 active_target;
    uint16 enable_mask;
    uint32 seq;
    uint32 flags;

    uint16 pvc_min_score_u16;
    uint16 reserved0;

    /* Reserved for later: ROI, mode flags, per-target enable windows. */
    int16 roi_xmin;
    int16 roi_ymin;
    int16 roi_xmax;
    int16 roi_ymax;

    uint32 reserved1;
    uint16 crc;
} vision_ipc_command_t;

typedef struct
{
    uint16 magic;
    uint16 size;
    uint8 version;
    uint8 active_target;
    uint8 stable_target;
    uint8 stable_detected;
    uint16 valid_mask;
    uint32 seq;
    uint32 frame_id;
    uint32 command_seq_echo;

    uint16 frame_dt_us;
    uint16 cost_us;

    /* Generic control-facing result. Unused fields are zero. */
    uint8 detected;
    uint8 raw_detected;
    uint16 confidence_u16;
    int16 forward_mm;
    int16 lateral_mm;
    int16 yaw_error_deg_x100;

    /* PVC entrance result. */
    uint8 pvc_detected;
    uint8 pvc_stable_detected;
    uint16 pvc_confidence_u16;
    int16 pvc_target_x_px_x100;
    int16 pvc_steer_error_px_x100;
    int16 pvc_forward_mm;
    int16 pvc_lateral_mm;
    int16 pvc_phy_x_mm;
    int16 pvc_phy_y_mm;
    int16 pvc_yaw_error_deg_x100;
    uint8 pvc_entry_bottom_y;
    uint8 pvc_entry_top_y;
    uint8 pvc_bbox_xmin;
    uint8 pvc_bbox_ymin;
    uint8 pvc_bbox_xmax;
    uint8 pvc_bbox_ymax;
    uint16 pvc_area;
    uint8 pvc_component_count;
    uint8 pvc_candidate_count;

    /* Bumpy road start */
    uint8 bumpy_detected;
    float bumpy_direction_x;
    float bumpy_direction_y;
    /* Bumpy road end */

    /* Reserved: stairs. */
    uint8 stair_detected;
    uint8 stair_stage;
    uint8 stair_jump_count;
    uint8 stair_exit_seen;
    int16 stair_next_jump_mm;
    int16 stair_jump1_mm;
    int16 stair_jump2_mm;
    int16 stair_jump3_mm;

    /* Reserved: grass. */
    uint8 grass_start_seen;
    uint8 grass_end_seen;
    uint16 reserved0;

    /* bridge V2 begin (C01: 新单边桥管线 bridge_detect 仲裁输出, 设计文档 §4.3;
   2026-08-14 起数据源为 bridge_output_filter 中值滤波层, valid/source/mode/gate 直通, 线系数为窗内中值, has_top 经多帧门控) */
    /* 控制线 x=a*y+b, 图像坐标 94x60; 定点 a×1000 b×100; 支撑 u_lo/u_hi 为 y 范围 */
    uint8 b2_valid;              /* 本帧仲裁后控制线原始可信 (0=失能回锁角) */
    uint8 b2_source;             /* 桥上: 0=红蓝中点 1=绿线 2=失能; ref阶段: 3=准备进入 4=准备脱出 (诊断) */
    uint8 b2_mode;               /* 位掩码 (2026-08-14 融合迁移起, 宏见下方 B2M_*): 高4位=检测状态, 低3位=融合阶段 */
    uint8 b2_gate;               /* 底部变白锁存 (融合层 gate_bottom, >75% 单帧锁存) */
    uint8 b2_has_top;            /* 退出线有效 (需 gate=1) */
    uint8 b2_line_u_lo;          /* 控制线支撑 y 下限 */
    uint8 b2_line_u_hi;          /* 控制线支撑 y 上限 */
    int16 b2_line_a_x1000;       /* 控制线斜率 a ×1000 */
    int16 b2_line_b_x100;        /* 控制线截距 b ×100 */
    int16 b2_top_a_x1000;        /* 退出线斜率 a ×1000 (横线 y=a*x+b) */
    int16 b2_top_b_x100;         /* 退出线截距 b ×100 */
    uint16 b2_spacing_x100;      /* 红蓝间距@y=55 ×100 (诊断) */
    uint16 b2_mid_ratio_x1000;   /* 中线底间距比 ×1000 (诊断) */
    /* bridge V2 end */

    uint32 reserved1;
    uint16 crc;
} vision_ipc_packet_t;

/* ---- b2_mode 位掩码 (2026-08-14 远近融合迁移起; 仅语义定义, 不改包布局) ----
 * 高 4 位 = 检测状态 (按当前阶段取对应引擎的检出标志):
 *   v8 桥上阶段: DET_RED/GREEN/BLUE/TOP = v8 红(左界)/绿(中缝)/蓝(右界)/结束线检出
 *   ref 阶段:    DET_RED=左边线有效  DET_GREEN=bridge_found  DET_BLUE=右边线有效  DET_TOP=顶边线(脱出线)检出
 * 低 3 位 = 正在干活的状态机 (融合阶段, 由 gate_bottom/gate_top 导出):
 *   0=准备进入(ref 远处中线)  1=桥上(v8)  2=准备脱出(ref 脱出线)
 * 0核解码: stage = b2_mode & B2M_STAGE_MASK; det = b2_mode & 0xF0            */
#define B2M_STAGE_MASK          0x07U
#define B2M_STAGE_PREPARE_ENTER 0x00U   /* 准备进入 (ref 引擎, 远处中线) */
#define B2M_STAGE_ON_BRIDGE     0x01U   /* 桥上     (v8 引擎)            */
#define B2M_STAGE_PREPARE_EXIT  0x02U   /* 准备脱出 (ref 引擎, 脱出线)   */
#define B2M_DET_TOP             0x10U
#define B2M_DET_BLUE            0x20U
#define B2M_DET_GREEN           0x40U
#define B2M_DET_RED             0x80U

static inline uint16 vision_ipc_checksum16(const void *data, uint16 size_without_crc)
{
    const uint8 *bytes = (const uint8 *)data;
    uint32 sum = 0U;
    for (uint16 i = 0U; i < size_without_crc; i++)
    {
        sum += bytes[i];
    }
    return (uint16)((sum & 0xFFFFU) + (sum >> 16));
}

static inline uint16 vision_ipc_command_crc(const vision_ipc_command_t *cmd)
{
    return vision_ipc_checksum16(cmd, (uint16)offsetof(vision_ipc_command_t, crc));
}

static inline uint16 vision_ipc_packet_crc(const vision_ipc_packet_t *packet)
{
    return vision_ipc_checksum16(packet, (uint16)offsetof(vision_ipc_packet_t, crc));
}

static inline uint8 vision_ipc_command_is_valid(const vision_ipc_command_t *cmd)
{
    return (uint8)((cmd->magic == VISION_IPC_CMD_MAGIC) &&
                   (cmd->version == VISION_IPC_VERSION) &&
                   (cmd->size == sizeof(vision_ipc_command_t)) &&
                   (cmd->crc == vision_ipc_command_crc(cmd)));
}

static inline uint8 vision_ipc_packet_is_valid(const vision_ipc_packet_t *packet)
{
    return (uint8)((packet->magic == VISION_IPC_RESULT_MAGIC) &&
                   (packet->version == VISION_IPC_VERSION) &&
                   (packet->size == sizeof(vision_ipc_packet_t)) &&
                   (packet->crc == vision_ipc_packet_crc(packet)));
}

#ifdef __cplusplus
}
#endif

#endif
