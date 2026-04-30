/*
 * =================================================================================
 * 文件: pvc_vision.c
 * 作用: 1 核 (Core 1) PVC 入口视觉检测模块的具体实现代码。
 * 说明: 这里是真正的“干活”的地方。主要步骤如下：
 *       1. 找到画面里所有白色的块（连通域提取）。
 *       2. 挑出长得最像 PVC 入口的那一块（根据面积、长宽、位置打分）。
 *       3. 算一算它离车多远、偏左还是偏右。
 *       4. 连续看好几张照片，确认是不是真的 PVC（防抖平滑）。
 * =================================================================================
 */
#include "pvc_vision.h"

#if PVC_VISION_ENABLE

/* --- 1. 内部数据结构定义 --- */

/**
 * @brief 单个白色连通域（白斑）的内部描述
 * @note  就像是给每一个白斑建了一个“档案”，记录它的特征，用来打分。
 */
typedef struct
{
    uint16 area;           /* 面积：包含多少个白色的像素点 */
    uint8 xmin;            /* 最左边在哪 */
    uint8 ymin;            /* 最上边在哪 */
    uint8 xmax;            /* 最右边在哪 */
    uint8 ymax;            /* 最下边在哪 */
    float centroid_x;      /* 重心的横坐标（它的中心点在哪） */
    float centroid_y;      /* 重心的纵坐标 */
    float fill_ratio;      /* 填充率：如果是实心的，值就接近 1；如果是空心的环，值就很小 */
    uint8 touches_border;  /* 有没有碰到照片的边缘？（PVC 刚出现时通常在照片边缘） */
    float mean_gray;       /* 平均有多亮（越接近 255 越好） */
    float score;           /* 算法给它打的“长得像不像 PVC”的综合分数 */
} pvc_component_t;

/**
 * @brief 处理每一张照片时用到的临时“草稿纸”
 * @note  放在全局区是为了防止撑爆内存栈。
 */
typedef struct
{
    uint8 visited[PVC_IMAGE_SIZE];                           /* 记录每个像素点是不是已经看过了，防止重复看 */
    uint16 stack[PVC_IMAGE_SIZE];                            /* 找白斑时用的“记忆栈”，记录接下来要去哪找 */
    pvc_component_t components[PVC_VISION_MAX_COMPONENTS];   /* 画面里所有的白斑档案 */
    pvc_component_t candidates[PVC_VISION_MAX_COMPONENTS];   /* 淘汰掉太小的白斑后，剩下的候选名单 */
} pvc_scratch_t;

/* --- 2. 全局变量（对外公开的数据） --- */
/* 这些是给其他模块（比如 0 核）看的最终结果和统计信息 */
volatile runtime_profiler_t g_pvc_vision_cost_profiler = {0};   /* 算法花了多少时间 */
volatile runtime_profiler_t g_pvc_vision_frame_profiler = {0};  /* 照片来了有多快 */
volatile pvc_vision_output_t g_pvc_vision_output = {0};         /* 最终的识别结果 */
volatile uint8 g_pvc_vision_output_write_busy = 0U;             /* 正在写数据的锁，防冲突 */

/* --- 3. 内部全局变量（不对外公开的私有数据） --- */
static pvc_scratch_t g_pvc_scratch;                             /* 刚才说的“草稿纸” */
static pvc_vision_output_t g_pvc_output_shadow;                 /* 自己用的备份结果，算完再一起拷贝给外面 */

/* 防抖平滑器（让距离和偏差数据不要跳来跳去） */
static uint8 g_pvc_smooth_inited = 0U;                          /* 平滑器是不是刚开始工作 */
static int16 g_pvc_smooth_forward_mm = -1;                      /* 平滑后的距离 */
static int16 g_pvc_smooth_lateral_mm = 0;                       /* 平滑后的横向偏差 */
static uint32 g_pvc_last_frame_time_us = 0U;                    /* 上一张照片是啥时候来的 */

/* --- 4. 基础数学工具函数 --- */

/**
 * @brief 取两个浮点数里较小的一个
 */
static float pvc_min_f(float a, float b)
{
    return (a < b) ? a : b;
}

/**
 * @brief 取两个浮点数里较大的一个
 */
static float pvc_max_f(float a, float b)
{
    return (a > b) ? a : b;
}

/**
 * @brief 计算一个白斑的宽度
 */
static uint8 pvc_component_width(const pvc_component_t *component)
{
    return (uint8)(component->xmax - component->xmin + 1U);
}

/**
 * @brief 计算一个白斑的高度
 */
static uint8 pvc_component_height(const pvc_component_t *component)
{
    return (uint8)(component->ymax - component->ymin + 1U);
}

/* --- 5. 核心逻辑：估算距离与偏差 --- */

/**
 * @brief 根据 PVC 白边在照片里的高度，估算车离入口还有多远
 * 
 * @param row 照片中 PVC 最下边的行号（照片越往下行号越大，说明离车越近）
 * @return int16 估算出的前向距离（毫米）
 * 
 * @note 【新手注意】这个只是个简单的粗略公式。
 *       真实情况是：你得把车摆在离入口 10cm、20cm、30cm 的地方，
 *       看看 row 是多少，然后做个表格（查表法）或者拟合一个曲线，才能算得准。
 */
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

/**
 * @brief 根据 PVC 白斑在照片里的左右位置，估算车偏离中心多少
 * 
 * @param x 照片中 PVC 的中心横坐标
 * @return int16 估算出的横向偏差（毫米）。正数说明车偏左了（所以要向右打方向），负数反之。
 * 
 * @note 【优化点】这也是个粗略公式。加入逆透视查表实现
 *       如果跑偏了，建议去调 0 核里面的控制参数，不要先改这里的视觉公式。
 */
static int16 pvc_estimate_lateral_mm_from_x(float x)
{
    /*
     * 当前做法：
     * - 照片宽度的一半就是中心（0点）。
     * - 每偏离中心 1 个像素，算作现实中的 8 毫米。
     */
    return (int16)((x - ((float)PVC_IMAGE_W - 1.0f) * 0.5f) * 8.0f);
}

/* --- 6. 核心逻辑：打分与排序 --- */

/**
 * @brief 把一帧的识别结果清零
 * @note 找不到 PVC 时，用这个函数把数据都设成“无效”状态。
 */
static void pvc_clear_frame_result(pvc_vision_frame_result_t *result)
{
    /* 全部填 0，然后再把表示位置的值填成 0xFF（255），因为坐标不可能是 255 */
    memset(result, 0, sizeof(*result));
    result->bbox_xmin = 0xFFU;
    result->bbox_ymin = 0xFFU;
    result->bbox_xmax = 0xFFU;
    result->bbox_ymax = 0xFFU;
    result->entry_bottom_y = 0xFFU;
    result->entry_top_y = 0xFFU;
    result->forward_mm = -1; /* 距离设为 -1 表示未知 */
}

/**
 * @brief 给一个白斑打分，看看它到底多像 PVC 入口
 * 
 * @param component 要打分的白斑“档案”
 * @return float 分数（0.0 ~ 1.0）
 * 
 * @note 就像选美比赛一样，根据多个方面综合打分：
 *       - 面积：越大越好（占比 35%）
 *       - 宽度：越宽越好（占比 18%）
 *       - 高度：有点高度最好（占比 12%）
 *       - 实心度：不能是碎的（占比 12%）
 *       - 是否靠边：PVC 都是从照片边缘进入视野的（占比 18%）
 *       - 亮度：越亮越好（占比 5%）
 */
static float pvc_score_component(const pvc_component_t *component)
{
    /* 计算各项单项分，最高不超过 1.0 */
    const float area_score = pvc_min_f((float)component->area / 600.0f, 1.0f);
    const float width_score = pvc_min_f((float)pvc_component_width(component) / 45.0f, 1.0f);
    const float height_score = pvc_min_f((float)pvc_component_height(component) / 18.0f, 1.0f);
    const float fill_score = pvc_min_f(component->fill_ratio / 0.55f, 1.0f);
    const float border_score = component->touches_border ? 1.0f : 0.0f;
    const float brightness_score = pvc_min_f(
        pvc_max_f((component->mean_gray - 235.0f) / 20.0f, 0.0f),
        1.0f);

    /* 乘以各自的权重，加起来得到总分 */
    return 0.35f * area_score
         + 0.18f * width_score
         + 0.12f * height_score
         + 0.12f * fill_score
         + 0.18f * border_score
         + 0.05f * brightness_score;
}

/**
 * @brief 把所有的白斑按照分数从高到低排队（冒泡/插入排序）
 */
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

/**
 * @brief 把所有的白斑按照面积从大到小排队
 */
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

/* --- 7. 核心逻辑：找白斑与筛选 --- */

/**
 * @brief 在照片上找到一块连在一起的白色区域（找白斑）
 * 
 * @param gray 灰度照片数据（94x60）
 * @param start_index 从照片的哪个点开始找
 * @param out 找完之后，把这个白斑的特征填到这个档案里
 * 
 * @note 【算法解释：Flood Fill 漫水填充法】
 *       就像用画图软件的“油漆桶”工具。点一下白色的地方，
 *       程序就会自动往上下左右找，把所有连在一起的白色点都涂上颜色，
 *       顺便记下它们一共有多大、最靠左/右/上/下在哪里。
 *       这里没有用“递归”，而是用了一个数组当做“栈”，防止车机的内存爆掉。
 */
static void pvc_flood_component(const uint8 *gray, uint16 start_index, pvc_component_t *out)
{
    uint16 stack_top = 0U; /* 栈顶指针，记录栈里有几个点 */
    uint16 area = 0U;      /* 这个白斑一共有多少个像素 */
    uint8 xmin = (uint8)(PVC_IMAGE_W - 1U); /* 初始化为最大值 */
    uint8 ymin = (uint8)(PVC_IMAGE_H - 1U); /* 初始化为最大值 */
    uint8 xmax = 0U;       /* 初始化为最小值 */
    uint8 ymax = 0U;       /* 初始化为最小值 */
    uint32 sum_x = 0U;     /* 累加所有 x 坐标，算重心用 */
    uint32 sum_y = 0U;     /* 累加所有 y 坐标，算重心用 */
    uint32 sum_gray = 0U;  /* 累加亮度，算平均亮度用 */

    /* 把第一个点放进栈，并标记为已访问 */
    g_pvc_scratch.stack[stack_top++] = start_index;
    g_pvc_scratch.visited[start_index] = 1U;

    /* 只要栈里还有点没处理完，就一直循环 */
    while (stack_top > 0U)
    {
        /* 从栈里拿出一个点 */
        const uint16 index = g_pvc_scratch.stack[--stack_top];
        /* 算出这个点的 x（列）和 y（行） */
        const uint8 y = (uint8)(index / PVC_IMAGE_W);
        const uint8 x = (uint8)(index - (uint16)y * PVC_IMAGE_W);

        /* 统计数据累加 */
        area++;
        sum_x += x;
        sum_y += y;
        sum_gray += gray[index];

        /* 更新上下左右的边界 */
        if (x < xmin) { xmin = x; }
        if (x > xmax) { xmax = x; }
        if (y < ymin) { ymin = y; }
        if (y > ymax) { ymax = y; }

        /* 检查上面的邻居点 */
        if (y > 0U)
        {
            const uint16 ni = (uint16)(index - PVC_IMAGE_W);
            /* 如果没访问过，且也是白色，就放进栈里待会儿处理 */
            if ((g_pvc_scratch.visited[ni] == 0U) && (gray[ni] >= PVC_VISION_WHITE_THRESHOLD))
            {
                g_pvc_scratch.visited[ni] = 1U;
                g_pvc_scratch.stack[stack_top++] = ni;
            }
        }
        /* 检查下面的邻居点 */
        if (y < (PVC_IMAGE_H - 1U))
        {
            const uint16 ni = (uint16)(index + PVC_IMAGE_W);
            if ((g_pvc_scratch.visited[ni] == 0U) && (gray[ni] >= PVC_VISION_WHITE_THRESHOLD))
            {
                g_pvc_scratch.visited[ni] = 1U;
                g_pvc_scratch.stack[stack_top++] = ni;
            }
        }
        /* 检查左边的邻居点 */
        if (x > 0U)
        {
            const uint16 ni = (uint16)(index - 1U);
            if ((g_pvc_scratch.visited[ni] == 0U) && (gray[ni] >= PVC_VISION_WHITE_THRESHOLD))
            {
                g_pvc_scratch.visited[ni] = 1U;
                g_pvc_scratch.stack[stack_top++] = ni;
            }
        }
        /* 检查右边的邻居点 */
        if (x < (PVC_IMAGE_W - 1U))
        {
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
        out->centroid_x = (float)sum_x / (float)area; /* 计算平均 x 作为重心 */
        out->centroid_y = (float)sum_y / (float)area; /* 计算平均 y 作为重心 */
        out->fill_ratio = (float)area / (float)bbox_area; /* 计算填充率 */
        /* 判断有没有碰到照片边缘 */
        out->touches_border = (uint8)((xmin == 0U) || (ymin == 0U) || (xmax == (PVC_IMAGE_W - 1U)) || (ymax == (PVC_IMAGE_H - 1U)));
        out->mean_gray = (float)sum_gray / (float)area; /* 计算平均亮度 */
        out->score = 0.0f; /* 此时还没打分 */
    }
}

/**
 * @brief 扫描整张照片，找出所有的白斑
 * 
 * @param gray 灰度照片数据
 * @return uint8 找出了几个白斑
 */
static uint8 pvc_collect_components(const uint8 *gray)
{
    uint8 component_count = 0U;

    /* 每处理一帧新照片前，先把“草稿纸”擦干净 */
    memset(g_pvc_scratch.visited, 0, sizeof(g_pvc_scratch.visited));

    /* 逐个像素扫描整张图（94x60） */
    for (uint16 i = 0U; i < PVC_IMAGE_SIZE; i++)
    {
        /* 如果这个点已经看过了，或者不是白色，就跳过 */
        if ((g_pvc_scratch.visited[i] != 0U) || (gray[i] < PVC_VISION_WHITE_THRESHOLD))
        {
            continue;
        }

        /* 发现了一个新的白色点，开始用“油漆桶”工具找出整个白斑 */
        if (component_count < PVC_VISION_MAX_COMPONENTS)
        {
            pvc_flood_component(gray, i, &g_pvc_scratch.components[component_count]);
            component_count++;
        }
        else
        {
            /* 
             * 如果画面太乱，白斑太多超过了限制，
             * 为了防止程序卡死，就只标记一下，不再去详细找它了。
             */
            g_pvc_scratch.visited[i] = 1U;
        }
    }

    /* 找完之后，按照白斑的大小（面积）排个队，大的在前面 */
    pvc_sort_by_area(g_pvc_scratch.components, component_count);
    return component_count;
}

/**
 * @brief 筛选白斑，淘汰那些明显不是 PVC 的，并对剩下的打分排序
 * 
 * @param component_count 刚才找到了几个白斑
 * @return uint8 剩下几个合格的候选者
 */
static uint8 pvc_filter_candidates(uint8 component_count)
{
    uint8 candidate_count = 0U;

    /* 挨个检查每一个白斑 */
    for (uint8 i = 0U; i < component_count; i++)
    {
        pvc_component_t component = g_pvc_scratch.components[i];
        
        /* 给这个白斑打个分 */
        component.score = pvc_score_component(&component);

        /* 【硬性淘汰条件】 */
        /* 面积太小，多半是地上的反光小点或噪声，淘汰！ */
        if (component.area < PVC_VISION_MIN_AREA)
        {
            continue;
        }
        /* 太窄或太矮，不是一大块板子，淘汰！ */
        if ((pvc_component_width(&component) < PVC_VISION_MIN_WIDTH) ||
            (pvc_component_height(&component) < PVC_VISION_MIN_HEIGHT))
        {
            continue;
        }
        /* 填充率太低（比如是个空心圈），不像完整的 PVC 板，淘汰！ */
        if (component.fill_ratio < PVC_VISION_MIN_FILL_RATIO)
        {
            continue;
        }
        /* 没挨着照片边缘？PVC 入口应该是由远及近从画面边缘进入的，淘汰！ */
        if (component.touches_border == 0U)
        {
            continue;
        }

        /* 活下来的都是“好苗子”，放进候选名单 */
        if (candidate_count < PVC_VISION_MAX_COMPONENTS)
        {
            g_pvc_scratch.candidates[candidate_count++] = component;
        }
    }

    /* 把候选者按照分数从高到低排队，最好的在最前面 */
    pvc_sort_by_score(g_pvc_scratch.candidates, candidate_count);
    return candidate_count;
}

/* --- 8. 核心逻辑：组装单帧结果与防抖 --- */

/**
 * @brief 把内部的“白斑档案”转换成对外输出的格式
 * 
 * @param best 得分最高的那一块白斑（最像 PVC 的）
 * @param result 要填写的输出结果表
 */
static void pvc_copy_best_to_result(const pvc_component_t *best, pvc_vision_frame_result_t *result)
{
    /* 抄写基本数据 */
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
    
    /* 调用前面的估算函数，算出距离和偏差 */
    result->forward_mm = pvc_estimate_forward_mm_from_row(best->ymax);
    result->lateral_mm = pvc_estimate_lateral_mm_from_x(best->centroid_x);
    result->yaw_error_deg_x100 = 0; /* 目前没有算角度，填 0 */
}

/**
 * @brief 处理单张照片的主流程（看一眼照片）
 * 
 * @param gray 灰度照片数据
 * @param result 看完的结果填在这里
 */
static void pvc_detect_frame(const uint8 *gray, pvc_vision_frame_result_t *result)
{
    /* 1. 找出所有白斑 */
    const uint8 component_count = pvc_collect_components(gray);
    /* 2. 筛选并打分排队 */
    const uint8 candidate_count = pvc_filter_candidates(component_count);

    /* 3. 先把结果表清空 */
    pvc_clear_frame_result(result);
    result->component_count = component_count;
    result->candidate_count = candidate_count;

    /* 4. 如果有合格的候选者，取分数最高的第一名 */
    if (candidate_count > 0U)
    {
        const pvc_component_t *best = &g_pvc_scratch.candidates[0];
        pvc_copy_best_to_result(best, result);
        
        /* 5. 只有分数达到及格线，才算真的看到了 PVC */
        result->detected = (uint8)(best->score >= PVC_VISION_MIN_DECISION_SCORE);
        
        /* 如果没达到及格线，距离就不算数了 */
        if (result->detected == 0U)
        {
            result->forward_mm = -1;
        }
    }
}

/**
 * @brief 防抖过滤器（不能看一眼就信，得多看几眼确认）
 * 
 * @param raw 刚才那“一眼”看到的结果
 * 
 * @note 为什么需要这个？因为有时候地上的反光会恰好长得像 PVC，
 *       或者真的 PVC 因为曝光问题突然在照片里黑了一下。
 *       这会导致车子“抽风”。所以我们要“连续几帧都看到才算数”，
 *       以及“偶尔一两帧没看到，别急着放弃”。
 */
static void pvc_update_filter(const pvc_vision_frame_result_t *raw)
{
    /* 拿出之前的历史记录 */
    pvc_vision_output_t next = g_pvc_output_shadow;

    next.frame_id++; /* 照片编号加 1 */
    next.raw = *raw; /* 记录刚才这一眼的原始结果 */
    next.raw_detected = raw->detected;

#if PVC_VISION_SMOOTH_ENABLE
    /* 如果这一眼看到了 */
    if (raw->detected)
    {
        if (next.detected_streak < 255U)
        {
            next.detected_streak++; /* 连续看到的次数 +1 */
        }
        next.lost_streak = 0U; /* 连续丢失的次数清零 */

        /* 如果是刚开始平滑 */
        if (g_pvc_smooth_inited == 0U)
        {
            /* 第一次看到，直接相信当前的数据 */
            g_pvc_smooth_forward_mm = raw->forward_mm;
            g_pvc_smooth_lateral_mm = raw->lateral_mm;
            g_pvc_smooth_inited = 1U;
        }
        else
        {
            /*
             * IIR 平滑算法（老数据占大头，新数据占小头）
             * 新的平滑值 = (老值 * 3 + 新值 * 1) / 4
             * 这样数据变化就不会太猛。
             */
            g_pvc_smooth_forward_mm = (int16)((3 * g_pvc_smooth_forward_mm + raw->forward_mm) / 4);
            g_pvc_smooth_lateral_mm = (int16)((3 * g_pvc_smooth_lateral_mm + raw->lateral_mm) / 4);
        }

        /* 连续看到的次数达到了我们要求的标准，宣布：真正看到了！ */
        if (next.detected_streak >= PVC_VISION_CONFIRM_FRAMES)
        {
            next.stable_detected = 1U;
        }
    }
    else /* 如果这一眼没看到 */
    {
        next.detected_streak = 0U; /* 连续看到的次数清零 */
        if (next.lost_streak < 255U)
        {
            next.lost_streak++; /* 连续丢失的次数 +1 */
        }
        
        /* 如果连续好几眼都没看到，宣布：真的丢了！ */
        if (next.lost_streak >= PVC_VISION_LOST_HOLD_FRAMES)
        {
            next.stable_detected = 0U;
            g_pvc_smooth_inited = 0U; /* 结束平滑状态 */
        }
    }

    /* 整理要对外输出的稳定结果 */
    if (next.stable_detected)
    {
        /* 如果当前这帧原始数据有效，就把包围框等信息也更新一下 */
        if (raw->detected)
        {
            next.stable = *raw;
        }
        next.stable.detected = 1U;
        /* 距离和偏差用平滑过的，更稳 */
        next.stable.forward_mm = g_pvc_smooth_forward_mm;
        next.stable.lateral_mm = g_pvc_smooth_lateral_mm;
    }
    else
    {
        /* 没看到的话，结果清空 */
        pvc_clear_frame_result(&next.stable);
    }
#else
    /* 如果没开防抖，那“看一眼”的结果就是最终结果 */
    next.detected_streak = raw->detected ? 1U : 0U;
    next.lost_streak = raw->detected ? 0U : 1U;
    next.stable_detected = raw->detected;
    next.stable = *raw;
#endif

    /* 把算好的结果放进全局变量，供别人读取 */
    g_pvc_output_shadow = next;
    g_pvc_vision_output_write_busy = 1U; /* 挂上“正在写”的牌子 */
    g_pvc_vision_output = next;
    g_pvc_vision_output_write_busy = 0U; /* 摘下“正在写”的牌子 */
}

/* --- 9. 对外公开的函数接口 --- */

/**
 * @brief 模块初始化
 */
void pvc_vision_init(void)
{
    /* 先把以前的记忆都忘掉 */
    pvc_vision_reset_filter();
#if PVC_VISION_PROFILE_ENABLE
    /* 初始化用来计时的秒表 */
    timer_init(PVC_VISION_PROFILE_TIMER, TIMER_US);
    timer_start(PVC_VISION_PROFILE_TIMER);
    RUNTIME_PROFILE_RESET(&g_pvc_vision_cost_profiler);
    RUNTIME_PROFILE_RESET(&g_pvc_vision_frame_profiler);
    g_pvc_last_frame_time_us = timer_get(PVC_VISION_PROFILE_TIMER);
#endif
}

/**
 * @brief 重置防抖过滤器（忘掉过去的记忆）
 */
void pvc_vision_reset_filter(void)
{
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

/**
 * @brief 获取当前的检测结果
 */
const volatile pvc_vision_output_t *pvc_vision_get_output(void)
{
    return &g_pvc_vision_output;
}

/**
 * @brief 处理一张新照片（外部每来一张照片就调用一次这个函数）
 * 
 * @param gray 压缩后的灰度照片（94x60）
 */
void pvc_vision_process_camera_frame(const uint8 *gray)
{
    pvc_vision_frame_result_t raw;

    /* 如果没传照片过来，直接退出 */
    if (gray == NULL)
    {
        return;
    }

#if PVC_VISION_PROFILE_ENABLE
    {
        /* 算一下距离上一张照片过去了多久（帧间隔） */
        const uint32 now_us = timer_get(PVC_VISION_PROFILE_TIMER);
        runtime_profiler_update(&g_pvc_vision_frame_profiler, (uint32)(now_us - g_pvc_last_frame_time_us));
        g_pvc_last_frame_time_us = now_us;
    }
    /* 开始掐表，算算这次处理花了多长时间 */
    RUNTIME_PROFILE_BEGIN(g_pvc_vision_cost_profiler, PVC_VISION_PROFILE_TIMER);
#endif

    /* 1. 看一眼照片，得出原始结果 */
    pvc_detect_frame(gray, &raw);
    /* 2. 经过防抖处理，得出最终结果 */
    pvc_update_filter(&raw);

#if PVC_VISION_PROFILE_ENABLE
    /* 掐表结束 */
    RUNTIME_PROFILE_END(&g_pvc_vision_cost_profiler, PVC_VISION_PROFILE_TIMER);
#endif

#if (PVC_VISION_DEBUG_PRINT_EVERY > 0U)
    /* 如果开启了打印，就定期在串口输出一下情况 */
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
