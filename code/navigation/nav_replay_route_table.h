#ifndef _NAV_REPLAY_ROUTE_TABLE_H_
#define _NAV_REPLAY_ROUTE_TABLE_H_

#include "nav_ram.h"

// 由 tools/webview_nav_marker速度规划_科目二/csv_to_nav_table.py 自动生成
// 源 CSV：D:/autocar/project3/project3/project/tools/webview_nav_marker速度规划_科目二/nav_mark_points_20260729_103533.csv
// 生成时间：2026-07-29 10:35:44

#define NAV_REPLAY_START_HEADING_VALID 0
#define NAV_REPLAY_START_HEADING_DEG 59.542f

#define NAV_REPLAY_STATIC_ROUTE_COUNT 7

static const NavRamPoint_t nav_replay_static_route_points[7] = {
    {-5019.906f, -1448.393f, 16.519f, 80.558f, (uint8)1, 0.000f},
    {-8461.745f, -3655.490f, 33.232f, 86.387f, (uint8)1, 0.000f},
    {-12367.769f, -1201.299f, -30.876f, 116.894f, (uint8)1, 0.000f},
    {-13942.402f, -3900.000f, 54.664f, 88.507f, (uint8)1, 0.000f},
    {-16590.496f, -4059.653f, 3.766f, 75.551f, (uint8)6, 0.000f},
    {-18484.326f, -2893.368f, -48.417f, 134.725f, (uint8)6, 0.000f},
    {-16896.352f, -583.237f, -175.170f, 121.782f, (uint8)6, 0.000f},
};

#endif // _NAV_REPLAY_ROUTE_TABLE_H_
