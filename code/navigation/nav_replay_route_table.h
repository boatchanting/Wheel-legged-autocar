#ifndef _NAV_REPLAY_ROUTE_TABLE_H_
#define _NAV_REPLAY_ROUTE_TABLE_H_

#include "nav_ram.h"

// 由 tools/webview_nav_marker速度规划_科目二/csv_to_nav_table.py 自动生成
// 源 CSV：D:/autocar/project3/project3/project/tools/webview_nav_marker速度规划_科目二/nav_mark_points_20260721_154110.csv
// 生成时间：2026-07-21 15:41:40

#define NAV_REPLAY_START_HEADING_VALID 0
#define NAV_REPLAY_START_HEADING_DEG 92.726f

#define NAV_REPLAY_STATIC_ROUTE_COUNT 4

static const NavRamPoint_t nav_replay_static_route_points[4] = {
    {-4958.976f, -3423.630f, 44.798f, 136.842f, (uint8)1, 0.000f},
    {-8305.968f, 1704.842f, -77.569f, 119.475f, (uint8)1, 0.000f},
    {-14961.826f, -2856.744f, 48.386f, 150.628f, (uint8)1, 0.000f},
    {126.760f, -4058.874f, 178.301f, 112.370f, (uint8)0, 0.000f},
};

#endif // _NAV_REPLAY_ROUTE_TABLE_H_
