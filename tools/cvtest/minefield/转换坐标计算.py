import numpy as np
import math

# --- 1. 相机和场景参数 ---
X_c, Y_c, Z_c = 0.0, 42.0, 120.0  # 相机位置
P, Y, R = -30.0, 0.0, 0.0          # 相机欧拉角 (Pitch, Yaw, Roll)
FOV_deg = 45.0                     # 垂直视场角

W = 752                            # 图像宽度
H = 480                            # 图像高度

P_w = np.array([0.0, 0.0, 0.0])    # 世界原点

# 转换为弧度
P_rad = np.deg2rad(P)
Y_rad = np.deg2rad(Y)
R_rad = np.deg2rad(R)

# --- 2. 构造 World -> Camera 旋转矩阵 R_cw ---
# OpenGL/航空航天约定：Rx(Pitch), Ry(Yaw), Rz(Roll)
# R_cw = Rz(-R) * Rx(-P) * Ry(-Y)
# 由于 Y=R=0, R_cw 简化为 Rx(-P) = Rx(30 deg)

theta_x = -P_rad # 30 degrees positive rotation (looking up transforms world down)

R_cw = np.array([
    [1, 0, 0],
    [0, np.cos(theta_x), -np.sin(theta_x)],
    [0, np.sin(theta_x), np.cos(theta_x)]
])

# --- 3. 计算 Camera 坐标 P_c ---
# P_c = R_cw * (P_w - T_c)
T_c = np.array([X_c, Y_c, Z_c])
P_c = R_cw @ (P_w - T_c)

x_c, y_c, z_c = P_c

print(f"P_c (Camera Coordinates): ({x_c:.3f}, {y_c:.3f}, {z_c:.3f})")

# --- 4. 计算 NDC (Normalized Device Coordinates) ---
fov_half_rad = np.deg2rad(FOV_deg / 2)
f = 1.0 / np.tan(fov_half_rad) # f = cot(FOV/2)
aspect = W / H

# NDC mapping (assuming projection matrix structure)
x_ndc = (x_c / (-z_c)) * (f / aspect)
y_ndc = (y_c / (-z_c)) * f

print(f"NDC Coordinates: ({x_ndc:.4f}, {y_ndc:.4f})")

# --- 5. 计算屏幕像素坐标 (x', y') ---
# X' mapping: [-1, 1] -> [0, W]
# Y' mapping: [1, -1] -> [0, H] (Top-Left origin standard)

x_prime = (x_ndc + 1) * (W / 2)
y_prime = (1 - y_ndc) * (H / 2)

print("\n--- 最终结果 ---")
print(f"x' = {x_prime:.2f}")
print(f"y' = {y_prime:.2f}")