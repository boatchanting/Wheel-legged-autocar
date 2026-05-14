#ifndef _SBUS_H_
#define _SBUS_H_

#include "zf_common_headfile.h" // 包含基础类型定义




// 遥控器解析后的控制数据
typedef struct {
    float target_angle;     // [输出] 期望转向角度 (增量积分后)
    float target_speed;     // [输出] 期望电机速度 (增量积分后)
    
    uint8 mark_trigger;     // [标志] CH3 打点触发 (按下瞬间置1，需手动清零)
    uint8 motor_enable;     // [状态] CH6 总开关 (1=使能, 0=急停)
    uint8 brake_active;     // [状态] CH5 刹车开关 (1=按下, 0=松开)
    uint8 reverse_brake_active; // [状态] 速度杆持续反向减速触发的重刹标志，摇杆回中解除
    
    uint8 point_type;      // 导航的打点类型
    // 模式枚举 (对应 CH4 三态开关和CH5开关的组合状态，使用ch3开关进行触发)
    // NAV_POINT_PATH = 0,     // 普通路径点
    // NAV_POINT_CIRCLE = 1,   // 转圈点
    // NAV_POINT_SLOPE = 2,    // 上坡点
    // NAV_POINT_JUMP = 3,     // 跳跃点
    // NAV_POINT_BRIDGE = 4,   // 单边桥点
    // NAV_POINT_BUMP = 5      // 颠簸路段点
} robot_ctrl_t;

// ==========================================
// 全局变量声明 (extern)
// ==========================================
// 在其他文件包含此头文件后，可直接访问 robot_ctrl 变量
extern robot_ctrl_t robot_ctrl;
extern volatile uint8 g_brake_active;
extern volatile uint8 g_reverse_brake_active;

// ==========================================
// 函数声明
// ==========================================

// 初始化遥控器逻辑变量
void Remote_Control_Init(void);

// 遥控器数据处理任务 (建议每 10ms 或 20ms 调用一次)
void Remote_Control_Process(void);

#endif
