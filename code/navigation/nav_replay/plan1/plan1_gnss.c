#include "../nav_replay.h"
#include "../../../common.h"
#include "../../gps_nav_replay_route_table.h"
#include "../../gnss_transform.h"
#include "../../fusion_nav.h"
#include "../../../calculate/ekf.h"
#if (CURRENT_NAV_PLAN == 1) && (GNSS_NAV == 1) && (NAV_PLAN1_METHOD == PLAN1_METHOD_GNSS)
extern volatile float target_speed_set; extern volatile float err_degree;

/** @brief 回放状态机全局变量，由上层任务查询 */
NavReplayState_e g_replay_state = REPLAY_IDLE;
/** @brief 当前基准索引（单调前进） */
uint16 g_target_idx = 0;
/** @brief 当前目标点类型，供上层动作分发使用 */
uint8 g_current_point_type = NAV_POINT_PATH;
/** @brief 特殊动作触发标志，1 表示暂停轨迹跟踪并交由上层处理 */
uint8 g_special_action_trigger = 0;


NavReplayState_e g_gps_replay_state = REPLAY_IDLE;
uint8 g_gps_current_point_type = NAV_POINT_PATH;
uint8 g_gps_special_action_trigger = 0;

static uint16 g_gps_target_idx = 0;

static uint8 g_gyro_yaw_initialized = 0;
static float g_yaw_offset_deg = 0.0f;

/**
 * @brief 将角度归一化到 [-180, 180] 区间
 * @param angle 输入角度（deg）
 * @return 归一化后的角度（deg）
 * @note 由各 plan 的误差计算流程调用
 */
static float NormalizeAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

/**
 * @brief 计算两点欧氏距离
 * @param x1 点1 x 坐标（mm）
 * @param y1 点1 y 坐标（mm）
 * @param x2 点2 x 坐标（mm）
 * @param y2 点2 y 坐标（mm）
 * @return 两点距离（mm）
 * @note 由回放主循环中的到点判定/前瞻估计调用
 */
static float CalcDistance(float x1, float y1, float x2, float y2)
{
    return sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

/**
 * @brief 计算两点距离平方（避免频繁开方）
 * @param x1 点1 x 坐标（mm）
 * @param y1 点1 y 坐标（mm）
 * @param x2 点2 x 坐标（mm）
 * @param y2 点2 y 坐标（mm）
 * @return 距离平方（mm^2）
 * @note 由最近点搜索与前瞻搜索调用
 */
static float CalcDistanceSq(float x1, float y1, float x2, float y2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

static float GpsNormalizeCourse360(float angle)
{
    while (angle >= 360.0f) angle -= 360.0f;
    while (angle < 0.0f) angle += 360.0f;
    return angle;
}

// Bearing measured counter-clockwise from X-axis (Standard Math)
static float GpsCalcBearingDegFromNorth(float from_x, float from_y, float to_x, float to_y)
{
    float dx = to_x - from_x;
    float dy = to_y - from_y;
    // 使用标准 atan2f(dy, dx) 匹配打点坐标系
    return GpsNormalizeCourse360(atan2f(dy, dx) * 57.2957795f);
}

static float GpsNavCurrentXmm(void)
{
    return g_fuse_state.fuse_x;
}

static float GpsNavCurrentYmm(void)
{
    return g_fuse_state.fuse_y;
}

static float GpsNav_GetCurrentHeadingDeg(void)
{
    return GpsNormalizeCourse360(g_fuse_state.fuse_yaw + GPS_NAV_HEADING_OFFSET_DEG);
}

uint16 GpsNavReplay_LoadStaticRouteToRam(void)
{
#if GPS_NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    uint16 i = 0;
    uint16 load_count = GPS_NAV_REPLAY_STATIC_ROUTE_COUNT;
    if (load_count > NAV_RAM_MAX_POINTS)
    {
        load_count = NAV_RAM_MAX_POINTS;
    }
    nav_ram_data.plan_type = NAV_PLAN_1;
    nav_ram_data.point_count = load_count;
    for (i = 0U; i < load_count; i++)
    {
        nav_ram_data.points[i] = gps_nav_replay_static_route_points[i];
    }
    return load_count;
#else
    return nav_ram_data.point_count;
#endif
}

void GpsNavReplay_Start(void)
{
#if GPS_NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    GpsNavReplay_LoadStaticRouteToRam();
#endif

    if (nav_ram_data.point_count == 0U)
    {
#if DEBUG_LOG_ENABLE
        printf("[GPS-NAV] RAM is empty, cannot start replay.\r\n");
#endif
        return;
    }

    NavReplay_Stop();

    g_gps_target_idx = 0U;
    g_gps_special_action_trigger = 0U;
    g_gps_current_point_type = NAV_POINT_PATH;
    g_gps_replay_state = REPLAY_RUNNING;
    target_speed_set = GPS_NAV_SPEED_STOP;
    err_degree = 0.0f;
    
    g_gyro_yaw_initialized = 0U;

    // 起步时记录绝对航向并重置融合状态
#if ENABLE_DYNAMIC_HEADING
    g_track_base_yaw = g_startup_avg_heading; // 使用上电静置2秒期间的抗干扰均值作为发车基准航向
#else
    g_track_base_yaw = FIXED_BASE_YAW; // 使用预设好的固定朝向
#endif
    Fusion_Set_Origin();

#if DEBUG_LOG_ENABLE
    printf("[GPS-NAV] Replay START. Points: %d\r\n", nav_ram_data.point_count);
#endif
}

void GpsNavReplay_Stop(void)
{
    if (g_gps_replay_state == REPLAY_IDLE)
    {
        return;
    }

    target_speed_set = GPS_NAV_SPEED_STOP;
    err_degree = 0.0f;
    g_gps_replay_state = REPLAY_IDLE;
    g_gps_special_action_trigger = 0U;

#if DEBUG_LOG_ENABLE
    printf("[GPS-NAV] Replay STOPPED.\r\n");
#endif
}

void GpsNavReplay_Process(void)
{
    float cx = 0.0f, cy = 0.0f;
    float target_x = 0.0f, target_y = 0.0f;
    float target_bearing = 0.0f;
    float current_heading = 0.0f;
    float raw_err_degree = 0.0f;

    // 1. 状态与 GPS 有效性校验
    if (g_gps_replay_state != REPLAY_RUNNING) return;

    if (!gnss_trans.is_valid || !gnss_trans.is_origin_set || gnss.state != 1U || gnss.satellite_used < GPS_NAV_MIN_SAT_USED)
    {
        target_speed_set = GPS_NAV_SPEED_STOP;
        err_degree = 0.0f;
        return;
    }

    cx = GpsNavCurrentXmm();
    cy = GpsNavCurrentYmm();
    current_heading = GpsNav_GetCurrentHeadingDeg();

    // === 2. 终点防冲过头判定 ===
    if (g_gps_target_idx >= nav_ram_data.point_count - 1)
    {
        target_x = nav_ram_data.points[nav_ram_data.point_count - 1].x;
        target_y = nav_ram_data.points[nav_ram_data.point_count - 1].y;
        float dist_to_end = CalcDistance(cx, cy, target_x, target_y);
        
        uint8 is_crossed_finish = 0;
        if (nav_ram_data.point_count >= 2)
        {
            float px = nav_ram_data.points[nav_ram_data.point_count - 2].x;
            float py = nav_ram_data.points[nav_ram_data.point_count - 2].y;
            float v1_x = target_x - px;
            float v1_y = target_y - py;
            float v2_x = cx - px;
            float v2_y = cy - py;
            float dot = v1_x * v2_x + v1_y * v2_y;
            float len_sq = v1_x * v1_x + v1_y * v1_y;
            // 投影法：如果车子越过了倒数第二个点到终点的连线，说明冲线了
            if (len_sq > 0.001f && dot >= len_sq) is_crossed_finish = 1;
        }

        if (dist_to_end <= GPS_NAV_DIST_ARRIVE || is_crossed_finish)
        {
            g_gps_replay_state = REPLAY_FINISHED;
            target_speed_set = GPS_NAV_SPEED_STOP;
            err_degree = 0.0f;
            return;
        }
    }
    else
    {
        // === 3. 极简 Pure Pursuit：只寻目标，绝不回头 ===
        uint8 skips = 0;
        while (g_gps_target_idx < nav_ram_data.point_count - 1)
        {
            float check_tx = nav_ram_data.points[g_gps_target_idx].x;
            float check_ty = nav_ram_data.points[g_gps_target_idx].y;
            float dist_current = CalcDistance(cx, cy, check_tx, check_ty);

            uint8 is_passed = 0;
            if (g_gps_target_idx > 0)
            {
                float px = nav_ram_data.points[g_gps_target_idx - 1].x;
                float py = nav_ram_data.points[g_gps_target_idx - 1].y;
                float v1_x = check_tx - px;
                float v1_y = check_ty - py;
                float v2_x = cx - px;
                float v2_y = cy - py;
                float dot = v1_x * v2_x + v1_y * v2_y;
                float len_sq = v1_x * v1_x + v1_y * v1_y;
                // 投影法：判定是否在物理上跑过了这个点
                if (len_sq > 0.001f && dot >= len_sq) is_passed = 1;
            }

            // 核心逻辑：如果点在 2.5米 圈内，或者已经被甩在身后，立刻抛弃它！吃下一个！
            if (dist_current < GPS_NAV_LOOKAHEAD_DIST || is_passed)
            {
                g_gps_target_idx++;
                skips++;
                if (skips >= 3) break; // 防跨界锁：每周期最多吃 3 个点
            }
            else
            {
                // 找到前方的点了！直接锁定目标，不做任何画蛇添足的插值运算
                break;
            }
        }
        
        target_x = nav_ram_data.points[g_gps_target_idx].x;
        target_y = nav_ram_data.points[g_gps_target_idx].y;
    }

    // === 4. 计算航向误差 ===
    target_bearing = GpsCalcBearingDegFromNorth(cx, cy, target_x, target_y);
    raw_err_degree = NormalizeAngle(target_bearing - current_heading);
    
    // 核心抗噪 A：低通滤波
    static float s_filtered_err = 0.0f;
    float alpha = 0.25f; 
    if (g_gps_target_idx == 0 && target_speed_set == GPS_NAV_SPEED_STOP) {
        s_filtered_err = raw_err_degree; 
    } else {
        s_filtered_err = (1.0f - alpha) * s_filtered_err + alpha * raw_err_degree;
    }

    float final_err = s_filtered_err;

    // 核心抗噪 B：死区
    if (fabsf(final_err) < 2.0f) final_err = 0.0f;

    // 核心抗噪 C：暴力限幅
    if (final_err > 35.0f) final_err = 35.0f;
    if (final_err < -35.0f) final_err = -35.0f;

    err_degree = final_err; 
    float abs_err = fabsf(err_degree);

    // === 5. 动态速度控制（提高保底动力防卡死） ===
    float dist_to_target = CalcDistance(cx, cy, target_x, target_y);
    float base_speed;
    
    if (dist_to_target > GPS_NAV_DIST_NEAR && abs_err < 10.0f) {
        base_speed = GPS_NAV_SPEED_FAST; 
    } else {
        base_speed = GPS_NAV_SPEED_SLOW; 
    }

    // 弯道动态降速
    float speed_factor = 1.0f - (abs_err / 40.0f); 
    
    // 【重要修改】：将保底推力从 0.2 提高到了 0.4！
    // 防止大转弯时底盘输出的 PWM 太低，导致小车电机无力、原地震颤
    if (speed_factor < 0.40f) speed_factor = 0.40f; 

    target_speed_set = base_speed * speed_factor; 
}

//【优化点】惯导上层控制占位用
void NavReplay_Start(void){ g_replay_state = REPLAY_RUNNING; g_special_action_trigger = 0; }
void NavReplay_Stop(void){ g_replay_state = REPLAY_IDLE; g_special_action_trigger = 0; }
void NavReplay_Process(void){ (void)NormalizeAngle(0.0f); (void)CalcDistance(0,0,0,0); }


#endif
