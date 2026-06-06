#ifndef _RAM2FLASH_H_
#define _RAM2FLASH_H_

#include "zf_common_headfile.h"

// ==========================================
// Flash 存储配置
// ==========================================
// 注意：PID 参数占用了第 0 页，惯导数据建议使用第 1 页
#define NAV_FLASH_SECTION          (0)
#define NAV_FLASH_PAGE             (1)   

// 存储校验码，用于判断 Flash 是否有有效数据
#define NAV_FLASH_MAGIC            0xABCD1234

// 缓冲区偏移量定义
#define OFF_MAGIC                  0   // 魔数位置
#define OFF_COUNT                  1   // 总点数位置
#define OFF_PLAN                   2   // Plan 类型位置
#define OFF_POINTS_START           3   // 点数据开始的索引

// ==========================================
// 外部变量声明
// ==========================================
extern volatile uint8 g_save_flash_request;  // 1: 请求将 RAM 存入 Flash
extern volatile uint8 g_load_flash_request;  // 1: 请求将 Flash 读取到 RAM

// ==========================================
// 函数声明
// ==========================================

/**
 * @brief  Flash 任务处理器
 * @note   建议放在 main 的 while(1) 中调用，处理读取和保存请求
 */
void NavFlash_ProcessRequests(void);

/**
 * @brief  底层保存函数：RAM -> Flash
 */
uint8 NavFlash_SaveRamToFlash(void);

/**
 * @brief  底层读取函数：Flash -> RAM
 */
uint8 NavFlash_ReadFlashToRam(void);

#ifndef NAV_FLASH_POINT_WORDS
#define NAV_FLASH_POINT_WORDS      3U
#endif

#ifndef NAV_FLASH_MAX_RAW_POINTS
#define NAV_FLASH_MAX_RAW_POINTS   ((FLASH_PAGE_LENGTH - OFF_POINTS_START) / NAV_FLASH_POINT_WORDS)
#endif

#endif
