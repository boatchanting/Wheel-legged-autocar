#ifndef _NAV_REPLAY_ROUTE_TABLE_H_
#define _NAV_REPLAY_ROUTE_TABLE_H_

#include "nav_ram.h"

// 由 tools/webview_nav_marker速度规划_科目二/csv_to_nav_table.py 自动生成
// 源 CSV：D:/autocar/project3/project3/project/tools/webview_nav_marker速度规划_科目二/nav_mark_points_20260725_103854.csv
// 生成时间：2026-07-25 10:39:12

#define NAV_REPLAY_START_HEADING_VALID 0
#define NAV_REPLAY_START_HEADING_DEG 102.093f

#define NAV_REPLAY_STATIC_ROUTE_COUNT 7

static const NavRamPoint_t nav_replay_static_route_points[7] = {
    {-6157.630f, -2400.899f, 23.986f, 140.901f, (uint8)1, 0.000f},
    {-9010.166f, 1548.725f, -66.847f, 152.547f, (uint8)1, 0.000f},
    {-13279.622f, -1906.268f, 52.104f, 131.388f, (uint8)1, 0.000f},
    {-13813.029f, -4210.723f, 80.275f, 115.941f, (uint8)1, 0.000f},
    {-15472.892f, -4835.228f, 13.565f, 164.491f, (uint8)0, 0.000f},
    {-16274.644f, -3431.491f, -75.186f, 150.369f, (uint8)0, 0.000f},
    {-8777.436f, 158.597f, -156.977f, 119.221f, (uint8)0, 0.000f},
};

#endif // _NAV_REPLAY_ROUTE_TABLE_H_
