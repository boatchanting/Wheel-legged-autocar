/*
 * =================================================================================
 * 文件: line_vision.c
 * 作用: 1 核 (Core 1) 桥梁/直线任务视觉检测的具体实现代码。
 * 说明: 这里是直线和桥梁检测真正干活的地方。它做两件事：
 *       1. 找桥梁：在画面里找深色的“黑块”（因为桥梁通常是黑色的）。
 *       2. 找直线：如果没看到桥，就在画面里找白色的引导线，算出车子偏离了多少。
 *       最后把算好的偏差、角度等信息发出去，供 0 核用来控制车子。
 * =================================================================================
 */
#include "line_vision.h"

#if LINE_VISION_ENABLE

/* --- 1. 内部数据结构定义 --- */

/**
 * @brief 记录照片上一个点的位置（坐标）
 */
typedef struct
{
    uint16 x; /* 横坐标 */
    uint16 y; /* 纵坐标 */
} line_point_t;

/**
 * @brief 记录桥梁上的一个“黑块”特征
 */
typedef struct
{
    uint16 area;       /* 黑块有多少个像素 */
    uint8 xmin;        /* 最左边 */
    uint8 ymin;        /* 最上面 */
    uint8 xmax;        /* 最右边 */
    uint8 ymax;        /* 最下面 */
    float mean_gray;   /* 平均有多黑（越接近 0 越黑） */
    float fill_ratio;  /* 填得满不满（如果是空心圈，值就很小） */
    float score;       /* 长得像不像桥？算法打的分数 */
} line_bridge_component_t;

/**
 * @brief 找直线和找桥时用到的“草稿纸”（临时工作区）
 * @note  放在全局区，防止把单片机的内存栈撑爆。
 */
typedef struct
{
    uint8 white_mask[LINE_IMAGE_SIZE];  /* 标记哪些地方是白色的（1=白，0=不白） */
    uint8 visited[LINE_IMAGE_SIZE];     /* 找黑块时，标记哪些点已经找过了 */
    uint16 stack[LINE_IMAGE_SIZE];      /* 找黑块时用的“记忆栈” */
    uint16 hist[256];                   /* 亮度直方图：用来统计画面里每种亮度各有多少个点 */
    line_point_t points[LINE_IMAGE_H];  /* 找直线时，找到的一堆中心点 */
    uint8 widths[LINE_IMAGE_H];         /* 每个中心点所在的白线有多宽 */
} line_scratch_t;

/* --- 2. 全局变量（对外公开的统计数据和结果） --- */
volatile runtime_profiler_t g_line_vision_cost_profiler = {0};  /* 算法花了多长时间 */
volatile runtime_profiler_t g_line_vision_frame_profiler = {0}; /* 帧间隔（照片来的频率） */
volatile line_vision_output_t g_line_vision_output = {0};       /* 最终的输出结果 */
volatile uint8 g_line_vision_output_write_busy = 0U;            /* 写数据时的防冲突锁 */

/* 内部使用的全局变量 */
static line_scratch_t g_line_scratch;                           /* 刚才说的“草稿纸” */
static line_vision_output_t g_line_output_shadow;               /* 自己用的结果备份 */
static uint32 g_line_last_frame_time_us = 0U;                   /* 上一张照片是什么时候来的 */

/* --- 3. 基础数学工具函数 --- */

/**
 * @brief 求绝对值（把负数变成正数）
 */
static float line_abs_f(float value)
{
    return (value < 0.0f) ? -value : value;
}

/**
 * @brief 取两个数里较小的一个
 */
static float line_min_f(float a, float b)
{
    return (a < b) ? a : b;
}

/**
 * @brief 取两个数里较大的一个
 */
static float line_max_f(float a, float b)
{
    return (a > b) ? a : b;
}

/**
 * @brief 把一个数限制在指定的最小值和最大值之间
 * @note  如果 value 太小就等于 min_value，太大就等于 max_value。
 */
static float line_constrain_f(float value, float min_value, float max_value)
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
 * @brief 把一帧的原始结果清零
 * @note  没看到直线也没看到桥的时候，把这些框和距离都设成无效。
 */
static void line_clear_frame_result(line_vision_frame_result_t *result)
{
    memset(result, 0, sizeof(*result));
    result->bridge_bbox_xmin = 0xFFU;
    result->bridge_bbox_ymin = 0xFFU;
    result->bridge_bbox_xmax = 0xFFU;
    result->bridge_bbox_ymax = 0xFFU;
}

/* --- 4. 找直线辅助函数 --- */

/**
 * @brief 从直方图里算出某个百分比对应的亮度值
 * 
 * @param hist 直方图数组（记录了每个亮度有多少个像素）
 * @param total 像素总数
 * @param percent 百分比（比如 70，代表我们要找排在 70% 位置的那个亮度）
 * @return uint8 算出的亮度值
 * 
 * @note 这个函数用来动态调整寻找白线的门槛。
 *       不同地方的光线不一样，不能死板地用一个固定的亮度。
 */
static uint8 line_percentile_from_hist(const uint16 *hist, uint16 total, uint16 percent)
{
    uint16 target = (uint16)((uint32)total * (uint32)percent / 100U);
    uint16 acc = 0U;

    if (target >= total)
    {
        target = (uint16)(total - 1U);
    }

    for (uint16 i = 0U; i < 256U; i++)
    {
        acc = (uint16)(acc + hist[i]);
        if (acc > target)
        {
            return (uint8)i;
        }
    }
    return 255U;
}

/**
 * @brief 把照片里可能是白线的地方“抠”出来
 * 
 * @param gray 原始灰度照片
 * @param y_min 从照片的第几行开始找
 * @param y_max 找到照片的第几行结束
 * @param white_ratio 输出：找到的白点占了多大比例
 * 
 * @note 算法会每一行单独看，自动计算这一行“多亮才算白线”，
 *       然后把白线所在的位置在草稿纸 (white_mask) 上涂黑。
 */
static void line_build_near_white_mask(const uint8 *gray, uint8 y_min, uint8 y_max, float *white_ratio)
{
    uint32 white_count = 0U; /* 记下有多少个白点 */
    uint32 roi_count = (uint32)(y_max - y_min + 1U) * (uint32)LINE_IMAGE_W; /* 总共看了多少个点 */

    /* 先把草稿纸擦干净 */
    memset(g_line_scratch.white_mask, 0, sizeof(g_line_scratch.white_mask));

    /* 逐行扫描 */
    for (uint8 y = y_min; y <= y_max; y++)
    {
        uint32 sum = 0U;
        uint8 p70;
        uint8 p92;
        float threshold_f;
        uint8 threshold;
        const uint16 row_base = (uint16)y * LINE_IMAGE_W;

        /* 清空直方图，准备统计这一行的亮度分布 */
        memset(g_line_scratch.hist, 0, sizeof(g_line_scratch.hist));

        /* 统计这一行每个像素的亮度 */
        for (uint8 x = 0U; x < LINE_IMAGE_W; x++)
        {
            const uint8 pixel = gray[row_base + x];
            sum += pixel;
            g_line_scratch.hist[pixel]++;
        }

        /* 算一下这一行的动态亮度门槛 */
        p70 = line_percentile_from_hist(g_line_scratch.hist, LINE_IMAGE_W, 70U);
        p92 = line_percentile_from_hist(g_line_scratch.hist, LINE_IMAGE_W, 92U);
        threshold_f = line_max_f((float)sum / (float)LINE_IMAGE_W + 8.0f, (float)p70 + 4.0f);
        threshold_f = line_max_f(threshold_f, (float)p92 - 22.0f);
        threshold_f = line_constrain_f(threshold_f, 145.0f, 245.0f); /* 门槛不能太离谱 */
        threshold = (uint8)(threshold_f + 0.5f);

        /* 根据算好的门槛，把亮的点标记在草稿纸上 */
        for (uint8 x = 0U; x < LINE_IMAGE_W; x++)
        {
            if (gray[row_base + x] >= threshold)
            {
                g_line_scratch.white_mask[row_base + x] = 255U;
                white_count++;
            }
        }
    }

    /* 算一下白点的比例 */
    *white_ratio = (roi_count > 0U) ? ((float)white_count / (float)roi_count) : 0.0f;
}

/**
 * @brief 在白线区域里，找出每一行白线的“中心点”
 * 
 * @param y_min 查找范围的顶
 * @param y_max 查找范围的底
 * @param out_y_span 输出：这些中心点一共跨越了多少行
 * @param out_mean_width 输出：白线的平均宽度
 * @return uint8 找出了几个中心点
 * 
 * @note 为什么找中心点？
 *       因为赛道引导线是一条有宽度的带子，我们得算出它的中心，
 *       把这些中心点连起来，就是车子该走的方向。
 */
static uint8 line_extract_center_points(uint8 y_min, uint8 y_max, uint8 *out_y_span, float *out_mean_width)
{
    uint8 point_count = 0U;
    uint16 width_sum = 0U;
    uint8 min_y = 0xFFU;
    uint8 max_y = 0U;
    uint8 lost_rows = 0U;
    /* 刚开始假设前一行的中心点在画面正中央 */
    float prev_center = ((float)LINE_IMAGE_W - 1.0f) * 0.5f;

    /* 从下往上找（因为下面的线离车更近，看得更清楚） */
    for (int y = (int)y_max; y >= (int)y_min; y--)
    {
        uint8 found = 0U;
        uint8 best_x0 = 0U;
        uint8 best_x1 = 0U;
        float best_center = 0.0f;
        float best_diff = 100000.0f;
        uint8 x = 0U;

        /* 在这一行里找一段一段的白线 */
        while (x < LINE_IMAGE_W)
        {
            uint8 x0;
            uint8 x1;
            uint8 run_width;
            float center;
            float diff;

            /* 略过黑的地方，找到白线的起点 */
            while ((x < LINE_IMAGE_W) &&
                   (g_line_scratch.white_mask[(uint16)y * LINE_IMAGE_W + x] == 0U))
            {
                x++;
            }
            if (x >= LINE_IMAGE_W)
            {
                break;
            }

            x0 = x; /* 白线的左边缘 */
            /* 找到白线的终点 */
            while ((x < LINE_IMAGE_W) &&
                   (g_line_scratch.white_mask[(uint16)y * LINE_IMAGE_W + x] != 0U))
            {
                x++;
            }
            x1 = (uint8)(x - 1U); /* 白线的右边缘 */
            run_width = (uint8)(x1 - x0 + 1U);
            
            /* 太细的不要 */
            if (run_width < LINE_VISION_MIN_WIDTH)
            {
                continue;
            }

            /* 算出这段白线的中心 */
            center = ((float)x0 + (float)x1) * 0.5f;
            /* 算算这个中心和上一行的中心偏了多少 */
            diff = line_abs_f(center - prev_center);
            /* 如果这一行有好几段白线，挑一个和上一行中心最接近的 */
            if (diff < best_diff)
            {
                best_diff = diff;
                best_center = center;
                best_x0 = x0;
                best_x1 = x1;
                found = 1U;
            }
        }

        /* 如果这一行啥也没找到 */
        if (found == 0U)
        {
            lost_rows++;
            /* 如果连续 4 行都断了，就当线到此为止了，不找了 */
            if ((lost_rows >= 4U) && (point_count > 0U))
            {
                break;
            }
            continue;
        }

        /* 如果找到了，但偏得太离谱（突然跳跃了 30% 画面宽），说明可能是个干扰，不要 */
        if ((point_count > 0U) &&
            (line_abs_f(best_center - prev_center) > ((float)LINE_IMAGE_W * 0.30f)))
        {
            lost_rows++;
            continue;
        }

        lost_rows = 0U;
        /* 平滑一下预估的中心点位置，给下一行用 */
        prev_center = 0.70f * prev_center + 0.30f * best_center;

        /* 把找到的这个点存起来 */
        if (point_count < LINE_IMAGE_H)
        {
            const uint8 run_width = (uint8)(best_x1 - best_x0 + 1U);
            /* 放大 100 倍存整数，防丢精度 */
            g_line_scratch.points[point_count].x = (uint16)(best_center * 100.0f + 0.5f);
            g_line_scratch.points[point_count].y = (uint16)y;
            g_line_scratch.widths[point_count] = run_width;
            width_sum = (uint16)(width_sum + run_width);
            if ((uint8)y < min_y) { min_y = (uint8)y; }
            if ((uint8)y > max_y) { max_y = (uint8)y; }
            point_count++;
        }
    }

    *out_y_span = (point_count >= 2U) ? (uint8)(max_y - min_y) : 0U;
    *out_mean_width = (point_count > 0U) ? ((float)width_sum / (float)point_count) : 0.0f;
    return point_count;
}

/**
 * @brief 把刚才找到的一堆中心点，连成一条直线
 * 
 * @param point_count 一共有几个点
 * @param out_k 输出：直线的斜率 (k)
 * @param out_b 输出：直线的截距 (b)
 * @param out_rmse 输出：这些点连成直线有多“直”（误差大不大）
 * @return uint8 1: 拟合成功; 0: 失败
 * 
 * @note 用的是“最小二乘法”：数学上用来把一堆点画成一根最佳直线的方法。
 */
static uint8 line_fit_points(uint8 point_count,
                             float *out_k,
                             float *out_b,
                             float *out_rmse)
{
    float sum_x = 0.0f;
    float sum_y = 0.0f;
    float sum_yy = 0.0f;
    float sum_xy = 0.0f;
    float denom;
    float err_sum = 0.0f;

    if (point_count < 2U)
    {
        return 0U; /* 至少得有 2 个点才能连成线 */
    }

    /* 把所有的坐标累加起来套公式 */
    for (uint8 i = 0U; i < point_count; i++)
    {
        const float x = (float)g_line_scratch.points[i].x * 0.01f;
        const float y = (float)g_line_scratch.points[i].y;
        sum_x += x;
        sum_y += y;
        sum_yy += y * y;
        sum_xy += x * y;
    }

    /* 分母，如果太接近 0 就没法除了 */
    denom = (float)point_count * sum_yy - sum_y * sum_y;
    if (line_abs_f(denom) < 0.001f)
    {
        return 0U;
    }

    /* 算出 k 和 b ( x = k * y + b ) */
    *out_k = ((float)point_count * sum_xy - sum_y * sum_x) / denom;
    *out_b = (sum_x - (*out_k) * sum_y) / (float)point_count;

    /* 算一算每个点离算出来的直线有多远（计算均方根误差） */
    for (uint8 i = 0U; i < point_count; i++)
    {
        const float x = (float)g_line_scratch.points[i].x * 0.01f;
        const float y = (float)g_line_scratch.points[i].y;
        const float pred = (*out_k) * y + (*out_b); /* 理论上的 x */
        const float e = x - pred; /* 实际偏了多少 */
        err_sum += e * e;
    }

    *out_rmse = sqrtf(err_sum / (float)point_count);
    return 1U;
}

/* --- 5. 找桥梁黑块辅助函数 --- */

/**
 * @brief 在照片上找到一块连在一起的深色区域（找黑斑，也就是找桥）
 * 
 * @param gray 原始灰度照片
 * @param start_index 从照片的哪个点开始找
 * @param y_min 查找范围的上边界
 * @param y_max 查找范围的下边界
 * @param out 找完之后，把这个黑块的特征填到这个档案里
 * 
 * @note 和前面找 PVC 的漫水填充法一模一样，只不过这里是“找黑”而不是“找白”。
 */
static void line_flood_dark_component(const uint8 *gray,
                                      uint16 start_index,
                                      uint8 y_min,
                                      uint8 y_max,
                                      line_bridge_component_t *out)
{
    uint16 stack_top = 0U;
    uint16 area = 0U;
    uint8 xmin = (uint8)(LINE_IMAGE_W - 1U);
    uint8 ymin = (uint8)(LINE_IMAGE_H - 1U);
    uint8 xmax = 0U;
    uint8 ymax = 0U;
    uint32 sum_gray = 0U;

    g_line_scratch.stack[stack_top++] = start_index;
    g_line_scratch.visited[start_index] = 1U;

    while (stack_top > 0U)
    {
        const uint16 index = g_line_scratch.stack[--stack_top];
        const uint8 y = (uint8)(index / LINE_IMAGE_W);
        const uint8 x = (uint8)(index - (uint16)y * LINE_IMAGE_W);

        area++;
        sum_gray += gray[index];

        if (x < xmin) { xmin = x; }
        if (x > xmax) { xmax = x; }
        if (y < ymin) { ymin = y; }
        if (y > ymax) { ymax = y; }

        /* 向上找 */
        if (y > y_min)
        {
            const uint16 ni = (uint16)(index - LINE_IMAGE_W);
            if ((g_line_scratch.visited[ni] == 0U) &&
                (gray[ni] < LINE_VISION_BRIDGE_DARK_THRESHOLD)) /* 注意这里是小于阈值，因为桥是黑的 */
            {
                g_line_scratch.visited[ni] = 1U;
                if (stack_top < LINE_IMAGE_SIZE)
                {
                    g_line_scratch.stack[stack_top++] = ni;
                }
            }
        }
        /* 向下找 */
        if (y < y_max)
        {
            const uint16 ni = (uint16)(index + LINE_IMAGE_W);
            if ((g_line_scratch.visited[ni] == 0U) &&
                (gray[ni] < LINE_VISION_BRIDGE_DARK_THRESHOLD))
            {
                g_line_scratch.visited[ni] = 1U;
                if (stack_top < LINE_IMAGE_SIZE)
                {
                    g_line_scratch.stack[stack_top++] = ni;
                }
            }
        }
        /* 向左找 */
        if (x > 0U)
        {
            const uint16 ni = (uint16)(index - 1U);
            if ((g_line_scratch.visited[ni] == 0U) &&
                (gray[ni] < LINE_VISION_BRIDGE_DARK_THRESHOLD))
            {
                g_line_scratch.visited[ni] = 1U;
                if (stack_top < LINE_IMAGE_SIZE)
                {
                    g_line_scratch.stack[stack_top++] = ni;
                }
            }
        }
        /* 向右找 */
        if (x < (LINE_IMAGE_W - 1U))
        {
            const uint16 ni = (uint16)(index + 1U);
            if ((g_line_scratch.visited[ni] == 0U) &&
                (gray[ni] < LINE_VISION_BRIDGE_DARK_THRESHOLD))
            {
                g_line_scratch.visited[ni] = 1U;
                if (stack_top < LINE_IMAGE_SIZE)
                {
                    g_line_scratch.stack[stack_top++] = ni;
                }
            }
        }
    }

    {
        const uint16 bbox_area = (uint16)((xmax - xmin + 1U) * (ymax - ymin + 1U));
        out->area = area;
        out->xmin = xmin;
        out->ymin = ymin;
        out->xmax = xmax;
        out->ymax = ymax;
        out->mean_gray = (area > 0U) ? ((float)sum_gray / (float)area) : 255.0f;
        out->fill_ratio = (bbox_area > 0U) ? ((float)area / (float)bbox_area) : 0.0f;
        out->score = 0.0f;
    }
}

/**
 * @brief 给一个黑块打分，看看它到底多像一座桥
 * 
 * @param component 要打分的黑块档案
 * @return float 分数 (0.0 ~ 1.0)
 * 
 * @note 评分标准：
 *       - 面积：要够大（38%）
 *       - 尺寸：要有一定的宽度和高度（28%）
 *       - 颜色：要够黑（22%）
 *       - 位置：如果在照片上半部分，说明桥还在远处，加分（12%）
 */
static float line_score_bridge_component(const line_bridge_component_t *component)
{
    const uint8 comp_w = (uint8)(component->xmax - component->xmin + 1U);
    const uint8 comp_h = (uint8)(component->ymax - component->ymin + 1U);
    const float area_score = line_min_f((float)component->area / 260.0f, 1.0f);
    const float size_score = 0.5f * line_min_f((float)comp_w / 28.0f, 1.0f) +
                             0.5f * line_min_f((float)comp_h / 12.0f, 1.0f);
    const float dark_score = line_constrain_f((190.0f - component->mean_gray) / 70.0f, 0.0f, 1.0f);
    const float top_score = line_max_f(0.0f, 1.0f - (float)component->ymin / ((float)LINE_IMAGE_H * 0.45f));

    return 0.38f * area_score + 0.28f * size_score + 0.22f * dark_score + 0.12f * top_score;
}

/**
 * @brief 在单帧照片里找桥的主流程
 * 
 * @param gray 灰度照片
 * @param result 把找桥的结果填进这个表里
 * @return uint8 1: 找到了桥; 0: 没找到
 */
static uint8 line_detect_dark_bridge(const uint8 *gray, line_vision_frame_result_t *result)
{
    const uint8 y_min = (uint8)((uint32)LINE_IMAGE_H * 3U / 100U);
    const uint8 y_max = (uint8)(LINE_IMAGE_H - 2U);
    line_bridge_component_t best;
    uint8 component_count = 0U;

    memset(&best, 0, sizeof(best));
    best.xmin = 0xFFU;
    best.ymin = 0xFFU;
    best.xmax = 0xFFU;
    best.ymax = 0xFFU;

    /* 擦干草稿纸 */
    memset(g_line_scratch.visited, 0, sizeof(g_line_scratch.visited));

    /* 遍历照片找黑块 */
    for (uint8 y = y_min; y <= y_max; y++)
    {
        for (uint8 x = 0U; x < LINE_IMAGE_W; x++)
        {
            const uint16 index = (uint16)y * LINE_IMAGE_W + x;
            line_bridge_component_t component;
            uint8 comp_w;
            uint8 comp_h;

            /* 看过了，或者不够黑，跳过 */
            if ((g_line_scratch.visited[index] != 0U) ||
                (gray[index] >= LINE_VISION_BRIDGE_DARK_THRESHOLD))
            {
                continue;
            }

            /* 找出一个完整的黑块 */
            line_flood_dark_component(gray, index, y_min, y_max, &component);
            component_count++;

            comp_w = (uint8)(component.xmax - component.xmin + 1U);
            comp_h = (uint8)(component.ymax - component.ymin + 1U);
            
            /* 如果太小、太细、太空心，就不是桥，淘汰！ */
            if ((component.area < LINE_VISION_BRIDGE_MIN_AREA) ||
                (comp_w < LINE_VISION_BRIDGE_MIN_WIDTH) ||
                (comp_h < LINE_VISION_BRIDGE_MIN_HEIGHT) ||
                (component.fill_ratio < LINE_VISION_BRIDGE_MIN_FILL_RATIO))
            {
                continue;
            }

            /* 给剩下的合格黑块打分，只留分数最高的那一个 */
            component.score = line_score_bridge_component(&component);
            if (component.score > best.score)
            {
                best = component;
            }
        }
    }

    /* 记录一共找到了几个黑块（用来给外面的人调试用） */
    result->bridge_component_count = component_count;
    
    /* 如果最高分达到了及格线，就说明真的看到桥了 */
    if (best.score >= LINE_VISION_BRIDGE_MIN_CONFIDENCE)
    {
        result->bridge_detected = 1U;
        result->bridge_confidence = best.score;
        result->bridge_bbox_xmin = best.xmin;
        result->bridge_bbox_ymin = best.ymin;
        result->bridge_bbox_xmax = best.xmax;
        result->bridge_bbox_ymax = best.ymax;
        /* 告诉 0 核：“我看到桥了，建议车速降到 -90（或者某个指定速度）” */
        result->target_speed_hint = LINE_VISION_BRIDGE_SPEED_HINT;
        return 1U;
    }

    /* 如果没及格，就只是把最好那个的数据填进去，但不算真正看到 */
    result->bridge_confidence = best.score;
    result->bridge_bbox_xmin = best.xmin;
    result->bridge_bbox_ymin = best.ymin;
    result->bridge_bbox_xmax = best.xmax;
    result->bridge_bbox_ymax = best.ymax;
    return 0U;
}

/* --- 6. 单帧检测主流程与防抖 --- */

/**
 * @brief 单张照片的直线/桥梁检测主流程（看一眼照片）
 * 
 * @param gray 灰度照片数据
 * @param result 看完的结果填在这里
 * 
 * @note 逻辑顺序：先找桥。如果找到了桥，就不找直线了；如果没找到桥，再去找直线。
 */
static void line_detect_frame(const uint8 *gray, line_vision_frame_result_t *result)
{
    /* 确定找线的范围：从顶部 25% 的地方开始，到最底下结束 */
    const uint8 y_min = (uint8)((uint32)LINE_IMAGE_H * LINE_VISION_ROI_TOP_RATIO_X100 / 100U);
    const uint8 y_max = (uint8)(LINE_IMAGE_H - 2U);
    /* 远处的预瞄点（在照片的 62% 处）和近处的底部点 */
    const uint8 lookahead_y = (uint8)((uint32)LINE_IMAGE_H * 62U / 100U);
    const uint8 bottom_y = y_max;
    
    uint8 point_count;
    uint8 y_span;
    float mean_width;
    float k = 0.0f;
    float b = 0.0f;
    float rmse = 0.0f;

    /* 1. 先清空结果表 */
    line_clear_frame_result(result);

    /* 2. 找桥梁 */
    line_detect_dark_bridge(gray, result);
    
    /* 3. 找直线（即使找到了桥，也先在后台算一下直线，但如果后面桥确认了，直线的权重会归零） */
    line_build_near_white_mask(gray, y_min, y_max, &result->roi_white_ratio);
    point_count = line_extract_center_points(y_min, y_max, &y_span, &mean_width);

    result->points_used = point_count;
    result->y_span = y_span;
    result->mean_track_width = mean_width;

    /* 4. 如果找到了足够多的直线中心点，就尝试把它们连成线 */
    if ((point_count >= LINE_VISION_MIN_ROWS) &&
        (line_fit_points(point_count, &k, &b, &rmse) != 0U))
    {
        /* 算出底部的横向偏差，以及远处的预瞄偏差 */
        const float line_x_bottom = k * (float)bottom_y + b;
        const float line_x_lookahead = k * (float)lookahead_y + b;
        const float lateral_error_px = line_x_bottom - ((float)LINE_IMAGE_W - 1.0f) * 0.5f; /* 偏差 = 线的位置 - 屏幕中心 */
        const float yaw_error_deg = atanf(k) * 57.29578f; /* 算角度，弧度转度 */
        
        /* 给这条找出的线打分 */
        const float row_score = line_min_f((float)point_count /
                                           ((float)(y_max - y_min + 1U) * 0.70f), 1.0f);
        const float span_score = line_min_f((float)y_span /
                                            ((float)(y_max - y_min) * 0.75f), 1.0f);
        const float rmse_score = line_max_f(0.0f, 1.0f - rmse / 5.5f);
        const float width_score = line_min_f(mean_width / ((float)LINE_IMAGE_W * 0.45f), 1.0f);
        const float center_score = line_max_f(0.0f,
                                              1.0f - line_abs_f(lateral_error_px) /
                                              ((float)LINE_IMAGE_W * 0.55f));

        result->fit_rmse = rmse;
        result->line_x_bottom = line_x_bottom;
        result->line_x_lookahead = line_x_lookahead;
        result->lateral_error_px = lateral_error_px;
        result->yaw_error_deg = yaw_error_deg;
        /* 综合打分：点数(30%) + 跨度(22%) + 误差(24%) + 宽度(14%) + 居中程度(10%) */
        result->confidence = 0.30f * row_score +
                             0.22f * span_score +
                             0.24f * rmse_score +
                             0.14f * width_score +
                             0.10f * center_score;

        /* 如果分数够高，且角度没歪得太离谱，就算真正找到了直线 */
        if ((result->confidence >= LINE_VISION_MIN_CONFIDENCE) &&
            (y_span >= LINE_VISION_MIN_Y_SPAN) &&
            (line_abs_f(yaw_error_deg) <= LINE_VISION_MAX_ABS_YAW_DEG))
        {
            result->detected = 1U;
        }
    }

    /* 5. 最终抉择：桥梁的优先级大于直线 */
    if (result->bridge_detected)
    {
        /* 只要看到桥了，就告诉 0 核：“别管直线了，我现在看到的是桥！” */
        result->detected = 0U;
        result->confidence = 0.0f;
        result->lateral_error_px = 0.0f;
        result->yaw_error_deg = 0.0f;
        result->line_x_bottom = 0.0f;
        result->line_x_lookahead = 0.0f;
        result->target_speed_hint = LINE_VISION_BRIDGE_SPEED_HINT;
    }
}

/**
 * @brief 防抖过滤器（连续确认机制）
 * 
 * @param raw 刚才那“一眼”看到的结果
 * 
 * @note 逻辑和 PVC 模块的防抖一样，为了防止偶尔的一帧误判导致车子抽风。
 */
static void line_update_filter(const line_vision_frame_result_t *raw)
{
    line_vision_output_t next = g_line_output_shadow;

    next.frame_id++; /* 照片序号 +1 */
    next.raw = *raw;
    next.raw_detected = raw->detected;
    next.bridge_raw_detected = raw->bridge_detected;

    /* 1. 桥梁防抖 */
    if (raw->bridge_detected)
    {
        if (next.bridge_detected_streak < 255U)
        {
            next.bridge_detected_streak++;
        }
        next.bridge_lost_streak = 0U;
        /* 连续看到了，确认是桥！ */
        if (next.bridge_detected_streak >= LINE_VISION_BRIDGE_CONFIRM_FRAMES)
        {
            next.bridge_stable_detected = 1U;
        }
    }
    else
    {
        next.bridge_detected_streak = 0U;
        if (next.bridge_lost_streak < 255U)
        {
            next.bridge_lost_streak++;
        }
        /* 连续丢了，确认桥没了！ */
        if (next.bridge_lost_streak >= LINE_VISION_BRIDGE_LOST_HOLD_FRAMES)
        {
            next.bridge_stable_detected = 0U;
        }
    }

    /* 2. 直线防抖（如果已经确认是桥了，直线的防抖就不管了） */
    if ((raw->detected != 0U) && (next.bridge_stable_detected == 0U))
    {
        if (next.detected_streak < 255U)
        {
            next.detected_streak++;
        }
        next.lost_streak = 0U;
        if (next.detected_streak >= LINE_VISION_CONFIRM_FRAMES)
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
        if (next.lost_streak >= LINE_VISION_LOST_HOLD_FRAMES)
        {
            next.stable_detected = 0U;
        }
    }

    /* 3. 整理最终的稳定结果，桥的优先级更高 */
    if (next.bridge_stable_detected)
    {
        if (raw->bridge_detected)
        {
            next.stable = *raw;
        }
        next.stable.bridge_detected = 1U;
        next.stable.detected = 0U;
        next.stable.confidence = 0.0f;
        next.stable.lateral_error_px = 0.0f;
        next.stable.yaw_error_deg = 0.0f;
        next.stable.target_speed_hint = LINE_VISION_BRIDGE_SPEED_HINT;
        next.stable_detected = 0U;
    }
    else if (next.stable_detected)
    {
        if (raw->detected)
        {
            next.stable = *raw;
        }
        next.stable.detected = 1U;
        next.stable.bridge_detected = 0U;
    }
    else
    {
        line_clear_frame_result(&next.stable);
    }

    /* 存入全局变量 */
    g_line_output_shadow = next;
    g_line_vision_output_write_busy = 1U;
    g_line_vision_output = next;
    g_line_vision_output_write_busy = 0U;
}

/* --- 7. 对外公开的函数接口 --- */

/**
 * @brief 模块初始化
 */
void line_vision_init(void)
{
    /* 忘掉历史记录 */
    line_vision_reset_filter();
#if LINE_VISION_PROFILE_ENABLE
    /* 初始化用来掐表的定时器 */
    RUNTIME_PROFILE_RESET(&g_line_vision_cost_profiler);
    RUNTIME_PROFILE_RESET(&g_line_vision_frame_profiler);
    g_line_last_frame_time_us = timer_get(LINE_VISION_PROFILE_TIMER);
#endif
}

/**
 * @brief 重置防抖记录
 */
void line_vision_reset_filter(void)
{
    line_vision_output_t empty;

    memset(&empty, 0, sizeof(empty));
    line_clear_frame_result(&empty.raw);
    line_clear_frame_result(&empty.stable);
    g_line_output_shadow = empty;
    g_line_vision_output_write_busy = 1U;
    g_line_vision_output = empty;
    g_line_vision_output_write_busy = 0U;
}

/**
 * @brief 获取当前的检测结果
 */
const volatile line_vision_output_t *line_vision_get_output(void)
{
    return &g_line_vision_output;
}

/**
 * @brief 处理一张新照片（外部每来一张照片就调用一次）
 * 
 * @param gray 压缩后的灰度照片（94x60）
 */
void line_vision_process_camera_frame(const uint8 *gray)
{
    line_vision_frame_result_t raw;

    if (gray == NULL)
    {
        return;
    }

#if LINE_VISION_PROFILE_ENABLE
    {
        /* 算一下离上一张照片过了多久 */
        const uint32 now_us = timer_get(LINE_VISION_PROFILE_TIMER);
        runtime_profiler_update(&g_line_vision_frame_profiler, (uint32)(now_us - g_line_last_frame_time_us));
        g_line_last_frame_time_us = now_us;
    }
    /* 开始掐表 */
    RUNTIME_PROFILE_BEGIN(g_line_vision_cost_profiler, LINE_VISION_PROFILE_TIMER);
#endif

    /* 1. 看一眼照片 */
    line_detect_frame(gray, &raw);
    /* 2. 防抖处理 */
    line_update_filter(&raw);

#if LINE_VISION_PROFILE_ENABLE
    /* 掐表结束 */
    RUNTIME_PROFILE_END(&g_line_vision_cost_profiler, LINE_VISION_PROFILE_TIMER);
#endif

#if (LINE_VISION_DEBUG_PRINT_EVERY > 0U)
    /* 如果开启了打印，就定期往电脑发数据 */
    if ((g_line_vision_output.frame_id % LINE_VISION_DEBUG_PRINT_EVERY) == 0U)
    {
        printf("[LINE] frame=%lu line=%u bridge=%u conf=%d bconf=%d cost=%lu us\r\n",
               (unsigned long)g_line_vision_output.frame_id,
               g_line_vision_output.stable_detected,
               g_line_vision_output.bridge_stable_detected,
               (int)(g_line_vision_output.raw.confidence * 1000.0f),
               (int)(g_line_vision_output.raw.bridge_confidence * 1000.0f),
               (unsigned long)g_line_vision_cost_profiler.last_us);
    }
#endif
}

#endif
