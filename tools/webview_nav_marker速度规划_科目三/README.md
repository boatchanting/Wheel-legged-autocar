# 科目三锚点补线工具

这个目录对应科目三方法一：人工打特殊点和必要的普通点，工具只在相邻锚点间补普通 `NAV_POINT_PATH`，不执行 `chazhi.py` 插值、曲率规划或速度规划。

## 使用流程

1. 运行 `python nav_marker_host.py` 打开上位机。
2. 使用“添加当前点”或双击地图打锚点；圆环、三级跳、单边桥、颠簸路等元素必须设置正确的 `point_type` 与偏航角。
3. 对会改变车辆位置的元素，在元素出口手动再打一个普通路径锚点；工具不会猜测元素动作结束位置。
4. 在“补点(mm)”中设置补点间距，点击“补点并生成路表”。
5. 工具会依次输出原始锚点 CSV、`plan3_completed_route_*.csv` 与 `code/navigation/nav_replay_route_table.h`。

默认补点间距为 250 mm。参数说明和调参建议写在 `plan3_route_builder.py` 顶部的“科目三补点关键调参区”。

## 日志

上位机的“开始记录日志/结束记录日志”会把全量 WiFi 遥测写入本目录的 `logs/`。日志包含原始帧、惯导/GPS/融合坐标、PID、打滑、目标速度和轮速等可用字段。

## 命令行

也可以绕过界面直接生成：

```powershell
python plan3_route_builder.py nav_mark_points_YYYYMMDD_HHMMSS.csv --spacing-mm 250
```

可通过 `--output-csv` 指定完成路线 CSV，通过 `--header` 指定 C 路表头文件输出位置。
