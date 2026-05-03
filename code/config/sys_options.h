#ifndef __SYS_OPTIONS_H__
#define __SYS_OPTIONS_H__

// WIFI_USE mode:
// 0  -> use core0 default WiFi
// 1  -> use core1 WiFi
// 10 -> use core0 custom WiFi
// others -> disable WiFi
#define WIFI_USE 255
#define WIFI_IMAGE_SEND 0

#define WIFI_USE_CORE0_DEFAULT   (0 == WIFI_USE)
#define WIFI_USE_CORE1           (1 == WIFI_USE)
#define WIFI_USE_CORE0_CUSTOM    (10 == WIFI_USE)
#define WIFI_USE_CORE0_ANY       (WIFI_USE_CORE0_DEFAULT || WIFI_USE_CORE0_CUSTOM)
#define WIFI_USE_ANY             (WIFI_USE_CORE0_ANY || WIFI_USE_CORE1)

#define DEBUG_DISPLAY 1
#define REMOTE_CONTROL 1
#define DEBUG_LOG_ENABLE 0
#define IMU_CATEGORY 3
#define SUBS_CATEGORY 1

// ---------------- plan configuration ----------------
#define CURRENT_NAV_PLAN   1
/*
【科目一优化与拆分】
1.1. 惯导条件下，去除了任务点搜索 code/navigation/nav_replay.c
1.2. 惯导模式下，注释了所有状态机代码

【科目二优化与拆分】
2.1. 惯导条件下，状态机触发不用先角度对准 code/navigation/nav_replay.c
2.2. 惯导条件下，状态机里保留了雷区状态机 code/navigation/nav_replay.c
*/

#endif // __SYS_OPTIONS_H__
