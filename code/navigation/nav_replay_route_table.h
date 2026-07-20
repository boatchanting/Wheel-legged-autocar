#ifndef _NAV_REPLAY_ROUTE_TABLE_H_
#define _NAV_REPLAY_ROUTE_TABLE_H_

#include "nav_ram.h"

// 由 tools/webview_nav_marker速度规划_科目二/csv_to_nav_table.py 自动生成
// 源 CSV：D:/New_luntui/project3/project 3/tools/webview_nav_marker速度规划_科目二/nav_mark_points_20260718_095531.csv
// 生成时间：2026-07-19 07:48:55

#define NAV_REPLAY_START_HEADING_VALID 0
#define NAV_REPLAY_START_HEADING_DEG 142.434f

#define NAV_REPLAY_STATIC_ROUTE_COUNT 5

static const NavRamPoint_t nav_replay_static_route_points[5] = {
    {-6836.056f, 5233.726f, -25.783f, 173.244f, (uint8)1, 0.000f},
    {-11242.704f, -1758.011f, 60.861f, 104.988f, (uint8)1, 0.000f},
    {-18286.234f, 5858.082f, -79.039f, 156.685f, (uint8)1, 0.000f},
    {-24131.344f, 1142.814f, 55.202f, 116.265f, (uint8)1, 0.000f},
    {518.056f, 68.373f, -175.001f, 138.615f, (uint8)0, 0.000f},
};

#endif // _NAV_REPLAY_ROUTE_TABLE_H_
