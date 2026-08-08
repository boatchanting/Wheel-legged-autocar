#ifndef CODE__PID_NEW_H__
#define CODE__PID_NEW_H__
#include "zf_common_headfile.h"
#include "../config/generated/sys_options_accel.h"//系统配置开关
#include "../config/car_select.h"//根据小车选择配置不同的PID参数

#ifndef ACCEL_FF_ENABLE
#define ACCEL_FF_ENABLE 1U      /* 加速前馈总开关兜底值；正常应由 config/sys_options.h 配置 */
#endif

#ifndef ACCEL_FF_MODE
#define ACCEL_FF_MODE 1U        /* 加速前馈模式兜底值；正常应由 config/sys_options.h 配置 */
#endif

#define ACCEL_FF_MODE_DISABLE 0U /* 关闭加速前馈 */
#define ACCEL_FF_MODE_PWM     1U /* 直接叠加 PWM 前馈 */
#define ACCEL_FF_MODE_KP      2U /* 加速时临时增强舵机速度环 Kp */

#if (ACCEL_FF_MODE != ACCEL_FF_MODE_DISABLE) && \
    (ACCEL_FF_MODE != ACCEL_FF_MODE_PWM) && \
    (ACCEL_FF_MODE != ACCEL_FF_MODE_KP)
#error "ACCEL_FF_MODE 只能为 0(关闭)、1(PWM 前馈) 或 2(Kp 增强)。"
#endif

#if CAR_SELECT ==  3 // 3代表 【2026/3/30新车】 对应板子 【2026/03/24 最后的舵机v腿】
// *************************** 【2026/03/24 最后的舵机v腿】pid参数定义开始 ***************************

// ----------------------------------------------------------------------------
// 4. 舵机速度环参数 (周期9ms date20260702)
//    作用：控制舵机的转动速度，使其平滑地达到目标位置，避免突然动作
// ----------------------------------------------------------------------------
// 当前Core0调度目标周期：9ms
#define SERVO_SPEED_KP  -4.5f   // [比例控制] 控制舵机速度响应的快慢
#define SERVO_SPEED_KI  0.0f   // [积分控制] 
// 周期换算：20ms -> 9ms，ratio=0.45，Kd /= 0.45
#define SERVO_SPEED_KD  -0.17f   // [微分控制]
#define SERVO_SPEED_MAX_I  100000.0f  // [积分限幅] 限制积分项的最大值
#define SERVO_SPEED_MAX_O  2000.0f   // [输出限幅] 限制舵机速度的最大值，避免过快
#define SERVO_SPEED_COMP   0.0f   // [关键补偿] 舵机速度环的补偿值
extern float current_actual_speed;

// ----------------------------------------------------------------------------
// 2. 角度环参数 (中间环 - 周期约 5ms)
//    作用：根据期望角度(来自机械零点+速度环)，计算出需要的角速度。
//    这是维持直立最关键的一环。
// ----------------------------------------------------------------------------
// 当前Core0调度目标周期：3ms
#define ANG_KP      -12.0f   //[直立刚度] 类似于弹簧的硬度。值太小车软绵绵扶不正；值太大车会剧烈低频抖动。
#define ANG_KI      0.0f    // [一般不用] 平衡车本身是不稳定系统，加积分容易导致无法直立，除非是完全静态的高精度控制。
// 周期换算：5ms -> 3ms，ratio=0.6，Kd /= 0.6
#define ANG_KD      -13.3333f    //[直立阻尼] 极重要！类似于减震器。值太小车会有余震；值太大车反应迟钝且有高频噪音。

#define ANG_MAX_I   0.0f    // 积分限幅
#define ANG_MAX_O   8000.0f // [最大角速度] 限制期望的旋转速度，防止电机指令过大。

// [关键补偿] 机械零点 (Mechanical Zero)
// 理想情况下0度是平衡点。但因电池安装、传感器贴歪等原因，实际平衡点可能是 -1.5度。
// 调试方法：如果车总是往“前”跑，说明它觉得自己后仰了，需要减小这个值；反之增大。
#define ANG_MECH_ZERO  -3.0f   //机械零点变化后看上面的调试方法微调

// ----------------------------------------------------------------------------
// 3. 角速度环参数 (最内环 - 周期约 1ms)
//    作用：直接控制电机PWM，让车身角速度迅速跟随角度环的指令。
//    这一环必须响应最快。
// ----------------------------------------------------------------------------
#define GYR_KP      -16.0f    // [响应速度] 决定了电机对旋转的抵抗力。
#define GYR_KI      0.0f    // [一般不用] 响应太快，积分来不及反应，反而造成滞后。
#define GYR_KD      0.0f    // [消除抖动] 抑制高频噪声和电机抖动。

#define GYR_MAX_I   0.0f    
#define GYR_MAX_O   9000.0f // [PWM满幅] 满是10000，这里留点余量设3000。

// [关键补偿] 电机死区 (Dead Zone Voltage)
// 直流电机存在静摩擦，PWM太小(如200)时不转。
// 如果PID算出输出100，加上死区300，实际给400，车轮正好能动，消除了低速时的非线性迟滞。
#define GYR_DEAD_ZONE  0.0f  

// [传感器误差] 陀螺仪静态零偏 (需静止测量)
#define GYRO_SENSOR_OFFSET  0.0f 

// ----------------------------------------------------------------------------
// 5. 转向角度环参数 (外环 - 周期6ms)
//    作用：根据视觉/编码器计算的角度误差，生成期望转向角速度
//    特性：无积分项（避免转向累积误差），支持赛道场景自适应增益
// ----------------------------------------------------------------------------
// 当前Core0调度目标周期：3ms
#define TURN_ANG_KP     -8.0f   // -12[转向刚度] 值越大转向越灵敏，但易振荡
#define TURN_ANG_KI     0.0f   // [一般不用] 无积分项，避免转向累积误差
#define TURN_ANG_KD     0.0f   // [转向阻尼] 抑制转向超调，值过大会导致响应迟钝
#define TURN_ANG_MAX_I  0.0f    // [一般不用] 无积分项，避免转向累积误差
#define TURN_ANG_DEAD_ZONE 0.0f // [死区] 消除低速时的非线性迟滞
#define TURN_ANG_MAX_O  8000.0f  // [角速度限幅] 限制最大期望转向角速度

// ----------------------------------------------------------------------------
// 6. 转向角速度环参数 (内环 - 周期2ms)
//    作用：快速跟踪期望角速度，直接输出转向专用PWM
// ----------------------------------------------------------------------------
// 当前Core0调度目标周期：1ms
#define TURN_GYR_KP     20.0f    // 35[响应速度] 决定转向电机响应刚度
#define TURN_GYR_KI     0.0f     // [一般不用] 无积分项，避免转向累积误差
// 周期换算：2ms -> 1ms，ratio=0.5，Kd /= 0.5
#define TURN_GYR_KD     16.0f     // 12[抖动抑制] 消除高频抖动
#define TURN_GYR_MAX_I  0.0f     // [一般不用] 无积分项，避免转向累积误差
#define TURN_GYR_DEAD_ZONE 0.0f  // [死区] 消除低速时的非线性迟滞
#define TURN_GYR_MAX_O  8000.0f  // [PWM限幅] 普通赛道转向PWM上限
#define TURN_GYR_MAX_O_BRIDGE 9000.0f // [单边桥限幅] 单边桥需更大转向力矩

// ----------------------------------------------------------------------------
// 7. 单边桥 Rolling (横滚) 自适应平衡环参数
//    作用：过单边桥时，根据横滚角偏差，自动调整左右腿高度差，保持车身水平。
//    策略：一边不动，一边缩短 (Drop-Leg Strategy)
// ----------------------------------------------------------------------------
#define ROLL_KP      44.0f   // 44[响应力度] 决定对抗倾斜的猛烈程度
#define ROLL_KI      0.0f    // [一般不用] 单边桥是瞬态过程，不需要积分消除静差
#define ROLL_KD      7.0f    // 7[阻尼] 抑制车身左右晃动，防止超调
#define ROLL_MAX_I   0.0f    
#define ROLL_MAX_O   1000.0f // [PWM限幅] 限制单次调整的最大舵机PWM值 (假设舵机满量程10000)
#define ROLL_MECH_ZERO 0.0f  // [机械零点] 理想水平是0度

// *************************** 【2026/03/24 最后的舵机v腿】pid参数定义结束***************************
#endif

// ============================================================================
// 1. PID 参数结构体
// ============================================================================
typedef struct {
    // --- 调节参数 ---
    float kp;               
    float ki;               
    float kd;               

    // --- 限幅参数 ---
    float max_output;       
    float max_integral;     

    // --- 补偿参数 (零点/死区) ---
    float compensation;     

    // --- 运行时变量 (Runtime Variables) ---
    float error;            // e(k)   : 当前误差
    float last_error;       // e(k-1) : 上一次误差
    float prev_error;       // e(k-2) : [新增] 上上次误差 (预留给增量计算)
    
    float error_integral;   // 积分累加
    float output;           // 最终输出

} PID_Param_t;

typedef enum {
    CONTROL_MODE_NORMAL = 0U,
    CONTROL_MODE_ACCEL  = 1U,
    CONTROL_MODE_BRAKE  = 2U
} ControlMode_e;

typedef struct {
    float servo_speed_kp;
    float servo_speed_ki;
    float servo_speed_kd;
    float servo_speed_max_output;
    float servo_speed_max_integral;
    float servo_speed_compensation;

    float angle_kp;
    float angle_ki;
    float angle_kd;
    float angle_max_output;
    float angle_max_integral;
    float angle_compensation;

    float gyro_kp;
    float gyro_ki;
    float gyro_kd;
    float gyro_max_output;
    float gyro_max_integral;
    float gyro_compensation;

    float turn_angle_kp;
    float turn_angle_ki;
    float turn_angle_kd;
    float turn_angle_max_output;
    float turn_angle_max_integral;
    float turn_angle_compensation;

    float turn_gyro_kp;
    float turn_gyro_ki;
    float turn_gyro_kd;
    float turn_gyro_max_output;
    float turn_gyro_max_integral;
    float turn_gyro_compensation;

    float roll_kp;
    float roll_ki;
    float roll_kd;
    float roll_max_output;
    float roll_max_integral;
    float roll_compensation;

    float brake_gain_light;
    float brake_gain_med;
    float brake_gain_heavy;
    float brake_max_light;
    float brake_max_med;
    float brake_max_heavy;
    float brake_ramp_up_light;
    float brake_ramp_up_med;
    float brake_ramp_up_heavy;
    float brake_ramp_down;

    float accel_ff_gain;
    float accel_ff_max;
    float accel_ff_ramp_up;
    float accel_ff_ramp_down;

    float servo_exec_acc_limit;
    float servo_exec_dec_limit;
    float servo_exec_boost_from_speed;
    float servo_exec_boost_from_error;
    float servo_exec_boost_max;
} ControlProfile_t;

// ============================================================================
// 2. 全局声明
// ============================================================================
extern PID_Param_t pid_servo_speed;//速度环(舵机)pid参数
extern PID_Param_t pid_angle;//角度环(pid参数)
extern PID_Param_t pid_gyro;//加速度环pid参数
extern PID_Param_t pid_turn_angle;//转向角度环pid参数
extern PID_Param_t pid_turn_gyro;//转向角速度环pid参数
extern PID_Param_t pid_roll; //Rolling环pid参数

extern volatile float now_speed;        // 当前速度 (来自编码器)
extern volatile float now_angle;        // 当前角度 (来自IMU)
extern volatile float now_gyro;         // 当前角速度 (来自IMU)

extern float speed_loop_out;    // 速度环的输出 (目标角度)
extern float angle_loop_out;    // 角度环的输出 (目标角速度)
extern float gyro_loop_out;     // 角速度环的输出 (目标角加速度)
// 转向环输出变量
extern volatile float turn_angle_loop_out; // 转向角度环输出（期望角速度）
extern volatile float turn_gyro_loop_out; // 转向角速度环输出（PWM）

extern volatile float final_motor_pwm;  // 最终输出到电机的PWM值

extern volatile float target_speed_set;
extern volatile uint8 profile_switch_beep_request; // 复刻模式下PID切换蜂鸣请求，ISR中消费后清零
extern volatile ControlMode_e g_control_mode_requested;
extern volatile ControlMode_e g_control_mode_applied;
extern ControlProfile_t g_control_profile_active;
extern uint8_t roll_balance_enable; // rolling环使能开关
extern volatile float g_turn_active_roll_height_delta_cm; // 转向主动侧倾单侧目标高度差，单位 cm
extern volatile float g_turn_active_roll_request_degree; // 转向主动侧倾未斜率限制前的目标横滚角，单位 deg
extern volatile float g_turn_active_roll_forward_speed_mps; // 转向主动侧倾计算用纵向速度，单位 m/s
extern volatile float g_turn_active_roll_yaw_rate_radps; // 转向主动侧倾计算用实际 yaw 角速度，单位 rad/s
extern volatile float g_turn_active_roll_lateral_accel_mps2; // v*w 得到的向心加速度，单位 m/s^2
extern volatile uint8 g_brake_active;
extern volatile uint8 g_reverse_brake_active;

// 全局刹车前馈参数
#define BRAKE_SPEED_DEADBAND     15.0f     /* 当前速度绝对值低于该值时不启用刹车前馈，避免低速抖动和符号噪声 */
#define BRAKE_LOW_SPEED_TH       40.0f    /* 普通速度差触发刹车时的低速保护阈值，低于该速度只允许轻刹 */
#define BRAKE_ZERO_TARGET_MAX    10.0f    /* 目标速度绝对值低于该值时，允许进入零速停车迟滞区 */
#define BRAKE_ZERO_HOLD_ENTER    18.0f    /* 刹停过程中速度低于该值时进入零速迟滞区并清空刹车前馈 */
#define BRAKE_ZERO_HOLD_EXIT     30.0f    /* 零速迟滞区退出阈值；只有速度重新明显离开零区才允许再次建压 */
#define BRAKE_ERR_MIN            40.0f    /* 启用比例判定前的最小绝对速度差，避免速度很小时比例被放大误判 */
#define BRAKE_ERR_MED_MIN        80.0f    /* 中刹最小绝对速度差，避免低速小幅速度差仅因比例大而升级 */
#define BRAKE_ERR_HEAVY_MIN      150.0f   /* 重刹最小绝对速度差，必须有足够大的真实降速需求 */
#define BRAKE_MED_SPEED_TH       120.0f   /* 普通减速进入中刹的当前速度下限 */
#define BRAKE_HEAVY_SPEED_TH     220.0f   /* 普通减速进入重刹的当前速度下限 */
#define BRAKE_TARGET_DECEL_MIN   40.0f    /* 目标速度下降超过该值才认为是主动减速指令；调大可减少弯前误刹，调小会更早介入收速 */
#define BRAKE_OVERSPEED_ERR_MIN  60.0f    /* 持续超速判定阈值：实际速度绝对值比目标速度绝对值大这么多才算真超速 */
#define BRAKE_OVERSPEED_HOLD_TICKS 3U     /* 超速持续 tick 数，当前刹车前馈约 9ms 调用一次；3U 约等于 27ms，调大可抑制瞬时噪声误刹 */
#define BRAKE_CH5_LIGHT_SPEED    80.0f    /* CH5 急停低于该速度只给轻刹，避免低速急停过猛 */
#define BRAKE_CH5_MED_SPEED      220.0f   /* CH5 急停低于该速度给中刹，高于该速度才给重刹 */
#define BRAKE_RATIO_LIGHT        0.18f    /* 轻刹触发比例：速度差达到当前速度的 18% 才进入轻刹；调大可减少轻微速度差触发 */
#define BRAKE_RATIO_MED          0.28f    /* 中刹触发比例：速度差达到当前速度的 28% 才进入中刹；调大可降低弯前中刹概率 */
#define BRAKE_RATIO_HEAVY        0.55f    /* 重刹触发比例：速度差达到当前速度的 55% 才进入重刹，CH5 急停不受此限制 */
#define BRAKE_GAIN_LIGHT         4.0f     /* 轻刹前馈增益，输出约为 -gain * 当前速度 */
#define BRAKE_GAIN_MED           10.0f    /* 中刹前馈增益，输出约为 -gain * 当前速度 */
#define BRAKE_GAIN_HEAVY         22.0f    /* 重刹前馈增益，主要用于 CH5 急停或速度差很大的情况 */
#define NAV_HARD_BRAKE_GAIN      80.0f    /* 导航强停刹增益，仅由科目二雷区刹车准备圆请求，明显强于普通重刹 */
#define BRAKE_MAX_LIGHT          800.0f   /* 轻刹前馈 PWM 最大幅值，限制轻微减速时的反向力矩 */
#define BRAKE_MAX_MED            1600.0f  /* 中刹前馈 PWM 最大幅值，限制普通减速时的反向力矩 */
#define BRAKE_MAX_HEAVY          3500.0f  /* 重刹前馈 PWM 最大幅值，限制急停时的最大反向力矩 */
#define NAV_HARD_BRAKE_MAX_PWM   8200.0f  /* 导航强停刹最大反向 PWM；只提高高速刹车上限，不继续放大中低速刹车比例 */
#define BRAKE_RAMP_UP_LIGHT      120.0f   /* 轻刹输出每次更新的最大上升步长，数值越小刹车介入越柔 */
#define BRAKE_RAMP_UP_MED        300.0f   /* 中刹输出每次更新的最大上升步长 */
#define BRAKE_RAMP_UP_HEAVY      700.0f   /* 重刹输出每次更新的最大上升步长，急停时允许更快建立制动力 */
#define NAV_HARD_BRAKE_RAMP_UP   2200.0f  /* 导航强停刹建压步长，保证进入雷区准备圆后能快速建立制动力 */
#define BRAKE_RAMP_DOWN          800.0f   /* 刹车前馈退出时每次更新的最大回落步长，数值越大释放越快 */
#define NAV_HARD_BRAKE_RELEASE_SPEED 35.0f /* 当前速度低于该值时释放导航强停刹，避免中心附近低速反抽和原地抽搐 */
#define NAV_HARD_BRAKE_LIFE_TICKS 4U      /* 导航强停请求保持 tick，桥接导航周期和 9ms 刹车前馈周期 */
#define BRAKE_SERVO_PROTECT_PWM_TH 2500.0f /* 刹车前馈超过该值时启用舵机刹车姿态保护，限制速度环把车身继续压成后坐 */
#define BRAKE_SERVO_BACK_SIT_SIGN -1.0f    /* 后坐方向标定：1 表示 speed_adj 为正会后坐；若实车方向相反，改成 -1.0f */
#define BRAKE_SERVO_BACK_SIT_LIMIT 0.0f   /* 强刹时允许保留的后坐方向 speed_adj，上调会更贴近原速度环，下调更防后坐蹭地 */
#define BRAKE_SERVO_ANTI_BACK_SIT_DUTY 180.0f /* 强刹时主动给一点反后坐支撑；太小仍会后坐，太大可能前栽，建议 120~260 之间试 */
#define BRAKE_TURN_ROLL_CLEAR_PWM_TH 2500.0f /* 刹车前馈超过该值时清主动侧倾，防止强刹叠加侧倾压低单侧车身 */
// 全局加速前馈参数
#define ACCEL_FF_ERR_MIN         30.0f    /* 目标速度绝对值比当前速度绝对值至少大这么多，才认为速度没跟上 */
#define ACCEL_FF_TARGET_STEP_MIN 30.0f    /* 目标速度绝对值相对上一周期至少增加这么多，才认为是急加速请求 */
#define ACCEL_FF_SPEED_DEADBAND  15.0f    /* 目标/实际速度低于该值时视为低速死区，避免零点噪声误触发 */
#define ACCEL_FF_GAIN            10.0f    /* 加速前馈增益，输出约为 gain * 速度缺口；起步还慢先小步加大，若抬头/冲击明显则回退 */
#define ACCEL_FF_MAX             3000.0f  /* 加速前馈 PWM 最大幅值，限制起步/提速时的额外力矩；只限制峰值，不决定建立速度 */
#define ACCEL_FF_RAMP_UP         800.0f   /* 加速前馈每个 9ms 更新周期允许增加的最大 PWM；调大起步更有劲，过大会有突兀冲击 */
#define ACCEL_FF_RAMP_DOWN       700.0f   /* 加速前馈退出时每个 9ms 更新周期允许释放的最大 PWM */
#define ACCEL_FF_START_WINDOW_MS 800U     /* 复刻刚进入 RUNNING 后允许起步前馈的时间窗口；起步补偿不够可适当加大 */
#define ACCEL_FF_BOOST_WINDOW_MS 550U     /* 运行中目标速度明显抬升后，保持加速前馈判定的短窗口；影响出弯/直道提速补偿持续时间 */
#define ACCEL_FF_UPDATE_PERIOD_MS 9U      /* Accel_Feedforward_Update() 当前在 9ms 速度控制段调用 */
#define ACCEL_FF_SIGN            1.0f     /* 前馈符号校正；若实车表现为减速，先改为 -1.0f，不要加大增益 */
#define ACCEL_FF_BUZZER_PWM_TH   150.0f   /* 大幅加速前馈蜂鸣阈值；调试任意前馈是否输出时可临时降到 150 */
#define ACCEL_FF_BUZZER_ON_MS    40U      /* 大幅加速前馈触发后的蜂鸣持续时间，ISR 中按 1ms 计数 */
#define ACCEL_FF_BUZZER_COOLDOWN_MS 500U  /* 蜂鸣冷却时间，避免前馈持续较大时连续响 */
#define BRAKE_ACCEL_INHIBIT_PWM  500.0f   /* 加速前馈计算阶段的刹车屏蔽阈值；刹车前馈小于该值时仍允许加速前馈继续计算 */
#define BRAKE_BLEND_CUT_PWM      500.0f   /* 最终 PWM 融合阶段的刹车主导阈值；刹车达到该值后加速前馈不再参与输出 */
#define ACCEL_BLEND_KEEP_RATIO   0.35f    /* 轻刹/小窗口共存时保留的加速前馈比例；调大出弯更有力，调小更偏保守 */

// 转向主动 Rolling 参数：向心加速度先得到可执行 roll_degree，再查表生成四腿差动，Rolling 环只做小幅反馈。
#define TURN_ACTIVE_ROLL_YAW_RATE_DEAD_DPS 15.0f       /* yaw 角速度死区，单位 deg/s；使用期望/实际较大值，调头前可提前压弯 */
#define TURN_ACTIVE_ROLL_SPEED_DEADBAND    25.0f       /* 原始纵向速度死区；低速和原地旋转时主动侧倾回零 */
#define TURN_ACTIVE_ROLL_SPEED_PREVIEW_RATIO 0.50f     /* 纵向速度预加载比例；0只看实际速度，1完全看目标速度 */
#if CAR_SELECT == 0
#define TURN_ACTIVE_ROLL_SPEED_TO_MPS      0.0034596f  /* current_actual_speed 到 m/s 的换算系数，CAR_SELECT 0 */
#elif CAR_SELECT == 2
#define TURN_ACTIVE_ROLL_SPEED_TO_MPS      0.0051830f  /* current_actual_speed 到 m/s 的换算系数，CAR_SELECT 2 */
#else
#define TURN_ACTIVE_ROLL_SPEED_TO_MPS      0.0049360f  /* current_actual_speed 到 m/s 的换算系数，其他车型默认值 */
#endif
#define TURN_ACTIVE_ROLL_FORWARD_SPEED_SIGN (-1.0f)    /* 纵向速度符号修正：当前车前进时 current_actual_speed 为负 */
#define TURN_ACTIVE_ROLL_GRAVITY_MPS2      9.80665f    /* 重力加速度，用于 atan2(a_lat, g) */
#define TURN_ACTIVE_ROLL_DEG_TO_RAD        0.0174532925f/* 角度转弧度系数 */
#define TURN_ACTIVE_ROLL_RAD_TO_DEG        57.2957795f /* 弧度转角度系数 */
#define TURN_ACTIVE_ROLL_SIGN              -1.0f        /* 主动侧倾方向修正，实车方向反了改为 -1.0f */
#define TURN_ACTIVE_ROLL_MAX               18.0f        /* 向心加速度理论侧倾角限幅，单位 deg；实际 roll_degree 还会受高度差再次限幅 */
#define TURN_ACTIVE_ROLL_RAMP_UP           0.35f       /* roll_degree 每 5ms 最大建立步长，单位 deg；调大可让调头前腿更快伸开 */
#define TURN_ACTIVE_ROLL_RAMP_DOWN         0.45f       /* roll_degree 每 5ms 最大回零步长，单位 deg */
#define TURN_ACTIVE_ROLL_HALF_TRACK_CM     11.2f        /* 左右轮距的一半，目标横滚角换算左右高度差时使用 */
#define TURN_ACTIVE_ROLL_HEIGHT_MAX_CM     3.5f        /* 对称动作时单侧最大高度差，单位 cm；收腿触底后伸腿侧会尝试补足左右高度差 */
#define TURN_ACTIVE_ROLL_SHRINK_HEIGHT_MARGIN_CM 0.20f /* 收腿侧至少保留的高度余量，低车身时提前改用另一侧伸腿 */
#define TURN_ACTIVE_ROLL_SHRINK_DUTY_MARGIN 40         /* 收腿侧 duty 安全余量，避免贴着舵机限位抖动 */
#define TURN_ACTIVE_ROLL_TARGET_DEAD_DEG   0.8f        /* 目标横滚角死区，低于该角度不查表动作，避免腿部小幅抖动 */
#define TURN_ACTIVE_ROLL_DUTY_DEADBAND     25          /* 查表差动 duty 死区，低于该值认为动作不可见并清零 */
#define TURN_ACTIVE_ROLL_FB_KEEP_RATIO     0.10f       /* 普通转向主动侧倾时保留的 Rolling 反馈比例，调小可减少前馈和反馈打架 */
#define TURN_ACTIVE_ROLL_FB_MAX_PWM        80.0f       /* 普通转向主动侧倾时 Rolling 反馈最大 PWM，防止一侧收腿抖动 */

#define ACCEL_KP_BOOST_MAX       1.60f    /* Kp 增强模式下舵机速度环 Kp 的最大倍率 */
#define ACCEL_KP_BOOST_RAMP_UP   0.18f    /* Kp 增强模式下每个 9ms 更新周期允许增加的最大倍率 */
#define ACCEL_KP_BOOST_RAMP_DOWN 0.08f    /* Kp 增强模式下每个 9ms 更新周期允许释放的最大倍率 */
#define ACCEL_KP_OUTPUT_MAX      1500.0f  /* Kp 增强模式下临时放宽后的舵机速度环输出上限 */

void Control_Profile_Init(void);//控制参数调度层初始化，默认切到 NORMAL 档
void Control_Profile_RequestMode(ControlMode_e mode);//上层只请求场景，底层按 profile 接管参数
void Control_Profile_ApplyNow(ControlMode_e mode);//立即切到指定 profile，用于上电/复位
void Control_Profile_Update1ms(void);//1ms 平滑参数更新器：当前生效参数追踪目标 profile
void PID_Param_Init(void);//pid参数初始化，同时也可以用于倒地保护
void PID_Data_Reset(void);//pid运算相关数据重置，参数不重置
void PID_Data_Clean_All(void);//pid运算相关数据全重置，参数也重置为0，即所有数据全部清空
float Float_Constrain(float val, float min, float max);//限幅函数

float Turn_Angle_Loop_Control(float angle_error);//转向角度环控制
float Turn_Gyro_Loop_Control(float target_gyro, float actual_gyro);//转向角速度环控制
float Servo_Speed_Control(float target_speed, float actual_speed, float actual_angle);//速度环(舵机)
float Angle_Loop_Control(float speed_loop_output, float actual_angle);//角度环(中环)
float Gyro_Loop_Control(float angle_loop_output, float actual_gyro);//角速度环(内环)
float Roll_Balance_Control(float actual_roll,float target_roll);//横滚平衡环控制
float Turn_Active_Roll_Target_Update(float turn_cmd, uint8 hard_clear);//转向主动侧倾目标计算
void Turn_Active_Roll_Duty_Update(float target_roll, uint8 hard_clear);//转向主动侧倾查表舵机差动

/**
 * @brief 刹车前馈更新：根据目标/实际速度差输出轻刹、中刹或重刹 PWM
 * @param target_speed 当前目标速度
 * @param actual_speed 当前实际速度
 * @param motor_enable 电机使能，0 时释放刹车前馈
 * @param jump_flag 跳跃保护标志，非 0 时释放刹车前馈
 * @return 本周期刹车前馈 PWM，通常与实际速度方向相反
 */
float Brake_Feedforward_Update(float target_speed, float actual_speed, uint8 motor_enable, uint8 jump_flag);
void Brake_Feedforward_Reset(void);//清空刹车前馈并短暂上锁，避免复位后立即被旧条件重新触发
float Brake_Feedforward_GetPwm(void);//读取当前刹车前馈 PWM，供 ISR 前馈仲裁使用
void Brake_NavHardStop_Update(uint8 active);//导航强停刹请求，主要用于科目二雷区刹车准备圆
void Brake_NavHardStop_UpdateStrength(float strength);//导航强停刹强度请求，0.0 释放，1.0 等价旧强停刹
void Brake_NavHardStop_Reset(void);//清空导航强停刹请求，退出雷区刹车准备区或复位时调用
/**
 * @brief 加速前馈更新：在复刻起步和目标速度明显抬升时补额外驱动力
 * @param target_speed 当前目标速度
 * @param actual_speed 当前实际速度
 * @param motor_enable 电机使能，0 时清空加速前馈
 * @param jump_flag 跳跃保护标志，非 0 时清空加速前馈
 * @param replay_running 复刻运行标志，非 0 时才允许加速补偿
 * @param inhibit_accel 外部仲裁屏蔽标志，刹车较强/特殊任务接管时置 1
 * @return 本周期加速前馈 PWM；起步慢优先小步调大 ACCEL_FF_GAIN、ACCEL_FF_RAMP_UP
 */
float Accel_Feedforward_Update(float target_speed, float actual_speed, uint8 motor_enable, uint8 jump_flag, uint8 replay_running, uint8 inhibit_accel);
void Accel_Feedforward_Reset(void);//清空加速前馈和起步/出弯补偿窗口
float Accel_Feedforward_GetPwm(void);//读取当前加速前馈 PWM，供 ISR 与刹车前馈融合
float Accel_Feedforward_GetKpBoost(void);//读取当前加速Kp增强系数，供速度控制环使用

// 辅助宏：取绝对值
#define MY_ABS(x) ((x) > 0 ? (x) : -(x))

#endif
