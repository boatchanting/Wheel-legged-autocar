#include "system_monitor.h"

static system_monitor_info_t g_monitor_info = {0};

static uint64 g_window_start_cycle = 0;
static uint64 g_loop_begin_cycle = 0;
static uint64 g_busy_cycle_acc = 0;
static uint32 g_cpu_hz = 0;
static uint32 g_stack_top = 0;

static uint32 system_monitor_get_stack_top(void)
{
    const uint32 *vector_table = (const uint32 *)SCB->VTOR;
    return vector_table[0];
}

static uint32 system_monitor_cycles_to_ms(uint64 cycles)
{
    if (g_cpu_hz == 0U)
    {
        return 0U;
    }
    return (uint32)((cycles * 1000ULL) / g_cpu_hz);
}

void system_monitor_init(void)
{
    g_cpu_hz = SYS_MONITOR_CPU_HZ;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0U;

    g_stack_top = system_monitor_get_stack_top();
    g_window_start_cycle = (uint64)DWT->CYCCNT;
    g_busy_cycle_acc = 0ULL;
    g_monitor_info.sample_window_ms = SYS_MONITOR_WINDOW_MS;
    g_monitor_info.ram_total_bytes = SYS_MONITOR_TOTAL_RAM_BYTES;
}

void system_monitor_loop_begin(void)
{
    g_loop_begin_cycle = (uint64)DWT->CYCCNT;
}

void system_monitor_loop_end(void)
{
    uint64 now_cycle = (uint64)DWT->CYCCNT;
    if (now_cycle >= g_loop_begin_cycle)
    {
        g_busy_cycle_acc += (now_cycle - g_loop_begin_cycle);
    }
}

void system_monitor_update(void)
{
    uint64 now_cycle = (uint64)DWT->CYCCNT;
    uint64 elapsed_cycle = 0ULL;
    uint32 elapsed_ms = 0U;

    if (now_cycle >= g_window_start_cycle)
    {
        elapsed_cycle = now_cycle - g_window_start_cycle;
    }

    elapsed_ms = system_monitor_cycles_to_ms(elapsed_cycle);
    if (elapsed_ms < SYS_MONITOR_WINDOW_MS)
    {
        return;
    }

    if (elapsed_cycle > 0ULL)
    {
        float usage = ((float)g_busy_cycle_acc * 100.0f) / (float)elapsed_cycle;
        if (usage > 100.0f)
        {
            usage = 100.0f;
        }
        g_monitor_info.cpu_usage_percent = usage;
    }
    else
    {
        g_monitor_info.cpu_usage_percent = 0.0f;
    }

    uint32 current_sp = __get_MSP();
    if (g_stack_top > current_sp)
    {
        g_monitor_info.ram_used_bytes = g_stack_top - current_sp;
    }
    else
    {
        g_monitor_info.ram_used_bytes = 0U;
    }

    if (g_monitor_info.ram_total_bytes > 0U)
    {
        g_monitor_info.ram_usage_percent =
            ((float)g_monitor_info.ram_used_bytes * 100.0f) / (float)g_monitor_info.ram_total_bytes;
        if (g_monitor_info.ram_usage_percent > 100.0f)
        {
            g_monitor_info.ram_usage_percent = 100.0f;
        }
    }
    else
    {
        g_monitor_info.ram_usage_percent = 0.0f;
    }

    g_window_start_cycle = now_cycle;
    g_busy_cycle_acc = 0ULL;
}

const system_monitor_info_t *system_monitor_get_info(void)
{
    return &g_monitor_info;
}
