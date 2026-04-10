#ifndef __BRIDGE_H__
#define __BRIDGE_H__

#include "zf_common_headfile.h" 
#include "../config/car_select.h"//根据小车选择配置不同的PID参数
#if CAR_SELECT == 0 // 0代表学习板小车 板子 学习板 v1.2
// 【单边桥高姿态专属 PID 参数】 (用于抬腿时的线性插值，防止原地震荡)
// 中环 (角度环) : 抬高后惯性变大，需要稍微加大推力，大幅增加阻尼
#define ANG_KP_HIGH          22.0f   // [原20.0] 微增
#define ANG_KD_HIGH          12.0f   // [原8.0]  显著增加阻尼，防止荡秋千
#define ANG_MECH_ZERO_HIGH   2.6f    // [原2.6]  实车需重新测定最高姿态时的绝对平衡点

// 内环 (角速度环) : 抬高后机械变软，减小响应防止高频共振"嗡嗡"响
#define GYR_KP_HIGH          -14.0f  // [原-18.0] 减小绝对值

//舵机速度环
#define SERVO_SPEED_KP_HIGH         -4.0f   // [原-6.5] 微减
#define SERVO_SPEED_KI_HIGH   -0.06f    // [原-0.03]

// 转向环 : 重心变高容易侧翻，转向必须变柔和
#define TURN_ANG_KP_HIGH     -10.0f   // [原-12.0] 减小绝对值
#define TURN_GYR_KP_HIGH     -2.0f   // [原-3.0]  减小绝对值
#endif

#if CAR_SELECT == 3 // 3代表 【2026/3/30新车】 对应板子 【2026/03/24 最后的舵机v腿】
// 【单边桥高姿态专属 PID 参数】 (用于抬腿时的线性插值，防止原地震荡)
// 中环 (角度环) : 抬高后惯性变大，需要稍微加大推力，大幅增加阻尼
#define ANG_KP_HIGH          -27.5f   // [原-25.0] 微增
#define ANG_KD_HIGH          -10.0f   // [原-6.5]  显著增加阻尼，防止荡秋千
#define ANG_MECH_ZERO_HIGH   -1.5f    // [原-1.5]  实车需重新测定最高姿态时的绝对平衡点

// 内环 (角速度环) : 抬高后机械变软，减小响应防止高频共振"嗡嗡"响
#define GYR_KP_HIGH          -6.0f  // [原-7.5] 减小绝对值

//舵机速度环
#define SERVO_SPEED_KP_HIGH         -3.5f   // [原-6.5] 微减
#define SERVO_SPEED_KI_HIGH   -0.09f    // [原-0.03]

// 转向环 : 重心变高容易侧翻，转向必须变柔和
#define TURN_ANG_KP_HIGH     -5.0f   // [原-6.0] 减小绝对值
#define TURN_GYR_KP_HIGH     13.0f   // [原20.0]  减小绝对值
#endif
// ============================================================================
// 单边桥状态机枚举
// ============================================================================
typedef enum {
    BRIDGE_STATE_IDLE = 0,      // 0: 空闲状态，未遇到单边桥
    BRIDGE_STATE_BRAKE,         // 1: 发现单边桥，准备减速刹车
    BRIDGE_STATE_READY,         // 2: 距离逼近，平滑抬高底盘并更新PID
    BRIDGE_STATE_CLIMB,         // 3: 驶入单边桥坡道，解开舵机限制
    BRIDGE_STATE_ON_BRIDGE,     // 4: 完全在桥面上，开启 Roll 平衡自适应
    BRIDGE_STATE_LEAVING,       // 5: 驶出单边桥，平滑降低底盘并恢复PID
    BRIDGE_STATE_COOLDOWN = 99  // 99: 冷却阶段，防止重复触发
} BridgeState_e;

// ============================================================================
// 单边桥参数配置结构体
// ============================================================================
typedef struct {
    // --- 距离参数 (单位: mm) ---
    float trigger_brake_dist;   // 距离桥多远开始刹车
    float trigger_ready_dist;   // 距离桥多远开始抬高车身
    float enter_bridge_dist;    // 驶过桥头多远认为完全上桥
    float on_bridge_length;     // 桥面的总长度
    float cooldown_distance;    // 冷却距离

    // --- 速度参数 (依据你的控制单位，如 -60 约 20cm/s) ---
    float speed_brake;          // 刹车时的目标速度
    float speed_ready;          // 准备上桥的目标速度
    float speed_climb;          // 爬坡/过桥时的目标速度
    float speed_normal;         // 离开桥后的正常目标速度

    // --- 姿态高度参数 ---
    float height_normal;        // 平地正常高度 (如 5.0f)
    float height_bridge;        // 过桥时的目标高度 (如 20.0f)
    float height_step_rise;     // 抬高步长 (每次调用增加的值)
    float height_step_drop;     // 降低步长 

    // --- 舵机动态刚度 ---
    int32 servo_acc_bridge;     // 过桥时允许舵机瞬间响应的最大斜率
    int32 servo_dec_bridge;
} BridgeParams_t;

// ============================================================================
// 外部接口声明
// ============================================================================
extern BridgeState_e current_bridge_state;

// 初始化参数
void Bridge_Init(void);

// 触发单边桥 (传入摄像头/电磁预估的距离)
void Bridge_Trigger(float distance_to_bridge);

// 核心更新函数 (建议放在 20ms 等周期任务中调用)
void Bridge_Update(void);

// ============================================================================
// 调试与测试接口
// ============================================================================
// 测试开关：0 = 正常平地高度，1 = 抬高到单边桥高度
extern uint8_t debug_force_height_up; 

// 仅用于测试 PID 平滑升降的函数 (替代 Bridge_Update 放在 20ms 定时器中)
void Bridge_Test_Smooth_PID(void);

extern uint8_t debug_triple_bridge_test_enable;
void Bridge_Test_Triple_SingleSide_Inertial(void);

// ============================================================================
// 【新增】视觉触发接口声明
// ============================================================================
// 检查单边桥测试是否处于活动状态
bool Bridge_Test_Triple_SingleSide_Is_Active(void);
// 启动单边桥测试
void Bridge_Test_Triple_SingleSide_Start(void);
// 状态机新增：视觉触发标志位（由视觉模块设置）
extern bool vision_detected_bridge_point;

#endif // __BRIDGE_H__
