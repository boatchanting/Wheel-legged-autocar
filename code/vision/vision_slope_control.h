/*
 * =================================================================================
 * 文件: vision_slope_control.h
 * 作用: 0 核(Core 0)斜坡路段视觉控制状态机的参数配置与对外接口。
 * 说明: 斜坡入口使用既有 PVC 视觉结果完成方向校准；进入斜坡后不再识别蓝色坡面，
 *       前 300ms 锁定惯导航向，之后方向误差清零并依靠定速、惯导里程自动退出。
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
#define VISION_SLOPE_TASK_EXIT_DISTANCE_MM           (2500.0f)   /* 从锁定航向开始累计 2500mm 后退出 */
#define VISION_SLOPE_TASK_PVC_FULL_RATIO_U16          (400U)      /* 白色 PVC 占图比例达到 40% 时认为车辆已进入斜坡入口 */

/* PVC 稳定识别且白色区域占满入口后，连续保持 50ms 才允许锁角进入斜坡。 */
#define VISION_SLOPE_TASK_PVC_ALIGN_OK_TICKS          (25U)       /* 连续满足入口确认条件的时长：50ms */

/* --- 3. 航向锁定参数 --- */
#define VISION_SLOPE_TASK_YAW_HOLD_MAX_ERR_DEG       (10.0f)     /* 锁定航向时发送到底层前的最大修正角 */

/* --- 3b. 进入段物理域 PID 方向修正参数（2026-08-20 轻量方案，无 LQR） ---
 * 控制律: ω = P·ψ_err + D·ψ_err' + K_E·e，e = D·sin(β−ψ_err)
 *   - P·ψ_err:  航向保持(基准=进入任务时刻 entry_yaw)，保证"基本正开上去"
 *   - K_E·e:    视觉横向偏差修正(物理域, 距离归一化)，保证轮子对准桥面
 *   - D·ψ_err': 航向微分阻尼(2ms差分+一阶低通)
 * 输出: err_degree = ω·57.29578/TURN_ANG_KP（角度环输出恒=ω，与底层解耦）
 * 门控: phy 无效 或 D>1.5m → 无效(直行回退)
 * 验收: 上桥瞬间 |x|≤11.5cm 且 |ψ|≤20°，进入后 30cm 内转正
 * 参数起点: 参考 12/8/0.2（MuJoCo 物理仿真验证）
 */
#define VISION_SLOPE_ENTRY_PID_ENABLE           (1U)    /* 1=进入段用物理域PID方向; 0=回退原PVC像素域err */
#define VISION_SLOPE_ENTRY_PID_K_E              (12.0f) /* 视觉横向偏差增益(主旋钮, 参考12/8/0.2起点) */
#define VISION_SLOPE_ENTRY_PID_P_PSI            (8.0f)  /* 航向保持增益(次旋钮, 保证基本正, 勿调小) */
#define VISION_SLOPE_ENTRY_PID_D_PSI            (0.2f)  /* 航向微分阻尼(甜点0~0.5, 严禁>0.5×P) */
#define VISION_SLOPE_ENTRY_PID_TAU_S            (0.05f) /* 微分一阶低通时间常数(s), 防IMU噪声 */
#define VISION_SLOPE_ENTRY_PID_CTRL_HZ          (500.0f)/* 控制周期2ms, 微分分母(若改周期需同步) */
#define VISION_SLOPE_ENTRY_PID_W_MAX_RADPS      (2.2f)  /* ω限幅(执行器极限, 换硬件才改) */
#define VISION_SLOPE_ENTRY_PID_DETECT_RANGE_M   (1.5f)  /* 视觉段检测距离(现场标定) */
#define VISION_SLOPE_ENTRY_PID_PHY_INVALID_MM   (32767) /* IPM无效标记(与PVC_VISION_PHY_INVALID_MM一致) */

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
    float entry_e_m;                     /* 进入段PID 横向偏差 e (m, 诊断) */
    float entry_psi_err_deg;             /* 进入段PID 航向偏差 ψ_err (deg, 诊断) */
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
