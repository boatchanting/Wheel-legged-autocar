#ifndef __GNSS_TRANSFORM_H
#define __GNSS_TRANSFORM_H

#include "zf_common_headfile.h"

// ================= GPS 物理安装与延时参数 =================
// 【需实车测量】请拿着卷尺去实车测量后填入具体数值

// 天线离地高度 (以两轮轴心连线中点为 0 点，垂直向上的距离，单位：米)
#define ANTENNA_HEIGHT_Z    0.15f

// 天线前后偏移量 (天线中心相对于两轮轴心所在垂直平面的前后水平距离)
// 偏向车头方向为正，偏向车尾为负，正上方为 0.0f
#define ANTENNA_OFFSET_X    0.04f

// 天线左右偏移量 (天线中心相对于车身纵向中轴线的左右水平距离)
// 偏向车身左侧为正，偏向右侧为负，正中心为 0.0f
#define ANTENNA_OFFSET_Y    0.04f

// GPS 模块硬延时 (从真实位置到串口解析出数据的延迟时间，单位：秒)
// 115200波特率+10Hz解算，通常在 0.1s ~ 0.15s 之间
#define GPS_DELAY_SEC       0.12f

// --- 输出结构体 ---

typedef struct
{
    // 相对坐标 (单位: 米)
    // 以锁定原点为基准
    // x: 东向位移 (对应高斯投影东向)
    // y: 北向位移 (对应高斯投影北向)
    float    x;
    float    y;

    // 杆臂补偿 + 延时前馈后的地面中心坐标 (单位: 米)
    float    ground_x;
    float    ground_y;

    // 原点经纬度 (手动锁定时写入)
    double   origin_lat;
    double   origin_lon;

    // 当前帧经纬度 (始终更新，供手动锁定使用)
    double   current_lat;
    double   current_lon;

    // 状态标志
    uint8_t  is_origin_set; // 原点是否已设置 (0: 未初始化, 1: 已锁定)
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
 * @brief 主处理函数，在 GNSS 数据解析后调用
 * @note  读取全局 gnss 数据，进行高斯投影转换，结果写入全局 gnss_trans
 *        始终更新 current_lat/lon 和 is_valid
 *        仅在 is_origin_set==1 时计算 x/y 相对坐标
 */
void Gnss_Transform_Update(void);

/**
 * @brief 强制重置原点
 */
void Gnss_Transform_Reset_Origin(void);

/**
 * @brief 直接设定原点 (手动锁定模式)
 * @param lat  原点纬度
 * @param lon  原点经度
 * @note  计算高斯投影绝对坐标并存储为原点基准
 */
void Gnss_Transform_SetOriginDirect(double lat, double lon);

/**
 * @brief 杆臂补偿 + 延时前馈，将天线 GPS 坐标还原为地面中心坐标
 * @param gps_x         天线投影 X (米，东向)
 * @param gps_y         天线投影 Y (米，北向)
 * @param pitch_rad     IMU 俯仰角 (弧度)
 * @param yaw_deg       融合偏航角 (度)
 * @param vx_body_mm_s  车身纵向速度 (mm/s，前进为正)
 * @param out_ground_x  输出地面 X (米)
 * @param out_ground_y  输出地面 Y (米)
 *
 * 坐标系约定：底层 X 向后为正，Y 向右为正
 */
void Gnss_Transform_ComputeGround(float gps_x, float gps_y,
                                   float pitch_rad, float yaw_deg,
                                   float vx_body_mm_s,
                                   float *out_ground_x, float *out_ground_y);

#endif // __GNSS_TRANSFORM_H
