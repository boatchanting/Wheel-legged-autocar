#ifndef __SYSTEM_MONITOR_H__
#define __SYSTEM_MONITOR_H__

#include "zf_common_headfile.h"

// CPU频率(Hz)，默认与主频250MHz一致。
#ifndef SYS_MONITOR_CPU_HZ
#define SYS_MONITOR_CPU_HZ 250000000UL
#endif

// 采样窗口(毫秒)
#ifndef SYS_MONITOR_WINDOW_MS
#define SYS_MONITOR_WINDOW_MS 1000U
#endif

// 如果你知道1核可用RAM大小，可以在编译选项或此处重定义此宏。
// 例如：#define SYS_MONITOR_TOTAL_RAM_BYTES (512U * 1024U)
#ifndef SYS_MONITOR_TOTAL_RAM_BYTES
#define SYS_MONITOR_TOTAL_RAM_BYTES 0U
#endif

typedef struct
{
    float cpu_usage_percent;      // CPU占用率(0~100)
    uint32 sample_window_ms;      // 采样窗口(毫秒)
    uint32 ram_used_bytes;        // RAM占用估算(基于栈增长)
    uint32 ram_total_bytes;       // RAM总大小(配置项)
    float ram_usage_percent;      // RAM占用率(0~100)，当ram_total_bytes=0时为0
} system_monitor_info_t;

void system_monitor_init(void);
void system_monitor_loop_begin(void);
void system_monitor_loop_end(void);
void system_monitor_update(void);
const system_monitor_info_t *system_monitor_get_info(void);

#endif
