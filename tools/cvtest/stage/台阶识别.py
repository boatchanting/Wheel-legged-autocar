import cv2
import numpy as np
import json
import os
import math
from pathlib import Path


class StairDetectorStepProfile:
    def __init__(self, json_path):
        if not os.path.exists(json_path):
            raise FileNotFoundError(f"找不到标定配置文件: {json_path}")

        with open(json_path, "r", encoding="utf-8") as f:
            self.config = json.load(f)

        # 加载 IPM 映射表（这些表通常对应 resize 后的小图坐标）
        self.map_x = np.loadtxt(self.config["output_files"]["map_x"], dtype=np.float32)
        self.map_y = np.loadtxt(self.config["output_files"]["map_y"], dtype=np.float32)
        self.ipm_h, self.ipm_w = self.map_x.shape

        # 像素与距离参数
        self.m_per_pixel = float(self.config["m_per_pixel"])
        scale_corr = float(self.config.get("scale_correction", 1.0))
        self.m_per_pixel *= scale_corr

        # 相机在 BEV 图中的参考像素位置
        cam_ref = self.config.get("camera_ref_bev", None)
        if cam_ref is None:
            self.cam_x_pix = self.ipm_w / 2.0
            self.cam_y_pix = self.ipm_h * 1.0
        else:
            self.cam_x_pix = float(cam_ref[0])
            self.cam_y_pix = float(cam_ref[1])

        # 台阶（赛道）宽度 500mm
        self.step_width_mm = 500.0
        self.step_width_px = (self.step_width_mm / 1000.0) / self.m_per_pixel

        self.resize_scale = float(self.config.get("resize_scale", 0.5))

        # 输出目录
        self.out_dir = self.config.get("output_dir", "stageresult")
        os.makedirs(self.out_dir, exist_ok=True)

    # ------------------------- BEV 预处理：无效区域置黑 -------------------------
    def preprocess_to_bev(self, img_path):
        raw_img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
        if raw_img is None:
            return None, None, None

        h, w = raw_img.shape[:2]
        new_w, new_h = int(w * self.resize_scale), int(h * self.resize_scale)
        if new_w <= 0 or new_h <= 0:
            return None, None, None

        small_img = cv2.resize(raw_img, (new_w, new_h), interpolation=cv2.INTER_AREA)

        # 生成有效映射 mask：只要 map_x/map_y 落在 small_img 范围内，就是有效像素
        valid_mask = (
            (self.map_x >= 0) & (self.map_x < (new_w - 1)) &
            (self.map_y >= 0) & (self.map_y < (new_h - 1))
        )

        # IPM 变换（无效映射先 remap 出来，再用 mask 置黑，保证“其余区域黑色”）
        bev_gray = cv2.remap(
            small_img, self.map_x, self.map_y,
            interpolation=cv2.INTER_LINEAR,
            borderMode=cv2.BORDER_CONSTANT,
            borderValue=0
        )

        bev_gray[~valid_mask] = 0
        return small_img, bev_gray, valid_mask.astype(np.uint8)  # mask 用 0/1

    # ------------------------- 角度估计（用有效 mask 降噪） -------------------------
    def estimate_dominant_angle(self, bev_img, bev_mask=None):
        """
        估计台阶边缘相对于水平线的旋转角度。
        返回角度 degree（warpAffine 的 angle，正数逆时针）
        """
        h, w = bev_img.shape

        # 只取中心区域做统计，避免边缘杂散
        roi_w = int(w * 0.45)
        roi_x = int((w - roi_w) / 2)
        roi = bev_img[:, roi_x:roi_x + roi_w]

        if bev_mask is not None:
            roi_m = bev_mask[:, roi_x:roi_x + roi_w].astype(bool)
        else:
            roi_m = np.ones_like(roi, dtype=bool)

        # Sobel 梯度
        grad_x = cv2.Sobel(roi, cv2.CV_32F, 1, 0, ksize=3)
        grad_y = cv2.Sobel(roi, cv2.CV_32F, 0, 1, ksize=3)
        mag, angle = cv2.cartToPolar(grad_x, grad_y, angleInDegrees=True)

        # 过滤弱梯度 + 无效区域
        mag_thr = 40.0
        mask = (mag > mag_thr) & roi_m
        if np.count_nonzero(mask) < 200:
            return 0.0

        valid_angles = angle[mask]

        # 梯度角 -> 边缘角：edge = grad - 90，并映射到 [-90, 90]
        edge_angles = valid_angles - 90.0
        edge_angles = np.mod(edge_angles + 90, 180) - 90

        # 假设偏航不会太离谱
        edge_angles = edge_angles[np.abs(edge_angles) < 30]
        if len(edge_angles) < 50:
            return 0.0

        hist, bins = np.histogram(edge_angles, bins=180, range=(-90, 90))
        peak_idx = int(np.argmax(hist))
        dominant_angle = (bins[peak_idx] + bins[peak_idx + 1]) / 2.0
        return float(dominant_angle)

    # ------------------------- 旋转工具（同时支持 mask） -------------------------
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

    # ------------------------- 关键：在 Step2 带内求横向中心 -------------------------
    def compute_step_center_x(self, analyze_img, analyze_mask, y_top, y_bottom, cam_x_straight):
        """
        在“拉直坐标系”里，用特征 + 500mm 宽度窗口定位 Step2 的横向中心。
        返回：cx, (x_left, x_right), method_name
        """
        h, w = analyze_img.shape
        y0 = int(max(0, min(y_top, y_bottom)))
        y1 = int(min(h, max(y_top, y_bottom)))
        if y1 - y0 < 8:
            return cam_x_straight, (cam_x_straight - self.step_width_px / 2, cam_x_straight + self.step_width_px / 2), "fallback_short_band"

        band = analyze_img[y0:y1, :]
        band_m = analyze_mask[y0:y1, :].astype(bool) if analyze_mask is not None else np.ones_like(band, dtype=bool)

        # ---- A) 先用“竖直边缘能量”找左右边界候选（更像“特征”）----
        #   用 Sobel x（竖边）强度 -> 每列平均强度
        gx = cv2.Sobel(band, cv2.CV_32F, 1, 0, ksize=3)
        gx = np.abs(gx)
        gx[~band_m] = 0
        edge_col = gx.mean(axis=0)  # shape (w,)

        # 平滑一下
        edge_col_s = cv2.GaussianBlur(edge_col.reshape(1, -1), (1, 31), 0).flatten()

        # 取较强边缘列
        thr = max(3.0, float(np.percentile(edge_col_s, 95) * 0.45))
        strong = np.where(edge_col_s > thr)[0]

        # 如果边缘列足够多，取 5% 和 95% 分位做左右边界
        if len(strong) > 30:
            xl = float(np.percentile(strong, 5))
            xr = float(np.percentile(strong, 95))
            if xr - xl > max(0.35 * self.step_width_px, 30):
                cx = 0.5 * (xl + xr)
                return cx, (xl, xr), "edge_energy"

        # ---- B) 边缘不稳定：用“500mm 宽度窗口”做鲁棒定位（占用度+靠近相机中心惩罚）----
        # presence：band 内“有效且非黑”的像素占比（用阈值很低，避免蓝色太暗被丢）
        presence = ((band > 2) & band_m).astype(np.float32)
        col_sum = presence.sum(axis=0)  # 每列有效像素数

        win = int(max(20, min(w, round(self.step_width_px))))
        if win >= w:
            # 宽度窗口比图还宽，直接用有效区域的左右边界
            cols = np.where(col_sum > 0)[0]
            if len(cols) > 0:
                xl, xr = float(cols.min()), float(cols.max())
                cx = 0.5 * (xl + xr)
                return cx, (xl, xr), "presence_full"
            return cam_x_straight, (cam_x_straight - self.step_width_px / 2, cam_x_straight + self.step_width_px / 2), "fallback_no_presence"

        # 滑动窗口得分：窗口内 col_sum 求和
        kernel = np.ones(win, dtype=np.float32)
        score = np.convolve(col_sum, kernel, mode="valid")  # len = w-win+1

        # 加一个“离 cam_x_straight 越远越惩罚”的项，避免 pick 到边缘噪声窗口
        centers = np.arange(len(score), dtype=np.float32) + win * 0.5
        penalty = np.abs(centers - float(cam_x_straight))
        lam = float(np.max(score) + 1e-6) * 0.02 / max(1.0, win)  # 轻微惩罚
        score2 = score - lam * penalty

        best = int(np.argmax(score2))
        xl = float(best)
        xr = float(best + win)
        cx = 0.5 * (xl + xr)
        return cx, (xl, xr), "presence_window"

    # ------------------------- 主检测流程 -------------------------
    def analyze_profile_and_detect(self, bev_gray, bev_mask):
        h, w = bev_gray.shape
        debug_img = cv2.cvtColor(bev_gray, cv2.COLOR_GRAY2BGR)

        # 1) 估计偏航角
        detected_angle = self.estimate_dominant_angle(bev_gray, bev_mask)
        print(f"检测到的旋转偏角: {detected_angle:.2f} 度")

        # 2) 拉直图像（分析用）
        analyze_img, M = self.rotate_image(bev_gray, detected_angle, interp=cv2.INTER_LINEAR)
        analyze_mask, _ = self.rotate_image((bev_mask * 255).astype(np.uint8), detected_angle, interp=cv2.INTER_NEAREST)
        analyze_mask = (analyze_mask > 0).astype(np.uint8)

        # 逆变换矩阵：把“拉直坐标系”点映射回原 BEV（用于绘制）
        M_inv = cv2.invertAffineTransform(M)

        def to_original(pt_straight):
            x, y = pt_straight
            ox, oy = self._apply_affine(M_inv, x, y)
            return int(round(ox)), int(round(oy))

        # 相机点在“拉直坐标系”里的位置（横向偏移必须用这个）
        cam_x_s, cam_y_s = self._apply_affine(M, self.cam_x_pix, self.cam_y_pix)

        # 3) 垂直 profile（在拉直图上做）
        roi_w = int(w * 0.45)
        roi_x = int((w - roi_w) / 2)
        roi = analyze_img[:, roi_x:roi_x + roi_w]
        roi_m = analyze_mask[:, roi_x:roi_x + roi_w].astype(bool)

        # 用 mask 做加权均值，避免无效黑区影响
        roi_f = roi.astype(np.float32)
        m_f = roi_m.astype(np.float32)
        denom = np.maximum(1.0, m_f.sum(axis=1))
        v_profile = (roi_f * m_f).sum(axis=1) / denom  # (h,)

        grad = np.gradient(v_profile)
        grad_smooth = cv2.GaussianBlur(grad.reshape(-1, 1).astype(np.float32), (5, 1), 0).flatten()

        # 动态阈值：让不同曝光下更稳
        g_abs = np.abs(grad_smooth)
        thr = max(8.0, float(np.percentile(g_abs, 98) * 0.35))

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
                edge_type = "BRIGHT_TO_DARK"   # 远端
            elif g_val > thr:
                edge_type = "DARK_TO_BRIGHT"   # 近端
            detected_lines.append((int(idx), edge_type, g_val))

        detected_lines.sort(key=lambda x: x[0])

        # 4) 匹配 Step2（长度约 500mm）
        candidates_step2 = []
        expected_len_px = (0.5 / self.m_per_pixel)

        for i in range(len(detected_lines) - 1):
            y_top, _, g_top = detected_lines[i]
            y_btm, _, g_btm = detected_lines[i + 1]
            if g_top < -thr and g_btm > thr:
                dist_px = y_btm - y_top
                if abs(dist_px - expected_len_px) < expected_len_px * 0.55:
                    candidates_step2.append((y_top, y_btm))

        result_info = {"yaw_angle_deg": float(detected_angle)}

        # 5) 画 Step2：计算横向中心 + 横向偏移 + 画蓝框/中心线
        if candidates_step2:
            # 取最靠近相机的候选（y 更大的一组通常更近；这里取最后一个）
            y_step2_top, y_step2_bottom = candidates_step2[-1]  # top(远端小), bottom(近端大)

            # 5.1 计算 Step2 横向中心（在拉直坐标系）
            cx, (xl, xr), cx_method = self.compute_step_center_x(
                analyze_img, analyze_mask, y_step2_top, y_step2_bottom, cam_x_s
            )

            # 5.2 横向偏移：以“相机在拉直坐标系的 x”为基准
            offset_px = cx - cam_x_s
            offset_m = offset_px * self.m_per_pixel
            result_info["step2_center_x_pix_straight"] = float(cx)
            result_info["step2_offset_m"] = float(offset_m)
            result_info["step2_offset_mm"] = float(offset_m * 1000.0)
            result_info["step2_center_method"] = cx_method

            # 5.3 蓝色框：用已知 500mm 宽度（step_width_px）在拉直坐标系构建矩形，再映射回原图
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

            # 5.4 画 Step2 中心线（绿色）
            line_p1 = to_original((cx, float(y_step2_top)))
            line_p2 = to_original((cx, float(y_step2_bottom)))
            cv2.line(debug_img, line_p1, line_p2, (0, 255, 0), 2)

            # 5.5 画相机中心线（黄）
            camline_p1 = to_original((cam_x_s, float(y_step2_top)))
            camline_p2 = to_original((cam_x_s, float(y_step2_bottom)))
            cv2.line(debug_img, camline_p1, camline_p2, (0, 255, 255), 2)

            # 5.6 距离（仍用拉直坐标系的 y，更接近物理）
            dist_step2_front_m = (cam_y_s - float(y_step2_bottom)) * self.m_per_pixel
            result_info["step2_dist"] = float(dist_step2_front_m)

            # 标注文本
            label_pos = pts_orig[3]  # BL
            cv2.putText(
                debug_img,
                f"S2: {dist_step2_front_m*1000:.0f}mm",
                (label_pos[0] - 20, label_pos[1] + 25),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.6,
                (0, 255, 255),
                2
            )
            cv2.putText(
                debug_img,
                f"OffsetX: {offset_m*1000:+.0f}mm ({cx_method})",
                (label_pos[0] - 20, label_pos[1] + 50),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.6,
                (0, 255, 0),
                2
            )

            # 在图上画出用于定位的 (xl,xr)（可选，便于调试）
            # 画在 step2 的中间 y
            midy = 0.5 * (y_step2_top + y_step2_bottom)
            xl_p = to_original((xl, midy))
            xr_p = to_original((xr, midy))
            cv2.circle(debug_img, xl_p, 3, (255, 255, 0), -1)
            cv2.circle(debug_img, xr_p, 3, (255, 255, 0), -1)

        # 6) Step1 起始线（保持你原来的逻辑：在 Step2 之后找强的 BRIGHT->DARK）
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

            dist_s1 = (cam_y_s - float(best_s1_y)) * self.m_per_pixel
            result_info["step1_dist"] = float(dist_s1)
            cv2.putText(
                debug_img,
                f"S1 Start: {dist_s1*1000:.0f}mm",
                (pt_start_orig[0] + 20, pt_start_orig[1] - 10),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.6,
                (0, 0, 255),
                2
            )

        # 7) 显示 yaw
        cv2.putText(
            debug_img,
            f"Yaw: {detected_angle:.1f} deg",
            (20, 40),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.8,
            (0, 255, 0),
            2
        )

        return debug_img, result_info

    def run(self, img_path):
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
            print(f" -> 蓝台阶(第二级) 横向偏移(相对相机中心): {info['step2_offset_mm']:+.1f} mm  ({info.get('step2_center_method','')})")
        if "step1_dist" in info:
            print(f" -> 白台阶(第一级) 起始垂直距离: {info['step1_dist']*1000:.1f} mm")

        out_path = os.path.join(self.out_dir, f"result_{os.path.basename(img_path)}")
        cv2.imwrite(out_path, result_img)


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
