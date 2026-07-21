#ifndef _NAV_REPLAY_ROUTE_TABLE_H_
#define _NAV_REPLAY_ROUTE_TABLE_H_

#include "nav_ram.h"

// 由 tools/webview_nav_marker速度规划_科目二/csv_to_nav_table.py 自动生成
// 源 CSV：D:/New_luntui/project3/project 3/tools/webview_nav_marker速度规划_科目二/nav_mark_points_20260720_221941.csv
// 生成时间：2026-07-20 22:19:56

#define NAV_REPLAY_START_HEADING_VALID 0
#define NAV_REPLAY_START_HEADING_DEG 107.359f

#define NAV_REPLAY_STATIC_ROUTE_COUNT 4

static const NavRamPoint_t nav_replay_static_route_points[4] = {
    {-5266.332f, -3730.951f, 54.417f, 156.416f, (uint8)1, 0.000f},
    {-8648.837f, 1871.762f, -69.883f, 125.325f, (uint8)1, 0.000f},
    {-15519.892f, -3197.369f, 63.324f, 151.535f, (uint8)1, 0.000f},
    {340.146f, -4195.585f, -178.373f, 118.749f, (uint8)0, 0.000f},
};

#endif // _NAV_REPLAY_ROUTE_TABLE_H_
