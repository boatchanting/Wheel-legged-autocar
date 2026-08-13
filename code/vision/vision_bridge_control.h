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
#define VISION_BRIDGE_TASK_RUN_MIN_MM                (2300.0f)   /* 上桥后至少行驶 1m，才允许根据视觉出口线脱出 */
#define VISION_BRIDGE_TASK_VISUAL_CONTROL_DISTANCE_MM (1100.0f)  /* 上桥后仅前 1.2m 使用视觉方向控制 */
#define VISION_BRIDGE_TASK_LOCKED_SPEED_SCALE        (2.0f)      /* 超过视觉控制距离后，速度提高倍率 */
#define VISION_BRIDGE_TASK_BRIDGE_HOLD_TICKS         (220U)      /* 看到桥梁黑块后，保持“桥梁模式”0.44秒，防抖 */
#define VISION_BRIDGE_TASK_RUN_AUTO_EXIT_TICKS       (5000U)     /* 视觉长期异常时，10 秒后自动进入退出阶段，不停车等待 */
#define VISION_BRIDGE_TASK_EXIT_SETTLE_TICKS         (1000U)     /* 退出阶段最长 2 秒，随后自动交还导航 */

/* --- 3. 角度与偏差阈值 --- */
#define VISION_BRIDGE_TASK_ALIGN_YAW_TOL_DEG         (3.0f)      /* 对齐时，车头偏角误差允许的范围（小于 4 度算对齐） */
#define VISION_BRIDGE_TASK_ALIGN_ERR_TOL_DEG         (1.5f)      /* 对齐时，综合误差（方向盘该打多少）允许的范围 */
#define VISION_BRIDGE_TASK_IMAGE_CENTER_X            (47.0f)    /* 车辆实际直行对应的图像中心；现场标定值 */
#define VISION_BRIDGE_TASK_LOOKAHEAD_Y               (25U)       /* IPM 前视控制行，图像坐标由上向下增大 */

/* Control-side center-line temporal filter.  These parameters deliberately
 * live here instead of the detector so that the raw vision output remains
 * available to the rest of the system. */
#define VISION_BRIDGE_TASK_CENTER_FILTER_ALPHA       (0.40f)
#define VISION_BRIDGE_TASK_CENTER_JUMP_REJECT_PX     (8.0f)
#define VISION_BRIDGE_TASK_CENTER_JUMP_REJECT_DEG    (8.0f)
#define VISION_BRIDGE_TASK_CENTER_JUMP_CONFIRM_PX    (3.0f)
#define VISION_BRIDGE_TASK_CENTER_JUMP_CONFIRM_DEG   (3.0f)

/* --- 3.5 新管线 (b2_*) 专用参数 (C15) --- */
#define VISION_BRIDGE_TASK_ON_BRIDGE_TRIGGER_MM       (900.0f)  /* 上桥惯导门: 从交接点起 traveled 达此值进 RUN (桥入口→桥面起点+余量, 现场标定) */
#define VISION_BRIDGE_TASK_VALID_LOST_FRAMES          (8U)      /* b2_valid 失能连续帧数: 达到回锁角 (N, C09) */
#define VISION_BRIDGE_TASK_VALID_RECOVER_FRAMES       (4U)      /* b2_valid 恢复连续帧数: 达到回视觉 (M, C09) */
#define VISION_BRIDGE_TASK_ERR_RAMP_STEP_DEG          (0.5f)    /* 视觉↔锁角换源 ramp: 每 2ms 最多变化 (C10) */
/* --- 3.6 退出线视觉确认: exit_y>阈值 + 衰减累计 (简单版, 2026-08-09) ---
   旧单帧 y<10 无选择性 (远场 exit_y≈4 恒满足, 接近时反而爬升>10);
   复杂滤波(远场EMA/相对爬升/锁存/消失)过度设计——录像是手持的, 不具唯一性。
   简单版: exit_y>15 = 退出线进入近场带 (IPM 透视, 物理≈桥尾≤0.85m);
   衰减累计(+1/-1)≥3 tick 过滤单帧杂散, 容忍 has_top 抖动 (轻微递增感)。
   回放 230901: 8 次 FIRE 全命中真实接近(y=16~40), 远场 y≈4~11 零误触发。 */
#define VISION_BRIDGE_TASK_EXIT_Y_TH_PX               (15.0f)   /* 退出线下移带 (图像行) */
#define VISION_BRIDGE_TASK_EXIT_HOLD_TICKS            (3U)      /* 确认: 衰减累计 ≥N tick (2ms) */

/* --- 4. 转向指令参数 --- */
/* IPM 坐标为 X 向右、Y 向前；底层航向环的正方向与其相反，因此默认取 -1。
 * 若实车向反方向修正，仅修改此符号。 */
#define VISION_BRIDGE_TASK_LINE_SIGN                 (-1.0f)
#define VISION_BRIDGE_TASK_MAX_ERR_DEG               (16.0f)     /* 发送到底层航向环前的差角限幅 */
#define VISION_BRIDGE_TASK_YAW_HOLD_MAX_ERR_DEG      (10.0f)     /* 锁死航向盲跑时，最多修 10 度 */

/* --- 5. 各阶段速度与姿态设置 --- */
#define VISION_BRIDGE_TASK_ALIGN_SPEED_SET           (0.0f)      /* 对齐时：速度为 0（边停边对） */
#define VISION_BRIDGE_TASK_RUN_SPEED_SET             (-300.0f)   /* 桥上正常跑：速度 150 (负数表示前进) */
#define VISION_BRIDGE_TASK_BRIDGE_SPEED_SET          (-300.0f)   /* 看见黑块时：速度 110 */
#define VISION_BRIDGE_TASK_BLIND_SPEED_SET           (-300.0f)    /* 盲跑（看不清线和桥时）：速度 90，慢慢开 */
#define VISION_BRIDGE_TASK_EXIT_SPEED_SET            (-300.0f)    /* 下桥缓冲时：速度 90 */
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

typedef enum
{
    VISION_BRIDGE_EXIT_NONE = 0,
    VISION_BRIDGE_EXIT_VISUAL_CONFIRMED,
    VISION_BRIDGE_EXIT_AUTO_TIMEOUT
} vision_bridge_exit_reason_e;

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
    uint8 b2_valid;                      /* 仲裁后控制线原始可信 */
    uint8 b2_source;                     /* 0=红蓝中点 1=绿线 2=失能 */
    uint8 b2_mode;                       /* 原始 mode */
    uint8 b2_gate;                       /* 底部变白锁存 */
    uint8 b2_has_top;                    /* 退出线有效 */
    float exit_line_y;                   /* 退出线在图像中心列 x=47 处的行坐标(调试用, 无效为 -1) */
    uint8 center_filter_valid;
    uint8 center_filter_pending_jump;
    float filtered_lookahead_x;
    float filtered_heading_deg;          /* IPM 前视点相对标定直行方向的差角 */
    uint16 bridge_hold_ticks;            /* 看见黑块后的保持倒计时 */
} vision_bridge_task_status_t;

/* --- 7. 外部变量与函数接口 --- */
extern volatile uint8 g_bridge_vision_task_enable;           /* 桥梁任务总开关 */
extern volatile vision_bridge_task_status_t g_bridge_vision_task_status; /* 任务状态大表 */
extern volatile vision_bridge_exit_reason_e g_bridge_vision_task_exit_reason;
extern volatile uint8 g_bridge_exit_timeout_beep_request;    /* 兜底退出(AUTO_TIMEOUT)蜂鸣请求 (主循环响1声; 视觉确认用 exit_beep_request 响2声) */

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
