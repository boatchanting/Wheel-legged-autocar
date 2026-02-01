#ifndef _NAV_RAM_H_
#define _NAV_RAM_H_

#include "zf_common_headfile.h"

// 配置参数
#define NAV_RECORD_INTERVAL_MS    100     // 记录间隔(ms)
#define NAV_RECORD_DT             0.1f    // 记录间隔(s)

// RAM存储配置
#define NAV_MAX_RAM_SIZE_KB       128     // 最大占用RAM大小(KB)
#define NAV_RECORD_SIZE_BYTES     12      // 每条记录大小(字节): x(4) + y(4) + yaw(4)
#define NAV_MAX_RECORDS           ((NAV_MAX_RAM_SIZE_KB * 1024) / NAV_RECORD_SIZE_BYTES)  // 最大记录条数

// 状态标志
typedef enum {
    NAV_STATUS_IDLE = 0,      // 空闲
    NAV_STATUS_RECORDING,     // 记录中
    NAV_STATUS_FULL,          // 存储已满
    NAV_STATUS_ERROR          // 错误
} NavRecordStatus_t;

// 轨迹点数据结构
typedef struct {
    float x;      // X坐标(mm)
    float y;      // Y坐标(mm)
    float yaw;    // 偏航角(度)
} NavPoint_t;

// 存储管理结构体
typedef struct {
    NavPoint_t* buffer;           // 存储缓冲区指针
    uint16_t write_index;         // 写入索引
    uint16_t read_index;          // 读取索引
    uint16_t record_count;        // 当前记录数量
    NavRecordStatus_t status;     // 当前状态
    uint32_t last_record_time;    // 上次记录时间(ms)
    uint8_t overflow_flag;        // 溢出标志: 0=未溢出, 1=已溢出
} NavRecordManager_t;

// 全局变量声明
extern NavRecordManager_t nav_manager;

// 函数声明
void NAV_RAM_Init(void);
uint8_t NAV_RAM_AddRecord(float x, float y, float yaw);
uint8_t NAV_RAM_GetRecord(uint16_t index, NavPoint_t* point);
uint16_t NAV_RAM_GetRecordCount(void);
void NAV_RAM_ClearRecords(void);
NavRecordStatus_t NAV_RAM_GetStatus(void);
uint8_t NAV_RAM_IsFull(void);
float NAV_RAM_GetUsedPercentage(void);
uint16_t NAV_RAM_GetFreeSpace(void);

#endif // _NAV_RAM_H_