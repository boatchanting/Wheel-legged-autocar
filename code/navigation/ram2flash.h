#ifndef _RAM_TO_FLASH_H_
#define _RAM_TO_FLASH_H_

#include "zf_common_headfile.h"

// Flash 区域配置
#define FLASH_INFO_PAGE       10      // 信息页：存储标志位和点数
#define FLASH_DATA_START_PAGE 11      // 数据起始页
#define FLASH_DATA_END_PAGE   90      // 数据结束页

// 校验魔数 (用于判断Flash中是否有有效轨迹)
#define TRAJECTORY_MAGIC_NUM  0x5A5A1234 

// 数据转换辅助宏 (避免使用结构体指针)
#define FLOAT_TO_UINT32(val)  (*((uint32*)&(val)))
#define UINT32_TO_FLOAT(val)  (*((float*)&(val)))

// 状态标志
extern uint8_t g_save_finished_flag;  // 存档完成标志位

// 函数声明
void Ram2Flash_Init(void);
uint8_t Ram2Flash_Save(void);
uint8_t Ram2Flash_Load(void);
uint8_t Ram2Flash_IsDataValid(void);
void Ram2Flash_ClearStorage(void);

#endif // _RAM_TO_FLASH_H_