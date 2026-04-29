# Bumpy Road C Detector

This directory ports the desktop Python bumpy-road line-following algorithm to C.

Core entrypoint:

```c
int bumpy_detect_frame_gray(
    const uint8_t *gray,
    int width,
    int height,
    float prev_white_threshold,
    int has_prev_white_threshold,
    BumpyDetectScratch *scratch,
    BumpyDetectResult *result);
```

The C result keeps the fields needed for comparison and controller integration:

- `phase`
- `mode`
- `white_threshold`
- `dark_threshold`
- `target_x`
- `steer_error_px`
- `centerline_row_count`
- `rib_count`
- `best_component`

Desktop pipeline:

```powershell
.\tools\07_针对小车车载视频的cv算法\bumpy_road\c_bumpy_detector\run_bumpy_pc_pipeline.ps1
```

Useful switches:

```powershell
.\tools\07_针对小车车载视频的cv算法\bumpy_road\c_bumpy_detector\run_bumpy_pc_pipeline.ps1 -MaxFrames 200 -DebugEvery 20
.\tools\07_针对小车车载视频的cv算法\bumpy_road\c_bumpy_detector\run_bumpy_pc_pipeline.ps1 -NoCsv
.\tools\07_针对小车车载视频的cv算法\bumpy_road\c_bumpy_detector\run_bumpy_pc_pipeline.ps1 -NoTiming
```
