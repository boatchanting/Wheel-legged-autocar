#include "camera_menu.h"

#include "../../code/config/config.h"
#include "../wifi.h"
#include "../vision/bumpy_vision.h"
#include "../vision/bridge_vision.h"
#include "../vision/pvc_vision.h"
#include "../vision/vision_ipc_core1.h"

#define CAMERA_MENU_IPS200_TYPE      (IPS200_TYPE_SPI)
#define CAMERA_MENU_IMAGE_X          (26U)
#define CAMERA_MENU_IMAGE_Y          (16U)
#define CAMERA_MENU_IMAGE_DIS_W      (188U)
#define CAMERA_MENU_IMAGE_DIS_H      (120U)
#define CAMERA_MENU_TEXT_Y_BASE      (145U)
#define CAMERA_MENU_TEXT_Y_STEP      (15U)

#if DEBUG_DISPLAY_CORE1

static uint8 g_camera_menu_inited = 0U;
static uint32 g_camera_menu_refresh_counter = 0U;
#if CAMERA_MENU_DEBUG_LOG_ENABLE
static uint32 g_camera_menu_log_counter = 0U;
#endif

static uint32 CameraMenu_MaxFrameId(uint32 a, uint32 b, uint32 c)
{
    uint32 max_value = (a > b) ? a : b;
    return (max_value > c) ? max_value : c;
}

static uint32 CameraMenu_ProfileMinOrZero(const volatile runtime_profiler_t *profiler)
{
    return (profiler->count == 0U) ? 0U : profiler->min_us;
}

static const char *CameraMenu_TargetToString(uint8 active_target)
{
    switch (active_target)
    {
        case VISION_TARGET_PVC_ENTRY:  return "PVC     ";
        case VISION_TARGET_BRIDGE:     return "BRIDGE  ";
        case VISION_TARGET_BUMPY:      return "BUMPY   ";
        case VISION_TARGET_STAIR_UP:   return "STAIR UP";
        case VISION_TARGET_STAIR_DOWN: return "STAIR DN";
        case VISION_TARGET_GRASS:      return "GRASS   ";
        default:                       return "IDLE    ";
    }
}

static void CameraMenu_DrawStaticLayout(void)
{
    const uint16 y = CAMERA_MENU_TEXT_Y_BASE;

    ips200_clear();
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(42, 0, "Core1 Camera Debug");
    ips200_draw_line(0, 14, 239, 14, RGB565_RED);
    ips200_draw_line(0, 138, 239, 138, RGB565_RED);

    ips200_show_string(0, y + 0U * CAMERA_MENU_TEXT_Y_STEP, "Task:");
    ips200_show_string(120, y + 0U * CAMERA_MENU_TEXT_Y_STEP, "Frm:");
    ips200_show_string(0, y + 1U * CAMERA_MENU_TEXT_Y_STEP, "Mask:");
    ips200_show_string(0, y + 2U * CAMERA_MENU_TEXT_Y_STEP, "PVC D/S:");
    ips200_show_string(120, y + 2U * CAMERA_MENU_TEXT_Y_STEP, "C:");
    ips200_show_string(0, y + 3U * CAMERA_MENU_TEXT_Y_STEP, "PVC F:");
    ips200_show_string(120, y + 3U * CAMERA_MENU_TEXT_Y_STEP, "L:");
    ips200_show_string(0, y + 4U * CAMERA_MENU_TEXT_Y_STEP, "BRG D/S:");
    ips200_show_string(120, y + 4U * CAMERA_MENU_TEXT_Y_STEP, "St:");
    ips200_show_string(0, y + 5U * CAMERA_MENU_TEXT_Y_STEP, "BRG G:");
    ips200_show_string(120, y + 5U * CAMERA_MENU_TEXT_Y_STEP, "CX:");
    ips200_show_string(0, y + 6U * CAMERA_MENU_TEXT_Y_STEP, "BMP D/S:");
    ips200_show_string(120, y + 6U * CAMERA_MENU_TEXT_Y_STEP, "DX:");
    ips200_show_string(0, y + 7U * CAMERA_MENU_TEXT_Y_STEP, "BMP DY:");
    ips200_show_string(120, y + 7U * CAMERA_MENU_TEXT_Y_STEP, "C:");
    ips200_show_string(0, y + 8U * CAMERA_MENU_TEXT_Y_STEP, "B L:");
    ips200_show_string(78, y + 8U * CAMERA_MENU_TEXT_Y_STEP, "A:");
    ips200_show_string(150, y + 8U * CAMERA_MENU_TEXT_Y_STEP, "X:");
    ips200_show_string(0, y + 9U * CAMERA_MENU_TEXT_Y_STEP, "B m:");
    ips200_show_string(78, y + 9U * CAMERA_MENU_TEXT_Y_STEP, "N:");
    ips200_show_string(158, y + 9U * CAMERA_MENU_TEXT_Y_STEP, "Dt:");
    ips200_show_string(0, y + 10U * CAMERA_MENU_TEXT_Y_STEP, "B us: L last A avg X max m min");
}

static void CameraMenu_PrintDebug(const pvc_vision_output_t *pvc,
                                  const bridge_vision_output_t *bridge,
                                  const bumpy_vision_output_t *bumpy,
                                  uint8 active_target,
                                  uint16 enable_mask)
{
#if CAMERA_MENU_DEBUG_LOG_ENABLE
    g_camera_menu_log_counter++;
    if (g_camera_menu_log_counter < CAMERA_MENU_DEBUG_LOG_DIV)
    {
        return;
    }
#if CAMERA_MENU_DEBUG_LOG_ENABLE
    g_camera_menu_log_counter = 0U;
#endif

    printf("[CAM1] task=%u mask=%u frame=%lu pvc=%u/%u conf=%.3f line=%u/%u state=%u geo=%u bumpy=%u/%u dir=(%.2f,%.2f) cost=%lu\r\n",
           (unsigned int)active_target,
           (unsigned int)enable_mask,
           (unsigned long)CameraMenu_MaxFrameId(pvc->frame_id, bridge->frame_id, bumpy->frame_id),
           (unsigned int)pvc->raw_detected,
           (unsigned int)pvc->stable_detected,
           (double)pvc->stable.confidence,
           (unsigned int)bridge->bridge_raw_detected,
           (unsigned int)bridge->bridge_stable_detected,
           (unsigned int)bridge->stable.state,
           (unsigned int)bridge->stable.geometry_valid,
           (unsigned int)bumpy->raw_detected,
           (unsigned int)bumpy->stable_detected,
           (double)bumpy->stable.direction_x,
           (double)bumpy->stable.direction_y,
           (unsigned long)g_bumpy_vision_cost_profiler.last_us);
#else
    (void)pvc;
    (void)bridge;
    (void)bumpy;
    (void)active_target;
    (void)enable_mask;
#endif
}

void CameraMenu_Init(void)
{
    ips200_set_dir(IPS200_PORTAIT);
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_init(CAMERA_MENU_IPS200_TYPE);
    CameraMenu_DrawStaticLayout();
    g_camera_menu_inited = 1U;
    g_camera_menu_refresh_counter = 0U;
#if CAMERA_MENU_DEBUG_LOG_ENABLE
    g_camera_menu_log_counter = 0U;
#endif
}

void CameraMenu_Update(void)
{
    const pvc_vision_output_t *pvc;
    const bridge_vision_output_t *bridge;
    const bumpy_vision_output_t *bumpy;
    const uint16 y = CAMERA_MENU_TEXT_Y_BASE;
    const uint8 active_target = VisionIpc_Core1_GetActiveTarget();
    const uint16 enable_mask = VisionIpc_Core1_GetEnableMask();

    if (g_camera_menu_inited == 0U)
    {
        return;
    }

    g_camera_menu_refresh_counter++;
    if (g_camera_menu_refresh_counter < CAMERA_MENU_REFRESH_DIV)
    {
        return;
    }
    g_camera_menu_refresh_counter = 0U;

    pvc = (const pvc_vision_output_t *)pvc_vision_get_output();
    bridge = (const bridge_vision_output_t *)bridge_vision_get_output();
    bumpy = (const bumpy_vision_output_t *)bumpy_vision_get_output();

#if CAMERA_MENU_IMAGE_RENDER_ENABLE
    ips200_show_gray_image(CAMERA_MENU_IMAGE_X,
                           CAMERA_MENU_IMAGE_Y,
                           (const uint8 *)compressed_image_copy[0],
                           PVC_IMAGE_W,
                           PVC_IMAGE_H,
                           CAMERA_MENU_IMAGE_DIS_W,
                           CAMERA_MENU_IMAGE_DIS_H,
                           0U);
#endif

    ips200_show_string(40, y + 0U * CAMERA_MENU_TEXT_Y_STEP, CameraMenu_TargetToString(active_target));
    ips200_show_uint(152, y + 0U * CAMERA_MENU_TEXT_Y_STEP,
                     CameraMenu_MaxFrameId(pvc->frame_id, bridge->frame_id, bumpy->frame_id), 6);
    ips200_show_uint(48, y + 1U * CAMERA_MENU_TEXT_Y_STEP, enable_mask, 5);

    ips200_show_uint(72, y + 2U * CAMERA_MENU_TEXT_Y_STEP, pvc->raw_detected, 1);
    ips200_show_uint(96, y + 2U * CAMERA_MENU_TEXT_Y_STEP, pvc->stable_detected, 1);
    ips200_show_float(138, y + 2U * CAMERA_MENU_TEXT_Y_STEP, pvc->stable.confidence, 1, 3);
    ips200_show_int(48, y + 3U * CAMERA_MENU_TEXT_Y_STEP, pvc->stable.forward_mm, 5);
    ips200_show_int(138, y + 3U * CAMERA_MENU_TEXT_Y_STEP, pvc->stable.lateral_mm, 5);

    ips200_show_uint(72, y + 4U * CAMERA_MENU_TEXT_Y_STEP, bridge->bridge_raw_detected, 1);
    ips200_show_uint(96, y + 4U * CAMERA_MENU_TEXT_Y_STEP, bridge->bridge_stable_detected, 1);
    ips200_show_uint(138, y + 4U * CAMERA_MENU_TEXT_Y_STEP, bridge->stable.state, 1);
    ips200_show_uint(48, y + 5U * CAMERA_MENU_TEXT_Y_STEP, bridge->stable.geometry_valid, 1);
    ips200_show_int(138, y + 5U * CAMERA_MENU_TEXT_Y_STEP, bridge->stable.center_line_x1, 4);

    ips200_show_uint(72, y + 6U * CAMERA_MENU_TEXT_Y_STEP, bumpy->raw_detected, 1);
    ips200_show_uint(96, y + 6U * CAMERA_MENU_TEXT_Y_STEP, bumpy->stable_detected, 1);
    ips200_show_float(150, y + 6U * CAMERA_MENU_TEXT_Y_STEP, bumpy->stable.direction_x, 1, 2);
    ips200_show_float(54, y + 7U * CAMERA_MENU_TEXT_Y_STEP, bumpy->stable.direction_y, 1, 2);
    ips200_show_uint(138, y + 7U * CAMERA_MENU_TEXT_Y_STEP, bumpy->stable.confidence_u16, 4);

    ips200_show_uint(30, y + 8U * CAMERA_MENU_TEXT_Y_STEP, g_bumpy_vision_cost_profiler.last_us, 5);
    ips200_show_uint(100, y + 8U * CAMERA_MENU_TEXT_Y_STEP, g_bumpy_vision_cost_profiler.avg_us, 5);
    ips200_show_uint(172, y + 8U * CAMERA_MENU_TEXT_Y_STEP, g_bumpy_vision_cost_profiler.max_us, 5);

    ips200_show_uint(30, y + 9U * CAMERA_MENU_TEXT_Y_STEP,
                      CameraMenu_ProfileMinOrZero(&g_bumpy_vision_cost_profiler), 5);
    ips200_show_uint(100, y + 9U * CAMERA_MENU_TEXT_Y_STEP, g_bumpy_vision_cost_profiler.count, 6);
    ips200_show_uint(190, y + 9U * CAMERA_MENU_TEXT_Y_STEP, g_bumpy_vision_frame_profiler.last_us, 5);

    CameraMenu_PrintDebug(pvc, bridge, bumpy, active_target, enable_mask);
}

#else

void CameraMenu_Init(void)
{
}

void CameraMenu_Update(void)
{
}

#endif
