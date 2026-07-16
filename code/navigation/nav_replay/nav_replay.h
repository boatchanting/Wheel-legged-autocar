#ifndef _NAV_REPLAY_H_
#define _NAV_REPLAY_H_

#include "zf_common_headfile.h"
#include "nav_ram.h"
#include "nav_options.h"

// 全局通用的状态机枚举，外部依然可以直接使用
typedef enum
{
    REPLAY_IDLE,        // 回放空闲/停止
    REPLAY_RUNNING,     // 回放正在执行
    REPLAY_FINISHED     // 回放已到达终点
} NavReplayState_e;

#if GNSS_NAV == 1
extern NavReplayState_e g_gps_replay_state;
extern uint8 g_gps_current_point_type;
extern uint8 g_gps_special_action_trigger;

uint16 GpsNavReplay_LoadStaticRouteToRam(void);
void GpsNavReplay_Start(void);
void GpsNavReplay_Stop(void);
void GpsNavReplay_Process(void);
#endif

/* ========================================================
 * 动态路由与编译期防呆检查
 * 根据 sys_options.h 和 nav_options.h 的组合，仅拉取对应的唯一实现。
 * 包含严密的逻辑冲突检查，配置出错时直接打断编译。
 * ======================================================== */

#if CURRENT_NAV_PLAN == 1
    // ---------------- [ 科目一路由 ] ----------------
    #if NAV_PLAN1_METHOD == PLAN1_METHOD_GNSS
        // 【逻辑互斥检查】：如果选中 GNSS 寻迹方案，但系统总开关却没开GNSS，则阻断编译！
        #if GNSS_NAV != 1
            #error "[Nav Config Error] Plan 1 selected GNSS method, but GNSS_NAV is not set to 1 in sys_options.h! Please check config."
        #endif
        #include "plan1/plan1_gnss.h"
        
    #elif NAV_PLAN1_METHOD == PLAN1_PURE_PURSUIT_SPEED_PLANNING
        // 【资源浪费提醒】：如果选中惯导，但系统总开关依然开了 GNSS，弹出黄色警告提醒。
        #if GNSS_NAV == 1
            #warning "[Nav Warning] Global GNSS_NAV=1, but Plan 1 uses INS pure pursuit method! GNSS data is invalid for tracking, check for wasted CPU."
        #endif
        #include "plan1/plan1_pure_pursuit_speed_planning.h"

    #elif NAV_PLAN1_METHOD == PLAN1_PURE_PURSUIT
        // 【资源浪费提醒】：如果选中惯导，但系统总开关依然开了 GNSS，弹出黄色警告提醒。
        #if GNSS_NAV == 1
            #warning "[Nav Warning] Global GNSS_NAV=1, but Plan 1 uses INS pure pursuit method! GNSS data is invalid for tracking, check for wasted CPU."
        #endif
        #include "plan1/plan1_pure_pursuit.h"

    #elif NAV_PLAN1_METHOD == PLAN1_LQR_TRACKING
        // GNSS_NAV may be enabled here for GNSS telemetry/fusion visualization;
        // Plan1 LQR still uses the INS route replay for tracking.
        #include "plan1/plan1_lqr_tracking.h"
        
    #else
        // 【越界检查】：填了一个不存在的方案编号
        #error "[Nav Config Error] Plan 1 (CURRENT_NAV_PLAN == 1) has no valid NAV_PLAN1_METHOD selected!"
    #endif

#elif CURRENT_NAV_PLAN == 2
    // ---------------- [ 科目二路由 ] ----------------
    #if NAV_PLAN2_METHOD == PLAN2_PURE_PURSUIT_SPEED_PLANNING
        #include "plan2/plan2_pure_pursuit_speed_planning.h"
    #elif NAV_PLAN2_METHOD == PLAN2_PURE_PURSUIT
        #include "plan2/plan2_pure_pursuit.h"
    #elif NAV_PLAN2_METHOD == PLAN2_METHOD_PRECISE
        #include "plan2/plan2_precise.h"
    #elif NAV_PLAN2_METHOD == PLAN2_POINT_SPEED_PLANNING
        #include "plan2/plan2_point_speed_planning.h"
    #else
        // 【科目二越界检查】
        #error "[Nav Config Error] Plan 2 currently supports PLAN2_PURE_PURSUIT_SPEED_PLANNING, PLAN2_PURE_PURSUIT, PLAN2_METHOD_PRECISE and PLAN2_POINT_SPEED_PLANNING only."
    #endif

#elif CURRENT_NAV_PLAN == 3
    // ---------------- [ 科目三路由 ] ----------------
    #if NAV_PLAN3_METHOD == PLAN3_METHOD_PRECISE
        #include "plan3/plan3_precise.h"
    #else
        // 【越界检查】
        #error "[Nav Config Error] Plan 3 (CURRENT_NAV_PLAN == 3) has no valid NAV_PLAN3_METHOD selected!"
    #endif

#elif CURRENT_NAV_PLAN == 99
    // ---------------- [ 空白测试路由 ] ----------------
    // 保留数字99：供队员后续开发完全独立的新算法沙盒时使用，避免污染主线代码
    #include "template/nav_plan_template.h"

#else
    // ---------------- [ 全局科目越界检查 ] ----------------
    #error "[Nav Config Error] Invalid CURRENT_NAV_PLAN value in sys_options.h! Only 1, 2, 3 or 99 are supported."
#endif

#endif // _NAV_REPLAY_H_
