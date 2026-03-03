#ifndef _GNSSRAM2FLASH_H_
#define _GNSSRAM2FLASH_H_

#include "zf_common_headfile.h"

// ==========================================
// Flash 存储配置
// ==========================================
// 注意：PID 占用第 0 页，惯导(nav)占用第 1 页，GNSS 建议使用第 2 页
#define GNSS_FLASH_SECTION         (0)
#define GNSS_FLASH_PAGE            (2)   

// 存储校验码，用于判断 Flash 是否有有效数据 (使用不同的魔数区分惯导和GNSS)
#define GNSS_FLASH_MAGIC           0x69551234

// 缓冲区偏移量定义
#define OFF_MAGIC                  0   // 魔数位置
#define OFF_COUNT                  1   // 总点数位置
#define OFF_PLAN                   2   // Plan 类型位置
#define OFF_POINTS_START           3   // 点数据开始的索引

// ==========================================
// 外部变量声明
// ==========================================
extern volatile uint8 g_gnss_save_flash_request;  // 1: 请求将 GNSS RAM 存入 Flash
extern volatile uint8 g_gnss_load_flash_request;  // 1: 请求将 Flash 读取到 GNSS RAM

// ==========================================
// 函数声明
// ==========================================

/**
 * @brief  GNSS Flash 任务处理器
 * @note   建议放在 main 的 while(1) 中调用，处理读取和保存请求
 */
void GnssFlash_ProcessRequests(void);

/**
 * @brief  底层保存函数：GNSS RAM -> Flash
 */
uint8 GnssFlash_SaveRamToFlash(void);

/**
 * @brief  底层读取函数：Flash -> GNSS RAM
 */
uint8 GnssFlash_ReadFlashToRam(void);

#endif // _GNSSRAM2FLASH_H_