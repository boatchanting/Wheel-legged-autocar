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
* 文件名称          main_cm7_0
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT4BB
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-4       pudding            first version
********************************************************************************************************************/

#include "main0/init_main0.h"

// 全局变量定义区域 (已在 init_main0.h 外部声明)
uint8 uart_get_data[64];                                                        // 串口接收数据缓冲区
uint8 fifo_get_data[64];                                                        // fifo 输出读出缓冲区
uint8  get_data = 0;                                                            // 接收数据变量
uint32 fifo_data_count = 0;                                                     // fifo 数据个数
fifo_struct uart_data_fifo;

int8 duty = 0;
bool dir = true;

volatile uint8 pit_state = 0;  //通道0中断标志位
uint8 pit_state_1 = 0;//通道1中断标志位
volatile runtime_profiler_t g_ekf_profiler = {0};

#if IMU_REFRESH_TEST_ENABLE
extern volatile uint32 g_imu_refresh_test_elapsed_ms;
extern volatile uint32 g_imu_refresh_test_gyro_change_count;
extern volatile uint32 g_imu_refresh_test_acc_change_count;
extern volatile uint32 g_imu_refresh_test_mag_change_count;
extern volatile uint32 g_imu_refresh_test_gyro_freq_x100;
extern volatile uint32 g_imu_refresh_test_acc_freq_x100;
extern volatile uint32 g_imu_refresh_test_mag_freq_x100;
extern volatile uint8 g_imu_refresh_test_done;
extern volatile uint8 g_imu_refresh_test_start_beep_request;
extern volatile uint8 g_imu_refresh_test_done_beep_request;
#endif


// *************************** EKF中断声明 ***************************
extern void IMU_Calibrate_All_Gyro(void); // 校准陀螺仪声明
extern void EKF_Init(void);
extern void EKF_UpData(void);
extern EulerAngles euler_angle; // 引用 ekf.c 中计算出的角度
// =================================================================================
// PID控制中间变量开始
float pid_out_speed = 0.0f; // 速度环输出 (角度调整量)
float pid_out_angle = 0.0f; // 角度环输出 (期望角速度)
float pid_out_pwm   = 0.0f; // 角速度环输出 (电机占空比)
int g_motor_enable =G_MOTOR_ENABLE_INIT; // 电机使能安全开关，1为使能，0为关机
#if REMOTE_CONTROL
volatile bool g_fallen = false; // 主动起立/倒下控制，false为尝试起立，true为保持倒下
#else
volatile bool g_fallen = true;  // 无遥控器时默认保持倒下，等待菜单触发起立发车
#endif
// =================================================================================

// =================================================================================
// 导航记录控制标志位
volatile uint8_t g_nav_recording = 0;       // 1: 正在记录 RAM, 0: 停止记录
volatile uint8_t g_nav_start_recording = 0;  // 1: 请求开始录制，创建内存区
volatile uint8_t g_save_flash_request = 0;  // 1: 请求将 RAM 数据存入 Flash
volatile uint8_t g_load_flash_request = 0;      // 1: 请求从 Flash 加载数据
volatile uint8_t g_replay_start_request = 0;
volatile uint8_t g_replay_stop_request = 0;
volatile uint8_t vision_detected_bumpy_point = 0; // 模拟视觉检测到“颠簸入口”
// =================================================================================

int main(void)
{
    // 调用剥离的初始化函数
    Main0_Init();
    
    uint8 display_count = 0; // 用于屏幕刷新分频
    uint8 ekf_print_div = 0; // 50ms*10 = 500ms 
#if IMU_REFRESH_TEST_ENABLE
    uint8 imu_refresh_test_printed = 0; // IMU刷新率测试结果只打印一次
#endif

    while(true)
    {
#if IMU_REFRESH_TEST_ENABLE
        if (g_imu_refresh_test_start_beep_request != 0U)
        {
            g_imu_refresh_test_start_beep_request = 0;
            Buzzer_Beep_By_PointType(0); // 姿态角收敛后，IMU刷新率测试开始响一声
        }

        if (g_imu_refresh_test_done_beep_request != 0U)
        {
            g_imu_refresh_test_done_beep_request = 0;
            Buzzer_Beep_By_PointType(0); // IMU刷新率测试结束响一声
        }

        if ((g_imu_refresh_test_done != 0U) && (imu_refresh_test_printed == 0U))
        {
            printf("[IMU_REFRESH] time=%lu ms\r\n",
                   (unsigned long)g_imu_refresh_test_elapsed_ms);
            printf("[IMU_REFRESH] gyro: changes=%lu, freq=%lu.%02lu Hz\r\n",
                   (unsigned long)g_imu_refresh_test_gyro_change_count,
                   (unsigned long)(g_imu_refresh_test_gyro_freq_x100 / 100U),
                   (unsigned long)(g_imu_refresh_test_gyro_freq_x100 % 100U));
            printf("[IMU_REFRESH] acc : changes=%lu, freq=%lu.%02lu Hz\r\n",
                   (unsigned long)g_imu_refresh_test_acc_change_count,
                   (unsigned long)(g_imu_refresh_test_acc_freq_x100 / 100U),
                   (unsigned long)(g_imu_refresh_test_acc_freq_x100 % 100U));
#if IMU_CATEGORY == 3
            printf("[IMU_REFRESH] mag : changes=%lu, freq=%lu.%02lu Hz\r\n",
                   (unsigned long)g_imu_refresh_test_mag_change_count,
                   (unsigned long)(g_imu_refresh_test_mag_freq_x100 / 100U),
                   (unsigned long)(g_imu_refresh_test_mag_freq_x100 % 100U));
#endif
            imu_refresh_test_printed = 1;
        }
#endif

        // 检查中断标志位 (由 isr.c 中的 pit0_ch0_isr 置位)
        if(pit_state == 1)//10mswifi，100ms屏幕刷新
        {
            // 如果需要 WiFi 发送，建议也放在这里(50ms一次)，或者放在5ms的逻辑里
            pit_state = 0; // 清除标志   

            // ekf_print_div++;
            // if(ekf_print_div >= 10)
            // {
            //     uint32 ekf_cnt = g_ekf_profiler.count;
            //     if(ekf_cnt > 0U)
            //     {
            //         printf("[EKF] cnt=%lu, avg=%lu us, min=%lu us, max=%lu us, last=%lu us\r\n",
            //                (unsigned long)g_ekf_profiler.count,
            //                (unsigned long)g_ekf_profiler.avg_us,
            //                (unsigned long)g_ekf_profiler.min_us,
            //                (unsigned long)g_ekf_profiler.max_us,
            //                (unsigned long)g_ekf_profiler.last_us);
            //     }
            //     ekf_print_div = 0;
            // }
            #if WIFI_CORE0_CUSTOM_PROTOCOL
                wifi_protocol_send_data();//自定义wifi协议（惯导/GNSS/打点状态）
            #endif
                //TelemetryIpc_Core0_PublishPvcDefault();
            #if WIFI_CORE0_ASSISTANT
                //逐飞助手示波器发送代码        
                //1.【调试直立环，左右轮，俯仰角，角速度环输出，角度环输出，舵机环输出，翻滚角，偏航角】
                seekfree_assistant_oscilloscope_data.data[0] = (float)motor_value.receive_left_speed_data;
                seekfree_assistant_oscilloscope_data.data[1] = (float)motor_value.receive_right_speed_data;
                seekfree_assistant_oscilloscope_data.data[2] = (float)euler_angle.pitch;
                seekfree_assistant_oscilloscope_data.data[3] = (float)gyro_loop_out;
                seekfree_assistant_oscilloscope_data.data[4] = (float)pid_angle.output;
                seekfree_assistant_oscilloscope_data.data[5] = (float)pid_servo_speed.error_integral;
                seekfree_assistant_oscilloscope_data.data[6] = (float)euler_angle.roll;
                seekfree_assistant_oscilloscope_data.data[7] = (float)euler_angle.yaw;


                // 2.【调试转向环，左右轮，偏航角，转向角速度环输出，转向角度环输出，舵机环输出，翻滚角，俯仰角】
                // seekfree_assistant_oscilloscope_data.data[0] = (float)motor_value.receive_left_speed_data;
                // seekfree_assistant_oscilloscope_data.data[1] = (float)motor_value.receive_right_speed_data;
                // seekfree_assistant_oscilloscope_data.data[2] = (float)euler_angle.pitch;
                // seekfree_assistant_oscilloscope_data.data[3] = (float)pid_gyro.output;
                // seekfree_assistant_oscilloscope_data.data[4] = (float)pid_turn_angle.output;
                // seekfree_assistant_oscilloscope_data.data[5] = (float)pid_servo_speed.output;
                // seekfree_assistant_oscilloscope_data.data[6] = (float)euler_angle.roll;
                // seekfree_assistant_oscilloscope_data.data[7] = (float)euler_angle.yaw;


                // //3.【调试遥控器，前六个通道】
                // seekfree_assistant_oscilloscope_data.data[0] = (float)uart_receiver.channel[0];
                // seekfree_assistant_oscilloscope_data.data[1] =(float)uart_receiver.channel[1];
                // seekfree_assistant_oscilloscope_data.data[2] = (float)uart_receiver.channel[2];
                // seekfree_assistant_oscilloscope_data.data[3] = (float)uart_receiver.channel[3];
                // seekfree_assistant_oscilloscope_data.data[4] =(float)uart_receiver.channel[4];
                // seekfree_assistant_oscilloscope_data.data[5] = (float)uart_receiver.channel[5];
                // seekfree_assistant_oscilloscope_data.data[6] = 0.0f;//(float)uart_receiver.channel[6];
                // seekfree_assistant_oscilloscope_data.data[7] = 0.0f;//(float)uart_receiver.channel[7];

                // 4.【调节颠簸路段状态机】
                // seekfree_assistant_oscilloscope_data.data[0] = (float)motor_value.receive_left_speed_data;
                // seekfree_assistant_oscilloscope_data.data[1] = (float)motor_value.receive_right_speed_data;
                // seekfree_assistant_oscilloscope_data.data[2] = (float)(gyro_loop_out + turn_gyro_loop_out); 
                // seekfree_assistant_oscilloscope_data.data[3] = (float)(-gyro_loop_out + turn_gyro_loop_out); 
                // seekfree_assistant_oscilloscope_data.data[4] = (float)target_speed_set;
                // seekfree_assistant_oscilloscope_data.data[5] = (float)err_degree;
                // seekfree_assistant_oscilloscope_data.data[6] = (float)euler_angle.pitch;
                // seekfree_assistant_oscilloscope_data.data[7] = (float)euler_angle.yaw; 

                // 4.【调节pvc识别】
                // data[0] 左轮速度
                // seekfree_assistant_oscilloscope_data.data[0] = (float)motor_value.receive_left_speed_data;

                // // data[1] 右轮速度
                // seekfree_assistant_oscilloscope_data.data[1] = (float)motor_value.receive_right_speed_data;

                // // data[2] 航向误差 (PVC 第一版暂时为 0)
                // seekfree_assistant_oscilloscope_data.data[2] = (float)g_vision_ipc_latest.pvc_yaw_error_deg_x100 / 100.0f;

                // // data[3] 横向偏差
                // seekfree_assistant_oscilloscope_data.data[3] = (float)g_vision_ipc_latest.pvc_lateral_mm;

                // // data[4] 目标速度
                // seekfree_assistant_oscilloscope_data.data[4] = (float)target_speed_set;

                // // data[5] 置信度 (0~1000)
                // seekfree_assistant_oscilloscope_data.data[5] = (float)g_vision_ipc_latest.pvc_confidence_u16;

                // // data[6] 近端白边行号
                // seekfree_assistant_oscilloscope_data.data[6] = (float)g_vision_ipc_latest.pvc_entry_bottom_y;

                // // data[7] 远端白边行号
                // seekfree_assistant_oscilloscope_data.data[7] = (float)g_vision_ipc_latest.pvc_entry_top_y;

                    // 4. 设置本次发送的通道数量 (一共8个数据)
                seekfree_assistant_oscilloscope_data.channel_num = 8;
                    
                    // 5. 调用发送函数
                seekfree_assistant_oscilloscope_send(&seekfree_assistant_oscilloscope_data);

                //用于上位机向小车发送pid信息
                wifi_update_pid_params(); 
            #endif
            //下面撰写的是100ms执行一次的代码
            // --- 屏幕刷新逻辑 (降频处理) ---
            display_count++;
            if(display_count >= 10) // 10* 10 ms = 100ms 刷新一次屏幕
            {
                display_count = 0;    
                #if DEBUG_DISPLAY_CORE0
                    Menu_ShowStatic();    // 静态显示
                    Menu_ShowDynamic();   // 动态显示
                #endif    
            }
        }


        // fifo_data_count = fifo_used(&uart_data_fifo);                           // 查看 fifo 是否有数据
        // if(fifo_data_count != 0)                                                // 读取到数据了
        // {
        //     fifo_read_buffer(&uart_data_fifo, fifo_get_data, &fifo_data_count, FIFO_READ_AND_CLEAN);    // 将 fifo 中数据读出并清空 fifo 挂载的缓冲
        //     uart_write_string(UART_INDEX, "\r\nUART get data:");                // 输出测试信息
        //     uart_write_buffer(UART_INDEX, fifo_get_data, fifo_data_count);      // 将读取到的数据发送出去
        // }

        #if WIFI_CORE0_USE && WIFI_IMAGE_SEND//wifi开关和图像发送开关均开启则发送图像
        // 处理摄像头图像数据
        if(mt9v03x_finish_flag)
        {
            mt9v03x_finish_flag = 0;
            // 在发送前将图像备份再进行发送，这样可以避免图像出现撕裂的问题
            memcpy(image_copy[0], mt9v03x_image[0], MT9V03X_IMAGE_SIZE);

            // 发送图像
            seekfree_assistant_camera_send();
            // 如果使用UDP协议传输数据则推荐在数据全部发送到模块之后立即调用wifi_spi_udp_send_now()函数，以告知模块立即将收到的数据发送到网络上
            // 如果没有立即调用则模块会在持续2毫秒未收到数据后，将数据发送到网络上
            // 调用wifi_spi_udp_send_now()前传输给模块的数据数量建议不要超过40960字节
            // wifi_spi_udp_send_now();
        }
        #endif

        if (vision_detected_marker == 1) {
            minefield_flag = 1; // 触发旋转
            vision_detected_marker = 0;
        }//雷区旋转调用，测试用

        //模拟视觉触发跳跃测试
        if (vision_detected_jump_point == 1) 
        {
            jump_trigger(); // <--- 只需要调用这一句
            vision_detected_jump_point = 0; // 清除标志位，防止连续触发
        }

         // 2. 模拟视觉触发【三级跳】测试
        if (vision_detected_three_jump_point == 1) 
        {
            // 判断当前是否处于空闲状态，防止跳跃中途重复触发打断动作
            if (!VisionThreeStageControl_IsActive()) 
            {
                VisionThreeStageControl_Start(); // <--- 启动三级跳状态机
            }
            vision_detected_three_jump_point = 0; // 清除标志位
        }

        // 模拟视觉触发颠簸测试
        if (vision_detected_bumpy_point == 1)
        {
            BumpyRoad_Trigger();             // <--- 只需要调用这一句
            vision_detected_bumpy_point = 0; // 清除标志位，防止连续触发
        }

        // 模拟视觉触发单边桥正式任务
        if (vision_detected_bridge_point == 1) 
        {
            // 判断当前是否处于空闲状态，防止任务中途重复触发打断动作
            if (!VisionBridgeTask_IsActive()) 
            {
                VisionBridgeTask_Start(); // 启动正式单边桥视觉任务：先 PVC 进门，再巡线找桥
            }
            vision_detected_bridge_point = 0; // 清除标志位，避免重复触发
        }

        // 纯惯导触发单边桥正式任务
        // if (vision_detected_bridge_point == 1) 
        // {
        //     // 判断当前是否处于空闲状态，防止测试中途重复触发打断动作
        //     if (!Bridge_Test_Triple_SingleSide_Is_Active()) 
        //     {
        //         Bridge_Test_Triple_SingleSide_Start(); // 启动单边桥测试状态机
        //     }
        //     vision_detected_bridge_point = 0; // 清除标志位，避免重复触发
        // }



        // ---------------------------------------------------------
        // ---------------- 【nav.1】初始化惯性导航模块和打点模块 ----------------
        // ---------------------------------------------------------
        if (g_nav_start_recording)
        {            
            InertialNav_Init();
            NavRam_Init();
            #if DEBUG_LOG_ENABLE
                printf("[NAV] Init OK: x=%.2f y=%.2f yaw=%.2f\r\n",
                       inertial_nav.x,
                       inertial_nav.y,
                       inertial_nav.relative_yaw);
            #endif
            Buzzer_Beep_By_PointType(2);//叫三次
            g_nav_start_recording = 0;//初始化后置0处理
        }


        // ---------------------------------------------------------
        // ---------------- 【nav.2】打点处理 ----------------
        // ---------------------------------------------------------
        // if (robot_ctrl.mark_trigger)
        // {
        //     uint8_t ret;

        //     // 写入 RAM
        //     ret = NavRam_RecordPoint(robot_ctrl.point_type);

        //     if (ret == 0)
        //     {
        //         // 写入成功 → 蜂鸣器反馈
        //         Buzzer_Beep_By_PointType(robot_ctrl.point_type);

        //     #if DEBUG_LOG_ENABLE
        //         printf("[NAV] Record OK: idx=%d type=%d x=%.2f y=%.2f\r\n",
        //                NavRam_GetPointCount() - 1,
        //                robot_ctrl.point_type,
        //                inertial_nav.x,
        //                inertial_nav.y);
        //     #endif
        //     }
        //     else
        //     {
        //         // RAM 满
        //         #if DEBUG_LOG_ENABLE
        //             printf("[NAV] Record FAILED: RAM FULL\r\n");
        //         #endif
        //     }

        //     // ★ 必须清零，否则会重复写入 ★
        //     robot_ctrl.mark_trigger = 0;
        // }

    // ---------------------------------------------------------
    //  【nav.3】静态点表模式：忽略 Flash 保存请求
    // ---------------------------------------------------------
    if(g_save_flash_request == 1)
    {
        #if DEBUG_LOG_ENABLE
        printf("Main: Flash save disabled (static route mode).\r\n");
        #endif
        g_save_flash_request = 0;
    }

    // ---------------------------------------------------------
    //  【nav.4】静态点表模式：加载 C 点表并开始复现
    // ---------------------------------------------------------
    if (g_motor_enable == 1 && g_load_flash_request == 1)
    {
        g_load_flash_request = 0;

        InertialNav_Init();

        NavReplay_Start();//start replay
        #if DEBUG_LOG_ENABLE
        printf("Main: Static route loaded.\r\n");
        printf("Main: Starting Inertial Navigation...\r\n");
        #endif

        Buzzer_Beep_By_PointType(2);//beep x3
    }

    #if GNSS_NAV == 1
    // ---------------------------------------------------------
    //  【gps-nav】静态点表模式：开始纯GPS复刻
    // ---------------------------------------------------------
    if (g_motor_enable == 1 && g_replay_start_request == 1)
    {
        g_replay_start_request = 0;
        Gnss_Transform_Reset_Origin();
        GpsNavReplay_Start();
        #if DEBUG_LOG_ENABLE
        printf("Main: Starting pure GPS replay...\r\n");
        #endif
        Buzzer_Beep_By_PointType(2);
    }

    // ---------------------------------------------------------
    //  【gps-nav】停止纯GPS复刻
    // ---------------------------------------------------------
    if (g_replay_stop_request == 1)
    {
        g_replay_stop_request = 0;
        GpsNavReplay_Stop();
        #if DEBUG_LOG_ENABLE
        printf("Main: Pure GPS replay stopped.\r\n");
        #endif
    }
    #endif


        // 此处编写需要循环执行的代码
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       UART_INDEX 的接收中断处理函数 这个函数将在 UART_INDEX 对应的中断调用
// 参数说明       void
// 返回参数       void
// 使用示例       uart_rx_interrupt_handler();
//-------------------------------------------------------------------------------------------------------------------
void uart_rx_interrupt_handler (void)
{
//    get_data = uart_read_byte(UART_INDEX);                                      // 接收数据 while 等待式 不建议在中断使用
    if(uart_query_byte(UART_INDEX, &get_data))                                  // 接收数据 查询式 有数据会返回 TRUE 没有数据会返回 FALSE
    {
        fifo_write_buffer(&uart_data_fifo, &get_data, 1);                       // 将数据写入 fifo 中
    }
}


