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

#if (LAUNCH_STRATEGY_SELECT == 1)
// 【直立发车 / 航向校准】状态机状态定义
typedef enum {
    UPRIGHT_LAUNCH_IDLE = 0,             // 待机状态 (倒地或普通运行)
    UPRIGHT_LAUNCH_WAIT_STANDUP,         // 按下 P10_0 第一次，延时 1s 准备起立
    UPRIGHT_LAUNCH_WAIT_STABILIZE,       // 起立后延时 1s 等待自平衡稳定
    UPRIGHT_LAUNCH_PLAY_BEEP_PREP,       // 等待主循环播放长-短-长提示音
    UPRIGHT_LAUNCH_MANUAL_AIMING,        // 转向环已关闭，手动对准航向中
    UPRIGHT_LAUNCH_WAIT_SAMPLE_DELAY,    // 按下 P10_0 第二次，先延时 0.5s 等待撤手
    UPRIGHT_LAUNCH_SAMPLING_2S,          // 响一声后，记录接下来 2s 的航向均值
    UPRIGHT_LAUNCH_HEADING_LOCKED,       // 2s 采样结束，矢量均值锁定航向，开启转向环，响单声
    UPRIGHT_LAUNCH_WAIT_START_2S         // 按下 P10_4，延时 2s 发车
} UprightLaunchState_e;
#endif

// ==================== 全局变量声明 ====================
extern MenuState_t current_state;
extern uint8_t menu_index;
extern uint8_t menu_values[MENU_STATE_COUNT];
extern const uint8_t menu_max_values[MENU_STATE_COUNT];
extern uint8_t g_is_push_mode; // 推车模式全局标志位（供底层PID控制环使用）
extern uint8_t need_redraw;
#if (LAUNCH_STRATEGY_SELECT == 1)
extern volatile UprightLaunchState_e g_upright_state; // 直立发车状态机当前状态
#endif



// ==================== 函数声明 ====================
void Menu_Init(void);
void Menu_HandleKey(void);
void Menu_ShowStatic(void);
void Menu_ShowDynamic(void);

// Reusable action handlers shared by menu keys and remote commands.
void Menu_TriggerRecordAction(void);
void Menu_TriggerStartAction(void);

#endif /* _MENU_H_ */
