/*
 * PID.h
 *
 *  Created on: 2024年3月2日
 *      Author: 31817
 */
#ifndef CODE_PID_H_
#define CODE_PID_H_

#include "zf_common_headfile.h"

typedef struct{
               float error;
               float lasterror;
               float prelasterror;
               float error_Integral;
               float kp;
               float ki;
               float kd;
                }PID_param;

float CascadePID_Gyro(int16 gyro, float PWM2);
float CascadePID_angle(float angle, float angle_zero, float PWM1);
float CascadePID_speed_S(int16 speed, int16 speed_qiwang);
float CascadePID_speed_V(int16 speed, int16 speed_qiwang);

extern float test_data;
extern PID_param speed_pid_s;
extern PID_param speed_pid_v;
extern PID_param angle_pid;
extern PID_param gyro_pid;

extern float speed_ratio, speed_ratio_last, speed_ratio_prelast;;

void cal_err_degree();
extern float err_degree;
float CascadePID_Turn_Gyro(float Gyro, float Gyro_qiwang);
float CascadePID_Turn_Angle(float angle);
//float CascadePID_Turn_Angle(float angle, float angle_speed);
extern double angle_kp, e;
extern float danbianqiao_angle_error;

extern float avg_speed, real_speed_mm;
void Now_Speed_Filter();

float CascadePID_Rolling_Angle(float angle, float angle_qiwang);

extern int float_point;
extern float delta_error;
#endif /* CODE_PID_H_ */




