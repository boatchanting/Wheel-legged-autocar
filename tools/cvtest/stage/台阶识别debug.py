import cv2
import numpy as np
import json
import os
import math
from pathlib import Path


class StairDetectorStepProfile:
    def __init__(self, json_path, debug=None):
        if not os.path.exists(json_path):
            raise FileNotFoundError(f"找不到标定配置文件: {json_path}")

        with open(json_path, "r", encoding="utf-8") as f:
            self.config = json.load(f)

        # ---------- Debug 选项（可选） ----------
        dbg_cfg = self.config.get("debug", {})
        self.debug = bool(dbg_cfg.get("enabled", False)) if debug is None else bool(debug)
        self.debug_dir = str(dbg_cfg.get("dir", "debug"))
        self.debug_verbose = bool(dbg_cfg.get("verbose", True))
        self.debug_save_images = bool(dbg_cfg.get("save_images", True))
        self.debug_save_arrays = bool(dbg_cfg.get("save_arrays", False))  # 保存 .npy（一般不必开）
        os.makedirs(self.debug_dir, exist_ok=True)

        # 加载 IPM 映射表
        self.map_x = np.loadtxt(self.config["output_files"]["map_x"], dtype=np.float32)
        self.map_y = np.loadtxt(self.config["output_files"]["map_y"], dtype=np.float32)
        self.ipm_h, self.ipm_w = self.map_x.shape

        # 像素与距离参数
        self.m_per_pixel = float(self.config["m_per_pixel"])
        scale_corr = float(self.config.get("scale_correction", 1.0))
        self.m_per_pixel *= scale_corr

        # 相机在 BEV 图中的位置
        cam_ref = self.config.get("camera_ref_bev", None)
        if cam_ref is None:
            self.cam_x_pix = self.ipm_w / 2.0
            self.cam_y_pix = self.ipm_h * 1.0
        else:
            self.cam_x_pix = float(cam_ref[0])
            self.cam_y_pix = float(cam_ref[1])

        # 预设台阶宽度 500mm
        self.step_width_mm = 500.0
        self.step_width_px = (self.step_width_mm / 1000.0) / self.m_per_pixel

        self.resize_scale = float(self.config.get("resize_scale", 0.5))

        # 输出目录
        self.out_dir = self.config.get("output_dir", "stageresult")
        os.makedirs(self.out_dir, exist_ok=True)

        # 当前图片 debug 子目录（run() 时赋值）
        self._dbg_cur_dir = None

    # ===================== Debug Helper =====================
    def _dbg_prepare_dir(self, img_path):
        if not self.debug:
            self._dbg_cur_dir = None
            return
        name = os.path.splitext(os.path.basename(img_path))[0]
        d = os.path.join(self.debug_dir, name)
        os.makedirs(d, exist_ok=True)
        self._dbg_cur_dir = d

    def _dbg_path(self, filename):
        if (not self.debug) or (self._dbg_cur_dir is None):
            return None
        return os.path.join(self._dbg_cur_dir, filename)

    def _dbg_save_gray(self, filename, img):
        if not (self.debug and self.debug_save_images):
            return
        p = self._dbg_path(filename)
        if p is None:
            return
        im = img
        if im is None:
            return
        if im.dtype != np.uint8:
            im = np.clip(im, 0, 255).astype(np.uint8)
        cv2.imwrite(p, im)

    def _dbg_save_bgr(self, filename, img):
        if not (self.debug and self.debug_save_images):
            return
        p = self._dbg_path(filename)
        if p is None:
            return
        if img is None:
            return
        cv2.imwrite(p, img)

    def _dbg_save_npy(self, filename, arr):
        if not (self.debug and self.debug_save_arrays):
            return
        p = self._dbg_path(filename)
        if p is None:
            return
        np.save(p, arr)

    def _dbg_save_json(self, filename, obj):
        if not self.debug:
            return
        p = self._dbg_path(filename)
        if p is None:
            return
        with open(p, "w", encoding="utf-8") as f:
            json.dump(obj, f, ensure_ascii=False, indent=2)

    def _curve_to_image(self, curve, width=900, height=260, marks=None, mark_colors=None, title=None):
        """
        把 1D 曲线画成一张图（OpenCV），用于快速看 profile/grad。
        marks: 列表，每个元素是 index（曲线坐标系），会画竖线标记
        """
        curve = np.asarray(curve, dtype=np.float32).flatten()
        n = len(curve)
        if n <= 1:
            return np.zeros((height, width, 3), dtype=np.uint8)

        img = np.zeros((height, width, 3), dtype=np.uint8)
        # 边框
        cv2.rectangle(img, (0, 0), (width - 1, height - 1), (80, 80, 80), 1)

        # 归一化到图像坐标
        vmin = float(np.min(curve))
        vmax = float(np.max(curve))
        if abs(vmax - vmin) < 1e-6:
            vmax = vmin + 1.0

        def map_x(i):
            return int(round(i * (width - 1) / (n - 1)))

        def map_y(v):
            # 上小下大
            return int(round((1.0 - (v - vmin) / (vmax - vmin)) * (height - 1)))

        pts = []
        for i in range(n):
            pts.append((map_x(i), map_y(curve[i])))
        pts = np.array(pts, dtype=np.int32).reshape(-1, 1, 2)
        cv2.polylines(img, [pts], False, (0, 255, 0), 2)

        # marks
        if marks is not None and len(marks) > 0:
            if mark_colors is None:
                mark_colors = [(0, 0, 255)] * len(marks)
            for k, m in enumerate(marks):
                c = mark_colors[k] if k < len(mark_colors) else (0, 0, 255)
                x = map_x(int(np.clip(m, 0, n - 1)))
                cv2.line(img, (x, 0), (x, height - 1), c, 1)

        # 标注范围
        cv2.putText(img, f"min={vmin:.2f} max={vmax:.2f}", (10, height - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200, 200, 200), 1)

        if title:
            cv2.putText(img, title, (10, 20),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (200, 200, 200), 2)

        return img

    # ===================== BEV 预处理：无效区域置黑 =====================
    def preprocess_to_bev(self, img_path):
        raw_img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
        if raw_img is None:
            return None, None, None

        h, w = raw_img.shape[:2]
        new_w, new_h = int(w * self.resize_scale), int(h * self.resize_scale)
        if new_w <= 0 or new_h <= 0:
            return None, None, None

        small_img = cv2.resize(raw_img, (new_w, new_h), interpolation=cv2.INTER_AREA)

        # 有效映射 mask（落在 small_img 范围内）
        valid_mask = (
            (self.map_x >= 0) & (self.map_x < (new_w - 1)) &
            (self.map_y >= 0) & (self.map_y < (new_h - 1))
        )

        bev_gray = cv2.remap(
            small_img, self.map_x, self.map_y,
            interpolation=cv2.INTER_LINEAR,
            borderMode=cv2.BORDER_CONSTANT,
            borderValue=0
        )
        bev_gray[~valid_mask] = 0

        # Debug 输出
        self._dbg_save_gray("01_small.png", small_img)
        self._dbg_save_gray("02_bev_gray.png", bev_gray)
        self._dbg_save_gray("03_bev_valid_mask.png", (valid_mask.astype(np.uint8) * 255))

        # 可视化有效区边界
        if self.debug and self.debug_save_images:
            overlay = cv2.cvtColor(bev_gray, cv2.COLOR_GRAY2BGR)
            m = (valid_mask.astype(np.uint8) * 255)
            contours, _ = cv2.findContours(m, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
            cv2.drawContours(overlay, contours, -1, (0, 255, 255), 2)
            self._dbg_save_bgr("04_bev_mask_contour.png", overlay)

        return small_img, bev_gray, valid_mask.astype(np.uint8)

    # ===================== 角度估计（用 mask 降噪） =====================
    def estimate_dominant_angle(self, bev_img, bev_mask=None):
        h, w = bev_img.shape
        roi_w = int(w * 0.45)
        roi_x = int((w - roi_w) / 2)
        roi = bev_img[:, roi_x:roi_x + roi_w]

        if bev_mask is not None:
            roi_m = bev_mask[:, roi_x:roi_x + roi_w].astype(bool)
        else:
            roi_m = np.ones_like(roi, dtype=bool)

        grad_x = cv2.Sobel(roi, cv2.CV_32F, 1, 0, ksize=3)
        grad_y = cv2.Sobel(roi, cv2.CV_32F, 0, 1, ksize=3)
        mag, angle = cv2.cartToPolar(grad_x, grad_y, angleInDegrees=True)

        mag_thr = 40.0
        mask = (mag > mag_thr) & roi_m
        if np.count_nonzero(mask) < 200:
            return 0.0

        valid_angles = angle[mask]
        edge_angles = valid_angles - 90.0
        edge_angles = np.mod(edge_angles + 90, 180) - 90
        edge_angles = edge_angles[np.abs(edge_angles) < 30]

        if len(edge_angles) < 50:
            return 0.0

        hist, bins = np.histogram(edge_angles, bins=180, range=(-90, 90))
        peak_idx = int(np.argmax(hist))
        dominant_angle = float((bins[peak_idx] + bins[peak_idx + 1]) / 2.0)

        # Debug：保存 ROI + 边缘角分布
        if self.debug and self.debug_save_images:
            roi_vis = cv2.cvtColor(roi, cv2.COLOR_GRAY2BGR)
            cv2.rectangle(roi_vis, (0, 0), (roi_w - 1, h - 1), (0, 255, 255), 2)
            self._dbg_save_bgr("10_angle_roi.png", roi_vis)

            # 画一个简单的直方图图像
            hist_img = np.zeros((240, 900, 3), dtype=np.uint8)
            cv2.rectangle(hist_img, (0, 0), (899, 239), (80, 80, 80), 1)
            hist_n = hist.astype(np.float32)
            hist_n = hist_n / (np.max(hist_n) + 1e-6)
            for i in range(len(hist_n)):
                x = int(i * 900 / len(hist_n))
                y = int((1.0 - hist_n[i]) * 239)
                cv2.line(hist_img, (x, 239), (x, y), (255, 255, 255), 1)
            cv2.putText(hist_img, f"dominant={dominant_angle:.2f} deg", (10, 25),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
            self._dbg_save_bgr("11_angle_hist.png", hist_img)

        return dominant_angle

    # ===================== 旋转工具（同时支持 mask） =====================
    def rotate_image(self, image, angle_deg, interp=cv2.INTER_LINEAR):
        h, w = image.shape[:2]
        center = (w * 0.5, h * 0.5)
        M = cv2.getRotationMatrix2D(center, angle_deg, 1.0)
        rotated = cv2.warpAffine(image, M, (w, h), flags=interp, borderValue=0)
        return rotated, M

    def _apply_affine(self, M, x, y):
        v = np.array([x, y, 1.0], dtype=np.float32)
        out = M @ v
        return float(out[0]), float(out[1])

    # ===================== Step2 横向中心：特征/窗口鲁棒定位 =====================
    def compute_step_center_x(self, analyze_img, analyze_mask, y_top, y_bottom, cam_x_straight):
        h, w = analyze_img.shape
        y0 = int(max(0, min(y_top, y_bottom)))
        y1 = int(min(h, max(y_top, y_bottom)))
        if y1 - y0 < 8:
            return cam_x_straight, (cam_x_straight - self.step_width_px / 2, cam_x_straight + self.step_width_px / 2), "fallback_short_band", None

        band = analyze_img[y0:y1, :]
        band_m = analyze_mask[y0:y1, :].astype(bool) if analyze_mask is not None else np.ones_like(band, dtype=bool)

        gx = cv2.Sobel(band, cv2.CV_32F, 1, 0, ksize=3)
        gx = np.abs(gx)
        gx[~band_m] = 0
        edge_col = gx.mean(axis=0)
        edge_col_s = cv2.GaussianBlur(edge_col.reshape(1, -1), (1, 31), 0).flatten()

        thr = max(3.0, float(np.percentile(edge_col_s, 95) * 0.45))
        strong = np.where(edge_col_s > thr)[0]

        debug_pack = {
            "band_y0": y0, "band_y1": y1,
            "edge_thr": float(thr),
            "strong_count": int(len(strong)),
        }

        if len(strong) > 30:
            xl = float(np.percentile(strong, 5))
            xr = float(np.percentile(strong, 95))
            if xr - xl > max(0.35 * self.step_width_px, 30):
                cx = 0.5 * (xl + xr)
                debug_pack.update({"xl": xl, "xr": xr, "cx": cx, "mode": "edge_energy"})
                return cx, (xl, xr), "edge_energy", debug_pack

        presence = ((band > 2) & band_m).astype(np.float32)
        col_sum = presence.sum(axis=0)

        win = int(max(20, min(w, round(self.step_width_px))))
        if win >= w:
            cols = np.where(col_sum > 0)[0]
            if len(cols) > 0:
                xl, xr = float(cols.min()), float(cols.max())
                cx = 0.5 * (xl + xr)
                debug_pack.update({"xl": xl, "xr": xr, "cx": cx, "mode": "presence_full"})
                return cx, (xl, xr), "presence_full", debug_pack
            debug_pack.update({"mode": "fallback_no_presence"})
            return cam_x_straight, (cam_x_straight - self.step_width_px / 2, cam_x_straight + self.step_width_px / 2), "fallback_no_presence", debug_pack

        kernel = np.ones(win, dtype=np.float32)
        score = np.convolve(col_sum, kernel, mode="valid")
        centers = np.arange(len(score), dtype=np.float32) + win * 0.5
        penalty = np.abs(centers - float(cam_x_straight))
        lam = float(np.max(score) + 1e-6) * 0.02 / max(1.0, win)
        score2 = score - lam * penalty

        best = int(np.argmax(score2))
        xl = float(best)
        xr = float(best + win)
        cx = 0.5 * (xl + xr)

        debug_pack.update({
            "win": int(win),
            "best": int(best),
            "xl": xl, "xr": xr, "cx": cx,
            "lam": float(lam),
            "mode": "presence_window"
        })
        return cx, (xl, xr), "presence_window", debug_pack

    # ===================== 主检测流程（含 Debug 输出） =====================
    def analyze_profile_and_detect(self, bev_gray, bev_mask):
        h, w = bev_gray.shape
        debug_img = cv2.cvtColor(bev_gray, cv2.COLOR_GRAY2BGR)

        log = {}

        detected_angle = self.estimate_dominant_angle(bev_gray, bev_mask)
        log["yaw_angle_deg"] = float(detected_angle)
        if self.debug_verbose:
            print(f"检测到的旋转偏角: {detected_angle:.2f} 度")

        analyze_img, M = self.rotate_image(bev_gray, detected_angle, interp=cv2.INTER_LINEAR)
        analyze_mask, _ = self.rotate_image((bev_mask * 255).astype(np.uint8), detected_angle, interp=cv2.INTER_NEAREST)
        analyze_mask = (analyze_mask > 0).astype(np.uint8)

        self._dbg_save_gray("20_analyze_img.png", analyze_img)
        self._dbg_save_gray("21_analyze_mask.png", analyze_mask * 255)

        # 逆变换矩阵：拉直 -> 原 BEV
        M_inv = cv2.invertAffineTransform(M)

        def to_original(pt_straight):
            x, y = pt_straight
            ox, oy = self._apply_affine(M_inv, x, y)
            return int(round(ox)), int(round(oy))

        # 相机点在拉直坐标系
        cam_x_s, cam_y_s = self._apply_affine(M, self.cam_x_pix, self.cam_y_pix)
        log["cam_x_straight"] = float(cam_x_s)
        log["cam_y_straight"] = float(cam_y_s)

        # ROI 做 profile
        roi_w = int(w * 0.45)
        roi_x = int((w - roi_w) / 2)
        roi = analyze_img[:, roi_x:roi_x + roi_w]
        roi_m = analyze_mask[:, roi_x:roi_x + roi_w].astype(bool)

        if self.debug and self.debug_save_images:
            roi_vis = cv2.cvtColor(analyze_img, cv2.COLOR_GRAY2BGR)
            cv2.rectangle(roi_vis, (roi_x, 0), (roi_x + roi_w - 1, h - 1), (0, 255, 255), 2)
            self._dbg_save_bgr("22_analyze_roi_box.png", roi_vis)

        roi_f = roi.astype(np.float32)
        m_f = roi_m.astype(np.float32)
        denom = np.maximum(1.0, m_f.sum(axis=1))
        v_profile = (roi_f * m_f).sum(axis=1) / denom

        grad = np.gradient(v_profile)
        grad_smooth = cv2.GaussianBlur(grad.reshape(-1, 1).astype(np.float32), (5, 1), 0).flatten()

        # Debug：保存曲线图
        self._dbg_save_bgr("30_v_profile.png", self._curve_to_image(v_profile, title="v_profile"))
        self._dbg_save_bgr("31_grad_smooth.png", self._curve_to_image(grad_smooth, title="grad_smooth"))

        g_abs = np.abs(grad_smooth)
        thr = max(8.0, float(np.percentile(g_abs, 98) * 0.35))
        log["profile_thr"] = float(thr)

        detected_lines = []
        min_dist_between_lines = 15
        sorted_indices = np.argsort(g_abs)[::-1]

        for idx in sorted_indices:
            if g_abs[idx] < thr:
                break
            if any(abs(y - idx) < min_dist_between_lines for (y, _, _) in detected_lines):
                continue

            g_val = float(grad_smooth[idx])
            edge_type = "UNKNOWN"
            if g_val < -thr:
                edge_type = "BRIGHT_TO_DARK"
            elif g_val > thr:
                edge_type = "DARK_TO_BRIGHT"
            detected_lines.append((int(idx), edge_type, g_val))

        detected_lines.sort(key=lambda x: x[0])
        log["detected_lines"] = [{"y": int(y), "type": t, "g": float(g)} for (y, t, g) in detected_lines]

        if self.debug_verbose:
            print(f"Detected lines (count={len(detected_lines)}), thr={thr:.2f}")
            if len(detected_lines) <= 20:
                for item in detected_lines:
                    print("  ", item)

        # Debug：把 detected_lines 画到 grad_smooth 图上
        if self.debug and self.debug_save_images:
            marks = [y for (y, _, _) in detected_lines]
            colors = []
            for (_, t, _) in detected_lines:
                if t == "BRIGHT_TO_DARK":
                    colors.append((0, 0, 255))
                elif t == "DARK_TO_BRIGHT":
                    colors.append((255, 0, 0))
                else:
                    colors.append((0, 255, 255))
            self._dbg_save_bgr("32_grad_with_lines.png",
                               self._curve_to_image(grad_smooth, marks=marks, mark_colors=colors, title="grad + lines"))

        # 匹配 Step2（长度约 500mm）
        candidates_step2 = []
        expected_len_px = (0.5 / self.m_per_pixel)
        log["expected_len_px_step2"] = float(expected_len_px)

        for i in range(len(detected_lines) - 1):
            y_top, _, g_top = detected_lines[i]
            y_btm, _, g_btm = detected_lines[i + 1]
            if g_top < -thr and g_btm > thr:
                dist_px = y_btm - y_top
                if abs(dist_px - expected_len_px) < expected_len_px * 0.55:
                    candidates_step2.append((int(y_top), int(y_btm), float(dist_px)))

        log["candidates_step2"] = [{"y_top": a, "y_bottom": b, "len_px": d} for (a, b, d) in candidates_step2]

        if self.debug_verbose:
            print(f"Step2 candidates: {len(candidates_step2)}")
            if candidates_step2:
                for c in candidates_step2[-5:]:
                    print("  cand:", c)

        result_info = {"yaw_angle_deg": float(detected_angle)}

        if candidates_step2:
            y_step2_top, y_step2_bottom, _ = candidates_step2[-1]
            log["final_step2"] = {"y_top": int(y_step2_top), "y_bottom": int(y_step2_bottom)}

            cx, (xl, xr), cx_method, cx_dbg = self.compute_step_center_x(
                analyze_img, analyze_mask, y_step2_top, y_step2_bottom, cam_x_s
            )
            result_info["step2_center_method"] = cx_method
            log["step2_center_method"] = cx_method
            if cx_dbg is not None:
                log["step2_center_debug"] = cx_dbg

            # Debug：保存 band/gx/edge_col 曲线
            if self.debug and self.debug_save_images:
                y0 = int(max(0, min(y_step2_top, y_step2_bottom)))
                y1 = int(min(h, max(y_step2_top, y_step2_bottom)))
                band = analyze_img[y0:y1, :]
                self._dbg_save_gray("40_step2_band.png", band)

                gx = cv2.Sobel(band, cv2.CV_32F, 1, 0, ksize=3)
                gx = np.abs(gx)
                gx_u8 = np.clip((gx / (np.max(gx) + 1e-6)) * 255.0, 0, 255).astype(np.uint8)
                self._dbg_save_gray("41_step2_band_gx.png", gx_u8)

            # 横向偏移（以拉直坐标系的相机 x 为基准）
            offset_px = float(cx - cam_x_s)
            offset_m = float(offset_px * self.m_per_pixel)

            result_info["step2_offset_m"] = offset_m
            result_info["step2_offset_mm"] = offset_m * 1000.0
            log["step2_offset_px"] = offset_px
            log["step2_offset_mm"] = result_info["step2_offset_mm"]
            log["step2_center_x"] = float(cx)
            log["step2_xl_xr"] = [float(xl), float(xr)]

            # 用 500mm 宽度画蓝色框（拉直坐标系构建 -> 映射回原图）
            half_w = float(self.step_width_px) * 0.5
            pts_straight = [
                (cx - half_w, float(y_step2_top)),     # TL
                (cx + half_w, float(y_step2_top)),     # TR
                (cx + half_w, float(y_step2_bottom)),  # BR
                (cx - half_w, float(y_step2_bottom)),  # BL
            ]
            pts_orig = [to_original(p) for p in pts_straight]
            poly = np.array(pts_orig, np.int32).reshape((-1, 1, 2))
            cv2.polylines(debug_img, [poly], True, (255, 0, 0), 2)

            # Step2 中心线（绿） & 相机中心线（黄）
            cv2.line(debug_img, to_original((cx, float(y_step2_top))), to_original((cx, float(y_step2_bottom))), (0, 255, 0), 2)
            cv2.line(debug_img, to_original((cam_x_s, float(y_step2_top))), to_original((cam_x_s, float(y_step2_bottom))), (0, 255, 255), 2)

            # 距离（用拉直 y）
            dist_step2_front_m = float((cam_y_s - float(y_step2_bottom)) * self.m_per_pixel)
            result_info["step2_dist"] = dist_step2_front_m
            log["step2_dist_mm"] = dist_step2_front_m * 1000.0

            label_pos = pts_orig[3]
            cv2.putText(debug_img, f"S2: {dist_step2_front_m*1000:.0f}mm",
                        (label_pos[0] - 20, label_pos[1] + 25),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)
            cv2.putText(debug_img, f"OffsetX: {offset_m*1000:+.0f}mm ({cx_method})",
                        (label_pos[0] - 20, label_pos[1] + 50),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

        # Step1 起始线（保持你的逻辑）
        best_s1_y = -1
        max_g = 0.0
        if candidates_step2:
            search_start_y = int(candidates_step2[-1][1] + 20)
        else:
            search_start_y = int(h * 0.55)

        for (yy, _, gv) in detected_lines:
            if yy > search_start_y and gv < -thr:
                if abs(gv) > max_g:
                    max_g = abs(gv)
                    best_s1_y = yy

        if best_s1_y != -1:
            pt_start_orig = to_original((0.0, float(best_s1_y)))
            pt_end_orig = to_original((float(w - 1), float(best_s1_y)))
            cv2.line(debug_img, pt_start_orig, pt_end_orig, (0, 0, 255), 2)

            dist_s1 = float((cam_y_s - float(best_s1_y)) * self.m_per_pixel)
            result_info["step1_dist"] = dist_s1
            log["step1_dist_mm"] = dist_s1 * 1000.0
            cv2.putText(debug_img, f"S1 Start: {dist_s1*1000:.0f}mm",
                        (pt_start_orig[0] + 20, pt_start_orig[1] - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)

        # 显示 yaw
        cv2.putText(debug_img, f"Yaw: {detected_angle:.1f} deg", (20, 40),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)

        # Debug：保存 log.json + 最终 debug_img
        self._dbg_save_json("log.json", log)

        return debug_img, result_info

    def run(self, img_path):
        self._dbg_prepare_dir(img_path)

        small, bev, bev_mask = self.preprocess_to_bev(img_path)
        if bev is None:
            print("图片读取失败")
            return

        result_img, info = self.analyze_profile_and_detect(bev, bev_mask)

        print(f"\n分析结果: {os.path.basename(img_path)}")
        print(f" -> 估计旋转角(Yaw): {info.get('yaw_angle_deg', 0):.2f} 度")
        if "step2_dist" in info:
            print(f" -> 蓝台阶(第二级) 前沿垂直距离: {info['step2_dist']*1000:.1f} mm")
        if "step2_offset_mm" in info:
            print(f" -> 蓝台阶(第二级) 横向偏移: {info['step2_offset_mm']:+.1f} mm ({info.get('step2_center_method','')})")
        if "step1_dist" in info:
            print(f" -> 白台阶(第一级) 起始垂直距离: {info['step1_dist']*1000:.1f} mm")

        out_path = os.path.join(self.out_dir, f"result_{os.path.basename(img_path)}")
        cv2.imwrite(out_path, result_img)

        # 也把最终图存到 debug 目录里一份，便于对照
        self._dbg_save_bgr("99_result_overlay.png", result_img)


def process_images_in_folder(folder_path, calib_json="ipm_calib_result.json"):
    detector = StairDetectorStepProfile(calib_json)

    if not os.path.exists(folder_path):
        print(f"文件夹不存在: {folder_path}")
        return

    img_list = sorted([p for p in Path(folder_path).glob("*") if p.is_file() and p.suffix.lower() in [".png", ".jpg", ".jpeg", ".bmp"]])
    if not img_list:
        print(f"文件夹内没有图片: {folder_path}")
        return

    for file_path in img_list:
        try:
            print(f"\n正在处理: {file_path}")
            detector.run(str(file_path))
        except Exception as e:
            print(f"处理 {file_path} 出错: {e}")


if __name__ == "__main__":
    image_folder = r"stage"
    process_images_in_folder(image_folder, calib_json="ipm_calib_result.json")
