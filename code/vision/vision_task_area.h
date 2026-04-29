#ifndef VISION_TASK_AREA_H
#define VISION_TASK_AREA_H

#include "zf_common_headfile.h"
#include "vision/vision_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VISION_BRIDGE_TASK_ENABLE                    (1)
#define VISION_BRIDGE_TASK_NAV_CORRECT_ENABLE        (0)
#define VISION_BRIDGE_TASK_NAV_CORRECT_DISTANCE_MM   (3000.0f)

#define VISION_BRIDGE_TASK_ENTER_TIMEOUT_TICKS       (2500U)
#define VISION_BRIDGE_TASK_ALIGN_TIMEOUT_TICKS       (1500U)
#define VISION_BRIDGE_TASK_ALIGN_OK_TICKS            (60U)
#define VISION_BRIDGE_TASK_RUN_MIN_MM                (1000.0f)
#define VISION_BRIDGE_TASK_RUN_MAX_MM                (3400.0f)
#define VISION_BRIDGE_TASK_EXIT_BUFFER_MM            (300.0f)
#define VISION_BRIDGE_TASK_EXIT_LOST_TICKS           (150U)
#define VISION_BRIDGE_TASK_BRIDGE_HOLD_TICKS         (220U)

#define VISION_BRIDGE_TASK_ALIGN_YAW_TOL_DEG         (4.0f)
#define VISION_BRIDGE_TASK_ALIGN_ERR_TOL_DEG         (1.5f)
#define VISION_BRIDGE_TASK_EXIT_WHITE_RATIO_U16      (180U)

#define VISION_BRIDGE_TASK_LINE_SIGN                 (-1.0f)
#define VISION_BRIDGE_TASK_K_LAT_DEG_PER_PX          (0.18f)
#define VISION_BRIDGE_TASK_K_YAW_DEG_PER_DEG         (0.65f)
#define VISION_BRIDGE_TASK_MAX_ERR_DEG               (16.0f)
#define VISION_BRIDGE_TASK_YAW_HOLD_MAX_ERR_DEG      (10.0f)

#define VISION_BRIDGE_TASK_ALIGN_SPEED_SET           (0.0f)
#define VISION_BRIDGE_TASK_RUN_SPEED_SET             (-150.0f)
#define VISION_BRIDGE_TASK_BRIDGE_SPEED_SET          (-110.0f)
#define VISION_BRIDGE_TASK_BLIND_SPEED_SET           (-90.0f)
#define VISION_BRIDGE_TASK_EXIT_SPEED_SET            (-90.0f)
#define VISION_BRIDGE_TASK_HEIGHT_STEP_SCALE         (0.10f)

typedef enum
{
    VISION_BRIDGE_TASK_IDLE = 0,
    VISION_BRIDGE_TASK_ENTER_PVC,
    VISION_BRIDGE_TASK_ALIGN,
    VISION_BRIDGE_TASK_RUN,
    VISION_BRIDGE_TASK_EXIT,
    VISION_BRIDGE_TASK_FINISH,
    VISION_BRIDGE_TASK_FAILSAFE,
} vision_bridge_task_state_e;

typedef struct
{
    uint8 enabled;
    vision_bridge_task_state_e state;
    uint32 state_ticks;
    uint32 last_seq;
    float traveled_mm;
    float err_degree_cmd;
    float speed_cmd;
    uint8 line_stable;
    uint8 bridge_stable;
    uint16 line_confidence_u16;
    uint16 bridge_confidence_u16;
    uint16 line_roi_white_ratio_u16;
    int16 line_lateral_px_x100;
    int16 line_yaw_error_deg_x100;
    uint16 exit_lost_ticks;
    uint16 bridge_hold_ticks;
} vision_bridge_task_status_t;

extern volatile uint8 g_bridge_vision_task_enable;
extern volatile vision_bridge_task_status_t g_bridge_vision_task_status;

void VisionBridgeTask_Init(void);
void VisionBridgeTask_Start(void);
void VisionBridgeTask_Stop(void);
uint8 VisionBridgeTask_IsActive(void);
void VisionBridgeTask_Update_2ms(void);

#ifdef __cplusplus
}
#endif

#endif
