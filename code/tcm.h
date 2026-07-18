/*
 * tcm.h — CYT4BB Cortex-M7 TCM (Tightly-Coupled Memory) 加速宏
 *
 * ITCM (Instruction TCM, 16KB @ 0x00000000 核内地址):
 *   0 等待周期取指，适合放置 ISR / PID / 滤波 / 汇编算子等热点函数
 *
 * DTCM (Data TCM, 16KB @ 0x20000000 核内地址):
 *   0 等待周期数据访问，适合放置频繁读写的运行缓冲区
 *
 * 对比:
 *   CODE_FLASH 经 AXI Cache 命中约 3-5 cycle，未命中约 20-30 cycle
 *   SRAM      经 AXI Cache 约 1-2 cycle
 *   TCM       永远 1 cycle
 *
 * 使用方式:
 *   ITCM_FUNC void hot_function(void) { ... }      // 函数置于 ITCM
 *   DTCM_DATA int32_t my_var = 42;                  // 带初值，Flash→DTCM 启动拷贝
 *   DTCM_BSS  int16_t my_buf[100];                  // 无初值，手动或 ECC 清零
 *
 * 注意事项:
 *   1. ITCM/DTCM 各仅 16KB，需严格控制规模（通过 .map 文件查看占用）
 *   2. 不要把"冷"代码放入 ITCM（挤掉真正的热点）
 *   3. DTCM 不放图像等大数组，优先放频繁访问的小型缓冲区
 *   4. icf 中使用 SELF_ITCM / SELF_DTCM 区段（核内地址 0x00000000 / 0x20000000）
 */
#ifndef _TCM_H_
#define _TCM_H_

#ifndef TCM_ACCEL_ENABLE
#define TCM_ACCEL_ENABLE 1
#endif

#if TCM_ACCEL_ENABLE && defined(__ICCARM__)
    /*
     * IAR _Pragma 将符号放入指定 section。
     * icf 负责:
     *   .itcm_text → SELF_ITCM (0x00000000, 核内 ITCM)
     *   .dtcm_data → SELF_DTCM (0x20000000, 核内 DTCM, 有初值)
     *   .dtcm_bss  → SELF_DTCM (0x20000000, 核内 DTCM, 零初始化)
     */
    #define ITCM_FUNC  _Pragma("location=\".itcm_text\"")
    #define DTCM_DATA  _Pragma("location=\".dtcm_data\"")
    #define DTCM_BSS   _Pragma("location=\".dtcm_bss\"")
#else
    #define ITCM_FUNC
    #define DTCM_DATA
    #define DTCM_BSS
#endif

#endif /* _TCM_H_ */
