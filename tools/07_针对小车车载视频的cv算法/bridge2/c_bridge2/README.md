# Single Bridge Detection C 实现

这是一份面向车机迁移的纯 C99 实现，算法主体对齐
`../single_bridge_prototype_with_middle.py`：多阈值候选、形态学、4 邻域连通域、填洞、凸包、候选评分、左右边拟合、入口横边判定和单边桥四态推断。白色 PVC 区域只是单边桥的内部视觉特征，不是模块业务名称。

核心模块不依赖 OpenCV、文件系统或计时代码：

```c
int bridge_detection_detect_gray(const uint8_t *gray,
                            int width,
                            int height,
                            int stride,
                            const BridgeDetectionConfig *config,
                            BridgeDetectionScratch *scratch,
                            BridgeDetectionResult *result);
```

`BridgeDetectionScratch` 由调用方静态分配，检测过程中不使用堆内存。当前上限是
`96x60`，对应现有 `94x60` 数据。车机集成时建议只编译
`bridge_detection.c/.h`，不要带入 `bridge_detection_pc.c`。

## 输出契约

- `bridge_found/state`：供项目状态机使用；状态为无、准备进入、在单边桥上、准备退出。
- `candidate_score/edge_contrast`：检测质量，适合做连续帧门控。
- `top/start/bottom_row`、宽度、裁边比例：调试及状态判断特征。
- `left_line/right_line`：`x = slope*y + intercept` 的左右边模型。
- `center_segment`：由左右边生成的控制中心线。
- `lateral_error_px`：中心线下端相对图像中心的横向像素误差，正值表示目标在右侧。
- `heading_dx_per_dy`：中心线每向下一像素的横向变化，可作为航向误差输入。

建议车机状态机要求 `bridge_found` 连续若干帧成立，并结合
`candidate_score`、`edge_contrast` 做迟滞；控制层只消费中心线和误差字段，不直接读取掩膜。

## PC 全量验证

在项目根目录执行：

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
  "tools\07_针对小车车载视频的cv算法\bridge2\c_bridge2\run_pc_comparison.ps1"
```

脚本会把 PNG 临时转换成 PGM、使用本机 Visual Studio 编译、运行纯 C 检测器、记录仅检测函数的平均/最小/最大/P50/P95/P99 帧耗时，并与 Python 的 `summary.csv` 对比。

Python 中为画线观感服务的逐场景端点平移/延长规则没有进入核心模块；C 输出的是拟合模型直接生成的控制几何。对比报告因此把检测、状态和线可见性作为一致性门槛，把线段端点像素差单独列为诊断项。

## 性能优化结果

- PC 568 帧平均：约 `0.097 ms/帧`（包含完全相同连续帧的严格缓存）。
- 独立新帧平均：约 `0.31 ms/帧`。
- `BridgeDetectionScratch`：约 `21.9 KB`。
- IAR Cortex-M7 `-Ohz`：约 `10.4 KB` CODE。
- 核心无 `double`、通用数学函数、`qsort`、`malloc/free`。
- 对完全重复帧使用逐字节缓存；对小噪声稳定帧使用最多 3 阈值的时序路径，并每 3 帧强制全量校正。

详细瓶颈、复杂度、Cortex-M7 说明和方案取舍见 `PERFORMANCE_REPORT.md`。
