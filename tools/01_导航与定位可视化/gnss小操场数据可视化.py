# filename: ins_gps_visualizer.py
import streamlit as st
import pandas as pd
import numpy as np
import plotly.graph_objects as go
from pyproj import Transformer
from datetime import datetime
import math

# ==================== 页面配置 ====================
st.set_page_config(
    page_title="惯性导航与GPS数据融合可视化",
    page_icon="🛰️",
    layout="wide"
)

st.title("🛰️ 惯性导航与GPS数据融合可视化系统")
st.markdown("---")

# ==================== 侧边栏配置 ====================
st.sidebar.header("⚙️ 参数配置")

# 速度阈值配置
speed_threshold = st.sidebar.number_input(
    "小车启动速度阈值 (mm/s)",
    min_value=0.0,
    value=50.0,
    step=10.0,
    help="用于计算初始航向对齐的速度阈值"
)

# 对齐时间窗口
alignment_window = st.sidebar.number_input(
    "航向对齐时间窗口 (秒)",
    min_value=1.0,
    value=5.0,
    step=1.0,
    help="小车启动后用于计算平均航向差异的时间"
)

# GPS方向过滤
gps_direction_filter = st.sidebar.checkbox(
    "启用GPS方向过滤",
    value=True,
    help="基于车身速度过滤不稳定的GPS方向数据"
)

# 融合策略
fusion_strategy = st.sidebar.selectbox(
    "数据融合策略",
    ["惯导为主", "自适应融合", "GPS为主"],
    index=0,
    help="选择主要信任的数据源"
)

# 坐标系说明
st.sidebar.markdown("---")
st.sidebar.info("""
**坐标系定义:**

🧭 **GPS航向角:**
- 0°: 正北
- 90°: 正东
- 180°: 正南
- 270°: 正西
- 顺时针增加

🚗 **惯导坐标系 (车体坐标系):**
- X轴: 小车**向后**为正方向
- Y轴: 小车**向右**为正方向

📊 **车身速度:**
- vx_body: 向前为**负**, 向后为**正** (mm/s)
- vy_body: 向右为**正** (mm/s)
""")

# ==================== 文件上传 ====================
uploaded_file = st.file_uploader("📁 上传CSV数据文件", type=["csv"])

if uploaded_file is not None:
    # 读取数据
    df = pd.read_csv(uploaded_file)
    st.success(f"✅ 成功加载 {len(df)} 条数据记录")
    
    # ==================== 数据预处理 ====================
    st.header("1️⃣ 数据预处理")
    
    # 创建时间戳
    df['timestamp'] = pd.to_datetime(df[['year', 'month', 'day', 'hour', 'minute', 'second']])
    df['time_elapsed'] = (df['timestamp'] - df['timestamp'].iloc[0]).dt.total_seconds()
    
    # 计算车身合速度
    df['body_speed'] = np.sqrt(df['vx_body']**2 + df['vy_body']**2)
    
    # 显示数据概览
    col1, col2, col3 = st.columns(3)
    with col1:
        st.metric("数据点数", f"{len(df):,}")
    with col2:
        st.metric("时间跨度", f"{df['time_elapsed'].max():.1f} 秒")
    with col3:
        st.metric("平均车身速度", f"{df['body_speed'].mean():.1f} mm/s")
    
    # ==================== 高斯-克吕格投影转换 ====================
    st.header("2️⃣ 高斯-克吕格投影转换")
    
    # 根据经度自动确定投影带
    central_lon = df['longitude'].mean()
    zone = int((central_lon + 3) / 6)  # 6度分带
    central_meridian = zone * 6 - 3
    
    st.info(f"📍 自动识别投影带: {zone}带, 中央经线: {central_meridian}°E")
    
    # 创建高斯-克吕格投影转换器 (WGS84 -> Gauss-Kruger)
    transformer = Transformer.from_crs(
        "EPSG:4326",
        f"+proj=tmerc +lat_0=0 +lon_0={central_meridian} +k=1 +x_0=500000 +y_0=0 +ellps=WGS84 +units=m +no_defs",
        always_xy=True
    )
    
    # 转换GPS坐标到笛卡尔坐标系 (单位: 米)
    df['gps_x_m'], df['gps_y_m'] = transformer.transform(df['longitude'], df['latitude'])
    
    # 转换为毫米以便与惯导数据统一
    df['gps_x_mm'] = df['gps_x_m'] * 1000
    df['gps_y_mm'] = df['gps_y_m'] * 1000
    
    # 以第一个点为原点
    df['gps_x_relative_mm'] = df['gps_x_mm'] - df['gps_x_mm'].iloc[0]
    df['gps_y_relative_mm'] = df['gps_y_mm'] - df['gps_y_mm'].iloc[0]
    
    st.success("✅ 完成高斯-克吕格投影转换 (精度: 厘米级)")
    
    # ==================== 航向角计算与坐标系转换 ====================
    st.header("3️⃣ 航向角计算与坐标系转换")
    
    # ============ GPS航向角 (地理坐标系) ============
    df['gps_heading_geo'] = df['direction']  # 直接使用，已经是地理坐标系
    
    # ============ 惯导航向角 (车体坐标系) ============
    df['ins_heading_body'] = np.degrees(np.arctan2(df['nav_y'], df['nav_x']))
    df['ins_heading_body'] = (df['ins_heading_body'] + 360) % 360
    
    # 车身速度航向角 (车体坐标系)
    df['body_heading_body'] = np.degrees(np.arctan2(df['vy_body'], df['vx_body']))
    df['body_heading_body'] = (df['body_heading_body'] + 360) % 360
    
    st.markdown("### 航向角对齐计算")
    
    # 识别小车启动阶段 (速度大于阈值)
    moving_mask = df['body_speed'] > speed_threshold
    
    heading_offset = 0  # 初始化偏移角
    
    if moving_mask.sum() > 10:
        moving_indices = df[moving_mask].index
        if len(moving_indices) > 0:
            start_time = df.loc[moving_indices[0], 'time_elapsed']
            end_time = start_time + alignment_window
            alignment_mask = (df['time_elapsed'] >= start_time) & (df['time_elapsed'] <= end_time) & moving_mask
            
            if alignment_mask.sum() > 5:
                alignment_df = df[alignment_mask].copy()
                
                # 计算惯导位置变化
                delta_nav_x = alignment_df['nav_x'].diff().mean()
                delta_nav_y = alignment_df['nav_y'].diff().mean()
                
                if abs(delta_nav_x) > 0.001 or abs(delta_nav_y) > 0.001:
                    ins_motion_body = np.degrees(np.arctan2(delta_nav_y, delta_nav_x))
                    ins_motion_body = (ins_motion_body + 360) % 360
                else:
                    ins_motion_body = 0
                
                # GPS运动方向
                delta_gps_x = alignment_df['gps_x_relative_mm'].diff().mean()
                delta_gps_y = alignment_df['gps_y_relative_mm'].diff().mean()
                
                if abs(delta_gps_x) > 0.001 or abs(delta_gps_y) > 0.001:
                    gps_motion_geo = np.degrees(np.arctan2(delta_gps_x, delta_gps_y))
                    gps_motion_geo = (gps_motion_geo + 360) % 360
                else:
                    gps_motion_geo = df.loc[alignment_mask, 'gps_heading_geo'].mean()
                
                heading_offset = gps_motion_geo - ins_motion_body
                heading_offset = ((heading_offset + 180) % 360) - 180
                
                st.success(f"✅ 航向对齐完成:")
                st.info(f"- 惯导运动方向 (车体): {ins_motion_body:.2f}°")
                st.info(f"- GPS运动方向 (地理): {gps_motion_geo:.2f}°")
                st.info(f"- 航向偏移角: {heading_offset:.2f}°")
                
                df['ins_heading_geo'] = (df['ins_heading_body'] + heading_offset + 360) % 360
            else:
                st.warning("⚠️ 对齐时间窗口内数据不足，使用原始航向")
                df['ins_heading_geo'] = df['ins_heading_body']
    else:
        st.warning("⚠️ 未检测到小车启动阶段，使用原始航向")
        df['ins_heading_geo'] = df['ins_heading_body']
    
    if 'ins_heading_geo' not in df.columns:
        df['ins_heading_geo'] = df['ins_heading_body']
    
    # ==================== GPS方向过滤 ====================
    st.header("4️⃣ GPS方向数据过滤")
    
    if gps_direction_filter:
        df['gps_direction_valid'] = False
        
        speed_condition = df['body_speed'] > speed_threshold
        
        gps_heading_diff = df['gps_heading_geo'].diff().abs()
        gps_heading_diff = gps_heading_diff.fillna(0)
        gps_heading_diff = np.where(gps_heading_diff > 180, 360 - gps_heading_diff, gps_heading_diff)
        direction_stable = gps_heading_diff < 15
        
        heading_diff = np.abs(df['ins_heading_geo'] - df['gps_heading_geo'])
        heading_diff = np.where(heading_diff > 180, 360 - heading_diff, heading_diff)
        heading_consistent = heading_diff < 45
        
        sat_condition = df['sat_used'] >= 10
        
        df['gps_direction_valid'] = speed_condition & direction_stable & heading_consistent & sat_condition
        
        valid_count = df['gps_direction_valid'].sum()
        st.info(f"📊 GPS方向有效数据: {valid_count}/{len(df)} ({valid_count/len(df)*100:.1f}%)")
        
        df['gps_heading_filtered'] = df['gps_heading_geo'].where(df['gps_direction_valid'], np.nan)
    else:
        df['gps_heading_filtered'] = df['gps_heading_geo']
        df['gps_direction_valid'] = True
    
    # ==================== 数据融合 ====================
    st.header("5️⃣ 惯导与GPS数据融合")
    
    heading_rad = np.radians(df['ins_heading_geo'])
    
    df['nav_x_geo_mm'] = df['nav_x'] * np.sin(heading_rad) + df['nav_y'] * np.cos(heading_rad)
    df['nav_y_geo_mm'] = -df['nav_x'] * np.cos(heading_rad) + df['nav_y'] * np.sin(heading_rad)
    
    if fusion_strategy == "惯导为主":
        df['fused_x_mm'] = df['nav_x_geo_mm']
        df['fused_y_mm'] = df['nav_y_geo_mm']
        df['fused_heading'] = df['ins_heading_geo']
        
        ins_stationary = df['body_speed'] < 10
        gps_moving = df['body_speed'].rolling(window=10, min_periods=1).std() > 50
        
        df['gps_anomaly'] = ins_stationary & gps_moving
        anomaly_count = df['gps_anomaly'].sum()
        
        if anomaly_count > 0:
            st.warning(f"⚠️ 检测到 {anomaly_count} 个GPS异常点 (惯导静止但GPS移动)")
        
    elif fusion_strategy == "自适应融合":
        df['gps_quality'] = df['sat_used'] / 20.0
        df['gps_quality'] = df['gps_quality'].clip(0, 1)
        
        df['gps_weight'] = df['gps_quality'] * df['gps_direction_valid'].astype(float)
        df['ins_weight'] = 1 - df['gps_weight']
        
        df['fused_x_mm'] = df['nav_x_geo_mm'] * df['ins_weight'] + df['gps_x_relative_mm'] * df['gps_weight']
        df['fused_y_mm'] = df['nav_y_geo_mm'] * df['ins_weight'] + df['gps_y_relative_mm'] * df['gps_weight']
        
        df['fused_heading'] = df['ins_heading_geo'] * df['ins_weight'] + df['gps_heading_filtered'] * df['gps_weight']
        
        st.info("🔄 使用自适应融合策略 (根据卫星数量和方向稳定性调整权重)")
        
    else:
        df['fused_x_mm'] = df['gps_x_relative_mm']
        df['fused_y_mm'] = df['gps_y_relative_mm']
        df['fused_heading'] = df['gps_heading_filtered']
    
    # ==================== 可视化 ====================
    st.header("6️⃣ 数据可视化")
    
    tab1, tab2, tab3, tab4, tab5 = st.tabs([
        "🗺️ 轨迹对比", 
        "🧭 航向角对比", 
        "📊 速度分析", 
        "🔍 异常检测",
        "📈 位置偏差"
    ])
    
    with tab1:
        st.subheader("轨迹对比 (地理坐标系, 可缩放至厘米级)")
        
        fig = go.Figure()
        
        # 惯导轨迹
        fig.add_trace(go.Scatter(
            x=df['nav_x_geo_mm'] / 10,
            y=df['nav_y_geo_mm'] / 10,
            mode='lines',
            name='惯性导航 (地理系)',
            line={'color': 'blue', 'width': 2},
            hovertemplate='惯导<br>东: %{x:.2f} cm<br>北: %{y:.2f} cm<extra></extra>'
        ))
        
        # GPS轨迹
        fig.add_trace(go.Scatter(
            x=df['gps_x_relative_mm'] / 10,
            y=df['gps_y_relative_mm'] / 10,
            mode='lines',
            name='GPS原始',
            line={'color': 'red', 'width': 2, 'dash': 'dash'},
            hovertemplate='GPS<br>东: %{x:.2f} cm<br>北: %{y:.2f} cm<extra></extra>'
        ))
        
        # 融合轨迹
        fig.add_trace(go.Scatter(
            x=df['fused_x_mm'] / 10,
            y=df['fused_y_mm'] / 10,
            mode='lines',
            name='融合结果',
            line={'color': 'green', 'width': 3},
            hovertemplate='融合<br>东: %{x:.2f} cm<br>北: %{y:.2f} cm<extra></extra>'
        ))
        
        fig.update_layout(
            title="轨迹对比 (地理坐标系: X=东, Y=北, 单位: 厘米)",
            xaxis_title="东向 (cm)",
            yaxis_title="北向 (cm)",
            hovermode='closest',
            height=600,
            showlegend=True,
            yaxis_scaleanchor="x",
            yaxis_scaleratio=1
        )
        
        fig.update_xaxes(fixedrange=False)
        fig.update_yaxes(fixedrange=False)
        
        st.plotly_chart(fig, use_container_width=True)
        
        col1, col2, col3 = st.columns(3)
        with col1:
            max_deviation = np.max(np.sqrt((df['nav_x_geo_mm'] - df['gps_x_relative_mm'])**2 + 
                                          (df['nav_y_geo_mm'] - df['gps_y_relative_mm'])**2)) / 10
            st.metric("最大轨迹偏差", f"{max_deviation:.2f} cm")
        with col2:
            mean_deviation = np.mean(np.sqrt((df['nav_x_geo_mm'] - df['gps_x_relative_mm'])**2 + 
                                            (df['nav_y_geo_mm'] - df['gps_y_relative_mm'])**2)) / 10
            st.metric("平均轨迹偏差", f"{mean_deviation:.2f} cm")
        with col3:
            total_distance = np.sqrt(df['nav_x_geo_mm'].diff()**2 + df['nav_y_geo_mm'].diff()**2).sum() / 10
            st.metric("总行驶距离", f"{total_distance:.2f} cm")
    
    with tab2:
        st.subheader("航向角对比 (地理坐标系, 北0°东90°)")
        
        fig = go.Figure()
        
        fig.add_trace(go.Scatter(
            x=df['time_elapsed'],
            y=df['ins_heading_body'],
            mode='lines',
            name='惯导航向 (车体系)',
            line={'color': 'lightblue', 'dash': 'dash'}
        ))
        
        fig.add_trace(go.Scatter(
            x=df['time_elapsed'],
            y=df['ins_heading_geo'],
            mode='lines',
            name='惯导航向 (地理系)',
            line={'color': 'blue', 'width': 2}
        ))
        
        fig.add_trace(go.Scatter(
            x=df['time_elapsed'],
            y=df['gps_heading_geo'],
            mode='lines',
            name='GPS航向',
            line={'color': 'red', 'dash': 'dash'}
        ))
        
        if gps_direction_filter:
            fig.add_trace(go.Scatter(
                x=df[df['gps_direction_valid']]['time_elapsed'],
                y=df[df['gps_direction_valid']]['gps_heading_filtered'],
                mode='markers',
                name='GPS航向 (有效)',
                marker={'color': 'green', 'size': 5}
            ))
        
        fig.update_layout(
            title="航向角对比 (单位: 度, 北0°东90°顺时针)",
            xaxis_title="时间 (秒)",
            yaxis_title="航向角 (°)",
            height=500,
            showlegend=True,
            yaxis={'range': [0, 360]}
        )
        
        st.plotly_chart(fig, use_container_width=True)
        
        heading_diff = np.abs(df['ins_heading_geo'] - df['gps_heading_geo'])
        heading_diff = np.where(heading_diff > 180, 360 - heading_diff, heading_diff)
        
        col1, col2, col3 = st.columns(3)
        with col1:
            st.metric("最大航向差异", f"{np.max(heading_diff):.2f}°")
        with col2:
            st.metric("平均航向差异", f"{np.mean(heading_diff):.2f}°")
        with col3:
            st.metric("航向对齐偏移", f"{heading_offset:.2f}°")
    
    with tab3:
        st.subheader("速度分析")
        
        fig = go.Figure()
        
        fig.add_trace(go.Scatter(
            x=df['time_elapsed'],
            y=df['body_speed'],
            mode='lines',
            name='车身合速度',
            line={'color': 'blue'}
        ))
        
        fig.add_trace(go.Scatter(
            x=df['time_elapsed'],
            y=df['vx_body'],
            mode='lines',
            name='vx_body (前后)',
            line={'color': 'green', 'dash': 'dash'}
        ))
        
        fig.add_trace(go.Scatter(
            x=df['time_elapsed'],
            y=df['vy_body'],
            mode='lines',
            name='vy_body (左右)',
            line={'color': 'red', 'dash': 'dash'}
        ))
        
        fig.add_hline(
            y=speed_threshold,
            line_dash="dot",
            annotation_text="启动阈值",
            annotation_position="top right",
            line_color="orange"
        )
        
        fig.update_layout(
            title="车身速度分析 (单位: mm/s)",
            xaxis_title="时间 (秒)",
            yaxis_title="速度 (mm/s)",
            height=500,
            showlegend=True
        )
        
        st.plotly_chart(fig, use_container_width=True)
    
    with tab4:
        st.subheader("异常检测")
        
        if 'gps_anomaly' in df.columns:
            anomaly_df = df[df['gps_anomaly']]
            
            if len(anomaly_df) > 0:
                st.warning(f"⚠️ 检测到 {len(anomaly_df)} 个GPS异常点")
                
                fig = go.Figure()
                
                fig.add_trace(go.Scatter(
                    x=df['time_elapsed'],
                    y=df['body_speed'],
                    mode='lines',
                    name='车身速度',
                    line={'color': 'blue'}
                ))
                
                fig.add_trace(go.Scatter(
                    x=anomaly_df['time_elapsed'],
                    y=anomaly_df['body_speed'],
                    mode='markers',
                    name='GPS异常点',
                    marker={'color': 'red', 'size': 10, 'symbol': 'x'}
                ))
                
                fig.add_hline(
                    y=10,
                    line_dash="dot",
                    annotation_text="静止阈值",
                    line_color="gray"
                )
                
                fig.update_layout(
                    title="GPS异常点检测 (惯导静止但GPS移动)",
                    xaxis_title="时间 (秒)",
                    yaxis_title="车身速度 (mm/s)",
                    height=400
                )
                
                st.plotly_chart(fig, use_container_width=True)
                
                with st.expander("查看异常点详情"):
                    st.dataframe(anomaly_df[['timestamp', 'nav_x', 'nav_y', 'latitude', 'longitude', 
                                            'body_speed', 'gps_heading_geo', 'sat_used']].head(20))
            else:
                st.success("✅ 未检测到GPS异常")
        else:
            st.info("当前融合策略未启用异常检测")
    
    with tab5:
        st.subheader("位置偏差分析")
        
        df['position_diff_mm'] = np.sqrt(
            (df['nav_x_geo_mm'] - df['gps_x_relative_mm'])**2 + 
            (df['nav_y_geo_mm'] - df['gps_y_relative_mm'])**2
        )
        
        fig = go.Figure()
        
        fig.add_trace(go.Scatter(
            x=df['time_elapsed'],
            y=df['position_diff_mm'] / 10,
            mode='lines',
            name='位置偏差',
            line={'color': 'purple'}
        ))
        
        fig.update_layout(
            title="惯导与GPS位置偏差 (单位: 厘米)",
            xaxis_title="时间 (秒)",
            yaxis_title="偏差 (cm)",
            height=400
        )
        
        st.plotly_chart(fig, use_container_width=True)
        
        col1, col2, col3, col4 = st.columns(4)
        with col1:
            st.metric("最小偏差", f"{df['position_diff_mm'].min()/10:.2f} cm")
        with col2:
            st.metric("最大偏差", f"{df['position_diff_mm'].max()/10:.2f} cm")
        with col3:
            st.metric("平均偏差", f"{df['position_diff_mm'].mean()/10:.2f} cm")
        with col4:
            st.metric("偏差标准差", f"{df['position_diff_mm'].std()/10:.2f} cm")
    
    # ==================== 数据导出 ====================
    st.header("7️⃣ 数据导出")
    
    export_cols = ['timestamp', 'nav_x', 'nav_y', 'nav_x_geo_mm', 'nav_y_geo_mm',
                   'gps_x_relative_mm', 'gps_y_relative_mm',
                   'fused_x_mm', 'fused_y_mm', 
                   'ins_heading_body', 'ins_heading_geo',
                   'gps_heading_geo', 'gps_heading_filtered', 
                   'body_speed', 'vx_body', 'vy_body',
                   'gps_direction_valid', 'heading_offset']
    
    export_df = df[export_cols].copy()
    
    csv_data = export_df.to_csv(index=False)
    
    st.download_button(
        label="📥 下载处理后的数据 (CSV)",
        data=csv_data,
        file_name=f"processed_ins_gps_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv",
        mime="text/csv"
    )
    
    with st.expander("📖 使用说明"):
        st.markdown("""
        ### 坐标系说明
        
        **GPS航向角 (地理坐标系):**
        - 0°: 正北
        - 90°: 正东
        - 180°: 正南
        - 270°: 正西
        - 顺时针增加
        
        **惯导坐标系 (车体坐标系):**
        - X轴: 小车**向后**为正方向
        - Y轴: 小车**向右**为正方向
        - 通过航向对齐转换到地理坐标系
        
        **地理坐标系 (可视化用):**
        - X轴: 正东方向
        - Y轴: 正北方向
        
        ### 核心功能
        
        1. **高斯-克吕格投影**: 将GPS经纬度转换为笛卡尔坐标，支持厘米级精度
        2. **航向角对齐**: 使用小车启动阶段计算车体坐标系与地理坐标系的偏移角
        3. **坐标系转换**: 将惯导数据从车体坐标系转换到地理坐标系
        4. **GPS方向过滤**: 基于车身速度、方向稳定性、卫星数量过滤
        5. **数据融合**: 三种策略可选 (惯导为主/自适应融合/GPS为主)
        6. **异常检测**: 检测惯导静止但GPS移动的异常情况
        """)

else:
    st.info("👆 请上传CSV文件开始分析")

st.markdown("---")
st.markdown("© 2026 惯性导航与GPS数据融合可视化系统")