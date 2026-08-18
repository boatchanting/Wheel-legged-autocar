/*********************************************************************************************************************
* CYT4BB Opensourec Library 即（ CYT4BB 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 CYT4BB 开源库的一部分
*
* CYT4BB 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          cm7_0_isr
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT4BB
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-9      pudding            first version
* 2024-5-14     pudding            新增12个pit周期中断 增加部分注释说明
********************************************************************************************************************/

#include "zf_common_headfile.h"
#include "config/config.h"//【提醒】配置请在这里修改
#include "tools/runtime_profiler.h"
#include "plan/bumpy_road.h"
#include "vision/vision_ipc_core0.h"
#include "vision/vision_pvc_control.h"
#include "vision/vision_bumpy_control.h"
#include "vision/vision_bridge_control.h"
#include "vision/vision_slope_control.h"
#include "vision/vision_three_stage_control.h"
#include "servo/servo_executor.h"
#include "navigation/nav_replay/nav_replay.h"
#include "tools/wifi_protocol.h"

// 声明外部函数

// 加上 volatile，告诉编译器这个变量会在中断中突变
extern volatile uint8 pit_state; 
extern volatile uint8 pit_state1; 
// 声明外部函数，确保编译器能找到 ekf.c 中的函数
extern void EKF_UpData(void);//卡尔曼滤波
extern volatile runtime_profiler_t g_ekf_profiler;

volatile uint32_t control_tick = 0;        // 1ms计数器
extern volatile float pid_out_speed;  // 速度环输出（目标角度）
extern volatile float pid_out_angle;// 角度环输出（目标角速度）
extern volatile float pid_out_pwm;// 角速度环输出（PWM值）      

volatile struct {
    float angle;
    float gyro;
    float speed;
} sensor_data = {0};
# define OUR_PWM_MAX_LIMIT 8000.0f // 最大PWM值（根据实际情况调整）


volatile float err_degree = 0.0f;//  转向控制全局变量（需在视觉/gps/编码器模块中更新）
volatile float roll_degree = 0.0f;//  转向控制全局变量（需在视觉/gps/编码器模块中更新）
volatile float filtered_gyro_z = 0.0f;//陀螺仪数据滤波z轴角速度，用于转向角速度环和示波器观测
uint32_t loop_counter = 0;
static uint16 accel_ff_buzzer_on_ticks = 0U;       // 大幅加速前馈蜂鸣剩余时间，1ms 递减
static uint16 accel_ff_buzzer_cooldown_ticks = 0U; // 蜂鸣冷却时间，避免持续大前馈时重复鸣叫
static uint8 accel_ff_buzzer_was_large = 0U;       // 上一周期是否处于大前馈状态，用于边沿触发
static uint16 profile_switch_beep_ticks = 0U;      // 复刻PID切换蜂鸣剩余时间，1ms 递减
static uint16 minefield_beep_ticks = 0U;           // 自转结束蜂鸣剩余时间，1ms 递减
static bool g_fallen_last = false;
static uint16 g_fallen_standup_grace_ticks = 0U;

#define FALLEN_STANDUP_GRACE_MS (1500U)

#if IMU_REFRESH_TEST_ENABLE
#define IMU_REFRESH_TEST_TIME_MS (10000U)

typedef struct
{
    int16 x;
    int16 y;
    int16 z;
} imu_refresh_axis_raw_t;

typedef struct
{
    imu_refresh_axis_raw_t gyro;
    imu_refresh_axis_raw_t acc;
    imu_refresh_axis_raw_t mag;
} imu_refresh_raw_t;

volatile uint32 g_imu_refresh_test_elapsed_ms = 0;
volatile uint32 g_imu_refresh_test_gyro_change_count = 0;
volatile uint32 g_imu_refresh_test_acc_change_count = 0;
volatile uint32 g_imu_refresh_test_mag_change_count = 0;
volatile uint32 g_imu_refresh_test_gyro_freq_x100 = 0;
volatile uint32 g_imu_refresh_test_acc_freq_x100 = 0;
volatile uint32 g_imu_refresh_test_mag_freq_x100 = 0;
volatile uint8 g_imu_refresh_test_running = 0;
volatile uint8 g_imu_refresh_test_done = 0;
volatile uint8 g_imu_refresh_test_start_beep_request = 0;
volatile uint8 g_imu_refresh_test_done_beep_request = 0;

static imu_refresh_raw_t g_imu_refresh_test_last = {0};
static uint8 g_imu_refresh_test_has_last = 0;

static uint8 imu_refresh_axis_changed(const imu_refresh_axis_raw_t *now, const imu_refresh_axis_raw_t *last)
{
    return (uint8)((now->x != last->x) ||
                  (now->y != last->y) ||
                  (now->z != last->z));
}

static void imu_refresh_read_all_raw(imu_refresh_raw_t *raw)
{
#if IMU_CATEGORY == 1
    imu660ra_get_gyro();
    imu660ra_get_acc();
    raw->gyro.x = imu660ra_gyro_x;
    raw->gyro.y = imu660ra_gyro_y;
    raw->gyro.z = imu660ra_gyro_z;
    raw->acc.x = imu660ra_acc_x;
    raw->acc.y = imu660ra_acc_y;
    raw->acc.z = imu660ra_acc_z;
    raw->mag.x = 0;
    raw->mag.y = 0;
    raw->mag.z = 0;
#elif IMU_CATEGORY == 2
    imu660rb_get_gyro();
    imu660rb_get_acc();
    raw->gyro.x = imu660rb_gyro_x;
    raw->gyro.y = imu660rb_gyro_y;
    raw->gyro.z = imu660rb_gyro_z;
    raw->acc.x = imu660rb_acc_x;
    raw->acc.y = imu660rb_acc_y;
    raw->acc.z = imu660rb_acc_z;
    raw->mag.x = 0;
    raw->mag.y = 0;
    raw->mag.z = 0;
#elif IMU_CATEGORY == 3
    imu963ra_get_gyro();
    imu963ra_get_acc();
    imu963ra_get_mag();
    raw->gyro.x = imu963ra_gyro_x;
    raw->gyro.y = imu963ra_gyro_y;
    raw->gyro.z = imu963ra_gyro_z;
    raw->acc.x = imu963ra_acc_x;
    raw->acc.y = imu963ra_acc_y;
    raw->acc.z = imu963ra_acc_z;
    raw->mag.x = imu963ra_mag_x;
    raw->mag.y = imu963ra_mag_y;
    raw->mag.z = imu963ra_mag_z;
#else
#error "Unsupported IMU_CATEGORY value"
#endif
}
#endif

static void imu_refresh_read_gyro_1ms(void)
{
#if IMU_CATEGORY == 1
    imu660ra_get_gyro();
#elif IMU_CATEGORY == 2
    imu660rb_get_gyro();
#elif IMU_CATEGORY == 3
    imu963ra_get_gyro();
#else
#error "Unsupported IMU_CATEGORY value"
#endif
}

static void ImuRefreshTest_Update1ms(void)
{
#if IMU_REFRESH_TEST_ENABLE
    imu_refresh_raw_t now;

    if (g_imu_refresh_test_done)
    {
        imu_refresh_read_gyro_1ms();
        return;
    }

    if (!g_yaw_initialized)
    {
        imu_refresh_read_gyro_1ms();
        return;
    }

    imu_refresh_read_all_raw(&now);

    if (!g_imu_refresh_test_running)
    {
        g_imu_refresh_test_last = now;
        g_imu_refresh_test_has_last = 1;
        g_imu_refresh_test_elapsed_ms = 0;
        g_imu_refresh_test_gyro_change_count = 0;
        g_imu_refresh_test_acc_change_count = 0;
        g_imu_refresh_test_mag_change_count = 0;
        g_imu_refresh_test_gyro_freq_x100 = 0;
        g_imu_refresh_test_acc_freq_x100 = 0;
        g_imu_refresh_test_mag_freq_x100 = 0;
        g_imu_refresh_test_running = 1;
        g_imu_refresh_test_start_beep_request = 1;
        return;
    }

    g_imu_refresh_test_elapsed_ms++;
    if (g_imu_refresh_test_has_last && imu_refresh_axis_changed(&now.gyro, &g_imu_refresh_test_last.gyro))
    {
        g_imu_refresh_test_gyro_change_count++;
        g_imu_refresh_test_last.gyro = now.gyro;
    }
    if (g_imu_refresh_test_has_last && imu_refresh_axis_changed(&now.acc, &g_imu_refresh_test_last.acc))
    {
        g_imu_refresh_test_acc_change_count++;
        g_imu_refresh_test_last.acc = now.acc;
    }
#if IMU_CATEGORY == 3
    if (g_imu_refresh_test_has_last && imu_refresh_axis_changed(&now.mag, &g_imu_refresh_test_last.mag))
    {
        g_imu_refresh_test_mag_change_count++;
        g_imu_refresh_test_last.mag = now.mag;
    }
#endif

    if (g_imu_refresh_test_elapsed_ms >= IMU_REFRESH_TEST_TIME_MS)
    {
        g_imu_refresh_test_gyro_freq_x100 =
            (g_imu_refresh_test_gyro_change_count * 100000U) / g_imu_refresh_test_elapsed_ms;
        g_imu_refresh_test_acc_freq_x100 =
            (g_imu_refresh_test_acc_change_count * 100000U) / g_imu_refresh_test_elapsed_ms;
#if IMU_CATEGORY == 3
        g_imu_refresh_test_mag_freq_x100 =
            (g_imu_refresh_test_mag_change_count * 100000U) / g_imu_refresh_test_elapsed_ms;
#else
        g_imu_refresh_test_mag_freq_x100 = 0;
#endif
        g_imu_refresh_test_running = 0;
        g_imu_refresh_test_done = 1;
        g_imu_refresh_test_done_beep_request = 1;
    }
#else
    imu_refresh_read_gyro_1ms();
#endif
}

static void Accel_Feedforward_Buzzer_Update(float accel_pwm)
{
#if ACCEL_FF_BUZZER_ENABLE
    uint8 large_active = (uint8)(fabsf(accel_pwm) >= ACCEL_FF_BUZZER_PWM_TH);

    if (accel_ff_buzzer_on_ticks > 0U)
    {
        gpio_set_level(BUZZER_PIN, GPIO_HIGH);
        accel_ff_buzzer_on_ticks--;
        if (accel_ff_buzzer_on_ticks == 0U)
        {
            gpio_set_level(BUZZER_PIN, GPIO_LOW);
        }
    }
    else if (accel_ff_buzzer_cooldown_ticks > 0U)
    {
        accel_ff_buzzer_cooldown_ticks--;
    }

    if ((large_active != 0U) &&
        (accel_ff_buzzer_was_large == 0U) &&
        (accel_ff_buzzer_on_ticks == 0U) &&
        (accel_ff_buzzer_cooldown_ticks == 0U))
    {
        accel_ff_buzzer_on_ticks = ACCEL_FF_BUZZER_ON_MS;
        accel_ff_buzzer_cooldown_ticks = ACCEL_FF_BUZZER_COOLDOWN_MS;
        gpio_set_level(BUZZER_PIN, GPIO_HIGH);
    }

    accel_ff_buzzer_was_large = large_active;
#else
    (void)accel_pwm;
#endif
}
// **************************** PIT中断函数 ****************************

void pit0_ch0_isr()                     // 定时器通道 0 周期中断服务函数      
{
    // 1. 清除中断标志位 (必须第一步做)
    pit_isr_flag_clear(PIT_CH0);
    loop_counter++;

    if ((loop_counter % 2U) == 0U)
    {
        VisionIpc_Core0_Update_2ms();
        VisionPvcControl_Update_2ms();
        VisionBumpyControl_Update_2ms();
        VisionBridgeTask_Update_2ms();
        VisionSlopeTask_Update_2ms();
        VisionThreeStageControl_Update_2ms();
    }

    ImuRefreshTest_Update1ms(); // 每1ms读取当前IMU，测试开启时分别统计陀螺仪/加速度计/磁力计跳变次数

    if (g_fallen != g_fallen_last)
    {
        PID_Param_Init();
        Brake_Feedforward_Reset();
        Accel_Feedforward_Reset();
        if (!g_fallen)
        {
            g_fallen_standup_grace_ticks = FALLEN_STANDUP_GRACE_MS;
        }
        else
        {
            g_fallen_standup_grace_ticks = 0U;
        }
        g_fallen_last = g_fallen;
    }
    else if (g_fallen_standup_grace_ticks > 0U)
    {
        g_fallen_standup_grace_ticks--;
        
        // 【新增安全优化】提前结束起立免死金牌期
        // 如果赛车已经成功起立（倾角误差小于 30 度），则取消免死金牌
        // 这样可以防止起立后马上又摔倒，但因为还在免死金牌期内导致轮胎狂转
        if (fabsf(euler_angle.pitch - ANG_MECH_ZERO) < 30.0f)
        {
            g_fallen_standup_grace_ticks = 0U;
        }
    }

    // 颠簸路段状态机（1ms调度）：
    // 1) 锁定 target_speed_set = -150.0f
    // 2) 里程累计到 1000mm 后停车并退出状态机
    if((g_motor_enable == 1) && (!g_fallen)){
        BumpyRoad_Update_1ms(); // 颠簸路段状态机
    }

    
    // ==========================================================
    // 【nav.1】惯性导航解算 (10ms 跑一次)
    // 按下记录按钮之前的惯性导航不可信==========================================================
    if(loop_counter % 10 == 1 && g_yaw_initialized)
    {
        if (jump_flag == 0)
        {
            // 调用导航更新函数
            #if IMU_CATEGORY == 1&&CAR_SELECT == 3 //如果小车不同再对小车加&&加以区分
            InertialNav_Update(
                euler_angle.yaw,                                 // 当前偏航角
                -9806.65*((float)imu_data.acc_y/4096-(float)imu_data.grav_y), // 横向加速度 (左+) 9.80665是重力加速度，这里乘了1000倍是因为转换为mm/s^2，imu数据是4096位的，所以需要除4096
                9806.65*((float)imu_data.acc_x/4096-(float)imu_data.grav_x),              // 纵向加速度 (前+)
                (float)motor_value.receive_left_speed_data,      // 左轮速
                (float)motor_value.receive_right_speed_data,     // 右轮速
                filtered_gyro_z * 0.0174532925f                  // gyro_z_rad_s
            );
            #endif
            #if IMU_CATEGORY == 1&&CAR_SELECT == 4 //小车4初版惯导输入与小车3相同
            InertialNav_Update(
                euler_angle.yaw,
                -9806.65*((float)imu_data.acc_y/4096-(float)imu_data.grav_y),
                9806.65*((float)imu_data.acc_x/4096-(float)imu_data.grav_x),
                (float)motor_value.receive_left_speed_data,
                (float)motor_value.receive_right_speed_data,
                filtered_gyro_z * 0.0174532925f
            );
            #endif
            #if IMU_CATEGORY == 3 &&CAR_SELECT == 3//imu963ra 如果小车不同再对小车加&&加以区分
            
            InertialNav_Update(
            euler_angle.yaw,                                 // 当前偏航角
            9806.65*((float)imu_data.acc_x/4098 - (float)imu_data.grav_x), // 横向加速度 (左+)
            9806.65*((float)imu_data.acc_y/4098 - (float)imu_data.grav_y), // 纵向加速度 (前+)
            (float)motor_value.receive_left_speed_data,
            (float)motor_value.receive_right_speed_data,
            filtered_gyro_z * 0.0174532925f
            );
            #endif
            #if IMU_CATEGORY == 3 &&CAR_SELECT == 4//imu963ra，小车4初版惯导输入与小车3相同
            InertialNav_Update(
            euler_angle.yaw,
            9806.65*((float)imu_data.acc_x/4098 - (float)imu_data.grav_x),
            9806.65*((float)imu_data.acc_y/4098 - (float)imu_data.grav_y),
            (float)motor_value.receive_left_speed_data,
            (float)motor_value.receive_right_speed_data,
            filtered_gyro_z * 0.0174532925f
            );
            #endif
        }
        else
        {
            // 跳跃期间轮胎可能空转，冻结惯导速度状态，避免里程突增。
            inertial_nav.vx_body = 0.0f;
            inertial_nav.vy_body = 0.0f;
        }

        // 此后, 可以直接使用 inertial_nav.x 和 inertial_nav.y
        // 例如, 用于路径规划、位置闭环等
        // float current_pos_x = inertial_nav.x;
        // float current_pos_y = inertial_nav.y;
        // float current_heading = inertial_nav.relative_yaw; // 获取相对航向角 (度)
    }

    #if GNSS_NAV == 1
    //【gnss.1】GNSS定位更新
    if (loop_counter % 100 == 2) {  // 100ms 一次
        if (gnss_flag) {
            gnss_flag = 0;//将标志位清零
            gnss_data_parse();           //开始解析数据
            Gnss_Transform_Update();//gnss转换为高斯克吕格投影
        } // GNSS更新
    }
    #endif


   // ------------------------------------------------------
    // 【nav.4】复现控制任务 (10ms运行一次)
    // ------------------------------------------------------
    if (loop_counter % 10 == 3) {  // 10ms 一次
        // 颠簸状态机激活时，暂停导航复现，避免覆盖锁速目标
        if (g_motor_enable && (!g_fallen) && (BumpyRoad_Is_Active() == 0U) && (VisionBridgeTask_IsActive() == 0U) && (VisionSlopeTask_IsActive() == 0U))
        { 
            NavReplay_Process();
            #if GNSS_NAV == 1
            GpsNavReplay_Process(); 
            #endif
        } //复现控制
    }

    if (loop_counter % 20 == 4) {  // 20ms 一次
        if(g_motor_enable && (!g_fallen) && (VisionBridgeTask_IsActive() == 0U) && (VisionSlopeTask_IsActive() == 0U)){Bridge_Test_Triple_SingleSide_Inertial();
        } //复现控制
    };//【测试】抬高双腿

    Control_Profile_Update1ms();//pid参数调度更新

    // 自转结束蜂鸣器提示【优化点】不要放中断
    if (g_minefield_beep_request != 0U)
    {
        g_minefield_beep_request = 0U;
        minefield_beep_ticks = 50U; // 蜂鸣50ms
        gpio_set_level(BUZZER_PIN, GPIO_HIGH);
    }
    if (minefield_beep_ticks > 0U)
    {
        minefield_beep_ticks--;
        if (minefield_beep_ticks == 0U)
        {
            gpio_set_level(BUZZER_PIN, GPIO_LOW);
        }
    }

    // 复刻模式下PID切换蜂鸣提示
    if (profile_switch_beep_request != 0U)
    {
        profile_switch_beep_request = 0U;
        if (g_replay_state == REPLAY_RUNNING)
        {
            profile_switch_beep_ticks = 50U; // 蜂鸣50ms
            gpio_set_level(BUZZER_PIN, GPIO_HIGH);
        }
    }
    if (profile_switch_beep_ticks > 0U)
    {
        profile_switch_beep_ticks--;
        if (profile_switch_beep_ticks == 0U)
        {
            gpio_set_level(BUZZER_PIN, GPIO_LOW);
        }
    }

    // ==========================================================
    // 步骤 1: 速度环(舵机控制) (20ms 跑一次，现改为9ms)
    // ==========================================================
    if(g_is_push_mode==0)
    {  


        if (loop_counter % 9 == 5 && g_yaw_initialized)  // 20ms周期，现改为9ms周期，且偏航角已初始化，且不在推车模式
        {
            // 2.1 获取编码器速度
            //small_driver_get_speed();//这句话应该不用，它只要调用一次，逐飞的库里写了
            float left_speed = (float)motor_value.receive_left_speed_data;
            float right_speed = (float)motor_value.receive_right_speed_data;
            current_actual_speed = 0.5f * (right_speed - left_speed);

            if (jump_flag != 0U)
            {
                /* 【跳跃冻结速度环 2026-08-12】
                 * 跳跃期间速度环不参与控制：
                 * 1) 跳过 Servo_Speed_Control 计算——跳跃中轮子空转，速度误差巨大，
                 *    若继续计算会把 g_target_pwm_speed_adj 顶到限幅并污染 PID 状态；
                 * 2) 清零速度环运行态（含输入滤波），跳跃结束后从干净状态起步，D 项无突跳；
                 * 3) g_target_pwm_speed_adj 归零——跳跃结束后 servo_executor_update 的
                 *    目标=基础身高(平衡态)，与跳跃恢复位置一致，关节舵机不乱动。 */
                Servo_Speed_Control_Reset();
                g_target_pwm_speed_adj = 0;
            }
            else
            {
            // 2.2 全局刹车前馈
            uint8 brake_ff_enable = (uint8)((g_motor_enable != 0) && (!g_fallen));
            if ((Minefield_Is_Active() != 0U) || (g_special_action_trigger != 0U))
            {
                brake_ff_enable = 0U;
                Brake_Feedforward_Reset();
            }
            Brake_Feedforward_Update(target_speed_set, current_actual_speed, brake_ff_enable, jump_flag);
            {
                uint8 accel_ff_replay_running = 0U;
                uint8 accel_ff_inhibit = 0U;
                float brake_pwm_now = Brake_Feedforward_GetPwm();

                if (g_replay_state == REPLAY_RUNNING)
                {
                    accel_ff_replay_running = 1U;
                }
                #if GNSS_NAV == 1
                if (g_gps_replay_state == REPLAY_RUNNING)
                {
                    accel_ff_replay_running = 1U;
                }
                #endif

                // 加速前馈计算阶段的屏蔽条件：
                // 刹车前馈达到 BRAKE_ACCEL_INHIBIT_PWM 或特殊/视觉任务接管时，不再生成新的加速补偿；
                // 小于该阈值的轻刹交给后面的最终仲裁处理，避免出弯瞬间加速前馈被完全清掉。
                uint8 nav_special_zero_brake_active = 0U;

#if (CURRENT_NAV_PLAN == 2) && (NAV_PLAN2_METHOD == PLAN2_POINT_SPEED_PLANNING)
                nav_special_zero_brake_active = NavReplay_SpecialPointZeroBrakeActive();
#endif

                if ((g_is_push_mode == 1U) ||
                    (nav_special_zero_brake_active != 0U) ||
                    (g_brake_active != 0U) ||
                    (g_reverse_brake_active != 0U) ||
                    (fabsf(brake_pwm_now) >= BRAKE_ACCEL_INHIBIT_PWM) ||
                    (g_special_action_trigger != 0U) ||
                    (BumpyRoad_Is_Active() != 0U) ||
                    (VisionThreeStageControl_IsActive() != 0U) ||
                    (VisionBridgeTask_IsActive() != 0U) ||
                    (VisionSlopeTask_IsActive() != 0U) ||
                    (Bridge_Test_Triple_SingleSide_Is_Active()) ||
                    (g_pvc_control_enable != 0U)
                    #if GNSS_NAV == 1
                    || (g_gps_special_action_trigger != 0U)
                    #endif
                   )
                {
                    accel_ff_inhibit = 1U;
                }

                Accel_Feedforward_Update(target_speed_set,
                                         current_actual_speed,
                                         (uint8)((g_motor_enable != 0) && (!g_fallen)),
                                         jump_flag,
                                         accel_ff_replay_running,
                                         accel_ff_inhibit);
            }


            // 2.3 计算目标速度调整分量
            float duty_adjustment = Servo_Speed_Control(target_speed_set, current_actual_speed,euler_angle.pitch);
            {
                float brake_pwm_now = Brake_Feedforward_GetPwm();
                
                // 修复：强刹时刹车前馈产生的反作用力矩方向与当前运动方向有关。
                // 前进刹车时(brake_pwm_now > 0)，车身有前栽趋势，需要给后坐方向的舵机支撑。
                // 倒车刹车时(brake_pwm_now < 0)，车身有后坐趋势，需要给前倾方向(反后坐)的舵机支撑。
                float required_duty_dir = (brake_pwm_now > 0.0f) ? BRAKE_SERVO_BACK_SIT_SIGN : -BRAKE_SERVO_BACK_SIT_SIGN;

                float brake_reaction_component = duty_adjustment * (-required_duty_dir);
                float brake_anti_reaction_component = duty_adjustment * required_duty_dir;

                // 强刹时优先保护车身姿态：提供对抗制动力矩的舵机支撑量，防止前栽或后坐蹭地。
                if (fabsf(brake_pwm_now) >= BRAKE_SERVO_PROTECT_PWM_TH)
                {
                    if ((brake_reaction_component > BRAKE_SERVO_BACK_SIT_LIMIT) ||
                        (brake_anti_reaction_component < BRAKE_SERVO_ANTI_BACK_SIT_DUTY))
                    {
                        duty_adjustment = required_duty_dir * BRAKE_SERVO_ANTI_BACK_SIT_DUTY;
                        pid_servo_speed.error_integral = 0.0f;
                    }
                }
            }
            g_target_pwm_speed_adj = (int16)duty_adjustment;
            } /* end if (jump_flag == 0) */
        }
    }
    else if(g_is_push_mode==1)
    {    
        // 重置舵机速度环状态变量
        pid_servo_speed.error = 0;
        pid_servo_speed.last_error = 0;
        pid_servo_speed.prev_error = 0;
        pid_servo_speed.error_integral = 0;
        pid_servo_speed.output = 0;

        Brake_Feedforward_Reset();
        Accel_Feedforward_Reset();
    }
    // ==========================================================
    // 步骤 2: 转向角度环 (6ms，现改为3ms) - 外环
    // ==========================================================
        if (loop_counter % 3 == 1)  // 6ms周期，现改为3ms周期
        {
            // err_degree: 由视觉/gps/编码器提供的转向角度误差（期望-实际，单位：度），预留的调用位置，调用要写到if之后【优化点】需要知道向哪个方向为正值
            // 示例：视觉识别到赛道偏左5° → err_degree = +5.0f
            // turn_angle_loop_out = Turn_Angle_Loop_Control(err_degree);
             // 只有在偏航角成功初始化后，才执行航向保持控制
            // 如果正在雷区(Minefield)中旋转，屏蔽正常的PID转向角度环(外环)
            if ((g_yaw_initialized != 0U) &&
                (Minefield_Is_Active() == 0U))
            {
                // 1. 计算航向误差，err_degree是视觉/gps/编码器/遥控器提供的期望转向角度误差（期望-实际，单位：度）
                
                
                if(g_is_push_mode==1)
                {
                    g_initial_yaw = euler_angle.yaw; // 改变目标航向角，单位：度
                }
                float yaw_error = err_degree;
                // yaw_error =  g_initial_yaw-euler_angle.yaw ; // 调节pid转向角度环时使用【调试pid打开】
                // 2. [关键] 处理角度“卷绕”问题 (Wraparound)
                //    例如：目标是-179度，当前是179度，实际误差是向右偏2度(-2)，
                //    但直接相减得到 -358度，这会导致PID控制器输出巨大的错误值。
                //    我们需要将误差归一化到 -180 ~ +180 度之间。
                yaw_error = fmod(yaw_error, 360.0f);//先对yaw_error取模，确保在-360到360之间
                if (yaw_error > 180.0f)
                {
                    yaw_error -= 360.0f; // 例如: 358 -> -2
                }
                else if (yaw_error < -180.0f)
                {
                    yaw_error += 360.0f; // 例如: -358 -> 2
                }
                
                // ============= 输入误差限幅，这个防止视觉或者gps给的参数一下过大导致小车疯狂旋转，优化方案是如果大角度可以关角度环转一下，但是先这么用，后续可以优化【优化点】 ==================
                // 设定一个最大误差阈值，例如 30度 或 45度
                // 如果误差太大，就骗PID说误差只有这么大，防止输出饱和
                float max_error_limit = 45.0f; 

                if (yaw_error > max_error_limit)
                {
                    yaw_error = max_error_limit;
                }
                else if (yaw_error < -max_error_limit)
                {
                    yaw_error = -max_error_limit;
                }

                // 4. 将计算出的精确航向误差送入PID控制器
                //    控制器的目标就是将这个 yaw_error 减小到0
                turn_angle_loop_out = Turn_Angle_Loop_Control(yaw_error);
            }
            else
            {
                //1.角度未初始化状态下，外环不输出
                //2.在雷区旋转模式下，切断外环对内环的控制
                turn_angle_loop_out = 0.0f; 
            }
        }
    // ==========================================================
    // 步骤 3: 平衡角度环 (5ms 跑一次，现EKF为1ms、角度环为3ms)
    // ==========================================================
    // 运行姿态解算 (EKF / 互补滤波)
    EKF_UpData();
    record_initial_yaw_task(loop_counter);//初始化偏航角，里面的代码只会在初始化的时候被调用一次，记录初始的偏航角
    #if IMU_CATEGORY == 3 // IMU963RA的磁力计模块
    if (loop_counter % 5 == 2)
    {
        EKF_Update_Heading();//磁力计北更新
    }
    #endif
    if (loop_counter % 3 == 2)
    {
        now_angle = euler_angle.pitch; // 获取解算后的角度 (单位：度)

        // --- [调用优化] ---
        // 变化点：删除了第3个参数 "mechanical_zero"，因为它已经包含在 pid_angle.compensation 中了
        // 参数1: speed_loop_out (速度环算出来的目标角度)。这个就给0即可，舵机速度环的输出不要给到这里
        // 参数2: now_angle (当前角度)
        // 返回: angle_loop_out (期望的角速度)
        angle_loop_out = Angle_Loop_Control(speed_loop_out, now_angle);
    }

    
    // ==========================================================
    // 步骤 4: 转向角速度环 (2ms，现改为1ms) - 内环
    // ==========================================================
    if (g_yaw_initialized)  // 2ms周期，现改为1ms周期
    {
        // Z轴(yaw)处理：用于转向角速度环
        float gyro_z_deg = imu_data.gyro_z * 57.2957795f;  // 使用EKF减零偏后的rad/s，转换为°/s
        filtered_gyro_z = 0.8f * filtered_gyro_z + 0.2f * gyro_z_deg;//低通滤波
        // 输入：转向角度环输出(期望角速度) + 实际角速度(filtered_gyro_z)

        //==================== [雷区旋转调用开始] =================
        // lq.1. 获取旋转控制器的输出
        //    参数：当前滤波后的Z轴角速度, 时间间隔(0.002s，现为0.001s), 当前Yaw角, 全局Yaw目标指针
        // 旋转规划与执行统一使用 inertial_nav.relative_yaw 这一套惯导相对航向，
        // 避免规划阶段和执行阶段混用不同角度基准，导致“转满后还要慢慢补角”。
        float spin_cmd = Minefield_Spin_Controller(filtered_gyro_z, 0.001f, inertial_nav.relative_yaw, &g_initial_yaw);

        // lq.2. 决策：如果旋转模块激活，则覆盖外环输出
        float final_turn_cmd;
        
        if (Minefield_Is_Active()) 
        {
            final_turn_cmd = spin_cmd; // 使用平滑的旋转指令
        }
        else
        {
            final_turn_cmd = turn_angle_loop_out; // 使用正常的PID外环指令
        }
        //==================== [雷区旋转调用结束] =================
        // 将雷区旋转指令或者正常转向角速度指令送入内环PID
        turn_gyro_loop_out = Turn_Gyro_Loop_Control(final_turn_cmd, filtered_gyro_z);
    }

    // ==========================================================
    // 步骤 5: 平衡角速度环 (1ms 跑一次，最内环)
    // ==========================================================
    
    // 5.1 使用 EKF 预处理后的俯仰角速度，保留原来的安装轴向和符号。
    #if IMU_CATEGORY == 1&&CAR_SELECT == 0 //如果小车不同再对小车加&&加以区分
    float now_gyro_deg = -imu_data.gyro_x * 57.2957795f; // 根据实际安装方向调整符号[学习板小车1][学习板小车2使用]
    #endif
    #if IMU_CATEGORY == 1&&CAR_SELECT == 3 //如果小车不同再对小车加&&加以区分
    float now_gyro_deg = -imu_data.gyro_y * 57.2957795f; // 根据实际安装方向调整符号[学习板小车3使用]
    #endif
    #if IMU_CATEGORY == 1&&CAR_SELECT == 4 //小车4初版与小车3使用相同陀螺仪轴向
    float now_gyro_deg = -imu_data.gyro_y * 57.2957795f;
    #endif
    #if IMU_CATEGORY == 3 && CAR_SELECT == 0 //如果小车不同再对小车加&&加以区分
    float now_gyro_deg = -imu_data.gyro_y * 57.2957795f; // 根据实际安装方向调整符号
    #endif
    #if IMU_CATEGORY == 3&&CAR_SELECT == 3 //如果小车不同再对小车加&&加以区分
    float now_gyro_deg = imu_data.gyro_x * 57.2957795f; // 根据实际安装方向调整符号[学习板小车3使用]
    #endif
    #if IMU_CATEGORY == 3&&CAR_SELECT == 4 //小车4初版与小车3使用相同陀螺仪轴向
    float now_gyro_deg = imu_data.gyro_x * 57.2957795f;
    #endif

    // 5.2 简单的低通滤波 (平滑噪声)
    now_gyro = 0.8f * now_gyro + 0.2f * now_gyro_deg;

    // --- [调用优化] ---
    // 变化点：移除了手动减零偏逻辑 (GYRO_SENSOR_OFFSET 宏在函数内处理)
    // 变化点：移除了手动死区补偿 (GYR_DEAD_ZONE 宏在函数内处理)
    // 参数1: angle_loop_out (角度环算出来的期望角速度)
    // 参数2: now_gyro (当前实际角速度)
    // 返回: gyro_loop_out (最终PWM)
    gyro_loop_out = Gyro_Loop_Control(angle_loop_out, now_gyro);

    // 6.rolling平衡环(5ms一次)
    if (loop_counter % 5 == 3){
        // 彻底清空并屏蔽历史转向主动侧倾差动，防止干扰纯净的单边桥横滚平衡
        Turn_Active_Roll_Duty_Clear();

        if (roll_balance_enable != 0U)
        {
            // 开启了 Rolling 环使能时，执行纯净的横滚平衡闭环 (以 roll_degree 为目标，默认 0.0f)
            Roll_Balance_Control(euler_angle.roll, roll_degree);
        }
        else
        {
            // 普通未开启 Rolling 时，关闭被动 Rolling
            roll_degree = 0.0f;
            Roll_Balance_Control(euler_angle.roll, euler_angle.roll);
            g_target_pwm_roll_adj = 0;
        }
    }


    // ==========================================================
    // 步骤 6: 安全保护 (倒地停止)(9ms)
    // ==========================================================
    if (loop_counter % 9 == 6){
        // 【倒地保护条件】：必须同时满足
        //   (1) 偏航角已初始化
        //   (2) 非跳跃状态（jump_flag == 0）
        //   (3) 倾角超过安全阈值（±30°）
        //   (4) 第一次站起来之后，loop_counter > 2000(中断开启两秒后)
        if (g_yaw_initialized && (jump_flag == 0) && (loop_counter > 2000))
        {
             // 如果角度过大（例如超过 70 度），判定为倒地
            if (((now_angle-ANG_MECH_ZERO > 70.0f) || (now_angle-ANG_MECH_ZERO < -70.0f)) &&
                (g_fallen_standup_grace_ticks == 0U))
            {
                gyro_loop_out = 0;          // 清零平衡PWM
                turn_gyro_loop_out = 0.0f;  // 清零转向PWM  
                //PID_Data_Reset();// 清除 PID 的除了限幅之外所有参数，否则扶起来的瞬间电机还是全速旋转
                PID_Param_Init();
                Accel_Feedforward_Reset();//【accel_ff】倒地保护时清空加速前馈
                // 倒地保护只进入倒下状态，不再拉低 g_motor_enable，便于后续主动起立。
                g_fallen = true;
                g_fallen_last = true;
                // 彻底关闭电机使能
                //g_motor_enable = 0; 
                NavReplay_Stop();//【nav】复现停止
                #if GNSS_NAV == 1
                GpsNavReplay_Stop();//【gnss】复现停止
                #endif
            }
        }
            
        if((g_motor_enable==0) || (g_fallen))
        {
            gyro_loop_out = 0.0f;      // 清零平衡PWM
            turn_gyro_loop_out = 0.0f; // 清零转向PWM
            PID_Data_Clean_All();// 清除 PID 的除了限幅之外所有参数，否则扶起来的瞬间电机还是全速旋转
            // 【惯性导航】检查当前是否正在运行复现
            // 检查回放是否正在运行，若是则停止
            // if (replay_status == REPLAY_STATUS_RUNNING) {
            //     NAV_REPLAY_Stop();  // 新函数名
            //     target_speed_set = 0.0f;  // 确保速度归零
            //     printf("ISR: Motor disabled, replay stopped.\r\n");
            // }
            NavReplay_Stop();//【nav】复现停止
            Brake_Feedforward_Reset();//【brake】锁定刹车关闭
            Accel_Feedforward_Reset();//【accel_ff】锁定加速前馈关闭
            #if GNSS_NAV == 1
                GpsNavReplay_Stop();//【gnss】复现停止
            #endif
        }
    }
    
    // ==========================================================
    // 步骤 7: PWM矢量融合与电机输出
    // ==========================================================
    if (g_yaw_initialized)
    {
        // - 平衡控制：差动输出 (±gyro_loop_out) → 维持直立
        // - 转向控制：同向输出 (+turn_gyro_loop_out) → 实现旋转
        float brake_pwm = Brake_Feedforward_GetPwm();
        float accel_ff_pwm = Accel_Feedforward_GetPwm();
        uint8 accel_ff_replay_running_fast = 0U;
        if (g_replay_state == REPLAY_RUNNING)
        {
            accel_ff_replay_running_fast = 1U;
        }
        #if GNSS_NAV == 1
        if (g_gps_replay_state == REPLAY_RUNNING)
        {
            accel_ff_replay_running_fast = 1U;
        }
        #endif
        if ((accel_ff_replay_running_fast == 0U) ||
            (g_motor_enable == 0) ||
            (g_fallen) ||
            (jump_flag != 0) ||
            (g_is_push_mode == 1U) ||
            (g_brake_active != 0U) ||
            (g_reverse_brake_active != 0U) ||
            (fabsf(brake_pwm) >= BRAKE_BLEND_CUT_PWM) ||
            (g_special_action_trigger != 0U) ||
            (BumpyRoad_Is_Active() != 0U) ||
            (VisionThreeStageControl_IsActive() != 0U) ||
            (VisionBridgeTask_IsActive() != 0U) ||
            (VisionSlopeTask_IsActive() != 0U) ||
            (Bridge_Test_Triple_SingleSide_Is_Active()) ||
            (g_pvc_control_enable != 0U)
            #if GNSS_NAV == 1
            || (g_gps_special_action_trigger != 0U)
            #endif
           )
        {
            Accel_Feedforward_Reset();
            accel_ff_pwm = 0.0f;
        }
        // 前馈仲裁：
        // 1) brake_pwm >= BRAKE_BLEND_CUT_PWM：刹车主导，加速前馈不参与最终输出；
        // 2) brake_pwm 很小但非零：小窗口共存，仅保留 ACCEL_BLEND_KEEP_RATIO 比例的加速前馈；
        // 3) brake_pwm 为 0：加速前馈完整输出。
        float accel_ff_pwm_effective = accel_ff_pwm;
        if ((fabsf(brake_pwm) >= BRAKE_BLEND_CUT_PWM) && (fabsf(accel_ff_pwm_effective) > 0.0f))
        {
            accel_ff_pwm_effective = 0.0f;
        }
        else if ((fabsf(brake_pwm) > 0.0f) && (fabsf(accel_ff_pwm_effective) > 0.0f))
        {
            accel_ff_pwm_effective *= ACCEL_BLEND_KEEP_RATIO;
        }
        Accel_Feedforward_Buzzer_Update(accel_ff_pwm_effective);
        float drive_ff_pwm = brake_pwm + accel_ff_pwm_effective;
        int16_t pwm_left  = (int16_t)( gyro_loop_out + drive_ff_pwm + turn_gyro_loop_out);
        int16_t pwm_right = (int16_t)(-gyro_loop_out - drive_ff_pwm + turn_gyro_loop_out);

        // 统一限幅（防止叠加后超限）
        pwm_left  = (int16_t)Float_Constrain(pwm_left,  -OUR_PWM_MAX_LIMIT, OUR_PWM_MAX_LIMIT);
        pwm_right = (int16_t)Float_Constrain(pwm_right, -OUR_PWM_MAX_LIMIT, OUR_PWM_MAX_LIMIT);
        if ((g_motor_enable == 0) || (g_fallen))
        {
            pwm_left = 0;
            pwm_right = 0;
        }

        #if WIFI_CORE0_CUSTOM_PROTOCOL 
        // 把数据传给wifi上位机
        g_wifi_target_speed_set = target_speed_set;
        g_wifi_speed_l = (float)motor_value.receive_left_speed_data;
        g_wifi_speed_r = (float)motor_value.receive_right_speed_data;
        g_wifi_pwm_left = (float)pwm_left;
        g_wifi_pwm_right = (float)pwm_right;
        #endif

        // 直接输出即可
        
         // --- 【科目三：跳跃时的电机保护逻辑开始】 ---
        if (jump_flag != 0) 
        {
            // 在跳跃过程中（特别是空中），轮子失去摩擦力。
            // 如果此时平衡环继续工作，轮子会疯狂加速。
            // 建议：直接给0，或者给一个极小的保持速度。
            // 进阶玩法：利用动量轮效应调整空中姿态，可以在这里写逻辑
            // 但为了安全，先置0。
            // 进入跳跃模式，根据精确的阶段执行不同电机策略
        switch(g_current_jump_phase)
        {
            // 阶段A: 起跳瞬间，车轮在地面，关闭电机防止干扰
            case JUMP_PHASE_LAUNCH:
                small_driver_set_duty(pwm_left, pwm_right);
                break;

            // 阶段B: 空中飞行，启用动量轮控制
            case JUMP_PHASE_FLIGHT:
            { // 使用花括号创建一个局部作用域
                //int16_t air_pwm = Momentum_Wheel_Control_Run(now_angle, now_gyro);
                
                // [关键] 根据你的电机极性，使用异号PWM使两轮同向转动
                // 假设 air_pwm > 0 意图让车头抬起 (轮子前转)
                small_driver_set_duty(pwm_left, pwm_right);
                break;
            }

            // 阶段C/D: 准备落地和缓冲，关闭电机，防止轮速过快触地导致弹射
            case JUMP_PHASE_LANDING:
            case JUMP_PHASE_RECOVERY:
                small_driver_set_duty(pwm_left, pwm_right);
                break;

            // 默认或未知状态，安全起见关闭电机
            default:
                small_driver_set_duty(pwm_left, pwm_right);
                break;
        }
        }
        else
        {
            // 正常平衡模式
            small_driver_set_duty(pwm_left, pwm_right);
        }
        // --- 【科目三：跳跃时的电机保护逻辑结束】 --- 
    }

    // ==========================================================
    // 步骤 8: 舵机执行器更新
    // ==========================================================
    if(g_yaw_initialized){//当姿态角可信时舵机执行器才工作
        // --- 【科目三：跳跃时舵机控制权切换开始】 ---
        if (jump_flag != 0)
        {
            // 如果处于跳跃状态，调用跳跃专用执行器
            // 它会根据 loop_counter - start_time 精确控制动作
            servo_jump_executor();
        }
        else
        {
            // 正常行驶状态，调用常规平滑执行器
            servo_executor_update();
        }
        // --- 【科目三：跳跃时舵机控制权切换结束】 ---
    }

    // ==========================================================
    // 步骤 9: 系统心跳
    // ==========================================================
    #if WIFI_CORE0_CUSTOM_PROTOCOL==0
    if(loop_counter % 50 == 11) 
    {
        pit_state = 1; 
    }
    #endif
    #if WIFI_CORE0_CUSTOM_PROTOCOL==1 //wifi传日志时候100帧
    if(loop_counter % 10 == 2) 
    {
        pit_state = 1; 
    }
    #endif

}

void pit0_ch1_isr()                     // 定时器通道 1 周期中断服务函数      
{
    // 1. 清除中断标志位
    pit_isr_flag_clear(PIT_CH1);

#if REMOTE_CONTROL
      // 2. 执行遥控器积分计算
    Remote_Control_Process();

    // ---------------------------------------------------------------
    // 3. 变量映射 (将遥控器结构体映射到主控全局变量)
    // ---------------------------------------------------------------
    //暂时在遥控器内部做了角速度和速度的内部逻辑，没有在这里再做保护，输出那里会有一个保护，这里先这样

    // [映射 1: 安全开关]
    // robot_ctrl.motor_enable: 1=使能, 0=急停
    // g_motor_enable:          1=使能, 0=关机
    // g_fallen:                true=主动倒下, false=主动起立
    if (robot_ctrl.motor_enable == 0) {
        g_motor_enable = 0; // 关机/急停
    } else {
        g_motor_enable = 1; // 正常工作
    }

    if ((robot_ctrl.brake_active == 1U) && (g_replay_state != REPLAY_IDLE))
    {
        NavReplay_Stop();//【nav】复现停止
        Accel_Feedforward_Reset();//【accel_ff】遥控刹车停止复刻时清空加速前馈
    }
    #if GNSS_NAV == 1
    if ((robot_ctrl.brake_active != 0U) && (g_gps_replay_state == REPLAY_RUNNING))
    {
        GpsNavReplay_Stop();//【gnss】复现停止
        Accel_Feedforward_Reset();//【accel_ff】遥控刹车停止 GNSS 复刻时清空加速前馈
    }
    #endif

    /* REPLAY_FINISHED keeps navigation ownership through terminal deceleration. */
    #if GNSS_NAV == 1
    if (g_replay_state == REPLAY_IDLE &&
        (g_gps_replay_state != REPLAY_RUNNING) &&
        (!VisionThreeStageControl_IsActive()) &&
        (BumpyRoad_Is_Active() == 0U) && !Bridge_Test_Triple_SingleSide_Is_Active() 
        && (VisionBridgeTask_IsActive() == 0U)
        && (VisionSlopeTask_IsActive() == 0U)
        && g_pvc_control_enable ==0
    )//【nav】不在复现/颠簸状态机时才允许遥控器写目标速度，不在单边桥时，pvc进入控制关闭
    #endif
    #if GNSS_NAV == 0
        if (g_replay_state == REPLAY_IDLE &&
        (!VisionThreeStageControl_IsActive()) &&
        (BumpyRoad_Is_Active() == 0U) && !Bridge_Test_Triple_SingleSide_Is_Active() 
        && (VisionBridgeTask_IsActive() == 0U)
        && (VisionSlopeTask_IsActive() == 0U)
        && g_pvc_control_enable ==0
    )//【nav】不在复现/颠簸状态机时才允许遥控器写目标速度，不在单边桥时，pvc进入控制关闭
    #endif
    {
        // [映射 2: 转向角度]
    // (注意方向，如果方向反了，加负号: -robot_ctrl.target_angle)
    err_degree = -robot_ctrl.target_angle + g_initial_yaw - euler_angle.yaw;//目标想要增加/减少的角度+初始角度-当前角度

    // [映射 3: 速度控制]
    // 主函数定义: 负数代表向前 (-60 = 20m/s)
    // 遥控器逻辑: 假设推油门 robot_ctrl.target_speed 为正数
    // 转换逻辑: 取反
    target_speed_set = -robot_ctrl.target_speed;
    }
    
    // [可选: 保护] 如果处于未使能状态，强制目标速度归零，防止后台积分
    if((g_motor_enable == 0) || (g_fallen)) {
        target_speed_set = 0.0f;
        Accel_Feedforward_Reset();//【accel_ff】关机/急停时清空加速前馈
        // 同时清除遥控器内部积分，防止再次使能时车突然冲出去
        robot_ctrl.target_speed = 0.0f; 
    }
#endif

}

void pit0_ch2_isr()                     // 定时器通道 2 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH2);
}

void pit0_ch10_isr()                    // 定时器通道 10 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH10);
    key_scanner();   // 必须定期调用！可写在中断或者循环
    Menu_HandleKey();
}

void pit0_ch11_isr()                    // 定时器通道 11 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH11);
    
}

void pit0_ch12_isr()                    // 定时器通道 12 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH12);
    
}

void pit0_ch13_isr()                    // 定时器通道 13 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH13);
    
}

void pit0_ch14_isr()                    // 定时器通道 14 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH14);
    
}

void pit0_ch15_isr()                    // 定时器通道 15 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH15);
    
}

void pit0_ch16_isr()                    // 定时器通道 16 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH16);
    
}

void pit0_ch17_isr()                    // 定时器通道 17 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH17);
    
}

void pit0_ch18_isr()                    // 定时器通道 18 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH18);
    
}

void pit0_ch19_isr()                    // 定时器通道 19 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH19);
    
}

void pit0_ch20_isr()                    // 定时器通道 20 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH20);
    
}

void pit0_ch21_isr()                    // 定时器通道 21 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH21);
    tsl1401_collect_pit_handler();
}
// **************************** PIT中断函数 ****************************


// **************************** 外部中断函数 ****************************
void gpio_0_exti_isr()                  // 外部 GPIO_0 中断服务函数     
{
    
  
  
}

void gpio_1_exti_isr()                  // 外部 GPIO_1 中断服务函数     
{
    if(exti_flag_get(P01_0))		// 示例P1_0端口外部中断判断
    {

      
      
            
    }
    if(exti_flag_get(P01_1))
    {

            
            
    }
}

void gpio_2_exti_isr()                  // 外部 GPIO_2 中断服务函数     
{
    if(exti_flag_get(P02_0))
    {
            
            
    }
    if(exti_flag_get(P02_4))
    {
            
            
    }

}

void gpio_3_exti_isr()                  // 外部 GPIO_3 中断服务函数     
{



}

void gpio_4_exti_isr()                  // 外部 GPIO_4 中断服务函数     
{



}

void gpio_5_exti_isr()                  // 外部 GPIO_5 中断服务函数     
{



}

void gpio_6_exti_isr()                  // 外部 GPIO_6 中断服务函数     
{
	


}

void gpio_7_exti_isr()                  // 外部 GPIO_7 中断服务函数     
{



}

void gpio_8_exti_isr()                  // 外部 GPIO_8 中断服务函数     
{



}

void gpio_9_exti_isr()                  // 外部 GPIO_9 中断服务函数     
{



}

void gpio_10_exti_isr()                  // 外部 GPIO_10 中断服务函数     
{



}

void gpio_11_exti_isr()                  // 外部 GPIO_11 中断服务函数     
{



}

void gpio_12_exti_isr()                  // 外部 GPIO_12 中断服务函数     
{



}

void gpio_13_exti_isr()                  // 外部 GPIO_13 中断服务函数     
{



}

void gpio_14_exti_isr()                  // 外部 GPIO_14 中断服务函数     
{



}

void gpio_15_exti_isr()                  // 外部 GPIO_15 中断服务函数     
{



}

void gpio_16_exti_isr()                  // 外部 GPIO_16 中断服务函数     
{



}

void gpio_17_exti_isr()                  // 外部 GPIO_17 中断服务函数     
{



}

void gpio_18_exti_isr()                  // 外部 GPIO_18 中断服务函数     
{



}

void gpio_19_exti_isr()                  // 外部 GPIO_19 中断服务函数     
{



}

void gpio_20_exti_isr()                  // 外部 GPIO_20 中断服务函数     
{
    // // ==========================================================
    // // 按键 1 (P20_0): 开始录制
    // // ==========================================================
    // if(exti_flag_get(EXTI_PORT20_0))
    // {
    //     gpio_toggle_level(P19_0);       // 指示灯切换
        
    //     if (g_motor_enable && g_yaw_initialized)
    //     {      
    //         g_nav_start_recording = 1;//开始录制，会在main中调用惯导系统初始化和点的记录的初始化,然后再变回1
    //         g_nav_recording = 1;
    //     }
    // }
    // // ==========================================================
    // // 按键 2 (P20_1): 停止录制并请求保存
    // // ==========================================================
    // if(exti_flag_get(EXTI_PORT20_1))
    // {
      
    //     gpio_toggle_level(P19_0);       // 指示灯切换
    //     if (g_nav_recording)
    //     {
    //         g_nav_recording = 0; // 停止录制
            
    //         // 只有电机仍开启才保存（防止倒地保存）
    //         if (g_motor_enable) 
    //         {
    //             g_save_flash_request = 1; // 通知 main 循环执行 Flash 写操作。不能在中断中进行写入，以免阻塞中断
    //         }
    //         else
    //         {
    //             #if DEBUG_LOG_ENABLE
    //             printf("Button2: Recording stopped (motor disabled), data discarded.\r\n");
    //             #endif
    //         }
    //     }
    // }

    // // ==========================================================
    // // 按键 3 (P20_2): 读取，然后开始复现轨迹
    // // ==========================================================
    // if(exti_flag_get(EXTI_PORT20_2))
    // {
       
    //     gpio_toggle_level(P19_0);       // 指示灯切换
        
    //     if(g_motor_enable)
    //     {
    //         g_load_flash_request = 1;      // 请求读取测试
    //         g_save_flash_request = 0;     // 清除保存请求
    //         g_nav_recording = 0;          // 确保停止录制
    //     }
    // }
}

void gpio_21_exti_isr()                  // 外部 GPIO_21 中断服务函数     
{
    
}

void gpio_22_exti_isr()                  // 外部 GPIO_22 中断服务函数     
{

}

void gpio_23_exti_isr()                  // 外部 GPIO_23 中断服务函数     
{



}
// **************************** 外部中断函数 ****************************

//// **************************** DMA中断函数 ****************************
//void dma_event_callback(void* callback_arg, cyhal_dma_event_t event)
//{
//    CY_UNUSED_PARAMETER(event);
//	
//
//	
//	
//}
// **************************** DMA中断函数 ****************************

void uart_rx_interrupt_handler (void);
// **************************** 串口中断函数 ****************************
// 串口0默认作为调试串口
void uart0_isr (void)
{
    if(Cy_SCB_GetRxInterruptMask(get_scb_module(UART_0)) & CY_SCB_UART_RX_NOT_EMPTY)            // 串口0接收中断
    {
        Cy_SCB_ClearRxInterrupt(get_scb_module(UART_0), CY_SCB_UART_RX_NOT_EMPTY);              // 清除接收中断标志位
        uart_rx_interrupt_handler();
#if DEBUG_UART_USE_INTERRUPT                        				                // 如果开启 debug 串口中断
        debug_interrupr_handler();                  				                // 调用 debug 串口接收处理函数 数据会被 debug 环形缓冲区读取
#endif                                              				                // 如果修改了 DEBUG_UART_INDEX 那这段代码需要放到对应的串口中断去
      
        
        
    }
    else if(Cy_SCB_GetTxInterruptMask(get_scb_module(UART_0)) & CY_SCB_UART_TX_DONE)            // 串口0发送中断
    {           
        Cy_SCB_ClearTxInterrupt(get_scb_module(UART_0), CY_SCB_UART_TX_DONE);                   // 清除接收中断标志位
        
        
        
    }
}


void uart1_isr (void)
{
    if(Cy_SCB_GetRxInterruptMask(get_scb_module(UART_1)) & CY_SCB_UART_RX_NOT_EMPTY)            // 串口1接收中断
    {
        Cy_SCB_ClearRxInterrupt(get_scb_module(UART_1), CY_SCB_UART_RX_NOT_EMPTY);              // 清除接收中断标志位
      
                uart_receiver_handler();    
        //wireless_module_uart_handler();
        
        
    }
    else if(Cy_SCB_GetTxInterruptMask(get_scb_module(UART_1)) & CY_SCB_UART_TX_DONE)            // 串口1发送中断
    {
        Cy_SCB_ClearTxInterrupt(get_scb_module(UART_1), CY_SCB_UART_TX_DONE);                   // 清除接收中断标志位
        
        
        
    }
}

void uart2_isr (void)
{
    if(Cy_SCB_GetRxInterruptMask(get_scb_module(UART_2)) & CY_SCB_UART_RX_NOT_EMPTY)            // 串口2接收中断
    {
        Cy_SCB_ClearRxInterrupt(get_scb_module(UART_2), CY_SCB_UART_RX_NOT_EMPTY);              // 清除接收中断标志位

        gnss_uart_callback();
        
        
    }
    else if(Cy_SCB_GetTxInterruptMask(get_scb_module(UART_2)) & CY_SCB_UART_TX_DONE)            // 串口2发送中断
    {
        Cy_SCB_ClearTxInterrupt(get_scb_module(UART_2), CY_SCB_UART_TX_DONE);                   // 清除接收中断标志位
        
        
        
    }
}

void uart3_isr (void)
{
    if(Cy_SCB_GetRxInterruptMask(get_scb_module(UART_3)) & CY_SCB_UART_RX_NOT_EMPTY)            // 串口3接收中断
    {
        Cy_SCB_ClearRxInterrupt(get_scb_module(UART_3), CY_SCB_UART_RX_NOT_EMPTY);              // 清除接收中断标志位

        
        
        
    }
    else if(Cy_SCB_GetTxInterruptMask(get_scb_module(UART_3)) & CY_SCB_UART_TX_DONE)            // 串口3发送中断
    {
        Cy_SCB_ClearTxInterrupt(get_scb_module(UART_3), CY_SCB_UART_TX_DONE);                   // 清除接收中断标志位
        
        
        
    }
}

void uart4_isr (void)
{
    
    if(Cy_SCB_GetRxInterruptMask(get_scb_module(UART_4)) & CY_SCB_UART_RX_NOT_EMPTY)            // 串口4接收中断
    {
        Cy_SCB_ClearRxInterrupt(get_scb_module(UART_4), CY_SCB_UART_RX_NOT_EMPTY);              // 清除接收中断标志位

        uart_control_callback();  
        //uart_receiver_handler();                                                                // 串口接收机回调函数
        
                                                            
        
    }
    else if(Cy_SCB_GetTxInterruptMask(get_scb_module(UART_4)) & CY_SCB_UART_TX_DONE)            // 串口4发送中断
    {
        Cy_SCB_ClearTxInterrupt(get_scb_module(UART_4), CY_SCB_UART_TX_DONE);                   // 清除接收中断标志位
        
        
        
    }
}
// **************************** 串口中断函数 ****************************
