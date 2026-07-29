/*
 * =================================================================================
 * 文件: vision_three_stage_control.h
 * 作用: 三级跳视觉融合状态机（0 核控制层）
 * 说明:
 *   1) 外部只需要一个触发标志位即可启动状态机；
 *   2) 速度不在本模块内控制，仍由惯导/导航链路负责；
 *   3) 本模块只负责“方向修正 + 三次跳跃触发”。
 * =================================================================================
 */
#ifndef VISION_THREE_STAGE_CONTROL_H
#define VISION_THREE_STAGE_CONTROL_H

#include "zf_common_headfile.h"
#include "vision/vision_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif
extern volatile float err_degree;           /* 方向盘打多少度 */
extern volatile float target_speed_set;     /* 目标速度（负数代表前进） */

/* ---------------- 编译开关与默认配置 ---------------- */
#define VISION_THREE_STAGE_CONTROL_ENABLE                (1)
#define VISION_THREE_STAGE_CONTROL_DEFAULT_ACTIVE        (1U)

/* 方向控制参数（与 PVC 控制同量纲） */
#define VISION_THREE_STAGE_LATERAL_SIGN                  (0.0f)
#define VISION_THREE_STAGE_K_LAT_DEG_PER_MM              (0.20f)
#define VISION_THREE_STAGE_K_YAW_DEG_PER_DEG             (0.50f)
#define VISION_THREE_STAGE_MAX_ERR_DEG                   (18.0f)
#define VISION_THREE_STAGE_DEADBAND_DEG                  (0.30f)

/* 时序参数（2ms tick） */
#define VISION_THREE_STAGE_STALE_TIMEOUT_TICKS           (1500U)  /* 3.0s */
#define VISION_THREE_STAGE_STATE_TIMEOUT_TICKS           (8000U)  /* 16.0s */
#define VISION_THREE_STAGE_JUMP_COOLDOWN_TICKS           (22U)    /* 44ms */
#define VISION_THREE_STAGE_JUMP3_DELAY_AFTER_JUMP2_TICKS (251U)    /* 180ms，2ms tick */
#define VISION_THREE_STAGE_LOCK_STABLE_FRAMES            (3U)
#define VISION_THREE_STAGE_BLACK_GAP_LOST_FRAMES         (2U)
#define VISION_THREE_STAGE_REACQUIRE_STABLE_FRAMES       (2U)
#define VISION_THREE_STAGE_EXIT_STABLE_FRAMES            (2U)

/* 像素阈值默认值（可在线调整） */
#define VISION_THREE_STAGE_JUMP1_BOTTOM_Y_DEFAULT        (32U) //第一级台阶底端小于该值时跳跃，增大阈值，车必须走得更近才跳
#define VISION_THREE_STAGE_JUMP2_TOP_Y_DEFAULT           (38U) //第二级台阶顶端小于该值时跳跃，增大阈值，车必须走得更近才跳 
//在30-40阈值情况下大概1cm参数加1 ，不识别竖着的台阶，只识别横着的台阶，群里7.29早晨的图片，大概4cm的阴影，使用的是40阈值；大概1cm的，用的38的阈值；也就是阴影越大相当于应该离"台阶"(小车认为的白块)越近再跳
#define VISION_THREE_STAGE_JUMP3_BOTTOM_Y_DEFAULT        (32U)//这个没有使用。科目三目前的逻辑最后一跳是依赖写死时间实现的，需要调的话调上面那个“VISION_THREE_STAGE_JUMP3_DELAY_AFTER_JUMP2_TICKS”
//第三级台阶底端小于该值时跳跃，增大阈值，车必须走得更近才跳，粗调参时候，可以用第一级台阶那个值用一下
#define VISION_THREE_STAGE_EXIT_TOP_Y_DEFAULT            (24U)//脱出时候阈值，目前来看这个阈值会在三级跳顶部脱出，比较稳定

typedef enum
{
    VISION_THREE_STAGE_CTRL_IDLE = 0,
    VISION_THREE_STAGE_CTRL_WAIT_PVC_LOCK,
    VISION_THREE_STAGE_CTRL_WAIT_JUMP1_BOTTOM,
    VISION_THREE_STAGE_CTRL_WAIT_JUMP2_TOP,
    VISION_THREE_STAGE_CTRL_WAIT_SECOND_PVC,
    VISION_THREE_STAGE_CTRL_WAIT_JUMP3_BOTTOM,
    VISION_THREE_STAGE_CTRL_WAIT_EXIT_TOP,
    VISION_THREE_STAGE_CTRL_FINISH,
    VISION_THREE_STAGE_CTRL_FAILSAFE,
} vision_three_stage_ctrl_state_e;

typedef enum
{
    VISION_THREE_STAGE_EXIT_NONE = 0,
    VISION_THREE_STAGE_EXIT_SUCCESS,
    VISION_THREE_STAGE_EXIT_TIMEOUT,
    VISION_THREE_STAGE_EXIT_STALE,
    VISION_THREE_STAGE_EXIT_MOTOR_OFF,
    VISION_THREE_STAGE_EXIT_YAW_INVALID,
    VISION_THREE_STAGE_EXIT_MANUAL_STOP,
} vision_three_stage_exit_reason_e;

typedef struct
{
    uint8 enabled;                       /* 模块使能 */
    uint8 active;                        /* 状态机是否在运行 */
    vision_three_stage_ctrl_state_e state;
    uint8 jump_count;                    /* 已触发跳跃次数（0~3） */
    uint8 black_gap_seen;                /* 是否已看到中间“黑区/PVC丢失区” */
    uint8 pvc_stable_detected;           /* 当前帧是否稳定检测到 PVC */
    uint8 pvc_raw_detected;              /* 当前帧是否原始检测到 PVC */
    uint8 pvc_entry_bottom_y;            /* 当前帧 PVC 下边界像素 y */
    uint8 pvc_entry_top_y;               /* 当前帧 PVC 上边界像素 y */
    uint16 state_ticks;                  /* 当前状态持续时间（2ms tick） */
    uint16 stale_ticks;                  /* 数据未更新计数（2ms tick） */
    uint16 stable_count;                 /* 通用稳定计数器 */
    uint16 lost_count;                   /* 通用丢失计数器 */
    uint16 jump_cooldown_ticks;          /* 跳跃最小间隔计数 */
    uint32 last_seq;                     /* 最近处理的数据包序号 */
    int16 pvc_lateral_mm;                /* 方向控制输入：横向偏差 */
    int16 pvc_yaw_error_deg_x100;        /* 方向控制输入：偏航误差 */
    float err_degree_cmd;                /* 输出给底盘的方向修正量 */
    vision_three_stage_exit_reason_e exit_reason;
} vision_three_stage_control_status_t;

extern volatile vision_three_stage_control_status_t g_vision_three_stage_control_status;
extern volatile uint8 g_vision_three_stage_control_enable;

/* 三个跳跃阈值 + 退出阈值（像素行号，可独立调参） */
extern volatile uint8 g_vision_three_stage_jump1_bottom_y;
extern volatile uint8 g_vision_three_stage_jump2_top_y;
extern volatile uint8 g_vision_three_stage_jump3_bottom_y;
extern volatile uint8 g_vision_three_stage_exit_top_y;

void VisionThreeStageControl_Init(void);
void VisionThreeStageControl_SetEnable(uint8 enable);
uint8 VisionThreeStageControl_IsEnabled(void);

void VisionThreeStageControl_Start(void);
void VisionThreeStageControl_Stop(void);
uint8 VisionThreeStageControl_IsActive(void);

void VisionThreeStageControl_Update_2ms(void);

#ifdef __cplusplus
}
#endif

#endif
