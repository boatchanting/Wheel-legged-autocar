import cv2
import numpy as np
import json
import os

def build_ipm_map_auto_fit(
    calib_img_path,
    resize_scale=0.5,
    orig_bl=(16, 234),
    orig_tl=(201, 62),
    orig_tr=(550, 62),
    orig_br=(735, 234),
    square_size_m=1.0,
    ipm_out_size=(376, 240),
    save_prefix="ipm",
    save_json=True,
    json_path=None
):
    """
    自动适配版 IPM：
    1) 保证原图可见地面尽量不丢失（通过 bbox + fit）
    2) 自动缩放+平移+居中，让内容落入输出图
    3) 额外保存 camera_ref_bev：把“原图底部中心点”映射到 BEV，当作相机参考点
    """

    # ========== 1) 读图 ==========
    calib_img = cv2.imread(calib_img_path, cv2.IMREAD_GRAYSCALE)
    if calib_img is None:
        raise FileNotFoundError(f"[ERROR] 无法读取标定图片: {calib_img_path}")
    orig_h, orig_w = calib_img.shape[:2]

    # ========== 2) 压缩处理 ==========
    new_w = int(round(orig_w * resize_scale))
    new_h = int(round(orig_h * resize_scale))
    calib_small = cv2.resize(calib_img, (new_w, new_h))

    # ========== 3) 标定点缩放到小图坐标 ==========
    def scale_pt(pt, s):
        return (pt[0] * s, pt[1] * s)

    p_bl = scale_pt(orig_bl, resize_scale)
    p_tl = scale_pt(orig_tl, resize_scale)
    p_tr = scale_pt(orig_tr, resize_scale)
    p_br = scale_pt(orig_br, resize_scale)

    src_pts = np.array([p_bl, p_br, p_tr, p_tl], dtype=np.float32)

    # ========== 4) Image -> Metric (米) ==========
    dst_metric = np.array([
        [0.0,          square_size_m],   # 左下
        [square_size_m, square_size_m],   # 右下
        [square_size_m, 0.0],            # 右上
        [0.0,          0.0],             # 左上
    ], dtype=np.float32)

    H_metric = cv2.getPerspectiveTransform(src_pts, dst_metric)

    # ========== 5) 原图四角投影到 Metric bbox ==========
    img_corners = np.array([
        [0,     0],
        [new_w, 0],
        [new_w, new_h],
        [0,     new_h]
    ], dtype=np.float32).reshape(-1, 1, 2)

    metric_corners = cv2.perspectiveTransform(img_corners, H_metric)

    x_coords = metric_corners[:, 0, 0]
    y_coords = metric_corners[:, 0, 1]

    min_x, max_x = float(np.min(x_coords)), float(np.max(x_coords))
    min_y, max_y = float(np.min(y_coords)), float(np.max(y_coords))

    scene_width_m = max_x - min_x
    scene_height_m = max_y - min_y

    print(f"[Info] 原图视野覆盖范围(小图尺度): 宽 {scene_width_m:.3f}m, 高 {scene_height_m:.3f}m")

    # ========== 6) 缩放+平移+居中 -> 输出 BEV ==========
    out_w, out_h = ipm_out_size

    scale_x = out_w / max(scene_width_m, 1e-9)
    scale_y = out_h / max(scene_height_m, 1e-9)
    final_scale = min(scale_x, scale_y)  # pixels per meter（统一尺度）

    # T: 把(min_x,min_y)移到(0,0)
    T = np.array([
        [1, 0, -min_x],
        [0, 1, -min_y],
        [0, 0, 1]
    ], dtype=np.float64)

    # S: 米 -> 像素
    S = np.array([
        [final_scale, 0, 0],
        [0, final_scale, 0],
        [0, 0, 1]
    ], dtype=np.float64)

    fitted_w = scene_width_m * final_scale
    fitted_h = scene_height_m * final_scale
    offset_x = (out_w - fitted_w) / 2.0
    offset_y = (out_h - fitted_h) / 2.0

    T_center = np.array([
        [1, 0, offset_x],
        [0, 1, offset_y],
        [0, 0, 1]
    ], dtype=np.float64)

    H_final = (T_center @ S @ T @ H_metric).astype(np.float64)

    # ========== 7) 生成 remap 表 ==========
    H_inv = np.linalg.inv(H_final)

    xx, yy = np.meshgrid(np.arange(out_w), np.arange(out_h))
    ones = np.ones_like(xx)
    ipm_coords = np.stack([xx, yy, ones], axis=-1).reshape(-1, 3).T

    img_coords = H_inv @ ipm_coords
    img_coords /= (img_coords[2, :] + 1e-8)

    map_x = img_coords[0, :].reshape(out_h, out_w).astype(np.float32)
    map_y = img_coords[1, :].reshape(out_h, out_w).astype(np.float32)

    # ========== 8) remap + debug ==========
    ipm_img = cv2.remap(
        calib_small, map_x, map_y,
        cv2.INTER_LINEAR,
        borderMode=cv2.BORDER_CONSTANT,
        borderValue=0
    )

    calib_pts_transformed = cv2.perspectiveTransform(src_pts.reshape(-1, 1, 2), H_final)
    debug_img = ipm_img.copy()
    if len(debug_img.shape) == 2:
        debug_img = cv2.cvtColor(debug_img, cv2.COLOR_GRAY2BGR)
    cv2.polylines(debug_img, [calib_pts_transformed.astype(np.int32)], True, (0, 0, 255), 2)

    # ======= 关键：计算并画出 camera_ref_bev（小图底部中心） =======
    cam_ref_src = np.array([[[new_w / 2.0, new_h - 1.0]]], dtype=np.float32)  # 小图底部中心点
    cam_ref_bev = cv2.perspectiveTransform(cam_ref_src, H_final)[0, 0]
    cam_ref_bev = (float(cam_ref_bev[0]), float(cam_ref_bev[1]))
    cv2.circle(debug_img, (int(round(cam_ref_bev[0])), int(round(cam_ref_bev[1]))), 6, (255, 0, 0), -1)
    cv2.putText(debug_img, "CAM_REF", (int(cam_ref_bev[0]) + 8, int(cam_ref_bev[1]) - 8),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 2)

    # 保存文件
    ipm_img_path = f"{save_prefix}_calib.png"
    ipm_dbg_path = f"{save_prefix}_calib_debug.png"
    map_x_path = f"{save_prefix}_map_x.txt"
    map_y_path = f"{save_prefix}_map_y.txt"

    cv2.imwrite(ipm_img_path, ipm_img)
    cv2.imwrite(ipm_dbg_path, debug_img)
    np.savetxt(map_x_path, map_x, fmt="%.6f")
    np.savetxt(map_y_path, map_y, fmt="%.6f")

    m_per_pixel = 1.0 / float(final_scale)

    print("========== 自动适配完成 ==========")
    print(f"[OK] m_per_pixel: {m_per_pixel:.6f} m/pixel")
    print(f"[OK] pixels_per_meter: {final_scale:.3f}")
    print(f"[OK] camera_ref_bev (px): ({cam_ref_bev[0]:.2f}, {cam_ref_bev[1]:.2f})")
    print(f"[OK] Saved: {ipm_img_path}, {ipm_dbg_path}, {map_x_path}, {map_y_path}")

    # ========== 9) 保存 JSON（给检测用） ==========
    if save_json:
        if json_path is None:
            json_path = f"{save_prefix}_calib_result.json"

        cfg = {
            "resize_scale": float(resize_scale),
            "m_per_pixel": float(m_per_pixel),
            "pixels_per_meter": float(final_scale),
            "ipm_out_size": [int(out_w), int(out_h)],
            "camera_ref_bev": [float(cam_ref_bev[0]), float(cam_ref_bev[1])],
            "square_size_m": float(square_size_m),
            "output_files": {
                "map_x": map_x_path,
                "map_y": map_y_path,
                "ipm_img": ipm_img_path,
                "ipm_debug": ipm_dbg_path
            },
            "calib_points_orig": {
                "orig_bl": list(orig_bl),
                "orig_tl": list(orig_tl),
                "orig_tr": list(orig_tr),
                "orig_br": list(orig_br)
            }
        }
        with open(json_path, "w", encoding="utf-8") as f:
            json.dump(cfg, f, ensure_ascii=False, indent=2)
        print(f"[OK] 标定配置已保存: {json_path}")

    return H_final, map_x, map_y, m_per_pixel, cam_ref_bev


if __name__ == "__main__":
    calib_img_path = r"img_gray_X0_Y42_Z120_P-30_Y0_R0.png"

    build_ipm_map_auto_fit(
        calib_img_path=calib_img_path,
        resize_scale=0.5,
        orig_bl=(16, 234),
        orig_tl=(201, 62),
        orig_tr=(550, 62),
        orig_br=(735, 234),
        square_size_m=1.0,
        ipm_out_size=(376, 240),
        save_prefix="ipm",
        save_json=True,
        json_path="ipm_calib_result.json"
    )
