import cv2
import numpy as np
import json
import os
import math


class MinefieldDetector:
    """
    目标：BEV 平面上识别 1m*1m（内边0.8m）胶带方框（胶带宽0.1m），并在极端可见情形下依然能估计中心点。
    关键改进：
      1) 角点必须由两条近似正交的胶带边线段支撑（|cos| <= cos(75°)），过滤噪点角点；
      2) 评分从纯 IoU 改为 soft-score（偏向“解释观测”），对只看到一小部分更友好；
      3) “一个角点 + 两条半边”捷径：用两条正交线段求交点 -> 直接用几何偏移求中心 -> 评分挑最优。
    """

    def __init__(self, json_path):
        if not os.path.exists(json_path):
            raise FileNotFoundError(f"找不到标定配置文件: {json_path}")

        with open(json_path, "r", encoding="utf-8") as f:
            self.config = json.load(f)

        self.map_x_path = self.config["output_files"]["map_x"]
        self.map_y_path = self.config["output_files"]["map_y"]

        self.map_x = np.loadtxt(self.map_x_path, dtype=np.float32)
        self.map_y = np.loadtxt(self.map_y_path, dtype=np.float32)
        self.ipm_h, self.ipm_w = self.map_x.shape

        # === 真实 m_per_pixel ===
        self.m_per_pixel = float(self.config["m_per_pixel"])
        scale_corr = float(self.config.get("scale_correction", 1.0))
        self.m_per_pixel *= scale_corr

        # === 相机参考点（BEV 像素）===
        cam_ref = self.config.get("camera_ref_bev", None)
        if cam_ref is None:
            raise ValueError("json 中缺少 camera_ref_bev")
        self.cam_x_pix = float(cam_ref[0])
        self.cam_y_pix = float(cam_ref[1])

        shift = self.config.get("camera_origin_shift_px", [0.0, 0.0])
        self.cam_x_pix += float(shift[0])
        self.cam_y_pix += float(shift[1])

        # ===== 目标几何（米）=====
        self.square_outer_m = float(self.config.get("square_outer_m", 1.0))  # 外边 1.0
        self.square_inner_m = float(self.config.get("square_inner_m", 0.8))  # 内边 0.8
        self.tape_width_m = float(self.config.get("tape_width_m", 0.1))      # 胶带宽 0.1

        self.outer_half_m = self.square_outer_m * 0.5  # 0.5
        self.inner_half_m = self.square_inner_m * 0.5  # 0.4

        # ===== 基础参数 =====
        self.resize_scale = float(self.config.get("resize_scale", 0.5))

        # 形态学 / 环带容忍（米）
        self.band_tol_m = float(self.config.get("band_tol_m", 0.02))

        # 面积阈值（用于无元素快速退出）
        self.min_area_threshold = (0.1 / self.m_per_pixel) ** 2

        # ===== 角点检测参数 =====
        self.max_corners = int(self.config.get("max_corners", 80))
        self.corner_quality = float(self.config.get("corner_quality", 0.01))
        self.corner_min_dist_m = float(self.config.get("corner_min_dist_m", 0.05))

        # ===== 线段检测参数 =====
        self.hough_min_len_m = float(self.config.get("hough_min_len_m", 0.20))
        self.hough_max_gap_m = float(self.config.get("hough_max_gap_m", 0.05))

        # ===== 角点-边线一致性（抗噪）=====
        self.ortho_deg_min = float(self.config.get("ortho_deg_min", 75.0))  # 夹角>=75°
        self.cos_ortho_max = math.cos(math.radians(self.ortho_deg_min))     # |cos|<=cos(75)

        self.corner_support_dist_m = float(self.config.get("corner_support_dist_m", 0.06))
        self.corner_support_dist_px = max(3, int(self.corner_support_dist_m / self.m_per_pixel))

        self.corner_support_min_len_m = float(self.config.get("corner_support_min_len_m", 0.18))
        self.corner_support_min_len_px = max(10, int(self.corner_support_min_len_m / self.m_per_pixel))

        # ===== 软评分（替代纯IoU）=====
        # alpha 越小越偏向“解释观测”，对只看到一点更友好（0.2~0.35）
        self.partial_alpha = float(self.config.get("partial_alpha", 0.25))
        # soft 评分阈值（比 IoU 更合理）
        self.pose_soft_thresh = float(self.config.get("pose_soft_thresh", 0.10))

        print(f"[Init] BEV: {self.ipm_w}x{self.ipm_h}")
        print(f"[Init] m_per_pixel: {self.m_per_pixel:.6f} (corr={scale_corr})")
        print(f"[Init] camera_ref_bev: ({self.cam_x_pix:.2f}, {self.cam_y_pix:.2f})")
        print(f"[Init] target outer={self.square_outer_m}m inner={self.square_inner_m}m tape={self.tape_width_m}m")

    # -----------------------------
    # 工具函数
    # -----------------------------
    def _wrap_pi(self, a):
        while a > math.pi:
            a -= 2 * math.pi
        while a < -math.pi:
            a += 2 * math.pi
        return a

    def _pt_seg_dist(self, px, py, x1, y1, x2, y2):
        vx, vy = (x2 - x1), (y2 - y1)
        wx, wy = (px - x1), (py - y1)
        l2 = vx * vx + vy * vy + 1e-6
        t = (wx * vx + wy * vy) / l2
        t = max(0.0, min(1.0, t))
        cx = x1 + t * vx
        cy = y1 + t * vy
        return math.hypot(px - cx, py - cy)

    def _seg_dir_unit(self, seg):
        x1, y1 = seg["p1"]
        x2, y2 = seg["p2"]
        dx, dy = (x2 - x1), (y2 - y1)
        n = math.hypot(dx, dy) + 1e-6
        return (dx / n, dy / n)

    def _abs_cos(self, u, v):
        return abs(u[0] * v[0] + u[1] * v[1])

    def _line_intersection(self, a1, a2, b1, b2):
        # 两直线（无限延长）交点
        x1, y1 = a1
        x2, y2 = a2
        x3, y3 = b1
        x4, y4 = b2
        den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)
        if abs(den) < 1e-6:
            return None
        px = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / den
        py = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / den
        return (float(px), float(py))

    # -----------------------------
    # 1) 预处理：IPM + 增强 + 胶带二值
    # -----------------------------
    def preprocess_image(self, img_path):
        raw_img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
        if raw_img is None:
            raise FileNotFoundError(f"无法读取图片: {img_path}")

        scale = self.resize_scale
        h, w = raw_img.shape[:2]
        new_w, new_h = int(w * scale), int(h * scale)
        small_img = cv2.resize(raw_img, (new_w, new_h))

        bev_gray = cv2.remap(
            small_img, self.map_x, self.map_y,
            cv2.INTER_LINEAR,
            borderMode=cv2.BORDER_CONSTANT,
            borderValue=0
        )

        valid_mask = (bev_gray > 0).astype(np.uint8) * 255

        clahe = cv2.createCLAHE(clipLimit=4.0, tileGridSize=(8, 8))
        enhanced = clahe.apply(bev_gray)

        tape_px = int((self.tape_width_m * 1.2) / self.m_per_pixel)
        if tape_px % 2 == 0:
            tape_px += 1
        tape_px = max(tape_px, 3)

        tophat_kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (tape_px, tape_px))
        tophat = cv2.morphologyEx(enhanced, cv2.MORPH_TOPHAT, tophat_kernel)

        thr = int(self.config.get("tape_thresh", 50))
        _, binary = cv2.threshold(tophat, thr, 255, cv2.THRESH_BINARY)
        binary = cv2.bitwise_and(binary, binary, mask=valid_mask)

        return small_img, bev_gray, enhanced, binary, valid_mask

    def process_morphology(self, binary_img):
        tape_pixels = max(3, int(self.tape_width_m / self.m_per_pixel))
        k_size = max(3, tape_pixels // 2)

        kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (k_size, k_size))
        closed = cv2.morphologyEx(binary_img, cv2.MORPH_CLOSE, kernel)
        eroded = cv2.erode(closed, kernel, iterations=1)
        result = cv2.dilate(eroded, kernel, iterations=1)
        return result

    # -----------------------------
    # 2) 特征提取：角点 + 线段
    # -----------------------------
    def extract_features(self, enhanced_gray, tape_mask):
        # 角点：只在胶带附近找（mask 略膨胀）
        dil = cv2.dilate(tape_mask, cv2.getStructuringElement(cv2.MORPH_RECT, (5, 5)), iterations=1)

        min_dist_px = max(3, int(self.corner_min_dist_m / self.m_per_pixel))
        corners = cv2.goodFeaturesToTrack(
            enhanced_gray,
            maxCorners=self.max_corners,
            qualityLevel=self.corner_quality,
            minDistance=min_dist_px,
            mask=dil,
            blockSize=5,
            useHarrisDetector=True,
            k=0.04
        )

        corner_pts = []
        if corners is not None:
            corners = corners.reshape(-1, 2)
            for (x, y) in corners:
                corner_pts.append((float(x), float(y)))

        # 线段：Canny + HoughLinesP
        canny1 = int(self.config.get("canny1", 60))
        canny2 = int(self.config.get("canny2", 180))
        edges = cv2.Canny(enhanced_gray, canny1, canny2)
        edges = cv2.bitwise_and(edges, edges, mask=dil)

        min_len_px = max(10, int(self.hough_min_len_m / self.m_per_pixel))
        max_gap_px = max(3, int(self.hough_max_gap_m / self.m_per_pixel))

        lines = cv2.HoughLinesP(
            edges,
            rho=1,
            theta=np.pi / 180.0,
            threshold=int(self.config.get("hough_threshold", 40)),
            minLineLength=min_len_px,
            maxLineGap=max_gap_px
        )

        segments = []
        if lines is not None:
            for l in lines.reshape(-1, 4):
                x1, y1, x2, y2 = map(float, l)
                dx, dy = x2 - x1, y2 - y1
                length = math.hypot(dx, dy)
                if length < min_len_px:
                    continue
                ang = math.atan2(dy, dx)
                segments.append({
                    "p1": (x1, y1),
                    "p2": (x2, y2),
                    "len": float(length),     # 像素长度
                    "ang": float(ang),
                    "mid": ((x1 + x2) * 0.5, (y1 + y2) * 0.5)
                })

        return corner_pts, segments, edges

    # -----------------------------
    # 3) 角点必须由两条正交线段支撑（过滤噪点角点）
    # -----------------------------
    def _pick_two_orth_segments_near_pt(self, pt, segments):
        px, py = pt
        cands = []
        for seg in segments:
            if seg["len"] < self.corner_support_min_len_px:
                continue
            x1, y1 = seg["p1"]
            x2, y2 = seg["p2"]
            d = self._pt_seg_dist(px, py, x1, y1, x2, y2)
            if d <= self.corner_support_dist_px:
                cands.append(seg)

        if len(cands) < 2:
            return None

        cands = sorted(cands, key=lambda s: -s["len"])[:20]
        best = None
        best_len = -1.0
        for i in range(len(cands)):
            ui = self._seg_dir_unit(cands[i])
            for j in range(i + 1, len(cands)):
                uj = self._seg_dir_unit(cands[j])
                if self._abs_cos(ui, uj) <= self.cos_ortho_max:
                    score = cands[i]["len"] + cands[j]["len"]
                    if score > best_len:
                        best_len = score
                        best = (cands[i], cands[j], ui, uj)
        return best

    def filter_corners_by_line_support(self, corners, segments, obs_mask):
        kept = []
        for (x, y) in corners:
            xi, yi = int(round(x)), int(round(y))
            if xi < 0 or xi >= self.ipm_w or yi < 0 or yi >= self.ipm_h:
                continue

            # 必须在胶带响应附近（否则极易是噪声）
            x0 = max(0, xi - 2)
            x1 = min(self.ipm_w, xi + 3)
            y0 = max(0, yi - 2)
            y1 = min(self.ipm_h, yi + 3)
            if np.mean(obs_mask[y0:y1, x0:x1]) < 5:
                continue

            ok = self._pick_two_orth_segments_near_pt((x, y), segments)
            if ok is None:
                continue

            kept.append((x, y))
        return kept

    # -----------------------------
    # 4) 评分：soft-score（对局部可见更友好） + 同时返回 IoU
    # -----------------------------
    def score_pose_soft(self, obs_mask, cx, cy, phi):
        margin_m = float(self.config.get("score_roi_margin_m", 0.20))
        half = self.outer_half_m + margin_m
        half_px = int(half / self.m_per_pixel)

        x0 = int(max(0, math.floor(cx) - half_px))
        x1 = int(min(self.ipm_w, math.ceil(cx) + half_px))
        y0 = int(max(0, math.floor(cy) - half_px))
        y1 = int(min(self.ipm_h, math.ceil(cy) + half_px))
        if x1 <= x0 + 5 or y1 <= y0 + 5:
            return 0.0, 0.0

        roi = obs_mask[y0:y1, x0:x1]
        obs_cnt = int(np.sum(roi > 0))
        if obs_cnt <= 0:
            return 0.0, 0.0

        xs = np.arange(x0, x1, dtype=np.float32)
        ys = np.arange(y0, y1, dtype=np.float32)
        X, Y = np.meshgrid(xs, ys)

        dx = X - float(cx)
        dy = Y - float(cy)

        c = math.cos(phi)
        s = math.sin(phi)

        xp = dx * c + dy * s
        yp = -dx * s + dy * c

        d = np.maximum(np.abs(xp), np.abs(yp)) * self.m_per_pixel

        inner = (self.inner_half_m - self.band_tol_m)
        outer = (self.outer_half_m + self.band_tol_m)
        pred = (d >= inner) & (d <= outer)
        pred_cnt = int(np.sum(pred))
        if pred_cnt <= 0:
            return 0.0, 0.0

        inter = int(np.sum(pred & (roi > 0)))
        union = int(np.sum(pred | (roi > 0)))
        iou = float(inter) / float(union + 1e-6)

        denom = self.partial_alpha * pred_cnt + (1.0 - self.partial_alpha) * obs_cnt
        soft = float(inter) / float(denom + 1e-6)
        return soft, iou

    # -----------------------------
    # 5) 候选生成（角点对 / 线段）
    # -----------------------------
    def gen_candidates(self, corners, segments):
        cands = []

        side_ms = [self.square_outer_m, self.square_inner_m]
        diag_ms = [m * math.sqrt(2.0) for m in side_ms]

        def add_cand(cx, cy, phi, reason):
            if not (0 <= cx < self.ipm_w and 0 <= cy < self.ipm_h):
                return
            cands.append({"cx": float(cx), "cy": float(cy), "phi": float(self._wrap_pi(phi)), "reason": reason})

        # A) 角点对：邻边 / 对角
        max_corner_use = min(len(corners), int(self.config.get("max_corner_use", 30)))
        corners_use = corners[:max_corner_use]

        for i in range(len(corners_use)):
            x1, y1 = corners_use[i]
            for j in range(i + 1, len(corners_use)):
                x2, y2 = corners_use[j]
                dx, dy = (x2 - x1), (y2 - y1)
                dpx = math.hypot(dx, dy)
                if dpx < 5:
                    continue
                d_m = dpx * self.m_per_pixel
                ang = math.atan2(dy, dx)

                for side_m in side_ms:
                    if abs(d_m - side_m) < 0.18:
                        mx, my = (x1 + x2) * 0.5, (y1 + y2) * 0.5
                        n1 = (-math.sin(ang), math.cos(ang))
                        off = (side_m * 0.5) / self.m_per_pixel
                        add_cand(mx + n1[0] * off, my + n1[1] * off, ang, f"corner_pair_adj_side={side_m}")
                        add_cand(mx - n1[0] * off, my - n1[1] * off, ang, f"corner_pair_adj_side={side_m}")

                for diag_m in diag_ms:
                    if abs(d_m - diag_m) < 0.22:
                        mx, my = (x1 + x2) * 0.5, (y1 + y2) * 0.5
                        add_cand(mx, my, ang - math.pi / 4.0, f"corner_pair_diag={diag_m:.2f}")
                        add_cand(mx, my, ang + math.pi / 4.0, f"corner_pair_diag={diag_m:.2f}")

        # B) 单线段：法向偏移 + 沿边滑动
        ring_offsets_m = [self.inner_half_m, (self.inner_half_m + self.outer_half_m) * 0.5, self.outer_half_m]
        segs = sorted(segments, key=lambda s: -s["len"])
        segs = segs[:int(self.config.get("max_seg_use", 25))]

        slide_range_m = float(self.config.get("seg_slide_range_m", 0.60))
        slide_step_m = float(self.config.get("seg_slide_step_m", 0.06))
        slide_steps = max(1, int(slide_range_m / max(slide_step_m, 1e-6)))

        for seg in segs:
            ang = seg["ang"]
            mx, my = seg["mid"]
            u = (math.cos(ang), math.sin(ang))
            n = (-u[1], u[0])

            for off_m in ring_offsets_m:
                off_px = off_m / self.m_per_pixel
                for k in range(-slide_steps, slide_steps + 1):
                    t_m = k * slide_step_m
                    t_px = t_m / self.m_per_pixel
                    base_x = mx + u[0] * t_px
                    base_y = my + u[1] * t_px

                    add_cand(base_x + n[0] * off_px, base_y + n[1] * off_px, ang, f"seg_off={off_m:.2f}")
                    add_cand(base_x - n[0] * off_px, base_y - n[1] * off_px, ang, f"seg_off={off_m:.2f}")

        return cands

    # -----------------------------
    # 6) 图2捷径：一角 + 两半边（两条正交线段相交）
    # -----------------------------
    def solve_pose_from_one_corner_two_edges(self, obs_mask, segments):
        segs = sorted(segments, key=lambda s: -s["len"])[:30]
        best = None
        best_soft = -1.0
        best_iou = 0.0
        best_corner = None
        best_meta = None

        for i in range(len(segs)):
            ui = self._seg_dir_unit(segs[i])
            for j in range(i + 1, len(segs)):
                uj = self._seg_dir_unit(segs[j])
                if self._abs_cos(ui, uj) > self.cos_ortho_max:
                    continue

                p = self._line_intersection(segs[i]["p1"], segs[i]["p2"], segs[j]["p1"], segs[j]["p2"])
                if p is None:
                    continue
                px, py = p
                if not (0 <= px < self.ipm_w and 0 <= py < self.ipm_h):
                    continue

                # 交点必须靠近两段线（不是延长线瞎交）
                x1, y1 = segs[i]["p1"]
                x2, y2 = segs[i]["p2"]
                di = self._pt_seg_dist(px, py, x1, y1, x2, y2)
                x3, y3 = segs[j]["p1"]
                x4, y4 = segs[j]["p2"]
                dj = self._pt_seg_dist(px, py, x3, y3, x4, y4)
                if di > self.corner_support_dist_px or dj > self.corner_support_dist_px:
                    continue

                # 交点附近要有胶带响应
                r = max(3, int(0.04 / self.m_per_pixel))
                x0 = max(0, int(px) - r)
                x1p = min(self.ipm_w, int(px) + r + 1)
                y0 = max(0, int(py) - r)
                y1p = min(self.ipm_h, int(py) + r + 1)
                if np.mean(obs_mask[y0:y1p, x0:x1p]) < 10:
                    continue

                # 枚举：外/内两种半边 & 4种象限
                for half_m in [self.outer_half_m, self.inner_half_m]:
                    off_px = half_m / self.m_per_pixel
                    for sx in (+1, -1):
                        for sy in (+1, -1):
                            cx = px + sx * ui[0] * off_px + sy * uj[0] * off_px
                            cy = py + sx * ui[1] * off_px + sy * uj[1] * off_px
                            if not (0 <= cx < self.ipm_w and 0 <= cy < self.ipm_h):
                                continue

                            phi = math.atan2(ui[1], ui[0])  # 用 ui 作为边方向
                            soft, iou = self.score_pose_soft(obs_mask, cx, cy, phi)
                            if soft > best_soft:
                                best_soft = soft
                                best_iou = iou
                                best = (cx, cy, phi)
                                best_corner = (px, py)
                                best_meta = (half_m, (sx, sy))

        if best is None:
            return None

        cx, cy, phi = best
        return {
            "cx": float(cx), "cy": float(cy), "phi": float(self._wrap_pi(phi)),
            "soft": float(best_soft), "iou": float(best_iou),
            "corner": best_corner, "meta": best_meta,
        }

    # -----------------------------
    # 7) 可见角点估计（用于输出标签）
    # -----------------------------
    def count_visible_corners(self, obs_mask, cx, cy, phi):
        c = math.cos(phi)
        s = math.sin(phi)

        corners_local = [
            (+self.outer_half_m, +self.outer_half_m),
            (+self.outer_half_m, -self.outer_half_m),
            (-self.outer_half_m, +self.outer_half_m),
            (-self.outer_half_m, -self.outer_half_m),
        ]

        r_px = max(3, int(float(self.config.get("corner_check_radius_m", 0.06)) / self.m_per_pixel))
        vis = 0
        pts = []

        for (lx, ly) in corners_local:
            px = (lx / self.m_per_pixel)
            py = (ly / self.m_per_pixel)

            dx = px * c - py * s
            dy = px * s + py * c

            x = int(round(cx + dx))
            y = int(round(cy + dy))
            pts.append((x, y))

            x0 = max(0, x - r_px)
            x1 = min(self.ipm_w, x + r_px + 1)
            y0 = max(0, y - r_px)
            y1 = min(self.ipm_h, y + r_px + 1)
            if x1 <= x0 or y1 <= y0:
                continue
            patch = obs_mask[y0:y1, x0:x1]
            if np.mean(patch) > 20:
                vis += 1

        return vis, pts

    def draw_square_overlay(self, img_bgr, cx, cy, phi, color=(0, 255, 0)):
        c = math.cos(phi)
        s = math.sin(phi)

        def rect_points(half_m):
            pts = []
            for (lx, ly) in [(+half_m, +half_m), (+half_m, -half_m), (-half_m, -half_m), (-half_m, +half_m)]:
                px = lx / self.m_per_pixel
                py = ly / self.m_per_pixel
                dx = px * c - py * s
                dy = px * s + py * c
                pts.append((int(round(cx + dx)), int(round(cy + dy))))
            return np.array(pts, dtype=np.int32)

        outer_pts = rect_points(self.outer_half_m)
        inner_pts = rect_points(self.inner_half_m)

        cv2.polylines(img_bgr, [outer_pts], True, color, 2)
        cv2.polylines(img_bgr, [inner_pts], True, (0, 255, 255), 2)
        cv2.circle(img_bgr, (int(round(cx)), int(round(cy))), 5, (0, 0, 255), -1)

    # -----------------------------
    # 8) 主检测：先捷径，再候选扫描（soft-score 排序）
    # -----------------------------
    def detect_by_pose(self, obs_mask, enhanced_gray, debug_img_color):
        corners, segments, edges = self.extract_features(enhanced_gray, obs_mask)

        # 相机参考点
        cv2.circle(debug_img_color, (int(round(self.cam_x_pix)), int(round(self.cam_y_pix))), 6, (255, 0, 0), -1)
        cv2.putText(debug_img_color, "CAM_REF",
                    (int(self.cam_x_pix) + 8, int(self.cam_y_pix) - 8),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 2)

        # 线段可视化（少量）
        for seg in sorted(segments, key=lambda s: -s["len"])[:40]:
            x1, y1 = seg["p1"]
            x2, y2 = seg["p2"]
            cv2.line(debug_img_color, (int(x1), int(y1)), (int(x2), int(y2)), (128, 128, 255), 1)

        # 1) 过滤噪点角点：必须有两条正交边线支撑（解决图1）
        corners_f = self.filter_corners_by_line_support(corners, segments, obs_mask)

        # 角点可视化
        for (x, y) in corners_f[:60]:
            cv2.circle(debug_img_color, (int(round(x)), int(round(y))), 2, (255, 255, 0), -1)

        best = None
        best_soft = -1.0
        best_iou = 0.0
        best_reason = "none"
        best_corner = None

        # 2) 图2捷径：一角 + 两半边直接求中心
        fast = self.solve_pose_from_one_corner_two_edges(obs_mask, segments)
        if fast is not None and fast["soft"] >= self.pose_soft_thresh:
            best = {"cx": fast["cx"], "cy": fast["cy"], "phi": fast["phi"]}
            best_soft = fast["soft"]
            best_iou = fast["iou"]
            best_reason = "corner+2edges"
            best_corner = fast.get("corner", None)

        # 3) 候选扫描：用 soft-score 排序
        cands = self.gen_candidates(corners_f, segments)
        if cands:
            seen = set()
            for cand in cands:
                key = (int(cand["cx"] // 5), int(cand["cy"] // 5), int((cand["phi"] + math.pi) * 20))
                if key in seen:
                    continue
                seen.add(key)

                soft, iou = self.score_pose_soft(obs_mask, cand["cx"], cand["cy"], cand["phi"])
                if soft > best_soft:
                    best_soft = soft
                    best_iou = iou
                    best = cand
                    best_reason = cand.get("reason", "cand")
                    best_corner = None

        if best is None or best_soft < self.pose_soft_thresh:
            cv2.putText(debug_img_color, f"POSE_FAIL soft={best_soft:.3f} iou={best_iou:.3f}",
                        (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)
            return None, debug_img_color, edges

        cx, cy, phi = best["cx"], best["cy"], best["phi"]

        # 可视化：画预测方框 & 角点检查
        self.draw_square_overlay(debug_img_color, cx, cy, phi, color=(0, 255, 0))
        vis_cnt, corner_pix = self.count_visible_corners(obs_mask, cx, cy, phi)
        for p in corner_pix:
            cv2.circle(debug_img_color, p, 4, (0, 0, 255), 1)

        # 若是捷径，画出交点角
        if best_corner is not None:
            cv2.circle(debug_img_color, (int(round(best_corner[0])), int(round(best_corner[1]))), 5, (0, 255, 255), -1)

        cv2.putText(debug_img_color,
                    f"POSE_OK soft={best_soft:.3f} iou={best_iou:.3f} visCorners={vis_cnt} {best_reason}",
                    (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 255, 0), 2)

        # 输出结果（中心点）
        dist_x_m = (cx - self.cam_x_pix) * self.m_per_pixel
        dist_y_m = (self.cam_y_pix - cy) * self.m_per_pixel

        if vis_cnt >= 4:
            tag = "Square [4 corners]"
        elif vis_cnt == 3:
            tag = "Square [3 corners]"
        elif vis_cnt == 2:
            tag = "Square [2 corners]"
        elif vis_cnt == 1:
            tag = "Square [1 corner]"
        else:
            tag = "Square [partial]"

        obj_info = {
            "type": tag,
            "uv": (float(cx), float(cy)),
            "xy_meter": (float(dist_x_m), float(dist_y_m)),
            "wh_meter": (float(self.square_outer_m), float(self.square_outer_m)),
            "pose": (float(cx), float(cy), float(phi)),
            "soft": float(best_soft),
            "iou": float(best_iou),
            "visible_corners": int(vis_cnt),
            "reason": best_reason,
        }
        return obj_info, debug_img_color, edges

    # -----------------------------
    # 9) 运行入口
    # -----------------------------
    def run(self, img_path):
        small, bev, enhanced, binary, valid_mask = self.preprocess_image(img_path)
        morph = self.process_morphology(binary)

        debug_img = cv2.cvtColor(bev, cv2.COLOR_GRAY2BGR)

        # 无元素快速退出
        total_area = int(np.sum(morph > 0))
        if total_area < max(80, int(self.min_area_threshold * 0.2)):
            results = []
            edges = np.zeros_like(bev)
            cv2.putText(debug_img, "NO_TAPE_AREA", (10, 20),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
        else:
            obj, debug_img, edges = self.detect_by_pose(morph, enhanced, debug_img)
            results = []
            if obj is not None:
                results.append(obj)

        print(f"\n========== 识别报告: {os.path.basename(img_path)} ==========")
        if not results:
            print("未检测到可靠的雷区方框（或只检测到噪声）。")
        else:
            r = results[0]
            print(f"目标 1 [{r['type']}]:")
            print(f"  - soft评分: {r['soft']:.3f}  IoU: {r['iou']:.3f}  可见角点: {r['visible_corners']}")
            print(f"  - 来源: {r['reason']}")
            print(f"  - BEV像素坐标: ({r['uv'][0]:.1f}, {r['uv'][1]:.1f})")
            print(f"  - 相对相机坐标: X={r['xy_meter'][0]:.3f}m, Y={r['xy_meter'][1]:.3f}m")
            print(f"  - 目标尺寸(固定): {r['wh_meter'][0]:.2f}m x {r['wh_meter'][1]:.2f}m")

        out_prefix = "result_" + os.path.splitext(os.path.basename(img_path))[0]
        cv2.imwrite(f"{out_prefix}_bev.png", bev)
        cv2.imwrite(f"{out_prefix}_binary.png", binary)
        cv2.imwrite(f"{out_prefix}_morph.png", morph)
        cv2.imwrite(f"{out_prefix}_edges.png", edges)
        cv2.imwrite(f"{out_prefix}_detect.png", debug_img)
        print(f"结果图片已保存为 {out_prefix}_detect.png 等")


if __name__ == "__main__":
    json_config_path = "ipm_calib_result.json"
    test_img_path = r"img_gray_X0_Y42_Z120_P-30_Y0_R0.png"

    if not os.path.exists(json_config_path):
        print("错误：请先运行【表格标定.py】生成 ipm_calib_result.json 和映射表文件！")
    else:
        detector = MinefieldDetector(json_config_path)
        detector.run(test_img_path)
