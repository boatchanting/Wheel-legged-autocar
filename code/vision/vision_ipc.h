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

    /* Reserved: bridge. */
    uint8 bridge_detected;
    uint8 bridge_count;
    int8 bridge_side;
    uint8 bridge_exit_seen;
    int16 bridge_center_err;

    /* Bridge task straight-line result. */
    uint8 line_detected;
    uint8 line_stable_detected;
    uint16 line_confidence_u16;
    int16 line_lateral_px_x100;
    int16 line_yaw_error_deg_x100;
    int16 line_x_bottom_x100;
    int16 line_x_lookahead_x100;
    uint8 line_points_used;
    uint8 line_y_span;
    uint16 line_rmse_px_x100;
    uint16 line_roi_white_ratio_u16;
    int16 line_speed_hint;

    /* Dark single-side bridge block detected inside the straight-line module. */
    uint8 line_bridge_detected;
    uint8 line_bridge_stable_detected;
    uint16 line_bridge_confidence_u16;
    uint8 line_bridge_component_count;
    uint8 line_bridge_bbox_xmin;
    uint8 line_bridge_bbox_ymin;
    uint8 line_bridge_bbox_xmax;
    uint8 line_bridge_bbox_ymax;

    /* Bumpy road result. */
    uint8 bumpy_detected;
    uint8 bumpy_stable_detected;
    uint16 bumpy_confidence_u16;
    int16 bumpy_steer_error_px_x100;
    int16 bumpy_target_x_px_x100;
    uint8 bumpy_phase;
    uint8 bumpy_mode;
    uint8 bumpy_component_count;
    uint8 bumpy_candidate_count;
    uint8 bumpy_run_count;
    uint8 bumpy_rib_count;
    uint8 bumpy_centerline_rows;
    uint8 bumpy_centerline_bottom_rows;
    uint8 bumpy_centerline_top_y;
    uint8 bumpy_centerline_bottom_y;
    uint8 bumpy_bbox_xmin;
    uint8 bumpy_bbox_ymin;
    uint8 bumpy_bbox_xmax;
    uint8 bumpy_bbox_ymax;
    uint16 bumpy_bbox_area;
    uint16 bumpy_white_threshold_x10;
    uint16 bumpy_dark_threshold_x10;
    uint8 bumpy_start_seen;
    uint8 bumpy_end_seen;
    uint16 bumpy_local_s_mm;

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

    uint32 reserved1;
    uint16 crc;
} vision_ipc_packet_t;

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
