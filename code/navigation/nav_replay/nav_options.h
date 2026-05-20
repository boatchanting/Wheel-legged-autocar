#ifndef _NAV_OPTIONS_H_
#define _NAV_OPTIONS_H_

#include "../config/sys_options.h"

// ========================================================
// [ 科目一 (Plan 1) 算法方案定义 ]
// ========================================================
#define PLAN1_PURE_PURSUIT_SPEED_PLANNING 1 // 方案A：惯导纯追踪算法方案
#define PLAN1_METHOD_GNSS         2 // 方案B：GNSS纯追踪算法方案

// 科目一方案
#ifndef NAV_PLAN1_METHOD
#define NAV_PLAN1_METHOD PLAN1_PURE_PURSUIT_SPEED_PLANNING
#endif

// ========================================================
// [ 科目二 (Plan 2) 算法方案定义 ]
// ========================================================
#define PLAN2_METHOD_PURE_PURSUIT 1 // 方案A：带雷区状态机支持的纯追踪方案
// #define PLAN2_xxx       n // 预留：未来可能的其他策略

// 科目二方案
#ifndef NAV_PLAN2_METHOD
#define NAV_PLAN2_METHOD PLAN2_METHOD_PURE_PURSUIT
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