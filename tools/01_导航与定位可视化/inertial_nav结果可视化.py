import json
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import FancyArrowPatch
import matplotlib.patches as patches

def read_trajectory_data(file_path):
    """
    读取轨迹数据文件
    
    Args:
        file_path: 数据文件路径
    
    Returns:
        tuple: (yaw_angles, x_positions, y_positions)
    """
    with open(file_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    
    # 提取各个通道的数据
    yaw_data = None
    x_data = None
    y_data = None
    
    for channel_info in data:
        if channel_info['Channel'] == 5:  # 朝向角
            yaw_data = channel_info['Points']
        elif channel_info['Channel'] == 6:  # x方向位置
            x_data = channel_info['Points']
        elif channel_info['Channel'] == 7:  # y方向位置
            y_data = channel_info['Points']
    
    if yaw_data is None or x_data is None or y_data is None:
        raise ValueError("数据文件中缺少必要的通道信息（通道5、6、7）")
    
    # 确保所有通道数据长度一致
    min_length = min(len(yaw_data), len(x_data), len(y_data))
    yaw_angles = np.array(yaw_data[:min_length])
    x_positions = np.array(x_data[:min_length])
    y_positions = np.array(y_data[:min_length])
    
    return yaw_angles, x_positions, y_positions

def visualize_trajectory(yaw_angles, x_positions, y_positions, sampling_interval=50):
    """
    可视化轨迹数据
    
    Args:
        yaw_angles: 朝向角数组
        x_positions: x方向位置数组
        y_positions: y方向位置数组
        sampling_interval: 采样间隔(ms)
    """
    fig, axes = plt.subplots(2, 2, figsize=(15, 12))
    fig.suptitle('惯性导航轨迹可视化分析', fontsize=16, fontweight='bold')
    
    # 计算时间轴
    time_axis = np.arange(len(x_positions)) * sampling_interval / 1000.0  # 转换为秒
    
    # 1. 轨迹路径图
    ax1 = axes[0, 0]
    ax1.plot(x_positions, y_positions, 'b-', linewidth=2, label='轨迹路径', alpha=0.7)
    ax1.scatter(x_positions[0], y_positions[0], color='green', s=100, label='起点', zorder=5)
    ax1.scatter(x_positions[-1], y_positions[-1], color='red', s=100, label='终点', zorder=5)
    
    # 添加方向箭头（每隔一定点数显示一次方向）
    step = max(1, len(x_positions) // 20)  # 最多显示20个方向箭头
    for i in range(0, len(x_positions), step):
        dx = np.cos(np.radians(yaw_angles[i]))
        dy = np.sin(np.radians(yaw_angles[i]))
        ax1.arrow(x_positions[i], y_positions[i], 
                 dx*0.1*(max(max(x_positions)-min(x_positions), max(y_positions)-min(y_positions))), 
                 dy*0.1*(max(max(x_positions)-min(x_positions), max(y_positions)-min(y_positions))),
                 head_width=0.05, head_length=0.1, fc='red', ec='red', alpha=0.6)
    
    ax1.set_xlabel('X Position (m)')
    ax1.set_ylabel('Y Position (m)')
    ax1.set_title('轨迹路径图')
    ax1.grid(True, alpha=0.3)
    ax1.legend()
    ax1.axis('equal')
    
    # 2. X位置随时间变化
    ax2 = axes[0, 1]
    ax2.plot(time_axis, x_positions, 'g-', linewidth=2)
    ax2.set_xlabel('Time (s)')
    ax2.set_ylabel('X Position (m)')
    ax2.set_title('X方向位置变化')
    ax2.grid(True, alpha=0.3)
    
    # 3. Y位置随时间变化
    ax3 = axes[1, 0]
    ax3.plot(time_axis, y_positions, 'r-', linewidth=2)
    ax3.set_xlabel('Time (s)')
    ax3.set_ylabel('Y Position (m)')
    ax3.set_title('Y方向位置变化')
    ax3.grid(True, alpha=0.3)
    
    # 4. 朝向角随时间变化
    ax4 = axes[1, 1]
    ax4.plot(time_axis, yaw_angles, 'purple', linewidth=2)
    ax4.set_xlabel('Time (s)')
    ax4.set_ylabel('Yaw Angle (degrees)')
    ax4.set_title('小车朝向角变化 (-180° to 180°)')
    ax4.grid(True, alpha=0.3)
    ax4.set_ylim(-180, 180)
    
    plt.tight_layout()
    plt.show()
    
    # 打印统计信息
    print("="*50)
    print("轨迹数据分析报告")
    print("="*50)
    print(f"总数据点数: {len(x_positions)}")
    print(f"总时长: {time_axis[-1]:.2f} 秒")
    print(f"X方向范围: [{x_positions.min():.3f}, {x_positions.max():.3f}] m")
    print(f"Y方向范围: [{y_positions.min():.3f}, {y_positions.max():.3f}] m")
    print(f"朝向角范围: [{yaw_angles.min():.2f}°, {yaw_angles.max():.2f}°]")
    
    # 计算总距离
    distances = np.sqrt(np.diff(x_positions)**2 + np.diff(y_positions)**2)
    total_distance = np.sum(distances)
    print(f"总行驶距离: {total_distance:.3f} m")
    
    if len(time_axis) > 1:
        avg_speed = total_distance / time_axis[-1] if time_axis[-1] > 0 else 0
        print(f"平均速度: {avg_speed:.3f} m/s")

def main():
    """
    主函数
    """
    # 文件路径
    file_path = r"E:..\Data\静止状态惯性导航校准数据2m.txt"
    
    try:
        # 读取数据
        print("正在读取轨迹数据...")
        yaw_angles, x_positions, y_positions = read_trajectory_data(file_path)
        print(f"成功读取 {len(x_positions)} 个数据点")
        
        # 可视化
        print("正在生成可视化图表...")
        visualize_trajectory(yaw_angles, x_positions, y_positions, sampling_interval=50)
        
    except FileNotFoundError:
        print(f"错误：找不到文件 {file_path}")
    except json.JSONDecodeError:
        print("错误：JSON文件格式不正确")
    except Exception as e:
        print(f"处理过程中出现错误: {str(e)}")

if __name__ == "__main__":
    main()