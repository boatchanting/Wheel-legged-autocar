#include "pvc_vision.h"

#if PVC_VISION_ENABLE

/*
 * 单个白色连通域的内部描述。
 * 这个结构只在本文件内部使用，外部控制层不要依赖它。
 */
typedef struct
{
    uint16 area;           /* 连通域像素数量。 */
    uint8 xmin;            /* 包围框左边界。 */
    uint8 ymin;            /* 包围框上边界。 */
    uint8 xmax;            /* 包围框右边界。 */
    uint8 ymax;            /* 包围框下边界。 */
    float centroid_x;      /* 连通域质心 x。 */
    float centroid_y;      /* 连通域质心 y。 */
    float fill_ratio;      /* 填充率：area / bbox_area。 */
    uint8 touches_border;  /* 是否接触图像边界。PVC 入口通常从图像边缘进入，这是强特征。 */
    float mean_gray;       /* 连通域平均灰度。 */
    float score;           /* 综合评分，数值越大越像 PVC 入口。 */
} pvc_component_t;

/*
 * 单帧检测的临时工作区。
 *
 * 注意：
 * - visited/stack/components/candidates 都放在静态区，避免每帧在栈上申请大数组。
 * - stack 最大为 PVC_IMAGE_SIZE，最坏情况下整幅图都是白色也不会溢出。
 */
typedef struct
{
    /* 【修改】使用解耦后的宏 PVC_IMAGE_SIZE */
    uint8 visited[PVC_IMAGE_SIZE];                           /* 连通域搜索访问标记。 */
    uint16 stack[PVC_IMAGE_SIZE];                            /* 非递归 flood fill 栈。 */
    pvc_component_t components[PVC_VISION_MAX_COMPONENTS];   /* 所有白色连通域，按面积排序。 */
    pvc_component_t candidates[PVC_VISION_MAX_COMPONENTS];   /* 通过基础过滤的候选，按 score 排序。 */
} pvc_scratch_t;

/* 对外暴露的运行统计和检测结果。volatile 是为了后续跨中断/跨核读取更安全。 */
volatile runtime_profiler_t g_pvc_vision_cost_profiler = {0};
volatile runtime_profiler_t g_pvc_vision_frame_profiler = {0};
volatile pvc_vision_output_t g_pvc_vision_output = {0};
volatile uint8 g_pvc_vision_output_write_busy = 0U;

/* 内部状态。 */
static pvc_scratch_t g_pvc_scratch;
/* 非 volatile 影子输出，避免每次更新都从 volatile 对象整结构读取。 */
static pvc_vision_output_t g_pvc_output_shadow;
/* IIR 平滑器状态。只平滑控制直接使用的距离/横偏，不平滑 bbox 等调试量。 */
static uint8 g_pvc_smooth_inited = 0U;
static int16 g_pvc_smooth_forward_mm = -1;
static int16 g_pvc_smooth_lateral_mm = 0;
/* 上一帧处理开始时间，用于统计帧间隔。 */
static uint32 g_pvc_last_frame_time_us = 0U;

static float pvc_min_f(float a, float b)
{
    return (a < b) ? a : b;
}

static float pvc_max_f(float a, float b)
{
    return (a > b) ? a : b;
}

static uint8 pvc_component_width(const pvc_component_t *component)
{
    return (uint8)(component->xmax - component->xmin + 1U);
}

static uint8 pvc_component_height(const pvc_component_t *component)
{
    return (uint8)(component->ymax - component->ymin + 1U);
}

static int16 pvc_estimate_forward_mm_from_row(uint8 row)
{
    /*
     * 前向距离估计，占位版。
     *
     * 输入：
     * - row 是最佳 PVC 候选包围框的 ymax，也就是图像里最靠近车的一行白边。
     *
     * 当前做法：
     * - 图像越靠下，说明 PVC 入口越近。
     * - 先用 20mm/行的线性关系作为初始参数：
     *   forward_mm = (59 - row) * 20。
     *
     * 后续实车建议：
     * - 把车放在 PVC 前方 100/200/300/400/600/800mm 处，各采几帧。
     * - 记录 entry_bottom_y 和真实距离，做 row->distance 查表。
     * - 控制层最终只用 forward_mm，不需要知道图像行号。
     */
    return (int16)((PVC_IMAGE_H - 1U - row) * 20U);
}

static int16 pvc_estimate_lateral_mm_from_x(float x)
{
    /*
     * 横向偏差估计，占位版。
     *
     * 输入：
     * - x 是最佳 PVC 候选白色区域的质心横坐标。
     *
     * 当前做法：
     * - 图像中心为 0。
     * - 白色区域质心在图像右侧时 lateral_mm 为正，左侧为负。
     * - 初始比例是 8mm/像素，调车时重点看方向是否正确。
     *
     * 实车调试：
     * - 如果车看到 PVC 偏右却向左修，先改 0 核
     *   VISION_PVC_CONTROL_LATERAL_SIGN，不要先动这里。
     * - 如果方向对但修正太弱/太猛，优先调 0 核
     *   VISION_PVC_CONTROL_K_LAT_DEG_PER_MM。
     */
    return (int16)((x - ((float)PVC_IMAGE_W - 1.0f) * 0.5f) * 8.0f);
}

static void pvc_clear_frame_result(pvc_vision_frame_result_t *result)
{
    /* 无效结果统一写 0xFF/-1，方便调试时一眼区分“没有候选”和“候选在 0 行/0 列”。 */
    memset(result, 0, sizeof(*result));
    result->bbox_xmin = 0xFFU;
    result->bbox_ymin = 0xFFU;
    result->bbox_xmax = 0xFFU;
    result->bbox_ymax = 0xFFU;
    result->entry_bottom_y = 0xFFU;
    result->entry_top_y = 0xFFU;
    result->forward_mm = -1;
}

static float pvc_score_component(const pvc_component_t *component)
{
    /*
     * 候选评分，与 PC 版 Python/C 算法保持一致。
     *
     * 评分特征解释：
     * - area_score：白色像素越多，越像一整块 PVC。
     * - width_score：入口 PVC 一般横向较宽，过窄更像反光点。
     * - height_score：入口进入视野后会形成一定高度，不只是单行白线。
     * - fill_score：包围框内部越实，越不像破碎噪声。
     * - border_score：入口阶段 PVC 常从图像下边缘或侧边进入，触边是强特征。
     * - brightness_score：PVC 应该接近高亮白色。
     *
     * 权重初始值偏向“面积”和“触边”，因为入口检测的首要目标是可靠触发，
     * 不是精确拟合边线。后续如果要做 PVC 内循迹，可以另写赛道线检测。
     */
    const float area_score = pvc_min_f((float)component->area / 600.0f, 1.0f);
    const float width_score = pvc_min_f((float)pvc_component_width(component) / 45.0f, 1.0f);
    const float height_score = pvc_min_f((float)pvc_component_height(component) / 18.0f, 1.0f);
    const float fill_score = pvc_min_f(component->fill_ratio / 0.55f, 1.0f);
    const float border_score = component->touches_border ? 1.0f : 0.0f;
    const float brightness_score = pvc_min_f(
        pvc_max_f((component->mean_gray - 235.0f) / 20.0f, 0.0f),
        1.0f);

    return 0.35f * area_score
         + 0.18f * width_score
         + 0.12f * height_score
         + 0.12f * fill_score
         + 0.18f * border_score
         + 0.05f * brightness_score;
}

static void pvc_sort_by_score(pvc_component_t *components, uint8 count)
{
    for (uint8 i = 1U; i < count; i++)
    {
        pvc_component_t key = components[i];
        int j = (int)i - 1;
        while ((j >= 0) && (components[j].score < key.score))
        {
            components[j + 1] = components[j];
            j--;
        }
        components[j + 1] = key;
    }
}

static void pvc_sort_by_area(pvc_component_t *components, uint8 count)
{
    for (uint8 i = 1U; i < count; i++)
    {
        pvc_component_t key = components[i];
        int j = (int)i - 1;
        while ((j >= 0) && (components[j].area < key.area))
        {
            components[j + 1] = components[j];
            j--;
        }
        components[j + 1] = key;
    }
}

static void pvc_flood_component(const uint8 *gray, uint16 start_index, pvc_component_t *out)
{
    /*
     * 4 邻域非递归 flood fill。
     *
     * 不用递归的原因：
     * - 车机栈空间有限，整块 PVC 白区可能包含几千个像素。
     * - 递归 DFS 最坏情况下会压爆栈。
     * - 显式 stack 数组大小固定，最坏也只有 PVC_IMAGE_SIZE。
     */
    uint16 stack_top = 0U;
    uint16 area = 0U;
    // 【修改】边界处理改为使用 PVC_IMAGE_W 和 PVC_IMAGE_H
    uint8 xmin = (uint8)(PVC_IMAGE_W - 1U);
    uint8 ymin = (uint8)(PVC_IMAGE_H - 1U);
    uint8 xmax = 0U;
    uint8 ymax = 0U;
    uint32 sum_x = 0U;
    uint32 sum_y = 0U;
    uint32 sum_gray = 0U;

    g_pvc_scratch.stack[stack_top++] = start_index;
    g_pvc_scratch.visited[start_index] = 1U;

    while (stack_top > 0U)
    {
        const uint16 index = g_pvc_scratch.stack[--stack_top];
        // 【修改】取行和列时除以/取余的宽度改为 PVC_IMAGE_W
        const uint8 y = (uint8)(index / PVC_IMAGE_W);
        const uint8 x = (uint8)(index - (uint16)y * PVC_IMAGE_W);

        area++;
        sum_x += x;
        sum_y += y;
        sum_gray += gray[index];

        if (x < xmin) { xmin = x; }
        if (x > xmax) { xmax = x; }
        if (y < ymin) { ymin = y; }
        if (y > ymax) { ymax = y; }

        if (y > 0U)
        {
            /* 上邻居。 */
            const uint16 ni = (uint16)(index - PVC_IMAGE_W);
            if ((g_pvc_scratch.visited[ni] == 0U) && (gray[ni] >= PVC_VISION_WHITE_THRESHOLD))
            {
                g_pvc_scratch.visited[ni] = 1U;
                g_pvc_scratch.stack[stack_top++] = ni;
            }
        }
        // 【修改】检测下边界使用 PVC_IMAGE_H
        if (y < (PVC_IMAGE_H - 1U))
        {
            /* 下邻居。 */
            const uint16 ni = (uint16)(index + PVC_IMAGE_W);
            if ((g_pvc_scratch.visited[ni] == 0U) && (gray[ni] >= PVC_VISION_WHITE_THRESHOLD))
            {
                g_pvc_scratch.visited[ni] = 1U;
                g_pvc_scratch.stack[stack_top++] = ni;
            }
        }
        if (x > 0U)
        {
            /* 左邻居。 */
            const uint16 ni = (uint16)(index - 1U);
            if ((g_pvc_scratch.visited[ni] == 0U) && (gray[ni] >= PVC_VISION_WHITE_THRESHOLD))
            {
                g_pvc_scratch.visited[ni] = 1U;
                g_pvc_scratch.stack[stack_top++] = ni;
            }
        }
        // 【修改】检测右边界使用 PVC_IMAGE_W
        if (x < (PVC_IMAGE_W - 1U))
        {
            /* 右邻居。 */
            const uint16 ni = (uint16)(index + 1U);
            if ((g_pvc_scratch.visited[ni] == 0U) && (gray[ni] >= PVC_VISION_WHITE_THRESHOLD))
            {
                g_pvc_scratch.visited[ni] = 1U;
                g_pvc_scratch.stack[stack_top++] = ni;
            }
        }
    }

    {
        /* 连通域搜索完成后，把累计量转成可评分的特征。 */
        const uint16 bbox_area = (uint16)((xmax - xmin + 1U) * (ymax - ymin + 1U));
        out->area = area;
        out->xmin = xmin;
        out->ymin = ymin;
        out->xmax = xmax;
        out->ymax = ymax;
        out->centroid_x = (float)sum_x / (float)area;
        out->centroid_y = (float)sum_y / (float)area;
        out->fill_ratio = (float)area / (float)bbox_area;
        // 【修改】判断触边条件使用新的宽和高宏
        out->touches_border = (uint8)((xmin == 0U) || (ymin == 0U) || (xmax == (PVC_IMAGE_W - 1U)) || (ymax == (PVC_IMAGE_H - 1U)));
        out->mean_gray = (float)sum_gray / (float)area;
        out->score = 0.0f;
    }
}

static uint8 pvc_collect_components(const uint8 *gray)
{
    uint8 component_count = 0U;

    /* 每帧重新清空访问标记。 */
    memset(g_pvc_scratch.visited, 0, sizeof(g_pvc_scratch.visited));

    /* 扫描整幅 94x60 图像。使用 PVC_IMAGE_SIZE 进行界定 */
    for (uint16 i = 0U; i < PVC_IMAGE_SIZE; i++)
    {
        if ((g_pvc_scratch.visited[i] != 0U) || (gray[i] < PVC_VISION_WHITE_THRESHOLD))
        {
            continue;
        }

        if (component_count < PVC_VISION_MAX_COMPONENTS)
        {
            pvc_flood_component(gray, i, &g_pvc_scratch.components[component_count]);
            component_count++;
        }
        else
        {
            /*
             * 如果连通域数量超过上限，只标记当前像素，避免死循环。
             * 真实 PVC 场景不会接近 128 个有效连通域；超过通常说明阈值或曝光异常。
             */
            g_pvc_scratch.visited[i] = 1U;
        }
    }

    pvc_sort_by_area(g_pvc_scratch.components, component_count);
    return component_count;
}

static uint8 pvc_filter_candidates(uint8 component_count)
{
    uint8 candidate_count = 0U;

    /* 对每个连通域先评分，再做硬阈值过滤。 */
    for (uint8 i = 0U; i < component_count; i++)
    {
        pvc_component_t component = g_pvc_scratch.components[i];
        component.score = pvc_score_component(&component);

        if (component.area < PVC_VISION_MIN_AREA)
        {
            /* 面积太小，多半是反光点或噪声。 */
            continue;
        }
        if ((pvc_component_width(&component) < PVC_VISION_MIN_WIDTH) ||
            (pvc_component_height(&component) < PVC_VISION_MIN_HEIGHT))
        {
            continue;
        }
        if (component.fill_ratio < PVC_VISION_MIN_FILL_RATIO)
        {
            /* 填充率太低，说明包围框内部很碎，不像完整 PVC 白边。 */
            continue;
        }
        if (component.touches_border == 0U)
        {
            /* PVC 入口识别阶段重点寻找从图像边缘出现的大白块。 */
            continue;
        }

        if (candidate_count < PVC_VISION_MAX_COMPONENTS)
        {
            g_pvc_scratch.candidates[candidate_count++] = component;
        }
    }

    pvc_sort_by_score(g_pvc_scratch.candidates, candidate_count);
    return candidate_count;
}

static void pvc_copy_best_to_result(const pvc_component_t *best, pvc_vision_frame_result_t *result)
{
    /* 把内部候选转换成模块对外输出结构。 */
    result->area = best->area;
    result->bbox_xmin = best->xmin;
    result->bbox_ymin = best->ymin;
    result->bbox_xmax = best->xmax;
    result->bbox_ymax = best->ymax;
    result->entry_bottom_y = best->ymax;
    result->entry_top_y = best->ymin;
    result->confidence = best->score;
    result->centroid_x = best->centroid_x;
    result->centroid_y = best->centroid_y;
    result->fill_ratio = best->fill_ratio;
    result->mean_gray = best->mean_gray;
    result->forward_mm = pvc_estimate_forward_mm_from_row(best->ymax);
    result->lateral_mm = pvc_estimate_lateral_mm_from_x(best->centroid_x);
    result->yaw_error_deg_x100 = 0;
}

static void pvc_detect_frame(const uint8 *gray, pvc_vision_frame_result_t *result)
{
    /*
     * 单帧检测主流程：
     * 1. 提取所有白色连通域。
     * 2. 根据几何/亮度条件筛候选。
     * 3. 选择 score 最大的候选。
     * 4. score 超过阈值才给 raw detected。
     *
     * 输出给控制层的关键量：
     * - detected：本帧是否可信。
     * - bbox_xmin/ymin/xmax/ymax：用于计算“PVC 占画面比例”，接近时可停车。
     * - forward_mm：入口距离估计，用于减速。
     * - lateral_mm：入口横向偏差，用于转向。
     * - confidence：现场调阈值时看这个值最直观。
     */
    const uint8 component_count = pvc_collect_components(gray);
    const uint8 candidate_count = pvc_filter_candidates(component_count);

    pvc_clear_frame_result(result);
    result->component_count = component_count;
    result->candidate_count = candidate_count;

    if (candidate_count > 0U)
    {
        const pvc_component_t *best = &g_pvc_scratch.candidates[0];
        pvc_copy_best_to_result(best, result);
        result->detected = (uint8)(best->score >= PVC_VISION_MIN_DECISION_SCORE);
        if (result->detected == 0U)
        {
            result->forward_mm = -1;
        }
    }
}

static void pvc_update_filter(const pvc_vision_frame_result_t *raw)
{
    /*
     * 多帧稳定输出策略：
     *
     * raw：单帧立即结果，响应快，但容易受反光/曝光扰动影响。
     * stable：控制层使用结果，必须连续 PVC_VISION_CONFIRM_FRAMES 帧检测到才置位。
     *
     * 丢帧保持：
     * stable 已经为 1 时，如果偶发 1 帧 raw=0，不立刻清掉 stable。
     * 这样能避免白边被车身震动、图传撕裂、短曝光波动打断。
     */
    pvc_vision_output_t next = g_pvc_output_shadow;

    next.frame_id++;
    next.raw = *raw;
    next.raw_detected = raw->detected;

#if PVC_VISION_SMOOTH_ENABLE
    if (raw->detected)
    {
        if (next.detected_streak < 255U)
        {
            next.detected_streak++;
        }
        next.lost_streak = 0U;

        if (g_pvc_smooth_inited == 0U)
        {
            /* 第一次检测到时直接初始化平滑值，避免从 0 慢慢爬。 */
            g_pvc_smooth_forward_mm = raw->forward_mm;
            g_pvc_smooth_lateral_mm = raw->lateral_mm;
            g_pvc_smooth_inited = 1U;
        }
        else
        {
            /*
             * 一阶 IIR 平滑：new = 0.75 old + 0.25 raw。
             * 只平滑控制直接使用的距离和横偏，bbox/area/score 保持原始值便于调试。
             */
            g_pvc_smooth_forward_mm = (int16)((3 * g_pvc_smooth_forward_mm + raw->forward_mm) / 4);
            g_pvc_smooth_lateral_mm = (int16)((3 * g_pvc_smooth_lateral_mm + raw->lateral_mm) / 4);
        }

        if (next.detected_streak >= PVC_VISION_CONFIRM_FRAMES)
        {
            next.stable_detected = 1U;
        }
    }
    else
    {
        next.detected_streak = 0U;
        if (next.lost_streak < 255U)
        {
            next.lost_streak++;
        }
        if (next.lost_streak >= PVC_VISION_LOST_HOLD_FRAMES)
        {
            next.stable_detected = 0U;
            g_pvc_smooth_inited = 0U;
        }
    }

    if (next.stable_detected)
    {
        /*
         * raw 有效时刷新 stable 的调试字段；raw 短暂丢失时保留上一帧 stable。
         * 这样控制层在保持窗口内仍能拿到最近一次有效的距离/横偏。
         */
        if (raw->detected)
        {
            next.stable = *raw;
        }
        next.stable.detected = 1U;
        next.stable.forward_mm = g_pvc_smooth_forward_mm;
        next.stable.lateral_mm = g_pvc_smooth_lateral_mm;
    }
    else
    {
        pvc_clear_frame_result(&next.stable);
    }
#else
    next.detected_streak = raw->detected ? 1U : 0U;
    next.lost_streak = raw->detected ? 0U : 1U;
    next.stable_detected = raw->detected;
    next.stable = *raw;
#endif

    g_pvc_output_shadow = next;
    g_pvc_vision_output_write_busy = 1U;
    g_pvc_vision_output = next;
    g_pvc_vision_output_write_busy = 0U;
}

void pvc_vision_init(void)
{
    /* 初始化时清空历史状态，防止上电残留值影响第一帧判断。 */
    pvc_vision_reset_filter();
#if PVC_VISION_PROFILE_ENABLE
    /* 使用硬件 timer 统计耗时，逻辑模仿 code/tools/runtime_profiler.h 的用法。 */
    timer_init(PVC_VISION_PROFILE_TIMER, TIMER_US);
    timer_start(PVC_VISION_PROFILE_TIMER);
    RUNTIME_PROFILE_RESET(&g_pvc_vision_cost_profiler);
    RUNTIME_PROFILE_RESET(&g_pvc_vision_frame_profiler);
    g_pvc_last_frame_time_us = timer_get(PVC_VISION_PROFILE_TIMER);
#endif
}

void pvc_vision_reset_filter(void)
{
    /*
     * 状态机切换项目时建议调用：
     * - 清空连续检测帧数。
     * - 清空丢失帧数。
     * - 清空距离/横偏平滑器。
     * - 输出重新变成无效。
     */
    pvc_vision_output_t empty;
    memset(&empty, 0, sizeof(empty));
    pvc_clear_frame_result(&empty.raw);
    pvc_clear_frame_result(&empty.stable);
    g_pvc_output_shadow = empty;
    g_pvc_vision_output_write_busy = 1U;
    g_pvc_vision_output = empty;
    g_pvc_vision_output_write_busy = 0U;
    g_pvc_smooth_inited = 0U;
    g_pvc_smooth_forward_mm = -1;
    g_pvc_smooth_lateral_mm = 0;
}

const volatile pvc_vision_output_t *pvc_vision_get_output(void)
{
    /*
     * 1 核本地读取可以直接使用返回指针。
     * 如果 0 核通过共享内存读取，不能直接读这个地址，应该由通信层做 DCache 同步和结构拷贝。
     */
    return &g_pvc_vision_output;
}

void pvc_vision_process_camera_frame(const uint8 *gray)
{
    /*
     * 每来一帧摄像头图像调用一次。
     *
     * 当前工程建议调用位置在 1 核 main_cm7_1.c：
     * if(mt9v03x_finish_flag) {
     *     mt9v03x_finish_flag = 0;
     *     memcpy(image_copy[0], mt9v03x_image[0], MT9V03X_IMAGE_SIZE);
     *     compress_image_to_target();
     *     if(VisionIpc_Core1_ShouldRunPvc()) {
     *         pvc_vision_process_camera_frame(compressed_image_copy[0]);
     *     }
     * }
     *
     * 注意：
     * - gray 必须是 94x60 连续灰度数组，不能直接传 188x120 原图。
     * - 算法只处理图像和更新 g_pvc_vision_output。
     * - IPC 发布在 1 核 2ms 中断里做，控制在 0 核 2ms 中断里做。
     */
    pvc_vision_frame_result_t raw;

    if (gray == NULL)
    {
        return;
    }

#if PVC_VISION_PROFILE_ENABLE
    {
        /* 帧间隔统计：用于估算实际视觉帧率，而不是算法耗时。 */
        const uint32 now_us = timer_get(PVC_VISION_PROFILE_TIMER);
        runtime_profiler_update(&g_pvc_vision_frame_profiler, (uint32)(now_us - g_pvc_last_frame_time_us));
        g_pvc_last_frame_time_us = now_us;
    }
    /* 算法耗时统计：只包住检测和平滑更新，不包含图传发送。 */
    RUNTIME_PROFILE_BEGIN(g_pvc_vision_cost_profiler, PVC_VISION_PROFILE_TIMER);
#endif

    pvc_detect_frame(gray, &raw);
    pvc_update_filter(&raw);

#if PVC_VISION_PROFILE_ENABLE
    RUNTIME_PROFILE_END(&g_pvc_vision_cost_profiler, PVC_VISION_PROFILE_TIMER);
#endif

#if (PVC_VISION_DEBUG_PRINT_EVERY > 0U)
    if ((g_pvc_vision_output.frame_id % PVC_VISION_DEBUG_PRINT_EVERY) == 0U)
    {
        printf("[PVC] frame=%lu raw=%u stable=%u score=%d cost=%lu us frame_dt=%lu us\r\n",
               (unsigned long)g_pvc_vision_output.frame_id,
               g_pvc_vision_output.raw_detected,
               g_pvc_vision_output.stable_detected,
               (int)(g_pvc_vision_output.raw.confidence * 1000.0f),
               (unsigned long)g_pvc_vision_cost_profiler.last_us,
               (unsigned long)g_pvc_vision_frame_profiler.last_us);
    }
#endif
}

#endif
