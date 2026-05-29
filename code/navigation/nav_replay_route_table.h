#ifndef _NAV_REPLAY_ROUTE_TABLE_H_
#define _NAV_REPLAY_ROUTE_TABLE_H_

#include "nav_ram.h"

// 由 tools/webview_nav_marker速度规划_科目二/csv_to_nav_table.py 自动生成
// 源 CSV：D:/New_luntui/project3/project/tools/webview_nav_marker速度规划_科目二/nav_mark_points_20260528_121010.csv
// 生成时间：2026-05-28 14:08:11

#define NAV_REPLAY_START_HEADING_VALID 0
#define NAV_REPLAY_START_HEADING_DEG 31.520f

#define NAV_REPLAY_STATIC_ROUTE_COUNT 4

static const NavRamPoint_t nav_replay_static_route_points[4] = {
    {-4040.032f, 191.214f, 0.836f, 32.457f, (uint8)1, 0.000f},
    {-7620.131f, -2853.983f, -25.939f, 30.989f, (uint8)1, 0.000f},
    {-12341.343f, 496.202f, 19.841f, 63.485f, (uint8)1, 0.000f},
    {-3003.468f, -131.531f, 20.244f, 59.036f, (uint8)0, 0.000f},
};

#endif // _NAV_REPLAY_ROUTE_TABLE_H_
