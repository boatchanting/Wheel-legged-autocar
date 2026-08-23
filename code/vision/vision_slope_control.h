/*
 * =================================================================================
 * 文件: vision_slope_control.h
 * 作用: 0 核(Core 0)斜坡路段视觉控制状态机的参数配置与对外接口。
 * 说明: 斜坡入口使用既有 PVC 视觉结果完成方向校准；进入斜坡后锁定前进航向，
 *       通过惯导闭环负反馈持续修正方向，定速冲坡并依靠惯导里程自动停车跳跃。
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
#define VISION_SLOPE_TASK_PVC_ALIGN_TIMEOUT_TICKS    (2500U)     /* PVC 入口阶段最长 5s，防止识别异常时一直占用控制权 */
#define VISION_SLOPE_TASK_ENTRY_SPEED_SET            (-600.0f)   /* PVC 校准及刚进入斜坡时的速度 */
#define VISION_SLOPE_TASK_RUN_SPEED_SET              (-600.0f)   /* 进入斜坡 300ms 后的稳定行驶速度 */
#define VISION_SLOPE_TASK_PVC_ALIGN_SPEED_SET        (-600.0f)   /* 搜索并校准白色 PVC 斜坡入口时的低速 */
#define VISION_SLOPE_TASK_ENTRY_HOLD_TICKS           (150U)      /* 上坡后保持入口速度的时长：300ms */
#define VISION_SLOPE_TASK_STOP_DISTANCE_MM           (5500.0f)   /* 从任务接管并清零惯导坐标起累计，到达后停车并开始三级跳时序 */
#define VISION_SLOPE_TASK_JUMP1_DELAY_TICKS          (2000U)     /* 停车后等待 4000ms 触发第一跳 */
#define VISION_SLOPE_TASK_JUMP_INTERVAL_TICKS        (750U)      /* 相邻两次跳跃的间隔：1500ms */
#define VISION_SLOPE_TASK_BRAKE_FF_ENABLE             (1U)        /* 到达停车里程后允许普通刹车前馈建立反向制动力 */
#define VISION_SLOPE_TASK_PVC_FULL_RATIO_U16          (400U)      /* 白色 PVC 占图比例达到 40% 时认为车辆已进入斜坡入口 */

/* PVC 稳定识别且白色区域占满入口后，连续保持 50ms 才允许锁角进入斜坡。 */
#define VISION_SLOPE_TASK_PVC_ALIGN_OK_TICKS          (25U)       /* 连续满足入口确认条件的时长：50ms */

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
    VISION_SLOPE_TASK_RUN,              /* 蓝色坡面期间持续锁定前进航向，定速行驶冲坡 */
    VISION_SLOPE_TASK_WAIT_JUMP1,       /* 到达指定惯导里程后停车，等待第一次跳跃 */
    VISION_SLOPE_TASK_WAIT_JUMP2,       /* 第一次跳跃后，等待第二次跳跃 */
    VISION_SLOPE_TASK_WAIT_JUMP3,       /* 第二次跳跃后，等待第三次跳跃 */
    VISION_SLOPE_TASK_LOCKED,           /* 三次跳跃完成后保持停车并锁定状态机 */
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
    uint32 last_seq;                     /* 最近一次收到的视觉 IPC 序号 */
    float traveled_mm;                   /* 从任务接管并清零惯导坐标起累计的惯导距离 */
    float locked_yaw_deg;                /* 状态机启动时锁定的惯导航向 */
    float err_degree_cmd;                /* 当前下发的方向误差指令 */
    float speed_cmd;                     /* 当前下发的目标速度指令 */
    uint8 jump_count;                    /* 终止序列已触发的跳跃次数（0~3） */
    uint8 brake_ff_request;              /* 到达停车里程后请求底层刹车前馈 */
    uint8 pvc_stable_detected;           /* PVC 控制模块是否稳定检测到入口 */
    uint16 pvc_ratio_u16;                /* PVC 白色区域在画面内的占比，千分比 */
    int16 pvc_steer_error_px_x100;       /* PVC 给出的横向像素误差，放大 100 倍 */
} vision_slope_task_status_t;

/* --- 4. 对外变量与函数接口 --- */
extern volatile uint8 g_slope_vision_task_enable;
extern volatile uint8 g_slope_brake_ff_request;
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
