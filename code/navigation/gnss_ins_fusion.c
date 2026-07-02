#include "gnss_ins_fusion.h"
#include "gnss_transform.h"
#include "inertial_nav.h"
#include "nav_ram.h"                // nav_ram_data, NavPointType_e
#include "../calculate/ekf.h"       // imu_data.gyro_z, euler_angle.pitch, heading
#include "../config/sys_options.h"  // IMU_CATEGORY, CURRENT_NAV_PLAN

// 获取当前寻迹目标索引 (定义在 plan1_gnss.c 或 plan2_*.c 中)
extern uint16 g_target_idx;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
#define RAD_TO_DEG (180.0f / (float)M_PI)
#define DEG_TO_RAD ((float)M_PI / 180.0f)

// ==================== 全局变量 ====================
FusionState_t g_fuse_state = {0};
float g_track_base_yaw = 0.0f;  // 发车基准角 (度)，由手动锁定时写入

// ==================== 内部状态 ====================
static float s_last_ground_x = 0.0f;  // 上一帧 GPS 地面坐标 (mm)
static float s_last_ground_y = 0.0f;
static uint8_t s_origin_initialized = 0; // 原点锁定后是否已完成融合初始化

// ==================== 内部辅助函数 ====================

static float _normalize_angle_180(float angle)
{
    while (angle > 180.0f)  angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

// ==================== API 实现 ====================

void Fusion_Init(void)
{
    g_fuse_state.ins_x   = 0.0f;
    g_fuse_state.ins_y   = 0.0f;
    g_fuse_state.ins_yaw = 0.0f;

    g_fuse_state.offset_x   = 0.0f;
    g_fuse_state.offset_y   = 0.0f;
    g_fuse_state.offset_yaw = 0.0f;

    g_fuse_state.fuse_x   = 0.0f;
    g_fuse_state.fuse_y   = 0.0f;
    g_fuse_state.fuse_yaw = 0.0f;

    g_track_base_yaw = 0.0f;

    s_last_ground_x = 0.0f;
    s_last_ground_y = 0.0f;
    s_origin_initialized = 0;
}

void Fusion_Manual_Lock_Origin(void)
{
    // 确保当前有有效定位 (防止乱锁)
    if (gnss.satellite_used < 8 || gnss.latitude <= 0.0)
    {
#if DEBUG_LOG_ENABLE
        printf("[FUSION] Lock rejected: sat=%d, lat=%.6f\r\n",
               gnss.satellite_used, gnss.latitude);
#endif
        return;
    }

    // 1. 锁定世界物理原点 (使用当前帧经纬度)
    Gnss_Transform_SetOriginDirect(gnss_trans.current_lat, gnss_trans.current_lon);

    // 2. 记录此时车头的绝对地理方向 (赛道基准角)
    //    单天线无 THS，使用 IMU 磁力计绝对航向
#if IMU_CATEGORY == 3
    g_track_base_yaw = heading;  // IMU963RA 磁力计航向 (度)
#else
    g_track_base_yaw = gnss.direction;  // 退化为 RMC 航向
#endif

    // 3. 惯导状态彻底清零
    //    在本地赛道坐标系下，发车瞬间的车头朝向永远是 0 度
    g_fuse_state.ins_x = 0.0f;
    g_fuse_state.ins_y = 0.0f;
    g_fuse_state.ins_yaw = 0.0f;

    g_fuse_state.offset_x = 0.0f;
    g_fuse_state.offset_y = 0.0f;
    g_fuse_state.offset_yaw = 0.0f;  // 单天线不更新此项

    g_fuse_state.fuse_x = 0.0f;
    g_fuse_state.fuse_y = 0.0f;
    g_fuse_state.fuse_yaw = 0.0f;

    // 4. 重置内部状态
    s_last_ground_x = 0.0f;
    s_last_ground_y = 0.0f;
    s_origin_initialized = 1;

#if DEBUG_LOG_ENABLE
    printf("[FUSION] Origin LOCKED. lat=%.6f lon=%.6f base_yaw=%.1f\r\n",
           gnss_trans.origin_lat, gnss_trans.origin_lon, g_track_base_yaw);
#endif
}

void Fusion_Ins_Update(void)
{
    // ==================== 100Hz 惯导推算 ====================
    // 坐标系约定：X 向后为正，Y 向右为正
    // 推导：向前(vx_body>0)产生负X位移，向右(vy_body>0)产生正Y位移

    float yaw_rad = g_fuse_state.fuse_yaw * DEG_TO_RAD;
    float vx_body = inertial_nav.vx_body; // mm/s，前进为正
    float vy_body = inertial_nav.vy_body; // mm/s，左侧滑为正

    // 车身 → 世界坐标变换 (适配 X后为正, Y右为正)
    float vx_world = -vx_body * cosf(yaw_rad) - vy_body * sinf(yaw_rad);
    float vy_world = -vx_body * sinf(yaw_rad) + vy_body * cosf(yaw_rad);

    // 积分 INS 坐标 (mm)
    g_fuse_state.ins_x += vx_world * 0.01f;
    g_fuse_state.ins_y += vy_world * 0.01f;

    // 陀螺仪 Z 轴积分偏航角 (度)
    g_fuse_state.ins_yaw += imu_data.gyro_z * RAD_TO_DEG * 0.01f;

    // 实时输出融合坐标 (喂给控制层)
    g_fuse_state.fuse_x   = g_fuse_state.ins_x   + g_fuse_state.offset_x;
    g_fuse_state.fuse_y   = g_fuse_state.ins_y   + g_fuse_state.offset_y;
    g_fuse_state.fuse_yaw = g_fuse_state.ins_yaw  + g_fuse_state.offset_yaw;
}

void Fusion_Gps_Correct(void)
{
    // ==================== 10Hz GPS 纠偏 ====================
    // 前提：Gnss_Transform_Update() 已执行，gnss_trans.x/y 已更新

    if (!gnss_trans.is_valid || !gnss_trans.is_origin_set)
    {
        return;
    }

    // --- 获取基于锁定原点的高斯投影地理位移 (米) ---
    float delta_E = gnss_trans.x;  // 东向位移
    float delta_N = gnss_trans.y;  // 北向位移

    // ========================================================
    // 坐标系旋转：地球系(东/北) -> 赛道本地系 (X向后，Y向右)
    // g_track_base_yaw: 0度=正北，顺时针为正
    // ========================================================
    float rad = g_track_base_yaw * DEG_TO_RAD;
    float raw_local_x = -(delta_E * sinf(rad) + delta_N * cosf(rad));
    float raw_local_y =   delta_E * cosf(rad) - delta_N * sinf(rad);

    // ========================================================
    // 全向杆臂补偿 (在赛道本地坐标系下进行)
    // ========================================================
    float d_forward = ANTENNA_OFFSET_X * cosf(euler_angle.pitch) + ANTENNA_HEIGHT_Z * sinf(euler_angle.pitch);
    float d_left    = ANTENNA_OFFSET_Y;

    float ground_x = raw_local_x + d_forward * cosf(g_fuse_state.fuse_yaw * DEG_TO_RAD)
                                  + d_left    * sinf(g_fuse_state.fuse_yaw * DEG_TO_RAD);
    float ground_y = raw_local_y + d_forward * sinf(g_fuse_state.fuse_yaw * DEG_TO_RAD)
                                  - d_left    * cosf(g_fuse_state.fuse_yaw * DEG_TO_RAD);

    // GPS 硬件延时补偿
    float vx_mps = inertial_nav.vx_body * 0.001f; // mm/s → m/s
    ground_x = ground_x - vx_mps * GPS_DELAY_SEC * cosf(g_fuse_state.fuse_yaw * DEG_TO_RAD);
    ground_y = ground_y - vx_mps * GPS_DELAY_SEC * sinf(g_fuse_state.fuse_yaw * DEG_TO_RAD);

    // 转为 mm (与 INS 坐标系一致)
    float ground_x_mm = ground_x * 1000.0f;
    float ground_y_mm = ground_y * 1000.0f;

    // ========================================================
    // 异常剔除与互补融合
    // ========================================================

    // 【补丁 1：防 GPS 闪现跃变】
    float dx_gps = ground_x_mm - s_last_ground_x;
    float dy_gps = ground_y_mm - s_last_ground_y;
    float delta_gps = sqrtf(dx_gps * dx_gps + dy_gps * dy_gps);
    if (delta_gps > 1500.0f)  // 1.5m = 1500mm
    {
        return;  // 丢弃本帧，靠惯导盲走
    }
    s_last_ground_x = ground_x_mm;
    s_last_ground_y = ground_y_mm;

    // ====================================================
    // 战术级 K_pos 调度器
    // ====================================================

    // 默认融合权重 (每次拉扯 2% 的误差，极度平滑)
    float K_pos = 0.02f;

    // --- 战术规则 1：基于卫星数量的分级调度 ---
    if (gnss.satellite_used < 10)
    {
        K_pos = 0.0f;   // 卫星严重不足，GPS 可能严重漂移，直接弃用
    }
    else if (gnss.satellite_used < 15)
    {
        K_pos = 0.01f;  // 卫星数一般，拉扯力度减半
    }
    // 卫星 >=15，维持默认 K_pos = 0.02f

    // --- 战术规则 2：基于赛道特殊元素的调度 (最高优先级) ---
    // 在雷区、单边桥、颠簸路段等对姿态要求极高的区域：彻底挂起 GPS
    // 此规则覆盖卫星数规则，确保特殊元素内 100% 信任惯导
    if (nav_ram_data.point_count > 0 && g_target_idx < nav_ram_data.point_count)
    {
        uint8 current_element = nav_ram_data.points[g_target_idx].point_type;
        if (current_element == NAV_POINT_CIRCLE ||
            current_element == NAV_POINT_BRIDGE ||
            current_element == NAV_POINT_BUMP)
        {
            K_pos = 0.0f;
        }
    }

    // --- 战术规则 3：底层物理异常兜底 ---

    // 零速挂起 ZUPT：停车时 GPS 散布导致原地乱动
    float wheel_speed = 0.5f * (fabsf(motor_value.receive_left_speed_data) +
                                 fabsf(motor_value.receive_right_speed_data));
    float gyro_z_deg_s = imu_data.gyro_z * RAD_TO_DEG;
    if (fabsf(wheel_speed) < 10.0f && fabsf(gyro_z_deg_s) < 2.0f)
    {
        K_pos = 0.0f;
    }

    // 打滑信任权重转移：打滑时惯导失真，被迫放大 GPS 权重
    if (inertial_nav.slip_flag == 1)
    {
        K_pos = 0.10f;
    }

    // ====================================================
    // 执行融合更新 (Offset 弹性更新，禁止硬覆盖)
    // ====================================================
    float err_x = ground_x_mm - g_fuse_state.fuse_x;
    float err_y = ground_y_mm - g_fuse_state.fuse_y;
    g_fuse_state.offset_x += K_pos * err_x;
    g_fuse_state.offset_y += K_pos * err_y;

    // 【单天线不修正 yaw】航向 100% 交给陀螺仪积分
}
