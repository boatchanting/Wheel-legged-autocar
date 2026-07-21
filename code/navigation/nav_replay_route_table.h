#ifndef _NAV_REPLAY_ROUTE_TABLE_H_
#define _NAV_REPLAY_ROUTE_TABLE_H_

#include "nav_ram.h"

// 由 tools/webview_nav_marker速度规划_科目二/csv_to_nav_table.py 自动生成
// 源 CSV：D:/autocar/project3/project3/project/tools/webview_nav_marker速度规划_科目二/nav_mark_points_20260721_201533.csv
// 生成时间：2026-07-21 20:15:49

#define NAV_REPLAY_START_HEADING_VALID 0
#define NAV_REPLAY_START_HEADING_DEG 107.781f

#define NAV_REPLAY_STATIC_ROUTE_COUNT 4

static const NavRamPoint_t nav_replay_static_route_points[4] = {
    {-5085.567f, -3501.524f, 43.685f, 148.857f, (uint8)1, 0.000f},
    {-8371.516f, 1720.564f, -66.992f, 119.940f, (uint8)1, 0.000f},
    {-14879.330f, -2810.283f, 32.806f, 147.086f, (uint8)1, 0.000f},
    {151.752f, -3687.605f, 178.172f, 120.313f, (uint8)0, 0.000f},
};

#endif // _NAV_REPLAY_ROUTE_TABLE_H_
