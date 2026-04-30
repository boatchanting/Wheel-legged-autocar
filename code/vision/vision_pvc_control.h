/*
 * =================================================================================
 * 文件: vision_pvc_control.h
 * 作用: 0 核 (Core 0) PVC 入口控制模块的配置、状态定义与接口声明。
 * 说明: 当车子快到 PVC 区域时，这个模块会接管方向盘和油门。
 *       它会根据 1 核传过来的 PVC 图像数据，让车子减速、对准入口、并最终停在里面。
 * =================================================================================
 */
#ifndef VISION_PVC_CONTROL_H
#define VISION_PVC_CONTROL_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"
#include "vision/vision_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * =================================================================================
 * 【新手必读：0 核 PVC 控制模块说明】
 * =================================================================================
 *
 * 工作原理：
 * - 1 核负责“看”：找出 PVC 入口在哪里，算出一个数据包 (g_vision_ipc_latest)。
 * - 0 核的这个模块负责“想”：读取 1 核的数据，决定方向盘打多少度 (err_degree)，速度定多少 (target_speed_set)。
 *
 * 过 PVC 的几个阶段：
 * 1. 搜索 (SEARCH)：还没看清 PVC 在哪，慢慢往前开，边走边找。
 * 2. 跟踪 (TRACK)：看清了，朝着 PVC 的方向正常开过去。
 * 3. 接近 (CLOSE)：离 PVC 很近了，赶紧踩刹车减速，防止冲过头。
 * 4. 到达 (ARRIVED)：进到 PVC 里面了（或者离得特别近），停车！
 *
 * 怎么使用它：
 * - 当小车的惯导算出来“我已经离某个入口不到 800mm 了”，主程序就调用 `VisionPvcControl_SetEnable(1)` 开启它。
 * - 等车子停稳，准备做后续动作时，调用 `VisionPvcControl_SetEnable(0)` 关掉它，把控制权还给主程序。
 * =================================================================================
 */

/* --- 1. 功能开关 --- */
#define VISION_PVC_CONTROL_ENABLE                 (1)     /* 模块总开关：1 为开启编译，0 为关闭 */

/*
 * 检测默认常开：
 * - 这是给 1 核下的命令。1 表示让 1 核一开机就一直找 PVC。
 * - 测试的时候一般常开，这样可以在电脑上随时看 1 核找得准不准。
 */
#define VISION_PVC_DETECT_DEFAULT_ACTIVE          (1)

/*
 * 控制默认关闭：
 * - 这个开关决定 0 核要不要把算出来的方向和速度真的下发给车子。
 * - 默认关闭，防止车子一开机就乱跑。等主程序觉得该过 PVC 了再打开。
 */
#define VISION_PVC_CONTROL_DEFAULT_ACTIVE         (0)

#define VISION_PVC_CONTROL_PROFILE_ENABLE         (1)     /* 性能统计开关：看 0 核算一次要多久 */
#define VISION_PVC_CONTROL_PROFILE_TIMER          (TC_TIME2_CH0) /* 测算时间用的定时器 */

/* --- 2. 核心控制参数（调车时常改的地方） --- */

/* 如果发现车子明明看到 PVC 在右边，却往左边猛打方向，就把 1.0f 改成 -1.0f */
#define VISION_PVC_CONTROL_LATERAL_SIGN           (-1.0f)

/* 照片的尺寸，必须和 1 核保持一致 */
#define VISION_PVC_CONTROL_IMAGE_W                (94U)
#define VISION_PVC_CONTROL_IMAGE_H                (60U)
#define VISION_PVC_CONTROL_IMAGE_AREA             (VISION_PVC_CONTROL_IMAGE_W * VISION_PVC_CONTROL_IMAGE_H)

/*
 * 【各阶段的速度设置】(负数代表往前走，因为这台车的电机方向可能反了)
 */
#define VISION_PVC_CONTROL_SEARCH_SPEED_SET       (-35.0f)  /* 搜索阶段：没看清时，龟速 35 往前挪 */
#define VISION_PVC_CONTROL_TRACK_SPEED_SET        (-200.0f) /* 跟踪阶段：看清了且还很远，速度 200 冲过去 */
#define VISION_PVC_CONTROL_CLOSE_SPEED_SET        (-80.0f)  /* 接近阶段：快到了，速度降到 80 */
#define VISION_PVC_CONTROL_ARRIVE_SPEED_SET       (0.0f)    /* 到达阶段：到了，速度 0 停车 */

/*
 * 【距离的门槛设置】(单位：毫米)
 * 注意：这些距离是 1 核“估算”出来的，不一定准。要根据实际跑的情况微调。
 */
#define VISION_PVC_CONTROL_CLOSE_FORWARD_MM       (320)   /* 小于 32 厘米，认为“接近了”，开始减速 */
#define VISION_PVC_CONTROL_ARRIVE_FORWARD_MM      (140)   /* 小于 14 厘米，认为“到了”，停车 */
#define VISION_PVC_CONTROL_STALE_TIMEOUT_TICKS    (100U)  /* 数据过期时间：200毫秒没收到 1 核的新数据，就认为瞎了 */

/*
 * 【备用的停车条件：看画面比例】
 * - 900 表示画面里 90% 都是 PVC 白板。
 * - 如果距离估算不准，但车已经压到 PVC 上，画面全白了，也能靠这个条件停车。
 * - 停车太早改大（如 950），冲过头了改小（如 800）。
 */
#define VISION_PVC_CONTROL_STOP_BBOX_RATIO_U16    (900U)

/*
 * 【方向盘 PID 参数】
 * 公式：打角 = 横向偏差 * 比例(0.20) + 角度偏差 * 比例(0.50)
 */
#define VISION_PVC_CONTROL_K_LAT_DEG_PER_MM       (0.20f) /* 车子偏了 1 毫米，方向盘打 0.20 度 */
#define VISION_PVC_CONTROL_K_YAW_DEG_PER_DEG      (0.50f) /* 车头偏了 1 度，方向盘多打 0.50 度 */
#define VISION_PVC_CONTROL_MAX_ERR_DEG            (18.0f) /* 方向盘最多打 18 度，防止打死翻车 */

/* --- 3. 数据结构定义 --- */

/**
 * @brief PVC 控制的状态机
 */
typedef enum
{
    VISION_PVC_CTRL_IDLE = 0,   /* 空闲：没在过 PVC */
    VISION_PVC_CTRL_SEARCH,     /* 搜索：没看清，慢慢找 */
    VISION_PVC_CTRL_TRACK,      /* 跟踪：看清了，快点走 */
    VISION_PVC_CTRL_ARRIVED,    /* 到达：停在里面了 */
    VISION_PVC_CTRL_STALE,      /* 数据过期：1 核死机或者通信断了 */
} vision_pvc_control_state_e;

/**
 * @brief 记录当前控制状态的“仪表盘”
 */
typedef struct
{
    uint8 enabled;               /* 0核有没有在用这些数据控车？ */
    uint8 has_new_packet;        /* 刚刚有没有收到新照片？ */
    uint8 stable_detected;       /* 1核是不是稳定看到 PVC 了？ */
    uint8 raw_detected;          /* 1核这一瞬间看到 PVC 了吗？ */
    uint32 last_seq;             /* 上一张照片的编号 */
    uint16 stale_ticks;          /* 多少次没收到新照片了 */
    vision_pvc_control_state_e state; /* 现在处于什么阶段？ */
    int16 forward_mm;            /* 离入口还有多远 */
    int16 lateral_mm;            /* 偏离中心多少 */
    int16 yaw_error_deg_x100;    /* 角度偏了多少 */
    uint16 bbox_area_ratio_u16;  /* 画面里 PVC 占了多少（千分比） */
    float err_degree_cmd;        /* 当前准备给方向盘下发的指令 */
    float speed_cmd;             /* 当前准备给电机下发的速度指令 */
} vision_pvc_control_status_t;

/* --- 4. 对外公开的全局变量与函数 --- */

extern volatile vision_pvc_control_status_t g_vision_pvc_control_status; /* 仪表盘 */
extern volatile runtime_profiler_t g_vision_pvc_control_profiler;        /* 测速器 */

/**
 * @brief PVC 控制开关（测试专用）
 * @note 
 *   如果为 0：1 核继续找 PVC，但 0 核不理它，不控车。
 *   如果为 1：0 核接管车子，让车子自动开进 PVC 里。
 */
extern volatile uint8 g_pvc_control_enable;

/**
 * @brief 初始化 PVC 控制模块
 */
void VisionPvcControl_Init(void);

/**
 * @brief 开启或关闭 PVC 控制
 * @param enable 1:开启接管车子; 0:关闭接管
 */
void VisionPvcControl_SetEnable(uint8 enable);

/**
 * @brief 看看现在是不是在接管车子
 * @return 1:接管中; 0:没接管
 */
uint8 VisionPvcControl_IsEnabled(void);

/**
 * @brief 控制模块的心脏（每 2ms 调用一次）
 * @note  它会读取 1 核的数据，算出新的方向和速度，发给底盘。
 */
void VisionPvcControl_Update_2ms(void);

#ifdef __cplusplus
}
#endif

#endif
