#include "zf_common_headfile.h"

// 定义最大角度和最大速度的宏
#define ANGLE_MAX  2000.0
#define SPEED_MAX  3000.0

// 测试数据变量
float test_data = 0;

// 速度环PID参数（用于舵机控制）
// 格式：{kp, ki, kd, error, error_Integral, kp, kd}
// 初始值：{0, 0, 0, 0, 5, 0.025, 0}
// 备用参数：{0, 0, 0, 0, 1, 2, 0} 和 {0, 0, 0, 0, 6, 0.03, 0}
PID_param speed_pid_s = {0, 0, 0, 0, 5, 0.025, 0};      //{0, 0, 0, 0, 1, 2, 0};        {0, 0, 0, 0, 6, 0.03, 0}

// 速度环PID参数（用于无刷电机控制）
PID_param speed_pid_v = {0, 0, 0, 0, 0, 0, 0};      //{0, 0, 0, 0, 1, 2, 0};        {0, 0, 0, 0, 0, 70, 0};

// 角度环PID参数
// 初始值：{0, 0, 0, 0, -1600, 0, -850}
// 备用参数：{0, 0, 0, 0, -3, 0, 0} 和 {0, 0, 0, 0, -600, 0, -300}
PID_param angle_pid = {0, 0, 0, 0, -1600, 0, -850};        //{0, 0, 0, 0, -3, 0, 0};       {0, 0, 0, 0, -600, 0, -300};
// 备用角度环PID参数（已注释）
//PID_param angle_pid = {0, 0, 0, 0, 0, 0, 0};

// 陀螺仪环PID参数
PID_param gyro_pid =  {0, 0, 0, 0, 0.5, 0, 0};        //{0, 0, 0, 0, 1, 0, 0}         {0, 0, 0, 0, 0.5, 0, 0}

// 转向内环（陀螺仪）PID参数（已注释）
//PID_param gyro_turn_pid =  {0, 0, 0, 0, 1.5, 0, 1.0};           //转向内环    750定速、770变速、新胎
// 转向外环（角度）PID参数（已注释）
//PID_param angle_turn_pid =  {0, 0, 0, 0, 1100, 0, 550};           //转向外环

// 转向内环（陀螺仪）PID参数（当前使用）
PID_param gyro_turn_pid =  {0, 0, 0, 0, 1.5, 0, 1.0};           //转向内环    750定速、770变速、新胎
// 转向外环（角度）PID参数（当前使用）
PID_param angle_turn_pid =  {0, 0, 0, 0, 800, 0, 1600};           //转向外环

// 转向内环（陀螺仪）PID参数（老胎版本，已注释）
//PID_param gyro_turn_pid =  {0, 0, 0, 0, 0.8, 0, 0.4};           //转向内环    750定速、770变速、老胎
// 转向外环（角度）PID参数（老胎版本，已注释）
//PID_param angle_turn_pid =  {0, 0, 0, 0, 1100, 0, 550};           //转向外环

// 调试用变量（已注释）
//float gkd = 0;      //陀螺仪kd
//float tkp = 0;      //二次kp

// 调试用PID参数（已注释）
//PID_param angle_turn_pid =  {0, 0, 0, 0, 0, 0, 0};      //调试用
//PID_param speed_pid_s = {0, 0, 0, 0, 0, 0, 0};      //{0, 0, 0, 0, 1, 2, 0};        {0, 0, 0, 0, 0, 70, 0};//调试用
//PID_param gyro_pid =  {0, 0, 0, 0, 0, 0, 0};          //调试用

// 横滚角PID参数
PID_param rolling_angle_pid =  {0, 0, 0, 0, 1, 2, 0};           //转向外环 1650, 0, 650 550-650


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     X轴行进电机控制串级PD，内环（角速度环）
// 参数说明     gyro实际角速度，PWM2是期望值
// 返回参数     void,传给PWM3
// 使用示例     CascadePID_Gyro(gyro);
// 备注信息     位置式PID,1ms运算一次
//y轴30 1.1m    48 0.6m   119 0.35m
//-------------------------------------------------------------------------------------------------------------------
float CascadePID_Gyro(int16 gyro, float gyro_qiwang)
{
//    return PWM2;  // 直接返回期望值（已注释）

    float PWM;

//    gyro_pid.kp = data[0];       // 动态调整kp（已注释）
//    gyro_pid.kd = data[1];       // 动态调整kd（已注释）

    // 计算误差
    gyro_pid.error = gyro_qiwang - gyro;
    // 积分项累加
    gyro_pid.error_Integral += gyro_pid.error;

    // 积分限幅
    if( gyro_pid.error_Integral >= 1000)
        gyro_pid.error_Integral = 1000;
    else if( gyro_pid.error_Integral <= -1000)
        gyro_pid.error_Integral = -1000;

    // 计算PID输出
    PWM = gyro_pid.error * gyro_pid.kp + gyro_pid.error_Integral * gyro_pid.ki + ( gyro_pid.error -  gyro_pid.lasterror) * gyro_pid.kd;
    // 更新上一次误差
    gyro_pid.lasterror = gyro_pid.error;

    // 根据单边桥标志进行PWM限幅
    if (danbianqiao_flag)
    {
        if (danbianqiao_flag==2)
        {
            if(PWM > 6500)
                PWM = 6500;
            else if(PWM < -6500)
                PWM = -6500;
        }
        else
        {
            if(PWM > 5000)
                PWM = 5000;
            else if(PWM < -5000)
                PWM = -5000;
        }
    }
    else
    {
        if(PWM > 5000)
            PWM = 5000;
        else if(PWM < -5000)
            PWM = -5000;
    }
//    if(my_abs(PWM) < 100)  // 死区设置（已注释）
//        PWM = 0;


    return PWM;
}



//-------------------------------------------------------------------------------------------------------------------
// 函数简介     X轴行进电机控制串级PD，中环（角度环）
// 参数说明     angle实际角度，angle_zero期望角度(y轴机械零点)，PWM1是期望角度值，gyro实际角速度
// 返回参数     void,传给PWM2
// 使用示例     void CascadePID_angle(float angle, float angle_zero);
// 备注信息     位置式PID,5ms运算一次
//-------------------------------------------------------------------------------------------------------------------
float CascadePID_angle(float angle, float angle_zero, float angle_qiwang)
{
//    return PWM1;  // 直接返回期望值（已注释）
    float PWM;
    float kp;
//    angle_pid.kp = data[2];       // 动态调整kp（已注释）
//    angle_pid.kd = data[3];       // 动态调整kd（已注释）

//    if(danbianqiao_flag==1 || danbianqiao_flag==2)  // 单边桥时kp调整（已注释）
//        kp = 2 * angle_pid.kp;
//    else
//        if(danbianqiao_flag!=99 && danbianqiao_flag)
//        kp = 1.5 * angle_pid.kp;
//    else
        kp = angle_pid.kp;


    // 计算误差
    angle_pid.error= angle_qiwang + angle_zero - angle;
    // 积分项累加
    angle_pid.error_Integral += angle_pid.error;

    // 积分限幅
    if( angle_pid.error_Integral >= 500)
        angle_pid.error_Integral = 500;
    else if( angle_pid.error_Integral <= -500)
        angle_pid.error_Integral = -500;

    // 计算PID输出
    PWM = angle_pid.error * kp + angle_pid.error_Integral * angle_pid.ki + ( angle_pid.error -  angle_pid.lasterror) * angle_pid.kd;
    // 更新上一次误差
    angle_pid.lasterror = angle_pid.error;

//    if (danbianqiao_flag)  // 单边桥时PWM限幅（已注释）
//    {
        if (danbianqiao_flag==2)
        {
            if(PWM > 10000)
                PWM = 10000;
            else if(PWM < -10000)
                PWM = -10000;
//        }
//        else
//        {
//            if(PWM > 6500)
//                PWM = 6500;
//            else if(PWM < -6500)
//                PWM = -6500;
//        }
    }
    else
    {
        if(PWM > 8500)
            PWM = 8500;
        else if(PWM < -8500)
            PWM = -8500;
    }

    // 跳跃时PWM减半
    if(jump_flag!=0 && jump_flag!=3)
        PWM = PWM * 0.5;

    return PWM;
}



//-------------------------------------------------------------------------------------------------------------------
// 函数简介     X轴行进电机控制串级PI，外环（速度环）        舵机控制
// 参数说明     speed实际速度，speed_qiwang期望速度
// 返回参数     void,传给PWM1
// 使用示例     void CascadePID_speed(int16 speed, 0);
// 备注信息     位置式PI, 20ms运算一次
//-------------------------------------------------------------------------------------------------------------------
// 速度滤波变量
float speed_a=0;
float speed_b=0;
float speed_c=0;
float speed_PWM=0;
float speed_ratio, speed_ratio_last, speed_ratio_prelast;
float speed_qiwang_now=0;
float speed_now, speed_last, speed_prelast;

// 速度环PID函数（舵机控制）
float CascadePID_speed_S(int16 speed, int16 speed_qiwang)
{
    float speed_kp,e,k;
//    speed_pid_s.kp = data[4];       // 动态调整kp（已注释）
//    speed_pid_s.ki = data[5];       // 动态调整ki（已注释）
//    speed_pid_s.kd = data[6];       // 动态调整kd（已注释）

    // 速度滤波处理
    speed_now=speed*0.6+speed_last*0.3+speed_prelast*0.1;
    speed_prelast=speed_last;
    speed_last=speed_now;

    // 初始化当前期望速度
    speed_qiwang_now=speed_qiwang;

    // 动态速度决策
    if (dongtaisudu && (danbianqiao_flag==0||danbianqiao_flag==99) && run_flag)        //速度决策开关
    {
        // 跳跃时速度调整
        if (jump_flag && jump_speed_qiwang!=0) speed_qiwang_now=(jump_speed_qiwang);// * 0.5 + speed_qiwang_now * 0.5;

        // 停止时速度调整
        else if (stop_flag==1 && congci_speed_qiwang!=0) speed_qiwang_now=(congci_speed_qiwang);// * 0.5 + speed_qiwang_now * 0.5;

        // 十字路口时速度调整
        else if (cross_flag==5 && cross_speed_qiwang!=0) speed_qiwang_now=(cross_speed_qiwang+100);// * 0.5 + speed_qiwang_now * 0.5;

//        else if (((cross_time%2==0 && cross_flag)||(cross_mode==1 && cross_time==1 && cross_flag==0)) && cross_speed_qiwang!=0) speed_qiwang_now=(cross_speed_qiwang+50);// * 0.5 + speed_qiwang_now * 0.5;

        // 十字路口奇数时速度调整
        else if ((cross_time%2==1) && cross_speed_qiwang!=0) speed_qiwang_now=(cross_speed_qiwang);// * 0.5 + speed_qiwang_now * 0.5;

        // 十字路口时速度调整
        else if (cross_flag && cross_speed_qiwang!=0) speed_qiwang_now=(cross_speed_qiwang);// * 0.5 + speed_qiwang_now * 0.5;

        // 圆环时速度调整
        else if (yuan_flag && yuan_flag<5 && yuan_speed_qiwang!=0) speed_qiwang_now=(yuan_speed_qiwang);// * 0.5 + speed_qiwang_now * 0.5;

        // 侧边时速度调整
        else if (cebian_flag && cebian_speed_qiwang!=0) speed_qiwang_now=(cebian_speed_qiwang);// * 0.5 + speed_qiwang_now * 0.5;

        // 侧边后速度调整
        else if (cebian_time>=1 && cebian_speed_after_qiwang!=0) speed_qiwang_now=(cebian_speed_after_qiwang);

        // 常规速度调整
        else
        {
            // 根据曲率调整速度
            if (my_abs(Last_Curvature_Value)>1.5)
                speed_count=0;
            else
                speed_count++;

            if (speed_count<10) speed_qiwang=750;

            // 根据中线白点数调整速度
            else if (mid_white_num==60)
                if ( (bin_image[1][l_border[5]-3]==255 && bin_image[1][l_border[5]-3]==255) || bin_image[1][r_border[5]+3]==255)
                speed_qiwang=950;

            else if (mid_white_num>=58) // && my_abs(err_degree)<3
                speed_qiwang=900;

            else if (mid_white_num>=56)
                speed_qiwang=850;

            else if (mid_white_num>=35)
                speed_qiwang=770;

            else
                speed_qiwang=750;
        }
    }

//    else
//        speed_qiwang_now=speed_qiwang;

    // 计算速度误差
    speed_pid_s.error = speed_qiwang_now - speed_now;

    // 根据误差动态调整kp
    e=exp(-my_abs(speed_pid_s.error/6));
    k = (my_abs( (e - 1) / (e + 1) ) * 0.6 + 0.4);
    speed_kp = speed_pid_s.kp * k;

    // 非单边桥时调整舵机限幅
    if (danbianqiao_flag==0)
        servo_limit=speed_servo_limit * k * k;


//    speed_kp = speed_pid_s.kp;

    ///////////////////////////////////////////////////////////////////////////////////////////////////位置式PID
    speed_pid_s.error_Integral += speed_pid_s.error;
    if( speed_pid_s.error_Integral >= 500)
        speed_pid_s.error_Integral = 500;
    else if( speed_pid_s.error_Integral <= -500)
        speed_pid_s.error_Integral = -500;

    speed_PWM = speed_pid_s.error * speed_kp + speed_pid_s.error_Integral * speed_pid_s.ki + ( speed_pid_s.error -  speed_pid_s.lasterror) * speed_pid_s.kd;

    ///////////////////////////////////////////////////////////////////////////////////////////////////增量式PID（已注释）
//    speed_a=speed_pid_s.kp*(speed_pid_s.error-speed_pid_s.lasterror);
//    speed_b=speed_pid_s.ki*speed_pid_s.error;
//    speed_c=speed_pid_s.kd*(speed_pid_s.error-2*speed_pid_s.lasterror+speed_pid_s.prelasterror);
//
//    speed_PWM += speed_a+speed_b+speed_c;


    // 更新误差
    speed_pid_s.prelasterror=speed_pid_s.lasterror;
    speed_pid_s.lasterror=speed_pid_s.error;

//    if (danbianqiao_flag == 1 || danbianqiao_flag == 2)  // 单边桥时PWM限幅（已注释）
//    {
//        if (speed_PWM > danbianqiao_in_servo_limit)
//            speed_PWM = danbianqiao_in_servo_limit;
//        else if (speed_PWM < -danbianqiao_in_servo_limit)
//            speed_PWM = -danbianqiao_in_servo_limit;
//    }
//    else if (danbianqiao_flag > 2 && danbianqiao_flag!=99)
//    {
//        if (speed_PWM > danbianqiao_servo_limit)
//            speed_PWM = danbianqiao_servo_limit;
//        else if (speed_PWM < -danbianqiao_servo_limit)
//            speed_PWM = -danbianqiao_servo_limit;
//    }
//    else
//    {
//        if (speed_PWM > speed_servo_limit)
//            speed_PWM = speed_servo_limit;
//        else if (speed_PWM < -speed_servo_limit)
//            speed_PWM = -speed_servo_limit;
//    }

    return speed_PWM;
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     X轴行进电机控制串级PI，外环（速度环）        无刷控制
// 参数说明     speed实际速度，speed_qiwang期望速度
// 返回参数     void,传给PWM1
// 使用示例     void CascadePID_speed(int16 speed, 0);
// 备注信息     位置式PI, 20ms运算一次
//-------------------------------------------------------------------------------------------------------------------
// 速度环PID函数（无刷电机控制）
float CascadePID_speed_V(int16 speed, int16 speed_qiwang)
{
    float PWM;
    float a=0;
    float b=0;
    float c=0;

//    speed_pid.kp = data[4];       // 动态调整kp（已注释）
//    speed_pid.ki = data[5];       // 动态调整ki（已注释）
//    speed_pid.kd = data[6];       // 动态调整kd（已注释）

    // 计算速度误差
    speed_pid_v.error = speed_qiwang - speed;


    ///////////////////////////////////////////////////////////////////////////////////////////////////位置式PID（已注释）
//    speed_pid_v.error_Integral += speed_pid_v.error;
//    if( speed_pid_v.error_Integral >= 500)
//        speed_pid_v.error_Integral = 500;
//    else if( speed_pid_v.error_Integral <= -500)
//        speed_pid_v.error_Integral = -500;
//
//    PWM = speed_pid_v.error * speed_pid_v.kp + speed_pid_v.error_Integral * speed_pid_v.ki + ( speed_pid_v.error -  speed_pid_v.lasterror) * speed_pid_v.kd;

    ///////////////////////////////////////////////////////////////////////////////////////////////////增量式PID
    a+=speed_pid_v.kp*(speed_pid_v.error-speed_pid_v.lasterror);
    b+=speed_pid_v.ki*speed_pid_v.error;
    c+=speed_pid_v.kd*(speed_pid_v.error-2*speed_pid_v.lasterror+speed_pid_v.prelasterror);

//    if(b>2500)  // 积分限幅（已注释）
//        b=2500;
//    if(b<-2500)
//        b=-2500;

    PWM = a+b+c;

    // 更新误差
    speed_pid_v.prelasterror=speed_pid_v.lasterror;
    speed_pid_v.lasterror=speed_pid_v.error;

    // PWM限幅
    if (PWM > 6500)
        PWM = 6500;
    else if (PWM < -6500)
        PWM = -6500;

    return PWM;
}




//-------------------------------------------------------------------------------------------------------------------
// 函数简介     转向内环（陀螺仪）PID控制
// 参数说明     Gyro实际角速度，Gyro_qiwang期望角速度
// 返回参数     PWM输出
// 使用示例     CascadePID_Turn_Gyro(Gyro, Gyro_qiwang);
// 备注信息     位置式PD控制
//-------------------------------------------------------------------------------------------------------------------
float CascadePID_Turn_Gyro(float Gyro, float Gyro_qiwang)
{
    float PWM;

//    gyro_turn_pid.kp = data[0];       // 动态调整kp（已注释）
////    gyro_turn_pid.ki = data[1];       // 动态调整ki（已注释）
//    gyro_turn_pid.kd = data[1];       // 动态调整kd（已注释）
//
//    if(my_abs(Gyro)<10)  // 陀螺仪死区（已注释）
//        Gyro=0;

    // 计算误差
    gyro_turn_pid.error = Gyro_qiwang - Gyro;

    // 计算PD输出
    PWM = gyro_turn_pid.error * gyro_turn_pid.kp + ( gyro_turn_pid.error -  gyro_turn_pid.lasterror) * gyro_turn_pid.kd;
    // 更新上一次误差
    gyro_turn_pid.lasterror = gyro_turn_pid.error;

    // 根据单边桥标志进行PWM限幅
    if (danbianqiao_flag)
    {
        if(PWM > 7000)
            PWM = 7000;
        else if(PWM < -7000)
            PWM = -7000;
    }
    else
    {
        if(PWM > 5000)
            PWM = 5000;
        else if(PWM < -5000)
            PWM = -5000;
    }
    return PWM;



//    // 增量式PID（已注释）
//    PWM = gyro_turn_pid.kp * (gyro_turn_pid.error - gyro_turn_pid.lasterror) + gyro_turn_pid.ki * gyro_turn_pid.error + gyro_turn_pid.kd * ( gyro_turn_pid.error - 2 * gyro_turn_pid.lasterror + gyro_turn_pid.prelasterror);
//
//    gyro_turn_pid.prelasterror = gyro_turn_pid.lasterror;
//    gyro_turn_pid.lasterror = gyro_turn_pid.error;
//
//    if(PWM > 500)
//        PWM = 500;
//    else if(PWM < -500)
//        PWM = -500;
//
//    return PWM;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     转向外环（角度）PID控制
// 参数说明     angle实际角度
// 返回参数     PWM输出
// 使用示例     CascadePID_Turn_Angle(angle);
// 备注信息     位置式PD控制，根据不同场景动态调整kp和kd
//-------------------------------------------------------------------------------------------------------------------
float CascadePID_Turn_Angle(float angle)
{
    float PWM;
    double angle_kp,angle_kd;
    double e;
    float k;

//    angle_turn_pid.kp = data[2];       // 动态调整kp（已注释）
//    angle_turn_pid.kd = data[3];       // 动态调整kd（已注释）

//    if (danbianqiao_angle_error!=-999 && danbianqiao_flag)  // 单边桥时角度误差调整（已注释）
//        angle_turn_pid.error = danbianqiao_angle_error;
//    else
    angle_turn_pid.error = angle;

//    e=exp(-my_abs(angle_turn_pid.error/5));  // 根据误差动态调整kp（已注释）
//    angle_kp = angle_turn_pid.kp * (my_abs( (e - 1) / (e + 1) ) / 2 + 0.5);

//    if (speed_qiwang_now>=800)  // 根据速度动态调整kp和kd（已注释）
//    {
//        e=exp(-my_abs(angle_turn_pid.error/5));
//        k = (my_abs( (e - 1) / (e + 1) ) * 0.2 + 0.8);
//    }
//
//    else
//    {
//        e=exp(-my_abs(angle_turn_pid.error/10));
//        k = (my_abs( (e - 1) / (e + 1) ) * 0.5 + 0.5);
//    }

//    angle_kp = angle_turn_pid.kp ;//* k
//    angle_kd = angle_turn_pid.kd ;

//    e=exp(-my_abs(angle_turn_pid.error/6));  // 根据误差动态调整kp和kd（已注释）
//    k = (my_abs( (e - 1) / (e + 1) ) * 0.5 + 0.5);

//    angle_kp = angle_turn_pid.kp * k;
//    angle_kd = angle_turn_pid.kd * k;
    angle_kp = angle_turn_pid.kp ;//* k
    angle_kd = angle_turn_pid.kd ;

    // 根据不同场景调整kp和kd
    if (yuan_flag && yuan_flag!=6)
    {
        angle_kp = angle_kp * yuan_kp;
        angle_kd = angle_kd * yuan_kp;
    }

    else if (cross_flag==1 || cross_flag==2 || cross_flag==5)    //进出十字kp
        angle_kp = angle_kp * cross_kp;

    else if (cross_flag==3 || cross_flag==4)    //十字内kp
        angle_kp = angle_kp * cross_inside_kp;

//    else if ((cross_flag==1 || cross_flag==4) && draw_type!=0)  // 绘图时十字kp（已注释）
//        angle_kp = angle_kp * cross_kp;

    else if (danbianqiao_flag && danbianqiao_flag!=99)
        angle_kp=angle_kp*danbianqiao_kp;
//
    else if (jump_flag==2)//jump_flag==1 ||
        angle_kp=angle_kp*jump_kp;

    // 计算PD输出
    PWM = angle_turn_pid.error * angle_kp + ( angle_turn_pid.error -  angle_turn_pid.lasterror) * angle_kd;
    // 更新上一次误差
    angle_turn_pid.lasterror = angle_turn_pid.error;

    // PWM限幅
    if(PWM > 8000)
        PWM = 8000;
    else if(PWM < -8000)
        PWM = -8000;

    return PWM;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     横滚角PID控制
// 参数说明     angle实际角度，angle_qiwang期望角度
// 返回参数     PWM输出
// 使用示例     CascadePID_Rolling_Angle(angle, angle_qiwang);
// 备注信息     增量式PID控制
//-------------------------------------------------------------------------------------------------------------------
float Rolling_PWM=0;
float pwm_a=0, pwm_b=0, pwm_c=0;
float CascadePID_Rolling_Angle(float angle, float angle_qiwang)
{
    float du, ki;

//    rolling_angle_pid.kp = data[4];       // 动态调整kp（已注释）
//    rolling_angle_pid.ki = data[5];       // 动态调整ki（已注释）
//    rolling_angle_pid.kd = data[6];       // 动态调整kd（已注释）

    // 根据单边桥标志调整ki
    if(danbianqiao_flag && danbianqiao_flag!=99)
        ki=rolling_angle_pid.ki;
    else
        ki=0.5*rolling_angle_pid.ki;

    // 计算误差
    rolling_angle_pid.error = angle_qiwang-angle;
//    rolling_angle_pid.error = rolling_angle_pid.error*0.8 + rolling_angle_pid.lasterror*0.2;  // 误差滤波（已注释）

//    if ( my_abs(rolling_angle_pid.error) < 3) rolling_angle_pid.error=0;  // 误差死区（已注释）

//    rolling_angle_pid.error_Integral += rolling_angle_pid.error;  // 积分项（已注释）
//    if( rolling_angle_pid.error_Integral >= 500)
//        rolling_angle_pid.error_Integral = 500;
//    else if( rolling_angle_pid.error_Integral <= -500)
//        rolling_angle_pid.error_Integral = -500;
//
//
//    PWM = rolling_angle_pid.error * rolling_angle_pid.kp + ( rolling_angle_pid.error -  rolling_angle_pid.lasterror) * rolling_angle_pid.kd + rolling_angle_pid.error_Integral * rolling_angle_pid.ki;

//    if(PWM > 2000)  // PWM限幅（已注释）
//        PWM = 2000;
//    else if(PWM < -2000)
//        PWM = -2000;
//    Rolling_PWM = PWM;

//    return PWM;

    // 计算增量式PID输出
    du = rolling_angle_pid.kp * (rolling_angle_pid.error - rolling_angle_pid.lasterror) + ki * rolling_angle_pid.error + rolling_angle_pid.kd * ( rolling_angle_pid.error - 2 * rolling_angle_pid.lasterror + rolling_angle_pid.prelasterror);

    // 更新误差
    rolling_angle_pid.prelasterror = rolling_angle_pid.lasterror;
    rolling_angle_pid.lasterror = rolling_angle_pid.error;

    // 跳跃时输出为0
    if (jump_flag)
        du=0;

    // PWM限幅
    if(du > 300)
        du = 300;
    else if(du < -300)
        du = -300;

    return du;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     计算转向角度误差
// 参数说明     无
// 返回参数     void
// 使用示例     cal_err_degree();
// 备注信息     基于图像中的道路边界线和中心线信息，考虑了多种特殊场景的处理
//-------------------------------------------------------------------------------------------------------------------
// 误差与速度限制关系表（已注释）
//float error_from_distance[6] = {49, 48 , 47    , 45    , 43    , 40};
//float distance_speed_limit[6] = {0, 1500  , 1800  , 2100  , 2400  , 2700};

// 误差与速度限制关系表（当前使用）
float error_from_distance[10] = {50 , 48    , 47    , 46    , 45    , 43    , 41    , 39    , 37    , 35};
float distance_speed_limit[10] = {0 , 1300  , 1500  , 1700  , 1900  , 2000  , 2100  , 2200  , 2300  , 2400};

// 误差计算相关变量
float err_degree=0;
float err_degree_last=0;
float err_degree_prelast=0;
float delta_error;
int dis;
int float_point;

// 权重系数表
float quan_speed_float[9]={5, 6, 7, 8, 9, 8, 7, 6, 5};
float quan_speed_base[60]={
        0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1
};
float quan_mid[60]={
        0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1
};

// 计算转向角度误差函数
void cal_err_degree()      //***************再修
{
    float total = 0;
    float total_base = 0;
//    for (int i = 5; i>0; i--)  // 根据速度选择误差计算点（已注释）
//    {
//        if ( real_speed_mm >= distance_speed_limit[i] )
//        {
//            float_point = error_from_distance[i];
//            break;
//        }
//    }

//    if (danbianqiao_flag || yuan_flag)      //单边桥或圆环时误差计算点（已注释）
//        float_point=52;
//    else
        float_point=error_dis;//700-750
//        float_point=42;//800

    // 复制权重系数
    memcpy(quan_mid,quan_speed_base,image_h);

//    dis = 40;
//    if (danbianqiao_flag || yuan_flag)      //单边桥或圆环时误差计算点（已注释）
//        float_point=45;
//    else
//    {
//        float_point = (distance-distance_list[59]) / (distance_list[0]-distance_list[59]) *0.3;
//
//        if (real_speed_mm>1500 && real_speed_mm<3000)
//            float_point += (real_speed_mm-1500)/(1500) *0.7;
//        else if (real_speed_mm>=3000)
//            float_point += 0.7;
//
//        float_point=(int)((1-float_point)*image_h);
//    }

//        float_point=(int)( ((0.7*real_speed_mm + 0.3*distance)-distance_list[59]) / (distance_list[0]-distance_list[59]) );

//        for (int i=0; i<image_h; i++)  // 根据距离选择误差计算点（已注释）
//            if (distance_list[i]<real_speed_mm*2)
//            {
//                float_point=(int)(0.7*i + 0.3*mid_white_num)+20;
//                break;
//            }
//            else if(i==image_start_limit_row)
//            {
//                float_point=(int)(0.7*i + 0.3*mid_white_num)+20;
//                break;
//            }
//        float_point=(int)(mid_white_num)+20;

//    if (float_point>image_start_limit_row) float_point=image_start_limit_row;  // 误差计算点限幅（已注释）
//    else if (float_point<20) float_point=20;

    // 设置权重系数
    for (int i = 0; i<11; i++)
        quan_mid[float_point-3+i]=quan_speed_float[i];

    // 计算加权误差
    for (int i=image_start_limit_row; i>hightest; i--)
    {
        if(l_border[i]>=left_stop_point[i]+2 || r_border[i]<=right_stop_point[i]-2)
        {
            total+=center_line[i]*quan_mid[i];
            total_base+=quan_mid[i];
        }
    }
//    if (total_base!=0)  // 计算误差（已注释）
//        err_degree=total/total_base - 47;
////    if (err_degree>47) err_degree=47;
////    else if (err_degree<-47) err_degree=-47;

        // 计算误差并考虑特殊场景
        if (total_base!=0)
        {
            err_degree = (total/total_base - mid_line);

            // 单边桥时误差调整
            if (danbianqiao_flag && danbianqiao_flag!=1 && danbianqiao_flag!=99)
                err_degree = err_degree * angle_lock_trust + danbianqiao_angle_error * (1-angle_lock_trust);

            // 十字路口时误差调整
            else if(cross_mode==1)
            {
                if (cross_flag==2)
                    err_degree = err_degree * 0.3 + (cross_yaw_angle_error) * 0.7;//            err_degree = cross_yaw_angle_error/5;

                else if (cross_flag==5)
                    err_degree = (cross_yaw_angle_error+270*cross_dir); //            err_degree = err_degree * 0.3 + (cross_yaw_angle_error+270*cross_dir) * 0.7;
            }
            else if(cross_mode==2)
                if (cross_flag==2)
                    err_degree = err_degree * 0.3 + (cross_yaw_angle_error) * 0.7;

            // 误差平滑处理
            delta_error=my_abs(err_degree-err_degree_last);
            err_degree = err_degree * 0.6 + err_degree_last * 0.3 + err_degree_prelast * 0.1;

            // 更新误差历史值
            err_degree_prelast=err_degree_last;
            err_degree_last=err_degree;
        }

// 这段代码主要用于计算车辆或机器人的转向角度误差(err_degree)，
// 基于图像中的道路边界线和中心线信息，考虑了多种特殊场景的处理（已注释）
//    if (danbianqiao_flag && danbianqiao_flag!=99)
//        dis=45;
//    else
//        for (int i = 9; i>0; i--)
//        {
//            if ( real_speed_mm >= distance_speed_limit[i] )
//            {
//                dis = error_from_distance[i];
//                break;
//            }
//        }
//
////    dis = 30;
//
////    if (real_speed_mm>2400) dis=35;
////    else if (real_speed_mm>2100) dis=38;
////    else if (real_speed_mm>1800) dis=41;
////    else if (real_speed_mm>1500) dis=44;
////    else if (real_speed_mm>1200) dis=47;
////    else dis=50;
//
//
////    if (real_speed_mm>2200) dis=38;
////    else if (real_speed_mm>2000) dis=40;
////    else if (real_speed_mm>1800) dis=42;
////    else if (real_speed_mm>1500) dis=45;
////    else if (real_speed_mm>1200) dis=47;
////    else dis=50;
//
//
//
//
////    if (danbianqiao_flag || (yuan_flag))      //待测****************************************
////        dis=45;
//
//    if (danbianqiao_flag && danbianqiao_flag!=1 && danbianqiao_flag!=99)
//    {
//        for (int i = 1; i+dis < error_dis; i++)    //取46~55
//        {
//            if(l_border[i+dis]>=left_stop_point[i+dis]+4 || r_border[i+dis]<=right_stop_point[i+dis]-4)//没有出边线的
//            {
//                if( abs(l_border[i+dis]-47) > abs(r_border[i+dis]-47) && r_border[i+dis]-l_border[i+dis]<15)
//                {
//                    total += i * i * (r_border[i+dis] - image_w/2);
//                    total_base += i * i;
//                }
//                else if( abs(l_border[i+dis]-47) < abs(r_border[i+dis]-47)  && r_border[i+dis]-l_border[i+dis]<15)
//                {
//                    total += i * i * (l_border[i+dis] - image_w/2);
//                    total_base += i * i;
//                }
//                else
//                {
//                total += i * i * (center_line[i+dis] - image_w/2);
//                total_base += i * i;
//                }
//
//            }
////            err_degree = (total/total_base * 0.6 + err_degree_last * 0.3 + err_degree_prelast * 0.1) + danbianqiao_angle_error/5;
//        }
//    }
//
////    else if (cross_flag==5 && cross_dir)
////    {
////        if (cross_dir == -1)
////            total=cross_yaw_angle_error-270;
////        else
////            total=cross_yaw_angle_error+270;
////        total_base=5;
////    }
//
//
////    else
////        for (int i = 1; i+dis < error_dis; i++)    //取45~58
////        {
////
////            if (i<=10)
////            {
////                total += i * i * (center_line[i+dis] - image_w/2);
////                total_base += i * i;
////            }
////            else
////            {
////                total += (20-i) * (20-i) * (center_line[i+dis] - image_w/2);
////                total_base += (20-i) * (20-i);
////            }
////        }
//
//
////    else
////        for (int i = 1; i+dis < error_dis; i++)    //取45~58
////        {
////
////            if (i<=10)
////            {
////                total += pow(1.5, i) * (center_line[i+dis] - image_w/2);
////                total_base += pow(1.5, i);
////            }
////            else
////            {
////                total += pow(1.5, (20-i)) * (center_line[i+dis] - image_w/2);
////                total_base += pow(1.5, (20-i));
////            }
////        }
//
////    else
////        for (int i = 1; i+dis < image_start_row; i++)    //取45~58
////        {
//////            if(l_border[i+dis]>=left_stop_point[i+dis]+4 && r_border[i+dis]<=right_stop_point[i+dis]-4)//没有出边线的
//////            {
////    //            total += pow((center_line[i+dis] - image_w/2)/8,2);
////    //            total_base += 1;
////
////                total += pow(1.5, i) * (center_line[i+dis] - image_w/2);
////                total_base += pow(1.5, i);
//////            }
////        }
//
//    else
//        for (int i = 1; i+dis < image_start_row; i++)    //取45~58
//        {
////            if(l_border[i+dis]>=left_stop_point[i+dis]+4 && r_border[i+dis]<=right_stop_point[i+dis]-4)//没有出边线的
////            {
//    //            total += pow((center_line[i+dis] - image_w/2)/8,2);
//    //            total_base += 1;
//
//                total += i * i * (center_line[i+dis] - image_w/2);
//                total_base += i * i;
////            }
//        }
//
//
//    if (total_base!=0)
//    {
//        err_degree = total/total_base * 0.6 + err_degree_last * 0.3 + err_degree_prelast * 0.1;
//        if (danbianqiao_flag && danbianqiao_flag!=1 && danbianqiao_flag!=99)
//            err_degree = err_degree * 0.7 + danbianqiao_angle_error/5 * 0.3;
//        else if (cross_flag==2 && cross_mode==1)
//            err_degree = err_degree * 0.3 + (cross_yaw_angle_error) * 0.7;
////            err_degree = cross_yaw_angle_error/5;
//        else if (cross_flag==5 && cross_mode==1)
////            err_degree = err_degree * 0.3 + (cross_yaw_angle_error+270*cross_dir) * 0.7;
//            err_degree = (cross_yaw_angle_error+270*cross_dir)/5;
//
//    }
//
////    if (total_base!=0)
////        err_degree = total/total_base;
//
////    if (total_base!=0)
////        err_degree = (total/total_base + err_degree * 4)/5;       //效果一般
//
//
////    if (danbianqiao_flag==1 || danbianqiao_flag==2)
////        err_degree=(degree_y-qiao_direction_angle)*0.5;
//
////    if (yuan_flag==1 && yuan_flag==2 && yuan_flag==3 && yuan_flag==4)
////        err_degree=err_degree*0.5;
//
//    err_degree_prelast = err_degree_last;
//    err_degree_last = err_degree;
//
//
//    if (jump_flag==2)
//        err_degree=0;
//
////    else if (danbianqiao_flag)
////        err_degree=err_degree*0.7;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     计算当前速度
// 参数说明     无
// 返回参数     void
// 使用示例     Now_Speed_Filter();
// 备注信息     对编码器返回的速度值进行滤波处理
//-------------------------------------------------------------------------------------------------------------------
// 前瞻比例系数
int qianzhan_ratio[11]={4,5,5,6,7,8,7,6,5,5,4};
//int speed_base[image_h]={0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1
};  //基值：最上方十行不采用，11-70三个采一个，71-120两个采一个

// 速度滤波变量
float avg_speed=0, last_avg_speed=0, prelast_avg_speed=0;
float real_speed_mm=0;

// 当前速度滤波函数
void Now_Speed_Filter()
{
    // 计算平均速度（加权平均）
    avg_speed = (0.5*( motor_value.receive_left_speed_data + motor_value.receive_right_speed_data )+9*avg_speed)/10;

    // 转换为mm/s
    real_speed_mm = avg_speed/0.27;

//    avg_speed = 0.3*( motor_value.receive_left_speed_data + motor_value.receive_right_speed_data ) + 0.3 * last_avg_speed + 0.1 * prelast_avg_speed;  // 三阶滤波（已注释）
//    prelast_avg_speed = last_avg_speed;
//    last_avg_speed = avg_speed;

    //编码器返回值694为1m/s左右
    //distance 实际距离   distance_index 距离对应的像素点（最上是0，最下是119）

//    real_speed_mm = 0.5*( motor_value.receive_left_speed_data + motor_value.receive_right_speed_data ) / 0.27;     //= 编码器返回值 / 一米对应编码器值(694) * 1000 ==> mm/s avg_speed/694.2*1000   400=1.5m/s  330=1.2m/s 200=0.8m/s
}



