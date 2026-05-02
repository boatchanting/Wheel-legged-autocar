#ifndef VISION_BUMPY_CONTROL_H
#define VISION_BUMPY_CONTROL_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"
#include "vision/vision_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VISION_BUMPY_CONTROL_ENABLE            (1)
#define VISION_BUMPY_CONTROL_DEFAULT_ACTIVE    (0)
#define VISION_BUMPY_CONTROL_PROFILE_ENABLE    (1)
#define VISION_BUMPY_CONTROL_PROFILE_TIMER     (TC_TIME2_CH0)

#define VISION_BUMPY_STALE_TIMEOUT_TICKS       (120U)
#define VISION_BUMPY_K_STEER_DEG_PER_PX        (0.06f)
#define VISION_BUMPY_MAX_ERR_DEG               (18.0f)
#define VISION_BUMPY_DEADBAND_DEG              (0.25f)

typedef enum
{
    VISION_BUMPY_CTRL_IDLE = 0,
    VISION_BUMPY_CTRL_SEARCH,
    VISION_BUMPY_CTRL_TRACK,
    VISION_BUMPY_CTRL_STALE,
} vision_bumpy_control_state_e;

typedef struct
{
    uint8 enabled;
    uint8 has_new_packet;
    uint8 stable_detected;
    uint8 raw_detected;
    uint8 phase;
    uint8 mode;
    uint16 confidence_u16;
    uint16 stale_ticks;
    uint32 last_seq;
    vision_bumpy_control_state_e state;
    int16 steer_error_px_x100;
    float err_degree_cmd;
} vision_bumpy_control_status_t;

extern volatile vision_bumpy_control_status_t g_vision_bumpy_control_status;
extern volatile runtime_profiler_t g_vision_bumpy_control_profiler;
extern volatile uint8 g_bumpy_control_enable;

void VisionBumpyControl_Init(void);
void VisionBumpyControl_SetEnable(uint8 enable);
uint8 VisionBumpyControl_IsEnabled(void);
void VisionBumpyControl_Update_2ms(void);
float VisionBumpyControl_GetErrDegreeCmd(void);

#ifdef __cplusplus
}
#endif

#endif
