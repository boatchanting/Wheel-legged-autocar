#include "gnss_replay.h"
#include "../common.h"
#include "gnss_transform.h"

// ========================= 内部变量 =========================
GnssReplayState_e g_gnss_replay_state = GNSS_REPLAY_IDLE;
uint16 g_gnss_target_idx = 0;                        
uint8 g_gnss_current_point_type = GNSS_POINT_PATH;   
uint8 g_gnss_special_action_trigger = 0;             

// --- 航向融合相关变量 ---
static float g_yaw_offset = 0.0f;       // 绝对角度与相对角度的差值
static uint8 g_yaw_initialized = 0;     // 是否已完成首次航向校准

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

// ========================= 航向融合核心算法 =========================

/**
 * @brief  更新航向融合偏移量
 * @note   利用 GNSS 动态方向过滤并校准 IMU 相对方向
 */
static void Update_Fused_Yaw(void)
{
    // 触发条件：
    // 1. GNSS 状态正常 (state == 1)
    // 2. 小车正在向前移动 (vx_body < -50)  [注: 你的设定中向前为负]
    // 3. 小车没有发生严重侧滑或原地剧烈转向 (fabs(vy_body) < 30)
    if (gnss.state == 1 && 
        inertial_nav.vx_body < -50.0f && 
        fabs(inertial_nav.vy_body) < 30.0f)
    {
        float raw_gnss_yaw = gnss.direction; // 0~360 绝对真北基准
        
        // 计算当前误差偏移 (Offset = GNSS绝对 - IMU相对)
        float raw_offset = NormalizeAngle(raw_gnss_yaw - inertial_nav.relative_yaw);

        if (!g_yaw_initialized)
        {
            // 首次满足条件，直接暴力初始化
            g_yaw_offset = raw_offset;
            g_yaw_initialized = 1;
        }
        else
        {
            // 后续使用低通滤波 (互补滤波) 平滑跳动
            // 取最短路径的差值，防止在 180/-180 处滤波崩溃
            float err = NormalizeAngle(raw_offset - g_yaw_offset);
            g_yaw_offset = NormalizeAngle(g_yaw_offset + YAW_FUSION_KP * err);
        }
    }
}

/**
 * @brief 获取当前融合后的绝对航向 (0=北, 90=东, CW正向)
 */
float GnssReplay_GetFusedYaw(void)
{
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
    g_yaw_initialized = 0;
    
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

    // 2. 检查是否未初始化航向 (防呆保护)
    // 如果还没校准出真北方向，小车直接打轮转向会迷失方向
    if (!g_yaw_initialized)
    {
        // 【自动校准阶段】强制小车直线慢速前进，以激活 Update_Fused_Yaw 的条件
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