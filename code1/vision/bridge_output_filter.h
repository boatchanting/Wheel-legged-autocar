/**
 * ============================================================================
 * bridge_output_filter.h  ——  单边桥仲裁输出中值滤波层 (v11 接入新增, 2026-08-14)
 * ============================================================================
 * 作用: 仲裁层 (bridge_v2_arbiter) 的原始输出必须经过本层滤波后才允许发布
 *       给 0核 控制侧 (IPC b2_* 字段的数据源)。
 *
 * 滤波策略 (定点域, 直接滤波 int16 定点量, 与控制侧消费值完全一致):
 *   控制线 line_a/line_b : W 帧滑动窗中值 (仅纳入 valid 帧);
 *                          窗内有效帧 < MIN_VALID 时输出当前帧原值;
 *                          source 切换时清窗 (红蓝中点线与绿线不跨源求中值)。
 *   结束线 top_a/top_b   : W 帧滑动窗中值 (仅纳入 has_top=1 的帧)。
 *   结束线 has_top 门控  : 连续 CONFIRM_FRAMES 帧检出才置 1;
 *                          确认后允许连续丢失 LOST_TOLERANCE 帧再清 0;
 *                          未确认期间一律输出 0。
 *   valid/source/mode/gate/u_lo/u_hi/spacing/mid_ratio : 直通当前帧。
 *
 * 写忙保护: update() 置 busy, 2ms 发布端读前检查 (与 bridge_v2_arbiter 同模式)。
 * ============================================================================
 */

#ifndef BRIDGE_OUTPUT_FILTER_H
#define BRIDGE_OUTPUT_FILTER_H

#include "zf_common_headfile.h"
#include "bridge_v2_arbiter.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------- 标定参数 (板端验证阶段定标) ---------------- */
#define BRIDGE_FILTER_WINDOW        5U   /* 中值窗口 (奇数)                 */
#define BRIDGE_FILTER_MIN_VALID     3U   /* 窗内最少有效帧才输出中值         */
#define BRIDGE_TOP_CONFIRM_FRAMES   3U   /* 结束线连续确认帧数门控           */
#define BRIDGE_TOP_LOST_TOLERANCE   2U   /* 确认后允许连续丢失帧数           */

/* 清空全部滤波状态 (TakeBridgeResetRequest 时调用) */
void bridge_output_filter_reset(void);

/* 每相机帧调用 (仲裁之后): 推入一帧原始仲裁输出, 刷新滤波结果 */
void bridge_output_filter_update(const bridge_v2_arb_t *raw);

/* 读取端 (2ms IPC 填充 / 示波器 / 渲染用): 滤波后输出 */
const bridge_v2_arb_t *bridge_output_filter_get(void);
uint32 bridge_output_filter_get_frame_id(void);
uint8  bridge_output_filter_is_busy(void);

/* 调试: 结束线门控内部状态 (当前连续检出帧数 / 是否已确认) */
void bridge_output_filter_get_debug(uint8 *top_streak, uint8 *top_confirmed);

#ifdef __cplusplus
}
#endif

#endif /* BRIDGE_OUTPUT_FILTER_H */
