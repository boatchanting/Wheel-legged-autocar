"""Build a pixel-to-ground inverse-perspective lookup table for 96x60 frames.

The table uses a pinhole camera model and a ground-plane intersection model.
It is meant for the subject-three visual fusion flow:

- inertial navigation opens the detector near an expected element
- vision reports element pixels / rows
- this lookup converts image points to forward/lateral distances
- the element state machine still owns local odometry and control

Coordinate convention used here:

- image: u right, v down
- vehicle ground frame: x right, y forward, z up, millimeters
- camera optical frame: x right, y down, z forward
- pitch_down_deg is positive when the camera looks downward
- roll_deg is positive by right-hand rotation around vehicle forward axis
"""

from __future__ import annotations

import argparse
import csv
import json
import math
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable

import numpy as np
from PIL import Image, ImageDraw


PROJECT_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_IMAGE = (
    PROJECT_ROOT
    / "data"
    / "frames"
    / "2026_04_17_21_18_39_Video"
    / "frame_000602.png"
)
DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "data" / "逆透视表标定示例"

INVALID_I16 = -32768


@dataclass(frozen=True)
class CameraParams:
    width: int
    height: int
    fx: float
    fy: float
    cx: float
    cy: float
    height_mm: float
    pitch_down_deg: float
    roll_deg: float
    yaw_deg: float
    cam_x_mm: float
    cam_y_mm: float


def deg2rad(deg: float) -> float:
    return deg * math.pi / 180.0


def rot_x(rad: float) -> np.ndarray:
    c = math.cos(rad)
    s = math.sin(rad)
    return np.array(
        [[1.0, 0.0, 0.0], [0.0, c, -s], [0.0, s, c]],
        dtype=np.float64,
    )


def rot_y(rad: float) -> np.ndarray:
    c = math.cos(rad)
    s = math.sin(rad)
    return np.array(
        [[c, 0.0, s], [0.0, 1.0, 0.0], [-s, 0.0, c]],
        dtype=np.float64,
    )


def rot_z(rad: float) -> np.ndarray:
    c = math.cos(rad)
    s = math.sin(rad)
    return np.array(
        [[c, -s, 0.0], [s, c, 0.0], [0.0, 0.0, 1.0]],
        dtype=np.float64,
    )


def focal_from_fov(size_px: int, fov_deg: float) -> float:
    return (size_px * 0.5) / math.tan(deg2rad(fov_deg) * 0.5)


def infer_vfov_from_hfov(width: int, height: int, hfov_deg: float) -> float:
    half_h = math.tan(deg2rad(hfov_deg) * 0.5)
    half_v = (height / width) * half_h
    return math.degrees(2.0 * math.atan(half_v))


def build_camera_to_vehicle_rotation(params: CameraParams) -> np.ndarray:
    # Zero attitude maps camera z-forward to vehicle y-forward, and image down
    # to vehicle down (-z).
    base_camera_to_vehicle = np.array(
        [[1.0, 0.0, 0.0], [0.0, 0.0, 1.0], [0.0, -1.0, 0.0]],
        dtype=np.float64,
    )

    attitude = (
        rot_z(deg2rad(params.yaw_deg))
        @ rot_x(deg2rad(-params.pitch_down_deg))
        @ rot_y(deg2rad(params.roll_deg))
    )
    return attitude @ base_camera_to_vehicle


def build_ground_lut(params: CameraParams) -> dict[str, np.ndarray]:
    u, v = np.meshgrid(
        np.arange(params.width, dtype=np.float64),
        np.arange(params.height, dtype=np.float64),
    )

    rays_cam = np.stack(
        [
            (u - params.cx) / params.fx,
            (v - params.cy) / params.fy,
            np.ones_like(u),
        ],
        axis=-1,
    )

    camera_to_vehicle = build_camera_to_vehicle_rotation(params)
    rays_vehicle = rays_cam @ camera_to_vehicle.T

    origin = np.array(
        [params.cam_x_mm, params.cam_y_mm, params.height_mm],
        dtype=np.float64,
    )

    z = rays_vehicle[..., 2]
    valid = z < -1e-6
    t = np.full((params.height, params.width), np.nan, dtype=np.float64)
    t[valid] = -origin[2] / z[valid]
    valid &= t > 0.0

    x_ground = origin[0] + t * rays_vehicle[..., 0]
    y_ground = origin[1] + t * rays_vehicle[..., 1]
    valid &= y_ground >= 0.0

    ground_range = np.sqrt(x_ground * x_ground + y_ground * y_ground)
    x_ground[~valid] = np.nan
    y_ground[~valid] = np.nan
    ground_range[~valid] = np.nan

    return {
        "x_right_mm": x_ground,
        "y_forward_mm": y_ground,
        "range_mm": ground_range,
        "valid": valid,
    }


def project_ground_to_image(
    x_mm: np.ndarray,
    y_mm: np.ndarray,
    params: CameraParams,
) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    camera_to_vehicle = build_camera_to_vehicle_rotation(params)
    vehicle_to_camera = np.linalg.inv(camera_to_vehicle)

    pts_vehicle = np.stack(
        [
            x_mm - params.cam_x_mm,
            y_mm - params.cam_y_mm,
            np.zeros_like(x_mm) - params.height_mm,
        ],
        axis=-1,
    )
    pts_cam = pts_vehicle @ vehicle_to_camera.T
    z = pts_cam[..., 2]
    valid = z > 1e-6

    u = params.fx * (pts_cam[..., 0] / z) + params.cx
    v = params.fy * (pts_cam[..., 1] / z) + params.cy
    valid &= (u >= 0.0) & (u <= params.width - 1) & (v >= 0.0) & (v <= params.height - 1)
    return u, v, valid


def sample_nearest(gray: np.ndarray, u: np.ndarray, v: np.ndarray, valid: np.ndarray) -> np.ndarray:
    out = np.zeros(u.shape, dtype=np.uint8)
    ui = np.rint(u[valid]).astype(np.int32)
    vi = np.rint(v[valid]).astype(np.int32)
    out[valid] = gray[vi, ui]
    return out


def write_matrix_csv(path: Path, matrix: np.ndarray, fmt: str = "%.3f") -> None:
    np.savetxt(path, matrix, delimiter=",", fmt=fmt)


def iter_pixel_rows(lut: dict[str, np.ndarray]) -> Iterable[list[float | int]]:
    h, w = lut["valid"].shape
    for y in range(h):
        for x in range(w):
            valid = bool(lut["valid"][y, x])
            yield [
                x,
                y,
                int(valid),
                float(lut["x_right_mm"][y, x]) if valid else "",
                float(lut["y_forward_mm"][y, x]) if valid else "",
                float(lut["range_mm"][y, x]) if valid else "",
            ]


def write_pixel_lut_csv(path: Path, lut: dict[str, np.ndarray]) -> None:
    with path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(["u", "v", "valid", "x_right_mm", "y_forward_mm", "range_mm"])
        writer.writerows(iter_pixel_rows(lut))


def write_center_row_table(path: Path, lut: dict[str, np.ndarray], center_col: int) -> None:
    with path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(["v", "u", "valid", "x_right_mm", "y_forward_mm", "range_mm"])
        for v in range(lut["valid"].shape[0]):
            valid = bool(lut["valid"][v, center_col])
            writer.writerow(
                [
                    v,
                    center_col,
                    int(valid),
                    float(lut["x_right_mm"][v, center_col]) if valid else "",
                    float(lut["y_forward_mm"][v, center_col]) if valid else "",
                    float(lut["range_mm"][v, center_col]) if valid else "",
                ]
            )


def to_i16_table(values: np.ndarray, valid: np.ndarray) -> np.ndarray:
    clipped = np.clip(np.rint(values), -32767, 32767).astype(np.int32)
    clipped[~valid] = INVALID_I16
    return clipped.astype(np.int16)


def format_c_array(name: str, arr: np.ndarray) -> str:
    lines = [f"static const int16_t {name}[BEV_LUT_H][BEV_LUT_W] = {{"]
    for row in arr:
        row_text = ", ".join(str(int(v)) for v in row)
        lines.append(f"    {{{row_text}}},")
    lines.append("};")
    return "\n".join(lines)


def write_c_header(path: Path, params: CameraParams, lut: dict[str, np.ndarray]) -> None:
    x_i16 = to_i16_table(lut["x_right_mm"], lut["valid"])
    y_i16 = to_i16_table(lut["y_forward_mm"], lut["valid"])
    range_i16 = to_i16_table(lut["range_mm"], lut["valid"])
    center_col = params.width // 2
    row_forward = to_i16_table(
        lut["y_forward_mm"][:, center_col],
        lut["valid"][:, center_col],
    )

    lines = [
        "#ifndef _BEV_PIXEL_GROUND_LUT_H_",
        "#define _BEV_PIXEL_GROUND_LUT_H_",
        "",
        "#include <stdint.h>",
        "",
        f"#define BEV_LUT_W ({params.width})",
        f"#define BEV_LUT_H ({params.height})",
        f"#define BEV_LUT_INVALID_MM ({INVALID_I16})",
        "",
        "/* x_right_mm[v][u], y_forward_mm[v][u], range_mm[v][u]. */",
        "/* Generated by tools/07_针对小车车载视频的cv算法/bev/build_bev_lut.py. */",
        "",
        format_c_array("bev_lut_x_right_mm", x_i16),
        "",
        format_c_array("bev_lut_y_forward_mm", y_i16),
        "",
        format_c_array("bev_lut_range_mm", range_i16),
        "",
        "static const int16_t bev_lut_center_row_forward_mm[BEV_LUT_H] = {",
        "    " + ", ".join(str(int(v)) for v in row_forward) + ",",
        "};",
        "",
        "#endif",
        "",
    ]
    path.write_text("\n".join(lines), encoding="utf-8")


def detect_bright_pvc_rows(gray: np.ndarray, lut: dict[str, np.ndarray]) -> list[dict[str, float | int]]:
    threshold = max(120.0, float(np.percentile(gray, 75)))
    mask = gray >= threshold
    rows: list[dict[str, float | int]] = []
    h, w = gray.shape

    for v in range(h):
        xs = np.flatnonzero(mask[v])
        if xs.size < max(6, int(w * 0.12)):
            continue

        left = int(xs[0])
        right = int(xs[-1])
        center = int(round(float(xs.mean())))
        center = max(0, min(w - 1, center))

        valid_center = bool(lut["valid"][v, center])
        valid_left = bool(lut["valid"][v, left])
        valid_right = bool(lut["valid"][v, right])

        width_mm = ""
        if valid_left and valid_right:
            width_mm = abs(float(lut["x_right_mm"][v, right] - lut["x_right_mm"][v, left]))

        rows.append(
            {
                "v": v,
                "left_u": left,
                "center_u": center,
                "right_u": right,
                "pixel_width": int(right - left + 1),
                "threshold": float(threshold),
                "valid": int(valid_center),
                "center_x_right_mm": float(lut["x_right_mm"][v, center]) if valid_center else "",
                "center_y_forward_mm": float(lut["y_forward_mm"][v, center]) if valid_center else "",
                "center_range_mm": float(lut["range_mm"][v, center]) if valid_center else "",
                "estimated_width_mm": width_mm,
            }
        )

    return rows


def write_pvc_query_csv(path: Path, rows: list[dict[str, float | int]]) -> None:
    if not rows:
        path.write_text("", encoding="utf-8")
        return
    with path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
        writer.writeheader()
        writer.writerows(rows)


def create_overlay(
    image: Image.Image,
    lut: dict[str, np.ndarray],
    pvc_rows: list[dict[str, float | int]],
    output_path: Path,
    scale: int = 8,
) -> None:
    rgb = image.convert("RGB")
    draw = ImageDraw.Draw(rgb)
    valid = lut["valid"]
    h, w = valid.shape

    for v in range(0, h, 2):
        for u in range(0, w, 2):
            if not valid[v, u]:
                continue
            y_forward = lut["y_forward_mm"][v, u]
            color_ratio = min(1.0, float(y_forward) / 2500.0)
            r = int(255 * (1.0 - color_ratio))
            g = int(255 * color_ratio)
            draw.point((u, v), fill=(r, g, 40))

    for row in pvc_rows:
        v = int(row["v"])
        left = int(row["left_u"])
        center = int(row["center_u"])
        right = int(row["right_u"])
        draw.line((left, v, right, v), fill=(0, 180, 255), width=1)
        draw.ellipse((center - 1, v - 1, center + 1, v + 1), fill=(255, 0, 0))

    center_col = w // 2
    draw.line((center_col, 0, center_col, h - 1), fill=(255, 255, 0), width=1)
    draw.line((0, h - 1, w - 1, h - 1), fill=(255, 255, 0), width=1)

    large = rgb.resize((w * scale, h * scale), Image.Resampling.NEAREST)
    large.save(output_path)


def create_bev_preview(
    gray: np.ndarray,
    params: CameraParams,
    output_path: Path,
    x_min_mm: float,
    x_max_mm: float,
    y_min_mm: float,
    y_max_mm: float,
    cell_mm: float,
) -> None:
    xs = np.arange(x_min_mm, x_max_mm + cell_mm, cell_mm, dtype=np.float64)
    ys = np.arange(y_max_mm, y_min_mm - cell_mm, -cell_mm, dtype=np.float64)
    grid_x, grid_y = np.meshgrid(xs, ys)

    u, v, valid = project_ground_to_image(grid_x, grid_y, params)
    bev = sample_nearest(gray, u, v, valid)
    img = Image.fromarray(bev, mode="L").convert("RGB")
    draw = ImageDraw.Draw(img)

    for y_mm in range(int(math.ceil(y_min_mm / 500.0) * 500), int(y_max_mm) + 1, 500):
        row = int(round((y_max_mm - y_mm) / cell_mm))
        if 0 <= row < img.height:
            draw.line((0, row, img.width - 1, row), fill=(60, 160, 255), width=1)
            draw.text((2, row + 2), f"{y_mm}mm", fill=(60, 160, 255))

    for x_mm in range(int(math.ceil(x_min_mm / 200.0) * 200), int(x_max_mm) + 1, 200):
        col = int(round((x_mm - x_min_mm) / cell_mm))
        if 0 <= col < img.width:
            draw.line((col, 0, col, img.height - 1), fill=(80, 220, 80), width=1)

    center_col = int(round((0.0 - x_min_mm) / cell_mm))
    if 0 <= center_col < img.width:
        draw.line((center_col, 0, center_col, img.height - 1), fill=(255, 255, 0), width=1)

    scale = 3 if img.width < 480 else 1
    if scale > 1:
        img = img.resize((img.width * scale, img.height * scale), Image.Resampling.NEAREST)
    img.save(output_path)


def write_calibration_doc(
    path: Path,
    params: CameraParams,
    image_path: Path,
    output_dir: Path,
) -> None:
    text = f"""# 逆透视表标定说明

## 本次示例输入

- 图像：`{image_path}`
- 输出目录：`{output_dir}`
- 分辨率：{params.width} x {params.height}
- fx/fy/cx/cy：{params.fx:.3f}, {params.fy:.3f}, {params.cx:.3f}, {params.cy:.3f}
- 相机高度：{params.height_mm:.1f} mm
- pitch_down：{params.pitch_down_deg:.2f} deg
- roll：{params.roll_deg:.2f} deg
- yaw：{params.yaw_deg:.2f} deg

## 查表含义

表中每个像素 `(u, v)` 对应地面点：

```text
x_right_mm    车体右侧为正
y_forward_mm 车体前方为正
range_mm      到相机地面投影点的平面距离
```

如果表里是 `NaN` 或 C 头文件中的 `{INVALID_I16}`，说明该像素射线没有打到车辆前方地面，通常是天空/车身/地平线以上区域。

## 标定步骤

1. 固定摄像头焦距和 96x60 输出分辨率，比赛前不要再改镜头、裁剪和缩放。
2. 做内参标定，得到 `fx, fy, cx, cy`。推荐用棋盘格或 AprilGrid 拍 15~30 张图，用 OpenCV `calibrateCamera` 得到内参。如果在高分辨率下标定，再按缩放比例换算到 96x60。
3. 测量相机光心离地高度 `height_mm`。注意是镜头光心，不是外壳最高点。
4. 在平地上放几条横向白胶带，距离相机地面投影点分别为 200、400、600、800、1000mm。
5. 运行本脚本，查看 `白色PVC中心线距离查询.csv` 中各行对应的 `center_y_forward_mm`。
6. 如果所有距离整体偏大或偏小，优先修正 `pitch_down_deg` 和 `fx/fy`。
7. 如果左右宽度不对，修正 `fx` 或 `roll_deg`。
8. 如果中心线整体偏左/偏右，修正 `cx` 或摄像头 yaw 安装偏角。
9. 动态行驶时，用 IMU/机构解算得到实时 `height_mm, pitch_down_deg, roll_deg`，每帧重算表，或者预生成离散表后按姿态选择。

## 车上使用建议

- MCU 上不一定要存完整 96x60 二维表；如果只检测入口线，可以只存 `center_row_forward_mm[60]`。
- 如果要根据任意像素求横向偏差，存 `x_right_mm[v][u]` 和 `y_forward_mm[v][u]`。
- pitch/roll 变化较大时，不要使用固定表；应按姿态动态生成或选择最近姿态表。
- 科目三状态机内建议用该表修正入口/出口距离，项目内部仍用局部里程、IMU 和轮速兜底。
"""
    path.write_text(text, encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build BEV / ground lookup table for on-car 96x60 frames.")
    parser.add_argument("--image", type=Path, default=DEFAULT_IMAGE, help="Input frame path.")
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR, help="Chinese output directory under data.")

    parser.add_argument("--fx", type=float, default=None, help="Focal length in pixels.")
    parser.add_argument("--fy", type=float, default=None, help="Focal length in pixels.")
    parser.add_argument("--cx", type=float, default=None, help="Principal point x.")
    parser.add_argument("--cy", type=float, default=None, help="Principal point y.")
    parser.add_argument("--hfov-deg", type=float, default=70.0, help="Used only when fx/fy are not supplied.")
    parser.add_argument("--vfov-deg", type=float, default=None, help="Used only when fx/fy are not supplied.")

    parser.add_argument("--height-mm", type=float, default=180.0, help="Camera optical center height above ground.")
    parser.add_argument("--pitch-down-deg", type=float, default=35.0, help="Positive when camera looks downward.")
    parser.add_argument("--roll-deg", type=float, default=0.0, help="Camera/body roll angle in degrees.")
    parser.add_argument("--yaw-deg", type=float, default=0.0, help="Camera yaw mounting offset in degrees.")
    parser.add_argument("--cam-x-mm", type=float, default=0.0, help="Camera lateral offset in vehicle frame.")
    parser.add_argument("--cam-y-mm", type=float, default=0.0, help="Camera forward offset in vehicle frame.")

    parser.add_argument("--bev-x-min-mm", type=float, default=-600.0)
    parser.add_argument("--bev-x-max-mm", type=float, default=600.0)
    parser.add_argument("--bev-y-min-mm", type=float, default=0.0)
    parser.add_argument("--bev-y-max-mm", type=float, default=2500.0)
    parser.add_argument("--bev-cell-mm", type=float, default=10.0)
    return parser.parse_args()


def load_gray(path: Path) -> np.ndarray:
    img = Image.open(path).convert("L")
    return np.array(img, dtype=np.uint8)


def make_params(args: argparse.Namespace, width: int, height: int) -> CameraParams:
    vfov = args.vfov_deg
    if vfov is None:
        vfov = infer_vfov_from_hfov(width, height, args.hfov_deg)

    fx = args.fx if args.fx is not None else focal_from_fov(width, args.hfov_deg)
    fy = args.fy if args.fy is not None else focal_from_fov(height, vfov)
    cx = args.cx if args.cx is not None else (width - 1) * 0.5
    cy = args.cy if args.cy is not None else (height - 1) * 0.5

    return CameraParams(
        width=width,
        height=height,
        fx=float(fx),
        fy=float(fy),
        cx=float(cx),
        cy=float(cy),
        height_mm=float(args.height_mm),
        pitch_down_deg=float(args.pitch_down_deg),
        roll_deg=float(args.roll_deg),
        yaw_deg=float(args.yaw_deg),
        cam_x_mm=float(args.cam_x_mm),
        cam_y_mm=float(args.cam_y_mm),
    )


def main() -> None:
    args = parse_args()
    image_path = args.image.resolve()
    output_dir = args.output_dir.resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    gray = load_gray(image_path)
    height, width = gray.shape
    params = make_params(args, width, height)
    lut = build_ground_lut(params)
    pvc_rows = detect_bright_pvc_rows(gray, lut)

    write_matrix_csv(output_dir / "ground_x_right_mm.csv", lut["x_right_mm"])
    write_matrix_csv(output_dir / "ground_y_forward_mm.csv", lut["y_forward_mm"])
    write_matrix_csv(output_dir / "ground_range_mm.csv", lut["range_mm"])
    write_pixel_lut_csv(output_dir / "pixel_ground_lut.csv", lut)
    write_center_row_table(output_dir / "center_row_forward_table.csv", lut, width // 2)
    write_pvc_query_csv(output_dir / "白色PVC中心线距离查询.csv", pvc_rows)
    write_c_header(output_dir / "bev_pixel_ground_lut.h", params, lut)

    create_overlay(
        Image.fromarray(gray, mode="L"),
        lut,
        pvc_rows,
        output_dir / "示例帧_距离查表叠加.png",
    )
    create_bev_preview(
        gray,
        params,
        output_dir / "示例帧_BEV预览.png",
        args.bev_x_min_mm,
        args.bev_x_max_mm,
        args.bev_y_min_mm,
        args.bev_y_max_mm,
        args.bev_cell_mm,
    )

    config = {
        "camera_params": asdict(params),
        "input_image": str(image_path),
        "output_dir": str(output_dir),
        "valid_pixel_count": int(np.count_nonzero(lut["valid"])),
        "pvc_query_rows": len(pvc_rows),
        "outputs": {
            "ground_x_right_mm": "ground_x_right_mm.csv",
            "ground_y_forward_mm": "ground_y_forward_mm.csv",
            "ground_range_mm": "ground_range_mm.csv",
            "pixel_ground_lut": "pixel_ground_lut.csv",
            "center_row_forward_table": "center_row_forward_table.csv",
            "pvc_query": "白色PVC中心线距离查询.csv",
            "c_header": "bev_pixel_ground_lut.h",
            "overlay": "示例帧_距离查表叠加.png",
            "bev_preview": "示例帧_BEV预览.png",
            "calibration_doc": "标定说明.md",
        },
    }
    (output_dir / "bev_lut_config.json").write_text(
        json.dumps(config, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    write_calibration_doc(output_dir / "标定说明.md", params, image_path, output_dir)

    print(f"[OK] 输出目录: {output_dir}")
    print(f"[OK] valid pixels: {config['valid_pixel_count']}")
    print(f"[OK] pvc rows: {config['pvc_query_rows']}")


if __name__ == "__main__":
    main()
