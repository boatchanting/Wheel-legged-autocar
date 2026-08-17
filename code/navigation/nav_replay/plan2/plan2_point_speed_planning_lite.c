#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"
#include "../../../plan/minefield.h"
#include "../../../calculate/pid-new.h"
#include "plan2_point_speed_planning_lite.h"
#include <math.h>

#if (CURRENT_NAV_PLAN == 2) && (NAV_PLAN2_METHOD == PLAN2_POINT_SPEED_PLANNING)

extern volatile float target_speed_set;
extern volatile float err_degree;

static uint16 g_target_idx = 0U;
NavReplayState_e g_replay_state = REPLAY_IDLE;
uint8 g_current_point_type = NAV_POINT_PATH;
uint8 g_special_action_trigger = 0U;

// Debug variables
volatile uint16 g_nav_point_spin_debug_idx = 0;
volatile float g_nav_point_spin_debug_current_yaw = 0.0f;
volatile float g_nav_point_spin_debug_exit_yaw = 0.0f;
volatile float g_nav_point_spin_debug_total_angle = 0.0f;
volatile float g_nav_point_spin_debug_direction = 0.0f;
volatile float g_nav_point_spin_debug_cw_total_angle = 0.0f;
volatile float g_nav_point_spin_debug_ccw_total_angle = 0.0f;
volatile uint16 g_nav_point_special_debug_target_idx = 0;
volatile float g_nav_point_special_debug_target_x = 0.0f;
volatile float g_nav_point_special_debug_target_y = 0.0f;
volatile float g_nav_point_special_debug_dist_mm = 0.0f;
volatile float g_nav_point_special_debug_brake_radius_mm = 0.0f;
volatile float g_nav_point_special_debug_speed_ref_mm_s = 0.0f;
volatile uint8 g_nav_point_special_debug_zero_brake_issued = 0;
volatile uint8 g_nav_point_special_debug_zero_brake_active = 0;

static float s_special_brake_dist_ratio = 0.6f; // 刹车修正系数
static uint8 s_special_zero_brake_issued = 0U;
static float s_prev_speed_cmd = 0.0f;
static uint8 s_dash_latched = 0U;
static float s_dash_target_x = 0.0f;
static float s_dash_yaw_deg = 0.0f;

static float NormalizeAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle <= -180.0f) angle += 360.0f;
    return angle;
}

static float CalcDistance(float x1, float y1, float x2, float y2)
{
    return sqrtf((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}

static float CalcBearingDeg(float x1, float y1, float x2, float y2)
{
    return -atan2f(y2 - y1, -(x2 - x1)) * 57.29578f;
}

static uint8 IsSpecialPointType(uint8 point_type)
{
    return (uint8)((point_type != NAV_POINT_PATH) && (point_type != NAV_POINT_FLYBY));
}

static float PositiveAngle360(float angle)
{
    while (angle < 0.0f) angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;
    return angle;
}

static void ConfigureSpinPlanForPoint_Lite(uint16 point_idx)
{
    uint16 next_idx = (uint16)(point_idx + 1U);
    float exit_yaw;
    
    if (next_idx < nav_ram_data.point_count)
    {
        exit_yaw = CalcBearingDeg(nav_ram_data.points[point_idx].x,
                                  nav_ram_data.points[point_idx].y,
                                  nav_ram_data.points[next_idx].x,
                                  nav_ram_data.points[next_idx].y);
    }
    else
    {
        exit_yaw = nav_ram_data.points[point_idx].target_yaw_deg;
    }

    float current_yaw = inertial_nav.relative_yaw;
    
    // 计算顺时针 (CW) 和逆时针 (CCW) 需要补齐的夹角
    float delta_cw = PositiveAngle360(current_yaw - exit_yaw);
    float delta_ccw = PositiveAngle360(exit_yaw - current_yaw);

    float total_cw = 720.0f + delta_cw;
    float total_ccw = 720.0f + delta_ccw;

    if (total_cw < 725.0f) total_cw = 725.0f;
    if (total_ccw < 725.0f) total_ccw = 725.0f;

    // 选择自转总角度较小的一侧
    float best_total_angle = total_cw;
    float best_spin_sign = 1.0f; // 1.0f = CW

    if (total_ccw < total_cw)
    {
        best_total_angle = total_ccw;
        best_spin_sign = -1.0f; // -1.0f = CCW
    }

    Minefield_SetSpinPlan(best_total_angle, exit_yaw, best_spin_sign);
}


// --------------------------- 核心简化版速度规划逻辑 ---------------------------

// 速度斜率限制器（简化版：仅限制最大步长）
static float NavReplay_SpeedSlew_Update_Lite(float raw_speed)
{
    float diff = raw_speed - s_prev_speed_cmd;
    float step_limit = NAV_POINT_SPEED_DECEL_STEP;

    if (diff > step_limit)
    {
        raw_speed = s_prev_speed_cmd + step_limit;
    }
    else if (diff < -step_limit)
    {
        raw_speed = s_prev_speed_cmd - step_limit;
    }

    s_prev_speed_cmd = raw_speed;
    return raw_speed;
}

// 底层对正：计算指向目标点的偏航误差
static void SelectDriveHeading(float point_yaw_deg, float *selected_err_deg, float *speed_sign)
{
    *selected_err_deg = NormalizeAngle(point_yaw_deg - inertial_nav.relative_yaw);
    *speed_sign = -1.0f; // 始终向车尾方向行驶（假设原框架负速度代表前进）
}

// 基于距离和对正角的常规降速
static float PlanSpeedAbsByDistance(float dist_mm, float stop_radius_mm, float yaw_err_deg)
{
    float remain = dist_mm - stop_radius_mm;
    float speed_abs;

    if (remain <= 0.0f)
    {
        return 0.0f;
    }

    speed_abs = sqrtf(2.0f * NAV_POINT_SPEED_DECEL_CMD2_PER_MM * remain);
    speed_abs = Float_Constrain(speed_abs, 0.0f, fabsf(NAV_POINT_SPEED_FAST));

    if (fabsf(yaw_err_deg) > NAV_POINT_YAW_SLOW_TOLERANCE)
    {
        speed_abs = 0.0f; // 角度误差太大，原地调整不发车
    }
    else if (fabsf(yaw_err_deg) > NAV_POINT_YAW_STOP_TOLERANCE)
    {
        speed_abs *= 0.35f; // 角度误差中等，限速行驶
    }

    return speed_abs;
}

// --------------------------- 核心特殊点刹车逻辑 ---------------------------

// 计算刹车触发半径
static float CalcSpecialBrakeRadius(float v_actual)
{
    float v_mmps = fabsf(v_actual);
    // 使用用户提供的新公式：stop_dist = (0.00025 * v^2 - 0.2877 * v + 887) * ratio
    float stop_dist = (0.00025f * v_mmps * v_mmps - 0.2877f * v_mmps + 887.0f) * s_special_brake_dist_ratio;
    
    // 确保刹车距离不会算成负数
    if (stop_dist < 0.0f) stop_dist = 0.0f;

    return NAV_POINT_SPECIAL_BRAKE_MARGIN_MM + stop_dist;//NAV_POINT_SPECIAL_EXECUTE_RADIUS 
}

// 处理特殊点的刹车和触发（精简版兜底策略）
static uint8 HandleSpecialPointStopAndTrigger_Lite(float dist_to_point)
{
    float v_actual = fabsf(inertial_nav.vx_body); // 严格使用实际速度

    // 1. 如果还没触发刹车状态，则动态计算并判断
    if (s_special_zero_brake_issued == 0U)
    {
        float brake_radius_mm = CalcSpecialBrakeRadius(v_actual);
        
        // 第一次满足刹车距离要求，LATCH！
        if (dist_to_point <= brake_radius_mm)
        {
            s_special_zero_brake_issued = 1U;
        }
    }

    // 2. 如果已经进入了刹车状态
    if (s_special_zero_brake_issued != 0U)
    {
        // 已经进入了执行圆
        if (dist_to_point <= NAV_POINT_SPECIAL_EXECUTE_RADIUS)
        {
            if (v_actual <= NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S)
            {
                // 完美！停稳，触发动作！
                g_special_action_trigger = 1U;
                if (IsSpecialPointType(g_current_point_type))
                {
                    ConfigureSpinPlanForPoint_Lite(g_target_idx);
                    minefield_flag = 1U;
                }
                target_speed_set = 0.0f;
                return 2U; // 返回 2U 表示触发动作，结束当前点导航
            }
            else
            {
                // 车速还太快，死死刹车
                target_speed_set = 0.0f; 
                s_prev_speed_cmd = 0.0f; // 绕过斜率限制器，硬给 0
                return 0U;
            }
        }
        else
        {
            // 还没进执行圆，但在刹车中
            if (v_actual <= NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S)
            {
                // 兜底逻辑：刹车过猛，速度已经掉到安全阈值以下了，给阈值的 1/2 蠕动进去，防止提前停死
                target_speed_set = - ((NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S / 2.0f) / SPEED_TO_MM_S); // 注意负号代表前进
                // 此时为了防止突变抬头，可以走简化版斜率，让 0.0 缓慢变为 1/2 阈值
                target_speed_set = NavReplay_SpeedSlew_Update_Lite(target_speed_set);
                return 0U;
            }
            else
            {
                // 车速还很快，继续硬刹
                target_speed_set = 0.0f;
                s_prev_speed_cmd = 0.0f; // 绕过斜率限制器，硬给 0
                return 0U;
            }
        }
    }

    return 1U; // 1U 表示未触发刹车，继续正常巡航
}


// --------------------------- 生命周期与主循环 ---------------------------

uint16 NavReplay_LoadStaticRouteToRam(void)
{
#if NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    uint16 load_count = NAV_REPLAY_STATIC_ROUTE_COUNT;
    if (load_count > NAV_RAM_MAX_POINTS)
    {
        load_count = NAV_RAM_MAX_POINTS;
    }
    nav_ram_data.plan_type = NAV_PLAN_2;
    nav_ram_data.point_count = load_count;
    for (uint16 i = 0; i < load_count; i++)
    {
        nav_ram_data.points[i] = nav_replay_static_route_points[i];
    }
    return load_count;
#else
    return nav_ram_data.point_count;
#endif
}

void NavReplay_Start(void)
{
#if NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    NavReplay_LoadStaticRouteToRam();
#endif
    if (nav_ram_data.point_count == 0U)
    {
        return;
    }
    g_target_idx = 0U;
    g_replay_state = REPLAY_RUNNING;
    g_current_point_type = NAV_POINT_PATH;
    g_special_action_trigger = 0U;
    target_speed_set = 0.0f;
    err_degree = 0.0f;
    s_prev_speed_cmd = 0.0f;
    s_special_zero_brake_issued = 0U;
    s_dash_latched = 0U;
    
    Minefield_Init();
    Brake_NavHardStop_Reset();
    Control_Profile_RequestMode(CONTROL_MODE_NORMAL);
}

void NavReplay_Stop(void)
{
    target_speed_set = 0.0f;
    err_degree = 0.0f;
    g_replay_state = REPLAY_IDLE;
    g_current_point_type = NAV_POINT_PATH;
    g_special_action_trigger = 0U;
    s_prev_speed_cmd = 0.0f;
    s_special_zero_brake_issued = 0U;
    s_dash_latched = 0U;
    
    Minefield_Init();
    Brake_NavHardStop_Reset();
    Control_Profile_RequestMode(CONTROL_MODE_NORMAL);
}

void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING || g_special_action_trigger != 0U)
    {
        return;
    }

    if (g_target_idx >= nav_ram_data.point_count)
    {
        // 终点冲刺阶段，不退出循迹以防止回正翻车
        if (s_dash_latched == 0U)
        {
            s_dash_target_x = -NAV_POINT_DASH_OVERRUN_MM;
            
            // 如果发车时是反向走的（惯导x为负），则冲刺目标应该是正向
            if (inertial_nav.x < 0.0f && s_dash_target_x < 0.0f)
            {
                s_dash_target_x = NAV_POINT_DASH_OVERRUN_MM;
            }
            else if (inertial_nav.x > 0.0f && s_dash_target_x > 0.0f) // 预防初始错位
            {
                s_dash_target_x = -NAV_POINT_DASH_OVERRUN_MM;
            }

            // 目标点在Y轴上的投影
            float target_y = inertial_nav.y;
            
            // 计算冲线航向角并锁死
            s_dash_yaw_deg = CalcBearingDeg(inertial_nav.x, inertial_nav.y, s_dash_target_x, target_y);
            s_dash_latched = 1U;
        }

        float selected_err_deg, speed_sign;
        SelectDriveHeading(s_dash_yaw_deg, &selected_err_deg, &speed_sign);
        err_degree = selected_err_deg;

        // 判断是否已经冲过界限
        uint8 is_finished = 0;
        if (s_dash_target_x < 0.0f && inertial_nav.x <= s_dash_target_x) is_finished = 1;
        if (s_dash_target_x > 0.0f && inertial_nav.x >= s_dash_target_x) is_finished = 1;

        if (is_finished)
        {
            // 越界后硬刹车并保持闭环不退出，使用锁死的航向防止翻转乱窜
            target_speed_set = 0.0f;
            s_prev_speed_cmd = 0.0f; 
            Control_Profile_RequestMode(CONTROL_MODE_NORMAL);
            return;
        }

        // 还在冲刺中，定速输出（不按距离减速，过线才刹车）
        float speed_mag = fabsf(NAV_POINT_SPEED_DASH);
        target_speed_set = NavReplay_SpeedSlew_Update_Lite(speed_sign * speed_mag);
        Control_Profile_RequestMode(CONTROL_MODE_NORMAL);
        return;
    }

    const NavRamPoint_t *point = &nav_ram_data.points[g_target_idx];
    uint8 point_type = point->point_type;
    g_current_point_type = point_type;
    float dist_to_point = CalcDistance(inertial_nav.x, inertial_nav.y, point->x, point->y);

    if (point_type == NAV_POINT_PATH && dist_to_point <= NAV_POINT_PATH_ARRIVE_RADIUS)
    {
        g_target_idx++;
        s_special_zero_brake_issued = 0U;
        return;
    }

    if (point_type == NAV_POINT_FLYBY && dist_to_point <= NAV_POINT_FLYBY_ARRIVE_RADIUS)
    {
        g_target_idx++;
        s_special_zero_brake_issued = 0U;
        return;
    }

    float point_yaw_deg = CalcBearingDeg(inertial_nav.x, inertial_nav.y, point->x, point->y);
    float selected_err_deg, speed_sign;
    SelectDriveHeading(point_yaw_deg, &selected_err_deg, &speed_sign);
    err_degree = selected_err_deg;

    if (IsSpecialPointType(point_type))
    {
        uint8 special_res = HandleSpecialPointStopAndTrigger_Lite(dist_to_point);
        if (special_res == 2U)
        {
            if (g_target_idx < nav_ram_data.point_count - 1U)
            {
                g_target_idx++;
                s_special_zero_brake_issued = 0U;
            }
            else
            {
                g_replay_state = REPLAY_FINISHED;
            }
            return;
        }
        else if (special_res == 0U)
        {
            Control_Profile_RequestMode(CONTROL_MODE_NORMAL);
            return;
        }
    }

    // 普通路径或飞越路径规划
    float stop_radius = IsSpecialPointType(point_type) ? NAV_POINT_SPECIAL_EXECUTE_RADIUS : NAV_POINT_PATH_ARRIVE_RADIUS;
    float speed_mag;
    
    if (point_type == NAV_POINT_FLYBY)
    {
        // 绕桩点：不按距离强行衰减速度，而是保持绕桩速度
        // 由于 point->target_speed 在之前配置里可能没有被很好地维护，这里可以直接硬塞宏定义，也可以用配置值
        speed_mag = fabsf(NAV_POINT_SPEED_FLYBY); 
    }
    else
    {
        speed_mag = PlanSpeedAbsByDistance(dist_to_point, stop_radius, selected_err_deg);
    }
    
    target_speed_set = NavReplay_SpeedSlew_Update_Lite(speed_sign * speed_mag);
    Control_Profile_RequestMode(CONTROL_MODE_NORMAL);
}

// Dummy implement to satisfy linking if original functions are referenced
uint8 NavReplay_SpecialPointZeroBrakeActive(void) { return s_special_zero_brake_issued; }
uint8 NavReplay_SpecialPointCrawlActive(void) { return 0; }
uint8 NavReplay_SpecialPointPrepZeroBrakeLatched(void) { return 0; }

#endif
