20260128晚上 学习板小车1 使用pid参数站在原地并锁定角度，但是有轻微超调的倾向，站在蓝色的车队的地面下
// 参数 0: 角速度环 Kp (pid_gyro.kp)
case 0: pid_gyro.kp  = seekfree_assistant_parameter[i]; break;
-18
// 参数 1: 角度环Kp
case 1: pid_angle.kp  = seekfree_assistant_parameter[i]; break;
26
// 参数 2: 角度环kd
case 2: pid_angle.kd  = seekfree_assistant_parameter[i]; break;
5.2
// 参数 3: 舵机速度控制环kp
case 3: pid_servo_speed.kp = seekfree_assistant_parameter[i]; break;
-6.5
// 参数 4: 舵机速度控制环ki
case 4: pid_servo_speed.ki = seekfree_assistant_parameter[i]; break;
-0.03
// 参数5: 转向角速度环kp
case 5: pid_turn_gyro.kp = seekfree_assistant_parameter[i]; break;
-12
// 参数6: 转向角度环kp
case 6: pid_turn_angle.kp = seekfree_assistant_parameter[i]; break;
-3