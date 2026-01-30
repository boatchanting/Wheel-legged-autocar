#ifndef _SBUS_H_
#define _SBUS_H_

#include "zf_common_headfile.h" // 包含基础类型定义



// 模式枚举 (对应 CH4 三态开关)
typedef enum {
    MODE_LOW    = 0,  // 开关拨在最下
    MODE_MIDDLE = 1,  // 开关拨在中间 
    MODE_HIGH   = 2   // 开关拨在最上 
} robot_mode_e;
// 遥控器解析后的控制数据
typedef struct {
    float target_angle;     // [输出] 期望转向角度 (增量积分后)
    float target_speed;     // [输出] 期望电机速度 (增量积分后)
    
    uint8 mark_trigger;     // [标志] CH3 打点触发 (按下瞬间置1，需手动清零)
    uint8 motor_enable;     // [状态] CH6 总开关 (1=使能, 0=急停)
    
    robot_mode_e mode;      // [状态] CH4 模式选择
} robot_ctrl_t;

// ==========================================
// 全局变量声明 (extern)
// ==========================================
// 在其他文件包含此头文件后，可直接访问 robot_ctrl 变量
extern robot_ctrl_t robot_ctrl;

// ==========================================
// 函数声明
// ==========================================

// 初始化遥控器逻辑变量
void Remote_Control_Init(void);

// 遥控器数据处理任务 (建议每 10ms 或 20ms 调用一次)
void Remote_Control_Process(void);

#endif