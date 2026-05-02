/**
 * @file    ipm_transform.h
 * @brief   车辆逆透视(IPM)查表模块 API
 * @note    适用于 94x60 分辨率，底层基于 int16_t 极速查表
 */

#ifndef __IPM_TRANSFORM_H__
#define __IPM_TRANSFORM_H__

#include "zf_common_headfile.h"

/* --- 宏定义配置 --- */
#define IPM_IMG_WIDTH       94      // 图像宽度
#define IPM_IMG_HEIGHT      60      // 图像高度
#define IPM_INVALID_VAL     32767   // 天空/越界无效标志位

/* --- 数据结构 --- */
/**
 * @brief 物理坐标点结构体
 */
typedef struct {
    int16_t x_mm;       // 物理横向坐标 (向右为正)
    int16_t y_mm;       // 物理纵向坐标 (向前为正)
    bool    is_valid;   // 坐标是否有效 (false 表示该像素在天上或视距外)
} IPM_Point_t;


/* --- 函数声明 --- */

/**
 * @brief 获取图像像素对应的物理坐标 (带严格的越界保护)
 * @param img_x 图像列坐标 (0 ~ 93)
 * @param img_y 图像行坐标 (0 ~ 59)
 * @return IPM_Point_t 结构体，包含真实坐标及有效性
 */
IPM_Point_t IPM_GetPhysicalCoord(uint8_t img_x, uint8_t img_y);

/**
 * @brief 计算图像上两点之间的实际物理直线距离 (耗时，使用了开方)
 * @param pt1_x 点1图像 X 坐标
 * @param pt1_y 点1图像 Y 坐标
 * @param pt2_x 点2图像 X 坐标
 * @param pt2_y 点2图像 Y 坐标
 * @return uint32_t 物理距离(毫米)。如果任一点无效，返回 0xFFFFFFFF
 */
uint32_t IPM_GetDistance_mm(uint8_t pt1_x, uint8_t pt1_y, uint8_t pt2_x, uint8_t pt2_y);

/**
 * @brief 获取两点物理距离的平方 (极速！适合用于距离阈值比较，避免了开方运算)
 * @return uint32_t 物理距离的平方。如果无效返回 0xFFFFFFFF
 */
uint32_t IPM_GetDistanceSq_mm(uint8_t pt1_x, uint8_t pt1_y, uint8_t pt2_x, uint8_t pt2_y);

#endif /* __IPM_TRANSFORM_H__ */