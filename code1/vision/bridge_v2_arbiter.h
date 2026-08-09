/**
 * ============================================================================
 * bridge_v2_arbiter.h  ——  新单边桥管线仲裁层 (bridge_result_t → 控制线)
 * ============================================================================
 * 作用: 按设计文档 §3.3 信任规则, 把新模块 (bridge_detect) 的原始三线结果
 *       仲裁成"一条控制线 + 原始可信标志", 供 IPC b2_* 发布。
 * 仲裁在 1核 完成; 0核 只消费 b2_* (不做跨源平滑)。
 *
 * 信任规则 (§3.3):
 *   RB/RMB → 红蓝中点线 (绿线仅校验)
 *   RM/MB  → 绿线 (直接出中线)
 *   R/B/M/NONE/RB_Q → 视觉失能 (回锁角)
 *
 * 坐标: 图像坐标 94x60, 控制线 x=a*y+b, 退出线 y=a*x+b; 支撑 u_lo/u_hi 为 y 范围。
 * 定点: a×1000, b×100 (与 IPC b2_* 字段一致)。
 * ============================================================================
 * 关联: 设计文档《新单边桥视觉管线接入设计.md》§3.3/§4.3/§5.2 (C21)
 */

#ifndef BRIDGE_V2_ARBITER_H
#define BRIDGE_V2_ARBITER_H

#include "zf_common_headfile.h"
#include "bridge_detect.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 仲裁输出 (与 IPC b2_* 字段一一对应, 设计文档 §4.3) */
typedef struct
{
    uint8  valid;            /* 仲裁后控制线原始可信 (0=失能回锁角) */
    uint8  source;           /* 0=红蓝中点 1=绿线 2=失能 (诊断) */
    uint8  mode;             /* 原始 bridge_mode_t 0~8 */
    uint8  gate;             /* 底部变白锁存 */
    uint8  has_top;          /* 退出线有效 (需 gate=1) */
    uint8  u_lo;             /* 控制线支撑 y 下限 */
    uint8  u_hi;             /* 控制线支撑 y 上限 */
    int16  line_a_x1000;     /* 控制线斜率 a ×1000 */
    int16  line_b_x100;      /* 控制线截距 b ×100 */
    int16  top_a_x1000;      /* 退出线斜率 a ×1000 (横线 y=a*x+b) */
    int16  top_b_x100;       /* 退出线截距 b ×100 */
    uint16 spacing_x100;     /* 红蓝间距@y=55 ×100 (诊断) */
    uint16 mid_ratio_x1000;  /* 中线底间距比 ×1000 (诊断) */
} bridge_v2_arb_t;

/* 单帧仲裁: res → out (out 全字段清零后按规则填充) */
void bridge_v2_arbitrate(const bridge_result_t *res, bridge_v2_arb_t *out);

/* 每帧处理入口 (main_cm7_1 每帧调用): 仲裁 + 帧计数 (写忙保护) */
void bridge_v2_arbiter_process(const bridge_result_t *res);

/* 读取端 (2ms 发布路径用) */
const bridge_v2_arb_t *bridge_v2_arbiter_get(void);
uint32 bridge_v2_arbiter_get_frame_id(void);
uint8  bridge_v2_arbiter_is_busy(void);

#ifdef __cplusplus
}
#endif

#endif /* BRIDGE_V2_ARBITER_H */
