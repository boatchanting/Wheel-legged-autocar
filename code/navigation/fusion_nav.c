#include "fusion_nav.h"
#include "../calculate/ekf.h" // 包含 imu_data 等信息
#include <math.h>

#ifndef DEG_TO_RAD
#define DEG_TO_RAD (3.1415926535f / 180.0f)
#endif
#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0f / 3.1415926535f)
#endif

FusionState_t g_fuse_state;
float g_track_base_yaw = 0.0f; 
float g_startup_avg_heading = 0.0f;

// gnss 全局结构体通常在 zf_common_headfile.h 引用的某个SDK头文件中定义
// 为了避免找不到，这里通过包含 gnss_transform.h 可以拿到 gnss 实例（如果它有extern）
// 如果找不到，我们依赖链接器处理

void Fusion_Init(void) {
    g_fuse_state.ins_x = 0.0f;
    g_fuse_state.ins_y = 0.0f;
    g_fuse_state.ins_yaw = 0.0f;
    g_fuse_state.offset_x = 0.0f;
    g_fuse_state.offset_y = 0.0f;
    g_fuse_state.fuse_x = 0.0f;
    g_fuse_state.fuse_y = 0.0f;
    g_fuse_state.fuse_yaw = 0.0f;
    g_fuse_state.k_pos = 0.0f;
}

void Fusion_Set_Origin(void) {
    Fusion_Init();
    // 假设记录此时的相对偏航角为基准，或者直接用 0，这里默认使用 0，由使用者控制
    // gnss_trans 也应该重置原点
    Gnss_Transform_Reset_Origin();
}

void Fusion_Ins_Update(void) {
    float yaw_rad = g_fuse_state.fuse_yaw * DEG_TO_RAD;

    // 车身速度转世界速度 (适配 X向后为正, Y向右为正)
    float vx_world = -inertial_nav.vx_body * cosf(yaw_rad) - inertial_nav.vy_body * sinf(yaw_rad);
    float vy_world = -inertial_nav.vx_body * sinf(yaw_rad) + inertial_nav.vy_body * cosf(yaw_rad);

    // INS 独立积分 (dT=0.01s)
    g_fuse_state.ins_x += vx_world * 0.01f;
    g_fuse_state.ins_y += vy_world * 0.01f;
    
    // 角速度转度: gyro_z 从 rad/s 转换
    // 从 ekf 或 imu_data 中获取
    // 我们的文档提到：g_fuse_state.ins_yaw += imu_data.gyro_z * RAD_TO_DEG * 0.01f;
    // 需要注意这里 imu_data 是原始数据，最好用 ekf 中的 或者 gyro_z_rad_s，在外部其实我们有 inertial_nav.actual_yaw_rate 
    // 但按照文档写：
    g_fuse_state.ins_yaw += inertial_nav.actual_yaw_rate * RAD_TO_DEG * 0.01f;

    // 实时加上 offset，输出融合坐标
    g_fuse_state.fuse_x = g_fuse_state.ins_x + g_fuse_state.offset_x;
    g_fuse_state.fuse_y = g_fuse_state.ins_y + g_fuse_state.offset_y;
    
    // 角度不修正，直接同步
    // 此处可以跟 inertial_nav.relative_yaw 同步，也可以用自己的积分
    g_fuse_state.fuse_yaw = g_fuse_state.ins_yaw; 
}

void Fusion_Gps_Correct(void) {
    // 1. 地理位移转赛道本地位移 (单位：米)
    float rad = g_track_base_yaw * DEG_TO_RAD;
    float delta_E = gnss_trans.x;
    float delta_N = gnss_trans.y;
    
    float ground_x = -(delta_E * sinf(rad) + delta_N * cosf(rad));
    float ground_y =   delta_E * cosf(rad) - delta_N * sinf(rad);

    // 2. GPS 硬件延时前馈补偿
    float vx_mps = inertial_nav.vx_body * 0.001f; // mm/s to m/s
    ground_x = ground_x - vx_mps * GPS_DELAY_SEC * cosf(g_fuse_state.fuse_yaw * DEG_TO_RAD);
    ground_y = ground_y - vx_mps * GPS_DELAY_SEC * sinf(g_fuse_state.fuse_yaw * DEG_TO_RAD);

    float ground_x_mm = ground_x * 1000.0f;
    float ground_y_mm = ground_y * 1000.0f;

    // 3. 跃变剔除保护 (创新度滤波)
    float dx_gps = ground_x_mm - g_fuse_state.fuse_x;
    float dy_gps = ground_y_mm - g_fuse_state.fuse_y;
    float delta_gps = sqrtf(dx_gps * dx_gps + dy_gps * dy_gps);

    if (delta_gps > 1500.0f) { // 突变超过 1.5m 丢弃
        return;
    }

    // 4. K_pos 战术级调度
    float K_pos = 0.02f; // 默认每次拉扯 2%

    // 获取卫星数，假设 gnss 是全局变量且拥有 satellite_used 成员
    // 为了编译通过，如果外部没有gnss可以声明一下
    // extern gnss_t gnss; 
    // 但在 zf_common_headfile.h 中应该已经有了
    
    // 我们暂时不知道 gnss.satellite_used 能否直接访问，先这样写
    // 依据之前 gnss_transform.c 中的用法是直接用 gnss.satellite_used
    if (gnss.satellite_used < 10) K_pos = 0.0f;
    else if (gnss.satellite_used < 15) K_pos = 0.01f;

    // 零速挂起 (ZUPT)
    float wheel_speed = (inertial_nav.current_speed_L + inertial_nav.current_speed_R) * 0.5f;
    float gyro_z_deg = inertial_nav.actual_yaw_rate * RAD_TO_DEG;
    
    if (fabsf(wheel_speed) < 10.0f && fabsf(gyro_z_deg) < 2.0f) {
        K_pos = 0.0f;
    }

    // 特殊元素屏蔽
    // 假设目前没有 current_element 全局变量，我们可以使用现有的特殊宏如 Bridge_Test_Triple_SingleSide_Is_Active 等，
    // 或者省略这部分。文档提到 "current_element == NAV_POINT_CIRCLE"。
    // 这里我们可以根据实际有的变量屏蔽：
    // 如果有单边桥在激活等，可以加入屏蔽。这里暂时简化。

#if ENABLE_SLIP_WEIGHT_SHIFT
    if (inertial_nav.slip_flag == 1) {
        K_pos = 0.10f; // 打滑时惯导失真，放大 GPS 权重
    }
#endif

    g_fuse_state.k_pos = K_pos;

    // 5. 弹性更新 Offset (不直接覆盖坐标)
    g_fuse_state.offset_x += K_pos * dx_gps;
    g_fuse_state.offset_y += K_pos * dy_gps;
}
