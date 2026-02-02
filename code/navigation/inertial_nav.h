#ifndef _INERTIAL_NAV_H_
#define _INERTIAL_NAV_H_
#include "zf_common_headfile.h"
//-------------------------------------------------------------------------------------------------------------------
//  @brief      惯性导航模块头文件
//  @note       1. 依赖外部提供正确的偏航角、加速度和轮速数据。
//              2. InertialNav_Update 函数需要在一个精确的 10ms 周期任务中被调用。
//

// --- 系统配置 ---
#define NAV_DT              0.01f   // 导航解算周期 (10ms)【提醒】这个要是改了中断里面也得改

// --- 融合参数 (需要根据实际小车表现进行调优) ---
#define NAV_ALPHA_VEL       1.0f   // 纵向速度融合系数 (0.9 表示90%信任轮速, 10%信任加速度积分)
#define NAV_DECAY_LAT       1.0f   // 横向速度(侧滑)衰减系数, 模拟摩擦力, 防止侧滑速度无限累积
#define NAV_LAT_ACC_DEADZONE 50.0f  // 横向加速度死区。小于此值视为传感器噪声, 不累积侧滑速度
#define NAV_LON_ACC_ZERO_THRESHOLD 20.0f // 需要实验调整，防止空转时漂移

// --- 【换车或者修车需要更换】里程计校准系数 ---
// 通过实验确定此值: 系数 = 实际行驶距离 / 程序计算距离
// 初始值设为 1.0f, 如果程序计算的距离偏小, 则该值 > 1.0; 如果偏大, 则该值 < 1.0
#define NAV_DISTANCE_SCALE_FACTOR   3.624049f // <--- 在这里填入你计算出的校准值

// --- 坐标系数据结构 ---
typedef struct {
    // [输出] 世界坐标系下的位置 (单位: mm)
    float x;
    float y;
    // [输出] 惯导坐标系下的小车偏航角 (单位: 度, 范围: -180 ~ +180)
    // 即相对于初始方向的角度
    float relative_yaw;
    // [内部状态] 车身坐标系下的速度 (单位: mm/s)
    float vx_body; // 纵向速度 (前进方向为正)
    float vy_body; // 横向速度 (向左侧滑为正)
} InertialNav_t;

// --- 全局变量声明 ---
extern InertialNav_t inertial_nav; 

// --- 函数声明 ---
void InertialNav_Init(void);//初始化惯性导航系统
void InertialNav_Update(float curr_yaw, float init_yaw, 
                        float acc_lat_left, float acc_lon_forward, 
                        float speed_L, float speed_R);

#endif // _INERTIAL_NAV_H_