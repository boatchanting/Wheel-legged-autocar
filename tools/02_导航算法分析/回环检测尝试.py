import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# 原始日志数据 (我们只关心索引、X、Y)
raw_data_log = """
NavFlash_ReadFlashToRam: 0, -447.163116, -295.528198, 0
NavFlash_ReadFlashToRam: 1, -672.751587, 195.519867, 0
NavFlash_ReadFlashToRam: 2, -1137.855225, 155.671310, 0
NavFlash_ReadFlashToRam: 3, -966.768188, -188.009567, 0
NavFlash_ReadFlashToRam: 4, -425.066895, -307.531372, 0
NavFlash_ReadFlashToRam: 5, 37.044724, -24.493494, 0
"""

def parse_trajectory_data(log_string):
    """解析日志数据并构建 DataFrame"""
    data = []
    
    # 插入起始点 (Index -1, 坐标 0, 0)
    # 虽然实际记录是从 P0 开始，但我们通常将起点设为 P_start
    data.append({'Index': -1, 'X': 0.0, 'Y': 0.0}) 
    
    for line in log_string.strip().split('\n'):
        try:
            parts = line.split(': ')[1].split(', ')
            index = int(parts[0])
            x = float(parts[1])
            y = float(parts[2])
            data.append({'Index': index, 'X': x, 'Y': y})
        except:
            continue
            
    df = pd.DataFrame(data)
    
    # 重新索引，方便后续处理
    df.index = df['Index']
    return df

# 原始轨迹数据 (包含 P_start 和 P0 到 P5)
df_raw = parse_trajectory_data(raw_data_log)
print("--- 原始轨迹数据 (P_start = 0, P0到P5) ---")
print(df_raw[['X', 'Y']])

def apply_loop_closure_correction(df):
    """
    应用线性误差分配进行回环修正。
    假设 P5 必须闭合到 P_start (0, 0)。
    """
    
    # P_start 是我们的目标点
    target_x, target_y = 0.0, 0.0
    
    # P5 是实际的闭合点
    if 5 not in df.index:
        raise ValueError("数据中找不到 P5")

    P5_actual_x = df.loc[5, 'X']
    P5_actual_y = df.loc[5, 'Y']
    
    # 1. 计算累积误差 (Closure Gap)
    error_x = P5_actual_x - target_x
    error_y = P5_actual_y - target_y
    
    # 包含 P0 到 P5 的点，共 6 个点需要修正 (P_start 不动)
    N_segments = 6 
    
    df_corrected = df.copy()
    
    print(f"\n--- 回环检测结果 ---")
    print(f"P5 实际坐标: ({P5_actual_x:.4f}, {P5_actual_y:.4f})")
    print(f"目标闭合坐标: ({target_x:.4f}, {target_y:.4f})")
    print(f"累积漂移误差 E: ({error_x:.4f}, {error_y:.4f})")

    # 2. 遍历 P0 到 P5，应用线性修正
    for i in range(N_segments):
        # 索引 i 对应于 df 中的 i (例如，i=0 是 P0, i=5 是 P5)
        
        # 修正系数 (i+1)/N，因为 P0 接收 1/N 的修正，P5 接收 N/N 的修正
        correction_factor = (i + 1) / N_segments
        
        correction_x = error_x * correction_factor
        correction_y = error_y * correction_factor
        
        # 修正后的坐标
        df_corrected.loc[i, 'X_corrected'] = df.loc[i, 'X'] - correction_x
        df_corrected.loc[i, 'Y_corrected'] = df.loc[i, 'Y'] - correction_y
    
    # 3. P_start (-1) 不动
    df_corrected.loc[-1, 'X_corrected'] = df.loc[-1, 'X']
    df_corrected.loc[-1, 'Y_corrected'] = df.loc[-1, 'Y']
    
    return df_corrected

# 执行修正
df_corrected = apply_loop_closure_correction(df_raw)

print("\n--- 修正后的 P0 到 P5 坐标 ---")
print(df_corrected.loc[0:5, ['X_corrected', 'Y_corrected']])

# 检查 P5 是否成功闭合
P5_corrected_x = df_corrected.loc[5, 'X_corrected']
P5_corrected_y = df_corrected.loc[5, 'Y_corrected']
print(f"\nP5 修正后坐标: ({P5_corrected_x:.6f}, {P5_corrected_y:.6f})")

def visualize_comparison(df_raw, df_corrected):
    """可视化修正前后的轨迹"""
    
    fig, ax = plt.subplots(1, 2, figsize=(15, 6))
    
    # --- 左图：原始轨迹 ---
    
    # 轨迹线
    ax[0].plot(df_raw['X'], df_raw['Y'], 'r-', label='原始轨迹 (Odometry)', marker='o', alpha=0.7)
    
    # 标记起点和终点
    ax[0].plot(df_raw.loc[-1, 'X'], df_raw.loc[-1, 'Y'], 'go', markersize=10, label='P_start (0,0)')
    ax[0].plot(df_raw.loc[5, 'X'], df_raw.loc[5, 'Y'], 'ko', markersize=10, label='P5_End (漂移点)')
    
    # 标记 P5 目标点 (虚线表示差距)
    ax[0].plot(0, 0, 'gx', markersize=10, label='Target P_start', mew=3)
    ax[0].plot([df_raw.loc[5, 'X'], 0], [df_raw.loc[5, 'Y'], 0], 'k--', alpha=0.5, label='漂移误差')
    
    ax[0].set_title('修正前：累积漂移轨迹 (未闭合)')
    ax[0].set_xlabel('X 坐标')
    ax[0].set_ylabel('Y 坐标')
    ax[0].legend()
    ax[0].grid(True)
    ax[0].axis('equal') # 保证 X 和 Y 轴比例尺一致

    # --- 右图：修正后轨迹 ---

    # 轨迹线
    ax[1].plot(df_corrected['X_corrected'], df_corrected['Y_corrected'], 'b-', label='修正后轨迹 (Loop Closure)', marker='s', alpha=0.7)
    
    # 标记起点和修正后的终点
    ax[1].plot(df_corrected.loc[-1, 'X_corrected'], df_corrected.loc[-1, 'Y_corrected'], 'go', markersize=10, label='P_start (0,0)')
    ax[1].plot(df_corrected.loc[5, 'X_corrected'], df_corrected.loc[5, 'Y_corrected'], 'bo', markersize=10, label='P5_End (已闭合)')
    
    ax[1].set_title('修正后：回环检测闭合轨迹')
    ax[1].set_xlabel('X 坐标')
    ax[1].set_ylabel('Y 坐标')
    ax[1].legend()
    ax[1].grid(True)
    ax[1].axis('equal')

    plt.suptitle('使用线性误差分配的回环检测轨迹修正', fontsize=16)
    plt.show()

# 调用可视化函数
visualize_comparison(df_raw, df_corrected)