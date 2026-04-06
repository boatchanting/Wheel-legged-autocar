import streamlit as st
import pandas as pd
import numpy as np
import plotly.graph_objects as go
from pyproj import Transformer
import io

# --------------------------
# 1. 数据处理与融合类
# --------------------------

class NavigationProcessor:
    def __init__(self, df):
        self.df = df.copy()
        self.origin_gps_x = 0.0
        self.origin_gps_y = 0.0
        self.origin_ins_x = 0.0 
        self.origin_ins_y = 0.0
        
        self.auto_angle = 0.0
        self.auto_scale = 1.0
        
        self.col_map = {}
        self._identify_columns()

    def _identify_columns(self):
        cols = self.df.columns
        if 'loop' in cols: self.col_map['time'] = 'loop'
        elif 'loop_counter' in cols: self.col_map['time'] = 'loop_counter'
        else: self.col_map['time'] = cols[0]
            
        if 'nav_x' in cols: self.col_map['nav_x'] = 'nav_x'
        if 'nav_y' in cols: self.col_map['nav_y'] = 'nav_y'
            
        if 'latitude' in cols: self.col_map['lat'] = 'latitude'
        if 'longitude' in cols: self.col_map['lon'] = 'longitude'
        if 'state' in cols: self.col_map['state'] = 'state'
        
        if 'speed' in cols: self.col_map['speed'] = 'speed'
        if 'direction' in cols: self.col_map['yaw'] = 'direction'
        elif 'ant_direction' in cols: self.col_map['yaw'] = 'ant_direction'

    def process_gps_projection(self):
        # ... (保持不变) ...
        valid_mask = self.df[self.col_map['state']] == 1
        if not np.any(valid_mask): return self.df
            
        first_valid_idx = self.df[valid_mask].index[0]
        lats = self.df[self.col_map['lat']].values
        lons = self.df[self.col_map['lon']].values
        
        mean_lon = np.mean(lons)
        zone = int(mean_lon // 3 + 1)
        central_meridian = zone * 3
        
        transformer = Transformer.from_crs(
            "EPSG:4326", 
            f"+proj=tmerc +lat_0=0 +lon_0={central_meridian} +k=1 +x_0=500000 +y_0=0 +ellps=WGS84 +units=m +no_defs",
            always_xy=True
        )
        x, y = transformer.transform(lons, lats)
        
        self.origin_gps_x = x[first_valid_idx]
        self.origin_gps_y = y[first_valid_idx]
        
        self.df['gps_x_proj'] = x - self.origin_gps_x
        self.df['gps_y_proj'] = y - self.origin_gps_y
        return self.df

    def calculate_alignment_params(self, calib_dist_threshold=5.0):
        # ... (保持不变) ...
        self.df['nav_x_m'] = self.df[self.col_map['nav_x']] / 1000.0 
        self.df['nav_y_m'] = self.df[self.col_map['nav_y']] / 1000.0
        
        first_valid_idx = self.df[self.df[self.col_map['state']] == 1].index[0]
        self.origin_ins_x = self.df.loc[first_valid_idx, 'nav_x_m']
        self.origin_ins_y = self.df.loc[first_valid_idx, 'nav_y_m']
        
        self.df['nav_x_rel'] = self.df['nav_x_m'] - self.origin_ins_x
        self.df['nav_y_rel'] = self.df['nav_y_m'] - self.origin_ins_y
        
        dx = self.df['gps_x_proj'].diff().fillna(0)
        dy = self.df['gps_y_proj'].diff().fillna(0)
        dist = np.sqrt(dx**2 + dy**2)
        cum_dist = dist.cumsum()
        
        calib_df = self.df[(cum_dist > calib_dist_threshold) & (self.df[self.col_map['state']] == 1)]
        if calib_df.empty: return

        calib_idx = calib_df.index[0]
        vec_gps = np.array([self.df.loc[calib_idx, 'gps_x_proj'], self.df.loc[calib_idx, 'gps_y_proj']])
        vec_ins = np.array([self.df.loc[calib_idx, 'nav_x_rel'], self.df.loc[calib_idx, 'nav_y_rel']])
        
        angle_gps = np.arctan2(vec_gps[1], vec_gps[0])
        angle_ins = np.arctan2(vec_ins[1], vec_ins[0])
        self.auto_angle = np.degrees(angle_gps - angle_ins)
        
        len_gps = np.linalg.norm(vec_gps)
        len_ins = np.linalg.norm(vec_ins)
        if len_ins > 1e-3: self.auto_scale = len_gps / len_ins

    def apply_alignment_and_fusion(self, manual_angle, manual_scale, k_coeff, base_noise):
        """
        应用动态阈值进行融合
        k_coeff: 速度比例系数
        base_noise: 基底噪声
        """
        # 1. 坐标对齐
        rad = np.radians(manual_angle)
        cos_a, sin_a = np.cos(rad), np.sin(rad)
        
        x_rel = self.df['nav_x_rel'].values
        y_rel = self.df['nav_y_rel'].values
        
        self.df['nav_x_aligned'] = (x_rel * cos_a - y_rel * sin_a) * manual_scale
        self.df['nav_y_aligned'] = (x_rel * sin_a + y_rel * cos_a) * manual_scale
        
        fused_x, fused_y, status_list, innov_list, threshold_list = [], [], [], [], []
        
        state = np.array([0.0, 0.0]) 
        P = np.diag([0.1, 0.1])
        Q = np.diag([0.005, 0.005])
        R = np.diag([0.5, 0.5])
        
        last_ins_x, last_ins_y = 0.0, 0.0
        last_time = self.df[self.col_map['time']].values[0]
        
        # 获取速度数据 (km/h -> m/s)
        speed_kmh = self.df[self.col_map['speed']].values if self.col_map['speed'] else np.zeros(len(self.df))
        speeds = speed_kmh / 3.6
        
        for i in range(len(self.df)):
            curr_time = self.df[self.col_map['time']].values[i]
            dt = (curr_time - last_time) / 1000.0
            if dt <= 0: dt = 0.1
            
            # --- A. 预测步 ---
            curr_ins_x = self.df.loc[i, 'nav_x_aligned']
            curr_ins_y = self.df.loc[i, 'nav_y_aligned']
            
            delta_x = curr_ins_x - last_ins_x
            delta_y = curr_ins_y - last_ins_y
            
            state_pred = state + np.array([delta_x, delta_y])
            P_pred = P + Q
            
            # --- B. 动态阈值计算 (核心改进) ---
            # 阈值 = 当前速度 * 时间间隔 * 比例系数 + 基底噪声
            # 代表：在这一瞬间，允许惯导推算和GPS测量之间的最大偏差距离
            v_curr = speeds[i]
            dynamic_threshold = v_curr * dt * k_coeff + base_noise
            
            # --- C. 更新步 ---
            gps_valid = (self.df.loc[i, self.col_map['state']] == 1)
            
            if gps_valid:
                z = np.array([self.df.loc[i, 'gps_x_proj'], self.df.loc[i, 'gps_y_proj']])
                y_innov = z - state_pred
                
                # 计算欧氏距离
                dist_innovation = np.linalg.norm(y_innov)
                
                # 记录数据
                innov_list.append(dist_innovation)
                threshold_list.append(dynamic_threshold)
                
                # 比较：新息距离 vs 动态阈值
                if dist_innovation > dynamic_threshold:
                    # 异常：拒绝GPS
                    state = state_pred
                    P = P_pred
                    status_list.append("Rejected")
                else:
                    # 正常：融合
                    K = P_pred @ np.linalg.inv(P_pred + R) # 简化卡尔曼增益
                    state = state_pred + K @ y_innov
                    P = (np.eye(2) - K) @ P_pred
                    status_list.append("Fused")
            else:
                # 纯惯导
                state = state_pred
                P = P_pred
                status_list.append("Dead Reckoning")
                innov_list.append(0)
                threshold_list.append(0)
            
            fused_x.append(state[0])
            fused_y.append(state[1])
            
            last_ins_x, last_ins_y = curr_ins_x, curr_ins_y
            last_time = curr_time
            
            if i == 0:
                state = np.array([0.0, 0.0])
                fused_x[0] = 0.0
                fused_y[0] = 0.0
        
        self.df['fused_x'] = fused_x
        self.df['fused_y'] = fused_y
        self.df['fusion_status'] = status_list
        self.df['innovation'] = innov_list
        self.df['dynamic_threshold'] = threshold_list
        
        return self.df

# --------------------------
# 2. Streamlit 界面
# --------------------------

def main():
    st.set_page_config(layout="wide", page_title="INS/GNSS 动态阈值融合")
    st.title("INS/GNSS 融合导航：自适应新息检测")
    
    st.markdown("""
    **核心改进：**
    新息检测阈值不再是固定值，而是基于**当前速度**动态计算：
    $$ \\text{阈值} = (\\text{速度} \\times \\Delta t \\times \\text{比例系数}) + \\text{基底噪声} $$     - **静止时**：阈值极小（仅基底噪声），严格剔除GPS漂移。
    - **运动时**：阈值随速度变大，允许惯导推算误差和GPS延迟带来的正常偏差。
    """)

    uploaded_file = st.file_uploader("上传导航数据 CSV 文件", type=["csv"])
    
    if uploaded_file is not None:
        try:
            uploaded_file.seek(0)
            df = pd.read_csv(uploaded_file, sep=None, engine='python', skipinitialspace=True)
            df.columns = df.columns.str.strip()
            
            processor = NavigationProcessor(df)
            processor.process_gps_projection()
            
            # 校准参数
            calib_dist = st.sidebar.number_input("自动校准行驶距离阈值", value=5.0, min_value=1.0)
            processor.calculate_alignment_params(calib_dist_threshold=calib_dist)
            
            st.sidebar.subheader("1. 坐标对齐微调")
            manual_angle = st.sidebar.number_input("旋转角度 (度)", value=float(processor.auto_angle), step=0.1, format="%.2f")
            manual_scale = st.sidebar.number_input("尺度系数", value=float(processor.auto_scale), step=0.001, format="%.4f")
            
            # 动态阈值参数
            st.sidebar.subheader("2. 动态新息检测参数")
            k_coeff = st.sidebar.slider("速度比例系数 (K)", 0.1, 10.0, 3.0, help="速度越快，允许的误差半径越大。建议3.0")
            base_noise = st.sidebar.slider("基底噪声 (米)", 0.0, 2.0, 0.2, help="速度为0时的最小允许误差")
            
            with st.spinner("正在计算轨迹..."):
                df_result = processor.apply_alignment_and_fusion(manual_angle, manual_scale, k_coeff, base_noise)
            
            # --------------------------
            # 可视化 1: 轨迹
            # --------------------------
            st.subheader("融合轨迹")
            fig = go.Figure()
            
            fig.add_trace(go.Scatter(x=df_result['gps_x_proj'], y=df_result['gps_y_proj'], mode='markers', name='原始GPS', marker=dict(size=3, color='rgba(0,0,255,0.3)')))
            fig.add_trace(go.Scatter(x=df_result['nav_x_aligned'], y=df_result['nav_y_aligned'], mode='lines', name='对齐后惯导', line=dict(color='orange', width=2)))
            fig.add_trace(go.Scatter(x=df_result['fused_x'], y=df_result['fused_y'], mode='lines', name='融合轨迹', line=dict(color='red', width=2)))
            
            rejected = df_result[df_result['fusion_status'] == "Rejected"]
            if not rejected.empty:
                fig.add_trace(go.Scatter(x=rejected['gps_x_proj'], y=rejected['gps_y_proj'], mode='markers', name='剔除的GPS', marker=dict(size=8, color='black', symbol='x')))
            
            fig.update_yaxes(scaleanchor="x", scaleratio=1)
            fig.update_layout(height=600, legend=dict(orientation="h", y=1.05))
            st.plotly_chart(fig, use_container_width=True)
            
            # --------------------------
            # 可视化 2: 动态阈值监控
            # --------------------------
            st.subheader("新息距离 vs 动态阈值")
            fig2 = go.Figure()
            
            # 画出实际的新息距离
            fig2.add_trace(go.Scatter(y=df_result['innovation'], mode='lines', name='实际偏差', line=dict(color='blue')))
            
            # 画出动态阈值线
            fig2.add_trace(go.Scatter(y=df_result['dynamic_threshold'], mode='lines', name='动态阈值', line=dict(color='red', dash='dash')))
            
            fig2.update_layout(yaxis_title="距离 (米)", xaxis_title="时间序列索引")
            st.plotly_chart(fig2, use_container_width=True)
            
            st.markdown("""
            **图表解读：**
            - **蓝线**：当前GPS测量值与惯导预测值之间的实际距离。
            - **红线**：动态允许的最大误差。
            - 当蓝线超过红线时，该GPS点被视为异常（黑叉），算法将信任惯导。
            - 你可以看到，在静止时（蓝线通常应为0），红线会下降到基底噪声水平，这能有效滤除静止漂移。
            """)
            
        except Exception as e:
            st.error(f"处理出错: {e}")
            import traceback
            st.text(traceback.format_exc())

if __name__ == "__main__":
    main()