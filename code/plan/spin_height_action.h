#ifndef __SPIN_HEIGHT_ACTION_H__
#define __SPIN_HEIGHT_ACTION_H__

#include "zf_common_headfile.h"

// ============================================================
// 组合动作：原地自转两圈 + 伸腿收腿伸腿收腿
// 复用 minefield 自转 + bridge 高度控制
// ============================================================

void SpinHeightAction_Init(void);
void SpinHeightAction_Trigger(void);    // sbus CH3 跳变时调用
void SpinHeightAction_Update(void);     // ISR 20ms 周期调用
uint8_t SpinHeightAction_IsActive(void); // 查询是否在执行中

#endif // __SPIN_HEIGHT_ACTION_H__
