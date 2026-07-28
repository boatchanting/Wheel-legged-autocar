/*
 * =================================================================================
 * 文件: vision_slope_control.h
 * 作用: 0 核(Core 0)斜坡路段视觉控制状态机的参数配置与对外接口。
 * 说明: 斜坡入口使用既有 PVC 视觉结果完成方向校准；进入斜坡后不再识别蓝色坡面，
 *       前 300ms 锁定惯导航向，之后方向误差清零并依靠定速行驶。
 *       脱出判据支持两种模式（SLOPE_IMPACT_DETECT_MODE 切换）：纯惯导里程退出，
 *       或基于 imu_acc_z 的 AP3 冲击检测退出（惯导里程降级为兜底）。
 *       算法来源: trials/slope-anlysis/STAIR_DETECTION_ALGORITHM.md
 *       落地方案: docs/任务规划/斜坡脱出冲击检测AP3落地方案.md
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
#define VISION_SLOPE_TASK_RUN_SPEED_SET              (-400.0f)   /* 进入斜坡 300ms 后的稳定行驶速度 */
#define VISION_SLOPE_TASK_PVC_ALIGN_SPEED_SET        (-200.0f)   /* 搜索并校准白色 PVC 斜坡入口时的低速 */
#define VISION_SLOPE_TASK_ENTRY_HOLD_TICKS           (150U)      /* 上坡后保持入口速度的时长：300ms */
#define VISION_SLOPE_TASK_EXIT_DISTANCE_MM           (2500.0f)   /* 从锁定航向开始累计 2500mm 后退出（仅 SLOPE_IMPACT_DETECT_MODE != 2 时生效） */

/* --- 2.5 斜坡脱出冲击检测参数（AP3 在线化，10ms 采样） --- */
/*
 * 两阶段落地用本宏区分：
 *   0: 关闭冲击检测，沿用纯惯导里程退出（旧行为）
 *   1: 仅检测记录（调试量写入 g_slope_impact_debug，仍按里程退出，不蜂鸣）
 *   2: 冲击检测作为脱出判据（累计 >= K 次冲击且安静期结束后退出并蜂鸣 3 声），惯导里程降级为兜底
 */
#define SLOPE_IMPACT_DETECT_MODE                     (2U)
#define SLOPE_IMPACT_SAMPLE_DIV                      (5U)        /* 2ms 调度 5 分频 → 10ms 采样（100Hz） */
#define SLOPE_IMPACT_BUF_LEN                         (320U)      /* accZ 环形缓冲长度，10ms*320 = 3.2s 窗口 */
#define SLOPE_IMPACT_K                               (3U)        /* 最少冲击次数（台阶数）：累计 >= K 次且之后进入安静期才判定脱出 */
#define SLOPE_IMPACT_ALPHA                           (1.5f)      /* 自适应阈值缩放系数：增大更保守（少检），减小更敏感（多检） */
#define SLOPE_IMPACT_PROM_MARGIN                     (2.5f)      /* Prominence 余量系数：prom >= T*MARGIN 才计入冲击，滤除缓坡段小起伏（6 组实测验证，假冲击比值 <= 2.1，真冲击簇内必有 >= 4.5） */
#define SLOPE_IMPACT_QUIET_TICKS                     (40U)       /* 安静期 40*10ms=400ms：末次冲击确认后平静这么久 -> 判定脱出（实测同级间隔最大 320ms，留 80ms 余量；对落地多次弹跳计数天然免疫） */
#define SLOPE_IMPACT_D_MIN                           (12U)       /* 最小冲击帧间距 12*10ms=120ms，防止同一冲击重复计数 */
#define SLOPE_IMPACT_MIN_SAMPLES                     (30U)       /* 缓冲至少积累 300ms 数据才开始判定（兼作布防延迟，滤除上坡闯动） */
#define SLOPE_IMPACT_FAILSAFE_DISTANCE_MM            (3500.0f)   /* 冲击检测模式下的惯导里程兜底退出（放宽原 2500mm） */
#define SLOPE_IMPACT_BEEP_TOTAL_TICKS                (300U)      /* 脱出蜂鸣总时长 300*2ms=600ms：响100ms/停100ms 共 3 声 */
#define SLOPE_IMPACT_BEEP_ON_TICKS                   (50U)       /* 每个 100tick 周期内前 50tick（100ms）鸣响 */
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
    VISION_SLOPE_TASK_RUN,              /* 蓝色坡面期间方向误差清零，以降低后的速度继续行驶 */
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
    uint32 last_seq;                     /* 最近一次收到的视觉 IPC 序号 */
    float traveled_mm;                   /* 从进入斜坡并锁角起累计的惯导距离 */
    float locked_yaw_deg;                /* 由 PVC 校准完成后锁定的惯导航向 */
    float err_degree_cmd;                /* 当前下发的方向误差指令 */
    float speed_cmd;                     /* 当前下发的目标速度指令 */
    uint8 pvc_stable_detected;           /* PVC 控制模块是否稳定检测到入口 */
    uint16 pvc_ratio_u16;                /* PVC 白色区域在画面内的占比，千分比 */
    int16 pvc_steer_error_px_x100;       /* PVC 给出的横向像素误差，放大 100 倍 */
} vision_slope_task_status_t;

/* --- 4. 对外变量与函数接口 --- */
extern volatile uint8 g_slope_vision_task_enable;
extern volatile vision_slope_task_status_t g_slope_vision_task_status;

/**
 * @brief 冲击检测调试状态，供遥测/屏幕查看。
 * @note 任务退出 cleanup 时不会被清空，便于事后分析最后一次任务的检测过程。
 */
typedef struct
{
    uint8 armed;                         /* 检测是否已布防（ENTRY_HOLD/RUN 阶段为 1） */
    uint8 impact_count;                  /* 已确认的冲击次数 */
    uint8 exit_detected;                 /* 第 K 次冲击已确认（脱出事件，锁存到下次布防） */
    uint8 exit_by_impact;                /* 上一次任务是否由冲击检测触发退出（1）还是里程退出（0） */
    float last_accz;                     /* 最近一次采样的垂直加速度 (mm/s^2) */
    float last_prominence;               /* 最近一次确认谷值的 Prominence */
    float threshold;                     /* 最近一次计算的自适应阈值 T */
    uint32 last_impact_frame;            /* 最近一次计入冲击的缓冲帧号 */
} slope_impact_debug_t;
extern volatile slope_impact_debug_t g_slope_impact_debug;

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
