# ipm_make_table.py
import os
import json
import numpy as np
import cv2

def build_ipm_maps_from_homography(H_img_from_bev: np.ndarray, bev_w: int, bev_h: int):
    """
    生成 remap 用的 ipm_map_x / ipm_map_y
    H_img_from_bev: 3x3，把 BEV 平面点(像素) -> 相机图像点(像素)
    """
    # BEV 像素网格
    xs, ys = np.meshgrid(np.arange(bev_w, dtype=np.float32),
                         np.arange(bev_h, dtype=np.float32))
    ones = np.ones_like(xs, dtype=np.float32)

    bev_pts = np.stack([xs, ys, ones], axis=-1).reshape(-1, 3).T  # (3, N)
    img_pts = (H_img_from_bev @ bev_pts)  # (3, N)
    img_pts[:2] /= (img_pts[2:3] + 1e-9)

    map_x = img_pts[0].reshape(bev_h, bev_w).astype(np.float32)
    map_y = img_pts[1].reshape(bev_h, bev_w).astype(np.float32)
    return map_x, map_y

def compute_homography_from_4pts(img_pts4, bev_pts4):
    """
    img_pts4: 4x2 相机图像点
    bev_pts4: 4x2 BEV 平面点(像素坐标)
    返回 H_img_from_bev: BEV -> IMG
    """
    img_pts4 = np.asarray(img_pts4, dtype=np.float32)
    bev_pts4 = np.asarray(bev_pts4, dtype=np.float32)

    # OpenCV findHomography 默认是 src->dst，所以我们想要 BEV->IMG：
    H, _ = cv2.findHomography(bev_pts4, img_pts4, method=0)
    if H is None:
        raise RuntimeError("findHomography 失败：检查输入的 4 对点是否正确且不共线。")
    return H

def main():
    """
    你可以改成从 json 读配置，或直接在下面填。
    """
    # ====== 输出分辨率（BEV）======
    bev_w = 376
    bev_h = 240

    # ====== 方式1：填 4 对点（推荐你先用这个快速出结果）======
    # 相机图像点：顺序要对应（比如：左下、右下、右上、左上）
    img_pts4 = [
        [  70, 396],  # TODO: 改成你的点
        [ 680, 396],
        [ 534, 160],
        [  216, 160],
    ]

    # BEV 平面点（像素）：同样顺序（左下、右下、右上、左上）
    bev_pts4 = [
        [  60, bev_h-1],
        [ bev_w-60, bev_h-1],
        [ bev_w-60,  20],
        [  60,  20],
    ]

    H_img_from_bev = compute_homography_from_4pts(img_pts4, bev_pts4)

    # ====== 方式2：如果你已有 H，就直接用（把上面 compute_homography 注释掉）======
    # H_img_from_bev = np.array([[...],[...],[...]], dtype=np.float64)

    map_x, map_y = build_ipm_maps_from_homography(H_img_from_bev, bev_w, bev_h)

    os.makedirs("ipm_tables", exist_ok=True)
    np.savetxt("ipm_tables/ipm_map_x.txt", map_x, fmt="%.6f")
    np.savetxt("ipm_tables/ipm_map_y.txt", map_y, fmt="%.6f")

    # 同时存一个 json，方便识别脚本读取 BEV 尺寸和 m_per_pixel（你可以按标定算更准）
    # 这里给一个“占位”，你后面用真实测量值替换。
    cfg = {
        "bev_w": bev_w,
        "bev_h": bev_h,
        "m_per_pixel": 0.164,   # TODO: 用你的标定真实值替换
        "camera_ref_bev": [bev_w/2.0, bev_h + 30.0],  # “相机投影到BEV的参考点”，可按你现有逻辑改
        "map_x_path": "ipm_tables/ipm_map_x.txt",
        "map_y_path": "ipm_tables/ipm_map_y.txt",
    }
    with open("ipm_tables/ipm_config.json", "w", encoding="utf-8") as f:
        json.dump(cfg, f, ensure_ascii=False, indent=2)

    print("[OK] Saved:")
    print(" - ipm_tables/ipm_map_x.txt")
    print(" - ipm_tables/ipm_map_y.txt")
    print(" - ipm_tables/ipm_config.json")
    print("[H] Homography (BEV->IMG):\n", H_img_from_bev)

if __name__ == "__main__":
    main()
