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
#define PLAN2_HYBRID_TERMINAL             5 // 方案5：远距离纯追踪/LOS + 近距离点对点终端制导
#define PLAN2_METHOD_PURE_PURSUIT         PLAN2_PURE_PURSUIT
// #define PLAN2_xxx       n // 预留：未来可能的其他策略

// 科目二方案
#ifndef NAV_PLAN2_METHOD
#define NAV_PLAN2_METHOD PLAN2_METHOD_PRECISE
#endif

// 科目二方案5（PLAN2_HYBRID_TERMINAL）的远距离引导模式
// 说明：
// 1. 这个开关只在 NAV_PLAN2_METHOD == PLAN2_HYBRID_TERMINAL 时生效。
// 2. 远距离阶段负责沿路表靠近雷区；进入终端距离后，方案5会统一切回点对点进雷区中心。
// 3. 纯追踪容错更强，适合打点间距不均匀、路线有圆角的情况；LOS更贴线，适合线段较直、希望转向更平顺的情况。
#define PLAN2_HYBRID_GUIDE_PURE_PURSUIT   1 // 纯追踪：按前瞻点给转向误差，默认推荐先用这个调车
#define PLAN2_HYBRID_GUIDE_LOS            2 // LOS：按当前线段视线点给转向误差，适合路线线段特征明显时试

#ifndef NAV_PLAN2_HYBRID_GUIDE_MODE
#define NAV_PLAN2_HYBRID_GUIDE_MODE PLAN2_HYBRID_GUIDE_LOS  // 方案5远距离默认使用LOS
#endif

// 科目二特殊点/雷区点触发模式
// 说明：
// 1. 这个开关用于方案3的特殊点触发判断，主要影响雷区中心点是否触发旋转动作。
// 2. CENTER_TRIGGER：严格中心触发，只有进入 NAV_SPECIAL_TRIGGER_RADIUS 才触发，定位要求更高但不容易误触发。
// 3. CENTER_RELAXED：宽松中心触发，接近中心且预测会进入触发半径，或已经越过最近点时也允许触发，适合速度较快或惯导误差稍大的情况。
#define PLAN2_SPECIAL_APPROACH_CENTER_TRIGGER 1 // 严格触发：到雷区中心触发半径内才触发
#define PLAN2_SPECIAL_APPROACH_CENTER_RELAXED 2 // 宽松触发：允许预测触发/越过最近点触发，降低错过雷区点概率

#ifndef NAV_PLAN2_SPECIAL_APPROACH_MODE
#define NAV_PLAN2_SPECIAL_APPROACH_MODE PLAN2_SPECIAL_APPROACH_CENTER_TRIGGER
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
