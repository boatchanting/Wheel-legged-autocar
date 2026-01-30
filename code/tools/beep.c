#include "zf_common_headfile.h"

// 内部变量
 //uint16 beep_counter = 0;       // 计时器
//uint16 beep_interval = 100;    // 默认间隔 100ms
//uint8  beep_times_left = 0;    // 剩余响声次数
//uint8  beep_state = 0;         // 0:静音, 1:正在响, 2:响声间的间隔

// 初始化
void beep_init(void)
{
    // 初始化引脚，推挽输出，默认低电平（不响）
    gpio_init(BUZZER_PIN, GPO, GPIO_LOW, GPO_PUSH_PULL);
}
    // if(count < 10)
    // {
    //     gpio_toggle_level(BUZZER_PIN); // 翻转电平
    // }
    // // 阶段二：静音阶段 (1秒 ~ 2秒)
    // else if(count < 20)
    // {
    //     gpio_set_level(BUZZER_PIN, GPIO_LOW); // 强制拉低（静音）
    // }
