#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"

#if (CURRENT_NAV_PLAN == 1) && (NAV_PLAN1_METHOD == PLAN1_LQR_TRACKING)

extern volatile float target_speed_set;
extern volatile float err_degree;

/* 回放对外状态变量：名称保持和其他 Plan1 方案一致，便于上层任务无感切换。 */
NavReplayState_e g_replay_state = REPLAY_IDLE;
uint16 g_target_idx = 0;
uint8 g_current_point_type = NAV_POINT_PATH;
uint8 g_special_action_trigger = 0;

#ifndef NAV_REPLAY_START_HEADING_VALID
#define NAV_REPLAY_START_HEADING_VALID 0
#endif

#ifndef NAV_REPLAY_START_HEADING_DEG
#define NAV_REPLAY_START_HEADING_DEG 0.0f
#endif

#define LQR_DEG_TO_RAD 0.0174532925f

/*
 * LQR 本周期参考量：
 *   x/y          ：车身当前位置投影到最近路径线段后的参考点
 *   yaw_deg      ：从预览点读取的路径切线航向 target_yaw_deg
 *   curvature    ：从预览点读取的离线路径曲率
 *   target_speed ：从最近点读取的离线速度规划值
 *   point_type   ：最近点类型，保留给上层特殊动作逻辑
 *   idx          ：预览参考点索引，主要用于调试观察
 */
typedef struct
{
    float x;
    float y;
    float yaw_deg;
    float curvature;
    float target_speed;
    uint8 point_type;
    uint16 idx;
} LqrReference_t;

static uint8 g_start_heading_aligned = 1;
static float s_prev_err_degree = 0.0f;
static float s_prev_speed_set = 0.0f;
static uint8 s_prev_trigger = 0;

#if IMU_CATEGORY == 3
static uint8 s_start_heading_stable_count = 0;
#define NAV_START_ALIGN_MAX_ERR      25.0f
#define NAV_START_ALIGN_STABLE_COUNT 6U
#endif

static void NavReplay_ResetProcessState(void);

/**
 * @brief 角度归一化到 [-180, 180] deg。
 * @param angle 原始角度，单位 deg。
 * @return 归一化后的角度，单位 deg。
 * @note 用于航向误差 e_psi，避免 179/-179 度跨界时突然跳变。
 */
static float NormalizeAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

/**
 * @brief 计算两点欧氏距离。
 * @return 距离，单位 mm。
 * @note 只在终点到达判断等低频位置使用，平方距离搜索用 CalcDistanceSq()。
 */
static float CalcDistance(float x1, float y1, float x2, float y2)
{
    return sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

/**
 * @brief 计算两点距离平方。
 * @return 距离平方，单位 mm^2。
 * @note 最近点搜索大量调用，用平方距离避免频繁开方。
 */
static float CalcDistanceSq(float x1, float y1, float x2, float y2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

/**
 * @brief 将编译期静态路径表装载到 nav_ram_data。
 * @return 实际装载点数。
 * @note 路径表每行 7 字段，NavRamPoint_t 最后一项 curvature 会一起复制。
 */
uint16 NavReplay_LoadStaticRouteToRam(void)
{
#if NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    uint16 i;
    uint16 load_count = NAV_REPLAY_STATIC_ROUTE_COUNT;

    if (load_count > NAV_RAM_MAX_POINTS)
    {
        load_count = NAV_RAM_MAX_POINTS;
    }

    nav_ram_data.plan_type = (uint8)CURRENT_NAV_PLAN;
    nav_ram_data.point_count = load_count;

    for (i = 0; i < load_count; i++)
    {
        nav_ram_data.points[i] = nav_replay_static_route_points[i];
    }

    return load_count;
#else
    return nav_ram_data.point_count;
#endif
}

/**
 * @brief 启动 Plan1 LQR 路径回放。
 * @note 完成静态路径装载、索引清零、状态机切到 REPLAY_RUNNING，并清空滤波/斜率历史。
 */
void NavReplay_Start(void)
{
#if GNSS_NAV == 1
    GpsNavReplay_Stop();
#endif

#if NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    NavReplay_LoadStaticRouteToRam();
#endif

    if (nav_ram_data.point_count == 0)
    {
#if DEBUG_LOG_ENABLE
        printf("[Nav-LQR] RAM is empty, cannot start replay.\r\n");
#endif
        return;
    }

    g_target_idx = 0;
    g_current_point_type = NAV_POINT_PATH;
    g_replay_state = REPLAY_RUNNING;
    g_special_action_trigger = 0;

#if IMU_CATEGORY == 3
    g_start_heading_aligned = (NAV_REPLAY_START_HEADING_VALID == 1) ? 0 : 1;
    s_start_heading_stable_count = 0;
#else
    g_start_heading_aligned = 1;
#endif

    NavReplay_ResetProcessState();

#if DEBUG_LOG_ENABLE
    printf("[Nav-LQR] Replay START. Plan: %d, Total Points: %d\r\n",
           nav_ram_data.plan_type, nav_ram_data.point_count);
#endif
}

/**
 * @brief 停止 Plan1 LQR 路径回放。
 * @note 清零速度、转向误差和过程状态。上层急停/任务结束时可直接调用。
 */
void NavReplay_Stop(void)
{
    target_speed_set = NAV_SPEED_STOP;
    err_degree = 0.0f;
    g_replay_state = REPLAY_IDLE;
    g_special_action_trigger = 0;
    g_current_point_type = NAV_POINT_PATH;
    g_start_heading_aligned = 1;

#if IMU_CATEGORY == 3
    s_start_heading_stable_count = 0;
#endif

    NavReplay_ResetProcessState();

#if DEBUG_LOG_ENABLE
    printf("[Nav-LQR] Replay STOPPED.\r\n");
#endif
}

/**
 * @brief 清空 LQR 跟踪过程状态。
 * @note 包括上一周期转向输出、速度输出、特殊动作恢复标志。
 */
static void NavReplay_ResetProcessState(void)
{
    s_prev_err_degree = 0.0f;
    s_prev_speed_set = 0.0f;
    s_prev_trigger = 0;
}

/**
 * @brief 速度指令斜率限制。
 * @param raw_speed 路径表给出的原始 target_speed，符号方向不改变。
 * @return 限制单周期变化量后的速度指令。
 * @note 保留纯追踪速度规划版的分段限斜率风格，避免速度目标突然跳变。
 */
static float NavReplay_SpeedSlew_Update(float raw_speed)
{
    float abs_raw = fabsf(raw_speed);
    float abs_prev = fabsf(s_prev_speed_set);
    float diff = raw_speed - s_prev_speed_set;
    float step_limit;

    if (((raw_speed * s_prev_speed_set) >= 0.0f) &&
        (abs_raw > (abs_prev + NAV_SPEED_SLEW_EPS)))
    {
        return raw_speed;
    }

    if ((raw_speed * s_prev_speed_set) < 0.0f)
    {
        step_limit = NAV_SPEED_SLEW_DOWN_CROSS_ZERO;
    }
    else if (abs_raw > (abs_prev + NAV_SPEED_SLEW_EPS))
    {
        step_limit = (abs_prev < NAV_SPEED_SLEW_LOW_SPEED_TH) ? NAV_SPEED_SLEW_UP_LOW : NAV_SPEED_SLEW_UP_NORMAL;
    }
    else if ((abs_raw + NAV_SPEED_SLEW_EPS) < abs_prev)
    {
        step_limit = (abs_prev > NAV_SPEED_SLEW_FAST_DECEL_TH) ? NAV_SPEED_SLEW_DOWN_FAST : NAV_SPEED_SLEW_DOWN_NORMAL;
    }
    else
    {
        step_limit = NAV_SPEED_SLEW_UP_NORMAL;
    }

    return s_prev_speed_set + Float_Constrain(diff, -step_limit, step_limit);
}

#if IMU_CATEGORY == 3
/**
 * @brief 处理起跑前航向对齐。
 * @return 1：对齐完成；0：仍在对齐中。
 * @note 只在 IMU_CATEGORY == 3 时启用，使用路径表头部生成的起跑航向配置。
 */
static uint8 NavReplay_HandleStartHeadingAlignment(void)
{
    float heading_err = NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);
    float heading_cmd = heading_err;

    if (heading_cmd > NAV_START_ALIGN_MAX_ERR) heading_cmd = NAV_START_ALIGN_MAX_ERR;
    if (heading_cmd < -NAV_START_ALIGN_MAX_ERR) heading_cmd = -NAV_START_ALIGN_MAX_ERR;

    err_degree = heading_cmd;
    target_speed_set = NAV_SPEED_STOP;

    if (fabsf(heading_err) <= NAV_START_HEADING_TOLERANCE)
    {
        if (s_start_heading_stable_count < NAV_START_ALIGN_STABLE_COUNT)
        {
            s_start_heading_stable_count++;
        }
    }
    else
    {
        s_start_heading_stable_count = 0;
    }

    if (s_start_heading_stable_count < NAV_START_ALIGN_STABLE_COUNT)
    {
        return 0;
    }

    g_start_heading_aligned = 1;
    s_start_heading_stable_count = 0;
    err_degree = 0.0f;
    target_speed_set = NAV_SPEED_STOP;
    return 1;
}

/**
 * @brief 起跑航向对齐完成后重置惯导起点。
 * @note 保持和旧 Plan1 方案一致，避免起跑前等待阶段累计的位置/速度影响首段跟踪。
 */
static void NavReplay_ResetLaunchPose(void)
{
    inertial_nav.x = 0.0f;
    inertial_nav.y = 0.0f;
    inertial_nav.vx_body = 0.0f;
    inertial_nav.vy_body = 0.0f;
    inertial_nav.slip_flag = 0;
    inertial_nav.relative_yaw = 0.0f;
    inertial_nav.init_yaw = euler_angle.yaw;

    g_target_idx = 0;
    g_current_point_type = NAV_POINT_PATH;
    g_special_action_trigger = 0;

    NavReplay_ResetProcessState();
    err_degree = 0.0f;
    target_speed_set = NAV_SPEED_STOP;
}
#endif

/**
 * @brief 查找前方最近的停车屏障点。
 * @param start_idx 搜索起点索引。
 * @param search_range 向前搜索窗口，单位：点数。
 * @return 圆环点、零速点或终点索引。
 * @note LQR 参考点不会越过停车屏障，防止到终点或特殊点附近还看穿过去。
 */
static uint16 NavReplay_FindStopBarrierIndex(uint16 start_idx, uint16 search_range)
{
    uint16 i;
    uint16 last_idx;
    uint16 end_idx;

    if (nav_ram_data.point_count == 0)
    {
        return 0;
    }

    last_idx = (uint16)(nav_ram_data.point_count - 1U);
    if (start_idx > last_idx)
    {
        start_idx = last_idx;
    }

    end_idx = start_idx + search_range;
    if (end_idx > last_idx)
    {
        end_idx = last_idx;
    }

    for (i = start_idx; i <= end_idx; i++)
    {
        const NavRamPoint_t *point = &nav_ram_data.points[i];
        if (point->point_type == NAV_POINT_CIRCLE)
        {
            return i;
        }
        if (fabsf(point->target_speed) <= NAV_STOP_LOCK_SPEED_EPS)
        {
            return i;
        }
        if (i == last_idx)
        {
            return i;
        }
    }

    return end_idx;
}

/**
 * @brief 单调向前最近点搜索。
 * @param current_idx 当前基准索引。
 * @param search_range 向前搜索窗口，单位：点数。
 * @param is_recovering 是否处于特殊动作结束后的恢复期。
 * @return 更新后的最近点索引。
 * @note 只允许索引向前搜索，不回头找点，用来压住掉头回程时的索引跳变。
 */
static int Find_Closest_Point_Index_Strict(int current_idx, int search_range, uint8 is_recovering)
{
    int i;
    int closest_idx;
    int end_idx;
    int barrier_idx;
    float min_dist_sq = 1e12f;

    if (nav_ram_data.point_count == 0)
    {
        return 0;
    }

    if (current_idx < 0)
    {
        current_idx = 0;
    }
    if (current_idx >= nav_ram_data.point_count)
    {
        current_idx = nav_ram_data.point_count - 1;
    }

    closest_idx = current_idx;
    end_idx = current_idx + search_range;
    if (end_idx >= nav_ram_data.point_count)
    {
        end_idx = nav_ram_data.point_count - 1;
    }

    barrier_idx = (int)NavReplay_FindStopBarrierIndex((uint16)current_idx, (uint16)search_range);
    if (barrier_idx < end_idx)
    {
        end_idx = barrier_idx;
    }

    for (i = current_idx; i <= end_idx; i++)
    {
        float d_sq = CalcDistanceSq(inertial_nav.x, inertial_nav.y,
                                    nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        if (d_sq < min_dist_sq)
        {
            min_dist_sq = d_sq;
            closest_idx = i;
        }
    }

    if (!is_recovering && min_dist_sq > (LQR_MAX_TRACK_DIST_MM * LQR_MAX_TRACK_DIST_MM))
    {
        return current_idx;
    }

    return closest_idx;
}

/**
 * @brief 根据最近点和停车屏障选择预览点。
 * @param base_idx 最近路径点索引。
 * @param stop_idx 前方停车屏障索引。
 * @return 预览点索引。
 * @note 预览点最多向前 LQR_PREVIEW_POINTS 个点，且不会越过 stop_idx。
 */
static uint16 NavReplay_GetPreviewIndex(uint16 base_idx, uint16 stop_idx)
{
    uint16 last_idx;
    uint16 ref_idx;

    if (nav_ram_data.point_count == 0)
    {
        return 0;
    }

    last_idx = (uint16)(nav_ram_data.point_count - 1U);
    if (base_idx > last_idx)
    {
        base_idx = last_idx;
    }
    if (stop_idx < base_idx)
    {
        stop_idx = base_idx;
    }
    if (stop_idx > last_idx)
    {
        stop_idx = last_idx;
    }

    ref_idx = (uint16)(base_idx + LQR_PREVIEW_POINTS);
    if (ref_idx > stop_idx)
    {
        ref_idx = stop_idx;
    }
    if (ref_idx > last_idx)
    {
        ref_idx = last_idx;
    }

    return ref_idx;
}

/**
 * @brief 构造本周期 LQR 参考量。
 * @param base_idx 最近点索引。
 * @param stop_idx 前方停车屏障索引。
 * @param ref 输出参考量结构体。
 * @note x/y 使用车当前位置到最近线段的投影点，yaw/curvature 使用预览点，速度使用最近点。
 */
static void NavReplay_BuildReference(uint16 base_idx, uint16 stop_idx, LqrReference_t *ref)
{
    uint16 last_idx = (uint16)(nav_ram_data.point_count - 1U);
    uint16 seg_end_idx;
    uint16 ref_idx;
    const NavRamPoint_t *base;
    const NavRamPoint_t *seg_end;
    const NavRamPoint_t *preview;
    float seg_dx;
    float seg_dy;
    float seg_len_sq;
    float proj_t = 0.0f;

    if (base_idx > last_idx)
    {
        base_idx = last_idx;
    }
    if (stop_idx < base_idx)
    {
        stop_idx = base_idx;
    }

    seg_end_idx = base_idx;
    if ((base_idx < stop_idx) && ((uint16)(base_idx + 1U) <= last_idx))
    {
        seg_end_idx = (uint16)(base_idx + 1U);
    }

    ref_idx = NavReplay_GetPreviewIndex(base_idx, stop_idx);
    base = &nav_ram_data.points[base_idx];
    seg_end = &nav_ram_data.points[seg_end_idx];
    preview = &nav_ram_data.points[ref_idx];

    seg_dx = seg_end->x - base->x;
    seg_dy = seg_end->y - base->y;
    seg_len_sq = seg_dx * seg_dx + seg_dy * seg_dy;

    if (seg_len_sq > (LQR_PROJECTION_MIN_SEG_LEN_MM * LQR_PROJECTION_MIN_SEG_LEN_MM))
    {
        float car_dx = inertial_nav.x - base->x;
        float car_dy = inertial_nav.y - base->y;
        proj_t = (car_dx * seg_dx + car_dy * seg_dy) / seg_len_sq;
        proj_t = Float_Constrain(proj_t, 0.0f, 1.0f);
    }

    ref->x = base->x + proj_t * seg_dx;
    ref->y = base->y + proj_t * seg_dy;
    ref->yaw_deg = preview->target_yaw_deg;
    ref->curvature = preview->curvature;
    ref->target_speed = base->target_speed;
    ref->point_type = base->point_type;
    ref->idx = ref_idx;
}

/**
 * @brief 根据速度方向得到曲率前馈符号。
 * @param target_speed 路径表速度指令。
 * @return 曲率符号修正系数，通常为 1 或 -1。
 * @note 本车默认 target_speed < 0 为前进；如果后续速度方向约定变了，只改头文件宏。
 */
static float NavReplay_GetCurvatureDirectionSign(float target_speed)
{
#if LQR_CURVATURE_SPEED_SIGN_ENABLE
    if (fabsf(target_speed) <= NAV_STOP_LOCK_SPEED_EPS)
    {
        return 1.0f;
    }
#if LQR_FORWARD_SPEED_IS_NEGATIVE
    return (target_speed < 0.0f) ? 1.0f : -1.0f;
#else
    return (target_speed >= 0.0f) ? 1.0f : -1.0f;
#endif
#else
    (void)target_speed;
    return 1.0f;
#endif
}

/**
 * @brief 计算简化 LQR 转向误差输出。
 * @param ref 本周期参考量。
 * @return 最终 err_degree，已经过限幅、变化率限制和低通滤波。
 * @note e_y 为横向误差，e_psi 为路径切线航向误差，curvature 为离线曲率前馈。
 */
static float NavReplay_CalcLqrErr(const LqrReference_t *ref)
{
    float psi_ref_rad = ref->yaw_deg * LQR_DEG_TO_RAD;
    float dx = inertial_nav.x - ref->x;
    float dy = inertial_nav.y - ref->y;
    float e_y = -sinf(psi_ref_rad) * dx + cosf(psi_ref_rad) * dy;
    float e_psi = NormalizeAngle(ref->yaw_deg - inertial_nav.relative_yaw);
    float curv_sign = NavReplay_GetCurvatureDirectionSign(ref->target_speed);
    float curv_ff = LQR_K_CURV * ref->curvature * curv_sign;
    float err_raw;
    float diff;

    e_y = Float_Constrain(e_y, -LQR_LATERAL_ERR_LIMIT_MM, LQR_LATERAL_ERR_LIMIT_MM);

    err_raw = LQR_SIGN * (curv_ff + LQR_K_LATERAL * e_y + LQR_K_HEADING * e_psi);
    err_raw = Float_Constrain(err_raw, -LQR_ERR_MAX_DEG, LQR_ERR_MAX_DEG);

    diff = err_raw - s_prev_err_degree;
    err_raw = s_prev_err_degree + Float_Constrain(diff, -LQR_ERR_SLEW_DEG, LQR_ERR_SLEW_DEG);

    return LQR_FILTER_ALPHA * err_raw + (1.0f - LQR_FILTER_ALPHA) * s_prev_err_degree;
}

/**
 * @brief Plan1 LQR 主循环。
 * @note 周期调用流程：
 *       1. 处理起跑航向对齐；
 *       2. 处理特殊动作接管和恢复；
 *       3. 单调更新最近点索引；
 *       4. 查找停车屏障和终点；
 *       5. 构造投影/预览参考点；
 *       6. 输出 err_degree 和 target_speed_set。
 */
void NavReplay_Process(void)
{
    int scan_range;
    int base_idx;
    uint8 is_recovering = 0;
    uint16 stop_idx;
    uint16 last_idx;
    float dist_to_stop;
    float raw_speed;
    LqrReference_t ref;

    if (g_replay_state != REPLAY_RUNNING)
    {
        return;
    }

#if IMU_CATEGORY == 3
    if (!g_start_heading_aligned)
    {
        if (!NavReplay_HandleStartHeadingAlignment())
        {
            return;
        }

        NavReplay_ResetLaunchPose();

#if DEBUG_LOG_ENABLE
        printf("[Nav-LQR] Start heading aligned, launch pose reset.\r\n");
#endif
        return;
    }
#endif

    if (g_special_action_trigger == 1)
    {
        s_prev_trigger = 1;
        return;
    }

    if (s_prev_trigger == 1)
    {
        is_recovering = 1;
        s_prev_trigger = 0;
        s_prev_err_degree = 0.0f;
        s_prev_speed_set = 0.0f;
    }

    if (nav_ram_data.point_count == 0)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        return;
    }

    scan_range = is_recovering ? (int)LQR_SEARCH_RANGE_RECOVER : (int)LQR_SEARCH_RANGE_NORMAL;
    base_idx = Find_Closest_Point_Index_Strict((int)g_target_idx, scan_range, is_recovering);
    g_target_idx = (uint16)base_idx;

    last_idx = (uint16)(nav_ram_data.point_count - 1U);
    stop_idx = NavReplay_FindStopBarrierIndex((uint16)base_idx, (uint16)(last_idx - (uint16)base_idx));
    dist_to_stop = CalcDistance(inertial_nav.x, inertial_nav.y,
                                nav_ram_data.points[stop_idx].x, nav_ram_data.points[stop_idx].y);

    if ((stop_idx == last_idx) && (dist_to_stop <= NAV_DIST_ARRIVE))
    {
        g_replay_state = REPLAY_FINISHED;
        g_target_idx = stop_idx;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        s_prev_speed_set = 0.0f;
        s_prev_err_degree = 0.0f;
        return;
    }

    NavReplay_BuildReference((uint16)base_idx, stop_idx, &ref);

    err_degree = NavReplay_CalcLqrErr(&ref);
    s_prev_err_degree = err_degree;

    raw_speed = ref.target_speed;
    target_speed_set = NavReplay_SpeedSlew_Update(raw_speed);
    s_prev_speed_set = target_speed_set;
    g_current_point_type = ref.point_type;
}

#endif
