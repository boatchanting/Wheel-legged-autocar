import json
import matplotlib.pyplot as plt
import os

# 设置中文字体支持（可选）
plt.rcParams['font.sans-serif'] = ['SimHei', 'Arial Unicode MS', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False  # 正常显示负号

def read_and_plot_trajectory(file_path):
    """
    读取包含多通道数据的JSON文件，提取Channel 6 (X) 和 Channel 7 (Y)，并绘制轨迹图。
    
    参数:
        file_path (str): 文件路径
    """
    if not os.path.exists(file_path):
        print(f"错误：文件未找到 -> {file_path}")
        return

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except Exception as e:
        print(f"读取或解析JSON失败：{e}")
        return

    # 初始化变量
    x_points = None
    y_points = None

    # 遍历所有通道
    for item in data:
        if not isinstance(item, dict) or "Channel" not in item or "Points" not in item:
            continue

        channel = item["Channel"]
        points = item["Points"]

        if channel == 6:
            x_points = points
        elif channel == 7:
            y_points = points

    # 检查是否都存在
    if x_points is None:
        print("警告：未找到 Channel 6 (X方向) 数据。")
    if y_points is None:
        print("警告：未找到 Channel 7 (Y方向) 数据。")
    if x_points is None or y_points is None:
        return

    # 截断到较短长度以确保匹配
    min_len = min(len(x_points), len(y_points))
    x_points = x_points[:min_len]
    y_points = y_points[:min_len]

    # 绘图
    plt.figure(figsize=(10, 6))
    plt.plot(x_points, y_points, marker='o', markersize=4, linewidth=1.5, label="小车运动轨迹")
    plt.title("小车运动轨迹图 (Channel 6: X方向, Channel 7: Y方向)", fontsize=14)
    plt.xlabel("X (Channel 6) [单位：未知]", fontsize=12)
    plt.ylabel("Y (Channel 7) [单位：未知]", fontsize=12)
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.axis('equal')  # 保持x/y轴比例一致，真实反映轨迹
    plt.legend()

    plt.tight_layout()
    plt.show()

# ========================
#       主程序入口
# ========================

if __name__ == "__main__":
    file_path = r"E:\github_projects\autocar1\environment4BB7\Seekfree_CYT4BB_Opensource_Library\Data\惯性导航测试数据50ms小车跑三次.txt"
    read_and_plot_trajectory(file_path)