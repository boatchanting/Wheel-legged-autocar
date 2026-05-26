/*
 * =================================================================================
 * File: vision_slope_control.h
 * Role: Core 0 visual slope task state machine configuration and public interface.
 * Notes:
 *   1) Trigger once through vision_detected_slope_point.
 *   2) Keep fixed high speed during the whole slope task.
 *   3) Use PVC visual result only for entrance alignment, then switch to yaw-lock.
 * =================================================================================
 */
#ifndef VISION_SLOPE_CONTROL_H
#define VISION_SLOPE_CONTROL_H

#include "zf_common_headfile.h"
#include "vision/vision_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- 1. Feature switch --- */
#define VISION_SLOPE_TASK_ENABLE                    (1)

/* --- 2. State machine timing and thresholds --- */
/* All TICKS below are based on the 2ms update period. */
#define VISION_SLOPE_TASK_ENTER_TIMEOUT_TICKS      (2500U)     /* 5s timeout for PVC entrance stage */
#define VISION_SLOPE_TASK_FAST_HOLD_TICKS          (150U)      /* 300ms yaw-lock hold after PVC is fully occupied */
#define VISION_SLOPE_TASK_EXIT_DISTANCE_MM         (2000.0f)   /* Auto-exit after 2000mm from yaw-lock start */
#define VISION_SLOPE_TASK_PVC_FULL_RATIO_U16       (900U)      /* 90% PVC occupation threshold */

/* --- 3. Fixed speed and yaw-lock parameters --- */
#define VISION_SLOPE_TASK_ENTER_SPEED_SET          (-400.0f)   /* Fixed high speed from trigger to finish */
#define VISION_SLOPE_TASK_CRUISE_SPEED_SET         (-400.0f)   /* Keep the same high speed after 300ms hold */
#define VISION_SLOPE_TASK_YAW_HOLD_MAX_ERR_DEG     (10.0f)

typedef enum
{
    VISION_SLOPE_TASK_IDLE = 0,
    VISION_SLOPE_TASK_ENTER_PVC,
    VISION_SLOPE_TASK_LOCK_FAST,
    VISION_SLOPE_TASK_LOCK_CRUISE,
    VISION_SLOPE_TASK_FINISH,
    VISION_SLOPE_TASK_FAILSAFE,
} vision_slope_task_state_e;

typedef struct
{
    uint8 enabled;
    vision_slope_task_state_e state;
    uint32 state_ticks;
    uint32 last_seq;
    float traveled_mm;
    float locked_yaw_deg;
    float err_degree_cmd;
    float speed_cmd;
    uint8 pvc_stable;
    uint16 pvc_ratio_u16;
    int16 pvc_steer_error_px_x100;
} vision_slope_task_status_t;

extern volatile uint8 g_slope_vision_task_enable;
extern volatile uint8_t vision_detected_slope_point;
extern volatile vision_slope_task_status_t g_slope_vision_task_status;

void VisionSlopeTask_Init(void);
void VisionSlopeTask_Start(void);
void VisionSlopeTask_Stop(void);
uint8 VisionSlopeTask_IsActive(void);
void VisionSlopeTask_Update_2ms(void);

#ifdef __cplusplus
}
#endif

#endif
