;
; ============================================================================
; edge_conv7_asm.s  ——  7-tap 可分离卷积汇编算子 (IAR ARM 汇编器)
;
; Copyright (C) 2026  Ji Zixiang
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.
;
; ============================================================================
; 平台:  Infineon CYT4BB7 Cortex-M7 @ 250MHz
; 段:    .itcm_text -> SELF_ITCM (0x00000000 核内零等待取指)
;
; 内核清单:
;   conv7_horiz_row — 水平 Pass: 1 行(pad后) -> Gx_h + Gy_h (SMLAD 双发射)
;   conv7_vert_col  — 垂直 Pass: 1 列(pad后) -> Gx 或 Gy   (SMLAL 64位)
;
; 核 (FS7 最优整数核):
;   D = [1,14,115,0,-115,-14,-1]  差分
;   P = [2,13,49,75,49,13,2]      平滑
;
; 卷积语义 (与 Python scipy convolve1d 一致, 中心对齐标准卷积):
;   out[i] = Σ_k w[k]·f[i+3-k]   (L=7)
; 边界 reflect (半采样对称) 由 C 骨架铺 pad:
;   水平: row_pad int16[100] = [f2 f1 f0 | f0..f93 | f91 f92 f93]  (4字节对齐)
;   垂直: col_pad int32[66]  = [g2 g1 g0 | g0..g59 | g57 g58 g59]
;
; 精度:
;   水平 |Gx_h| ≤ 255·260 = 66300 > int16 → 水平输出 int32, SMLAD 累加 int32 不溢出
;   垂直 |Gx|   ≤ 66300·203 = 13.5M     → SMLAL 64 位累加后取低 32 位 (精确)
; ============================================================================

    MODULE  edge_conv7_asm
    SECTION .itcm_text:CODE:ROOT(3)
    THUMB

    PUBLIC  conv7_horiz_row
    PUBLIC  conv7_horiz_row_gy
    PUBLIC  conv7_vert_col


; ============================================================================
; conv7_horiz_row  ——  水平 1D 卷积, Gx_h + Gy_h (两遍 SMLAD)
; ============================================================================
; AAPCS:
;   r0 = p_row_pad (const int16_t*, 100 个, 必须 4 字节对齐)
;   r1 = p_gx_h    (int32_t*, 94 个输出)
;   r2 = p_gy_h    (int32_t*, 94 个输出)
;   r3 = out_width (94)
;
; Gx 遍 (D 核, 权重序列 [-1,-14,-115,0,115,14,1] 从 row_pad[i] 起):
;   对0 {p[i+1]·-14, p[i]·-1}   对1 {p[i+3]·0,  p[i+2]·-115}
;   对2 {p[i+5]·14,  p[i+4]·115} 对3 {p[i+7]·0, p[i+6]·1}
; Gy 遍 (P 核, 权重 [2,13,49,75,49,13,2]):
;   对0 {p[i+1]·13, p[i]·2}     对1 {p[i+3]·75, p[i+2]·49}
;   对2 {p[i+5]·13, p[i+4]·49}  对3 {p[i+7]·0, p[i+6]·2}
;
; 寄存器: r0=输入 r1/r2=输出(分遍) r3-r6=核 r7-r10=数据 r11=acc r12=计数
; ============================================================================
    ALIGNROM 3
conv7_horiz_row:
    PUSH    {r4-r11, lr}        ; sp -= 36
    STR     r3, [sp, #-4]!      ; 存 count → [sp+4]
    STR     r0, [sp, #-4]!      ; 存 row_pad 起始 → [sp] = row_pad, [sp+4] = count

    ; ================= Gx 遍 (D 核, 输出 r1) =================
    LDR     r0, [sp]            ; row_pad
    LDR     r12, [sp, #4]       ; count
    MOVW    r3,  #0xFFFF
    MOVT    r3,  #0xFFF2        ; r3 = {-14, -1}
    MOVW    r4,  #0xFF8D
    MOVT    r4,  #0x0000        ; r4 = {0, -115}
    MOVW    r5,  #0x0073
    MOVT    r5,  #0x000E        ; r5 = {14, 115}
    MOVW    r6,  #0x0001
    MOVT    r6,  #0x0000        ; r6 = {0, 1}

    ALIGNROM 3
hgx_loop:
    LDR     r7, [r0], #4        ; r7  = {p[i+1], p[i]}
    LDR     r8, [r0], #4        ; r8  = {p[i+3], p[i+2]}
    LDR     r9, [r0], #4        ; r9  = {p[i+5], p[i+4]}
    LDR     r10, [r0], #4       ; r10 = {p[i+7], p[i+6]}

    MOVS    r11, #0
    SMLAD   r11, r7,  r3, r11   ; Gx += p[i]·(-1) + p[i+1]·(-14)
    SMLAD   r11, r8,  r4, r11   ; Gx += p[i+2]·(-115) + p[i+3]·0
    SMLAD   r11, r9,  r5, r11   ; Gx += p[i+4]·115 + p[i+5]·14
    SMLAD   r11, r10, r6, r11   ; Gx += p[i+6]·1 + p[i+7]·0

    STR     r11, [r1], #4       ; 存 Gx_h[i]
    SUB     r0, r0, #14         ; 回退 14 字节 (净前进 1 int16 = 2 字节)
    SUBS    r12, r12, #1
    BNE     hgx_loop

    ; ================= Gy 遍 (P 核, 输出 r2) =================
    LDR     r0, [sp]            ; row_pad 重新定位
    LDR     r12, [sp, #4]       ; count
    MOVW    r3,  #0x0002
    MOVT    r3,  #0x000D        ; r3 = {13, 2}
    MOVW    r4,  #0x0031
    MOVT    r4,  #0x004B        ; r4 = {75, 49}
    MOVW    r5,  #0x0031
    MOVT    r5,  #0x000D        ; r5 = {13, 49}
    MOVW    r6,  #0x0002
    MOVT    r6,  #0x0000        ; r6 = {0, 2}

    ALIGNROM 3
hgy_loop:
    LDR     r7, [r0], #4
    LDR     r8, [r0], #4
    LDR     r9, [r0], #4
    LDR     r10, [r0], #4

    MOVS    r11, #0
    SMLAD   r11, r7,  r3, r11   ; Gy += p[i]·2 + p[i+1]·13
    SMLAD   r11, r8,  r4, r11   ; Gy += p[i+2]·49 + p[i+3]·75
    SMLAD   r11, r9,  r5, r11   ; Gy += p[i+4]·49 + p[i+5]·13
    SMLAD   r11, r10, r6, r11   ; Gy += p[i+6]·2 + p[i+7]·0

    STR     r11, [r2], #4       ; 存 Gy_h[i]
    SUB     r0, r0, #14
    SUBS    r12, r12, #1
    BNE     hgy_loop

    ADD     sp, sp, #8
    POP     {r4-r11, pc}


; ============================================================================
; conv7_horiz_row_gy  ——  水平 1D 卷积, 仅 Gy_h (P 核单遍, v3 gy-only)
; ============================================================================
; AAPCS:
;   r0 = p_row_pad (const int16_t*, 100 个, 必须 4 字节对齐)
;   r1 = p_gy_h    (int32_t*, 94 个输出)
;   r2 = out_width (94)
; 与 conv7_horiz_row 中的 Gy 遍 (P 核) 完全一致 → 逐位等价;
; 仅省去 Gx 遍 (D 核) 的取指/乘加.
; ============================================================================
    ALIGNROM 3
conv7_horiz_row_gy:
    PUSH    {r4-r11, lr}        ; sp -= 36
    STR     r2, [sp, #-4]!      ; 存 count → [sp+4]
    STR     r0, [sp, #-4]!      ; 存 row_pad 起始 → [sp] = row_pad, [sp+4] = count

    LDR     r0, [sp]            ; row_pad
    LDR     r12, [sp, #4]       ; count
    MOVW    r3,  #0x0002
    MOVT    r3,  #0x000D        ; r3 = {13, 2}
    MOVW    r4,  #0x0031
    MOVT    r4,  #0x004B        ; r4 = {75, 49}
    MOVW    r5,  #0x0031
    MOVT    r5,  #0x000D        ; r5 = {13, 49}
    MOVW    r6,  #0x0002
    MOVT    r6,  #0x0000        ; r6 = {0, 2}

    ALIGNROM 3
hgy2_loop:
    LDR     r7, [r0], #4        ; r7  = {p[i+1], p[i]}
    LDR     r8, [r0], #4        ; r8  = {p[i+3], p[i+2]}
    LDR     r9, [r0], #4        ; r9  = {p[i+5], p[i+4]}
    LDR     r10, [r0], #4       ; r10 = {p[i+7], p[i+6]}

    MOVS    r11, #0
    SMLAD   r11, r7,  r3, r11   ; Gy += p[i]·2 + p[i+1]·13
    SMLAD   r11, r8,  r4, r11   ; Gy += p[i+2]·49 + p[i+3]·75
    SMLAD   r11, r9,  r5, r11   ; Gy += p[i+4]·49 + p[i+5]·13
    SMLAD   r11, r10, r6, r11   ; Gy += p[i+6]·2 + p[i+7]·0

    STR     r11, [r1], #4       ; 存 Gy_h[i]
    SUB     r0, r0, #14         ; 回退 14 字节 (净前进 1 int16 = 2 字节)
    SUBS    r12, r12, #1
    BNE     hgy2_loop

    ADD     sp, sp, #8
    POP     {r4-r11, pc}


; ============================================================================
; conv7_vert_col  ——  垂直 1D 卷积, 单遍 SMLAL (use_d 选核)
; ============================================================================
; AAPCS:
;   r0 = p_col_pad (const int32_t*, 66 个, gx_h 或 gy_h 列已 pad)
;   r1 = p_out     (int32_t*, 60 个输出)
;   r2 = out_height (60)
;   r3 = use_d     (0 → P 核 [gx_h 列 → Gx]; 非0 → D 核 [gy_h 列 → Gy])
;
; 公式: out[y] = Σ_k w[k]·col_pad[y+3-k]   (w = P 或 D, 由 use_d 决定)
;
; 寄存器: r0=输入 r1=输出 r2=计数 r3=数据 r4-r10=核 r11/r12=acc(64)
; ============================================================================
    ALIGNROM 3
conv7_vert_col:
    PUSH    {r4-r11, lr}

    CMP     r3, #0
    BNE     vload_d

    ; ---- P 核 (2,13,49,75,49,13,2) ----
    MOVW    r4,  #0x0002
    MOVT    r4,  #0x0000        ; r4 = 2
    MOVW    r5,  #0x000D
    MOVT    r5,  #0x0000        ; r5 = 13
    MOVW    r6,  #0x0031
    MOVT    r6,  #0x0000        ; r6 = 49
    MOVW    r7,  #0x004B
    MOVT    r7,  #0x0000        ; r7 = 75
    MOVW    r8,  #0x0031
    MOVT    r8,  #0x0000        ; r8 = 49
    MOVW    r9,  #0x000D
    MOVT    r9,  #0x0000        ; r9 = 13
    MOVW    r10, #0x0002
    MOVT    r10, #0x0000        ; r10 = 2
    B       vloop_enter

    ALIGNROM 3
vload_d:
    ; ---- D 核: pad[y] 起升序读, 权重须按 w6..w0 序 (公式 Σ_k w[k]·pad[y+6-k]) ----
    ; D = [1,14,115,0,-115,-14,-1] → 反转 = [-1,-14,-115,0,115,14,1]
    MOVW    r4,  #0xFFFF
    MOVT    r4,  #0xFFFF        ; r4 = -1
    MOVW    r5,  #0xFFF2
    MOVT    r5,  #0xFFFF        ; r5 = -14
    MOVW    r6,  #0xFF8D
    MOVT    r6,  #0xFFFF        ; r6 = -115
    MOVW    r7,  #0x0000
    MOVT    r7,  #0x0000        ; r7 = 0
    MOVW    r8,  #0x0073
    MOVT    r8,  #0x0000        ; r8 = 115
    MOVW    r9,  #0x000E
    MOVT    r9,  #0x0000        ; r9 = 14
    MOVW    r10, #0x0001
    MOVT    r10, #0x0000        ; r10 = 1

    ALIGNROM 3
vloop_enter:
    MOV     r11, r2             ; r11 临时: 先不用, 计数直接用 r2 (循环内)
    ; r2 是计数, r3 是数据临时
    ; 核在 r4-r10, acc 用 r11/r12

    ALIGNROM 3
vloop:
    MOVS    r11, #0
    MOVS    r12, #0
    LDR     r3, [r0], #4
    SMLAL   r11, r12, r3, r4
    LDR     r3, [r0], #4
    SMLAL   r11, r12, r3, r5
    LDR     r3, [r0], #4
    SMLAL   r11, r12, r3, r6
    LDR     r3, [r0], #4
    SMLAL   r11, r12, r3, r7
    LDR     r3, [r0], #4
    SMLAL   r11, r12, r3, r8
    LDR     r3, [r0], #4
    SMLAL   r11, r12, r3, r9
    LDR     r3, [r0], #4
    SMLAL   r11, r12, r3, r10
    STR     r11, [r1], #4       ; 存 out[y] (值 < 2^31, 低 32 位精确)
    SUB     r0, r0, #24         ; 回退 24 字节 (净前进 1 int32 = 4 字节)
    SUBS    r2, r2, #1
    BNE     vloop

    POP     {r4-r11, pc}

    END
