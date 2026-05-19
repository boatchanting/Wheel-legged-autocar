#ifndef _NAV_REPLAY_H_
#define _NAV_REPLAY_H_

#include "zf_common_headfile.h"
#include "nav_ram.h"
#include "nav_options.h"

typedef enum
{
    REPLAY_IDLE,
    REPLAY_RUNNING,
    REPLAY_FINISHED
} NavReplayState_e;

#if CURRENT_NAV_PLAN == 1
  #if GNSS_NAV == 1 && (NAV_PLAN1_METHOD == PLAN1_METHOD_GNSS)
    #include "plan1/plan1_gnss.h"
  #else
    #include "plan1/plan1_pure_pursuit.h"
  #endif
#elif CURRENT_NAV_PLAN == 2
  #include "plan2/plan2_pure_pursuit.h"
#elif CURRENT_NAV_PLAN == 3
  #include "plan3/plan3_precise.h"
#else
  #include "template/nav_plan_template.h"
#endif

#endif
