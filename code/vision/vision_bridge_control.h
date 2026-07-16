/*
 * =================================================================================
 * 文件: vision_bridge_control.h
 * 作用: 0 核 (Core 0) 桥梁任务视觉控制模块的参数配置与对外接口。
 * 说明: 这个文件定义了车子过桥时，状态机会用到的所有阈值（如速度、时间、偏差），
 *       以及当前任务处于什么状态（比如正在找桥、正在对齐、正在过桥等）。
 *       它是 0 核控制车子过桥的“参数说明书”。
 * =================================================================================
 */
#ifndef VISION_TASK_AREA_H
#define VISION_TASK_AREA_H

#include "zf_common_headfile.h"
#include "vision/vision_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- 1. 功能开关 --- */
#define VISION_BRIDGE_TASK_ENABLE                    (1)         /* 桥梁视觉任务总开关 */
#define VISION_BRIDGE_TASK_NAV_CORRECT_ENABLE        (0)         /* 惯导修正开关（下桥后是否用视觉来纠正惯导位置） */
#define VISION_BRIDGE_TASK_NAV_CORRECT_DISTANCE_MM   (3000.0f)   /* 纠正时假设桥的总长度是 3 米 */

/* --- 2. 状态机超时与计时参数 --- */
/* (注意：这些 TICKS 都是基于 2ms 中断的，所以 1000 TICKS = 2 秒) */
#define VISION_BRIDGE_TASK_ALIGN_TIMEOUT_TICKS       (1500U)     /* 对齐超时：3秒还没对齐好，强行上桥 */
#define VISION_BRIDGE_TASK_ALIGN_OK_TICKS            (60U)       /* 连续对齐好的帧数：大约 0.12秒 都稳定，认为对齐成功 */
#define VISION_BRIDGE_TASK_RUN_MIN_MM                (1000.0f)   /* 上桥后，至少跑 1 米才允许判定下桥（防误判） */
#define VISION_BRIDGE_TASK_RUN_MAX_MM                (3400.0f)   /* 桥上最多跑 3.4 米，跑到就强制下桥 */
#define VISION_BRIDGE_TASK_EXIT_BUFFER_MM            (300.0f)    /* 下桥后，再往前缓冲 30 厘米才算任务彻底结束 */
#define VISION_BRIDGE_TASK_EXIT_LOST_TICKS           (150U)      /* 连续 0.3 秒看不到桥，认为桥已经走完了 */
#define VISION_BRIDGE_TASK_BRIDGE_HOLD_TICKS         (220U)      /* 看到桥梁黑块后，保持“桥梁模式”0.44秒，防抖 */

/* --- 3. 角度与偏差阈值 --- */
#define VISION_BRIDGE_TASK_ALIGN_YAW_TOL_DEG         (3.0f)      /* 对齐时，车头偏角误差允许的范围（小于 4 度算对齐） */
#define VISION_BRIDGE_TASK_ALIGN_ERR_TOL_DEG         (1.5f)      /* 对齐时，综合误差（方向盘该打多少）允许的范围 */
#define VISION_BRIDGE_TASK_IMAGE_CENTER_X            (48.0f)    /* 车辆实际直行对应的图像中心；现场标定值 */
#define VISION_BRIDGE_TASK_LOOKAHEAD_Y               (40.0f)    /* 固定前视控制行，图像坐标由上向下增大 */

/* Control-side center-line temporal filter.  These parameters deliberately
 * live here instead of the detector so that the raw vision output remains
 * available to the rest of the system. */
#define VISION_BRIDGE_TASK_CENTER_FILTER_ALPHA       (0.50f)
#define VISION_BRIDGE_TASK_CENTER_JUMP_REJECT_PX     (8.0f)
#define VISION_BRIDGE_TASK_CENTER_JUMP_REJECT_DEG    (8.0f)
#define VISION_BRIDGE_TASK_CENTER_JUMP_CONFIRM_PX    (3.0f)
#define VISION_BRIDGE_TASK_CENTER_JUMP_CONFIRM_DEG   (3.0f)
#define VISION_BRIDGE_TASK_CENTER_LOST_FRAMES        (3U)

/* --- 4. 控制增益参数（PID 参数） --- */
#define VISION_BRIDGE_TASK_LINE_SIGN                 (-1.0f)     /* 转向符号，如果车子往反方向修偏，改成 1.0f */
#define VISION_BRIDGE_TASK_K_LAT_DEG_PER_PX          (0.18f)     /* 比例 P：横向每偏 1 个像素，方向盘打 0.18 度 */
#define VISION_BRIDGE_TASK_K_YAW_DEG_PER_DEG         (0.65f)     /* 比例 P：车头每偏 1 度，方向盘多打 0.65 度 */
#define VISION_BRIDGE_TASK_K_LAT_I_DEG_PER_PX_FRAME  (0.020f)    /* 每个有效视觉帧累积的横向补偿 */
#define VISION_BRIDGE_TASK_LAT_I_LEAK                (0.98f)     /* 防止积分长期饱和 */
#define VISION_BRIDGE_TASK_LAT_I_MAX_DEG             (5.0f)      /* 横向积分补偿限幅 */
#define VISION_BRIDGE_TASK_MAX_ERR_DEG               (16.0f)     /* 限幅：方向盘最多打 16 度，防打死 */
#define VISION_BRIDGE_TASK_YAW_HOLD_MAX_ERR_DEG      (10.0f)     /* 锁死航向盲跑时，最多修 10 度 */

/* --- 5. 各阶段速度与姿态设置 --- */
#define VISION_BRIDGE_TASK_ALIGN_SPEED_SET           (0.0f)      /* 对齐时：速度为 0（边停边对） */
#define VISION_BRIDGE_TASK_RUN_SPEED_SET             (-150.0f)   /* 桥上正常跑：速度 150 (负数表示前进) */
#define VISION_BRIDGE_TASK_BRIDGE_SPEED_SET          (-110.0f)   /* 看见黑块时：速度 110 */
#define VISION_BRIDGE_TASK_BLIND_SPEED_SET           (-90.0f)    /* 盲跑（看不清线和桥时）：速度 90，慢慢开 */
#define VISION_BRIDGE_TASK_EXIT_SPEED_SET            (-90.0f)    /* 下桥缓冲时：速度 90 */
#define VISION_BRIDGE_TASK_HEIGHT_STEP_SCALE         (0.10f)     /* 舵机升降的高度步进步长比例 */

/* --- 6. 数据结构定义 --- */

/**
 * @brief 桥梁任务的各个阶段（状态机）
 */
typedef enum
{
    VISION_BRIDGE_TASK_IDLE = 0,         /* 空闲：还没开始过桥 */
    VISION_BRIDGE_TASK_ALIGN,            /* 接近桥头并根据中心线对齐 */
    VISION_BRIDGE_TASK_RUN,              /* 上桥，在桥上跑 */
    VISION_BRIDGE_TASK_EXIT,             /* 下桥缓冲阶段 */
    VISION_BRIDGE_TASK_FINISH,           /* 完成：成功过桥，把控制权还给主程序 */
    VISION_BRIDGE_TASK_FAILSAFE,         /* 故障：出了问题，紧急放弃 */
} vision_bridge_task_state_e;

/**
 * @brief 桥梁任务运行时的状态信息（供监控或调试看）
 */
typedef struct
{
    uint8 enabled;                       /* 桥梁任务是否开启了 */
    vision_bridge_task_state_e state;    /* 当前处在哪个阶段 */
    uint32 state_ticks;                  /* 在这个阶段待了多久（计数） */
    uint32 last_seq;                     /* 收到的最后一包 1 核数据的序号 */
    float traveled_mm;                   /* 从上桥到现在跑了多远 */
    float err_degree_cmd;                /* 当前给方向盘下发的指令 */
    float speed_cmd;                     /* 当前给电机下发的速度指令 */
    uint8 bridge_stable;                 /* 1 核是否稳定检测到桥 */
    uint8 geometry_stable;               /* 1 核是否稳定得到可控的桥中心线 */
    uint8 geometry_valid;                /* 当前 IPC 中心线坐标是否有效 */
    uint8 bridge_state;                  /* BridgeDetectionState */
    int16 center_line_x0;
    int16 center_line_y0;
    int16 center_line_x1;
    int16 center_line_y1;
    uint8 center_filter_valid;
    uint8 center_filter_pending_jump;
    float filtered_lookahead_x;
    float filtered_heading_deg;
    float lateral_integral_deg;
    uint16 exit_lost_ticks;              /* 下桥时，连续看不到桥的计时 */
    uint16 bridge_hold_ticks;            /* 看见黑块后的保持倒计时 */
} vision_bridge_task_status_t;

/* --- 7. 外部变量与函数接口 --- */
extern volatile uint8 g_bridge_vision_task_enable;           /* 桥梁任务总开关 */
extern volatile vision_bridge_task_status_t g_bridge_vision_task_status; /* 任务状态大表 */

/**
 * @brief 初始化桥梁任务
 */
void VisionBridgeTask_Init(void);

/**
 * @brief 开始桥梁任务（从空闲进入准备阶段）
 */
void VisionBridgeTask_Start(void);

/**
 * @brief 强制停止桥梁任务
 */
void VisionBridgeTask_Stop(void);

/**
 * @brief 检查桥梁任务是否正在进行
 * @return 1: 正在进行; 0: 空闲
 */
uint8 VisionBridgeTask_IsActive(void);

/**
 * @brief 桥梁任务的定时更新（建议放在 2ms 定时中断里）
 * @note  这是控制车子过桥的核心大脑，会根据 1 核的数据和当前状态改变车速、打方向。
 */
void VisionBridgeTask_Update_2ms(void);

#ifdef __cplusplus
}
#endif

#endif
