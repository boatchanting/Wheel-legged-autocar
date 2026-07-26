#ifndef _NAV_REPLAY_ROUTE_TABLE_H_
#define _NAV_REPLAY_ROUTE_TABLE_H_

#include "nav_ram.h"

// 由 tools/webview_nav_marker速度规划_科目二/csv_to_nav_table.py 自动生成
// 源 CSV：D:/autocar/project3/project3/project/tools/webview_nav_marker速度规划_科目二/nav_mark_points_20260725_205429.csv
// 生成时间：2026-07-26 14:50:38

#define NAV_REPLAY_START_HEADING_VALID 0
#define NAV_REPLAY_START_HEADING_DEG 102.568f

#define NAV_REPLAY_STATIC_ROUTE_COUNT 5

static const NavRamPoint_t nav_replay_static_route_points[5] = {
    {-4954.764f, -3462.580f, 27.437f, 151.452f, (uint8)1, 0.000f},
    {-8291.938f, 1648.106f, -61.351f, 127.399f, (uint8)1, 0.000f},
    {-14901.735f, -3018.678f, 32.467f, 149.237f, (uint8)1, 0.000f},
    {-15074.154f, -854.538f, -89.486f, 118.350f, (uint8)1, 0.000f},
    {-1161.115f, 1437.469f, -171.564f, 110.704f, (uint8)0, 0.000f},
};

#endif // _NAV_REPLAY_ROUTE_TABLE_H_
