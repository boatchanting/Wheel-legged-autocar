#ifndef CAMERA_MENU_H
#define CAMERA_MENU_H

#include "zf_common_headfile.h"

/* 在对视觉进行基准测试时设置为 0。菜单文本/性能分析数值仍然
 * 可见，但耗时的 94x60 -> 188x120 LCD 图像传输将被跳过。 */
#define CAMERA_MENU_IMAGE_RENDER_ENABLE    (1)

#ifdef __cplusplus
extern "C" {
#endif

void CameraMenu_Init(void);
void CameraMenu_Update(void);

#ifdef __cplusplus
}
#endif

#endif
