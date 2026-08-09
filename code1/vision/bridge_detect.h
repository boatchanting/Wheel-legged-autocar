/**
 * ============================================================================
 * bridge_detect.h  ——  单边桥三线透视结构提取 (红=左界, 绿=中缝, 蓝=右界)
 * ============================================================================
 * Copyright (C) 2026  Ji Zixiang
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ============================================================================
 * 平台:  Infineon CYT4BB7 (Cortex-M7, CM7_1 核运行)
 *
 * 算法 (对齐 pc_tools/bridge_v11.py 链路, 2026-08-14 改造):
 *   1) 输入 94x60 灰度, 4x4 可分离卷积 (bridge_asm_ops.c) 得 Gx/Gy
 *   2) lock-x 锁定 (|Gx|>2|gvar|) + 动态阈值 (0.3*p99, 下限300)
 *   3) 行背景判断 (ROW_BG_FILTER): 每行过阈像素须构成 1~4 簇, 且距强簇
 *      >2px 的中间强度像素 ≤8; 杂乱行不入候选; 有效行<12 时回退全有效
 *   4) 每行 top-2 → 正/负响应分开序贯 RANSAC, 提出全部竖线 (不预设身份)
 *   5) 线身份分类 (v7 四态: 6段均值差比和 + 双峰参考 + gx极性, 含不可判态):
 *      红蓝间距先验 (近10帧滑动中位; 无样本时无先验取最宽对)
 *      - 2 条间距符合先验        → 红+蓝
 *      - 2 条间距明显小于先验    → 侧线+中线 (明确中线才作中线, 贴边组合→RB_Q)
 *      - 中间线须满足几何条件    → 斜率夹逼 + 支撑范围内间距比∈[0.35,0.65]
 *   6) 驳回链: 全图 bcv 非赛道驳回 → 红蓝夹角/交点/间距驳回 → 边线-中线
 *      交点驳回 → RB_Q 统一置空; valid_detect 线级有效级联 (C 端保留)
 *   7) 粉色退出线: 底部变白门控锁存后才识别; v11 gy 行游程连通域贯通法:
 *      包络内逐行 gy>200 run → 跨行 x 重叠并查集 → 贯通判定 → 逐列峰值
 *      拟合 → TOP_FAR 上方稳定白门控; TOP_GRAD=1 回退旧梯度法 (死代码)
 *
 * 使用:
 *   static bridge_state_t  st;
 *   static bridge_result_t res;
 *   bridge_detect_init(&st);
 *   while (1) {
 *       // 等一帧 94x60 灰度图就绪 (摄像头 DMA 缓冲)
 *       bridge_detect_frame(image_94x60, &st, &res);
 *   }
 * ============================================================================
 */

#ifndef _BRIDGE_DETECT_H_
#define _BRIDGE_DETECT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 图像规格: 输入 94x60 uint8 灰度 (行优先, 行宽 94) ---- */
#define BRIDGE_W        94
#define BRIDGE_H        60

/* ---- 线身份分类结果 ---- */
typedef enum {
    BRIDGE_MODE_NONE = 0,   /* 无可信线                       */
    BRIDGE_MODE_R,          /* 仅左界 (右界出画)              */
    BRIDGE_MODE_B,          /* 仅右界 (左界出画)              */
    BRIDGE_MODE_M,          /* 仅中线                         */
    BRIDGE_MODE_RB,         /* 红+蓝                          */
    BRIDGE_MODE_RMB,        /* 红+绿+蓝 (三线透视结构完整)    */
    BRIDGE_MODE_RM,         /* 红+中线 (摄像头右倾, 蓝出画)   */
    BRIDGE_MODE_MB,         /* 中线+蓝 (摄像头左倾, 红出画)   */
    BRIDGE_MODE_RB_Q        /* 间距过近但亮暗判不出, 保守红蓝 */
} bridge_mode_t;

/* ---- 拟合直线: 竖线 x = a*y+b / 顶线 y = a*x+b (同构复用) ---- */
typedef struct {
    float   a;          /* 斜率                              */
    float   b;          /* 截距                              */
    float   rms;        /* 内点残差 (px)                     */
    int16_t n;          /* 内点数                            */
    float   u_lo;       /* 支撑范围下限 (内点自变量 p10-8)   */
    float   u_hi;       /* 支撑范围上限 (内点自变量 p90+8)   */
} bridge_line_t;

/* ---- 跨帧状态 (随 y 变化红蓝间距先验自校准 + 底部变白门控锁存) ---- */
typedef struct {
    float    sp_buf[10];    /* 红蓝间距滑动窗 (y=55 参考行)  */
    uint8_t  sp_n;          /* 窗内样本数 (≤10)              */
    uint8_t  sp_head;       /* 环形写指针                    */
    uint8_t  gate;          /* 底部变白锁存 (锁存后不撤销)   */
    uint8_t  _pad;
    /* 随 y 变化先验间距 w(y) = wp_a*y + wp_b (透视线性, 最小二乘自校准) */
    float    wp_a;          /* 当前先验斜率 A (px/行)        */
    float    wp_b;          /* 当前先验截距 B (px @y=0)      */
    float    wp_n;          /* 有效校准样本数                */
    float    wp_sy;         /* Σy                            */
    float    wp_sw;         /* Σw                            */
    float    wp_syy;        /* Σy²                           */
    float    wp_syw;        /* Σy·w                          */
} bridge_state_t;

/* ---- 单帧结果 ---- */
typedef struct {
    bridge_mode_t mode;
    uint8_t   has_red;
    uint8_t   has_green;
    uint8_t   has_blue;
    uint8_t   has_top;
    uint8_t   gate;         /* 本帧门控状态                  */
    uint8_t   n_lines;      /* 本帧提取到的竖线条数          */
    uint8_t   n_rows_ok;    /* 行背景判断: 本帧有效行数      */
    uint8_t   valid;        /* 有效检测 (线级级联全通, 2026-08-09) */
    bridge_line_t red;      /* x = a*y+b, has_red 时有效     */
    bridge_line_t green;
    bridge_line_t blue;
    bridge_line_t top;      /* y = a*x+b, has_top 时有效     */
    float     spacing;      /* 本帧红蓝间距@y=55 (RB/RMB)    */
    float     mid_ratio;    /* 中线帧底间距比 (RMB)          */
} bridge_result_t;


void bridge_detect_init(bridge_state_t *st);

/* img94: 94x60 uint8 灰度, 行优先, 行宽 94 */
void bridge_detect_frame(const uint8_t *img94,
                         bridge_state_t *st,
                         bridge_result_t *out);

/* 精准分段测时报告 (未开 BRIDGE_PROF 时为空实现; 调用方在测时区外调用) */
void bridge_prof_report(void);


#ifdef __cplusplus
}
#endif

#endif /* _BRIDGE_DETECT_H_ */
