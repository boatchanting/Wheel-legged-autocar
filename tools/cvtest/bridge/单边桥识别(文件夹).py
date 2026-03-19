import os
import glob
import json
import cv2
import numpy as np
from dataclasses import dataclass
from typing import Optional, Dict, Any, Tuple


# ============================================================
#  DanBianQiao Detector (单边桥识别) - 复刻你贴的 C 思路
# ============================================================

@dataclass
class DanBianQiaoParams:
    # ===== 预处理阈值与形态学 =====
    use_otsu: bool = True
    fixed_thresh: int = 160
    blur_ksize: int = 5
    close_ksize: int = 3
    open_ksize: int = 3

    # ===== 边线提取 =====
    min_run: int = 30
    prefer_largest_run: bool = True

    # ===== 角点检测（移植C阈值）=====
    flat_tol: int = 3
    turn_step: int = 3
    local_tol: int = 1

    # ===== 搜索范围 =====
    search_start_row: Optional[int] = None
    search_end_row: Optional[int] = None
    safe_margin_to_stop: int = 4

    # ===== 单边桥判定（宽度突变）=====
    width_shrink_ratio: float = 0.60
    width_min_ratio_vs_down: float = 0.20
    width_dn_ratio_max: float = 1.25

    # ===== 跳变确认 =====
    jump_scan_down: int = 5
    jump_scan_up: int = 20
    jump_offset_px: int = 2
    require_jump_type: str = "black_to_white"  # "black_to_white" 或 "any"

    # ===== 可选：物理宽度强约束（BEV时很有用）=====
    m_per_pixel: Optional[float] = None
    expected_bridge_width_m: Optional[float] = 0.45
    expected_width_tol_m: float = 0.20


class DanBianQiaoDetector:
    def __init__(self, params: DanBianQiaoParams, debug: bool = True):
        self.p = params
        self.debug = debug

    def preprocess_to_binary(self, gray: np.ndarray) -> np.ndarray:
        assert gray.ndim == 2, "需要灰度图"
        g = cv2.GaussianBlur(gray, (self.p.blur_ksize, self.p.blur_ksize), 0)

        if self.p.use_otsu:
            _, bw = cv2.threshold(g, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
        else:
            _, bw = cv2.threshold(g, self.p.fixed_thresh, 255, cv2.THRESH_BINARY)

        if self.p.open_ksize > 1:
            k = cv2.getStructuringElement(cv2.MORPH_RECT, (self.p.open_ksize, self.p.open_ksize))
            bw = cv2.morphologyEx(bw, cv2.MORPH_OPEN, k, iterations=1)

        if self.p.close_ksize > 1:
            k = cv2.getStructuringElement(cv2.MORPH_RECT, (self.p.close_ksize, self.p.close_ksize))
            bw = cv2.morphologyEx(bw, cv2.MORPH_CLOSE, k, iterations=1)

        return bw

    @staticmethod
    def _largest_run_indices(xs: np.ndarray) -> Optional[Tuple[int, int]]:
        if xs.size == 0:
            return None
        breaks = np.where(np.diff(xs) > 1)[0]
        starts = np.r_[0, breaks + 1]
        ends = np.r_[breaks, xs.size - 1]
        lengths = ends - starts + 1
        k = int(np.argmax(lengths))
        return int(xs[starts[k]]), int(xs[ends[k]])

    @staticmethod
    def _median_smooth_1d(arr: np.ndarray, k: int = 5) -> np.ndarray:
        if k <= 1:
            return arr
        out = arr.copy()
        n = arr.size
        pad = k // 2
        for i in range(pad, n - pad):
            win = arr[i - pad:i + pad + 1]
            win = win[win >= 0]
            if win.size:
                out[i] = int(np.median(win))
        return out

    def extract_borders(self, bw: np.ndarray) -> Tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray, int]:
        h, w = bw.shape
        l_border = np.full(h, -1, dtype=np.int32)
        r_border = np.full(h, -1, dtype=np.int32)

        left_stop = np.zeros(h, dtype=np.int32)
        right_stop = np.full(h, w - 1, dtype=np.int32)

        valid_rows = []
        for y in range(h):
            xs = np.where(bw[y] > 0)[0]
            if xs.size < self.p.min_run:
                continue

            if self.p.prefer_largest_run:
                seg = self._largest_run_indices(xs)
                if seg is None:
                    continue
                l, r = seg
                if (r - l + 1) < self.p.min_run:
                    continue
            else:
                l, r = int(xs[0]), int(xs[-1])

            l_border[y] = l
            r_border[y] = r
            valid_rows.append(y)

        hightest = int(min(valid_rows)) if valid_rows else max(0, h - 1)

        l_border = self._median_smooth_1d(l_border, k=5)
        r_border = self._median_smooth_1d(r_border, k=5)

        return l_border, r_border, left_stop, right_stop, hightest

    def find_angle_point_danbianqiao(
        self,
        start: int,
        end: int,
        border: np.ndarray,
        dir_: int,
        l_border: np.ndarray,
        r_border: np.ndarray,
        left_stop: np.ndarray,
        right_stop: np.ndarray,
    ) -> int:
        h = border.size
        start = min(start, h - 4)
        end = max(end, 4)

        flat_tol = self.p.flat_tol
        turn_step = self.p.turn_step
        local_tol = self.p.local_tol
        margin = self.p.safe_margin_to_stop

        i0 = min(start, h - 4)
        i1 = max(end, 4)

        for i in range(i0, i1 - 1, -1):
            if i + 3 >= h or i - 4 < 0:
                continue
            if l_border[i] < left_stop[i] + margin:
                continue
            if r_border[i] > right_stop[i] - margin:
                continue
            if border[i] < 0 or border[i + 1] < 0 or border[i + 2] < 0 or border[i + 3] < 0:
                continue
            if border[i - 1] < 0 or border[i - 2] < 0 or border[i - 3] < 0 or border[i - 4] < 0:
                continue

            cond_flat = (
                abs(int(border[i]) - int(border[i + 1])) <= flat_tol
                and abs(int(border[i + 1]) - int(border[i + 2])) <= flat_tol
                and abs(int(border[i + 2]) - int(border[i + 3])) <= flat_tol
            )

            if dir_ == 1:
                cond_turn = (
                    (int(border[i - 2]) - int(border[i])) >= turn_step
                    and (int(border[i - 3]) - int(border[i])) >= turn_step
                    and (int(border[i - 4]) - int(border[i])) >= turn_step
                )
            else:
                cond_turn = (
                    (int(border[i - 2]) - int(border[i])) <= -turn_step
                    and (int(border[i - 3]) - int(border[i])) <= -turn_step
                    and (int(border[i - 4]) - int(border[i])) <= -turn_step
                )

            cond_local = (
                abs(int(border[i - 2]) - int(border[i - 1])) <= local_tol
                and abs(int(border[i - 3]) - int(border[i - 1])) <= local_tol
                and abs(int(border[i - 4]) - int(border[i - 1])) <= local_tol
            )

            if cond_flat and cond_turn and cond_local:
                return int(i)

        return 0

    def detect(self, gray: np.ndarray) -> Dict[str, Any]:
        bw = self.preprocess_to_binary(gray)
        l_border, r_border, left_stop, right_stop, hightest = self.extract_borders(bw)
        res = self.detect_from_borders(bw, l_border, r_border, left_stop, right_stop, hightest)
        res["_bw"] = bw
        res["_l_border"] = l_border
        res["_r_border"] = r_border
        return res

    def detect_from_borders(
        self,
        bw: np.ndarray,
        l_border: np.ndarray,
        r_border: np.ndarray,
        left_stop: np.ndarray,
        right_stop: np.ndarray,
        hightest: int,
    ) -> Dict[str, Any]:
        h, w = bw.shape
        left_lost = int(np.sum(l_border < 0))
        right_lost = int(np.sum(r_border < 0))
        if left_lost > 0.5 * h or right_lost > 0.5 * h:
            return {"found": False, "reason": "too_many_lost", "debug": {"left_lost": left_lost, "right_lost": right_lost}}

        start = self.p.search_start_row if self.p.search_start_row is not None else (h - 6)
        end = self.p.search_end_row if self.p.search_end_row is not None else hightest

        start = int(np.clip(start, 0, h - 6))
        end = int(np.clip(end, 4, h - 1))

        leftline_right_turn = self.find_angle_point_danbianqiao(start, end, l_border, 1, l_border, r_border, left_stop, right_stop)
        rightline_left_turn = self.find_angle_point_danbianqiao(start, end, r_border, -1, l_border, r_border, left_stop, right_stop)

        leftline_left_turn = self.find_angle_point_danbianqiao(start, end, l_border, -1, l_border, r_border, left_stop, right_stop)
        rightline_right_turn = self.find_angle_point_danbianqiao(start, end, r_border, 1, l_border, r_border, left_stop, right_stop)

        if leftline_right_turn == 0 and rightline_left_turn == 0:
            return {"found": False, "reason": "no_inner_corner", "debug": {"leftline_right_turn": leftline_right_turn, "rightline_left_turn": rightline_left_turn}}

        if (leftline_left_turn > leftline_right_turn and leftline_right_turn != 0) or \
           (rightline_right_turn > rightline_left_turn and rightline_left_turn != 0):
            return {"found": False, "reason": "outer_turn_earlier"}

        margin = self.p.safe_margin_to_stop
        if leftline_right_turn and (l_border[leftline_right_turn] < left_stop[leftline_right_turn] + margin):
            return {"found": False, "reason": "left_corner_too_close_stop"}
        if rightline_left_turn and (r_border[rightline_left_turn] > right_stop[rightline_left_turn] - margin):
            return {"found": False, "reason": "right_corner_too_close_stop"}

        if leftline_right_turn > rightline_left_turn:
            side = "LEFT"
            y0 = leftline_right_turn
            ok, detail = self._verify_width_and_jump(bw, l_border, r_border, y0, side)
        else:
            side = "RIGHT"
            y0 = rightline_left_turn
            ok, detail = self._verify_width_and_jump(bw, l_border, r_border, y0, side)

        if not ok:
            return {"found": False, "reason": "verify_failed", "debug": {"side": side, "corner_y": y0, "verify": detail}}

        return {"found": True, "side": side, "corner_y": int(y0), "debug": detail}

    def _verify_width_and_jump(self, bw: np.ndarray, l_border: np.ndarray, r_border: np.ndarray, y0: int, side: str):
        h, w = bw.shape
        if y0 - 3 < 0 or y0 + 3 >= h:
            return False, {"reason": "y0_out_of_range"}

        if l_border[y0] < 0 or r_border[y0] < 0 or l_border[y0 - 3] < 0 or r_border[y0 - 3] < 0 or l_border[y0 + 3] < 0 or r_border[y0 + 3] < 0:
            return False, {"reason": "invalid_border_value"}

        w0 = int(r_border[y0] - l_border[y0])
        w_up = int(r_border[y0 - 3] - l_border[y0 - 3])
        w_dn = int(r_border[y0 + 3] - l_border[y0 + 3])

        if not (w_up <= w0 * self.p.width_shrink_ratio and w_up >= w_dn * self.p.width_min_ratio_vs_down and w_dn <= w0 * self.p.width_dn_ratio_max):
            return False, {"reason": "width_check_fail", "w0": w0, "w_up": w_up, "w_dn": w_dn}

        if self.p.m_per_pixel is not None and self.p.expected_bridge_width_m is not None:
            width_m = w0 * self.p.m_per_pixel
            if abs(width_m - self.p.expected_bridge_width_m) > self.p.expected_width_tol_m:
                return False, {"reason": "metric_width_fail", "width_m": width_m, "w0_px": w0}

        if side == "LEFT":
            x = int(l_border[y0 - 3] - self.p.jump_offset_px)
        else:
            x = int(r_border[y0 - 3] + self.p.jump_offset_px)

        if x < 1 or x >= w:
            return False, {"reason": "jump_x_oob", "x": x}

        found_jump = False
        jump_type = None

        y_start = min(h - 2, y0 + self.p.jump_scan_down)
        y_end = max(1, y0 - self.p.jump_scan_up)

        for yy in range(y_start, y_end - 1, -1):
            a = bw[yy - 1, x]
            b = bw[yy, x]
            if a == 0 and b == 255:
                found_jump = True
                jump_type = "black_to_white"
                break
            if a == 255 and b == 0:
                found_jump = True
                jump_type = "white_to_black"
                if self.p.require_jump_type == "any":
                    break

        if not found_jump:
            return False, {"reason": "no_jump_found", "x": x, "scan": [y_start, y_end]}

        if self.p.require_jump_type != "any" and jump_type != self.p.require_jump_type:
            return False, {"reason": "jump_type_mismatch", "got": jump_type, "need": self.p.require_jump_type}

        return True, {"w0": w0, "w_up": w_up, "w_dn": w_dn, "jump_x": x, "jump_type": jump_type, "scan": [y_start, y_end]}

    def draw_debug(self, gray: np.ndarray, l_border: np.ndarray, r_border: np.ndarray, result: Dict[str, Any]) -> np.ndarray:
        vis = cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)
        h, w = gray.shape

        for y in range(h):
            if l_border[y] >= 0:
                cv2.circle(vis, (int(l_border[y]), y), 1, (255, 0, 0), -1)
            if r_border[y] >= 0:
                cv2.circle(vis, (int(r_border[y]), y), 1, (0, 255, 0), -1)

        if result.get("found", False):
            y0 = int(result["corner_y"])
            cv2.circle(vis, (int(l_border[y0]), y0), 4, (0, 0, 255), 2)
            cv2.circle(vis, (int(r_border[y0]), y0), 4, (0, 0, 255), 2)
            cv2.putText(vis, f"DanBianQiao: {result['side']}  y={y0}", (10, 25),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
        else:
            cv2.putText(vis, f"No DanBianQiao ({result.get('reason','')})", (10, 25),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)

        return vis


# ============================================================
#  Batch Runner: bridge/ -> bridgeresult/
# ============================================================

def ensure_dir(d: str):
    os.makedirs(d, exist_ok=True)


def batch_detect_bridge(input_dir="bridge", output_dir="bridgeresult"):
    ensure_dir(output_dir)
    ensure_dir(os.path.join(output_dir, "vis"))
    ensure_dir(os.path.join(output_dir, "bw"))
    ensure_dir(os.path.join(output_dir, "json"))

    exts = ["*.png", "*.jpg", "*.jpeg", "*.bmp", "*.webp", "*.tif", "*.tiff"]
    img_paths = []
    for e in exts:
        img_paths += glob.glob(os.path.join(input_dir, e))
    img_paths = sorted(img_paths)

    if not img_paths:
        print(f"[ERROR] 没找到图片: {input_dir}/")
        return

    params = DanBianQiaoParams(
        use_otsu=True,
        close_ksize=3,
        open_ksize=3,
        min_run=30,
        require_jump_type="black_to_white",
        # 如果你是 BEV 且知道比例，填这里会更稳
        # m_per_pixel=0.002, expected_bridge_width_m=0.45
    )
    det = DanBianQiaoDetector(params, debug=True)

    summary_lines = ["filename,found,side,corner_y,reason\n"]
    ok_count = 0

    for idx, path in enumerate(img_paths):
        name = os.path.basename(path)
        stem = os.path.splitext(name)[0]

        gray = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
        if gray is None:
            print(f"[SKIP] 读取失败: {path}")
            summary_lines.append(f"{name},0,,,read_failed\n")
            continue

        result = det.detect(gray)

        # 提取中间结果（用于保存）
        bw = result.pop("_bw")
        l_border = result.pop("_l_border")
        r_border = result.pop("_r_border")

        found = int(result.get("found", False))
        side = result.get("side", "")
        corner_y = result.get("corner_y", "")
        reason = result.get("reason", "")

        if found:
            ok_count += 1

        # 1) 保存 JSON
        json_path = os.path.join(output_dir, "json", f"{stem}.json")
        with open(json_path, "w", encoding="utf-8") as f:
            json.dump(result, f, ensure_ascii=False, indent=2)

        # 2) 保存二值图
        bw_path = os.path.join(output_dir, "bw", f"{stem}_bw.png")
        cv2.imwrite(bw_path, bw)

        # 3) 保存可视化图
        vis = det.draw_debug(gray, l_border, r_border, result)
        vis_path = os.path.join(output_dir, "vis", f"{stem}_vis.png")
        cv2.imwrite(vis_path, vis)

        # 4) summary
        summary_lines.append(f"{name},{found},{side},{corner_y},{reason}\n")

        print(f"[{idx+1}/{len(img_paths)}] {name} -> found={found} side={side} y={corner_y} {('' if found else 'reason='+reason)}")

    # 保存总表 CSV
    csv_path = os.path.join(output_dir, "summary.csv")
    with open(csv_path, "w", encoding="utf-8") as f:
        f.writelines(summary_lines)

    print("\n==================== DONE ====================")
    print(f"Input:  {input_dir}")
    print(f"Output: {output_dir}")
    print(f"Total:  {len(img_paths)}")
    print(f"Found:  {ok_count}")
    print(f"CSV:    {csv_path}")
    print("=============================================\n")


if __name__ == "__main__":
    batch_detect_bridge("bridge2", "bridgeresult2")
