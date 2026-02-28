import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

class EKFLocalizer:
    def __init__(self, initial_pos):
        # 状态向量: [x, y, vx, vy] (位置和速度)
        self.x = np.array([initial_pos[0], initial_pos[1], 0.0, 0.0]).reshape(-1, 1)
        
        # 状态协方差矩阵 P
        self.P = np.eye(4) * 0.1
        
        # 过程噪声 Q (代表 INS 积分的不确定性)
        # 如果 INS 漂移快，这个值应该设大
        self.Q = np.diag([0.1, 0.1, 0.5, 0.5]) 
        
        # 量测噪声 R (代表 GPS 的定位误差)
        # 如果 GPS 精度高 (如 RTK)，这个值设小；普通 GPS 设大
        self.R = np.diag([2.0, 2.0]) 
        
    def predict(self, ins_delta_pos, dt=1.0):
        """
        预测步：使用 INS 数据推算下一时刻状态
        ins_delta_pos: INS 在该周期内的位移增量
        """
        # 状态转移矩阵 (假设匀速模型 + INS 增量修正)
        # x_new = x + ins_dx
        # v_new = ins_dx / dt
        F = np.array([
            [1, 0, dt, 0],
            [0, 1, 0, dt],
            [0, 0, 1, 0],
            [0, 0, 0, 1]
        ])
        
        # 控制输入向量 u (INS 测量的位移增量)
        u = np.array([
            [ins_delta_pos[0]], 
            [ins_delta_pos[1]]
        ])
        
        # 简化的状态更新 (直接加上 INS 增量)
        # 实际紧耦合中，这里应该包含姿态旋转矩阵，这里简化为平面叠加
        B = np.array([
            [1, 0],
            [0, 1],
            [0, 0],
            [0, 0]
        ])
        
        self.x = F @ self.x + B @ u
        
        # 更新协方差
        self.P = F @ self.P @ F.T + self.Q

    def update(self, gps_pos):
        """
        更新步：使用 GPS 数据修正状态
        gps_pos: GPS 测量到的位置
        """
        # 量测矩阵 H (我们只能观测位置 x, y)
        H = np.array([
            [1, 0, 0, 0],
            [0, 1, 0, 0]
        ])
        
        # 量测值
        z = np.array([[gps_pos[0]], [gps_pos[1]]])
        
        # 残差
        y = z - H @ self.x
        
        # 卡尔曼增益
        S = H @ self.P @ H.T + self.R
        K = self.P @ H.T @ np.linalg.inv(S)
        
        # 状态更新
        self.x = self.x + K @ y
        
        # 协方差更新
        I = np.eye(4)
        self.P = (I - K @ H) @ self.P
        
        return self.x.flatten()[:2]

def run_loose_coupling_fusion(csv_file):
    df = pd.read_csv(csv_file)
    
    # 数据预处理 (同第一部分)
    meter_per_deg_lat = 111000.0
    meter_per_deg_lon = 111000.0 * np.cos(np.radians(df['latitude'].iloc[0]))
    df['gps_x'] = (df['longitude'] - df['longitude'].iloc[0]) * meter_per_deg_lon
    df['gps_y'] = (df['latitude'] - df['latitude'].iloc[0]) * meter_per_deg_lat
    
    # 这里的 INS 数据假设已经做好了坐标对齐 (如第一部分代码处理过)
    # 为了演示，假设 INS 数据已经是米单位且大致对齐
    df['ins_x_m'] = df['nav_x'] / 1000.0
    df['ins_y_m'] = df['nav_y'] / 1000.0
    
    # 初始化滤波器
    init_pos = [df['gps_x'].iloc[0], df['gps_y'].iloc[0]]
    ekf = EKFLocalizer(init_pos)
    
    fused_x, fused_y = [], []
    ins_x, ins_y = [], []
    gps_x, gps_y = [], []
    
    prev_ins_x = df['ins_x_m'].iloc[0]
    prev_ins_y = df['ins_y_m'].iloc[0]
    
    # 循环处理
    for i in range(len(df)):
        curr_ins_x = df['ins_x_m'].iloc[i]
        curr_ins_y = df['ins_y_m'].iloc[i]
        curr_gps_x = df['gps_x'].iloc[i]
        curr_gps_y = df['gps_y'].iloc[i]
        
        # 1. 预测步
        ins_dx = curr_ins_x - prev_ins_x
        ins_dy = curr_ins_y - prev_ins_y
        ekf.predict([ins_dx, ins_dy])
        
        # 2. 更新步 (如果有有效GPS数据)
        # 注意：state=1 表示数据有效，这里简化为每步都更新
        fx, fy = ekf.update([curr_gps_x, curr_gps_y])
        
        fused_x.append(fx)
        fused_y.append(fy)
        
        ins_x.append(curr_ins_x)
        ins_y.append(curr_ins_y)
        gps_x.append(curr_gps_x)
        gps_y.append(curr_gps_y)
        
        prev_ins_x = curr_ins_x
        prev_ins_y = curr_ins_y

    # 可视化结果
    plt.figure(figsize=(10, 6))
    plt.plot(gps_x, gps_y, 'g.', label='GPS 观测值', markersize=5)
    plt.plot(ins_x, ins_y, 'r--', label='INS 推算值')
    plt.plot(fused_x, fused_y, 'b-', label='EKF 融合结果', linewidth=2)
    plt.legend()
    plt.title("松耦合 EKF 融合效果演示")
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    # 注意：由于示例数据中 INS 数据几乎静止 (nav_x/nav_y 变化极小)，
    # 而GPS在移动，融合结果会主要由GPS主导。此代码用于演示算法逻辑。
    try:
        run_loose_coupling_fusion("E:\\github_projects\\autocar1\\environment4BB7\\Seekfree_CYT4BB_Opensource_Library\\Data\\gnss_data_20260218_171236.csv")
    except:
        print("请确保数据文件存在以运行融合演示")