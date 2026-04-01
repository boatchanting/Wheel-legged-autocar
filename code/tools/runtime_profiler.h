#ifndef _runtime_profiler_h_
#define _runtime_profiler_h_

#include "zf_common_headfile.h"

typedef struct
{
    uint32 start_us;
    uint32 last_us;
    uint32 min_us;
    uint32 max_us;
    uint32 avg_us;
    uint32 count;
} runtime_profiler_t;

static inline void runtime_profiler_reset(volatile runtime_profiler_t *prof)
{
    prof->start_us = 0U;
    prof->last_us  = 0U;
    prof->min_us   = 0xFFFFFFFFU;
    prof->max_us   = 0U;
    prof->avg_us   = 0U;
    prof->count    = 0U;
}

static inline void runtime_profiler_update(volatile runtime_profiler_t *prof, uint32 cost_us)
{
    uint32 prev_count = prof->count;
    uint32 next_count = prev_count + 1U;

    prof->last_us = cost_us;

    if (cost_us > prof->max_us)
    {
        prof->max_us = cost_us;
    }
    if (cost_us < prof->min_us)
    {
        prof->min_us = cost_us;
    }

    if (prev_count == 0U)
    {
        prof->avg_us = cost_us;
    }
    else
    {
        uint64_t sum = (uint64_t)prof->avg_us * (uint64_t)prev_count + (uint64_t)cost_us;
        prof->avg_us = (uint32)(sum / (uint64_t)next_count);
    }

    prof->count = next_count;
}

#define RUNTIME_PROFILE_RESET(_prof_ptr) \
    do { runtime_profiler_reset((_prof_ptr)); } while (0)

#define RUNTIME_PROFILE_BEGIN(_prof_obj, _timer_ch) \
    do { (_prof_obj).start_us = timer_get((_timer_ch)); } while (0)

#define RUNTIME_PROFILE_END(_prof_ptr, _timer_ch) \
    do { \
        uint32 _rp_end_us = timer_get((_timer_ch)); \
        uint32 _rp_cost_us = (uint32)(_rp_end_us - (_prof_ptr)->start_us); \
        runtime_profiler_update((_prof_ptr), _rp_cost_us); \
    } while (0)

#endif

//调用模版
// volatile runtime_profiler_t g_xxx_prof = {0};

// // init阶段
// timer_init(TC_TIME2_CH0, TIMER_US);
// timer_start(TC_TIME2_CH0);
// RUNTIME_PROFILE_RESET(&g_xxx_prof);

// // 被测代码前后
// RUNTIME_PROFILE_BEGIN(g_xxx_prof, TC_TIME2_CH0);
// your_function();
// RUNTIME_PROFILE_END(&g_xxx_prof, TC_TIME2_CH0);

// // 打印
// printf("[XXX] cnt=%lu, avg=%lu us, min=%lu us, max=%lu us, last=%lu us\r\n",
//        (unsigned long)g_xxx_prof.count,
//        (unsigned long)g_xxx_prof.avg_us,
//        (unsigned long)g_xxx_prof.min_us,
//        (unsigned long)g_xxx_prof.max_us,
//        (unsigned long)g_xxx_prof.last_us);


// 实际上这样用即可
// RUNTIME_PROFILE_BEGIN(g_ekf_profiler, TC_TIME2_CH0);//代码统计计时开始
//             // 2.3 计算目标速度调整分量
//             float duty_adjustment = Servo_Speed_Control(target_speed_set, current_actual_speed,euler_angle.pitch);
//             RUNTIME_PROFILE_END(&g_ekf_profiler, TC_TIME2_CH0);//代码统计计时结束