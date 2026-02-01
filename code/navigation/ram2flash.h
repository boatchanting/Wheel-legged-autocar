#ifndef _RAM2FLASH_H_
#define _RAM2FLASH_H_
#include "zf_common_headfile.h"

//-------------------------------------------------------------------------------------------------------------------
//  @brief      Flash 存储配置
//-------------------------------------------------------------------------------------------------------------------
#define R2F_FLASH_SECTOR          0       // 使用的 Flash 扇区 (根据你的硬件配置)
#define R2F_FLASH_START_PAGE      10      // 起始页码
#define R2F_FLASH_END_PAGE        90      // 结束页码
#define R2F_MAX_PAGES             (R2F_FLASH_END_PAGE - R2F_FLASH_START_PAGE + 1)
#define R2F_METADATA_PAGE         R2F_FLASH_START_PAGE // 元数据存储在第一页

#define R2F_MAGIC_NUMBER          0x4A465A54  // "ZFJ T" (Zhufly Trace), 用于验证数据有效性

//-------------------------------------------------------------------------------------------------------------------
//  @brief      轨迹压缩算法配置
//-------------------------------------------------------------------------------------------------------------------
#define R2F_LINE_DEVIATION_MM     20.0f   // 判断为直线的最大偏离距离(mm), 可根据车模循迹精度调整
#define R2F_MIN_LINE_POINTS       5       // 构成一条直线所需的最少连续点数 (例如: 100ms*5 = 500ms 的直线)

//-------------------------------------------------------------------------------------------------------------------
//  @brief      数据结构定义
//-------------------------------------------------------------------------------------------------------------------

// 存储状态
typedef enum {
    R2F_STATUS_IDLE,          // 空闲
    R2F_STATUS_BUSY,          // 正在保存或加载
    R2F_STATUS_SUCCESS,       // 操作成功
    R2F_STATUS_NO_DATA,       // Flash中无有效数据
    R2F_STATUS_DATA_ERROR,    // 数据校验失败
    R2F_STATUS_NO_RAM_DATA,   // RAM中无数据可保存
    R2F_STATUS_FULL           // Flash空间不足
} R2F_Status_t;

// 轨迹段类型
typedef enum {
    SEGMENT_TYPE_CURVE = 0x01, // 曲线段
    SEGMENT_TYPE_LINE  = 0x02  // 直线段
} R2F_SegmentType_t;

// Flash元数据结构 (存储在第一页)
typedef struct {
    uint32_t magic_number;    // 魔数
    uint16_t total_segments;  // 总段数
    uint16_t reserved;        // 预留
    uint32_t checksum;        // 所有轨迹点的校验和
} R2F_Metadata_t; // 大小: 12 字节

// 轨迹段头结构
typedef struct {
    uint8_t type;             // 段类型 (R2F_SegmentType_t)
    uint8_t reserved[1];      // 字节对齐
    uint16_t point_count;     // 该段包含的轨迹点数量
} R2F_SegmentHeader_t; // 大小: 4 字节

//-------------------------------------------------------------------------------------------------------------------
//  @brief      函数声明
//-------------------------------------------------------------------------------------------------------------------

void R2F_Init(void);
R2F_Status_t R2F_SaveTrajectoryFromRAM(void);
R2F_Status_t R2F_LoadTrajectoryInfo(uint16_t* segment_count);
R2F_Status_t R2F_EraseTrajectory(void);
uint8_t R2F_HasValidTrajectory(void);

#endif // _RAM2FLASH_H_