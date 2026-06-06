#include "../nav_replay.h"
#include "../../../common.h"
#ifdef PLAN1_SPEED_PLANNING_FLASH_BACKEND
#include "../../ram2flash.h"
#else
#include "../../nav_replay_route_table.h"
#endif
#if (CURRENT_NAV_PLAN == 1) && ((NAV_PLAN1_METHOD == PLAN1_PURE_PURSUIT_SPEED_PLANNING) || defined(PLAN1_SPEED_PLANNING_FLASH_BACKEND))
extern volatile float target_speed_set;
extern volatile float err_degree;

/** @brief 回放状态机全局变量，由上层任务查询 */
NavReplayState_e g_replay_state = REPLAY_IDLE;
/** @brief 当前基准索引（单调前进） */
uint16 g_target_idx = 0;
/** @brief 当前目标点类型，供上层动作分发使用 */
uint8 g_current_point_type = NAV_POINT_PATH;
/** @brief 特殊动作触发标志，1 表示暂停轨迹跟踪并交由上层处理 */
uint8 g_special_action_trigger = 0;

#ifndef NAV_REPLAY_START_HEADING_VALID
#define NAV_REPLAY_START_HEADING_VALID 0
#endif

#ifndef NAV_REPLAY_START_HEADING_DEG
#define NAV_REPLAY_START_HEADING_DEG 0.0f
#endif

static uint8 g_start_heading_aligned = 1;

#if IMU_CATEGORY == 3
static uint8 s_start_heading_stable_count = 0;
#endif

#if CURRENT_NAV_PLAN == 1 || CURRENT_NAV_PLAN == 2
static void NavReplay_ResetProcessState(void);
#endif

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

#ifdef PLAN1_SPEED_PLANNING_FLASH_BACKEND

#ifndef NAV_FLASH_POINT_WORDS
#define NAV_FLASH_POINT_WORDS 3U
#endif

#ifndef NAV_FLASH_RAW_MAX_POINTS
#define NAV_FLASH_RAW_MAX_POINTS ((FLASH_PAGE_LENGTH - OFF_POINTS_START) / NAV_FLASH_POINT_WORDS)
#endif

#define NAV_FLASH_INPUT_MAX_POINTS  (NAV_FLASH_RAW_MAX_POINTS + 1U)
#define NAV_FLASH_FILLET_MAX_POINTS ((NAV_FLASH_INPUT_MAX_POINTS * NAV_FLASH_BEZIER_SAMPLES) + 2U)

typedef struct
{
    float x;
    float y;
} NavReplayPoint2f_t;

static NavRamPoint_t s_flash_raw_points[NAV_FLASH_INPUT_MAX_POINTS];
static NavReplayPoint2f_t s_flash_fillet_points[NAV_FLASH_FILLET_MAX_POINTS];
static float s_flash_arc_len[NAV_RAM_MAX_POINTS];
static float s_flash_speed_plan[NAV_RAM_MAX_POINTS];

static float NavReplay_Min3(float a, float b, float c)
{
    float ret = (a < b) ? a : b;
    return (ret < c) ? ret : c;
}

static uint8 NavReplay_ClampPointType(uint8 point_type)
{
    return (point_type > NAV_POINT_BUMP) ? NAV_POINT_BUMP : point_type;
}

static float NavReplay_NormalizeHeading360(float angle)
{
    while (angle >= 360.0f) angle -= 360.0f;
    while (angle < 0.0f) angle += 360.0f;
    return angle;
}

static float NavReplay_CalcPathYawDeg(float x0, float y0, float x1, float y1)
{
    return NormalizeAngle(-atan2f(y1 - y0, -(x1 - x0)) * 57.29578f);
}

static void NavReplay_InferRawAngles(NavRamPoint_t *points, uint16 count)
{
    uint16 i;

    for (i = 0; i < count; i++)
    {
        float yaw = 0.0f;

        if ((i + 1U) < count)
        {
            yaw = NavReplay_CalcPathYawDeg(points[i].x, points[i].y,
                                           points[i + 1U].x, points[i + 1U].y);
        }
        else if (i > 0U)
        {
            yaw = NavReplay_CalcPathYawDeg(points[i - 1U].x, points[i - 1U].y,
                                           points[i].x, points[i].y);
        }

        points[i].target_yaw_deg = yaw;
        points[i].heading_deg = NavReplay_NormalizeHeading360(points[i].heading_deg);
        points[i].point_type = NavReplay_ClampPointType(points[i].point_type);
        points[i].target_speed = 0.0f;
    }
}

static uint16 NavReplay_PrepareFlashInput(uint16 raw_count, uint16 *drop_first_count)
{
    uint16 i;

    *drop_first_count = 0U;
    if (raw_count == 0U)
    {
        return 0U;
    }

    if ((fabsf(nav_ram_data.points[0].x) <= 1.0e-6f) &&
        (fabsf(nav_ram_data.points[0].y) <= 1.0e-6f))
    {
        for (i = 0; i < raw_count; i++)
        {
            s_flash_raw_points[i] = nav_ram_data.points[i];
        }
        NavReplay_InferRawAngles(s_flash_raw_points, raw_count);
        return raw_count;
    }

    s_flash_raw_points[0].x = 0.0f;
    s_flash_raw_points[0].y = 0.0f;
    s_flash_raw_points[0].target_yaw_deg = 0.0f;
    s_flash_raw_points[0].heading_deg = 0.0f;
    s_flash_raw_points[0].point_type = NAV_POINT_PATH;
    s_flash_raw_points[0].target_speed = 0.0f;

    for (i = 0; i < raw_count; i++)
    {
        s_flash_raw_points[i + 1U] = nav_ram_data.points[i];
    }

    *drop_first_count = 1U;
    raw_count++;
    NavReplay_InferRawAngles(s_flash_raw_points, raw_count);
    return raw_count;
}

static float NavReplay_CalcAverageSpacing(const NavRamPoint_t *points, uint16 count)
{
    uint16 i;
    float total = 0.0f;

    if (count < 2U)
    {
        return NAV_FLASH_INTERPOLATE_DIST_MM;
    }

    for (i = 1U; i < count; i++)
    {
        total += CalcDistance(points[i - 1U].x, points[i - 1U].y, points[i].x, points[i].y);
    }

    return total / (float)(count - 1U);
}

static uint16 NavReplay_PushFilletPoint(uint16 count, float x, float y)
{
    if (count >= NAV_FLASH_FILLET_MAX_POINTS)
    {
        return count;
    }

    s_flash_fillet_points[count].x = x;
    s_flash_fillet_points[count].y = y;
    return (uint16)(count + 1U);
}

static uint16 NavReplay_BuildCornerFillet(const NavRamPoint_t *points, uint16 count)
{
    uint16 i;
    uint16 out_count = 0U;
    float fillet_radius = NavReplay_CalcAverageSpacing(points, count) * NAV_FLASH_FILLET_RADIUS_SCALE;

    if (count == 0U)
    {
        return 0U;
    }

    out_count = NavReplay_PushFilletPoint(out_count, points[0].x, points[0].y);

    for (i = 1U; (i + 1U) < count; i++)
    {
        float p0x = points[i - 1U].x;
        float p0y = points[i - 1U].y;
        float p1x = points[i].x;
        float p1y = points[i].y;
        float p2x = points[i + 1U].x;
        float p2y = points[i + 1U].y;
        float v1x = p0x - p1x;
        float v1y = p0y - p1y;
        float v2x = p2x - p1x;
        float v2y = p2y - p1y;
        float l1 = sqrtf(v1x * v1x + v1y * v1y);
        float l2 = sqrtf(v2x * v2x + v2y * v2y);
        uint16 sample;

        if ((l1 < 1.0e-3f) || (l2 < 1.0e-3f))
        {
            out_count = NavReplay_PushFilletPoint(out_count, p1x, p1y);
            continue;
        }

        {
            float cut_len = NavReplay_Min3(fillet_radius, l1 * 0.45f, l2 * 0.45f);
            float start_x = p1x + (v1x / l1) * cut_len;
            float start_y = p1y + (v1y / l1) * cut_len;
            float end_x = p1x + (v2x / l2) * cut_len;
            float end_y = p1y + (v2y / l2) * cut_len;

            for (sample = 0U; sample < NAV_FLASH_BEZIER_SAMPLES; sample++)
            {
                float t = (NAV_FLASH_BEZIER_SAMPLES <= 1U) ? 0.0f :
                          ((float)sample / (float)(NAV_FLASH_BEZIER_SAMPLES - 1U));
                float one_minus_t = 1.0f - t;
                float bx = one_minus_t * one_minus_t * start_x +
                           2.0f * one_minus_t * t * p1x +
                           t * t * end_x;
                float by = one_minus_t * one_minus_t * start_y +
                           2.0f * one_minus_t * t * p1y +
                           t * t * end_y;
                out_count = NavReplay_PushFilletPoint(out_count, bx, by);
            }
        }
    }

    out_count = NavReplay_PushFilletPoint(out_count, points[count - 1U].x, points[count - 1U].y);
    return out_count;
}

static uint16 NavReplay_ResampleFilletToRam(uint16 fillet_count)
{
    uint16 i;
    uint16 out_count;
    uint16 seg_idx = 1U;
    float total_len = 0.0f;
    float seg_start_dist = 0.0f;

    if (fillet_count == 0U)
    {
        nav_ram_data.point_count = 0U;
        return 0U;
    }

    for (i = 1U; i < fillet_count; i++)
    {
        total_len += CalcDistance(s_flash_fillet_points[i - 1U].x,
                                  s_flash_fillet_points[i - 1U].y,
                                  s_flash_fillet_points[i].x,
                                  s_flash_fillet_points[i].y);
    }

    if (total_len <= 1.0e-3f)
    {
        nav_ram_data.points[0].x = s_flash_fillet_points[0].x;
        nav_ram_data.points[0].y = s_flash_fillet_points[0].y;
        nav_ram_data.points[0].target_yaw_deg = 0.0f;
        nav_ram_data.points[0].heading_deg = 0.0f;
        nav_ram_data.points[0].point_type = NAV_POINT_PATH;
        nav_ram_data.points[0].target_speed = 0.0f;
        nav_ram_data.point_count = 1U;
        return 1U;
    }

    out_count = (uint16)(total_len / NAV_FLASH_INTERPOLATE_DIST_MM);
    if (out_count < 2U)
    {
        out_count = 2U;
    }
    if (out_count > NAV_RAM_MAX_POINTS)
    {
        out_count = NAV_RAM_MAX_POINTS;
    }

    for (i = 0U; i < out_count; i++)
    {
        float target_dist = (out_count <= 1U) ? 0.0f :
                            (total_len * (float)i / (float)(out_count - 1U));

        while ((seg_idx + 1U) < fillet_count)
        {
            float seg_len = CalcDistance(s_flash_fillet_points[seg_idx - 1U].x,
                                         s_flash_fillet_points[seg_idx - 1U].y,
                                         s_flash_fillet_points[seg_idx].x,
                                         s_flash_fillet_points[seg_idx].y);
            if ((seg_start_dist + seg_len) >= target_dist)
            {
                break;
            }
            seg_start_dist += seg_len;
            seg_idx++;
        }

        {
            float x0 = s_flash_fillet_points[seg_idx - 1U].x;
            float y0 = s_flash_fillet_points[seg_idx - 1U].y;
            float x1 = s_flash_fillet_points[seg_idx].x;
            float y1 = s_flash_fillet_points[seg_idx].y;
            float seg_len = CalcDistance(x0, y0, x1, y1);
            float ratio = (seg_len <= 1.0e-6f) ? 0.0f : ((target_dist - seg_start_dist) / seg_len);

            if (ratio < 0.0f) ratio = 0.0f;
            if (ratio > 1.0f) ratio = 1.0f;

            nav_ram_data.points[i].x = x0 + (x1 - x0) * ratio;
            nav_ram_data.points[i].y = y0 + (y1 - y0) * ratio;
            nav_ram_data.points[i].target_yaw_deg = 0.0f;
            nav_ram_data.points[i].heading_deg = 0.0f;
            nav_ram_data.points[i].point_type = NAV_POINT_PATH;
            nav_ram_data.points[i].target_speed = 0.0f;
        }
    }

    nav_ram_data.point_count = out_count;
    return out_count;
}

static uint16 NavReplay_FindClosestFinalPoint(float x, float y, uint16 final_count)
{
    uint16 i;
    uint16 closest_idx = 0U;
    float best_dist_sq = 3.4e38f;

    for (i = 0U; i < final_count; i++)
    {
        float dist_sq = CalcDistanceSq(x, y, nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        if (dist_sq < best_dist_sq)
        {
            best_dist_sq = dist_sq;
            closest_idx = i;
        }
    }

    return closest_idx;
}

static void NavReplay_SnapSpecialAndLast(const NavRamPoint_t *raw_points, uint16 raw_count, uint16 final_count)
{
    uint16 i;

    if ((raw_count == 0U) || (final_count == 0U))
    {
        return;
    }

    for (i = 0U; i < raw_count; i++)
    {
        if ((raw_points[i].point_type != NAV_POINT_PATH) || (i == (raw_count - 1U)))
        {
            uint16 closest_idx = NavReplay_FindClosestFinalPoint(raw_points[i].x, raw_points[i].y, final_count);
            nav_ram_data.points[closest_idx].x = raw_points[i].x;
            nav_ram_data.points[closest_idx].y = raw_points[i].y;
            nav_ram_data.points[closest_idx].point_type = raw_points[i].point_type;
            nav_ram_data.points[closest_idx].heading_deg = raw_points[i].heading_deg;
        }
    }
}

static void NavReplay_FillTangentYaws(uint16 final_count)
{
    uint16 i;

    for (i = 0U; i < final_count; i++)
    {
        if ((i + 1U) < final_count)
        {
            nav_ram_data.points[i].target_yaw_deg =
                NavReplay_CalcPathYawDeg(nav_ram_data.points[i].x,
                                          nav_ram_data.points[i].y,
                                          nav_ram_data.points[i + 1U].x,
                                          nav_ram_data.points[i + 1U].y);
        }
        else if (i > 0U)
        {
            nav_ram_data.points[i].target_yaw_deg =
                NavReplay_CalcPathYawDeg(nav_ram_data.points[i - 1U].x,
                                          nav_ram_data.points[i - 1U].y,
                                          nav_ram_data.points[i].x,
                                          nav_ram_data.points[i].y);
        }
        else
        {
            nav_ram_data.points[i].target_yaw_deg = 0.0f;
        }
    }
}

static void NavReplay_RestoreSpecialAngles(const NavRamPoint_t *raw_points, uint16 raw_count, uint16 final_count)
{
    uint16 i;

    if ((raw_count == 0U) || (final_count == 0U))
    {
        return;
    }

    for (i = 0U; i < raw_count; i++)
    {
        if ((raw_points[i].point_type != NAV_POINT_PATH) || (i == (raw_count - 1U)))
        {
            uint16 closest_idx = NavReplay_FindClosestFinalPoint(raw_points[i].x, raw_points[i].y, final_count);
            nav_ram_data.points[closest_idx].target_yaw_deg = raw_points[i].target_yaw_deg;
            nav_ram_data.points[closest_idx].heading_deg = raw_points[i].heading_deg;
        }
    }
}

static uint16 NavReplay_DropSyntheticOrigin(uint16 final_count, uint16 drop_first_count)
{
    uint16 i;

    if ((drop_first_count == 0U) || (final_count <= drop_first_count))
    {
        return final_count;
    }

    for (i = 0U; (i + drop_first_count) < final_count; i++)
    {
        nav_ram_data.points[i] = nav_ram_data.points[i + drop_first_count];
    }

    return (uint16)(final_count - drop_first_count);
}

static void NavReplay_BuildArcLength(uint16 count)
{
    uint16 i;

    if (count == 0U)
    {
        return;
    }

    s_flash_arc_len[0] = 0.0f;
    for (i = 1U; i < count; i++)
    {
        s_flash_arc_len[i] = s_flash_arc_len[i - 1U] +
            CalcDistance(nav_ram_data.points[i - 1U].x, nav_ram_data.points[i - 1U].y,
                         nav_ram_data.points[i].x, nav_ram_data.points[i].y);
    }
}

static float NavReplay_SignedCurvatureAt(uint16 idx, uint16 count)
{
    float ax;
    float ay;
    float bx;
    float by;
    float a;
    float b;
    float c;
    float denom;
    float cross;

    if (count < 3U)
    {
        return 0.0f;
    }

    if (idx == 0U)
    {
        idx = 1U;
    }
    else if (idx >= (count - 1U))
    {
        idx = (uint16)(count - 2U);
    }

    ax = nav_ram_data.points[idx].x - nav_ram_data.points[idx - 1U].x;
    ay = nav_ram_data.points[idx].y - nav_ram_data.points[idx - 1U].y;
    bx = nav_ram_data.points[idx + 1U].x - nav_ram_data.points[idx].x;
    by = nav_ram_data.points[idx + 1U].y - nav_ram_data.points[idx].y;
    a = sqrtf(ax * ax + ay * ay);
    b = sqrtf(bx * bx + by * by);
    c = CalcDistance(nav_ram_data.points[idx + 1U].x, nav_ram_data.points[idx + 1U].y,
                     nav_ram_data.points[idx - 1U].x, nav_ram_data.points[idx - 1U].y);
    denom = a * b * c;
    if (denom <= NAV_FLASH_CURVATURE_EPS)
    {
        return 0.0f;
    }

    cross = ax * by - ay * bx;
    return 2.0f * cross / denom;
}

static void NavReplay_ApplyOfflineSpeedPlan(uint16 count)
{
    uint16 i;

    if (count == 0U)
    {
        return;
    }

    NavReplay_BuildArcLength(count);

    for (i = 0U; i < count; i++)
    {
        float kappa = NavReplay_SignedCurvatureAt(i, count);
        float curve_limit = NAV_FLASH_PATH_SPEED_MAX_MM_S;

        if (fabsf(kappa) > NAV_FLASH_CURVATURE_EPS)
        {
            float denom = fabsf(kappa);
            if (denom < NAV_FLASH_CURVATURE_EPS)
            {
                denom = NAV_FLASH_CURVATURE_EPS;
            }
            curve_limit = sqrtf(NAV_FLASH_MAX_LATERAL_ACCEL_MM_S2 / denom);
        }

        s_flash_speed_plan[i] = (curve_limit < NAV_FLASH_PATH_SPEED_MAX_MM_S) ?
                                curve_limit : NAV_FLASH_PATH_SPEED_MAX_MM_S;
    }

    s_flash_speed_plan[count - 1U] = 0.0f;

    for (i = 0U; i < count; i++)
    {
        if (nav_ram_data.points[i].point_type == NAV_POINT_CIRCLE)
        {
            s_flash_speed_plan[i] = 0.0f;
        }
    }

    for (i = (uint16)(count - 1U); i > 0U; i--)
    {
        float ds = s_flash_arc_len[i] - s_flash_arc_len[i - 1U];
        float candidate = s_flash_speed_plan[i] * s_flash_speed_plan[i] +
                          2.0f * NAV_FLASH_MAX_DECEL_MM_S2 * ds;
        float max_entry = sqrtf((candidate > 0.0f) ? candidate : 0.0f);
        if (s_flash_speed_plan[i - 1U] > max_entry)
        {
            s_flash_speed_plan[i - 1U] = max_entry;
        }
    }

    for (i = 1U; i < count; i++)
    {
        float ds = s_flash_arc_len[i] - s_flash_arc_len[i - 1U];
        float candidate = s_flash_speed_plan[i - 1U] * s_flash_speed_plan[i - 1U] +
                          2.0f * NAV_FLASH_MAX_ACCEL_MM_S2 * ds;
        float max_exit = sqrtf((candidate > 0.0f) ? candidate : 0.0f);
        if (s_flash_speed_plan[i] > max_exit)
        {
            s_flash_speed_plan[i] = max_exit;
        }
    }

    for (i = 0U; i < count; i++)
    {
        nav_ram_data.points[i].target_speed = -s_flash_speed_plan[i] / NAV_FLASH_SPEED_TO_MM_S;
    }
}

uint16 NavReplay_BuildMethod4RouteFromRam(void)
{
    uint16 raw_count = nav_ram_data.point_count;
    uint16 input_count;
    uint16 drop_first_count = 0U;
    uint16 fillet_count;
    uint16 final_count;

    if (raw_count > NAV_FLASH_RAW_MAX_POINTS)
    {
        raw_count = NAV_FLASH_RAW_MAX_POINTS;
    }

    if (raw_count < NAV_FLASH_MIN_RAW_POINTS)
    {
        nav_ram_data.point_count = 0U;
        return 0U;
    }

    input_count = NavReplay_PrepareFlashInput(raw_count, &drop_first_count);
    if (input_count < NAV_FLASH_MIN_RAW_POINTS)
    {
        nav_ram_data.point_count = 0U;
        return 0U;
    }

    fillet_count = NavReplay_BuildCornerFillet(s_flash_raw_points, input_count);
    final_count = NavReplay_ResampleFilletToRam(fillet_count);
    NavReplay_SnapSpecialAndLast(s_flash_raw_points, input_count, final_count);
    NavReplay_FillTangentYaws(final_count);
    NavReplay_RestoreSpecialAngles(s_flash_raw_points, input_count, final_count);
    final_count = NavReplay_DropSyntheticOrigin(final_count, drop_first_count);
    nav_ram_data.point_count = final_count;
    nav_ram_data.plan_type = (uint8)CURRENT_NAV_PLAN;
    NavReplay_ApplyOfflineSpeedPlan(final_count);

#if DEBUG_LOG_ENABLE
    printf("[NavFlash] raw=%d input=%d fillet=%d final=%d\r\n",
           raw_count, input_count, fillet_count, final_count);
#endif

    return final_count;
}

#endif

/**
 * @brief 装载编译期路表到 RAM 数据结构
 * @return 实际装载的轨迹点数量
 * @note 主要由 NavReplay_Start() 调用
 */
uint16 NavReplay_LoadStaticRouteToRam(void)
{
#ifdef PLAN1_SPEED_PLANNING_FLASH_BACKEND
    if (NavFlash_ReadFlashToRam() != 0U)
    {
        nav_ram_data.point_count = 0U;
        return 0U;
    }

    return NavReplay_BuildMethod4RouteFromRam();
#else
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
#endif
}

/**
 * @brief 启动导航回放状态机
 * @note 典型调用入口：任务启动/遥控开始；会完成路表加载、状态清零和起跑对齐状态初始化
 */
void NavReplay_Start(void)
{
    #if GNSS_NAV == 1
        GpsNavReplay_Stop();//惯导的时候关闭gnss复现
    #endif
#if defined(PLAN1_SPEED_PLANNING_FLASH_BACKEND) || NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    NavReplay_LoadStaticRouteToRam();
#endif

    if (nav_ram_data.point_count == 0)
    {
#if DEBUG_LOG_ENABLE
        printf("[Nav] RAM is empty, cannot start replay.\r\n");
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

#if CURRENT_NAV_PLAN == 1 || CURRENT_NAV_PLAN == 2
    NavReplay_ResetProcessState();
#endif

#if CURRENT_NAV_PLAN == 3
    NavReplay_ResetPlan3State();
#endif

#if DEBUG_LOG_ENABLE
    printf("[Nav] Replay START. Plan: %d, Total Points: %d\r\n",
           nav_ram_data.plan_type, nav_ram_data.point_count);
#endif
}

/**
 * @brief 停止导航回放状态机并清空控制输出
 * @note 典型调用入口：任务停止/故障保护；会清零速度、角度和内部状态
 */
void NavReplay_Stop(void)
{
    target_speed_set = 0.0f;
    err_degree = 0.0f;
    g_replay_state = REPLAY_IDLE;
    g_special_action_trigger = 0;
    g_current_point_type = NAV_POINT_PATH;
    g_start_heading_aligned = 1;

#if IMU_CATEGORY == 3
    s_start_heading_stable_count = 0;
#endif

#if CURRENT_NAV_PLAN == 1 || CURRENT_NAV_PLAN == 2
    NavReplay_ResetProcessState();
#endif

#if CURRENT_NAV_PLAN == 3
    NavReplay_ResetPlan3State();
#endif

#if DEBUG_LOG_ENABLE
    printf("[Nav] Replay STOPPED.\r\n");
#endif
}

#if CURRENT_NAV_PLAN == 1 || CURRENT_NAV_PLAN == 2

static float s_prev_err_degree = 0.0f;
static float s_prev_speed_set = 0.0f;
/** @brief 特殊动作触发沿检测标记：1 表示上一周期进入接管态，等待恢复流程 */
static uint8 s_prev_trigger = 0;
/** @brief 近停点角度锁死状态：1 表示锁死已生效 */
static uint8 s_stop_lock_active = 0;
/** @brief 近停点锁存航向角（deg），锁死期间作为固定目标航向 */
static float s_stop_lock_yaw_deg = 0.0f;

#if IMU_CATEGORY == 3
/** @brief 起跑对齐阶段单周期最大航向纠偏输出（deg） */
#define NAV_START_ALIGN_MAX_ERR      25.0f
/** @brief 起跑航向连续稳定计数阈值（周期数） */
#define NAV_START_ALIGN_STABLE_COUNT 6U
#endif

/**
 * @brief 清除近停点角度锁死状态
 * @note 由启动/停止/特殊动作接管恢复/锁死条件失效路径调用
 */
static void NavReplay_ClearStopLock(void)
{
    s_stop_lock_active = 0;
    s_stop_lock_yaw_deg = 0.0f;
}

/**
 * @brief 复位 plan1/plan2 过程态缓存
 * @note 由 NavReplay_Start()/NavReplay_Stop() 调用；也在起跑姿态重置时调用
 */
static void NavReplay_ResetProcessState(void)
{
    s_prev_err_degree = 0.0f;
    s_prev_speed_set = 0.0f;
    s_prev_trigger = 0;
    NavReplay_ClearStopLock();
}

/**
 * @brief 目标速度分段限斜率
 * @param raw_speed 本周期路表/速度规划给出的原始目标速度，符号方向保持不变
 * @return 经过单周期步长限制后的目标速度
 * @note 代替单纯低通滤波：加速、普通减速、高速减速、跨零停车分别使用不同步长，
 *       调大 NAV_SPEED_SLEW_UP_* 会让起步/出弯提速更直接；
 *       调大 NAV_SPEED_SLEW_DOWN_* 会让弯前收速更快，但过大会更像急刹。
 */
static float NavReplay_SpeedSlew_Update(float raw_speed)
{
    float abs_raw = fabsf(raw_speed);
    float abs_prev = fabsf(s_prev_speed_set);
    float diff = raw_speed - s_prev_speed_set;
    float step_limit;

    // 加速段直接给目标速度，保留目标速度台阶，避免把加速前馈的触发条件抹平。
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
 * @brief 处理 IMU 起跑航向对齐流程
 * @return 1 对齐完成，0 仍在对齐中
 * @note 由 plan1/plan2 的 NavReplay_Process() 每周期调用
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
 * @brief 起跑前复位导航位姿与回放状态
 * @note 在起跑航向对齐完成后调用，避免历史位姿影响首段跟踪
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
 * @brief 搜索“停止屏障”索引
 * @param start_idx 起始索引
 * @param search_range 向前搜索范围（点数）
 * @return 首个停止屏障点索引（圆环点/零速点/终点）
 * @note 由最近点搜索与主循环前瞻截止逻辑调用，用于避免看穿停车点
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
 * @brief 单调最近点恢复（仅向前搜索）
 * @param current_idx 当前索引基准
 * @param search_range 搜索窗口（点数）
 * @param is_recovering 是否处于特殊动作恢复期
 * @return 更新后的最近点索引
 * @note 由 plan1/plan2 主循环调用，是轨迹跟踪索引推进的入口
 */
static int Find_Closest_Point_Index_Strict(int current_idx, int search_range, uint8 is_recovering)
{
    int i;
    int closest_idx = current_idx;
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

    if (!is_recovering && min_dist_sq > 800.0f * 800.0f)
    {
        return current_idx;
    }

    return closest_idx;
}

/**
 * @brief 在 [base_idx, stop_idx] 区间内查找纯追踪前瞻目标点
 * @param base_idx 基准索引
 * @param stop_idx 停止屏障索引
 * @param lookahead_dist 前瞻距离（mm）
 * @param tx 输出目标点 x（mm）
 * @param ty 输出目标点 y（mm）
 * @note 由 plan1/plan2 主循环调用，前瞻不会跨越停止屏障
 */
static void NavReplay_FindLookaheadTarget(uint16 base_idx, uint16 stop_idx, float lookahead_dist, float *tx, float *ty)
{
    uint16 i;
    float lookahead_dist_sq = lookahead_dist * lookahead_dist;

    *tx = nav_ram_data.points[stop_idx].x;
    *ty = nav_ram_data.points[stop_idx].y;

    for (i = base_idx; i <= stop_idx; i++)
    {
        float d_sq = CalcDistanceSq(inertial_nav.x, inertial_nav.y,
                                    nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        *tx = nav_ram_data.points[i].x;
        *ty = nav_ram_data.points[i].y;
        if (d_sq >= lookahead_dist_sq)
        {
            break;
        }
    }
}

/**
 * @brief 更新近停点角度锁死状态
 * @param stop_idx 当前停止屏障点索引
 * @param dist_to_stop 车体到停止屏障的距离（mm）
 * @note 由 plan1/plan2 主循环调用；满足“零速目标+近距离”时锁存当前航向
 */
static void NavReplay_UpdateStopLock(uint16 stop_idx, float dist_to_stop)
{
    if (stop_idx < nav_ram_data.point_count &&
        fabsf(nav_ram_data.points[stop_idx].target_speed) <= NAV_STOP_LOCK_SPEED_EPS &&
        dist_to_stop < NAV_STOP_LOCK_DIST_MM)
    {
        if (!s_stop_lock_active)
        {
            s_stop_lock_active = 1;
            s_stop_lock_yaw_deg = inertial_nav.relative_yaw;
        }
    }
    else
    {
        NavReplay_ClearStopLock();
    }
}

/**
 * @brief plan1/plan2 回放主流程（统一骨架）
 * @note 调用关系：由周期任务调用；内部串联“接管恢复→最近点恢复→停止屏障处理→纯追踪→速度低通”
 */
void NavReplay_Process(void)
{
    int scan_range;
    int base_idx;
    uint8 is_recovering = 0;
    uint16 stop_idx;
    uint16 last_idx;
    float dist_to_stop;
    float base_spacing = 0.0f;
    float lookahead_min = PP_LD_MIN_CURVE;
    float lookahead_dist;
    float tx;
    float ty;
    float raw_err_degree;
    float diff;
    float raw_speed;

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
        printf("[Nav] Start heading aligned, launch pose reset.\r\n");
#endif
        return;
    }
#endif

    /* 阶段1：特殊动作接管期，直接退出跟踪计算 */
    if (g_special_action_trigger == 1)
    {
        s_prev_trigger = 1;
        NavReplay_ClearStopLock();
        return;
    }

    /* 阶段2：接管恢复沿处理，恢复后先清空历史滤波状态 */
    if (s_prev_trigger == 1)
    {
        is_recovering = 1;
        s_prev_trigger = 0;
        s_prev_err_degree = 0.0f;
        s_prev_speed_set = 0.0f;
        NavReplay_ClearStopLock();
    }

    /* 阶段3：最近点恢复（恢复期放宽搜索窗口） */
    scan_range = is_recovering ? 300 : 80;
    base_idx = Find_Closest_Point_Index_Strict((int)g_target_idx, scan_range, is_recovering);
    g_target_idx = (uint16)base_idx;

    if (nav_ram_data.point_count == 0)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        return;
    }

    last_idx = (uint16)(nav_ram_data.point_count - 1U);
    /* 阶段4：搜索停止屏障，统一处理终点/圆环点/零速停车点 */
    stop_idx = NavReplay_FindStopBarrierIndex((uint16)base_idx, (uint16)(nav_ram_data.point_count - 1U - (uint16)base_idx));
    dist_to_stop = CalcDistance(inertial_nav.x, inertial_nav.y,
                                nav_ram_data.points[stop_idx].x, nav_ram_data.points[stop_idx].y);

    if (stop_idx == last_idx && dist_to_stop <= NAV_DIST_ARRIVE)
    {
        g_replay_state = REPLAY_FINISHED;
        g_target_idx = stop_idx;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        s_prev_speed_set = 0.0f;
        s_prev_err_degree = 0.0f;
        NavReplay_ClearStopLock();
        return;
    }

#if CURRENT_NAV_PLAN == 2
    if (nav_ram_data.points[stop_idx].point_type == NAV_POINT_CIRCLE &&
        dist_to_stop <= NAV_DIST_ARRIVE)
    {
        g_current_point_type = NAV_POINT_CIRCLE;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        s_prev_speed_set = 0.0f;
        s_prev_err_degree = 0.0f;
        NavReplay_ClearStopLock();
        minefield_flag = 1;
        g_special_action_trigger = 1;
        s_prev_trigger = 1;
        if (stop_idx < last_idx)
        {
            g_target_idx = (uint16)(stop_idx + 1U);
        }
        else
        {
            g_target_idx = stop_idx;
        }
        return;
    }
#endif

    /* 阶段5：更新近停点角度锁死状态 */
    NavReplay_UpdateStopLock(stop_idx, dist_to_stop);

    if (base_idx + 1 < nav_ram_data.point_count)
    {
        base_spacing = CalcDistance(nav_ram_data.points[base_idx].x, nav_ram_data.points[base_idx].y,
                                    nav_ram_data.points[base_idx + 1].x, nav_ram_data.points[base_idx + 1].y);
    }
    else if (base_idx > 0)
    {
        base_spacing = CalcDistance(nav_ram_data.points[base_idx - 1].x, nav_ram_data.points[base_idx - 1].y,
                                    nav_ram_data.points[base_idx].x, nav_ram_data.points[base_idx].y);
    }

    if (base_spacing * PP_LD_MIN_STRAIGHT > lookahead_min)
    {
        lookahead_min = base_spacing * PP_LD_MIN_STRAIGHT;
    }

    /* 阶段6：纯追踪选点（前瞻距离与速度相关，且不跨越停止屏障） */
    lookahead_dist = lookahead_min + fabsf(s_prev_speed_set) * PP_LD_SPEED_GAIN;
    NavReplay_FindLookaheadTarget((uint16)base_idx, stop_idx, lookahead_dist, &tx, &ty);

    if (s_stop_lock_active)
    {
        err_degree = NormalizeAngle(s_stop_lock_yaw_deg - inertial_nav.relative_yaw);
        s_prev_err_degree = err_degree;
    }
    else
    {
        float target_yaw = -atan2f(ty - inertial_nav.y, -(tx - inertial_nav.x)) * 57.29578f;
        raw_err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);
        diff = raw_err_degree - s_prev_err_degree;
        if (diff > SLEW_RATE_ANGLE) raw_err_degree = s_prev_err_degree + SLEW_RATE_ANGLE;
        else if (diff < -SLEW_RATE_ANGLE) raw_err_degree = s_prev_err_degree - SLEW_RATE_ANGLE;
        err_degree = FILTER_ALPHA_ANGLE * raw_err_degree + (1.0f - FILTER_ALPHA_ANGLE) * s_prev_err_degree;
        s_prev_err_degree = err_degree;
    }

    /* 阶段7：查表取速 + 单一低通；离线规划是速度唯一真源 */
    raw_speed = nav_ram_data.points[base_idx].target_speed;
    if (s_stop_lock_active && raw_speed > NAV_SPEED_STOP)
    {
        raw_speed = NAV_SPEED_STOP;
    }

    target_speed_set = NavReplay_SpeedSlew_Update(raw_speed);
    s_prev_speed_set = target_speed_set;
    g_current_point_type = nav_ram_data.points[base_idx].point_type;
}
#endif


#endif

/*这里注释了，保存的是原有的到一个点停一次的控制逻辑，仅仅能实现最基本的到达，但它的控制距离是精准的，逻辑是完备的，后面所有的代码都在其基础上进行优化和尝试*/
/*
void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING || g_special_action_trigger == 1) return;

#if IMU_CATEGORY == 3
    if (!g_start_heading_aligned)
    {
        float heading_err = NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);
        err_degree = heading_err;
        target_speed_set = NAV_SPEED_STOP;

        if (fabsf(heading_err) <= NAV_START_HEADING_TOLERANCE)
        {
            g_start_heading_aligned = 1;
            err_degree = 0.0f;
            #if DEBUG_LOG_ENABLE
            printf("[Nav] Start heading aligned: %.2f deg\r\n", heading);
            #endif
        }
        else
        {
            return;
        }
    }
#endif


    // 1. 检查是否跑完全部点位
    if (g_target_idx >= nav_ram_data.point_count)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        #if DEBUG_LOG_ENABLE
        printf("[Nav] Replay Finished.\r\n");
        #endif
        return;
    }

    // 2. 获取当前目标点数据
    float tx = nav_ram_data.points[g_target_idx].x;
    float ty = nav_ram_data.points[g_target_idx].y;
    g_current_point_type = nav_ram_data.points[g_target_idx].point_type;

    // 3. 计算距离和期望角度
    // 假设 inertial_nav 是全局结构体，x, y, relative_yaw 实时更新
    float dx = tx - inertial_nav.x;
    float dy = ty - inertial_nav.y;
    float dist = CalcDistance(inertial_nav.x, inertial_nav.y, tx, ty);

    // 计算期望方位角 (atan2 返回弧度值，转为角度)
    // 根据描述：X正方向向后，Y正方向向右，符合标准笛卡尔坐标旋转。
    float target_yaw = -atan2f(dy, -dx) * 57.29578f; 
    
    // err_degree = 期望 - 实际
    err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

    // 5. 控制策略：先转再走
    if (dist <= NAV_DIST_ARRIVE)
    {
        // --- A. 到达目标点 ---
        target_speed_set = NAV_SPEED_STOP;
        
        #if DEBUG_LOG_ENABLE
        printf("[Nav] Arrived Point[%d] Type[%d]\r\n", g_target_idx, g_current_point_type);
        #endif

        if (g_current_point_type != NAV_POINT_PATH)//处理特殊点
        {
             if (g_current_point_type == NAV_POINT_CIRCLE) {
                minefield_flag = 1;
            }
            g_special_action_trigger = 1;
        }
        
        g_target_idx++;
    }
    else
    {
        // --- B. 未到达目标点 ---
        // 先检查角度是否对准
        if (fabsf(NormalizeAngle(err_degree)) > NAV_YAW_TOLERANCE)
        {
            // 角度偏差较大，先原地旋转
            target_speed_set = NAV_SPEED_STOP;
            #if DEBUG_LOG_ENABLE
            printf("[Nav] Rotating to target, err: %.2f\r\n", err_degree);
            #endif
        }
        else
        {
            // 角度基本对准，开始移动
            if (dist > NAV_DIST_FAR)
            {
                // 远程段：满速行驶
                target_speed_set = NAV_SPEED_FAST;
            }
            else if (dist > NAV_DIST_NEAR)
            {
                // 减速段：线性插值减速
                float ratio = (dist - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR);
                target_speed_set = NAV_SPEED_SLOW + (NAV_SPEED_FAST - NAV_SPEED_SLOW) * ratio;
            }
            else
            {
                // 精准逼近段：极低速
                target_speed_set = NAV_SPEED_SLOW;
            }
        }
    }
}
*/
// 【使用说明】
//  // 惯导复现控制循环 (建议放在 20ms 定时器中)
//         // if (timer_20ms_flag) {
//             NavReplay_Process(); 
//         // }

//         // === 处理特殊点逻辑 ===
//         if (g_replay_state == REPLAY_RUNNING && g_special_action_trigger)
//         {
//             switch (g_current_point_type)
//             {
//                 case NAV_POINT_CIRCLE:
//                     // 暂停复现，执行转圈状态机
//                     // Run_Circle_Task();
//                     // 任务完成后清除标志
//                     break;
//                 case NAV_POINT_JUMP:
//                     // 只有在点类型为跳跃点时，可能需要加速冲过去
//                     // Override_Speed_For_Jump();
//                     break;
//                 // ... 其他类型
//             }
//         }
        
//         // 底层电机控制 (使用 target_speed_set 和 err_degree)
//         // Motor_Control(target_speed_set, err_degree);

