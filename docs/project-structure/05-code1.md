# code1/ - 1 核工程

## 概述

`code1/` 是 1 核（辅助核）的业务代码目录，负责摄像头图像处理、视觉识别算法和 WiFi 图传。1 核专注于并行计算与数据发布，通过跨核 IPC 与 0 核通信。

## 目录结构

```text
code1/
├─ wifi.c/h                   # 1 核 WiFi 初始化、图像压缩、视觉结果渲染
├─ wifi_diff_stream.c/h       # 灰度图差分帧流发送
├─ wifi_protocol.c/h          # 1 核 WiFi 控制/示波协议
├─ tools/
│  └─ camera_menu.c/h         # 摄像头菜单（调试用）
└─ vision/
   ├─ pvc_vision.c/h          # PVC 白色目标检测
   ├─ line_vision.c/h         # 单边桥/直线检测
   ├─ bumpy_vision.c/h        # 颠簸路特征检测
   ├─ ipm_transform.c/h       # 逆透视映射（IPM）
   ├─ vision_ipc_core1.c/h    # 1 核视觉 IPC 通信
   └─ telemetry_ipc_core1.c/h # 1 核遥测数据读取
```

## 文件详细说明

### wifi.c/h - WiFi 与图像处理

**职责：**
- 初始化 WiFi 模块
- 将原始摄像头图像压缩到视觉算法输入尺寸
- 将视觉识别结果渲染到压缩图像（用于图传调试）
- 管理图像缓冲区

### wifi_diff_stream.c/h - 差分帧流

**职责：**
- 灰度图差分帧流发送：仅发送变化的像素，减少带宽占用
- 关键帧间隔配置
- 实时图传数据打包

### wifi_protocol.c/h - WiFi 协议

**职责：**
- 1 核 WiFi 控制帧处理
- 示波数据发送
- 与上位机的通信协议

### tools/camera_menu.c/h - 摄像头菜单

**职责：**
- 摄像头参数调试菜单（分辨率、曝光等）
- 通过屏幕和按键进行现场配置

## vision/ - 视觉算法模块

### pvc_vision.c/h - PVC 目标检测

**功能：**
- 检测白色 PVC 目标
- 连通域筛选
- 物理坐标估计
- 输出：目标是否有效、置信度、中心点像素坐标、物理坐标

**典型输出结构：**
```c
typedef struct {
    uint8 valid;           // 目标是否有效
    float confidence;      // 置信度
    float center_x, center_y;  // 像素坐标
    float phys_x, phys_y;     // 物理坐标（mm）
} pvc_result_t;
```

### line_vision.c/h - 桥线/直线检测

**功能：**
- 提取近端白线/暗色桥体特征
- 输出线中心和偏差
- 用于单边桥任务的视觉对线

### bumpy_vision.c/h - 颠簸路检测

**功能：**
- 检测颠簸路相关白色/暗色结构
- 输出候选位置和置信度
- 用于颠簸路任务的视觉辅助

### ipm_transform.c/h - 逆透视映射

**功能：**
- 通过逆透视查表将像素位置转换为物理坐标或距离
- 用于将摄像头图像中的目标位置转换为实际距离

### vision_ipc_core1.c/h - 1 核视觉 IPC

**功能：**
- 读取 0 核下发的视觉任务命令
- 处理复位请求
- 发布当前视觉结果或空闲状态
- 维护跨核通信状态

**关键函数：**
- `VisionIpc_Core1_Init()`：初始化
- `VisionIpc_Core1_Update_2ms()`：2 ms 周期更新
- `VisionIpc_Core1_ShouldRunPvc()`：是否应执行 PVC 检测
- `VisionIpc_Core1_ShouldRunBridgeLine()`：是否应执行桥线检测
- `VisionIpc_Core1_ShouldRunBumpy()`：是否应执行颠簸路检测
- `VisionIpc_Core1_PublishCurrent()`：发布当前视觉结果

### telemetry_ipc_core1.c/h - 1 核遥测

**功能：**
- 读取 0 核发布的遥测数据（用于调试）
- 通过 IPC 共享结构获取传感器状态

## 1 核运行流程

```
1. 初始化
   ├─ clock_init(SYSTEM_CLOCK_250M)
   ├─ debug_info_init()
   ├─ mt9v03x_init()           // 摄像头
   ├─ pvc_vision_init()        // PVC 视觉
   ├─ line_vision_init()       // 桥线视觉
   ├─ bumpy_vision_init()      // 颠簸路视觉
   ├─ VisionIpc_Core1_Init()   // 视觉 IPC
   └─ pit_ms_init(PIT_CH2, 2) // 2ms IPC 中断

2. 主循环
   while(true) {
       if(mt9v03x_finish_flag) {
           mt9v03x_finish_flag = 0;
           compress_image_to_target();  // 压缩图像

           if(VisionIpc_Core1_ShouldRunPvc()) {
               pvc_vision_process_camera_frame(compressed_image_copy[0]);
               VisionIpc_Core1_PublishCurrent();
           }
           // 其他视觉任务类似...
       }
   }

3. 中断 (PIT_CH2 2ms)
   VisionIpc_Core1_Update_2ms();  // 维护 IPC 状态
```

## 与 0 核的协作

```
0 核                          1 核
  │                            │
  ├─ VisionIpc_Core0_Set*Enable() ──→ 读取命令
  │                            │
  │                     执行视觉算法
  │                            │
  ←── VisionIpc_Core1_PublishCurrent() ──┤
  │                            │
  ├─ VisionIpc_Core0_PollResult() ──→ 获取结果
```
