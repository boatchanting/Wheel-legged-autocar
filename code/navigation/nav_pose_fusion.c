#include "nav_pose_fusion.h"
#include "inertial_nav.h"
#include "gnss_transform.h"
#include "nav_replay/nav_replay.h"
#include "math.h"
#include "nav_replay_route_table.h"

NavPoseFusion_t nav_pose_fusion = {0};
static uint8_t last_replay_running = 0;
static uint32_t heading_align_timer = 0;
static float cog_sum = 0.0f;
static uint32_t cog_count = 0;

void NavPoseFusion_StartLock(float current_x, float current_y) {
    nav_pose_fusion.fused_x_mm = current_x;
    nav_pose_fusion.fused_y_mm = current_y;
    nav_pose_fusion.fused_vx_body = 0.0f;
    nav_pose_fusion.gps_weight = 0.0f;
    nav_pose_fusion.gps_valid = 0;
    nav_pose_fusion.v_err_filtered = 0.0f;
    
#if USE_GPS_INS_FUSION
#if HEADING_ALIGN_MODE == 1
    // 写死模式
    nav_pose_fusion.heading0 = NAV_REPLAY_START_HEADING_DEG;
    nav_pose_fusion.heading_lock = 1;
#elif HEADING_ALIGN_MODE == 2
    // 均值模式，等待后续收集
    nav_pose_fusion.heading0 = 0.0f;
    nav_pose_fusion.heading_lock = 0;
    heading_align_timer = 0;
    cog_sum = 0.0f;
    cog_count = 0;
#endif
#endif
}

void NavPoseFusion_Update(float delta_t) {
#if USE_GPS_INS_FUSION
    uint8_t current_running = (g_replay_state == REPLAY_RUNNING) ? 1 : 0;
    
    // 检测 REPLAY_RUNNING 上升沿，触发初始锁定
    if (current_running && !last_replay_running) {
        NavPoseFusion_StartLock(inertial_nav.x, inertial_nav.y);
    }
    
    if (current_running) {
#if HEADING_ALIGN_MODE == 2
        // 处理航向角均值锁定
        if (!nav_pose_fusion.heading_lock) {
            heading_align_timer++;
            // 假设10ms调用一次，前2000ms（2秒）收集稳定且速度>0.5m/s时的COG
            if (heading_align_timer < 200) {
                if (gnss.speed > 1.8f) { // 1.8km/h = 0.5m/s
                    cog_sum += gnss.direction;
                    cog_count++;
                }
            } else {
                if (cog_count > 0) {
                    nav_pose_fusion.heading0 = cog_sum / (float)cog_count;
                } else {
                    nav_pose_fusion.heading0 = 0.0f; // 兜底
                }
                nav_pose_fusion.heading_lock = 1;
            }
        }
#endif
        
        // 处理核心融合
        if (nav_pose_fusion.heading_lock) {
            float v_gps = gnss.speed; 
            // 航向角差值 (转化为弧度)。注意：必须使用车身实时绝对地理航向！
            float current_heading = nav_pose_fusion.heading0 + inertial_nav.relative_yaw;
            float delta_angle = (gnss.direction - current_heading) * 0.01745329f;
            float v_gps_body = v_gps * cosf(delta_angle);
            // 单位转换: km/h -> mm/s
            v_gps_body *= 277.7778f;
            
            // X轴向后为正
            float gps_vx_ins = -v_gps_body;
            
            float v_err = gps_vx_ins - inertial_nav.vx_body;
            nav_pose_fusion.v_err_filtered += 0.05f * (v_err - nav_pose_fusion.v_err_filtered);
            
            nav_pose_fusion.fused_vx_body = inertial_nav.vx_body + nav_pose_fusion.gps_weight * nav_pose_fusion.v_err_filtered;
            
            // 位移积分
            float dx_body = nav_pose_fusion.fused_vx_body * delta_t;
            float dy_body = inertial_nav.vy_body * delta_t; 
            
            float yaw_rad = inertial_nav.relative_yaw * 0.01745329f;
            float sin_yaw = sinf(yaw_rad);
            float cos_yaw = cosf(yaw_rad);
            
            nav_pose_fusion.fused_x_mm += dx_body * cos_yaw - dy_body * sin_yaw;
            nav_pose_fusion.fused_y_mm += dx_body * sin_yaw + dy_body * cos_yaw;
        }
    } else {
        // 未发车/遥控打点模式：完全同步惯导
        nav_pose_fusion.fused_x_mm = inertial_nav.x;
        nav_pose_fusion.fused_y_mm = inertial_nav.y;
        nav_pose_fusion.fused_vx_body = inertial_nav.vx_body;
    }
    
    last_replay_running = current_running;
#endif
}

void NavPoseFusion_UpdateWeight(float curvature) {
#if USE_GPS_INS_FUSION
    // 简单的线性/分段映射，曲率小（直道）权重低，曲率大（弯道）权重高
    // 假设 curvature 通常在 0.0 ~ 0.005 之间
    float abs_c = fabsf(curvature);
    float target_weight = 0.00f; // 默认直道底噪设为0，完全信任轮速计，屏蔽GPS测速滞后
    
    if (abs_c > 0.0001f) {
        target_weight = (abs_c - 0.0001f) * 100.0f; // 缩放系数根据实际测试调整
    }
    
    if (target_weight > 0.8f) {
        target_weight = 0.8f; // 最大权重限幅
    }
    
    // 一阶低通滤波平滑过渡
    nav_pose_fusion.gps_weight += 0.05f * (target_weight - nav_pose_fusion.gps_weight);
#endif
}
