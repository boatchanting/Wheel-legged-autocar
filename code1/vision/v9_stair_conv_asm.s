;
; ============================================================================
; v9_stair_conv_asm.s  ——  V9 台阶检测卷积汇编算子 (IAR ARM 汇编器)
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
;   v9_conv_gx_row  — Gx 2×4 Box-Diff: 2 行→1 行 (SMLAD 双发射)
;   v9_conv_gy_row  — Gy 4×2 Box-Diff: 4 行→1 行 (SMLAD 双发射, 输出 91 列)
;
; 图像规格: uint8 灰度 60×94, 全流水线 int16
; 核常量:   [-1,-1,1,1] 全部 MOVW+MOVT 寄存器常驻
; ============================================================================

    MODULE  v9_stair_conv_asm
    SECTION .itcm_text:CODE:ROOT(3)
    THUMB

    PUBLIC  v9_conv_gx_row
    PUBLIC  v9_conv_gy_row


; ============================================================================
; v9_conv_gx_row  ——  Gx 2×4 Box-Diff 卷积
; ============================================================================
; AAPCS:
;   r0 = p_row0   (const int16_t*, 94 个元素)
;   r1 = p_row1   (const int16_t*, 94 个元素)
;   r2 = p_out    (int16_t*, 91 个元素)
;   r3 = out_width (91)
;
; 核常量 (寄存器):
;   r5  = {-1, -1}  核前半: 差分负号
;   r9  = { 1,  1}  核后半: 差分正号
;
; 内循环 (每个输出像素):
;   4 LDR (2 行各 2 对) + 4 SMLAD (Slot A/B 交替双发射) + 1 STRH
;   每迭代 ~14c / 8 MAC ≈ 1.75 c/MAC
; ============================================================================
    ALIGNROM 3
v9_conv_gx_row:
    PUSH    {r4-r12, lr}

    ; ---- 初始化核常量到寄存器 ----
    MOVW    r5,  #0xFFFF
    MOVT    r5,  #0xFFFF         ; r5 = {-1, -1}
    MOVW    r9,  #0x0001
    MOVT    r9,  #0x0001         ; r9 = { 1,  1}

    MOV     r12, r0              ; r12 = p_row0 工作指针
    MOV     r0, r3               ; r0  = 循环计数 = out_width

    ALIGNROM 3
v9_gx_loop:
    LDR     r4, [r12], #4        ; r4 = {p0[x+1], p0[x]},     r12 += 4 → p0[x+2]
    LDR     r6, [r12]            ; r6 = {p0[x+3], p0[x+2]},   r12 不动
    LDR     r7, [r1], #4         ; r7 = {p1[x+1], p1[x]},     r1  += 4 → p1[x+2]
    LDR     r8, [r1]             ; r8 = {p1[x+3], p1[x+2]},   r1  不动

    MOVS    r10, #0               ; 清零累加器

    ; 4 条 SMLAD: r5 和 r9 为只读 → Slot A/B 交替双发射
    SMLAD   r10, r4, r5, r10     ; acc -= p0[x] + p0[x+1]
    SMLAD   r10, r6, r9, r10     ; acc += p0[x+2] + p0[x+3]
    SMLAD   r10, r7, r5, r10     ; acc -= p1[x] + p1[x+1]
    SMLAD   r10, r8, r9, r10     ; acc += p1[x+2] + p1[x+3]

    STRH    r10, [r2], #2         ; 存 Gx[x]

    ; stride-1 回退: 前进 4B, 回退 2B → 净 +1 元素
    SUB     r12, r12, #2
    SUB     r1,  r1,  #2

    SUBS    r0, r0, #1
    BNE     v9_gx_loop

    POP     {r4-r12, pc}


; ============================================================================
; v9_conv_gy_row  ——  Gy 4×2 Box-Diff 卷积
; ============================================================================
; AAPCS:
;   r0 = p_row0   (const int16_t*, 94 个元素)
;   r1 = p_row1   (const int16_t*, 94 个元素)
;   r2 = p_row2   (const int16_t*, 94 个元素)
;   r3 = p_row3   (const int16_t*, 94 个元素)
;   [sp+0]  = p_out     (int16_t*, 91 个元素)  ← PUSH {r4-r12,lr}=40B偏移
;   [sp+4]  = out_width (91)
;
; 核常量 (寄存器):
;   r8  = {-1, -1}  前2行的差分负号
;   r9  = { 1,  1}  后2行的差分正号
;
; 内循环 (每个输出像素):
;   4 LDR (4 行各 1 对) + 4 SMLAD + 1 STRH + 4 SUB + SUBS/BNE
;   每迭代 ~16c / 8 MAC ≈ 2.0 c/MAC
; ============================================================================
    ALIGNROM 3
v9_conv_gy_row:
    PUSH    {r4-r12, lr}

    ; ---- 加载栈参数 (PUSH 偏移 40) ----
    LDR     r12, [sp, #40]        ; r12 = p_out
    LDR     r11, [sp, #44]        ; r11 = out_width (循环计数)

    ; ---- 初始化核常量 ----
    MOVW    r8,  #0xFFFF
    MOVT    r8,  #0xFFFF         ; r8 = {-1, -1}
    MOVW    r9,  #0x0001
    MOVT    r9,  #0x0001         ; r9 = { 1,  1}

    ALIGNROM 3
v9_gy_loop:
    LDR     r4, [r0], #4         ; r4 = {p0[x+1], p0[x]}, r0 += 4
    LDR     r5, [r1], #4         ; r5 = {p1[x+1], p1[x]}, r1 += 4
    LDR     r6, [r2], #4         ; r6 = {p2[x+1], p2[x]}, r2 += 4
    LDR     r7, [r3], #4         ; r7 = {p3[x+1], p3[x]}, r3 += 4

    MOVS    r10, #0

    ; 前2行减, 后2行加 — r8,r9 只读 → 双发射
    SMLAD   r10, r4, r8, r10     ; acc -= p0[x] + p0[x+1]
    SMLAD   r10, r5, r8, r10     ; acc -= p1[x] + p1[x+1]
    SMLAD   r10, r6, r9, r10     ; acc += p2[x] + p2[x+1]
    SMLAD   r10, r7, r9, r10     ; acc += p3[x] + p3[x+1]

    STRH    r10, [r12], #2        ; 存 Gy[x]

    ; stride-1 回退: 4 个行指针各回退 2 字节 → 净 +1 元素
    SUB     r0, r0, #2
    SUB     r1, r1, #2
    SUB     r2, r2, #2
    SUB     r3, r3, #2

    SUBS    r11, r11, #1
    BNE     v9_gy_loop

    POP     {r4-r12, pc}


    END
