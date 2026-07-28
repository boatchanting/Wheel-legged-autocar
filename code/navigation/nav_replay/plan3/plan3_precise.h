#ifndef _PLAN3_PRECISE_H_
#define _PLAN3_PRECISE_H_

#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE   1 //use compile-time route table generated from CSV (no flash dependency)

// ========================= 控制参数宏定义 =========================
// 距离阈值 (单位: mm)
#define NAV_DIST_FAR            500.0f  // 远距离界限，全速
#define NAV_DIST_NEAR           150.0f  // 近距离界限，开始最低速 

#define NAV_DIST_ARRIVE         120.0f  // 到达捕获半径：不在点前减速，进入该范围直接切换
#define NAV_DIST_PASS_CAPTURE   250.0f  // 曾进入此范围后距离开始增大，判定为已穿过目标点
#define NAV_DIST_PASS_HYSTERESIS 30.0f  // 穿点判定的距离回升滞回，避免定位抖动误触发
#define NAV_YAW_TOLERANCE        1.0f    //转向阈值，先转再走
#define NAV_SPECIAL_ENTRY_YAW_TOLERANCE 1.0f // special state-machine entry yaw tolerance (deg)
#define NAV_START_HEADING_TOLERANCE 0.3f // 复刻起步前，绝对航向对齐阈值(度)

// 速度设定 (负数为前进，数值对应 motor rpm 或 pwm 级)
#define NAV_SPEED_FAST          (-140.0f) // 高速行驶速度
#define NAV_SPEED_SLOW          (-40.0f)  // 低速逼近速度 (期望rpm) 

#define NAV_SPEED_STOP          (0.0f)

// =================================================================
// 【性能调优宏定义区】 - 修改此处参数即可改变行驶风格
// =================================================================

// --- 1. 纯追踪 (Pure Pursuit) 导航参数 ---
#define PP_LD_MIN_CURVE        500.0f   // 弯道最小前瞻 (mm)。越小越贴线，但容易抖动。要求精度25mm建议不低于300。
#define PP_LD_MIN_STRAIGHT     1.2f     // 直道前瞻倍率。针对3m大点距，建议设为当前点距的1.1-1.5倍。
#define PP_LD_SPEED_GAIN       0.7f     // 速度增益系数。Ld = Ld_min + Speed * Gain。高速时看的更远。
#define CURVE_PREVIEW_DIST     1200.0f   // 曲率预判距离 (mm)。探测多远处的弯道，决定提早减速的时机。

// --- 2. 速度规划 (Speed Planning) 参数 ---
#define SPD_CURVE_DEADZONE     0.35f     // 曲率感应死区 (0-1)。低于此值的弯道视为直道，不减速，释放速度。
#define SPD_CURVE_EXPONENT     3.5f     // 曲率减速指数。1.0为线性，2.0为平方律。越大则轻微弯道速度越快。
#define SPD_ANGLE_PENALTY      0.08f     // 转向角度惩罚权重 (0-1)。值越小，纠偏时减速越少，动力更足。
#define SPD_ANGLE_TOLERANCE    60.0f    // 转向角度容忍门槛 (度)。角度偏差在此范围内不触发剧烈减速。

// --- 3. 丝滑滤波 (Smoothness) 参数 ---
#define FILTER_ALPHA_ANGLE     0.45f    // 角度滤波系数 (0-1)。值越大越跟手，值越小越丝滑。
#define FILTER_ALPHA_SPEED     1.0f    // 速度滤波系数 (0-1)。值越大提速越猛，值越小加速越柔和。
#define SLEW_RATE_ANGLE        35.0f    // 单次周期最大转角变化 (度)。防止电机/舵机瞬间猛打。

// 速度目标限斜率参数
#define NAV_SPEED_SLEW_EPS             1.0f    
#define NAV_SPEED_SLEW_LOW_SPEED_TH    80.0f   
#define NAV_SPEED_SLEW_FAST_DECEL_TH   220.0f  
#define NAV_SPEED_SLEW_UP_LOW          30.0f   
#define NAV_SPEED_SLEW_UP_NORMAL       45.0f   
#define NAV_SPEED_SLEW_DOWN_NORMAL     65.0f   
#define NAV_SPEED_SLEW_DOWN_FAST       95.0f   
#define NAV_SPEED_SLEW_DOWN_CROSS_ZERO 120.0f  

// ========================= 全局控制变量声明 =========================
// 这些变量由外部定义 (通常在 control.c 或 main.c)，此处引用
extern volatile float target_speed_set;
extern volatile float err_degree;
extern volatile float roll_degree;

// 暴露给外部的状态
extern NavReplayState_e g_replay_state;         // 当前复现状态
extern uint8 g_current_point_type;              // 当前正在前往/到达的点的类型
extern uint8 g_special_action_trigger;          // 特殊动作触发标志 (1: 到达特殊点，请执行动作)
// 由 main_cm7_0.c 主循环消费，避免在中断控制路径中阻塞蜂鸣。
extern volatile uint8 entry_beep_request;
extern volatile uint8 exit_beep_request;

// ========================= 函数接口 =========================

/**
 * @brief  开始复现路径
 * @note   Loads route from compile-time table when NAV_REPLAY_USE_STATIC_ROUTE_TABLE = 1
 *         会将状态置为 REPLAY_RUNNING，索引置 0
 */
void NavReplay_Start(void);

/**
 * @brief  停止复现
 * @note   速度置 0，状态置 IDLE
 */
void NavReplay_Stop(void);

/**
 * @brief  惯性导航复现控制周期函数
 * @note   建议放在 10ms 或 20ms 定时器中断或主循环中调用
 *         它会根据当前视觉/惯导融合坐标计算 target_speed_set 和 err_degree
 */
void NavReplay_Process(void);

/**
 * @brief  Load static route table (generated from CSV) into nav_ram_data
 * @return loaded point count
 */
uint16 NavReplay_LoadStaticRouteToRam(void);
#endif
