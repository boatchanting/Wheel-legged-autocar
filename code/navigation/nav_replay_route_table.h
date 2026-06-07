#ifndef _NAV_REPLAY_ROUTE_TABLE_H_
#define _NAV_REPLAY_ROUTE_TABLE_H_

#include "nav_ram.h"

// 由 tools/webview_nav_marker/csv_to_nav_table.py 自动生成
// 源 CSV：D:/autocar/project3/project3/project/tools/webview_nav_marker速度规划/nav_mark_points_20260607_215546.csv
// 生成时间：2026-06-07 21:55:55

#define NAV_REPLAY_START_HEADING_VALID 0
#define NAV_REPLAY_START_HEADING_DEG 194.378f

#define NAV_REPLAY_STATIC_ROUTE_COUNT 5

static const NavRamPoint_t nav_replay_static_route_points[5] = {
    {-1100.625f, -13.771f, 0.976f, 174.700f, 0.000f, (uint8)0},
    {-312.198f, 713.606f, -133.840f, 137.269f, 0.000f, (uint8)0},
    {-497.166f, -288.706f, 68.744f, 145.191f, 0.000f, (uint8)0},
    {-976.869f, 654.575f, -61.480f, 167.408f, 0.000f, (uint8)0},
    {91.934f, -52.743f, 162.348f, 121.251f, 0.000f, (uint8)0},
};

#endif // _NAV_REPLAY_ROUTE_TABLE_H_
