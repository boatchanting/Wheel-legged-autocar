"""三级台阶：先标注，再与算法输出对比。

默认读取 data/三级台阶/annotations/ground_truth_representative_v1.json。
该文件已包含从 图片说明.md 人工整理的阶段真值；运行 --annotate 可进一步鼠标标出
一级顶面四边形。随后脚本会计算状态准确率，且在存在四边形标注时计算预测凸包 IoU。
"""

from __future__ import annotations

import argparse
import json
import runpy
from pathlib import Path
from typing import Any

import cv2
import numpy as np


def project_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "data" / "三级台阶").is_dir():
            return parent
    raise FileNotFoundError("未找到项目根目录")


ROOT = project_root()
DATA_ROOT = ROOT / "data" / "三级台阶"
FRAMES_ROOT = DATA_ROOT / "frames"
DEFAULT_LABELS = DATA_ROOT / "annotations" / "ground_truth_representative_v1.json"
DEFAULT_OUTPUT = DATA_ROOT / "annotation_comparison_v1"
PROTOTYPE = Path(__file__).with_name("three_stage_prototype.py")


def load_labels(path: Path) -> list[dict[str, Any]]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    labels = payload.get("labels", [])
    if not labels:
        raise ValueError(f"标注文件没有 labels: {path}")
    for label in labels:
        if label.get("state") not in {"FAR", "FULL", "NEAR", "NONE"}:
            raise ValueError(f"非法状态标注: {label}")
        label.setdefault("stage1_polygon", [])
    return labels


def save_labels(path: Path, labels: list[dict[str, Any]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps({
        "description": "state 为人工阶段真值；stage1_polygon 为一级顶面四角，按左上、右上、右下、左下顺序。",
        "labels": labels,
    }, ensure_ascii=False, indent=2), encoding="utf-8")


def state_class(prototype_state: str) -> str:
    if prototype_state == "FULL_STAIR":
        return "FULL"
    if prototype_state == "NEAR_DEGRADED":
        return "NEAR"
    if prototype_state == "NONE":
        return "NONE"
    return "FAR"  # FAR_CANDIDATE 与 APPROACHING 都属于远距离/初见阶段


def polygon_iou(shape: tuple[int, int], predicted: np.ndarray | None, truth: list[list[int]]) -> float | None:
    if predicted is None or len(truth) < 3:
        return None
    predicted_mask = np.zeros(shape, dtype=np.uint8)
    truth_mask = np.zeros(shape, dtype=np.uint8)
    cv2.fillConvexPoly(predicted_mask, predicted.astype(np.int32), 1)
    cv2.fillConvexPoly(truth_mask, np.asarray(truth, dtype=np.int32), 1)
    union = int(np.count_nonzero(predicted_mask | truth_mask))
    return float(np.count_nonzero(predicted_mask & truth_mask) / union) if union else None


def draw_comparison(image: np.ndarray, label: dict[str, Any], predicted_hull: np.ndarray | None, predicted_state: str, iou: float | None) -> np.ndarray:
    canvas = image.copy()
    if canvas.ndim == 2:
        canvas = cv2.cvtColor(canvas, cv2.COLOR_GRAY2BGR)
    truth = label.get("stage1_polygon", [])
    if len(truth) >= 3:
        cv2.polylines(canvas, [np.asarray(truth, dtype=np.int32)], True, (0, 255, 0), 1, cv2.LINE_AA)
    if predicted_hull is not None:
        cv2.polylines(canvas, [predicted_hull.astype(np.int32)], True, (0, 0, 255), 1, cv2.LINE_AA)
    expected = label["state"]
    actual = state_class(predicted_state)
    good = expected == actual
    text = f"GT:{expected}  Pred:{actual}" + (f"  IoU:{iou:.2f}" if iou is not None else "")
    font_scale = 0.34 if canvas.shape[1] < 120 else 0.55
    color = (0, 220, 0) if good else (0, 0, 255)
    cv2.rectangle(canvas, (0, 0), (canvas.shape[1] - 1, max(12, int(canvas.shape[0] * 0.17))), (0, 0, 0), -1)
    cv2.putText(canvas, text, (2, max(10, int(canvas.shape[0] * 0.14))), cv2.FONT_HERSHEY_SIMPLEX, font_scale, color, 1, cv2.LINE_AA)
    return canvas


def make_contact(images: list[np.ndarray], output: Path) -> None:
    if not images:
        return
    width, height = 282, 180
    tiles = [cv2.resize(image, (width, height), interpolation=cv2.INTER_NEAREST) for image in images]
    columns = 4
    blank = np.zeros_like(tiles[0])
    while len(tiles) % columns:
        tiles.append(blank.copy())
    rows = [np.hstack(tiles[index:index + columns]) for index in range(0, len(tiles), columns)]
    cv2.imencode(".png", np.vstack(rows))[1].tofile(str(output))


def compare(labels: list[dict[str, Any]], output: Path) -> None:
    functions = runpy.run_path(str(PROTOTYPE))
    detect = functions["detect_primary_surface"]
    horizontal = functions["horizontal_edges"]
    classify = functions["classify"]
    output.mkdir(parents=True, exist_ok=True)
    rows: list[dict[str, Any]] = []
    panels: list[np.ndarray] = []
    for label in labels:
        path = FRAMES_ROOT / label["source"] / label["frame"]
        image = cv2.imdecode(np.fromfile(str(path), dtype=np.uint8), cv2.IMREAD_COLOR)
        if image is None:
            print(f"跳过不存在图片: {path}")
            continue
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        candidate = detect(gray)
        lines = horizontal(gray, candidate)
        predicted_state, confidence = classify(candidate, lines, gray.shape)
        hull = candidate.hull if candidate is not None else None
        iou = polygon_iou(gray.shape, hull, label.get("stage1_polygon", []))
        expected = label["state"]
        predicted = state_class(predicted_state)
        panel = draw_comparison(image, label, hull, predicted_state, iou)
        folder = output / "comparison" / label["source"]
        folder.mkdir(parents=True, exist_ok=True)
        cv2.imencode(".png", panel)[1].tofile(str(folder / label["frame"]))
        panels.append(panel)
        rows.append({
            "source": label["source"], "frame": label["frame"], "expected_state": expected,
            "predicted_state": predicted, "prototype_state": predicted_state,
            "state_match": expected == predicted, "confidence": round(float(confidence), 2),
            "polygon_iou": None if iou is None else round(iou, 4),
        })
    make_contact(panels, output / "comparison_contact.png")
    matched = sum(row["state_match"] for row in rows)
    ious = [row["polygon_iou"] for row in rows if row["polygon_iou"] is not None]
    states = ["FAR", "FULL", "NEAR", "NONE"]
    confusion = {truth: {prediction: 0 for prediction in states} for truth in states}
    for row in rows:
        confusion[row["expected_state"]][row["predicted_state"]] += 1
    report = {
        "labeled_frames": len(rows),
        "state_matches": matched,
        "state_accuracy": round(matched / max(1, len(rows)), 4),
        "polygon_iou_mean": None if not ious else round(float(np.mean(ious)), 4),
        "polygon_iou_count": len(ious),
        "confusion_matrix": confusion,
        "legend": {"green": "人工标注一级顶面", "red": "算法预测一级顶面"},
        "rows": rows,
    }
    (output / "comparison_report.json").write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps({key: value for key, value in report.items() if key != "rows"}, ensure_ascii=False, indent=2))


def annotate(labels: list[dict[str, Any]], labels_path: Path) -> None:
    """交互式补充四点标注。鼠标左键落点；0/1/2/3 设 NONE/FAR/FULL/NEAR；n 保存下一张。"""
    state_keys = {ord("0"): "NONE", ord("1"): "FAR", ord("2"): "FULL", ord("3"): "NEAR"}
    for index, label in enumerate(labels):
        path = FRAMES_ROOT / label["source"] / label["frame"]
        image = cv2.imdecode(np.fromfile(str(path), dtype=np.uint8), cv2.IMREAD_COLOR)
        if image is None:
            continue
        points = [list(point) for point in label.get("stage1_polygon", [])]
        window = "3stages annotation: LMB point, backspace undo, 0/1/2/3 state, n save-next, q quit"

        def click(event: int, x: int, y: int, _flags: int, _param: Any) -> None:
            if event == cv2.EVENT_LBUTTONDOWN and len(points) < 4:
                points.append([x, y])

        cv2.namedWindow(window, cv2.WINDOW_NORMAL)
        cv2.resizeWindow(window, image.shape[1] * 6, image.shape[0] * 6)
        cv2.setMouseCallback(window, click)
        while True:
            canvas = image.copy()
            if points:
                cv2.polylines(canvas, [np.asarray(points, dtype=np.int32)], len(points) >= 3, (0, 255, 0), 1)
                for point_index, point in enumerate(points):
                    cv2.circle(canvas, tuple(point), 1, (0, 0, 255), -1)
                    cv2.putText(canvas, str(point_index + 1), tuple(point), cv2.FONT_HERSHEY_SIMPLEX, 0.3, (0, 0, 255), 1)
            cv2.putText(canvas, f"{index + 1}/{len(labels)} {label['state']}", (2, 10), cv2.FONT_HERSHEY_SIMPLEX, 0.3, (0, 255, 255), 1)
            cv2.imshow(window, canvas)
            key = cv2.waitKey(30) & 0xFF
            if key in state_keys:
                label["state"] = state_keys[key]
            elif key in (8, 127) and points:
                points.pop()
            elif key == ord("n"):
                label["stage1_polygon"] = points
                save_labels(labels_path, labels)
                cv2.destroyWindow(window)
                break
            elif key in (ord("q"), 27):
                label["stage1_polygon"] = points
                save_labels(labels_path, labels)
                cv2.destroyAllWindows()
                return
    cv2.destroyAllWindows()


def main() -> None:
    parser = argparse.ArgumentParser(description="三级台阶标注与算法对比")
    parser.add_argument("--labels", type=Path, default=DEFAULT_LABELS)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--annotate", action="store_true", help="打开手工四点标注界面，然后保存并继续对比")
    args = parser.parse_args()
    labels = load_labels(args.labels)
    if args.annotate:
        annotate(labels, args.labels)
        labels = load_labels(args.labels)
    compare(labels, args.output)


if __name__ == "__main__":
    main()
