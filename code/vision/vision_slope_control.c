/*
 * =================================================================================
 * 文件: vision_slope_control.c
 * 作用: 0 核(Core 0)斜坡路段视觉识别与控制状态机。
 * 说明: 白色 PVC 斜坡入口阶段复用现有 PVC 控制模块修正方向；检测稳定后，
 *       立即锁定当前惯导航向。蓝色坡面不做视觉识别，前 300ms 后方向误差清零。
 *       脱出判据由 SLOPE_IMPACT_DETECT_MODE 切换：纯惯导里程退出，
 *       或基于 imu_acc_z 的 AP3 冲击检测退出（累计 >= K 次着地冲击且
 *       安静期 400ms 结束 = 脱出时间点，惯导里程降级为兜底，冲击脱出时蜂鸣 3 声）。
 *       算法来源: trials/slope-anlysis/STAIR_DETECTION_ALGORITHM.md
 *       落地方案: docs/任务规划/斜坡脱出冲击检测AP3落地方案.md
 * =================================================================================
 */
#include "vision/vision_slope_control.h"
#include "vision/vision_ipc_core0.h"
#include "vision/vision_pvc_control.h"
#include "navigation/inertial_nav.h"
#include "calculate/ekf.h"
#include "tools/sbus.h"
#include "tools/beep.h"

#if VISION_SLOPE_TASK_ENABLE

/* --- 外部变量引用 --- */
/* 这些变量由底层控制主循环维护；本状态机只在激活期间写入目标指令。 */
extern volatile float err_degree;
extern volatile float target_speed_set;
extern int g_motor_enable;
extern uint8 g_special_action_trigger;

/* --- 全局状态 --- */
volatile uint8 g_slope_vision_task_enable = 0U;
volatile vision_slope_task_status_t g_slope_vision_task_status = {0};
volatile slope_impact_debug_t g_slope_impact_debug = {0};

/* --- 内部状态 --- */
typedef struct
{
    vision_slope_task_state_e state;     /* 当前状态 */
    uint32 state_ticks;                   /* 当前状态的 2ms 计时 */
    float start_x_mm;                     /* 锁角进入斜坡瞬间的惯导 X 坐标 */
    float start_y_mm;                     /* 锁角进入斜坡瞬间的惯导 Y 坐标 */
    float locked_yaw_deg;                 /* PVC 校准完成后锁定的惯导航向 */
    uint16 pvc_align_ok_ticks;            /* PVC 入口确认条件连续满足的 2ms tick 数 */
} vision_slope_task_ctx_t;

static vision_slope_task_ctx_t s_slope_task;

#if SLOPE_IMPACT_DETECT_MODE != 0
/* ==================== AP3 斜坡脱出冲击检测（在线化） ==================== */
/*
 * 10ms 采样 imu 垂直加速度（不加重力补偿，与离线验证数据形状一致；
 * Prominence 为相对量，对直流偏置与线性缩放天然免疫）。
 * 流程：局部谷值 → 右边界确认 → Prominence → 自适应阈值(P90-median)*alpha
 *       → MARGIN 余量筛选 + 最小帧间距计数 → 累计 >= K 次且安静期结束 → 锁存"脱出事件"。
 * 仅在 ENTRY_HOLD/RUN 阶段布防，其余场景（颠簸路/跳跃等）的冲击不进入检测窗口。
 */
#define SLOPE_IMPACT_PENDING_MAX    (32U)   /* 待确认谷值队列容量 */

typedef struct
{
    float  buf[SLOPE_IMPACT_BUF_LEN];       /* accZ 环形缓冲 (mm/s^2)，按下标 frame % LEN 存取 */
    uint32 total;                           /* 已写入样本总数（单调递增，即全局帧号） */
    uint8  sample_div;                      /* 2ms -> 10ms 分频计数 */
    uint8  armed;                           /* 布防标志 */
    uint8  impact_count;                    /* 已确认冲击次数 */
    uint8  exit_detected;                   /* 脱出事件锁存 */
    uint32 last_impact_frame;               /* 最近一次计入冲击的谷值全局帧号 */
    uint32 last_confirm_frame;              /* 最近一次冲击被确认时的全局帧号（安静期从此时起算） */
    float  last_prominence;                 /* 最近一次确认谷值的 Prominence */
    float  threshold;                       /* 最近一次自适应阈值 T */
    uint32 pending[SLOPE_IMPACT_PENDING_MAX]; /* 待确认谷值的全局帧号（线性队列，队首最旧） */
    uint8  pending_count;
} slope_impact_detector_t;

static slope_impact_detector_t s_slope_impact;

/**
 * @brief 清空检测缓冲与计数；布防/初始化时调用。不清 exit_by_impact 以外的历史结论字段。
 */
static void slope_impact_reset(void)
{
    memset(&s_slope_impact, 0, sizeof(s_slope_impact));
    g_slope_impact_debug.armed = 0U;
    g_slope_impact_debug.impact_count = 0U;
    g_slope_impact_debug.exit_detected = 0U;
    g_slope_impact_debug.exit_by_impact = 0U;
    g_slope_impact_debug.last_accz = 0.0f;
    g_slope_impact_debug.last_prominence = 0.0f;
    g_slope_impact_debug.threshold = 0.0f;
    g_slope_impact_debug.last_impact_frame = 0U;
}

/**
 * @brief 谷值确认：右边界出现后计算 Prominence，过自适应阈值且满足帧间距则计入冲击。
 * @param v         谷值全局帧号
 * @param right_val 右边界样本值（首个高于谷底的样本）
 */
static void slope_impact_confirm_valley(uint32 v, float right_val)
{
    const float *buf = s_slope_impact.buf;
    const float av = buf[v % SLOPE_IMPACT_BUF_LEN];
    uint32 oldest;      /* 缓冲内最旧有效帧号 */
    uint32 l;           /* 左边界帧号 */
    uint32 j;
    uint32 n_valid;
    uint32 i;
    float prom;
    float median_abs;
    float p90_abs;
    float thr;
    static float s_sort_tmp[SLOPE_IMPACT_BUF_LEN];  /* 阈值统计用的临时数组（仅本函数使用） */

    /* 缓冲内最旧有效帧：total 已在写入当前样本后递增 */
    oldest = (s_slope_impact.total > SLOPE_IMPACT_BUF_LEN) ?
             (s_slope_impact.total - SLOPE_IMPACT_BUF_LEN) : 0U;

    /* 左边界：从 v 向左找首个高于谷底的点；找不到则取最旧有效帧（对应算法文档的 l=0 回退） */
    l = oldest;
    for (j = v; j-- > oldest; )
    {
        l = j;
        if (buf[j % SLOPE_IMPACT_BUF_LEN] > av)
        {
            break;
        }
    }

    /* Prominence = min(左山脊, 右山脊) - 谷底 */
    prom = fminf(buf[l % SLOPE_IMPACT_BUF_LEN], right_val) - av;
    s_slope_impact.last_prominence = prom;
    g_slope_impact_debug.last_prominence = prom;

    /* 自适应阈值：T = (P90(|a|) - median(|a|)) * alpha，按缓冲内有效样本统计 */
    n_valid = (s_slope_impact.total < SLOPE_IMPACT_BUF_LEN) ?
              s_slope_impact.total : SLOPE_IMPACT_BUF_LEN;
    for (i = 0U; i < n_valid; i++)
    {
        s_sort_tmp[i] = fabsf(buf[(oldest + i) % SLOPE_IMPACT_BUF_LEN]);
    }
    /* 插入排序（n<=320，仅在谷值确认时执行，开销可忽略） */
    for (i = 1U; i < n_valid; i++)
    {
        const float key = s_sort_tmp[i];
        uint32 k = i;
        while ((k > 0U) && (s_sort_tmp[k - 1U] > key))
        {
            s_sort_tmp[k] = s_sort_tmp[k - 1U];
            k--;
        }
        s_sort_tmp[k] = key;
    }
    median_abs = s_sort_tmp[n_valid / 2U];
    p90_abs = s_sort_tmp[(uint32)((float)n_valid * 0.9f)];
    thr = (p90_abs - median_abs) * SLOPE_IMPACT_ALPHA;
    s_slope_impact.threshold = thr;
    g_slope_impact_debug.threshold = thr;

    /* 筛选 + 计数：缓冲未攒够布防样本数时只更新调试量，不计冲击；
     * thr<=0 属退化情形（数据完全平坦或谷值贴缓冲边缘 prom=0），此时判定不可靠，同样不计；
     * MARGIN 余量：实测下台阶过程中缓坡段小起伏的 prom/T 比值 <= 2.1，
     * 真实着地冲击簇内必有比值 >= 4.5 的谷值，取 2.5 分隔。 */
    if ((n_valid < SLOPE_IMPACT_MIN_SAMPLES) || (thr <= 0.0f) ||
        (prom < thr * SLOPE_IMPACT_PROM_MARGIN))
    {
        return;
    }
    /* 帧号单调递增，与最近一次计入的冲击比较即完成非极大抑制 */
    if ((s_slope_impact.impact_count != 0U) &&
        ((v - s_slope_impact.last_impact_frame) < SLOPE_IMPACT_D_MIN))
    {
        return;
    }

    s_slope_impact.impact_count++;
    s_slope_impact.last_impact_frame = v;
    s_slope_impact.last_confirm_frame = s_slope_impact.total - 1U;
    g_slope_impact_debug.impact_count = s_slope_impact.impact_count;
    g_slope_impact_debug.last_impact_frame = v;
    /* 注意：此处不置 exit_detected。单次落地会弹跳产生多次计数，
     * 脱出判定由 slope_impact_feed 末尾的"够 K 次 + 安静期"统一完成。 */
}

/**
 * @brief 10ms 采样入口：写入缓冲、检出局部谷值、确认右边界。
 */
static void slope_impact_feed(float accz)
{
    float *buf = s_slope_impact.buf;
    const uint32 t = s_slope_impact.total;  /* 当前样本的全局帧号 */
    uint8 i;

    buf[t % SLOPE_IMPACT_BUF_LEN] = accz;
    s_slope_impact.total = t + 1U;
    g_slope_impact_debug.last_accz = accz;

    /* 1. 局部谷值判定：a[t-1] < a[t-2] 且 a[t-1] <= a[t] */
    if ((t >= 2U) &&
        (buf[(t - 1U) % SLOPE_IMPACT_BUF_LEN] < buf[(t - 2U) % SLOPE_IMPACT_BUF_LEN]) &&
        (buf[(t - 1U) % SLOPE_IMPACT_BUF_LEN] <= accz))
    {
        if (s_slope_impact.pending_count < SLOPE_IMPACT_PENDING_MAX)
        {
            s_slope_impact.pending[s_slope_impact.pending_count] = t - 1U;
            s_slope_impact.pending_count++;
        }
    }

    /* 2. 剔除已滑出缓冲窗口的陈旧谷值（队首最旧，帧号递增） */
    while ((s_slope_impact.pending_count > 0U) &&
           (s_slope_impact.pending[0] + SLOPE_IMPACT_BUF_LEN <= s_slope_impact.total))
    {
        memmove(&s_slope_impact.pending[0], &s_slope_impact.pending[1],
                (s_slope_impact.pending_count - 1U) * sizeof(uint32));
        s_slope_impact.pending_count--;
    }

    /* 3. 右边界确认：当前样本高于谷值 → 该谷值确认（全队列扫描，允许乱序确认） */
    i = 0U;
    while (i < s_slope_impact.pending_count)
    {
        const uint32 v = s_slope_impact.pending[i];
        if (accz > buf[v % SLOPE_IMPACT_BUF_LEN])
        {
            slope_impact_confirm_valley(v, accz);
            memmove(&s_slope_impact.pending[i], &s_slope_impact.pending[i + 1U],
                    (s_slope_impact.pending_count - i - 1U) * sizeof(uint32));
            s_slope_impact.pending_count--;
        }
        else
        {
            i++;
        }
    }

    /* 4. 安静期脱出判定：冲击够 K 次，且距最后一次冲击确认已平静 QUIET_TICKS。
     * 脱出时间点 = 末次冲击 + 400ms 安静期；落地弹跳产生的多余计数只会推迟
     * 安静期起点，不会造成提前脱出（实测同级台阶间隔 <= 320ms < 400ms）。 */
    if ((s_slope_impact.exit_detected == 0U) &&
        (s_slope_impact.impact_count >= SLOPE_IMPACT_K) &&
        ((t - s_slope_impact.last_confirm_frame) >= SLOPE_IMPACT_QUIET_TICKS))
    {
        s_slope_impact.exit_detected = 1U;
        g_slope_impact_debug.exit_detected = 1U;
    }
}
#endif /* SLOPE_IMPACT_DETECT_MODE != 0 */

/* --- 脱出蜂鸣（2ms 节拍驱动）：响 100ms / 停 100ms，共 3 声，总时长 600ms --- */
static uint16 s_slope_beep_ticks = 0U;      /* 蜂鸣时序剩余 tick；0 表示空闲 */

static void vision_slope_beep3_start(void)
{
    s_slope_beep_ticks = SLOPE_IMPACT_BEEP_TOTAL_TICKS;
}

static void vision_slope_beep_update(void)
{
    uint16 phase;

    if (s_slope_beep_ticks == 0U)
    {
        return;
    }
    s_slope_beep_ticks--;
    if (s_slope_beep_ticks == 0U)
    {
        gpio_set_level(BUZZER_PIN, GPIO_LOW);
        return;
    }
    /* 每个周期 2*ON_TICKS(200ms)，前 ON_TICKS(100ms) 鸣响 */
    phase = (uint16)((SLOPE_IMPACT_BEEP_TOTAL_TICKS - s_slope_beep_ticks) %
                     (2U * SLOPE_IMPACT_BEEP_ON_TICKS));
    gpio_set_level(BUZZER_PIN, (phase < SLOPE_IMPACT_BEEP_ON_TICKS) ? GPIO_HIGH : GPIO_LOW);
}

/* --- 基础数学工具函数 --- */

/**
 * @brief 将角度归一化到 -180 至 180 度，保证跨越正负 180 度时不会反向猛打方向。
 */
static float vision_slope_normalize_angle(float angle_deg)
{
    while (angle_deg > 180.0f)
    {
        angle_deg -= 360.0f;
    }
    while (angle_deg < -180.0f)
    {
        angle_deg += 360.0f;
    }
    return angle_deg;
}

/**
 * @brief 将数值限制在给定范围，防止锁角误差过大时方向输出突变。
 */
static float vision_slope_constrain_f(float value, float min_value, float max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

/**
 * @brief 计算车辆距锁角起点的惯导平面距离，单位为 mm。
 */
static float vision_slope_distance_from(float x_mm, float y_mm)
{
    const float dx = inertial_nav.x - x_mm;
    const float dy = inertial_nav.y - y_mm;
    return sqrtf(dx * dx + dy * dy);
}

/**
 * @brief 计算锁定航向所需的方向误差，并限制到安全输出范围。
 */
static float vision_slope_calc_yaw_hold_err(void)
{
    const float yaw_error = vision_slope_normalize_angle(
        s_slope_task.locked_yaw_deg - inertial_nav.relative_yaw);
    return vision_slope_constrain_f(yaw_error,
                                    -VISION_SLOPE_TASK_YAW_HOLD_MAX_ERR_DEG,
                                    VISION_SLOPE_TASK_YAW_HOLD_MAX_ERR_DEG);
}

/**
 * @brief 切换状态并从零开始计时，避免不同阶段复用旧计时值。
 */
static void vision_slope_set_state(vision_slope_task_state_e next_state)
{
    s_slope_task.state = next_state;
    s_slope_task.state_ticks = 0U;

    /* PVC 入口确认完成的瞬间记录航向和位置，后续里程从这里开始累计。 */
    if (next_state == VISION_SLOPE_TASK_ENTRY_HOLD)
    {
        s_slope_task.start_x_mm = inertial_nav.x;
        s_slope_task.start_y_mm = inertial_nav.y;
        s_slope_task.locked_yaw_deg = inertial_nav.relative_yaw;
#if SLOPE_IMPACT_DETECT_MODE != 0
        /* 锁角上坡即布防冲击检测：缓冲随 ENTRY_HOLD 开始积累基线，
         * SLOPE_IMPACT_MIN_SAMPLES 兼作布防延迟，滤除刚压上斜坡的闯动。 */
        slope_impact_reset();
        s_slope_impact.armed = 1U;
        g_slope_impact_debug.armed = 1U;
#endif
    }
}

/**
 * @brief 将内部状态一次性同步到全局状态，便于调试读取。
 */
static void vision_slope_publish_status(const volatile vision_ipc_packet_t *packet,
                                        float traveled_mm,
                                        float err_cmd,
                                        float speed_cmd)
{
    g_slope_vision_task_status.enabled = g_slope_vision_task_enable;
    g_slope_vision_task_status.state = s_slope_task.state;
    g_slope_vision_task_status.state_ticks = s_slope_task.state_ticks;
    g_slope_vision_task_status.last_seq = (packet != NULL) ? packet->seq : 0U;
    g_slope_vision_task_status.traveled_mm = traveled_mm;
    g_slope_vision_task_status.locked_yaw_deg = s_slope_task.locked_yaw_deg;
    g_slope_vision_task_status.err_degree_cmd = err_cmd;
    g_slope_vision_task_status.speed_cmd = speed_cmd;
    g_slope_vision_task_status.pvc_stable_detected = g_vision_pvc_control_status.stable_detected;
    g_slope_vision_task_status.pvc_ratio_u16 = g_vision_pvc_control_status.bbox_area_ratio_u16;
    g_slope_vision_task_status.pvc_steer_error_px_x100 = g_vision_pvc_control_status.steer_error_px_x100;
}

/**
 * @brief 释放 PVC 视觉控制与特殊任务标记；stop_car 为 1 时同时输出停车指令。
 */
static void vision_slope_cleanup(uint8 stop_car)
{
    /* 释放 PVC 控制，并恢复 PVC 检测的项目默认调度状态，保证下次任务可重新触发。 */
    VisionPvcControl_SetEnable(0U);
    VisionIpc_Core0_SetPvcEnable(VISION_PVC_DETECT_DEFAULT_ACTIVE);

    err_degree = 0.0f;
    if (stop_car != 0U)
    {
        target_speed_set = 0.0f;
    }

#if SLOPE_IMPACT_DETECT_MODE != 0
    /* 撤防冲击检测；调试结论字段（impact_count/exit_detected/exit_by_impact 等）保留供事后分析。 */
    s_slope_impact.armed = 0U;
    g_slope_impact_debug.armed = 0U;
#endif

    g_special_action_trigger = 0U;
    g_slope_vision_task_enable = 0U;
    memset(&s_slope_task, 0, sizeof(s_slope_task));
    memset((void *)&g_slope_vision_task_status, 0, sizeof(g_slope_vision_task_status));
    g_slope_vision_task_status.state = VISION_SLOPE_TASK_IDLE;
}

/**
 * @brief 进入 PVC 校准阶段：先启动既有 PVC 检测/方向控制，再由斜坡任务覆盖速度。
 */
static void vision_slope_enter_task(void)
{
    memset(&s_slope_task, 0, sizeof(s_slope_task));
    s_slope_task.state = VISION_SLOPE_TASK_PVC_ALIGN;
    s_slope_task.locked_yaw_deg = inertial_nav.relative_yaw;
    g_special_action_trigger = 1U;

    /* PVC 控制模块会提供方向误差；本状态机在本周期末统一强制入口速度。 */
    VisionIpc_Core0_SetPvcEnable(1U);
    VisionPvcControl_SetEnable(1U);
}

/* --- 对外接口函数 --- */

void VisionSlopeTask_Init(void)
{
    memset(&s_slope_task, 0, sizeof(s_slope_task));
    memset((void *)&g_slope_vision_task_status, 0, sizeof(g_slope_vision_task_status));
    g_slope_vision_task_enable = 0U;
    s_slope_beep_ticks = 0U;
#if SLOPE_IMPACT_DETECT_MODE != 0
    slope_impact_reset();
#endif
}

void VisionSlopeTask_Start(void)
{
    g_slope_vision_task_enable = 1U;
}

void VisionSlopeTask_Stop(void)
{
    vision_slope_cleanup(1U);
}

uint8 VisionSlopeTask_IsActive(void)
{
    return (uint8)((g_slope_vision_task_enable != 0U) ||
                   (s_slope_task.state != VISION_SLOPE_TASK_IDLE));
}

/**
 * @brief 斜坡任务的核心状态机，每 2ms 执行一次。
 *
 * 流程：PVC 白色入口校准 -> 锁定当前航向 -> -600 行驶 300ms -> -400 锁角行驶 ->
 *       SLOPE_IMPACT_DETECT_MODE==2 时由 AP3 冲击检测判定脱出（里程兜底），否则 2.5m 里程退出。
 */
void VisionSlopeTask_Update_2ms(void)
{
    const volatile vision_ipc_packet_t *packet = VisionIpc_Core0_GetLatest();
    float traveled_mm = 0.0f;
    float err_cmd = 0.0f;
    float speed_cmd = 0.0f;

    /* 蜂鸣时序优先服务：任务退出 cleanup 后仍需把 3 声蜂鸣放完。 */
    vision_slope_beep_update();

    if ((g_slope_vision_task_enable == 0U) &&
        (s_slope_task.state == VISION_SLOPE_TASK_IDLE))
    {
        return;
    }

    /* 急停、未使能电机或惯导尚未完成航向初始化时，立即安全退出。 */
#if REMOTE_CONTROL == 1
    if ((g_motor_enable == 0) || (g_yaw_initialized == 0U) || (robot_ctrl.brake_active == 1U))
    {
        vision_slope_cleanup(1U);
        return;
    }
#endif
#if REMOTE_CONTROL == 0
    if ((g_motor_enable == 0) || (g_yaw_initialized == 0U))
    {
        vision_slope_cleanup(1U);
        return;
    }
#endif

    if (s_slope_task.state == VISION_SLOPE_TASK_IDLE)
    {
        vision_slope_enter_task();
    }

    if (s_slope_task.state_ticks < 0xFFFFFFFFU)
    {
        s_slope_task.state_ticks++;
    }

    if ((s_slope_task.state == VISION_SLOPE_TASK_ENTRY_HOLD) ||
        (s_slope_task.state == VISION_SLOPE_TASK_RUN))
    {
        traveled_mm = vision_slope_distance_from(s_slope_task.start_x_mm,
                                                 s_slope_task.start_y_mm);
#if SLOPE_IMPACT_DETECT_MODE != 0
        /* 冲击检测采样：布防期间 10ms 一拍，acc_z 换算与惯导调用处同系（9806.65*counts/4098） */
        if (s_slope_impact.armed != 0U)
        {
            s_slope_impact.sample_div++;
            if (s_slope_impact.sample_div >= SLOPE_IMPACT_SAMPLE_DIV)
            {
                s_slope_impact.sample_div = 0U;
                slope_impact_feed(9806.65f * (imu_data.acc_z / 4098.0f));
            }
        }
#endif
    }

    switch (s_slope_task.state)
    {
        case VISION_SLOPE_TASK_PVC_ALIGN:
            /* 复用 PVC 控制模块给出的方向修正；搜索/校准期间以低速行驶，给转向收敛留出距离。 */
            err_cmd = g_vision_pvc_control_status.err_degree_cmd;
            speed_cmd = VISION_SLOPE_TASK_PVC_ALIGN_SPEED_SET;
            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            /*
             * 沿用参考提交的入口确认逻辑：稳定看到 PVC 且白色区域占满画面才表示已压上入口。
             * 入口条件连续保持 50ms，可滤除临界画面抖动，避免过早锁住尚未进入斜坡的航向。
             */
            if ((g_vision_pvc_control_status.stable_detected != 0U) &&
                (g_vision_pvc_control_status.bbox_area_ratio_u16 >= VISION_SLOPE_TASK_PVC_FULL_RATIO_U16))
            {
                if (s_slope_task.pvc_align_ok_ticks < 0xFFFFU)
                {
                    s_slope_task.pvc_align_ok_ticks++;
                }
            }
            else
            {
                s_slope_task.pvc_align_ok_ticks = 0U;
            }

            /* PVC 入口确认稳定 50ms 后，保存此刻航向；后续蓝色坡面不再依赖视觉。 */
            if (s_slope_task.pvc_align_ok_ticks >= VISION_SLOPE_TASK_PVC_ALIGN_OK_TICKS)
            {
                VisionPvcControl_SetEnable(0U);
                VisionIpc_Core0_SetPvcEnable(VISION_PVC_DETECT_DEFAULT_ACTIVE);
                vision_slope_set_state(VISION_SLOPE_TASK_ENTRY_HOLD);
            }
            else if (s_slope_task.state_ticks >= VISION_SLOPE_TASK_PVC_ALIGN_TIMEOUT_TICKS)
            {
                /* 5 秒内仍未确认进入 PVC，认为视觉入口异常并安全退出。 */
                vision_slope_set_state(VISION_SLOPE_TASK_FAILSAFE);
            }
            break;

        case VISION_SLOPE_TASK_ENTRY_HOLD:
            /* 刚压上斜坡的 300ms 保持较高入口速度，并只根据惯导航向锁角。 */
            err_cmd = vision_slope_calc_yaw_hold_err();
            speed_cmd = VISION_SLOPE_TASK_ENTRY_SPEED_SET;
            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            if (s_slope_task.state_ticks >= VISION_SLOPE_TASK_ENTRY_HOLD_TICKS)
            {
                vision_slope_set_state(VISION_SLOPE_TASK_RUN);
            }
            break;

        case VISION_SLOPE_TASK_RUN:
            /* 蓝色斜坡路面不使用视觉或惯导方向修正，直接保持方向误差为零。 */
            err_cmd = 0.0f;
            speed_cmd = VISION_SLOPE_TASK_RUN_SPEED_SET;
            err_degree = err_cmd;
            target_speed_set = speed_cmd;

#if SLOPE_IMPACT_DETECT_MODE == 2
            /* 冲击检测为主判据：累计 >= K 次冲击且安静期结束即脱出（蜂鸣 3 声）；
             * 惯导里程降级为兜底（冲击未检出时强制退出，不蜂鸣）。 */
            if ((s_slope_impact.exit_detected != 0U) ||
                (traveled_mm >= SLOPE_IMPACT_FAILSAFE_DISTANCE_MM))
            {
                if (s_slope_impact.exit_detected != 0U)
                {
                    g_slope_impact_debug.exit_by_impact = 1U;
                    vision_slope_beep3_start();
                }
                else
                {
                    g_slope_impact_debug.exit_by_impact = 0U;
                }
                vision_slope_set_state(VISION_SLOPE_TASK_FINISH);
            }
#else
            if (traveled_mm >= VISION_SLOPE_TASK_EXIT_DISTANCE_MM)
            {
                vision_slope_set_state(VISION_SLOPE_TASK_FINISH);
            }
#endif
            break;

        case VISION_SLOPE_TASK_FINISH:
            /* 保留上一周期的锁角和速度输出，让主控逻辑在下一周期平滑接管。 */
            vision_slope_cleanup(0U);
            return;

        case VISION_SLOPE_TASK_FAILSAFE:
        default:
            vision_slope_cleanup(1U);
            return;
    }

    vision_slope_publish_status(packet, traveled_mm, err_cmd, speed_cmd);
}

#endif
