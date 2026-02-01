#ifndef _RAM_TO_FLASH_H_
#define _RAM_TO_FLASH_H_

#include "zf_common_headfile.h"

//-------------------------------------------------------------------------
// 参数配置
//-------------------------------------------------------------------------
// Flash 空间分配 (用户指定 10-90 页)
#define FLASH_PAGE_INFO       10      // 信息存储页
#define FLASH_PAGE_DATA_START 11      // 数据存储起始页
#define FLASH_PAGE_DATA_END   90      // 数据存储结束页

// 压缩参数
#define COMPRESS_YAW_DIFF     1.0f    // 偏航角变化阈值(度)，大于此值视为曲线

// 校验魔数
#define FLASH_MAGIC_NUM       0xDEADBEEF

//-------------------------------------------------------------------------
// 变量与函数声明
//-------------------------------------------------------------------------

// 保存完成标志位 (0=未完成/闲置, 1=保存成功)
extern uint8 g_flash_save_finished;

void Ram2Flash_Init(void);
uint8 Ram2Flash_SaveCompressed(void); // 保存并压缩
uint8 Ram2Flash_Load(void);           // 读取
uint8 Ram2Flash_CheckValid(void);     // 检查是否有数据
void Ram2Flash_Clear(void);           // 清空数据

#endif // _RAM_TO_FLASH_H_