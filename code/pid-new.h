#ifndef CODE__PID_NEW_H__
#define CODE__PID_NEW_H__
#include "zf_common_headfile.h"
// ============================================================================
//  *** TUNING AREA / 核心调参区 ***
//  这里定义了初始化结构体的具体数值。
//  修改这里的宏定义，即可改变平衡车的特性。
// ============================================================================

// ----------------------------------------------------------------------------
// 4. 舵机速度环参数 (周期20ms)
//    作用：控制舵机的转动速度，使其平滑地达到目标位置，避免突然动作
// ----------------------------------------------------------------------------
#define SERVO_SPEED_KP  0.0f   // [比例控制] 控制舵机速度响应的快慢
#define SERVO_SPEED_KI  0.0f   // [积分控制] 
#define SERVO_SPEED_KD  0.0f   // [微分控制] 
#define SERVO_SPEED_MAX_I  30000.0f  // [积分限幅] 限制积分项的最大值
#define SERVO_SPEED_MAX_O  1000.0f   // [输出限幅] 限制舵机速度的最大值，避免过快
#define SERVO_SPEED_COMP   0.0f   // [关键补偿] 舵机速度环的补偿值


// ----------------------------------------------------------------------------
// 1. 速度环参数 (最外环 - 周期约 20ms~50ms)
//    作用：通过改变车身倾角，让车“跑”起来去追重心，从而保持位置或达到目标速度。
// ----------------------------------------------------------------------------
#define SPD_KP      0.0f   // [响应力度] 值越大，车对速度误差越敏感，加速越猛，但容易超调晃动
#define SPD_KI      0.0f  // [消除静差] 值越大，车越能克服阻力达到目标速度，但回正越慢
#define SPD_KD      0.0f    // [抑制震荡] 速度环一般不加D，因为编码器噪声大，且不需要极快响应

#define SPD_MAX_I   2000.0f // [积分防饱和] 限制积分项的最大贡献
#define SPD_MAX_O   1500.0f    // [安全角度] 速度环输出的是“期望角度”。限制为8度，意味着车最快加速时也不能倾斜超过8度，防止扑街。
#define SPD_COMP    0.0f    // 速度环暂不需要额外补偿

// ----------------------------------------------------------------------------
// 2. 角度环参数 (中间环 - 周期约 5ms)
//    作用：根据期望角度(来自机械零点+速度环)，计算出需要的角速度。
//    这是维持直立最关键的一环。
// ----------------------------------------------------------------------------
#define ANG_KP      0.0f   // [直立刚度] 类似于弹簧的硬度。值太小车软绵绵扶不正；值太大车会剧烈低频抖动。
#define ANG_KI      0.0f    // [一般不用] 平衡车本身是不稳定系统，加积分容易导致无法直立，除非是完全静态的高精度控制。
#define ANG_KD      0.0f    // [直立阻尼] 极重要！类似于减震器。值太小车会有余震；值太大车反应迟钝且有高频噪音。

#define ANG_MAX_I   0.0f    // 积分限幅
#define ANG_MAX_O   8000.0f // [最大角速度] 限制期望的旋转速度，防止电机指令过大。

// [关键补偿] 机械零点 (Mechanical Zero)
// 理想情况下0度是平衡点。但因电池安装、传感器贴歪等原因，实际平衡点可能是 -1.5度。
// 调试方法：如果车总是往“前”跑，说明它觉得自己后仰了，需要减小这个值；反之增大。
#define ANG_MECH_ZERO  4.1f   

// ----------------------------------------------------------------------------
// 3. 角速度环参数 (最内环 - 周期约 1ms)
//    作用：直接控制电机PWM，让车身角速度迅速跟随角度环的指令。
//    这一环必须响应最快。
// ----------------------------------------------------------------------------
#define GYR_KP      0.0f    // [响应速度] 决定了电机对旋转的抵抗力。
#define GYR_KI      0.0f    // [一般不用] 响应太快，积分来不及反应，反而造成滞后。
#define GYR_KD      0.0f    // [消除抖动] 抑制高频噪声和电机抖动。

#define GYR_MAX_I   0.0f    
#define GYR_MAX_O   6000.0f // [PWM满幅] 满是10000，这里留点余量设3000。

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
#define TURN_ANG_KP     0.0f   // [转向刚度] 值越大转向越灵敏，但易振荡
#define TURN_ANG_KI     0.0f   // [一般不用] 无积分项，避免转向累积误差
#define TURN_ANG_KD     0.0f   // [转向阻尼] 抑制转向超调，值过大会导致响应迟钝
#define TURN_ANG_MAX_I  0.0f    // [一般不用] 无积分项，避免转向累积误差
#define TURN_ANG_DEAD_ZONE 0.0f // [死区] 消除低速时的非线性迟滞
#define TURN_ANG_MAX_O  8000.0f  // [角速度限幅] 限制最大期望转向角速度

// ----------------------------------------------------------------------------
// 6. 转向角速度环参数 (内环 - 周期2ms)
//    作用：快速跟踪期望角速度，直接输出转向专用PWM
// ----------------------------------------------------------------------------
#define TURN_GYR_KP     0.0f    // [响应速度] 决定转向电机响应刚度
#define TURN_GYR_KI     0.0f     // [一般不用] 无积分项，避免转向累积误差
#define TURN_GYR_KD     0.0f     // [抖动抑制] 消除高频抖动
#define TURN_GYR_MAX_I  0.0f     // [一般不用] 无积分项，避免转向累积误差
#define TURN_GYR_DEAD_ZONE 0.0f  // [死区] 消除低速时的非线性迟滞
#define TURN_GYR_MAX_O  5000.0f  // [PWM限幅] 普通赛道转向PWM上限
#define TURN_GYR_MAX_O_BRIDGE 7000.0f // [单边桥限幅] 单边桥需更大转向力矩

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

// ============================================================================
// 2. 全局声明
// ============================================================================
extern PID_Param_t pid_servo_speed;//速度环(舵机)pid参数
extern PID_Param_t pid_angle;//角度环(pid参数)
extern PID_Param_t pid_speed;//速度环(外环)pid参数，未调用
extern PID_Param_t pid_gyro;//加速度环pid参数
extern PID_Param_t pid_turn_angle;//转向角度环pid参数
extern PID_Param_t pid_turn_gyro;//转向角速度环pid参数

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

extern float target_speed_set;

void PID_Param_Init(void);//pid参数初始化，同时也可以用于倒地保护
void PID_Data_Reset(void);//pid参数全清空，暂时未使用
float Float_Constrain(float val, float min, float max);//限幅函数

float Turn_Angle_Loop_Control(float angle_error);//转向角度环控制
float Turn_Gyro_Loop_Control(float target_gyro, float actual_gyro);//转向角速度环控制
float Servo_Speed_Control(float target_speed, float actual_speed);//速度环(舵机)
float Speed_Loop_Control(float target_speed, float actual_speed);//速度环(外环)(电机)
float Angle_Loop_Control(float speed_loop_output, float actual_angle);//角度环(中环)
float Gyro_Loop_Control(float angle_loop_output, float actual_gyro);//角速度环(内环)

// 辅助宏：取绝对值
#define MY_ABS(x) ((x) > 0 ? (x) : -(x))

#endif