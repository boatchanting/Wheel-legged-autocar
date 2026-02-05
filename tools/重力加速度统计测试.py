import json
import numpy as np
import matplotlib.pyplot as plt
import os

# --- 0. 离群点移除函数 ---
def remove_outliers_by_std(data, threshold=3.0):
    """
    使用3σ准则 (Standard Deviation Method) 移除离群点。
    
    参数:
    data (np.array): 输入的数据数组。
    threshold (float): 标准差的倍数，超过这个范围的点被视为离群点。
    
    返回:
    np.array: 移除了离群点的数据数组。
    """
    mean = np.mean(data)
    std_dev = np.std(data)
    
    # 防止标准差为0（所有数据点都相同）
    if std_dev == 0:
        return data
        
    # 计算每个数据点的Z-score (标准分数)
    z_scores = np.abs((data - mean) / std_dev)
    
    # 创建一个布尔掩码，只保留Z-score小于阈值的点
    filtered_mask = z_scores < threshold
    
    # 返回过滤后的数据
    return data[filtered_mask]

# --- 1. 设置文件路径和参数 ---

file_path = r"\Data\地上平衡状态加速度消除重力测试.txt"
target_channels = {
    2: "X-axis",
    3: "Y-axis",
    4: "Z-axis"
}
# 设置离群点判断的阈值（单位：标准差）。3是一个常用的值。
OUTLIER_THRESHOLD = 3.0

# 用于存储数据的字典
original_accel_data = {}
cleaned_accel_data = {}

# --- 2. 读取并解析数据文件 ---

if not os.path.exists(file_path):
    print(f"错误：文件未找到，请检查路径是否正确: {file_path}")
else:
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            data = json.load(f)

        for item in data:
            channel_num = item.get("Channel")
            if channel_num in target_channels:
                original_accel_data[channel_num] = np.array(item.get("Points", []))
        
        if len(original_accel_data) != len(target_channels):
            print("警告：并非所有目标通道 (2, 3, 4) 都在文件中找到。")

    except json.JSONDecodeError:
        print(f"错误：文件 '{file_path}' 不是有效的JSON格式。")
    except Exception as e:
        print(f"处理文件时发生未知错误: {e}")

# --- 3. 数据清洗：移除离群点 ---

if original_accel_data:
    print(f"\n--- 数据清洗 (使用 {OUTLIER_THRESHOLD}σ 准则移除离群点) ---")
    for channel_num, points in original_accel_data.items():
        # 调用函数移除离群点
        cleaned_points = remove_outliers_by_std(points, threshold=OUTLIER_THRESHOLD)
        cleaned_accel_data[channel_num] = cleaned_points
        
        original_count = len(points)
        cleaned_count = len(cleaned_points)
        removed_count = original_count - cleaned_count
        
        print(f"通道 {channel_num} ({target_channels[channel_num]}):")
        print(f"  原始数据点数量: {original_count}")
        print(f"  移除离群点数量: {removed_count}")
        print(f"  清洗后数据点数量: {cleaned_count}")


# --- 4. 基于清洗后数据的统计分析 ---

if cleaned_accel_data:
    print("\n--- 基于清洗后数据的统计分析 ---")
    for channel_num, points in cleaned_accel_data.items():
        axis_name = target_channels[channel_num]
        
        # 计算统计量
        mean_val = np.mean(points)
        std_dev = np.std(points)
        max_val = np.max(points)
        min_val = np.min(points)
        variance = np.var(points)
        
        print(f"\n通道 {channel_num} ({axis_name}):")
        print(f"  平均值 (Mean):   {mean_val:.6f}")
        print(f"  标准差 (Std Dev): {std_dev:.6f}")
        print(f"  方差 (Variance):  {variance:.6f}")
        print(f"  最大值 (Max):     {max_val:.6f}")
        print(f"  最小值 (Min):     {min_val:.6f}")

# --- 5. 绘图 (对比原始数据和清洗后数据) ---

if original_accel_data and cleaned_accel_data:
    plt.rcParams['font.sans-serif'] = ['SimHei']
    plt.rcParams['axes.unicode_minus'] = False

    # 创建一个图形，包含三个子图，每个通道一个
    fig, axes = plt.subplots(len(target_channels), 1, figsize=(15, 12), sharex=True)
    fig.suptitle('静止台架加速度数据分析 (含离群点移除)', fontsize=18)

    channel_list = sorted(original_accel_data.keys())

    for i, channel_num in enumerate(channel_list):
        ax = axes[i]
        axis_name = target_channels[channel_num]
        original_points = original_accel_data[channel_num]
        cleaned_points = cleaned_accel_data[channel_num]
        
        # 原始数据的所有点作为X轴
        x_axis_original = np.arange(len(original_points))

        # 绘制原始数据点 (半透明散点图)
        ax.scatter(x_axis_original, original_points, color='gray', alpha=0.3, s=10, label='原始数据点 (含离群点)')
        
        # 找到清洗后数据点在原始数据中的索引，以便正确绘制曲线
        # 注意：这里我们简单地绘制清洗后的数据序列，这在视觉上足够了
        x_axis_cleaned = np.arange(len(cleaned_points))
        # 为了更直观，我们仍然在原始的X轴上绘制，但只绘制保留下来的点
        # 这是一个简化的方法：直接绘制清洗后的序列
        mean_val = np.mean(cleaned_points)
        ax.plot(x_axis_original, original_points, linestyle='-', color='skyblue', alpha=0.7, label=f'清洗后数据趋势')
        ax.axhline(y=mean_val, color='red', linestyle='--', label=f'清洗后均值: {mean_val:.4f}')

        ax.set_title(f'通道 {channel_num} ({axis_name})', fontsize=14)
        ax.set_ylabel('加速度值')
        ax.legend()
        ax.grid(True, linestyle='--', alpha=0.6)

    axes[-1].set_xlabel('采样点 (Sample Index)', fontsize=12)
    plt.tight_layout(rect=[0, 0, 1, 0.96]) # 调整布局以适应主标题
    plt.show()