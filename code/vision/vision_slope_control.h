/*
 * =================================================================================
 * 文件: vision_slope_control.h
 * 作用: 0 核(Core 0)斜坡路段视觉控制状态机的参数配置与对外接口。
 * 说明: 斜坡入口使用既有 PVC 视觉结果完成方向校准；进入斜坡后不再识别蓝色坡面，
 *       而是锁定惯导航向和速度，依靠惯导里程在白色平地前自动退出。
 * =================================================================================
 */
#ifndef VISION_SLOPE_CONTROL_H
#define VISION_SLOPE_CONTROL_H

#include "zf_common_headfile.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- 1. 功能开关 --- */
#define VISION_SLOPE_TASK_ENABLE                     (1)         /* 斜坡视觉任务总开关 */

/* --- 2. 速度、时间与里程参数 --- */
/* 注意：任务由 2ms 中断调度，因此 150 TICKS 对应 300ms。 */
#define VISION_SLOPE_TASK_ENTRY_SPEED_SET            (-600.0f)   /* PVC 校准及刚进入斜坡时的速度 */
#define VISION_SLOPE_TASK_RUN_SPEED_SET              (-400.0f)   /* 进入斜坡 300ms 后的稳定行驶速度 */
#define VISION_SLOPE_TASK_ENTRY_HOLD_TICKS           (150U)      /* 上坡后保持入口速度的时长：300ms */
#define VISION_SLOPE_TASK_EXIT_DISTANCE_MM           (2500.0f)   /* 从锁定航向开始累计 2500mm 后退出 */

/* PVC 已稳定检测到入口后，必须让方向误差连续满足以下条件才允许锁角进入斜坡。 */
#define VISION_SLOPE_TASK_PVC_ALIGN_ERR_TOL_DEG       (1.5f)      /* PVC 方向校准完成时允许的误差 */
#define VISION_SLOPE_TASK_PVC_ALIGN_OK_TICKS          (50U)       /* 连续满足误差条件的时长：100ms */

/* --- 3. 航向锁定参数 --- */
#define VISION_SLOPE_TASK_YAW_HOLD_MAX_ERR_DEG       (10.0f)     /* 锁定航向时发送到底层前的最大修正角 */

/**
 * @brief 斜坡任务的各个阶段。
 */
typedef enum
{
    VISION_SLOPE_TASK_IDLE = 0,         /* 空闲：未接管车辆 */
    VISION_SLOPE_TASK_PVC_ALIGN,        /* 使用白色 PVC 斜坡入口完成方向校准 */
    VISION_SLOPE_TASK_ENTRY_HOLD,       /* 已进入斜坡，锁角并保持入口速度 300ms */
    VISION_SLOPE_TASK_RUN,              /* 蓝色坡面期间锁角，以降低后的速度继续行驶 */
    VISION_SLOPE_TASK_FINISH,           /* 任务完成，准备交还控制权 */
    VISION_SLOPE_TASK_FAILSAFE,         /* 故障或急停时释放控制权并停车 */
} vision_slope_task_state_e;

/**
 * @brief 斜坡任务运行状态，供调试界面或上位机查看。
 */
typedef struct
{
    uint8 enabled;                       /* 任务是否已启动 */
    vision_slope_task_state_e state;     /* 当前状态机阶段 */
    uint32 state_ticks;                  /* 当前阶段已运行的 2ms tick 数 */
    float traveled_mm;                   /* 从进入斜坡并锁角起累计的惯导距离 */
    float locked_yaw_deg;                /* 由 PVC 校准完成后锁定的惯导航向 */
    float err_degree_cmd;                /* 当前下发的方向误差指令 */
    float speed_cmd;                     /* 当前下发的目标速度指令 */
    uint8 pvc_stable_detected;           /* PVC 控制模块是否稳定检测到入口 */
} vision_slope_task_status_t;

/* --- 4. 对外变量与函数接口 --- */
extern volatile uint8 g_slope_vision_task_enable;
extern volatile vision_slope_task_status_t g_slope_vision_task_status;

/**
 * @brief 初始化斜坡任务状态机。
 */
void VisionSlopeTask_Init(void);

/**
 * @brief 启动斜坡视觉任务；真正的状态切换由 2ms 中断执行。
 */
void VisionSlopeTask_Start(void);

/**
 * @brief 强制停止斜坡任务，并清零车辆控制输出。
 */
void VisionSlopeTask_Stop(void);

/**
 * @brief 查询斜坡任务是否正在接管车辆。
 * @return 1: 正在进行；0: 空闲。
 */
uint8 VisionSlopeTask_IsActive(void);

/**
 * @brief 斜坡任务的 2ms 定时更新函数。
 */
void VisionSlopeTask_Update_2ms(void);

#ifdef __cplusplus
}
#endif

#endif
