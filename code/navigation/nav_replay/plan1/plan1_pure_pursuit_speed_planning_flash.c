#define PLAN1_SPEED_PLANNING_FLASH_BACKEND 1
#include "../nav_replay.h"
#include "plan1_pure_pursuit_speed_planning_flash.h"

#if (CURRENT_NAV_PLAN == 1) && (NAV_PLAN1_METHOD == PLAN1_PURE_PURSUIT_SPEED_PLANNING_FLASH)
#include "plan1_pure_pursuit_speed_planning.c"
#endif
