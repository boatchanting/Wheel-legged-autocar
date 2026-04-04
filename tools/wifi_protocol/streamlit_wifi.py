import streamlit as st
import threading
import socket
import struct
import time
import pandas as pd
import pydeck as pdk
import plotly.express as px
from datetime import datetime

# -------------------------------------------------------------------------
# 配置参数 (不改变原有配置)
# -------------------------------------------------------------------------
HOST_IP = '192.168.137.1'   # 本机作为 TCP Server 的 IP
HOST_PORT = 8086            # 监听端口
MAX_CHART_POINTS = 5000     # 前端图表保留的最大点数
REFRESH_RATE = 0.5          # UI 刷新频率（秒）

# -------------------------------------------------------------------------
# 协议解析配置
# -------------------------------------------------------------------------
PAYLOAD_SIZE = 76
FRAME_SIZE = PAYLOAD_SIZE + 6 
STRUCT_FMT = '<IffffHBBBBBBHHHHHHddbbffBfBf'
FIELD_NAMES = [
    'loop', 'nav_x', 'nav_y','vx_body', 'vy_body',
    'year', 'month', 'day', 'hour', 'minute', 'second',
    'state',
    'lat_deg', 'lat_cent', 'lat_sec',
    'lon_deg', 'lon_cent', 'lon_sec',
    'latitude', 'longitude',
    'ns', 'ew',
    'speed', 'direction',
    'ant_state', 'ant_direction',
    'sat_used', 'height'
]

# -------------------------------------------------------------------------
# Streamlit 全局数据存储 (利用 cache_resource 保持跨刷新状态)
# -------------------------------------------------------------------------
@st.cache_resource
def get_shared_state():
    return {
        "data_lock": threading.Lock(),
        "history_data": [],
        "server_running": False,
        "server_socket": None,
        "fps": 0,
        "frame_count": 0,
        "last_fps_time": time.time(),
        "client_connected": False
    }

shared_state = get_shared_state()

# -------------------------------------------------------------------------
# TCP 服务器线程逻辑
# -------------------------------------------------------------------------
def tcp_server_thread():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    # 设置超时以便能优雅退出
    server_socket.settimeout(1.0) 
    
    try:
        server_socket.bind((HOST_IP, HOST_PORT))
        server_socket.listen(1)
        shared_state["server_socket"] = server_socket
    except Exception as e:
        print(f"绑定失败: {e}")
        shared_state["server_running"] = False
        return

    while shared_state["server_running"]:
        try:
            conn, addr = server_socket.accept()
            shared_state["client_connected"] = True
            conn.settimeout(1.0)
            raw_buffer = bytearray()
            
            while shared_state["server_running"]:
                try:
                    chunk = conn.recv(1024)
                    if not chunk:
                        break 
                    
                    raw_buffer.extend(chunk)
                    
                    # 处理粘包
                    while len(raw_buffer) >= FRAME_SIZE:
                        if raw_buffer[0] != 0x5A or raw_buffer[1] != 0xA5:
                            del raw_buffer[0]
                            continue
                        
                        payload_len = raw_buffer[3]
                        calc_sum = sum(raw_buffer[0 : FRAME_SIZE-2]) & 0xFF
                        recv_sum = raw_buffer[FRAME_SIZE-2]
                        
                        if calc_sum == recv_sum and raw_buffer[FRAME_SIZE-1] == 0xED:
                            payload = raw_buffer[4 : 4+PAYLOAD_SIZE]
                            parse_and_store(payload)
                            del raw_buffer[0:FRAME_SIZE]
                        else:
                            del raw_buffer[0]
                            
                except socket.timeout:
                    continue
                except Exception as e:
                    break
                    
            conn.close()
            shared_state["client_connected"] = False
            
        except socket.timeout:
            continue
        except Exception as e:
            break

    # 关闭清理
    if shared_state["server_socket"]:
        shared_state["server_socket"].close()
        shared_state["server_socket"] = None

def parse_and_store(payload_bytes):
    try:
        unpacked = struct.unpack(STRUCT_FMT, payload_bytes)
        data_dict = dict(zip(FIELD_NAMES, unpacked))
        time_str = f"{data_dict['hour']:02d}:{data_dict['minute']:02d}:{data_dict['second']:02d}"
        data_dict['time_str'] = time_str

        with shared_state["data_lock"]:
            shared_state["history_data"].append(data_dict)
            # 限制内存最大点数
            if len(shared_state["history_data"]) > MAX_CHART_POINTS:
                shared_state["history_data"].pop(0)
                
            # 计算帧率 (FPS)
            shared_state["frame_count"] += 1
            now = time.time()
            if now - shared_state["last_fps_time"] >= 1.0:
                shared_state["fps"] = shared_state["frame_count"]
                shared_state["frame_count"] = 0
                shared_state["last_fps_time"] = now

    except Exception as e:
        pass

# -------------------------------------------------------------------------
# 控制函数
# -------------------------------------------------------------------------
def start_server():
    if not shared_state["server_running"]:
        shared_state["server_running"] = True
        t = threading.Thread(target=tcp_server_thread, daemon=True)
        t.start()

def stop_server():
    shared_state["server_running"] = False
    shared_state["client_connected"] = False

def clear_data():
    with shared_state["data_lock"]:
        shared_state["history_data"].clear()
        shared_state["fps"] = 0
        shared_state["frame_count"] = 0

# -------------------------------------------------------------------------
# Streamlit UI 构建
# -------------------------------------------------------------------------
st.set_page_config(page_title="GNSS/惯导 分析仪", layout="wide")

st.title("🛰️ GNSS & 惯性导航 实时监控平台")

# -- 侧边栏控制面板 --
with st.sidebar:
    st.header("⚙️ 设备控制")
    
    status_color = "🟢" if shared_state["server_running"] else "🔴"
    client_status = "已连接设备" if shared_state["client_connected"] else "等待连接..."
    st.markdown(f"**服务状态:** {status_color} ({HOST_IP}:{HOST_PORT})")
    if shared_state["server_running"]:
        st.markdown(f"**设备状态:** {client_status}")
    
    col1, col2 = st.columns(2)
    with col1:
        if st.button("▶️ 连接(监听)"):
            start_server()
            st.rerun()
    with col2:
        if st.button("⏹️ 断开连接"):
            stop_server()
            st.rerun()
            
    st.divider()
    
    # 状态数据展示
    st.metric("📶 接收帧率 (FPS)", shared_state["fps"])
    
    # 数据集大小
    with shared_state["data_lock"]:
        data_len = len(shared_state["history_data"])
    st.metric("📊 当前缓存点数", data_len)
    
    st.button("🗑️ 清除所有数据", on_click=clear_data)
    
    # 保存 CSV
    if data_len > 0:
        with shared_state["data_lock"]:
            df_export = pd.DataFrame(shared_state["history_data"])
        csv_data = df_export.to_csv(index=False).encode('utf-8-sig')
        file_name = f"gnss_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
        st.download_button("💾 导出为 CSV", data=csv_data, file_name=file_name, mime="text/csv")

# -- 主体界面 --
# 提取用于绘图的数据
df = pd.DataFrame()
with shared_state["data_lock"]:
    if data_len > 0:
        df = pd.DataFrame(shared_state["history_data"])

# 顶部指标卡
if not df.empty:
    latest = df.iloc[-1]
    m1, m2, m3, m4 = st.columns(4)
    m1.metric("🛰️ 卫星数量", int(latest['sat_used']))
    m2.metric("🚗 速度 (m/s)", f"{latest['speed']:.2f}")
    m3.metric("🧭 航向角", f"{latest['direction']:.2f}°")
    m4.metric("🏔️ 高度 (m)", f"{latest['height']:.2f}")

# 图表区域 (左: GNSS 地图, 右: 惯导平面图)
col_left, col_right = st.columns(2)

with col_left:
    st.subheader("🌍 GNSS 轨迹 (OpenStreetMap)")
    if not df.empty and 'latitude' in df.columns and 'longitude' in df.columns:
        # 获取最新的经纬度做中心点
        center_lat = df['latitude'].iloc[-1]
        center_lon = df['longitude'].iloc[-1]
        
        # 构建轨迹线数据
        path_data = [{"path": df[['longitude', 'latitude']].values.tolist()}]
        
        # PyDeck 配置
        view_state = pdk.ViewState(latitude=center_lat, longitude=center_lon, zoom=18, pitch=0)
        layer = pdk.Layer(
            "PathLayer",
            data=path_data,
            get_path="path",
            get_color=[255, 0, 0], # 红色轨迹
            width_scale=1,
            width_min_pixels=3,
        )
        
        # 使用 OpenStreetMap 风格的底图
        st.pydeck_chart(pdk.Deck(
            map_style="light", # 默认 light 会使用 Carto (基于OSM)，清晰度高且支持cm级呈现
            initial_view_state=view_state,
            layers=[layer],
            tooltip=True
        ))
    else:
        st.info("暂无 GNSS 数据...")

with col_right:
    st.subheader("📈 惯性导航相对轨迹")
    if not df.empty and 'nav_x' in df.columns and 'nav_y' in df.columns:
        # 使用 Plotly 绘制 X/Y 平面轨迹，并锁定长宽比例 (1:1) 防止形变
        fig = px.line(df, x="nav_x", y="nav_y", markers=True)
        fig.update_traces(marker=dict(size=3), line=dict(width=2))
        fig.update_layout(
            yaxis=dict(scaleanchor="x", scaleratio=1), # 保持1:1物理比例
            margin=dict(l=0, r=0, t=30, b=0),
            height=400
        )
        st.plotly_chart(fig, use_container_width=True)
    else:
        st.info("暂无惯导数据...")

# -- 实时刷新控制 --
# 仅在服务运行时自动刷新页面
if shared_state["server_running"]:
    time.sleep(REFRESH_RATE)
    st.rerun()