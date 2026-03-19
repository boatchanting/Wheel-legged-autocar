import cv2
import numpy as np
from dataclasses import dataclass
from typing import Optional, Dict, Any, Tuple


@dataclass
class DanBianQiaoParams:
    # ===== 预处理阈值与形态学 =====
    use_otsu: bool = True
    fixed_thresh: int = 160                 # use_otsu=False 时使用
    blur_ksize: int = 5                     # 高斯核大小(奇数)
    close_ksize: int = 3                    # 形态学 close 核(尽量小，别把楔形障碍填掉)
    open_ksize: int = 3

    # ===== 边线提取 =====
    min_run: int = 20                       # 每行最小白色连通段长度(像素)
    prefer_largest_run: bool = True         # 每行取最大白段(更像桥面)

    # ===== 角点检测（移植 C 阈值）=====
    flat_tol: int = 3                       # abs(border[i]-border[i+1]) <= flat_tol
    turn_step: int = 3                      # border[i-2]-border[i] >= turn_step 等
    local_tol: int = 1                      # abs(border[i-2]-border[i-1]) <= local_tol

    # ===== 搜索范围 =====
    search_start_row: Optional[int] = None  # None 则用 h-6
    search_end_row: Optional[int] = None    # None 则用 hightest(由有效边线自动估计)
    safe_margin_to_stop: int = 4            # 对应 C 里的 +4 / -4

    # ===== 单边桥判定（宽度突变）=====
    width_shrink_ratio: float = 0.60        # w_up <= w0 * 0.6
    width_min_ratio_vs_down: float = 0.20   # w_up >= w_dn * 0.2（防止太离谱）
    width_dn_ratio_max: float = 1.25        # w_dn <= w0*1.25（可根据你相机调整）

    # ===== 跳变确认 =====
    jump_scan_down: int = 5                 # 从角点往“下”偏移多少行开始扫
    jump_scan_up: int = 20                  # 从角点往“上”扫多少行
    jump_offset_px: int = 2                 # 在边线外侧取点（左：l_border-2，右：r_border+2）
    require_jump_type: str = "black_to_white"  # "black_to_white" 或 "any"

    # ===== 可选：用物理宽度做强约束（BEV 下很有用）=====
    m_per_pixel: Optional[float] = None     # 你若是 BEV，填这个
    expected_bridge_width_m: Optional[float] = 0.45
    expected_width_tol_m: float = 0.20      # 允许误差


class DanBianQiaoDetector:
    def __init__(self, params: DanBianQiaoParams, debug: bool = True):
        self.p = params
        self.debug = debug

    # -------------------- 1) 预处理与二值化 --------------------
    def preprocess_to_binary(self, gray: np.ndarray) -> np.ndarray:
        assert gray.ndim == 2, "需要灰度图"
        g = cv2.GaussianBlur(gray, (self.p.blur_ksize, self.p.blur_ksize), 0)

        if self.p.use_otsu:
            _, bw = cv2.threshold(g, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
        else:
            _, bw = cv2.threshold(g, self.p.fixed_thresh, 255, cv2.THRESH_BINARY)

        # 小开运算去噪
        if self.p.open_ksize > 1:
            k = cv2.getStructuringElement(cv2.MORPH_RECT, (self.p.open_ksize, self.p.open_ksize))
            bw = cv2.morphologyEx(bw, cv2.MORPH_OPEN, k, iterations=1)

        # 小闭运算：填掉灰色分段线（通常很细），但别把楔形障碍填没
        if self.p.close_ksize > 1:
            k = cv2.getStructuringElement(cv2.MORPH_RECT, (self.p.close_ksize, self.p.close_ksize))
            bw = cv2.morphologyEx(bw, cv2.MORPH_CLOSE, k, iterations=1)

        return bw

    # -------------------- 2) 从二值图提取每行左右边线 --------------------
    @staticmethod
    def _largest_run_indices(xs: np.ndarray) -> Optional[Tuple[int, int]]:
        """给定一行白像素坐标xs，返回最大连续段 [l,r]"""
        if xs.size == 0:
            return None
        # 分段
        breaks = np.where(np.diff(xs) > 1)[0]
        starts = np.r_[0, breaks + 1]
        ends = np.r_[breaks, xs.size - 1]
        lengths = ends - starts + 1
        k = int(np.argmax(lengths))
        return int(xs[starts[k]]), int(xs[ends[k]])

    def extract_borders(self, bw: np.ndarray) -> Tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray, int]:
        """
        返回:
          l_border, r_border, left_stop_point, right_stop_point, hightest
        这里 stop_point 先按全幅，和你 C 里的“截止线”概念对齐。
        """
        h, w = bw.shape
        l_border = np.full(h, -1, dtype=np.int32)
        r_border = np.full(h, -1, dtype=np.int32)

        left_stop = np.zeros(h, dtype=np.int32)
        right_stop = np.full(h, w - 1, dtype=np.int32)

        valid_rows = []

        for y in range(h):
            row = bw[y]
            xs = np.where(row > 0)[0]
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

        # 轻微平滑，减少噪声导致的假角点（不要太强）
        l_border = self._median_smooth_1d(l_border, k=5)
        r_border = self._median_smooth_1d(r_border, k=5)

        return l_border, r_border, left_stop, right_stop, hightest

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

    # -------------------- 3) C版 Find_Angle_Point_DanBianQiao 移植 --------------------
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
        """
        返回角点所在行 y（找不到返回0，保持和C一致）。
        dir_= 1 右转；dir_=-1 左转
        """
        h = border.size
        start = min(start, h - 4)
        end = max(end, 4)

        flat_tol = self.p.flat_tol
        turn_step = self.p.turn_step
        local_tol = self.p.local_tol
        margin = self.p.safe_margin_to_stop

        # 需要 i+3 <= h-1 且 i-4 >= 0
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

            if dir_ == 1:  # 右转
                cond_turn = (
                    (int(border[i - 2]) - int(border[i])) >= turn_step
                    and (int(border[i - 3]) - int(border[i])) >= turn_step
                    and (int(border[i - 4]) - int(border[i])) >= turn_step
                )
            else:  # 左转
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

    # -------------------- 4) 单边桥识别（对应 C 的 danbianqiao() 检测部分） --------------------
    def detect(self, gray: np.ndarray) -> Dict[str, Any]:
        bw = self.preprocess_to_binary(gray)
        l_border, r_border, left_stop, right_stop, hightest = self.extract_borders(bw)
        return self.detect_from_borders(bw, l_border, r_border, left_stop, right_stop, hightest)

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

        # 丢线粗判（对齐你的 C：一侧丢线太多直接 return）
        left_lost = int(np.sum(l_border < 0))
        right_lost = int(np.sum(r_border < 0))
        if left_lost > 0.5 * h or right_lost > 0.5 * h:
            return self._result(False, reason="too_many_lost", extra={"left_lost": left_lost, "right_lost": right_lost})

        start = self.p.search_start_row if self.p.search_start_row is not None else (h - 6)
        end = self.p.search_end_row if self.p.search_end_row is not None else hightest

        start = int(np.clip(start, 0, h - 6))
        end = int(np.clip(end, 4, h - 1))

        # 4个角点（内拐 / 外拐）
        leftline_right_turn = self.find_angle_point_danbianqiao(start, end, l_border, 1, l_border, r_border, left_stop, right_stop)
        rightline_left_turn = self.find_angle_point_danbianqiao(start, end, r_border, -1, l_border, r_border, left_stop, right_stop)

        leftline_left_turn = self.find_angle_point_danbianqiao(start, end, l_border, -1, l_border, r_border, left_stop, right_stop)
        rightline_right_turn = self.find_angle_point_danbianqiao(start, end, r_border, 1, l_border, r_border, left_stop, right_stop)

        if leftline_right_turn == 0 and rightline_left_turn == 0:
            return self._result(False, reason="no_inner_corner",
                                extra={"leftline_right_turn": leftline_right_turn, "rightline_left_turn": rightline_left_turn})

        # “向外拐比向内拐要早”过滤（对齐 C）
        if (leftline_left_turn > leftline_right_turn and leftline_right_turn != 0) or \
           (rightline_right_turn > rightline_left_turn and rightline_left_turn != 0):
            return self._result(False, reason="outer_turn_earlier",
                                extra={"leftline_left_turn": leftline_left_turn, "rightline_right_turn": rightline_right_turn})

        # 过于靠近截止线过滤（对齐 C）
        margin = self.p.safe_margin_to_stop
        if leftline_right_turn and (l_border[leftline_right_turn] < left_stop[leftline_right_turn] + margin):
            return self._result(False, reason="left_corner_too_close_stop")
        if rightline_left_turn and (r_border[rightline_left_turn] > right_stop[rightline_left_turn] - margin):
            return self._result(False, reason="right_corner_too_close_stop")

        # 判定是哪一侧单边桥（对齐 C 的比较方式）
        if leftline_right_turn > rightline_left_turn:
            side = "LEFT"   # 左单边桥（左边线向右拐入）
            y0 = leftline_right_turn
            ok, detail = self._verify_width_and_jump(bw, l_border, r_border, y0, side)
        else:
            side = "RIGHT"  # 右单边桥（右边线向左拐入）
            y0 = rightline_left_turn
            ok, detail = self._verify_width_and_jump(bw, l_border, r_border, y0, side)

        if not ok:
            return self._result(False, reason="verify_failed", extra={
                "side": side, "corner_y": y0,
                "verify": detail,
                "corners": {
                    "leftline_right_turn": leftline_right_turn,
                    "rightline_left_turn": rightline_left_turn,
                    "leftline_left_turn": leftline_left_turn,
                    "rightline_right_turn": rightline_right_turn
                }
            })

        return self._result(True, side=side, corner_y=y0, extra={
            "corners": {
                "leftline_right_turn": leftline_right_turn,
                "rightline_left_turn": rightline_left_turn,
                "leftline_left_turn": leftline_left_turn,
                "rightline_right_turn": rightline_right_turn
            },
            **detail
        })

    def _verify_width_and_jump(self, bw: np.ndarray, l_border: np.ndarray, r_border: np.ndarray, y0: int, side: str):
        h, w = bw.shape
        # 需要 y0±3 可用
        if y0 - 3 < 0 or y0 + 3 >= h:
            return False, {"reason": "y0_out_of_range"}

        if l_border[y0] < 0 or r_border[y0] < 0 or l_border[y0 - 3] < 0 or r_border[y0 - 3] < 0 or l_border[y0 + 3] < 0 or r_border[y0 + 3] < 0:
            return False, {"reason": "invalid_border_value"}

        w0 = int(r_border[y0] - l_border[y0])
        w_up = int(r_border[y0 - 3] - l_border[y0 - 3])
        w_dn = int(r_border[y0 + 3] - l_border[y0 + 3])

        # 宽度突变（对齐 C 的核心判断）
        if not (w_up <= w0 * self.p.width_shrink_ratio and w_up >= w_dn * self.p.width_min_ratio_vs_down and w_dn <= w0 * self.p.width_dn_ratio_max):
            return False, {"reason": "width_check_fail", "w0": w0, "w_up": w_up, "w_dn": w_dn}

        # 可选：BEV 下用物理宽度约束（桥宽 0.45m）
        if self.p.m_per_pixel is not None and self.p.expected_bridge_width_m is not None:
            width_m = w0 * self.p.m_per_pixel
            if abs(width_m - self.p.expected_bridge_width_m) > self.p.expected_width_tol_m:
                return False, {"reason": "metric_width_fail", "width_m": width_m, "w0_px": w0}

        # 跳变确认（对齐 C 的“扫固定 x，看黑白跳变”）
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

        # 注意：你C里是 i 从 y0+5 往上扫到 y0-20（i--）
        for yy in range(y_start, y_end - 1, -1):
            a = bw[yy - 1, x]
            b = bw[yy, x]
            # 以“背景黑、桥白”为默认
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

    @staticmethod
    def _result(found: bool, side: str = "", corner_y: int = 0, reason: str = "", extra: Optional[Dict[str, Any]] = None):
        out = {"found": bool(found)}
        if found:
            out.update({"side": side, "corner_y": int(corner_y)})
        else:
            out.update({"reason": reason})
        if extra:
            out["debug"] = extra
        return out

    # -------------------- 5) 可视化 --------------------
    def draw_debug(self, gray: np.ndarray, bw: np.ndarray, l_border: np.ndarray, r_border: np.ndarray, result: Dict[str, Any]) -> np.ndarray:
        vis = cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)
        h, w = gray.shape

        # 画边线
        for y in range(h):
            if l_border[y] >= 0:
                cv2.circle(vis, (int(l_border[y]), y), 1, (255, 0, 0), -1)
            if r_border[y] >= 0:
                cv2.circle(vis, (int(r_border[y]), y), 1, (0, 255, 0), -1)

        # 角点
        if result.get("found", False):
            y0 = int(result["corner_y"])
            if 0 <= y0 < h:
                cv2.circle(vis, (int(l_border[y0]), y0), 4, (0, 0, 255), 2)
                cv2.circle(vis, (int(r_border[y0]), y0), 4, (0, 0, 255), 2)
                txt = f"DanBianQiao: {result['side']}  y={y0}"
                cv2.putText(vis, txt, (10, 25), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
        else:
            cv2.putText(vis, f"No DanBianQiao ({result.get('reason','')})", (10, 25),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)

        return vis


def demo_image(image_path: str):
    gray = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
    if gray is None:
        raise FileNotFoundError(image_path)

    params = DanBianQiaoParams(
        # 你若是 BEV：把 m_per_pixel 填上可增强鲁棒性
        # m_per_pixel=0.002, expected_bridge_width_m=0.45
        use_otsu=True,
        close_ksize=3,
        open_ksize=3,
        min_run=30,
        require_jump_type="black_to_white",
    )
    det = DanBianQiaoDetector(params, debug=True)

    bw = det.preprocess_to_binary(gray)
    l_border, r_border, left_stop, right_stop, hightest = det.extract_borders(bw)
    result = det.detect_from_borders(bw, l_border, r_border, left_stop, right_stop, hightest)

    print("[DanBianQiao Result]")
    print(result)

    vis = det.draw_debug(gray, bw, l_border, r_border, result)

    cv2.imshow("gray", gray)
    cv2.imshow("bw", bw)
    cv2.imshow("danbianqiao_debug", vis)
    cv2.waitKey(0)
    cv2.destroyAllWindows()


if __name__ == "__main__":
    demo_image(r"bridge/img_gray_X0_Y45_Z106_P-30_Y0_R0.png")
