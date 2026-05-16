#include "nav_replay.h"
#include "../common.h"
#include "nav_replay_route_table.h"
#include "gps_nav_replay_route_table.h"
#include "gnss_transform.h"
#include "fused_nav.h"
#include "../config/sys_options.h"
#include "vision/vision_bridge_control.h"

// ========================= 鍐呴儴鍙橀噺 =========================
NavReplayState_e g_replay_state = REPLAY_IDLE;
uint16 g_target_idx = 0;                    // 褰撳墠姝ｅ湪鍓嶅線鐨勭偣绱㈠紩
uint8 g_current_point_type = NAV_POINT_PATH;// 褰撳墠鐐圭殑绫诲瀷
uint8 g_special_action_trigger = 0;         // 瑙﹀彂鏍囧織

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

#if CURRENT_NAV_PLAN == 1
static void NavReplay_ResetProcessState(void);
#endif

// ========================= 杈呭姪鍑芥暟 =========================

/**
 * @brief  瑙掑害褰掍竴鍖?(-180 ~ 180)
 * @param  angle 鍘熷瑙掑害
 * @return 褰掍竴鍖栧悗鐨勮搴?
 */
static float NormalizeAngle(float angle)
{
    while (angle > 180.0f)  angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

/**
 * @brief  璁＄畻涓ょ偣闂磋窛绂?
 */
static float CalcDistance(float x1, float y1, float x2, float y2)
{
    return sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

static float NavReplay_CurrentX(void)
{
#if GNSS_NAV == 2
    return fused_nav.fused_x;
#else
    return inertial_nav.x;
#endif
}

static float NavReplay_CurrentY(void)
{
#if GNSS_NAV == 2
    return fused_nav.fused_y;
#else
    return inertial_nav.y;
#endif
}

static float NavReplay_CurrentYaw(void)
{
#if GNSS_NAV == 2
    return fused_nav.fused_yaw;
#else
    return inertial_nav.relative_yaw;
#endif
}

// ========================= 鎺ュ彛瀹炵幇 =========================

uint16 NavReplay_LoadStaticRouteToRam(void)
{
#if NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    uint16 load_count = 0U;
#if GNSS_NAV == 2
    load_count = GPS_NAV_REPLAY_STATIC_ROUTE_COUNT;
#else
    load_count = NAV_REPLAY_STATIC_ROUTE_COUNT;
#endif
    if (load_count > NAV_RAM_MAX_POINTS)
    {
        load_count = NAV_RAM_MAX_POINTS;
    }
    nav_ram_data.plan_type = NAV_PLAN_1;
    nav_ram_data.point_count = load_count;
    for (uint16 i = 0; i < load_count; i++)
    {
#if GNSS_NAV == 2
        nav_ram_data.points[i] = gps_nav_replay_static_route_points[i];
#else
        nav_ram_data.points[i] = nav_replay_static_route_points[i];
#endif
    }
    return load_count;
#else
    return nav_ram_data.point_count;
#endif
}

void NavReplay_Start(void)
{
    #if GNSS_NAV == 1
        GpsNavReplay_Stop();//鎯鐨勬椂鍊欏叧闂璯nss澶嶇幇
    #endif
#if NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    NavReplay_LoadStaticRouteToRam();
#endif

    if (nav_ram_data.point_count == 0)
    {
        #if DEBUG_LOG_ENABLE
        printf("[Nav] RAM is empty, cannot start replay.\r\n");
        #endif
        return;
    }

    g_target_idx = 0; // 浠庣1涓偣寮€濮嬶紙璧峰鐐规病鏈夊偍瀛橈紝榛樿涓?0,0)锛?
    g_replay_state = REPLAY_RUNNING;
    g_special_action_trigger = 0;
#if IMU_CATEGORY == 3
    g_start_heading_aligned = (NAV_REPLAY_START_HEADING_VALID == 1) ? 0 : 1;
    s_start_heading_stable_count = 0;
#else
    g_start_heading_aligned = 1;
#endif

#if CURRENT_NAV_PLAN == 1
    NavReplay_ResetProcessState();
#endif
    
    #if DEBUG_LOG_ENABLE
    printf("[Nav] Replay START. Plan: %d, Total Points: %d\r\n", 
           nav_ram_data.plan_type, nav_ram_data.point_count);
    #endif
}

void NavReplay_Stop(void)
{
    target_speed_set = 0.0f;
    g_replay_state = REPLAY_IDLE;
    err_degree = 0.0f;
    g_special_action_trigger = 0;
    g_start_heading_aligned = 1;
#if IMU_CATEGORY == 3
    s_start_heading_stable_count = 0;
#endif

#if CURRENT_NAV_PLAN == 1
    NavReplay_ResetProcessState();
#endif
    
    #if DEBUG_LOG_ENABLE
    printf("[Nav] Replay STOPPED.\r\n");
    #endif
}

#if CURRENT_NAV_PLAN == 1 //濡傛灉鏄鐩竴锛屼粎浠呴渶瑕佺洿绾胯椹跺嵆鍙紝杩欐鏆傛椂涓嶅仛鐗硅皟鐨勬儏鍐典笅锛屼笉闇€瑕佺姸鎬佹満鍒囨崲

// 澶栭儴/闈欐€佸彉閲忓０鏄?
static float prev_err_degree = 0.0f;
static float prev_speed_set = 0.0f;
static float prev_curve_f = 0.0f;

#if IMU_CATEGORY == 3
#define NAV_START_ALIGN_MAX_ERR      25.0f
#define NAV_START_ALIGN_STABLE_COUNT 6U
#endif

// 楂樻晥骞虫柟璺濈璁＄畻
static inline float CalcDistanceSq(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

/**
 * @brief 涓ユ牸鍗曞悜绱㈠紩杩借釜
 * 寮哄埗瑕佹眰绱㈠紩鍙兘鍦ㄥ綋鍓嶄綅缃線鍚?[0, search_range] 鑼冨洿鍐呭鎵俱€?
 * 褰诲簳瑙ｅ喅鍦ㄥ師璺姌杩旇建杩逛腑锛岀储寮曡烦鍒板洖绋嬭矾寰勭殑闂銆?
 * 涓ユ牸鍗曞悜绱㈠紩杩借釜 (甯﹂槻绌挎ā閿?+ 鏀寔澶ц寖鍥撮噸瀹氫綅)
 * 寮哄埗瑕佹眰绱㈠紩鍙兘鍦ㄥ綋鍓嶄綅缃線鍚庡鎵撅紝涓旂粷涓嶅厑璁歌烦杩囩壒娈婄偣锛?
 */
static int Find_Closest_Point_Index_Strict(int current_idx, int search_range, uint8 is_recovering)
{
    int closest_idx = current_idx;
    float min_dist_sq = 1e9f; 

    int end_idx = current_idx + search_range;
    if (end_idx >= nav_ram_data.point_count) {
        end_idx = nav_ram_data.point_count - 1;
    }

    // 鍙線鍚庢悳锛屼笉鍥炲ご
    for (int i = current_idx; i <= end_idx; i++) {
        float d_sq = CalcDistanceSq(NavReplay_CurrentX(), NavReplay_CurrentY(),
                                    nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        if (d_sq < min_dist_sq) {
            min_dist_sq = d_sq;
            closest_idx = i;
        }
        
        // 銆愭牳蹇冧慨澶嶃€戯細鍙鎵弿閬囧埌鐗规畩鐐癸紝蹇呴』绔嬪埢缁堟锛?/ 杩欎釜绉戠洰涓€涓嶉渶瑕?
        // 鍝€?current_idx 鑷繁灏辨槸鐗规畩鐐癸紝涔熺粷涓嶅厑璁稿啀寰€鍚庢悳锛佹姝诲喕缁撶储寮曪紒
        // if (nav_ram_data.points[i].point_type != NAV_POINT_PATH) {
        //     if (closest_idx > i) closest_idx = i;
        //     break; 
        // }
    }
    
    // 涓綅淇濇姢锛氬鏋滄槸閲嶅畾浣嶇姸鎬侊紝璞佸厤 800mm 闄愬埗锛佸厑璁歌溅瀛愪粠杩滃寮鸿鍒囧洖涓昏矾
    if (!is_recovering && min_dist_sq > 800.0f * 800.0f) {
        return current_idx; 
    }
    return closest_idx;
}

/**
 * @brief 棰勫垽鍓嶆柟鏇茬巼鍥犲瓙
 */
static float Calculate_Upcoming_Curve_Factor(int start_idx, float preview_dist)
{
    if (start_idx >= nav_ram_data.point_count - 5) return 0.0f;

    float max_curve = 0.0f;
    // 鍒嗕笁娈垫壂鎻忓墠鏂?(杩戙€佷腑銆佽繙)锛屽鎵炬渶鎬ョ殑寮偣
    float check_dists[3] = {preview_dist * 0.4f, preview_dist * 0.7f, preview_dist};
    
    for(int step = 0; step < 3; step++) {
        float p_dist_sq = check_dists[step] * check_dists[step];
        int far_idx = start_idx;
        
        // 鍚屾牱闄愬埗鎵弿娣卞害锛岄槻姝㈡壂杩囧ご
        for (int i = start_idx; i < nav_ram_data.point_count; i++) {
            if (nav_ram_data.points[i].point_type != NAV_POINT_PATH) break;
            if (CalcDistanceSq(nav_ram_data.points[start_idx].x, nav_ram_data.points[start_idx].y, 
                               nav_ram_data.points[i].x, nav_ram_data.points[i].y) >= p_dist_sq) {
                far_idx = i; break;
            }
            if (i > start_idx + 150) break; // 鎵弿娣卞害闄愬埗
        }
        
        if (far_idx > start_idx) {
            float dx = nav_ram_data.points[far_idx].x - nav_ram_data.points[start_idx].x;
            float dy = nav_ram_data.points[far_idx].y - nav_ram_data.points[start_idx].y;
            float path_angle = -atan2f(dy, -dx) * 57.29578f;
            float angle_diff = fabsf(NormalizeAngle(path_angle - NavReplay_CurrentYaw()));
            
            float factor = (angle_diff / 60.0f) * (1.2f - 0.2f * step);
            if (factor > max_curve) max_curve = factor;
        }
    }
    return (max_curve > 1.0f) ? 1.0f : max_curve;
}

uint8 is_arrived = 0;  // 鍒拌揪鍒ゅ畾鐘舵€侀攣

// 灞€閮ㄩ潤鎬佸彉閲忥細鐢ㄤ簬婊ゆ尝鍘嗗彶淇濇寔涓庝笅闄嶆部妫€娴?
static uint8 s_is_aligning = 0;
static uint8 s_prev_trigger = 0;  // 鐢ㄤ簬妫€娴嬬姸鎬佹満缁撴潫鐨勭灛闂达紙涓嬮檷娌匡級

/*杩欓噷娉ㄩ噴浜嗭紝淇濆瓨鐨勬槸Pure Pursuit 鑱斿悎 鐗规畩鐐圭洿璧?鐘舵€佹満*/

static void NavReplay_ResetProcessState(void)
{
    prev_err_degree = 0.0f;
    prev_speed_set = 0.0f;
    prev_curve_f = 0.0f;
    is_arrived = 0;
    s_is_aligning = 0;
    s_prev_trigger = 0;
#if IMU_CATEGORY == 3
    s_start_heading_stable_count = 0;
#endif
}

#if IMU_CATEGORY == 3
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

static void NavReplay_ResetLaunchPose(void)
{
    inertial_nav.x = 0.0f;
    inertial_nav.y = 0.0f;
    inertial_nav.vx_body = 0.0f;
    inertial_nav.vy_body = 0.0f;
    inertial_nav.slip_flag = 0;
    inertial_nav.relative_yaw = 0.0f;
    inertial_nav.init_yaw = euler_angle.yaw;
#if GNSS_NAV == 2
    FusedNav_ResetSession();
    Gnss_Transform_Reset_Origin();
#endif

    g_target_idx = 0;
    g_current_point_type = NAV_POINT_PATH;
    g_special_action_trigger = 0;

    NavReplay_ResetProcessState();
    err_degree = 0.0f;
    target_speed_set = NAV_SPEED_STOP;
}
#endif

void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING) return;
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

    // 濡傛灉鐘舵€佹満姝ｅ湪骞查锛岃褰曠姸鎬佸苟閫€鍑?
    // if (g_special_action_trigger == 1) {
    //     s_prev_trigger = 1;
    //     return; 
    // }

    // ==========================================
    // 馃幆 鐏惧悗閲嶅缓鏈哄埗 (Recovery)锛氭娴嬬姸鎬佹満鍒氬垰缁撴潫鐨勭灛闂?
    // ==========================================
    uint8 is_recovering = 0; // 銆愭敞銆戜负淇濊瘉涓嬫柟鍑芥暟璋冪敤涓嶆姤閿欙紝灏嗗叾澹版槑鏀惧嚭
    // if (s_prev_trigger == 1 && g_special_action_trigger == 0) {
    //     is_recovering = 1;
    //     s_prev_trigger = 0;
    //     is_arrived = 0;
        
    //     // 銆愬叧閿€戯細娓呯┖鍘嗗彶鍖呰⒈锛?
    //     // 闃叉杞﹀瓙鎶婅繘鍏ョ壒娈婄偣鍓嶇殑鏃ц搴﹀拰閫熷害甯﹀叆鍒扮幇鍦紝瀵艰嚧绐佺劧鐚涙墦鏂瑰悜鐩?
    //     prev_err_degree = 0.0f;
    //     prev_speed_set = 0.0f;
    //     s_is_aligning = 0; 
        
    //     #if DEBUG_LOG_ENABLE
    //     printf("[Nav] Special Action Finished. Recovering back to route...\r\n");
    //     #endif
    // }

    // 1. 鑾峰彇褰撳墠杞﹁締鍦ㄨ矾寰勪笂鐨勫熀鍑嗙储寮?
    // 濡傛灉鏄垰鍒氱粨鏉熺姸鎬佹満(is_recovering=1)锛屾悳瀵昏寖鍥存墿澶у埌 300鐐?6绫?锛屽苟璞佸厤璺濈闄愬埗
    int scan_range = 80;
    int base_idx = Find_Closest_Point_Index_Strict(g_target_idx, scan_range, is_recovering);
    g_target_idx = base_idx;

    if (g_target_idx >= nav_ram_data.point_count - 1) {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = 0; err_degree = 0; 
        s_is_aligning = 0;
        return;
    }

    // ====================================================================
    // 馃憞 浠ヤ笅涓哄鎵剧壒娈婄偣銆佸幓鐗规畩鐐癸紙妯″紡A锛夊拰鐘舵€佹満鐨勫叏閮ㄩ€昏緫锛屽凡鎸夎姹傛暣浣撴敞閲?
    // ====================================================================

    // // 2. 寰€鍓嶆壂鎻忥紝瀵绘壘鍗冲皢鍒版潵鐨勭壒娈婄偣浠ュ強璁＄畻鍏剁湡瀹炶窛绂?
    // int special_idx = -1;
    // float dist_to_special = 99999.0f;
    // // 鎵弿鑼冨洿 100涓偣(2000mm)
    // for (int i = base_idx; i < nav_ram_data.point_count && i < base_idx + 100; i++) {
    //     if (nav_ram_data.points[i].point_type != NAV_POINT_PATH || i == nav_ram_data.point_count - 1) {
    //         special_idx = i;
    //         dist_to_special = CalcDistance(inertial_nav.x, inertial_nav.y, 
    //                                        nav_ram_data.points[i].x, nav_ram_data.points[i].y);
    //         break;
    //     }
    // }

    // // ====================================================================
    // // 鍙屾ā寮忚嚜鍔ㄥ垏鎹細1000mm 鍐呰繘鍏?鍏堣浆鍐嶈蛋"妯″紡锛涘惁鍒欐墽琛?楂橀€?Pure Pursuit"
    // // ====================================================================
    // if (special_idx != -1 && dist_to_special <= 1000.0f)
    // {
    //     // -------------------------------------------------------------
    //     // 銆愭ā寮廇銆戠簿鍑嗛€艰繎妯″紡 (1000mm浠ュ唴)锛氬厛杞啀璧帮紝缁濆浣嶇疆绮惧噯瑙﹀彂
    //     // -------------------------------------------------------------
    //     
    //     // 鑸悜鐬勫噯鐐硅绠楋細涓轰簡涓嶆妱杩戦亾锛岃窛绂诲ぇ浜?00mm鏃朵緷鐒剁湅璺緞鍓嶆柟锛屾瀬杩戞椂鐩存帴鐪嬬壒娈婄偣
    //     int aim_idx = base_idx + 15; // 寰€鍓嶇湅绾?00mm
    //     if (aim_idx > special_idx) aim_idx = special_idx;
    //     
    //     float tx = nav_ram_data.points[aim_idx].x;
    //     float ty = nav_ram_data.points[aim_idx].y;
    //
    //     float dx = tx - inertial_nav.x;
    //     float dy = ty - inertial_nav.y;
    //     float target_yaw = -atan2f(dy, -dx) * 57.29578f; 
    //     
    //     // 绮惧噯妯″紡涓嬶紝瑙掑害涓嶅仛婊ゆ尝锛岃姹傜洿鎺ユ墦鍒扮洰鏍囪搴?
    //     err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);
    //
    //     if (!is_arrived) {//鏍规嵁鐘舵€侀攣鍒ゆ柇
    //         // ==========================================
    //         // 銆愭牳蹇冧慨澶嶃€戯細寮曞叆瀹藉鍒拌揪鍒ゅ畾锛岄槻姝㈤珮閫熺┛閫?
    //         // ==========================================
    //         if (dist_to_special <= NAV_DIST_ARRIVE) {
    //             is_arrived = 1; // 绮剧‘瀹炶揪
    //         } 
    //         // 瀹藉鍒ゅ畾锛氬鏋滃簳灞傝拷韪储寮曞凡缁忚鍗℃鍦ㄨ繖涓壒娈婄偣涓婁簡锛?
    //         // 涓旂墿鐞嗚窛绂诲湪绋嶅ぇ鑼冨洿鍐?濡?60mm 鍐?锛岃鏄庤溅瀛愬洜涓烘儻鎬х◢寰啿杩囦簡涓€鐐癸紝寮哄埗鍒や綔鍒拌揪锛?
    //         // else if (base_idx == special_idx && dist_to_special <= NAV_DIST_ARRIVE + 40.0f) {
    //         //     is_arrived = 1;
    //         // }
    //     }
    //
    //     if (is_arrived)
    //     {
    //         // --- 1. 鍒拌揪鐗规畩鐐癸細瑙﹀彂鍔ㄤ綔 ---
    //         target_speed_set = NAV_SPEED_STOP;
    //         g_current_point_type = nav_ram_data.points[special_idx].point_type;
    //
    //         #if DEBUG_LOG_ENABLE
    //         printf("[Nav] Arrived Special Point[%d] Type[%d]\r\n", special_idx, g_current_point_type);
    //         #endif
    //
    //         // 鎻愬彇璇ョ壒娈婄偣璁板綍鐨勭洰鏍囧亸鑸
    //         float special_target_yaw = nav_ram_data.points[special_idx].target_yaw_deg;
    //         
    //         // 璁＄畻杞﹁韩褰撳墠瑙掑害涓庣洰鏍囪搴︾殑鍋忓樊
    //         float special_yaw_err = NormalizeAngle(special_target_yaw - inertial_nav.relative_yaw);
    //
    //         // 鍒ゆ柇瑙掑害鏄惁瀵归綈
    //         if (fabsf(special_yaw_err) > NAV_YAW_TOLERANCE)
    //         {
    //             // 瑙掑害杩樻病瀵归綈锛佸皢鐗规畩鐐圭殑瑙掑害璇樊鍠傜粰搴曞眰锛岃Е鍙戝師鍦拌嚜杞榻?
    //             err_degree = special_yaw_err;
    //             
    //             #if DEBUG_LOG_ENABLE
    //             // printf("[Nav] Aligning Yaw at Special Point... err: %.2f\r\n", special_yaw_err);
    //             #endif
    //         }
    //         else
    //         {
    //             // 浣嶇疆鍒颁簡锛岃搴︿篃瀵归綈浜嗭紒姝ｅ紡瑙﹀彂鐘舵€佹満锛?
    //             g_current_point_type = nav_ram_data.points[special_idx].point_type;
    //
    //             #if DEBUG_LOG_ENABLE
    //             printf("[Nav] Arrived & Aligned Special Point[%d] Type[%d]\r\n", special_idx, g_current_point_type);
    //             #endif
    //
    //             if (g_current_point_type != NAV_POINT_PATH)
    //             {
    //                 if (g_current_point_type == NAV_POINT_CIRCLE) {
    //                     minefield_flag = 1;
    //                 }
    //                 if (g_current_point_type == NAV_POINT_JUMP) {
    //                     vision_detected_three_jump_point = 1;//瑙﹀彂涓夌骇璺崇姸鎬佹満
    //                 }
    //                 if (g_current_point_type == NAV_POINT_BRIDGE) {
    //                     vision_detected_bridge_point = 1;//瑙﹀彂涓夋ˉ妗ョ姸鎬佹満
    //                 }
    //                 if (g_current_point_type == NAV_POINT_BUMP) {
    //                     BumpyRoad_Trigger();  // 瑙﹀彂棰犵案璺鐘舵€佹満
    //                 }
    //                 g_special_action_trigger = 1;
    //             }
    //             
    //             // 闃叉閿侊細鍔ㄤ綔瑙﹀彂鍚庯紝寮鸿璺ㄨ繃杩欎釜鐗规畩鐐?
    //             g_target_idx = special_idx + 1;
    //         }
    //     }
    //     else
    //     {
    //         // --- 2. 鏈埌杈剧壒娈婄偣锛氬厛杞啀璧?---
    //         if (fabsf(err_degree) > NAV_YAW_TOLERANCE)
    //         {
    //             // 瑙掑害鍋忓樊杈冨ぇ锛屽厛鍘熷湴/鏋佷綆閫熸棆杞?
    //             target_speed_set = NAV_SPEED_STOP;
    //             #if DEBUG_LOG_ENABLE
    //             // printf("[Nav] Rotating to target, err: %.2f\r\n", err_degree);
    //             #endif
    //         }
    //         else
    //         {
    //             // 瑙掑害鍩烘湰瀵瑰噯锛屾牴鎹埌鐗规畩鐐圭殑鐗╃悊璺濈鏉ヨ鍒掗€熷害
    //             if (dist_to_special > NAV_DIST_FAR)
    //             {
    //                 target_speed_set = NAV_SPEED_FAST/5.0f;
    //             }
    //             else if (dist_to_special > NAV_DIST_NEAR)
    //             {
    //                 float ratio = (dist_to_special - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR);
    //                 target_speed_set = NAV_SPEED_SLOW + (NAV_SPEED_FAST/5.0f - NAV_SPEED_SLOW) * ratio;
    //             }
    //             else
    //             {
    //                 target_speed_set = NAV_SPEED_SLOW;
    //             }
    //         }
    //     }
    //     
    //     // 鍚屾婊ゆ尝鍘嗗彶锛岄槻姝㈠垏鍥為珮閫熸ā寮忔椂杞﹁締鐚涙姈
    //     prev_err_degree = err_degree;
    //     prev_speed_set = target_speed_set;
    // }
    // else
    // {

    // ====================================================================
    // 馃憞 鍏ㄧ▼淇濇寔鎵ц浠ヤ笅銆愭ā寮廈銆戦珮閫?Pure Pursuit 瀵昏抗妯″紡浠ｇ爜
    // ====================================================================

    // -------------------------------------------------------------
    // 銆愭ā寮廈銆戦珮閫?Pure Pursuit 瀵昏抗妯″紡 (璺濈鐗规畩鐐?> 800mm 鎴栨棤鐗规畩鐐?
    // -------------------------------------------------------------
    
        // 鍔ㄦ€佹瀬闄愬墠鐬昏绠?
        float lookahead_dist = PP_LD_MIN_CURVE + fabsf(prev_speed_set) * PP_LD_SPEED_GAIN;
#if GNSS_NAV == 2
        if (lookahead_dist < 1200.0f)
        {
            lookahead_dist = 1200.0f;
        }
#endif
        float lookahead_dist_sq = lookahead_dist * lookahead_dist;

        // 鏋侀檺閫夌偣 (瀵绘壘杩滄柟鍓嶇灮鐐?
        float tx = nav_ram_data.points[base_idx].x;
        float ty = nav_ram_data.points[base_idx].y;
        int ld_scan_limit = base_idx + (int)(lookahead_dist / 15.0f) + 40;
        if (ld_scan_limit > nav_ram_data.point_count) ld_scan_limit = nav_ram_data.point_count;

        for (int i = base_idx; i < ld_scan_limit; i++) {
            float d_sq = CalcDistanceSq(NavReplay_CurrentX(), NavReplay_CurrentY(),
                                        nav_ram_data.points[i].x, nav_ram_data.points[i].y);
            tx = nav_ram_data.points[i].x; ty = nav_ram_data.points[i].y;
            if (d_sq >= lookahead_dist_sq || nav_ram_data.points[i].point_type != NAV_POINT_PATH) {
                break;
            }
        }

        // 璁＄畻鑸悜
        float target_yaw = -atan2f(ty - NavReplay_CurrentY(), -(tx - NavReplay_CurrentX())) * 57.29578f;
        float raw_err_degree = NormalizeAngle(target_yaw - NavReplay_CurrentYaw());

        // 鏇茬巼璁＄畻涓庢毚鍔涢€熷害瑙勫垝
        float curve_f = Calculate_Upcoming_Curve_Factor(base_idx, CURVE_PREVIEW_DIST);
        
        if (curve_f < prev_curve_f) {
            curve_f *= 0.4f; // 鍑哄集寮瑰皠
        }
        prev_curve_f = curve_f;

        if (curve_f < SPD_CURVE_DEADZONE) curve_f = 0.0f;
        else curve_f = (curve_f - SPD_CURVE_DEADZONE) / (1.0f - SPD_CURVE_DEADZONE);
        curve_f = powf(curve_f, SPD_CURVE_EXPONENT);

        float raw_spd = 0;
        float current_max_spd = (curve_f <= 0.0f) ? (NAV_SPEED_FAST * 1.3f) : NAV_SPEED_FAST;
        raw_spd = current_max_spd - (current_max_spd - NAV_SPEED_SLOW) * curve_f;
        
        // 3. 瑙掑害绾犲亸鍑忛€燂細鏋侀€熻椹舵椂锛屽皬鍋忓樊涓嶅噺閫燂紝澶у亸宸墠寰皟
        float ang_p = fabsf(raw_err_degree) / SPD_ANGLE_TOLERANCE;
        if (ang_p > 1.0f) ang_p = 1.0f;
        raw_spd *= (1.0f - SPD_ANGLE_PENALTY * ang_p);

        // 鏋侀€熸护娉㈣緭鍑?
        float diff = raw_err_degree - prev_err_degree;
        if (diff > SLEW_RATE_ANGLE) raw_err_degree = prev_err_degree + SLEW_RATE_ANGLE;
        else if (diff < -SLEW_RATE_ANGLE) raw_err_degree = prev_err_degree - SLEW_RATE_ANGLE;

        err_degree = FILTER_ALPHA_ANGLE * raw_err_degree + (1.0f - FILTER_ALPHA_ANGLE) * prev_err_degree;
        target_speed_set = FILTER_ALPHA_SPEED * raw_spd + (1.0f - FILTER_ALPHA_SPEED) * prev_speed_set;

        prev_err_degree = err_degree;
        prev_speed_set = target_speed_set;
    // }
}
#endif

#if CURRENT_NAV_PLAN == 2 //濡傛灉鏄鐩簩锛岃繖姝ユ殏鏃朵笉鍋氱壒璋冪殑鎯呭喌涓嬶紝浠呬粎闇€瑕侀浄鍖虹姸鎬佹満锛屼笉闇€瑕佽繘鍘荤殑鏃跺€欏鍑嗙鐩搴?

// 澶栭儴/闈欐€佸彉閲忓０鏄?
static float prev_err_degree = 0.0f;
static float prev_speed_set = 0.0f;
static float prev_curve_f = 0.0f;

// 楂樻晥骞虫柟璺濈璁＄畻
static inline float CalcDistanceSq(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

/**
 * @brief 涓ユ牸鍗曞悜绱㈠紩杩借釜
 * 寮哄埗瑕佹眰绱㈠紩鍙兘鍦ㄥ綋鍓嶄綅缃線鍚?[0, search_range] 鑼冨洿鍐呭鎵俱€?
 * 褰诲簳瑙ｅ喅鍦ㄥ師璺姌杩旇建杩逛腑锛岀储寮曡烦鍒板洖绋嬭矾寰勭殑闂銆?
 * 涓ユ牸鍗曞悜绱㈠紩杩借釜 (甯﹂槻绌挎ā閿?+ 鏀寔澶ц寖鍥撮噸瀹氫綅)
 * 寮哄埗瑕佹眰绱㈠紩鍙兘鍦ㄥ綋鍓嶄綅缃線鍚庡鎵撅紝涓旂粷涓嶅厑璁歌烦杩囩壒娈婄偣锛?
 */
static int Find_Closest_Point_Index_Strict(int current_idx, int search_range, uint8 is_recovering)
{
    int closest_idx = current_idx;
    float min_dist_sq = 1e9f; 

    int end_idx = current_idx + search_range;
    if (end_idx >= nav_ram_data.point_count) {
        end_idx = nav_ram_data.point_count - 1;
    }

    // 鍙線鍚庢悳锛屼笉鍥炲ご
    for (int i = current_idx; i <= end_idx; i++) {
        float d_sq = CalcDistanceSq(inertial_nav.x, inertial_nav.y, 
                                    nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        if (d_sq < min_dist_sq) {
            min_dist_sq = d_sq;
            closest_idx = i;
        }
        
        // 銆愭牳蹇冧慨澶嶃€戯細鍙鎵弿閬囧埌鐗规畩鐐癸紝蹇呴』绔嬪埢缁堟锛?
        // 鍝€?current_idx 鑷繁灏辨槸鐗规畩鐐癸紝涔熺粷涓嶅厑璁稿啀寰€鍚庢悳锛佹姝诲喕缁撶储寮曪紒
        if (nav_ram_data.points[i].point_type != NAV_POINT_PATH) {
            if (closest_idx > i) closest_idx = i;
            break; 
        }
    }
    
    // 涓綅淇濇姢锛氬鏋滄槸閲嶅畾浣嶇姸鎬侊紝璞佸厤 800mm 闄愬埗锛佸厑璁歌溅瀛愪粠杩滃寮鸿鍒囧洖涓昏矾
    if (!is_recovering && min_dist_sq > 800.0f * 800.0f) {
        return current_idx; 
    }
    return closest_idx;
}

/**
 * @brief 棰勫垽鍓嶆柟鏇茬巼鍥犲瓙
 */
static float Calculate_Upcoming_Curve_Factor(int start_idx, float preview_dist)
{
    if (start_idx >= nav_ram_data.point_count - 5) return 0.0f;

    float max_curve = 0.0f;
    // 鍒嗕笁娈垫壂鎻忓墠鏂?(杩戙€佷腑銆佽繙)锛屽鎵炬渶鎬ョ殑寮偣
    float check_dists[3] = {preview_dist * 0.4f, preview_dist * 0.7f, preview_dist};
    
    for(int step = 0; step < 3; step++) {
        float p_dist_sq = check_dists[step] * check_dists[step];
        int far_idx = start_idx;
        
        // 鍚屾牱闄愬埗鎵弿娣卞害锛岄槻姝㈡壂杩囧ご
        for (int i = start_idx; i < nav_ram_data.point_count; i++) {
            if (nav_ram_data.points[i].point_type != NAV_POINT_PATH) break;
            if (CalcDistanceSq(nav_ram_data.points[start_idx].x, nav_ram_data.points[start_idx].y, 
                               nav_ram_data.points[i].x, nav_ram_data.points[i].y) >= p_dist_sq) {
                far_idx = i; break;
            }
            if (i > start_idx + 150) break; // 鎵弿娣卞害闄愬埗
        }
        
        if (far_idx > start_idx) {
            float dx = nav_ram_data.points[far_idx].x - nav_ram_data.points[start_idx].x;
            float dy = nav_ram_data.points[far_idx].y - nav_ram_data.points[start_idx].y;
            float path_angle = -atan2f(dy, -dx) * 57.29578f;
            float angle_diff = fabsf(NormalizeAngle(path_angle - inertial_nav.relative_yaw));
            
            float factor = (angle_diff / 60.0f) * (1.2f - 0.2f * step);
            if (factor > max_curve) max_curve = factor;
        }
    }
    return (max_curve > 1.0f) ? 1.0f : max_curve;
}
uint8 is_arrived = 0;  // 鍒拌揪鍒ゅ畾鐘舵€侀攣

// 灞€閮ㄩ潤鎬佸彉閲忥細鐢ㄤ簬婊ゆ尝鍘嗗彶淇濇寔涓庝笅闄嶆部妫€娴?
static uint8 s_is_aligning = 0;
static uint8 s_prev_trigger = 0;  // 鐢ㄤ簬妫€娴嬬姸鎬佹満缁撴潫鐨勭灛闂达紙涓嬮檷娌匡級

/*杩欓噷娉ㄩ噴浜嗭紝淇濆瓨鐨勬槸Pure Pursuit 鑱斿悎 鐗规畩鐐圭洿璧?鐘舵€佹満*/

void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING) return;

    // 濡傛灉鐘舵€佹満姝ｅ湪骞查锛岃褰曠姸鎬佸苟閫€鍑?
    if (g_special_action_trigger == 1) {
        s_prev_trigger = 1;
        return; 
    }

    // ==========================================
    // 馃幆 鐏惧悗閲嶅缓鏈哄埗 (Recovery)锛氭娴嬬姸鎬佹満鍒氬垰缁撴潫鐨勭灛闂?
    // ==========================================
    uint8 is_recovering = 0;
    if (s_prev_trigger == 1 && g_special_action_trigger == 0) {
        is_recovering = 1;
        s_prev_trigger = 0;
        is_arrived = 0;
        
        // 銆愬叧閿€戯細娓呯┖鍘嗗彶鍖呰⒈锛?
        // 闃叉杞﹀瓙鎶婅繘鍏ョ壒娈婄偣鍓嶇殑鏃ц搴﹀拰閫熷害甯﹀叆鍒扮幇鍦紝瀵艰嚧绐佺劧鐚涙墦鏂瑰悜鐩?
        prev_err_degree = 0.0f;
        prev_speed_set = 0.0f;
        s_is_aligning = 0; 
        
        #if DEBUG_LOG_ENABLE
        printf("[Nav] Special Action Finished. Recovering back to route...\r\n");
        #endif
    }

    // 1. 鑾峰彇褰撳墠杞﹁締鍦ㄨ矾寰勪笂鐨勫熀鍑嗙储寮?
    // 濡傛灉鏄垰鍒氱粨鏉熺姸鎬佹満(is_recovering=1)锛屾悳瀵昏寖鍥存墿澶у埌 300鐐?6绫?锛屽苟璞佸厤璺濈闄愬埗
    int scan_range = is_recovering ? 300 : 80;
    int base_idx = Find_Closest_Point_Index_Strict(g_target_idx, scan_range, is_recovering);
    g_target_idx = base_idx;

    if (g_target_idx >= nav_ram_data.point_count - 1) {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = 0; err_degree = 0; 
        s_is_aligning = 0;
        return;
    }

    // 2. 寰€鍓嶆壂鎻忥紝瀵绘壘鍗冲皢鍒版潵鐨勭壒娈婄偣浠ュ強璁＄畻鍏剁湡瀹炶窛绂?
    int special_idx = -1;
    float dist_to_special = 99999.0f;
    // 鎵弿鑼冨洿 100涓偣(2000mm)
    for (int i = base_idx; i < nav_ram_data.point_count && i < base_idx + 100; i++) {
        if (nav_ram_data.points[i].point_type != NAV_POINT_PATH || i == nav_ram_data.point_count - 1) {
            special_idx = i;
            dist_to_special = CalcDistance(inertial_nav.x, inertial_nav.y, 
                                           nav_ram_data.points[i].x, nav_ram_data.points[i].y);
            break;
        }
    }

    // ====================================================================
    // 鍙屾ā寮忚嚜鍔ㄥ垏鎹細1000mm 鍐呰繘鍏?鍏堣浆鍐嶈蛋"妯″紡锛涘惁鍒欐墽琛?楂橀€?Pure Pursuit"
    // ====================================================================
    if (special_idx != -1 && dist_to_special <= 1000.0f)
    {
        // -------------------------------------------------------------
        // 銆愭ā寮廇銆戠簿鍑嗛€艰繎妯″紡 (1000mm浠ュ唴)锛氬厛杞啀璧帮紝缁濆浣嶇疆绮惧噯瑙﹀彂
        // -------------------------------------------------------------
        
        // 鑸悜鐬勫噯鐐硅绠楋細涓轰簡涓嶆妱杩戦亾锛岃窛绂诲ぇ浜?00mm鏃朵緷鐒剁湅璺緞鍓嶆柟锛屾瀬杩戞椂鐩存帴鐪嬬壒娈婄偣
        int aim_idx = base_idx + 15; // 寰€鍓嶇湅绾?00mm
        if (aim_idx > special_idx) aim_idx = special_idx;
        
        float tx = nav_ram_data.points[aim_idx].x;
        float ty = nav_ram_data.points[aim_idx].y;

        float dx = tx - inertial_nav.x;
        float dy = ty - inertial_nav.y;
        float target_yaw = -atan2f(dy, -dx) * 57.29578f; 
        
        // 绮惧噯妯″紡涓嬶紝瑙掑害涓嶅仛婊ゆ尝锛岃姹傜洿鎺ユ墦鍒扮洰鏍囪搴?
        err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

        if (!is_arrived) {//鏍规嵁鐘舵€侀攣鍒ゆ柇
            // ==========================================
            // 銆愭牳蹇冧慨澶嶃€戯細寮曞叆瀹藉鍒拌揪鍒ゅ畾锛岄槻姝㈤珮閫熺┛閫?
            // ==========================================
            if (dist_to_special <= NAV_DIST_ARRIVE) {
                is_arrived = 1; // 绮剧‘鍒拌揪
            } 
            // 瀹藉鍒ゅ畾锛氬鏋滃簳灞傝拷韪储寮曞凡缁忚鍗℃鍦ㄨ繖涓壒娈婄偣涓婁簡锛?
            // 涓旂墿鐞嗚窛绂诲湪绋嶅ぇ鑼冨洿鍐?濡?60mm 鍐?锛岃鏄庤溅瀛愬洜涓烘儻鎬х◢寰啿杩囦簡涓€鐐癸紝寮哄埗鍒や綔鍒拌揪锛?
            // else if (base_idx == special_idx && dist_to_special <= NAV_DIST_ARRIVE + 40.0f) {
            //     is_arrived = 1;
            // }
        }

        if (is_arrived)
        {
            // --- 1. 鍒拌揪鐗规畩鐐癸細瑙﹀彂鍔ㄤ綔 ---
            target_speed_set = NAV_SPEED_STOP;
            g_current_point_type = nav_ram_data.points[special_idx].point_type;

            #if DEBUG_LOG_ENABLE
            printf("[Nav] Arrived Special Point[%d] Type[%d]\r\n", special_idx, g_current_point_type);
            #endif

            // 鎻愬彇璇ョ壒娈婄偣璁板綍鐨勭洰鏍囧亸鑸
            // float special_target_yaw = nav_ram_data.points[special_idx].target_yaw_deg;
            
            // 璁＄畻杞﹁韩褰撳墠瑙掑害涓庣洰鏍囪搴︾殑鍋忓樊
            // float special_yaw_err = NormalizeAngle(special_target_yaw - inertial_nav.relative_yaw);

            // // 鍒ゆ柇瑙掑害鏄惁瀵归綈
            // if (fabsf(special_yaw_err) > NAV_YAW_TOLERANCE)
            // {
            //     // 瑙掑害杩樻病瀵归綈锛佸皢鐗规畩鐐圭殑瑙掑害璇樊鍠傜粰搴曞眰锛岃Е鍙戝師鍦拌嚜杞榻?
            //     err_degree = special_yaw_err;
                
            //     #if DEBUG_LOG_ENABLE
            //     // printf("[Nav] Aligning Yaw at Special Point... err: %.2f\r\n", special_yaw_err);
            //     #endif
            // }
            // else
            // {
                // 浣嶇疆鍒颁簡锛岃搴︿篃瀵归綈浜嗭紒姝ｅ紡瑙﹀彂鐘舵€佹満锛併€愮鐩簩浼樺寲銆戜笉瑕佽搴﹀榻?
                g_current_point_type = nav_ram_data.points[special_idx].point_type;

                #if DEBUG_LOG_ENABLE
                printf("[Nav] Arrived & Aligned Special Point[%d] Type[%d]\r\n", special_idx, g_current_point_type);
                #endif

                if (g_current_point_type != NAV_POINT_PATH)
                {
                    if (g_current_point_type == NAV_POINT_CIRCLE) {
                        minefield_flag = 1;
                    }
                    // if (g_current_point_type == NAV_POINT_JUMP) {
                    //     vision_detected_three_jump_point = 1;//瑙﹀彂涓夌骇璺崇姸鎬佹満
                    // }
                    // if (g_current_point_type == NAV_POINT_BRIDGE) {
                    //     vision_detected_bridge_point = 1;//瑙﹀彂涓夋ˉ妗ョ姸鎬佹満
                    // }
                    // if (g_current_point_type == NAV_POINT_BUMP) {
                    //     BumpyRoad_Trigger();  // 瑙﹀彂棰犵案璺鐘舵€佹満
                    // }
                    g_special_action_trigger = 1;
                }
                
                // 闃叉閿侊細鍔ㄤ綔瑙﹀彂鍚庯紝寮鸿璺ㄨ繃杩欎釜鐗规畩鐐?
                g_target_idx = special_idx + 1;
            // }
        }
        else
        {
            // --- 2. 鏈埌杈剧壒娈婄偣锛氬厛杞啀璧?---
            if (fabsf(err_degree) > NAV_YAW_TOLERANCE)
            {
                // 瑙掑害鍋忓樊杈冨ぇ锛屽厛鍘熷湴/鏋佷綆閫熸棆杞?
                target_speed_set = NAV_SPEED_STOP;
                #if DEBUG_LOG_ENABLE
                // printf("[Nav] Rotating to target, err: %.2f\r\n", err_degree);
                #endif
            }
            else
            {
                // 瑙掑害鍩烘湰瀵瑰噯锛屾牴鎹埌鐗规畩鐐圭殑鐗╃悊璺濈鏉ヨ鍒掗€熷害
                if (dist_to_special > NAV_DIST_FAR)
                {
                    target_speed_set = NAV_SPEED_FAST/5.0f;
                }
                else if (dist_to_special > NAV_DIST_NEAR)
                {
                    float ratio = (dist_to_special - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR);
                    target_speed_set = NAV_SPEED_SLOW + (NAV_SPEED_FAST/5.0f - NAV_SPEED_SLOW) * ratio;
                }
                else
                {
                    target_speed_set = NAV_SPEED_SLOW;
                }
            }
        }
        
        // 鍚屾婊ゆ尝鍘嗗彶锛岄槻姝㈠垏鍥為珮閫熸ā寮忔椂杞﹁締鐚涙姈
        prev_err_degree = err_degree;
        prev_speed_set = target_speed_set;
    }
    else
    {
        // -------------------------------------------------------------
        // 銆愭ā寮廈銆戦珮閫?Pure Pursuit 瀵昏抗妯″紡 (璺濈鐗规畩鐐?> 800mm 鎴栨棤鐗规畩鐐?
        // -------------------------------------------------------------
        
        // 鍔ㄦ€佹瀬闄愬墠鐬昏绠?
        float lookahead_dist = PP_LD_MIN_CURVE + fabsf(prev_speed_set) * PP_LD_SPEED_GAIN;
        float lookahead_dist_sq = lookahead_dist * lookahead_dist;

        // 鏋侀檺閫夌偣 (瀵绘壘杩滄柟鍓嶇灮鐐?
        float tx = nav_ram_data.points[base_idx].x;
        float ty = nav_ram_data.points[base_idx].y;
        int ld_scan_limit = base_idx + (int)(lookahead_dist / 15.0f) + 40;
        if (ld_scan_limit > nav_ram_data.point_count) ld_scan_limit = nav_ram_data.point_count;

        for (int i = base_idx; i < ld_scan_limit; i++) {
            float d_sq = CalcDistanceSq(inertial_nav.x, inertial_nav.y, nav_ram_data.points[i].x, nav_ram_data.points[i].y);
            tx = nav_ram_data.points[i].x; ty = nav_ram_data.points[i].y;
            if (d_sq >= lookahead_dist_sq || nav_ram_data.points[i].point_type != NAV_POINT_PATH) {
                break;
            }
        }

        // 璁＄畻鑸悜
        float target_yaw = -atan2f(ty - inertial_nav.y, -(tx - inertial_nav.x)) * 57.29578f;
        float raw_err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

        // 鏇茬巼璁＄畻涓庢毚鍔涢€熷害瑙勫垝
        float curve_f = Calculate_Upcoming_Curve_Factor(base_idx, CURVE_PREVIEW_DIST);
        
        if (curve_f < prev_curve_f) {
            curve_f *= 0.4f; // 鍑哄集寮瑰皠
        }
        prev_curve_f = curve_f;

        if (curve_f < SPD_CURVE_DEADZONE) curve_f = 0.0f;
        else curve_f = (curve_f - SPD_CURVE_DEADZONE) / (1.0f - SPD_CURVE_DEADZONE);
        curve_f = powf(curve_f, SPD_CURVE_EXPONENT);

        float raw_spd = 0;
        float current_max_spd = (curve_f <= 0.0f) ? (NAV_SPEED_FAST * 1.3f) : NAV_SPEED_FAST;
        raw_spd = current_max_spd - (current_max_spd - NAV_SPEED_SLOW) * curve_f;
        
        // 3. 瑙掑害绾犲亸鍑忛€燂細鏋侀€熻椹舵椂锛屽皬鍋忓樊涓嶅噺閫燂紝澶у亸宸墠寰皟
        float ang_p = fabsf(raw_err_degree) / SPD_ANGLE_TOLERANCE;
        if (ang_p > 1.0f) ang_p = 1.0f;
        raw_spd *= (1.0f - SPD_ANGLE_PENALTY * ang_p);

        // 鏋侀€熸护娉㈣緭鍑?
        float diff = raw_err_degree - prev_err_degree;
        if (diff > SLEW_RATE_ANGLE) raw_err_degree = prev_err_degree + SLEW_RATE_ANGLE;
        else if (diff < -SLEW_RATE_ANGLE) raw_err_degree = prev_err_degree - SLEW_RATE_ANGLE;

        err_degree = FILTER_ALPHA_ANGLE * raw_err_degree + (1.0f - FILTER_ALPHA_ANGLE) * prev_err_degree;
        target_speed_set = FILTER_ALPHA_SPEED * raw_spd + (1.0f - FILTER_ALPHA_SPEED) * prev_speed_set;

        prev_err_degree = err_degree;
        prev_speed_set = target_speed_set;
    }
}
#endif

#if CURRENT_NAV_PLAN == 3 //濡傛灉鏄鐩簩锛岃繖姝ユ殏鏃朵笉鍋氱壒璋冪殑鎯呭喌涓嬶紝涓嶉渶瑕侀浄鍖虹姸鎬佹満锛岃繘鍘荤殑鏃跺€欏鍑嗙鐩搴?
// ============================================================================
// 绮惧噯澶嶅埢澶勭悊閫昏緫 (鐐瑰埌鐐癸紝鍏堣浆鍐嶈蛋锛屽姞鍏ラ槻闇囪崱涓庡钩婊戞护娉?
// 鎱㈡參鐨勮窇绉戜笁
// ============================================================================
// --- 瑙掑害骞虫粦涓庨槻杩囧啿鍙傛暟 ---
#define MAX_SPIN_ERR        2.0f   // 鍘熷湴瀵归綈鏃剁殑鏈€澶ц緭鍑鸿搴?搴?銆傝秺灏忚浆寰楄秺鏌斿拰锛屽缓璁?20-40锛屽交搴曡В鍐冲師鍦版墦杞繃鍐诧紒
#define MAX_APPROACH_ERR    4.0f   // 鐩寸嚎閫艰繎鏃剁殑鏈€澶ц浆瑙?搴?銆傞槻姝㈣溅瀛愬湪琛岃繘涓寷鐑堝彉閬撱€?
#define ANGLE_FILTER_ALPHA  0.3f    // 瑙掑害婊ゆ尝绯绘暟(0~1)銆傝秺灏忚秺涓濇粦锛岃秺澶ц秺璺熸墜銆傞槻姝㈠湪鐐规梺杈规娊鎼愩€?

static float s_prev_err_degree = 0.0f; // 鐢ㄤ簬瑙掑害婊ゆ尝鐨勯潤鎬佸彉閲?
uint8 is_arrived = 0;  // 鍒拌揪鍒ゅ畾鐘舵€侀攣

// 灞€閮ㄩ潤鎬佸彉閲忥細鐢ㄤ簬婊ゆ尝鍘嗗彶淇濇寔涓庝笅闄嶆部妫€娴?
static uint8 s_is_aligning = 0;
static uint8 s_prev_trigger = 0;  // 鐢ㄤ簬妫€娴嬬姸鎬佹満缁撴潫鐨勭灛闂达紙涓嬮檷娌匡級
// 灞€閮ㄩ潤鎬佸彉閲忥紝鐢ㄤ簬璁板綍鍘嗗彶瑙掑害鍜岀姸鎬侀攣

void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING || g_special_action_trigger == 1) 
    {
        s_prev_err_degree = 0.0f; 
        s_is_aligning = 0; // 鐘舵€佹満鎺ョ鎴栧仠姝㈡椂锛岀‘淇濊В閿?
        return;
    }

#if IMU_CATEGORY == 3
    // 寮€灞€璧疯窇瑙掑害瀵归綈
    if (!g_start_heading_aligned)
    {
        float heading_err = NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);
        
        if (heading_err > MAX_SPIN_ERR) heading_err = MAX_SPIN_ERR;
        if (heading_err < -MAX_SPIN_ERR) heading_err = -MAX_SPIN_ERR;
        
        err_degree = heading_err;
        target_speed_set = NAV_SPEED_STOP;

        if (fabsf(NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading)) <= NAV_START_HEADING_TOLERANCE)
        {
            g_start_heading_aligned = 1;
            err_degree = 0.0f;
            s_prev_err_degree = 0.0f;
        }
        else return;
    }
#endif

    // 1. 妫€鏌ユ槸鍚﹁窇瀹屽叏閮ㄧ偣浣?
    if (g_target_idx >= nav_ram_data.point_count)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        s_is_aligning = 0;
        return;
    }

    // 2. 鑾峰彇褰撳墠鐩爣鐐规暟鎹?
    float tx = nav_ram_data.points[g_target_idx].x;
    float ty = nav_ram_data.points[g_target_idx].y;
    g_current_point_type = nav_ram_data.points[g_target_idx].point_type;

    // 3. 璁＄畻璺濈鍜屾湡鏈涗綅缃搴?
    float dx = tx - inertial_nav.x;
    float dy = ty - inertial_nav.y;
    float dist = CalcDistance(inertial_nav.x, inertial_nav.y, tx, ty);

    float target_yaw = -atan2f(dy, -dx) * 57.29578f; 
    float raw_err = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

    // 4. 鎺у埗绛栫暐锛氬厛杞啀璧?
    // 馃専 鏍稿績淇敼锛氬鏋滆窛绂诲杩戯紝鎴栬€呫€愬凡缁忚閿佸湪瀵归綈鐘舵€佷腑銆戯紝閮藉己琛岃繘鍏ュ埌杈鹃€昏緫锛侌煂?
    if (dist <= NAV_DIST_ARRIVE || s_is_aligning)
    {
        // ==========================================
        // 銆怉. 宸茬粡鍒拌揪鐩爣鐐?(鎵ц鍋滆溅 / 瀵硅)銆?
        // ==========================================
        target_speed_set = NAV_SPEED_STOP;

        if (g_current_point_type != NAV_POINT_PATH)
        {
            // 绗竴姝ワ細鍙杩涙潵浜嗭紝绔嬪埢閿佹鐘舵€侊紒鍗充究涓嬩竴甯?dist 鍙樺ぇ浜嗕篃涓嶄細閫€鍑哄幓锛?
            s_is_aligning = 1; 

            // 绗簩姝ワ細寮€濮嬩笓蹇冨榻愮壒娈婄偣鐨勮搴?
            float special_target_yaw = nav_ram_data.points[g_target_idx].target_yaw_deg;
            float special_yaw_err = NormalizeAngle(special_target_yaw - inertial_nav.relative_yaw);

            if (fabsf(special_yaw_err) > NAV_YAW_TOLERANCE)
            {
                // 闄愬箙淇濇姢锛屾俯鏌旇浆鍚?
                if (special_yaw_err > MAX_SPIN_ERR) special_yaw_err = MAX_SPIN_ERR;
                if (special_yaw_err < -MAX_SPIN_ERR) special_yaw_err = -MAX_SPIN_ERR;
                
                err_degree = special_yaw_err;
                s_prev_err_degree = err_degree; 
            }
            else
            {
                // 浣嶇疆鍒颁簡锛岃搴︿篃杞浜嗭紒姝ｅ紡瑙﹀彂鐘舵€佹満锛?
                if (g_current_point_type == NAV_POINT_CIRCLE) minefield_flag = 1;
                else if (g_current_point_type == NAV_POINT_JUMP) vision_detected_three_jump_point = 1;
                else if (g_current_point_type == NAV_POINT_BRIDGE) VisionBridgeTask_Start();
                else if (g_current_point_type == NAV_POINT_BUMP) BumpyRoad_Trigger();
                
                g_special_action_trigger = 1;
                g_target_idx++;     // 鍒囧悜涓嬩竴涓偣
                
                s_prev_err_degree = 0.0f;
                s_is_aligning = 0;  // 馃専 瀵归綈瀹屾垚锛岃В闄ら攣瀹氾紒馃専
            }
        }
        else
        {
            // 鏅€氳矾寰勭偣锛氬埌浜嗙洿鎺ュ垏涓嬩竴涓偣
            g_target_idx++;
            s_is_aligning = 0;      // 纭繚鏅€氱偣涓嶄細琚閿?
        }
    }
    else
    {
        // ==========================================
        // 銆怋. 鏈埌杈剧洰鏍囩偣 (杩樺湪璺笂)銆?
        // ==========================================
        
        // 璺濈鐐归潪甯歌繎鏃剁殑鎶芥悙淇濇姢
        if (dist < NAV_DIST_ARRIVE + 150.0f) {
            if (raw_err > 15.0f) raw_err = 15.0f;
            if (raw_err < -15.0f) raw_err = -15.0f;
        } 
        else {
            if (raw_err > MAX_APPROACH_ERR) raw_err = MAX_APPROACH_ERR;
            if (raw_err < -MAX_APPROACH_ERR) raw_err = -MAX_APPROACH_ERR;
        }

        err_degree = ANGLE_FILTER_ALPHA * raw_err + (1.0f - ANGLE_FILTER_ALPHA) * s_prev_err_degree;
        s_prev_err_degree = err_degree;

        // 妫€鏌ヨ溅澶存槸鍚﹀鍑嗙洰鏍囩偣
        if (fabsf(NormalizeAngle(target_yaw - inertial_nav.relative_yaw)) > NAV_YAW_TOLERANCE)
        {
            target_speed_set = NAV_SPEED_STOP; // 瑙掑害鍋忓ぇ锛屽師鍦拌浆
        }
        else
        {
            // 瑙掑害鍩烘湰瀵瑰噯锛屽紑濮嬬洿绾跨Щ鍔ㄩ€艰繎
            if (dist > NAV_DIST_FAR)
            {
                target_speed_set = NAV_SPEED_FAST;
            }
            else if (dist > NAV_DIST_NEAR)
            {
                float ratio = (dist - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR);
                target_speed_set = NAV_SPEED_SLOW + (NAV_SPEED_FAST - NAV_SPEED_SLOW) * ratio;
            }
            else
            {
                target_speed_set = NAV_SPEED_SLOW;
            }
        }
    }
}
#endif   

/*杩欓噷娉ㄩ噴浜嗭紝淇濆瓨鐨勬槸鍘熸湁鐨勫埌涓€涓偣鍋滀竴娆＄殑鎺у埗閫昏緫锛屼粎浠呰兘瀹炵幇鏈€鍩烘湰鐨勫埌杈撅紝浣嗗畠鐨勬帶鍒惰窛绂绘槸绮惧噯鐨勶紝閫昏緫鏄畬澶囩殑锛屽悗闈㈡墍鏈夌殑浠ｇ爜閮藉湪鍏跺熀纭€涓婅繘琛屼紭鍖栧拰灏濊瘯*/
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


    // 1. 妫€鏌ユ槸鍚﹁窇瀹屽叏閮ㄧ偣浣?
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

    // 2. 鑾峰彇褰撳墠鐩爣鐐规暟鎹?
    float tx = nav_ram_data.points[g_target_idx].x;
    float ty = nav_ram_data.points[g_target_idx].y;
    g_current_point_type = nav_ram_data.points[g_target_idx].point_type;

    // 3. 璁＄畻璺濈鍜屾湡鏈涜搴?
    // 鍋囪 inertial_nav 鏄叏灞€缁撴瀯浣擄紝x, y, relative_yaw 瀹炴椂鏇存柊
    float dx = tx - inertial_nav.x;
    float dy = ty - inertial_nav.y;
    float dist = CalcDistance(inertial_nav.x, inertial_nav.y, tx, ty);

    // 璁＄畻鏈熸湜鏂逛綅瑙?(atan2 杩斿洖寮у害鍊硷紝杞负瑙掑害)
    // 鏍规嵁鎻忚堪锛歑姝ｆ柟鍚戝悜鍚庯紝Y姝ｆ柟鍚戝悜鍙筹紝绗﹀悎鏍囧噯绗涘崱灏斿潗鏍囨棆杞€?
    float target_yaw = -atan2f(dy, -dx) * 57.29578f; 
    
    // err_degree = 鏈熸湜 - 瀹為檯
    err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

    // 5. 鎺у埗绛栫暐锛氬厛杞啀璧?
    if (dist <= NAV_DIST_ARRIVE)
    {
        // --- A. 鍒拌揪鐩爣鐐?---
        target_speed_set = NAV_SPEED_STOP;
        
        #if DEBUG_LOG_ENABLE
        printf("[Nav] Arrived Point[%d] Type[%d]\r\n", g_target_idx, g_current_point_type);
        #endif

        if (g_current_point_type != NAV_POINT_PATH)//澶勭悊鐗规畩鐐?
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
        // --- B. 鏈埌杈剧洰鏍囩偣 ---
        // 鍏堟鏌ヨ搴︽槸鍚﹀鍑?
        if (fabsf(NormalizeAngle(err_degree)) > NAV_YAW_TOLERANCE)
        {
            // 瑙掑害鍋忓樊杈冨ぇ锛屽厛鍘熷湴鏃嬭浆
            target_speed_set = NAV_SPEED_STOP;
            #if DEBUG_LOG_ENABLE
            printf("[Nav] Rotating to target, err: %.2f\r\n", err_degree);
            #endif
        }
        else
        {
            // 瑙掑害鍩烘湰瀵瑰噯锛屽紑濮嬬Щ鍔?
            if (dist > NAV_DIST_FAR)
            {
                // 杩滅▼娈碉細婊￠€熻椹?
                target_speed_set = NAV_SPEED_FAST;
            }
            else if (dist > NAV_DIST_NEAR)
            {
                // 鍑忛€熸锛氱嚎鎬ф彃鍊煎噺閫?
                float ratio = (dist - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR);
                target_speed_set = NAV_SPEED_SLOW + (NAV_SPEED_FAST - NAV_SPEED_SLOW) * ratio;
            }
            else
            {
                // 绮惧噯閫艰繎娈碉細鏋佷綆閫?
                target_speed_set = NAV_SPEED_SLOW;
            }
        }
    }
}
*/
// 銆愪娇鐢ㄨ鏄庛€?
//  // 鎯澶嶇幇鎺у埗寰幆 (寤鸿鏀惧湪 20ms 瀹氭椂鍣ㄤ腑)
//         // if (timer_20ms_flag) {
//             NavReplay_Process(); 
//         // }

//         // === 澶勭悊鐗规畩鐐归€昏緫 ===
//         if (g_replay_state == REPLAY_RUNNING && g_special_action_trigger)
//         {
//             switch (g_current_point_type)
//             {
//                 case NAV_POINT_CIRCLE:
//                     // 鏆傚仠澶嶇幇锛屾墽琛岃浆鍦堢姸鎬佹満
//                     // Run_Circle_Task();
//                     // 浠诲姟瀹屾垚鍚庢竻闄ゆ爣蹇?
//                     break;
//                 case NAV_POINT_JUMP:
//                     // 鍙湁鍦ㄧ偣绫诲瀷涓鸿烦璺冪偣鏃讹紝鍙兘闇€瑕佸姞閫熷啿杩囧幓
//                     // Override_Speed_For_Jump();
//                     break;
//                 // ... 鍏朵粬绫诲瀷
//             }
//         }
        
//         // 搴曞眰鐢垫満鎺у埗 (浣跨敤 target_speed_set 鍜?err_degree)
//         // Motor_Control(target_speed_set, err_degree);

#if GNSS_NAV == 1
NavReplayState_e g_gps_replay_state = REPLAY_IDLE;
uint8 g_gps_current_point_type = NAV_POINT_PATH;
uint8 g_gps_special_action_trigger = 0;

static uint16 g_gps_target_idx = 0;

static uint8 g_gyro_yaw_initialized = 0;
static float g_yaw_offset_deg = 0.0f;

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
    // 浣跨敤鏍囧噯 atan2f(dy, dx) 鍖归厤鎵撶偣鍧愭爣绯?
    return GpsNormalizeCourse360(atan2f(dy, dx) * 57.2957795f);
}

static float GpsNavCurrentXmm(void)
{
    return gnss_trans.x * 1000.0f;
}

static float GpsNavCurrentYmm(void)
{
    return gnss_trans.y * 1000.0f;
}

static float GpsNav_GetCurrentHeadingDeg(void)
{
    float current_absolute_heading = 0.0f;

    if (g_gyro_yaw_initialized == 0U)
    {
        float initial_heading = 0.0f;
#if IMU_CATEGORY == 3
        initial_heading = 226.0f;//heading
#else
        initial_heading = gnss.direction;
#endif
        g_yaw_offset_deg = initial_heading - euler_angle.yaw;
        g_gyro_yaw_initialized = 1U;
        
        current_absolute_heading = initial_heading;
    }
    else
    {
        current_absolute_heading = euler_angle.yaw + g_yaw_offset_deg;
    }

    return GpsNormalizeCourse360(current_absolute_heading + GPS_NAV_HEADING_OFFSET_DEG);
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

    // 1. 鐘舵€佷笌 GPS 鏈夋晥鎬ф牎楠?
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

    // === 2. 缁堢偣闃插啿杩囧ご鍒ゅ畾 ===
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
            // 鎶曞奖娉曪細濡傛灉杞﹀瓙瓒婅繃浜嗗€掓暟绗簩涓偣鍒扮粓鐐圭殑杩炵嚎锛岃鏄庡啿绾夸簡
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
        // === 3. 鏋佺畝 Pure Pursuit锛氬彧瀵荤洰鏍囷紝缁濅笉鍥炲ご ===
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
                // 鎶曞奖娉曪細鍒ゅ畾鏄惁鍦ㄧ墿鐞嗕笂璺戣繃浜嗚繖涓偣
                if (len_sq > 0.001f && dot >= len_sq) is_passed = 1;
            }

            // 鏍稿績閫昏緫锛氬鏋滅偣鍦?2.5绫?鍦堝唴锛屾垨鑰呭凡缁忚鐢╁湪韬悗锛岀珛鍒绘姏寮冨畠锛佸悆涓嬩竴涓紒
            if (dist_current < GPS_NAV_LOOKAHEAD_DIST || is_passed)
            {
                g_gps_target_idx++;
                skips++;
                if (skips >= 3) break; // 闃茶法鐣岄攣锛氭瘡鍛ㄦ湡鏈€澶氬悆 3 涓偣
            }
            else
            {
                // 鎵惧埌鍓嶆柟鐨勭偣浜嗭紒鐩存帴閿佸畾鐩爣锛屼笉鍋氫换浣曠敾铔囨坊瓒崇殑鎻掑€艰繍绠?
                break;
            }
        }
        
        target_x = nav_ram_data.points[g_gps_target_idx].x;
        target_y = nav_ram_data.points[g_gps_target_idx].y;
    }

    // === 4. 璁＄畻鑸悜璇樊 ===
    target_bearing = GpsCalcBearingDegFromNorth(cx, cy, target_x, target_y);
    raw_err_degree = NormalizeAngle(target_bearing - current_heading);
    
    // 鏍稿績鎶楀櫔 A锛氫綆閫氭护娉?
    static float s_filtered_err = 0.0f;
    float alpha = 0.25f; 
    if (g_gps_target_idx == 0 && target_speed_set == GPS_NAV_SPEED_STOP) {
        s_filtered_err = raw_err_degree; 
    } else {
        s_filtered_err = (1.0f - alpha) * s_filtered_err + alpha * raw_err_degree;
    }

    float final_err = s_filtered_err;

    // 鏍稿績鎶楀櫔 B锛氭鍖?
    if (fabsf(final_err) < 2.0f) final_err = 0.0f;

    // 鏍稿績鎶楀櫔 C锛氭毚鍔涢檺骞?
    if (final_err > 35.0f) final_err = 35.0f;
    if (final_err < -35.0f) final_err = -35.0f;

    err_degree = final_err; 
    float abs_err = fabsf(err_degree);

    // === 5. 鍔ㄦ€侀€熷害鎺у埗锛堟彁楂樹繚搴曞姩鍔涢槻鍗℃锛?===
    float dist_to_target = CalcDistance(cx, cy, target_x, target_y);
    float base_speed;
    
    if (dist_to_target > GPS_NAV_DIST_NEAR && abs_err < 10.0f) {
        base_speed = GPS_NAV_SPEED_FAST; 
    } else {
        base_speed = GPS_NAV_SPEED_SLOW; 
    }

    // 寮亾鍔ㄦ€侀檷閫?
    float speed_factor = 1.0f - (abs_err / 40.0f); 
    
    // 銆愰噸瑕佷慨鏀广€戯細灏嗕繚搴曟帹鍔涗粠 0.2 鎻愰珮鍒颁簡 0.4锛?
    // 闃叉澶ц浆寮椂搴曠洏杈撳嚭鐨?PWM 澶綆锛屽鑷村皬杞︾數鏈烘棤鍔涖€佸師鍦伴渿棰?
    if (speed_factor < 0.40f) speed_factor = 0.40f; 

    target_speed_set = base_speed * speed_factor; 
}
#endif


