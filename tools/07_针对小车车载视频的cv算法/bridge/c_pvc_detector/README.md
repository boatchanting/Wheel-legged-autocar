# PVC 入口检测 C 版

这个目录实现的是“白色 PVC 入口检测”的 C 语言版本，算法对齐 `../detect_white_pvc_video.py`。当前目标是先在电脑跑通、输出逐帧调试数据，并和 Python 版本的 `video_summary.json` 对比；下一步再把 `pvc_detector.c/.h` 搬到车机工程里。

## 模块边界

核心检测模块只依赖灰度图输入：

```c
int pvc_detect_frame_gray(const uint8_t *gray,
                          int width,
                          int height,
                          PvcDetectScratch *scratch,
                          PvcDetectResult *result);
```

车机侧后续可以直接传入 `96x60` 摄像头灰度图。PC 调试侧只是额外提供了 PNG 转 PGM 和命令行批处理，不应进入车机代码。

## 输出契约

`PvcDetectResult` 是视觉模块传给状态机/控制层的稳定接口：

- `detected`：是否确认看到 PVC 入口，建议状态机只用这个字段触发“进入项目局部视觉状态”。
- `confidence`：当前最佳候选的置信度，Python 同款阈值是 `0.58`。
- `bbox_xmin/ymin/xmax/ymax`：PVC 白色连通域包围框，用于调试、估计入口边界。
- `centroid_x/y`：白色区域中心，可转成横向偏差。
- `entry_bottom_y`：包围框底边行号，适合查表估计入口距离。
- `entry_top_y`：包围框顶边行号，适合判断入口白边展开程度。
- `area/fill_ratio/mean_gray/touches_border`：用于调试和误检分析。
- `forward_mm/lateral_mm/yaw_error_deg`：控制层友好字段；当前 C 版先放了线性占位估计，车机落地时应替换成 96x60 标定查表。

建议控制层不要直接读图像特征，而是只读：

```c
detected
confidence
forward_mm
lateral_mm
yaw_error_deg
entry_bottom_y
```

这样后续把入口检测从连通域升级成边线/逆透视/小模型时，控制层不用改。

## 算法对齐点

C 版刻意复刻 Python 版本的规则：

- 白色阈值：`gray >= 245`
- 连通域：4 邻域 BFS
- 候选过滤：面积、宽高、填充率、是否触边
- 评分：面积、宽度、高度、填充率、触边、亮度加权
- 进入判定：最佳候选 `score >= 0.58`

这意味着同一批帧理论上应得到接近一致的 `detected/score/bbox/area`。如果出现差异，优先检查输入帧是否和 Python 处理的帧完全一致。

## 一键运行

在项目根目录执行：

```powershell
.\tools\07_针对小车车载视频的cv算法\bridge\c_pvc_detector\run_pvc_pc_pipeline.ps1
```

脚本会执行：

1. 将 `data\frames\2026_04_17_21_18_39_Video\frame_*.png` 转成 PGM。
2. 编译 `pvc_detector.c + pvc_video_cli.c`。
3. 跑 C 版逐帧检测，生成 `data\bridge_white_pvc_c_run\pvc_c_summary.json`。
4. 和 Python 输出 `data\bridge_white_pvc_detection_video_2026_04_17_21_18_39\video_summary.json` 对比。

常用开关：

```powershell
.\tools\07_针对小车车载视频的cv算法\bridge\c_pvc_detector\run_pvc_pc_pipeline.ps1 -MaxFrames 200 -DebugEvery 20
.\tools\07_针对小车车载视频的cv算法\bridge\c_pvc_detector\run_pvc_pc_pipeline.ps1 -NoCsv
.\tools\07_针对小车车载视频的cv算法\bridge\c_pvc_detector\run_pvc_pc_pipeline.ps1 -NoTiming
```

如果当前 PowerShell 找不到 `gcc/clang/cl`，脚本会停在编译步骤。可以安装 MinGW/LLVM，或在 Visual Studio Developer PowerShell 里运行。

## 车机迁移建议

第一版车机集成保持简单：

1. 惯导接近项目入口 `800mm` 内，开启 PVC 入口视觉检测。
2. 连续 `N` 帧 `detected=1` 且 `confidence` 稳定后，锁定入口。
3. 状态机记录当前惯导位姿 `entry_pose`，切到项目局部控制。
4. 控制只消费 `forward_mm/lateral_mm/yaw_error_deg`，用 PD 做横向和航向修正。
5. 项目结束后，根据固定赛道长度和 `entry_pose` 回写惯导里程/航向。

第一版不建议让 PVC 检测直接改全局惯导位置。更稳的做法是：视觉只负责项目入口锁定和局部控制，全局惯导在状态切换点做一次“门控修正”。
