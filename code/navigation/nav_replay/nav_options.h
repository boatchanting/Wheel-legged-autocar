#ifndef _NAV_REPLAY_OPTIONS_H_
#define _NAV_REPLAY_OPTIONS_H_

#include "../config/sys_options.h"

#define PLAN1_METHOD_PURE_PURSUIT 1
#define PLAN1_METHOD_GNSS         2

#ifndef NAV_PLAN1_METHOD
#define NAV_PLAN1_METHOD PLAN1_METHOD_PURE_PURSUIT
#endif

#endif
