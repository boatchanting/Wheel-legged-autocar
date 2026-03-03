#ifndef __GNSS_TRANSFORM_H
#define __GNSS_TRANSFORM_H

#include "zf_common_headfile.h"

// --- 输出结构体 ---

typedef struct
{
    // 相对坐标 (单位: 米)
    // 以初始化时的 GPS 点为原点 (0,0)
    // x: 东向位移 (对应高斯投影东向)
    // y: 北向位移 (对应高斯投影北向)
    float    x;             
    float    y;                 
    
    // 原点记录 (用于调试或绝对坐标恢复)
    double   origin_lat;    
    double   origin_lon;    
    
    // 状态标志
    uint8_t  is_origin_set; // 原点是否已设置 (0: 未初始化, 1: 已初始化)
    uint8_t  is_valid;      // 当前数据是否有效 (基于 state 和卫星数)
    
    // 统计信息
    uint32_t update_count;  // 更新计数
    
} gnss_transform_struct;

// --- 全局变量声明 ---
extern gnss_transform_struct gnss_trans;

// --- API 接口 ---

/**
 * @brief 初始化转换模块
 * @note  清除原点，重置状态
 */
void Gnss_Transform_Init(void);

/**
 * @brief 主处理函数，需在主循环中调用
 * @note  读取全局 gnss 数据，进行投影转换，结果写入全局 gnss_trans
 */
void Gnss_Transform_Update(void);

#endif // __GNSS_TRANSFORM_H