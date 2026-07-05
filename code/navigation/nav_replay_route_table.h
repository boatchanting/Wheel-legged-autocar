#ifndef _NAV_REPLAY_ROUTE_TABLE_H_
#define _NAV_REPLAY_ROUTE_TABLE_H_

#include "nav_ram.h"

#define NAV_REPLAY_START_HEADING_VALID 0
#define NAV_REPLAY_START_HEADING_DEG 0.0f

#define NAV_REPLAY_STATIC_ROUTE_COUNT 1

static const NavRamPoint_t nav_replay_static_route_points[1] = {
    {0.0f, 0.0f, 0.0f, 0.0f, (uint8)0},
};

#endif // _NAV_REPLAY_ROUTE_TABLE_H_
