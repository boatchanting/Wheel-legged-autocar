#ifndef _NAV_OPTIONS_H_
#define _NAV_OPTIONS_H_

#include "../config/sys_options.h"

// ========================================================
// [ 科目一 (Plan 1) 算法方案定义 ]
// ========================================================
#define PLAN1_PURE_PURSUIT_SPEED_PLANNING 1 // 方案1：惯导纯追踪带速度规划算法方案
#define PLAN1_PURE_PURSUIT      2 // 方案2:惯导纯追踪算法方案
#define PLAN1_METHOD_GNSS         3 // 方案3：GNSS纯追踪算法方案

// 科目一方案
#ifndef NAV_PLAN1_METHOD
#define NAV_PLAN1_METHOD  PLAN1_PURE_PURSUIT_SPEED_PLANNING
#endif

// ========================================================
// [ 科目二 (Plan 2) 算法方案定义 ]
// ========================================================
#define PLAN2_PURE_PURSUIT_SPEED_PLANNING 1 // 方案1：纯追踪 + 离线路表速度规划
#define PLAN2_PURE_PURSUIT                2 // 方案2：纯追踪基础方案
#define PLAN2_METHOD_PRECISE              3 // 方案3：点对点 + 离线路表速度规划
#define PLAN2_POINT_SPEED_PLANNING        4 // 方案4：点对点 + 在线速度规划

// 科目二方案
#ifndef NAV_PLAN2_METHOD
#define NAV_PLAN2_METHOD PLAN2_POINT_SPEED_PLANNING
#endif

// 科目二特殊点/雷区点触发模式（方案3/4通用）
// 说明：
// 1. 方案3：宽松模式会引入预测触发/越过最近点触发逻辑。
// 2. 方案4：最终动作仍要求进入中心停车区，但宽松模式会更早压到慢速。
// 3. CENTER_TRIGGER：严格中心策略，更依赖定位精度，但不容易提前介入。
// 4. CENTER_RELAXED：宽松中心策略，更适合速度较快或惯导误差稍大的情况。
#define PLAN2_SPECIAL_APPROACH_CENTER_TRIGGER 1 // 严格触发：到雷区中心触发半径内才触发
#define PLAN2_SPECIAL_APPROACH_CENTER_RELAXED 2 // 宽松触发：允许预测触发/越过最近点触发，降低错过雷区点概率

#ifndef NAV_PLAN2_SPECIAL_APPROACH_MODE
#define NAV_PLAN2_SPECIAL_APPROACH_MODE PLAN2_SPECIAL_APPROACH_CENTER_TRIGGER  // 方案3/4默认使用严格中心触发
#endif

// 科目二特殊点宽松模式共享参数
// 说明：
// 1. 方案3在宽松模式下用它做“是否即将进入中心区”的预测窗口。
// 2. 方案4在宽松模式下用它做“提前压慢速”的逼近窗口。
#ifndef NAV_PLAN2_SPECIAL_RELAX_APPROACH_WINDOW_MM
#define NAV_PLAN2_SPECIAL_RELAX_APPROACH_WINDOW_MM 420.0f
#endif

#ifndef NAV_PLAN2_SPECIAL_STOP_PREDICT_TIME_S
#define NAV_PLAN2_SPECIAL_STOP_PREDICT_TIME_S 0.35f
#endif

// ========================================================
// [ 科目三 (Plan 3) 算法方案定义 ]
// ========================================================
#define PLAN3_METHOD_PRECISE      1 // 方案A：点到点慢速精准对齐方案
// #define PLAN3_xxx       n // 预留：未来可能的其他策略

// 科目三方案
#ifndef NAV_PLAN3_METHOD
#define NAV_PLAN3_METHOD PLAN3_METHOD_PRECISE
#endif

#endif // _NAV_OPTIONS_H_
