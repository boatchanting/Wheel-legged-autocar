#ifndef _MENU_H_
#define _MENU_H_

#include "zf_common_headfile.h"

// ==================== 类型定义 ====================
typedef enum {
    MENU_STATE_MAIN = 0,          // 主界面
    MENU_STATE_SUBJECT,           // 选择科目
    MENU_STATE_CALIBRATION,       // 选择标定方式
    MENU_STATE_ACTION_SELECT,     // 选择操作 (Record/Start)
    MENU_STATE_ACTION_CONFIRM,    // 确认操作（显示未发车）
    MENU_STATE_ACTION_RUNNING,    // 运行中（显示动态数据和发车成功）
    MENU_STATE_ACTION_COMPLETE,   // 完成（显示结果）
    MENU_STATE_COUNT
} MenuState_t;

// ==================== 全局变量声明 ====================
extern MenuState_t current_state;
extern uint8_t menu_index;
extern uint8_t menu_values[MENU_STATE_COUNT];
extern const uint8_t menu_max_values[MENU_STATE_COUNT];
extern uint8_t g_is_push_mode; // 推车模式全局标志位（供底层PID控制环使用）
extern uint8_t need_redraw;



// ==================== 函数声明 ====================
void Menu_Init(void);
void Menu_HandleKey(void);
void Menu_ShowStatic(void);
void Menu_ShowDynamic(void);

#endif /* _MENU_H_ */