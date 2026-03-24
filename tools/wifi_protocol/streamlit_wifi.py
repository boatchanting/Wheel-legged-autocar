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
CMD_FIXED_PACKET = 0x01
CMD_CUSTOM_PACKET = 0x11

PAYLOAD_SIZE = 76
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

VALUE_FMT_MAP = {
    0: ('<B', 1),   # WIFI_VAL_U8
    1: ('<b', 1),   # WIFI_VAL_I8
    2: ('<H', 2),   # WIFI_VAL_U16
    3: ('<h', 2),   # WIFI_VAL_I16
    4: ('<I', 4),   # WIFI_VAL_U32
    5: ('<i', 4),   # WIFI_VAL_I32
    6: ('<f', 4),   # WIFI_VAL_FLOAT
    7: ('<d', 8),   # WIFI_VAL_DOUBLE
}

CHANNEL_NAME_MAP = {
    1: 'loop',
    2: 'nav_x',
    3: 'nav_y',
    4: 'vx_body',
    5: 'vy_body',
    6: 'year',
    7: 'month',
    8: 'day',
    9: 'hour',
    10: 'minute',
    11: 'second',
    12: 'state',
    13: 'lat_deg',
    14: 'lat_cent',
    15: 'lat_sec',
    16: 'lon_deg',
    17: 'lon_cent',
    18: 'lon_sec',
    19: 'latitude',
    20: 'longitude',
    21: 'ns',
    22: 'ew',
    23: 'speed',
    24: 'direction',
    25: 'ant_state',
    26: 'ant_direction',
    27: 'sat_used',
    28: 'height',
}

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
        "client_connected": False,
        "last_cmd": None,
        "last_profile_id": None,
        "parse_errors": 0
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
                    
                    # 处理粘包 + 变长帧
                    while True:
                        if len(raw_buffer) < 6:
                            break
                        if raw_buffer[0] != 0x5A or raw_buffer[1] != 0xA5:
                            del raw_buffer[0]
                            continue

                        payload_len = raw_buffer[3]
                        frame_size = payload_len + 6  # 2头 + cmd + len + payload + sum + tail
                        if len(raw_buffer) < frame_size:
                            break

                        frame = raw_buffer[:frame_size]
                        calc_sum = sum(frame[0:frame_size - 2]) & 0xFF
                        recv_sum = frame[frame_size - 2]

                        if calc_sum == recv_sum and frame[frame_size - 1] == 0xED:
                            parse_and_store(frame[2], frame[4:4 + payload_len])
                            del raw_buffer[0:frame_size]
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

def parse_fixed_packet(payload_bytes):
    if len(payload_bytes) != PAYLOAD_SIZE:
        raise ValueError(f"fixed packet payload size mismatch: {len(payload_bytes)} != {PAYLOAD_SIZE}")

    unpacked = struct.unpack(STRUCT_FMT, payload_bytes)
    data_dict = dict(zip(FIELD_NAMES, unpacked))
    data_dict['cmd'] = CMD_FIXED_PACKET
    data_dict['profile_id'] = -1
    data_dict['ch_count'] = len(FIELD_NAMES)
    time_str = f"{data_dict['hour']:02d}:{data_dict['minute']:02d}:{data_dict['second']:02d}"
    data_dict['time_str'] = time_str
    return data_dict


def parse_custom_packet(payload_bytes):
    if len(payload_bytes) < 3:
        raise ValueError("custom packet payload too short")

    profile_id = payload_bytes[0]
    seq = payload_bytes[1]
    ch_count = payload_bytes[2]
    pos = 3

    data_dict = {
        'cmd': CMD_CUSTOM_PACKET,
        'profile_id': profile_id,
        'seq': seq,
        'ch_count': ch_count,
    }

    parsed_channels = 0
    while pos + 2 <= len(payload_bytes) and parsed_channels < ch_count:
        ch_id = payload_bytes[pos]
        value_type = payload_bytes[pos + 1]
        pos += 2

        fmt_info = VALUE_FMT_MAP.get(value_type)
        if fmt_info is None:
            raise ValueError(f"unknown value type: {value_type}")
        fmt, size = fmt_info
        if pos + size > len(payload_bytes):
            raise ValueError("custom packet truncated")

        value = struct.unpack(fmt, payload_bytes[pos:pos + size])[0]
        pos += size
        parsed_channels += 1

        key_name = CHANNEL_NAME_MAP.get(ch_id, f'ch_{ch_id}')
        data_dict[key_name] = value
        data_dict[f'{key_name}_type'] = value_type

    # custom 包里如果有时间字段，就补 time_str，便于沿用 UI
    if all(k in data_dict for k in ('hour', 'minute', 'second')):
        data_dict['time_str'] = f"{int(data_dict['hour']):02d}:{int(data_dict['minute']):02d}:{int(data_dict['second']):02d}"

    return data_dict


def parse_and_store(cmd, payload_bytes):
    try:
        if cmd == CMD_FIXED_PACKET:
            data_dict = parse_fixed_packet(payload_bytes)
        elif cmd == CMD_CUSTOM_PACKET:
            data_dict = parse_custom_packet(payload_bytes)
        else:
            return

        with shared_state["data_lock"]:
            shared_state["history_data"].append(data_dict)
            shared_state["last_cmd"] = cmd
            shared_state["last_profile_id"] = data_dict.get('profile_id')
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
        with shared_state["data_lock"]:
            shared_state["parse_errors"] += 1

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
        shared_state["parse_errors"] = 0

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
    st.metric("🧩 最近协议", f"0x{shared_state['last_cmd']:02X}" if shared_state["last_cmd"] is not None else "-")
    st.metric("⚠️ 解析错误", shared_state["parse_errors"])
    st.metric("🗂️ 最近Profile", shared_state["last_profile_id"] if shared_state["last_profile_id"] is not None else "-")
    
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
    m1.metric("🛰️ 卫星数量", int(latest.get('sat_used', 0)))
    m2.metric("🚗 速度 (m/s)", f"{float(latest.get('speed', 0.0)):.2f}")
    m3.metric("🧭 航向角", f"{float(latest.get('direction', 0.0)):.2f}°")
    m4.metric("🏔️ 高度 (m)", f"{float(latest.get('height', 0.0)):.2f}")

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

st.subheader("🧪 最近一帧原始字段（支持自定义通道）")
if not df.empty:
    st.dataframe(df.tail(1).T, use_container_width=True)

# -- 实时刷新控制 --
# 仅在服务运行时自动刷新页面
if shared_state["server_running"]:
    time.sleep(REFRESH_RATE)
    st.rerun()
