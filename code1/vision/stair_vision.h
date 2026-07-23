/**
 * @file    stair_vision.h
 * @brief   台阶检测视觉模块 — Core 1 算法封装层
 * @details 基于 V9 台阶检测算法 (Gx/Gy 边缘卷积 + 三阶段后处理)。
 *          输入 188×120 uint8 灰度图像，输出台阶几何信息。
 *
 *          【重要】本模块是纯算法层，不涉及跨核 IPC 通信。
 *          跨核数据传输由后续集成者在 vision_ipc_core1.c 中完成。
 *
 *          === 集成指南 (写给后续 IPC 集成者) ===
 *
 *          1. 在 Core 1 的摄像头回调中调用:
 *             stair_vision_process_camera_frame(gray_image_ptr);
 *
 *          2. 在 vision_ipc_core1.c 的 PublishCurrent() 中读取结果:
 *             const volatile stair_vision_output_t *out = stair_vision_get_output();
 *             将 out->result 的字段填入 vision_ipc_packet_t 的 stair 相关字段。
 *
 *          3. stair_vision_output_t.result 字段映射建议:
 *             - has_stairs       → packet.stair_detected
 *             - joint_score      → packet.stair_joint_score (需新增 float 字段)
 *             - upper_mid1_x     → 上峰左半中点 x (y = upper_peak_y)
 *             - upper_mid2_x     → 上峰右半中点 x (y = upper_peak_y)
 *             - edge_span        → packet.stair_edge_span
 *             - crease_y         → packet.stair_crease_y
 *             - crease_span      → packet.stair_crease_span
 *             - upper_peak_y     → packet.stair_upper_peak_y
 *             - lower_peak_y     → packet.stair_lower_peak_y
 *
 *          4. Core 0 控制层读取 IPC 后:
 *             - 用 upper_peak_y / lower_peak_y 判断是否到达跳跃点
 *             - 用 upper_mid1_x / upper_mid2_x 做横向居中修正
 *             - 用 crease_y 做折痕位置参考
 *
 *          === 算法流水线 (内部) ===
 *
 *          摄像头 188×120 uint8 灰度
 *            │
 *            ▼ uint8 → int16 逐行展开
 *            │
 *            ▼ v9_conv_gx_row() 逐行 → Gx 环形缓冲 [119×185]
 *            │
 *            ▼ v9_conv_gy_row() 逐行 → Gy 环形缓冲 [117×185]
 *            │
 *            ▼ v9_stair_process_full() 三阶段后处理:
 *            │   1. stair_discriminate() — 台阶/背景判别
 *            │   2. detect_crease()      — 双峰配对 + 折痕检测
 *            │   3. fit_gy_edges()       — 上峰横线双中点提取
 *            │
 *            ▼ stair_vision_output_t (多帧滤波后发布)
 *
 * @date    2026-07-24
 */

#ifndef STAIR_VISION_H
#define STAIR_VISION_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"
#include "v9_stair_conv_asm.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ==========================================================================
 * 编译开关与配置
 * ========================================================================== */
#define STAIR_VISION_ENABLE                 (1)
#define STAIR_VISION_PROFILE_ENABLE         (1)
#define STAIR_VISION_PROFILE_TIMER          (TC_TIME2_CH2)

/* 图像尺寸: 全尺寸 188×120 */
#define STAIR_IMAGE_W                       (V9_STAIR_IMAGE_W)    /* 188 */
#define STAIR_IMAGE_H                       (V9_STAIR_IMAGE_H)    /* 120 */
#define STAIR_IMAGE_SIZE                    (STAIR_IMAGE_W * STAIR_IMAGE_H)

/* 卷积输出尺寸 */
#define STAIR_GX_OUT_W                      (V9_STAIR_GX_OUT_COLS) /* 185 */
#define STAIR_GY_OUT_ROWS                   (V9_STAIR_GY_OUT_ROWS) /* 117 */
#define STAIR_GY_OUT_COLS                   (V9_STAIR_GY_OUT_COLS) /* 185 */

/* 环形缓冲深度 (Gx 用 2, Gy 用 4) */
#define STAIR_GX_RING_DEPTH                 (2U)
#define STAIR_GY_RING_DEPTH                 (4U)


/* ==========================================================================
 * 对外输出结构体
 *
 * result 字段说明 (写给 IPC 集成者):
 *   has_stairs:      1=检测到台阶, 0=未检测到
 *   joint_score:     台阶/背景判别分数, 仅供参考
 *   upper_mid1_x:    上峰左半横线中点 x 坐标 (0~184)
 *   upper_mid2_x:    上峰右半横线中点 x 坐标 (0~184)
 *                    (两个中点的 y 坐标 = upper_peak_y, 不重复存储)
 *   edge_span:       上峰横线水平跨度 (px)
 *   num_edge_points: 参与提取的有效 run 数 (≥2 表示双中点有效)
 *   crease_y:        折痕行号 (0=图像顶部, -1=无效)
 *   crease_span:     双峰间距 (px)
 *   upper_peak_y:    上峰行号 (小的 y, 远离机器人, -1=无效)
 *   lower_peak_y:    下峰行号 (大的 y, 靠近机器人, -1=无效)
 * ========================================================================== */
typedef struct
{
    uint32 frame_id;            /* 帧号, 每处理一帧加一 */
    uint8  detected;            /* 稳定检测到台阶 (经过连续帧滤波) */
    uint8  raw_detected;        /* 当前帧原始检测结果 */
    uint8  detected_streak;     /* 连续检测到台阶的帧数 */
    uint8  lost_streak;         /* 连续丢失台阶的帧数 */
    v9_stair_result_t result;   /* 当前帧台阶几何信息 (稳定滤波后) */
} stair_vision_output_t;


/* ==========================================================================
 * 全局状态 (供 IPC 层读取)
 * ========================================================================== */
extern volatile runtime_profiler_t g_stair_vision_cost_profiler;
extern volatile runtime_profiler_t g_stair_vision_frame_profiler;
extern volatile stair_vision_output_t g_stair_vision_output;
extern volatile uint8 g_stair_vision_output_write_busy;


/* ==========================================================================
 * 对外 API
 * ========================================================================== */

/**
 * @brief 初始化台阶检测模块
 * @note  系统启动时调用一次, 清零所有内部状态和环形缓冲
 */
void stair_vision_init(void);

/**
 * @brief 重置多帧滤波器状态
 * @note  切换任务时调用, 清除历史帧的检测状态
 */
void stair_vision_reset_filter(void);

/**
 * @brief 获取当前稳定的检测结果 (只读指针)
 * @return 指向全局输出结构体的只读指针, 调用者应在 write_busy==0 时读取
 */
const volatile stair_vision_output_t *stair_vision_get_output(void);

/**
 * @brief 处理一帧摄像头图像, 执行完整台阶检测流水线
 * @param gray  输入灰度图像 (uint8[120][188], 行优先)
 * @note  耗时约 15~20ms @250MHz (含 Gx/Gy 卷积 + 后处理)
 *        调用者应在 write_busy==0 时写入, 写入期间置 write_busy=1
 */
void stair_vision_process_camera_frame(const uint8 *gray);


#ifdef __cplusplus
}
#endif

#endif /* STAIR_VISION_H */
