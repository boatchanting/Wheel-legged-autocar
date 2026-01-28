#ifndef _FLASH_H_
#define _FLASH_H_

#include "zf_common_headfile.h" // 包含逐飞库头文件以使用 float 定义

// ==========================================
// 1. Flash 存储位置配置
// ==========================================
// 请根据实际情况修改，确保不覆盖代码区
// 通常使用扇区 0 的第 0 页，或者最后几个扇区
// --- flash宏定义 ---
#define FLASH_SECTION_INDEX       (0)                                 // 存储数据用的扇区
#define FLASH_PAGE_INDEX          (0)                                // 存储数据用的页码 倒数第一个页码
// 参数在 flash_union_buffer 中的索引映射 (0-8)
#define IDX_SPD_P   0
#define IDX_SPD_I   1
#define IDX_SPD_D   2

#define IDX_ANG_P   3
#define IDX_ANG_I   4
#define IDX_ANG_D   5

#define IDX_GYR_P   6
#define IDX_GYR_I   7
#define IDX_GYR_D   8

#define IDX_SERVO_P  9
#define IDX_SERVO_I  10
#define IDX_SERVO_D  11

// 数据总个数 (9个 float)
#define PARAM_NUM   12
#define flash_enable   0
// ==========================================
// 函数声明
// ==========================================
void param_read_from_flash(void);   // 从 Flash 读取参数
void param_save_to_flash(void);     // 将当前参数保存到 Flash

#endif