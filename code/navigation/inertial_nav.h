#ifndef _INERTIAL_NAV_H_
#define _INERTIAL_NAV_H_
#include "zf_common_headfile.h"
#include "../config/car_select.h"//根据小车选择配置不同的车轮半径参数
//-------------------------------------------------------------------------------------------------------------------
//  @brief      惯性导航模块头文件
//  @note       1. 依赖外部提供正确的偏航角、加速度和轮速数据。
//              2. InertialNav_Update 函数需要在一个精确的 10ms 周期任务中被调用。
//

// --- 系统配置 ---
#define NAV_DT              0.01f   // 导航解算周期 (10ms)【提醒】这个要是改了中断里面也得改

// --- 融合参数 (需要根据实际小车表现进行调优) ---
#define NAV_ALPHA_VEL       1.0f   // 纵向速度融合系数 (0.9 表示90%信任轮速, 10%信任加速度积分)
#define YAW_RATE_DIFF_THRES 0.5f    // 理论角速度与IMU角速度的最大偏差(rad/s)，超过视为打滑
#define NAV_LAT_ACC_DEADZONE 50.0f  // 横向加速度死区。小于此值视为传感器噪声, 不累积侧滑速度
#define NAV_LON_ACC_ZERO_THRESHOLD 0.0f // 需要实验调整，防止空转时漂移

// --- 【换车或者修车需要更换】里程计校准系数 ---
// 通过实验确定此值: 系数 = 实际行驶距离 / 程序计算距离
// 初始值设为 1.0f, 如果程序计算的距离偏小, 则该值 > 1.0; 如果偏大, 则该值 < 1.0
#if CAR_SELECT == 0 // 0代表学习板小车 板子 学习板 v1.2
#define NAV_DISTANCE_SCALE_FACTOR   3.4596f // <--- 在这里填入你计算出的校准值，未调用【优化点】
#define WHEEL_BASE_MM       159.7f  // 小车轮距 (单位: mm)
#define SPEED_TO_MM_S       3.4596f//大致为车轮半径
#endif
#if CAR_SELECT == 2 // 2代表我们新车 板子 2026 /01/16 锦鲤跃龙门

#define NAV_DISTANCE_SCALE_FACTOR   3.566666f // <--- 在这里填入你计算出的校准值，未调用【优化点】
#define WHEEL_BASE_MM       185.0f  // 小车轮距 (单位: mm)
#define SPEED_TO_MM_S       4.866666f*1.065//大致为车轮半径
#endif

#if CAR_SELECT ==  3 // 3代表 【2026/3/30新车】 对应板子 【2026/03/24 最后的舵机v腿】

#define NAV_DISTANCE_SCALE_FACTOR   1.0f // <--- 在这里填入你计算出的校准值，未调用【优化点】
#define WHEEL_BASE_MM       175.0f  // 小车轮距 (单位: mm)
#define SPEED_TO_MM_S       4.79f//大致为车轮半径
#endif

// --- 坐标系数据结构 ---
typedef struct {
    // [输出] 世界坐标系下的位置 (单位: mm)
    float x;//惯性导航X轴位置，单位mm，小车向后为x正方向
    float y;//惯性导航Y轴位置，单位mm，小车向右为y正方向                   
    // [输出] 惯导坐标系下的小车偏航角 (单位: 度, 范围: -180 ~ +180)
    // 即相对于初始方向的角度
    float relative_yaw;
    float init_yaw;
    // [内部状态] 车身坐标系下的速度 (单位: mm/s)
    float vx_body; // 纵向速度 (前进方向为正)
    float vy_body; // 横向速度 (向左侧滑为正)
    float slip_angle; // 侧滑角 (度)
    uint8_t slip_flag;      // 打滑标志位 (0:正常, 1:侧滑, 2:静止, 3:原地自转)
    uint16_t slip_timer_ms; // 侧滑判定状态机计时器
    
    // [调试信息] 用于上位机日志分析打滑阈值
    float current_speed_L;
    float current_speed_R;
    float theoretical_yaw_rate;
    float actual_yaw_rate;
} InertialNav_t;

// --- 全局变量声明 ---
extern InertialNav_t inertial_nav; 

// --- 函数声明 ---
void InertialNav_Init(void);//初始化惯性导航系统
void InertialNav_Update(float curr_yaw,
                        float acc_lat_left, float acc_lon_forward, 
                        float speed_L, float speed_R, float gyro_z_rad_s);

#endif // _INERTIAL_NAV_H_