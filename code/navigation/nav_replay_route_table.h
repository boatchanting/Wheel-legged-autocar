#ifndef _NAV_REPLAY_ROUTE_TABLE_H_
#define _NAV_REPLAY_ROUTE_TABLE_H_

#include "nav_ram.h"

// 由 tools/webview_nav_marker/csv_to_nav_table.py 自动生成
// 源 CSV：D:/New_luntui/project3/project/tools/webview_nav_marker速度规划/nav_mark_points_20260705_172049.csv
// 生成时间：2026-07-05 20:12:14

#define NAV_REPLAY_START_HEADING_VALID 0
#define NAV_REPLAY_START_HEADING_DEG 88.867f

#define NAV_REPLAY_STATIC_ROUTE_COUNT 8

static const NavRamPoint_t nav_replay_static_route_points[8] = {
    {-12654.102f, -168.477f, 5.214f, 124.948f, (uint8)0, 0.000f, 0.000000f},
    {-11562.741f, -1725.164f, 161.975f, 170.821f, (uint8)0, 0.000f, 0.000000f},
    {-10132.299f, -2163.919f, 149.224f, 179.918f, (uint8)0, 0.000f, 0.000000f},
    {-8798.073f, -2596.508f, 159.844f, 187.796f, (uint8)0, 0.000f, 0.000000f},
    {-7232.793f, -2547.100f, 167.197f, 193.528f, (uint8)0, 0.000f, 0.000000f},
    {-5689.824f, -2182.294f, -173.903f, 174.724f, (uint8)0, 0.000f, 0.000000f},
    {-3835.331f, -2462.796f, -169.795f, 177.149f, (uint8)0, 0.000f, 0.000000f},
    {1855.921f, -1758.415f, -174.512f, 178.937f, (uint8)0, 0.000f, 0.000000f},
};

#endif // _NAV_REPLAY_ROUTE_TABLE_H_
