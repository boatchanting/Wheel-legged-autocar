#include "pvc_detector.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

static float pvc_min_f(float a, float b)
{
    return a < b ? a : b;
}

static float pvc_max_f(float a, float b)
{
    return a > b ? a : b;
}

static int pvc_component_width(const PvcComponent *component)
{
    return component->xmax - component->xmin + 1;
}

static int pvc_component_height(const PvcComponent *component)
{
    return component->ymax - component->ymin + 1;
}

void pvc_detect_result_clear(PvcDetectResult *result)
{
    if (result == NULL) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->bbox_xmin = -1;
    result->bbox_ymin = -1;
    result->bbox_xmax = -1;
    result->bbox_ymax = -1;
    result->entry_bottom_y = -1;
    result->entry_top_y = -1;
}

static float pvc_score_component(const PvcComponent *component)
{
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

static void pvc_sort_components_by_score(PvcComponent *components, int count)
{
    for (int i = 1; i < count; i++) {
        PvcComponent key = components[i];
        int j = i - 1;
        while (j >= 0 && components[j].score < key.score) {
            components[j + 1] = components[j];
            j--;
        }
        components[j + 1] = key;
    }
}

static void pvc_sort_components_by_area(PvcComponent *components, int count)
{
    for (int i = 1; i < count; i++) {
        PvcComponent key = components[i];
        int j = i - 1;
        while (j >= 0 && components[j].area < key.area) {
            components[j + 1] = components[j];
            j--;
        }
        components[j + 1] = key;
    }
}

static void pvc_flood_component(
    const uint8_t *gray,
    int width,
    int height,
    int start_index,
    PvcDetectScratch *scratch,
    PvcComponent *out)
{
    int stack_top = 0;
    int area = 0;
    int xmin = width - 1;
    int ymin = height - 1;
    int xmax = 0;
    int ymax = 0;
    int sum_x = 0;
    int sum_y = 0;
    int sum_gray = 0;

    scratch->stack[stack_top++] = start_index;
    scratch->visited[start_index] = 1;

    while (stack_top > 0) {
        const int index = scratch->stack[--stack_top];
        const int y = index / width;
        const int x = index - y * width;

        area++;
        sum_x += x;
        sum_y += y;
        sum_gray += gray[index];

        if (x < xmin) xmin = x;
        if (x > xmax) xmax = x;
        if (y < ymin) ymin = y;
        if (y > ymax) ymax = y;

        const int neighbors[4] = {
            index - width,
            index + width,
            index - 1,
            index + 1,
        };

        for (int n = 0; n < 4; n++) {
            const int ni = neighbors[n];
            int valid = 1;

            if (n == 0 && y == 0) valid = 0;
            if (n == 1 && y == height - 1) valid = 0;
            if (n == 2 && x == 0) valid = 0;
            if (n == 3 && x == width - 1) valid = 0;
            if (!valid) {
                continue;
            }

            if (!scratch->visited[ni] && gray[ni] >= PVC_WHITE_THRESHOLD) {
                scratch->visited[ni] = 1;
                scratch->stack[stack_top++] = ni;
            }
        }
    }

    const int bbox_area = (xmax - xmin + 1) * (ymax - ymin + 1);
    out->area = area;
    out->xmin = xmin;
    out->ymin = ymin;
    out->xmax = xmax;
    out->ymax = ymax;
    out->centroid_x = (float)sum_x / (float)area;
    out->centroid_y = (float)sum_y / (float)area;
    out->fill_ratio = (float)area / (float)bbox_area;
    out->touches_border = (uint8_t)(xmin == 0 || ymin == 0 || xmax == width - 1 || ymax == height - 1);
    out->mean_gray = (float)sum_gray / (float)area;
    out->score = 0.0f;
}

static int pvc_collect_components(
    const uint8_t *gray,
    int width,
    int height,
    PvcDetectScratch *scratch)
{
    const int pixels = width * height;
    int component_count = 0;
    memset(scratch->visited, 0, (size_t)pixels);

    for (int i = 0; i < pixels; i++) {
        if (scratch->visited[i] || gray[i] < PVC_WHITE_THRESHOLD) {
            continue;
        }

        if (component_count < PVC_MAX_COMPONENTS) {
            pvc_flood_component(gray, width, height, i, scratch, &scratch->components[component_count]);
            component_count++;
        } else {
            scratch->visited[i] = 1;
        }
    }

    pvc_sort_components_by_area(scratch->components, component_count);
    return component_count;
}

static int pvc_filter_candidates(
    const PvcComponent *components,
    int component_count,
    PvcDetectScratch *scratch)
{
    int candidate_count = 0;
    for (int i = 0; i < component_count; i++) {
        PvcComponent component = components[i];
        component.score = pvc_score_component(&component);

        if (component.area < PVC_MIN_AREA) {
            continue;
        }
        if (pvc_component_width(&component) < PVC_MIN_WIDTH ||
            pvc_component_height(&component) < PVC_MIN_HEIGHT) {
            continue;
        }
        if (component.fill_ratio < PVC_MIN_FILL_RATIO) {
            continue;
        }
        if (!component.touches_border) {
            continue;
        }

        if (candidate_count < PVC_MAX_COMPONENTS) {
            scratch->candidates[candidate_count++] = component;
        }
    }

    pvc_sort_components_by_score(scratch->candidates, candidate_count);
    return candidate_count;
}

float pvc_estimate_forward_mm_from_row(int row, int image_height)
{
    if (row < 0 || image_height <= 1) {
        return -1.0f;
    }
    /* Placeholder compatible with the Python debugging convention.
       Replace this with the measured row->distance table before on-car use. */
    return (float)(image_height - 1 - row) * 20.0f;
}

float pvc_estimate_lateral_mm_from_x(float x, int image_width)
{
    if (image_width <= 1) {
        return 0.0f;
    }
    return (x - ((float)image_width - 1.0f) * 0.5f) * 8.0f;
}

static void pvc_copy_best_to_result(
    const PvcComponent *best,
    int width,
    int height,
    PvcDetectResult *result)
{
    result->area = best->area;
    result->bbox_xmin = best->xmin;
    result->bbox_ymin = best->ymin;
    result->bbox_xmax = best->xmax;
    result->bbox_ymax = best->ymax;
    result->centroid_x = best->centroid_x;
    result->centroid_y = best->centroid_y;
    result->fill_ratio = best->fill_ratio;
    result->touches_border = best->touches_border;
    result->mean_gray = best->mean_gray;
    result->entry_bottom_y = best->ymax;
    result->entry_top_y = best->ymin;
    result->confidence = best->score;
    result->forward_mm = pvc_estimate_forward_mm_from_row(best->ymax, height);
    result->lateral_mm = pvc_estimate_lateral_mm_from_x(best->centroid_x, width);
    result->yaw_error_deg = 0.0f;
}

int pvc_detect_frame_gray(
    const uint8_t *gray,
    int width,
    int height,
    PvcDetectScratch *scratch,
    PvcDetectResult *result)
{
    int component_count;
    int candidate_count;

    if (gray == NULL || scratch == NULL || result == NULL) {
        return -1;
    }
    if (width <= 0 || height <= 0 || width > PVC_MAX_WIDTH || height > PVC_MAX_HEIGHT) {
        return -2;
    }

    pvc_detect_result_clear(result);
    component_count = pvc_collect_components(gray, width, height, scratch);
    candidate_count = pvc_filter_candidates(scratch->components, component_count, scratch);

    result->component_count = component_count;
    result->candidate_count = candidate_count;

    if (candidate_count > 0) {
        const PvcComponent *best = &scratch->candidates[0];
        pvc_copy_best_to_result(best, width, height, result);
        result->detected = (uint8_t)(best->score >= PVC_MIN_DECISION_SCORE);
        if (!result->detected) {
            result->forward_mm = -1.0f;
        }
    }

    return 0;
}
