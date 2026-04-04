#include "gnss_replay.h"
#include "../common.h"
#include "gnss_transform.h"
#include "../calculate/ekf.h"

// ========================= 内部变量 =========================
GnssReplayState_e g_gnss_replay_state = GNSS_REPLAY_IDLE;
uint16 g_gnss_target_idx = 0;                        
uint8 g_gnss_current_point_type = GNSS_POINT_PATH;   
uint8 g_gnss_special_action_trigger = 0;             

// --- 航向融合相关变量 ---
static float g_yaw_offset = 0.0f;         // 绝对角度与相对角度的差值
static float g_fused_abs_yaw = 0.0f;      // 融合后的绝对航向（-180~180）
static uint8 gnss_yaw_initialized = 0;    // 是否已完成首次航向校准

// 绝对航向融合参数（磁北为主，GNSS方向为辅）
#define MAG_YAW_FUSION_KP      0.20f
#define GNSS_DIR_ASSIST_KP     0.08f

// ========================= 辅助函数 =========================

/**
 * @brief  角度归一化到 (-180 ~ 180)
 */
static float NormalizeAngle(float angle)
{
    while (angle > 180.0f)  angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

/**
 * @brief  计算两点间距离 (单位: 米)
 */
static float CalcDistance(float x1, float y1, float x2, float y2)
{
    return sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

/**
 * @brief  判断当前GNSS方向是否可靠（仅在前进且侧滑较小时使用）
 */
static uint8 IsMotionDirectionReliable(void)
{
    return (gnss.state == 1 &&
            inertial_nav.vx_body < -50.0f &&
            fabsf(inertial_nav.vy_body) < 30.0f);
}

/**
 * @brief  获取磁力计航向（0~360，北为0，顺时针为正）
 * @retval 1: 有效 0: 无效
 */
static uint8 TryGetMagHeading(float *out_yaw)
{
#if IMU_CATEGORY == 3
    float mag_energy = fabsf(mag_x) + fabsf(mag_y) + fabsf(mag_z);

    if ((out_yaw != 0) &&
        (mag_energy > 0.001f) &&
        (heading >= 0.0f) &&
        (heading < 360.0f))
    {
        *out_yaw = NormalizeAngle(heading);
        return 1;
    }
#else
    (void)out_yaw;
#endif
    return 0;
}

// ========================= 航向融合核心算法 =========================

/**
 * @brief  更新航向融合偏移量
 * @note   利用 GNSS 动态方向过滤并校准 IMU 相对方向
 */
static void Update_Fused_Yaw(void)
{
    float measured_abs_yaw = 0.0f;
    float mag_abs_yaw = 0.0f;
    uint8 has_abs_yaw = 0;
    uint8 motion_reliable = IsMotionDirectionReliable();

    // 1) 主航向来源：磁力计北向
    if (TryGetMagHeading(&mag_abs_yaw))
    {
        measured_abs_yaw = mag_abs_yaw;
        has_abs_yaw = 1;
    }

    // 2) 辅助航向来源：GNSS运动方向（仅在可靠运动状态下参与）
    if (motion_reliable)
    {
        float gnss_abs_yaw = NormalizeAngle(gnss.direction);

        if (!has_abs_yaw)
        {
            measured_abs_yaw = gnss_abs_yaw;
            has_abs_yaw = 1;
        }
        else
        {
            float gnss_err = NormalizeAngle(gnss_abs_yaw - measured_abs_yaw);
            measured_abs_yaw = NormalizeAngle(measured_abs_yaw + GNSS_DIR_ASSIST_KP * gnss_err);
        }
    }

    if (!has_abs_yaw) return;

    // 3) 把绝对航向平滑映射到相对航向偏移量，保持外部调用方式不变
    if (!gnss_yaw_initialized)
    {
        g_fused_abs_yaw = measured_abs_yaw;
        g_yaw_offset = NormalizeAngle(g_fused_abs_yaw - inertial_nav.relative_yaw);
        gnss_yaw_initialized = 1;
        return;
    }

    {
        float err = NormalizeAngle(measured_abs_yaw - g_fused_abs_yaw);
        g_fused_abs_yaw = NormalizeAngle(g_fused_abs_yaw + MAG_YAW_FUSION_KP * err);
        g_yaw_offset = NormalizeAngle(g_fused_abs_yaw - inertial_nav.relative_yaw);
    }
}

/**
 * @brief 获取当前融合后的绝对航向 (0=北, 90=东, CW正向)
 */
float GnssReplay_GetFusedYaw(void)
{
    if (gnss_yaw_initialized)
    {
        return g_fused_abs_yaw;
    }
    return NormalizeAngle(inertial_nav.relative_yaw + g_yaw_offset);
}

// ========================= 接口实现 =========================

void GnssReplay_Start(void)
{
    if (gnss_ram_data.point_count == 0) return;

    g_gnss_target_idx = 0; 
    g_gnss_replay_state = GNSS_REPLAY_RUNNING;
    g_gnss_special_action_trigger = 0;
    
    // 重置航向校准状态（每次跑图开始都重新校准）
    g_yaw_offset = 0.0f;
    g_fused_abs_yaw = 0.0f;
    gnss_yaw_initialized = 0;
    
    #if DEBUG_LOG_ENABLE
    printf("[GNSS] Replay START. Total Points: %d\r\n", gnss_ram_data.point_count);
    #endif
}

void GnssReplay_Stop(void)
{
    target_speed_set = 0.0f;
    g_gnss_replay_state = GNSS_REPLAY_IDLE;
    err_degree = 0.0f;
}

void GnssReplay_Process(void)
{
    if (g_gnss_replay_state != GNSS_REPLAY_RUNNING || g_gnss_special_action_trigger == 1) return;

    // 1. 每周期更新一次航向融合
    Update_Fused_Yaw();

    // 2. 检查绝对航向是否已就绪 (防呆保护)
    // 若磁北未就绪且GNSS方向也不可靠，则先低速直行等待航向初始化
    if (!gnss_yaw_initialized)
    {
        // 自动校准阶段：保持直行，等待绝对航向来源可用
        target_speed_set = GNSS_SPEED_SLOW; 
        err_degree = 0.0f; // 保持直走
        return;            // 不进行路径点计算
    }

    // 3. 检查是否跑完全部点位
    if (g_gnss_target_idx >= gnss_ram_data.point_count)
    {
        g_gnss_replay_state = GNSS_REPLAY_FINISHED;
        target_speed_set = GNSS_SPEED_STOP;
        err_degree = 0.0f;
        return;
    }

    // 4. 获取当前目标点数据 (单位: 米)
    float tx = gnss_ram_data.points[g_gnss_target_idx].x;
    float ty = gnss_ram_data.points[g_gnss_target_idx].y;
    g_gnss_current_point_type = gnss_ram_data.points[g_gnss_target_idx].point_type;

    // 5. 计算距离
    float dx = tx - gnss_trans.x; // Delta Easting (东向误差)
    float dy = ty - gnss_trans.y; // Delta Northing (北向误差)
    float dist = CalcDistance(gnss_trans.x, gnss_trans.y, tx, ty);

    // =========================================================
    // 6. 计算期望方位角 (核心数学逻辑更改)
    // =========================================================
    // 在之前的 gnss_transform 中：x代表东，y代表北
    // 在真北坐标系中(GNSS标准)：0度为北，90度为东，顺时针为正
    // 此时目标方位角的纯数学计算公式为: atan2(dx, dy) * 57.3
    // （注意不是 atan2(dy, dx)，因为 atan2 默认 x 是 0 度轴，这里 North 才是 0 度轴）
    float target_yaw = atan2f(dx, dy) * 57.29578f; 
    
    // 获取当前融合绝对航向
    float current_fused_yaw = GnssReplay_GetFusedYaw();

    // 计算偏差 (目标 - 实际)
    err_degree = NormalizeAngle(target_yaw - current_fused_yaw);

    // 7. 运动控制策略
    if (dist <= GNSS_DIST_ARRIVE)
    {
        // --- A. 到达目标点 ---
        target_speed_set = GNSS_SPEED_STOP;
        
        if (g_gnss_current_point_type != GNSS_POINT_PATH) 
        {
             if (g_gnss_current_point_type == GNSS_POINT_CIRCLE) {
                minefield_flag = 1;
            }
            g_gnss_special_action_trigger = 1;
        }
        g_gnss_target_idx++;
    }
    else
    {
        // --- B. 未到达目标点 ---
        if (fabsf(err_degree) > GNSS_YAW_TOLERANCE)
        {
            // 角度偏差较大，先原地/低速旋转
            target_speed_set = GNSS_SPEED_STOP;
        }
        else
        {
            // 角度基本对准，开始移动
            if (dist > GNSS_DIST_FAR)
            {
                target_speed_set = GNSS_SPEED_FAST;
            }
            else if (dist > GNSS_DIST_NEAR)
            {
                float ratio = (dist - GNSS_DIST_NEAR) / (GNSS_DIST_FAR - GNSS_DIST_NEAR);
                target_speed_set = GNSS_SPEED_SLOW + (GNSS_SPEED_FAST - GNSS_SPEED_SLOW) * ratio;
            }
            else
            {
                target_speed_set = GNSS_SPEED_SLOW;
            }
        }
    }
}
