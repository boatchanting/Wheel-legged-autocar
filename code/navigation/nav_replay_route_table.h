#ifndef _NAV_REPLAY_ROUTE_TABLE_H_
#define _NAV_REPLAY_ROUTE_TABLE_H_

#include "nav_ram.h"

// 由 tools/webview_nav_marker速度规划_科目二/csv_to_nav_table.py 自动生成
// 源 CSV：D:/New_luntui/project3/project/tools/webview_nav_marker速度规划_科目二/nav_mark_points_20260801_145113.csv
// 生成时间：2026-08-01 20:47:20

#define NAV_REPLAY_START_HEADING_VALID 0
#define NAV_REPLAY_START_HEADING_DEG 51.075f

#define NAV_REPLAY_STATIC_ROUTE_COUNT 7

static const NavRamPoint_t nav_replay_static_route_points[7] = {
    {-4740.621f, -0.236f, 0.143f, 91.397f, (uint8)1, 0.000f},
    {-5186.450f, -4561.109f, 86.102f, 113.702f, (uint8)1, 0.000f},
    {-8056.352f, -2321.229f, -34.089f, 87.237f, (uint8)1, 0.000f},
    {-12105.081f, -2508.037f, 1.866f, 91.987f, (uint8)1, 0.000f},
    {-21228.918f, -2785.657f, 1.666f, 91.883f, (uint8)6, 0.000f},
    {-23173.377f, -1810.672f, -75.177f, 149.789f, (uint8)6, 0.000f},
    {-22605.998f, 34.897f, 179.363f, 132.584f, (uint8)6, 0.000f},
};

#endif // _NAV_REPLAY_ROUTE_TABLE_H_
