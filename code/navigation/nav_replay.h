#ifndef _NAV_REPLAY_H_
#define _NAV_REPLAY_H_

#include "zf_common_headfile.h"

typedef enum
{
    REPLAY_IDLE,
    REPLAY_RUNNING,
    REPLAY_FINISHED
} NavReplayState_e;

#include "nav_replay/nav_options.h"

#if (CURRENT_NAV_PLAN == 1) && (NAV_PLAN1_METHOD == PLAN1_METHOD_PURE_PURSUIT)
#include "nav_replay/plan1/plan1_pure_pursuit.h"
#elif (CURRENT_NAV_PLAN == 1) && (NAV_PLAN1_METHOD == PLAN1_METHOD_GNSS)
#include "nav_replay/plan1/plan1_gnss.h"
#elif (CURRENT_NAV_PLAN == 2)
#include "nav_replay/plan2/plan2_minefield.h"
#elif (CURRENT_NAV_PLAN == 3)
#include "nav_replay/plan3/plan3_precise.h"
#else
#include "nav_replay/template/nav_plan_template.h"
#endif

#endif
