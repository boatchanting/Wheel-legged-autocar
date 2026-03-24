import webview
import threading
import socket
import struct
import json
import time
import csv
import os

# -------------------------------------------------------------------------
# 配置参数
# -------------------------------------------------------------------------
HOST_IP = '192.168.137.1'   # 本机作为 TCP Server 的 IP
HOST_PORT = 8086            # 监听端口
MAX_CHART_POINTS = 5000     # 前端图表保留的最大点数

# -------------------------------------------------------------------------
# 协议解析配置 (修正版)
# -------------------------------------------------------------------------
# 修正说明：
# 1. PAYLOAD_SIZE 修正为 76 字节
# 2. STRUCT_FMT 修正为 <IffffHBBBBBBHHHHHHddbbffBfBf
#    差异在于：时间+状态共6个u8(BBBBBB)，整型经纬度共6个u16(HHHHHH)

CMD_FIXED_PACKET = 0x01
CMD_CUSTOM_PACKET = 0x11
PAYLOAD_SIZE = 76

# Python struct 格式化字符串 (严格对应 C 语言发送顺序)
# < : 小端模式
# I : u32 (loop) -> 4
# f : float (nav_x, nav_y) -> 4+4
# H : u16 (year) -> 2
# B : u8 (month) -> 1
# B : u8 (day) -> 1
# B : u8 (hour) -> 1
# B : u8 (minute) -> 1
# B : u8 (second) -> 1
# B : u8 (state) -> 1  <-- 之前这里少算了一个B，或者与后面的H混淆了
# H : u16 (lat_deg) -> 2
# H : u16 (lat_cent) -> 2
# H : u16 (lat_sec) -> 2
# H : u16 (lon_deg) -> 2
# H : u16 (lon_cent) -> 2
# H : u16 (lon_sec) -> 2
# d : double (lat) -> 8
# d : double (lon) -> 8
# b : i8 (ns) -> 1
# b : i8 (ew) -> 1
# f : float (speed) -> 4
# f : float (dir) -> 4
# B : u8 (ant_state) -> 1
# f : float (ant_dir) -> 4
# B : u8 (sat_used) -> 1
# f : float (height) -> 4
# 总计：4+8+2+6+12+16+2+8+5+5 = 68 字节
STRUCT_FMT = '<IffffHBBBBBBHHHHHHddbbffBfBf'

# 字段名称映射 (必须与 STRUCT_FMT 解析出的 26 个变量一一对应)
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
    0: ('<B', 1), 1: ('<b', 1), 2: ('<H', 2), 3: ('<h', 2),
    4: ('<I', 4), 5: ('<i', 4), 6: ('<f', 4), 7: ('<d', 8)
}

CHANNEL_NAME_MAP = {
    1: 'loop', 2: 'nav_x', 3: 'nav_y', 4: 'vx_body', 5: 'vy_body',
    6: 'year', 7: 'month', 8: 'day', 9: 'hour', 10: 'minute', 11: 'second',
    12: 'state', 13: 'lat_deg', 14: 'lat_cent', 15: 'lat_sec',
    16: 'lon_deg', 17: 'lon_cent', 18: 'lon_sec', 19: 'latitude', 20: 'longitude',
    21: 'ns', 22: 'ew', 23: 'speed', 24: 'direction', 25: 'ant_state',
    26: 'ant_direction', 27: 'sat_used', 28: 'height'
}

# -------------------------------------------------------------------------
# 全局数据存储
# -------------------------------------------------------------------------
data_lock = threading.Lock()
all_history_data = []  
new_data_buffer = []   

# -------------------------------------------------------------------------
# TCP 服务器线程
# -------------------------------------------------------------------------
def tcp_server_thread():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    try:
        server_socket.bind((HOST_IP, HOST_PORT))
        server_socket.listen(1)
        print(f"[TCP] Listening on {HOST_IP}:{HOST_PORT}...")
    except Exception as e:
        print(f"[Error] Bind failed: {e}")
        return

    while True:
        try:
            conn, addr = server_socket.accept()
            print(f"[TCP] Connected by {addr}")
            
            raw_buffer = bytearray()
            
            while True:
                chunk = conn.recv(1024)
                if not chunk:
                    break 
                
                raw_buffer.extend(chunk)
                
                # 处理粘包 + 变长帧
                while True:
                    if len(raw_buffer) < 6:
                        break
                    # 1. 寻找帧头 0x5A 0xA5
                    if raw_buffer[0] != 0x5A or raw_buffer[1] != 0xA5:
                        del raw_buffer[0]
                        continue

                    payload_len = raw_buffer[3]
                    frame_size = payload_len + 6
                    if len(raw_buffer) < frame_size:
                        break

                    # 3. 校验和
                    frame = raw_buffer[:frame_size]
                    calc_sum = sum(frame[0 : frame_size-2]) & 0xFF
                    recv_sum = frame[frame_size-2]

                    if calc_sum == recv_sum and frame[frame_size-1] == 0xED:
                        payload = frame[4 : 4+payload_len]
                        parse_and_store(frame[2], payload)
                        # 移除已处理的帧
                        del raw_buffer[0:frame_size]
                    else:
                        # print("Checksum error")
                        del raw_buffer[0]
                        
        except Exception as e:
            print(f"[TCP] Connection error: {e}")
            time.sleep(1)

def parse_and_store(cmd, payload_bytes):
    try:
        if cmd == CMD_FIXED_PACKET:
            unpacked = struct.unpack(STRUCT_FMT, payload_bytes)
            data_dict = dict(zip(FIELD_NAMES, unpacked))
            time_str = f"{data_dict['hour']:02d}:{data_dict['minute']:02d}:{data_dict['second']:02d}"
            data_dict['time_str'] = time_str
            data_dict['cmd'] = cmd
            data_dict['profile_id'] = -1
        elif cmd == CMD_CUSTOM_PACKET:
            data_dict = {'cmd': cmd}
            if len(payload_bytes) < 3:
                return
            data_dict['profile_id'] = payload_bytes[0]
            data_dict['seq'] = payload_bytes[1]
            ch_count = payload_bytes[2]
            data_dict['ch_count'] = ch_count
            pos = 3
            parsed = 0
            while pos + 2 <= len(payload_bytes) and parsed < ch_count:
                ch_id = payload_bytes[pos]
                val_type = payload_bytes[pos + 1]
                pos += 2
                fmt_info = VALUE_FMT_MAP.get(val_type)
                if fmt_info is None:
                    break
                fmt, size = fmt_info
                if pos + size > len(payload_bytes):
                    break
                value = struct.unpack(fmt, payload_bytes[pos:pos + size])[0]
                pos += size
                parsed += 1
                data_dict[CHANNEL_NAME_MAP.get(ch_id, f'ch_{ch_id}')] = value
        else:
            return

        with data_lock:
            all_history_data.append(data_dict)
            new_data_buffer.append(data_dict)
            
    except struct.error as e:
        print(f"[Parse Error] {e} | BufLen: {len(payload_bytes)}")
    except Exception as e:
        print(f"[Error] {e}")

# -------------------------------------------------------------------------
# PyWebview API
# -------------------------------------------------------------------------
class Api:
    def get_new_data(self):
        with data_lock:
            if not new_data_buffer:
                return []
            data = list(new_data_buffer)
            new_data_buffer.clear()
            return data

    def save_csv(self):
        filename = f"gnss_data_{time.strftime('%Y%m%d_%H%M%S')}.csv"
        filepath = os.path.abspath(filename)
        try:
            with data_lock:
                if not all_history_data:
                    return {"success": False, "msg": "没有数据可保存"}
                
                # 重新组织表头顺序（可选）
                keys = list(all_history_data[0].keys())
                
                with open(filepath, 'w', newline='', encoding='utf-8-sig') as f:
                    writer = csv.DictWriter(f, fieldnames=keys)
                    writer.writeheader()
                    writer.writerows(all_history_data)
            
            return {"success": True, "msg": f"保存成功: {filepath}"}
        except Exception as e:
            return {"success": False, "msg": f"保存失败: {str(e)}"}

    def clear_data(self):
        with data_lock:
            all_history_data.clear()
            new_data_buffer.clear()
        return "数据已清除"

# -------------------------------------------------------------------------
# 主程序
# -------------------------------------------------------------------------
if __name__ == '__main__':
    # 启动 TCP 线程
    t = threading.Thread(target=tcp_server_thread, daemon=True)
    t.start()
    
    # 获取 HTML 文件的绝对路径
    # 获取当前脚本所在的目录
    current_dir = os.path.dirname(os.path.abspath(__file__))
    # 构建 index.html 的绝对路径
    html_path = os.path.join(current_dir, 'navigation.html')
    if not os.path.exists(html_path):
        print("错误：找不到 index.html 文件！")
        exit()

    api = Api()
    window = webview.create_window(
        'GNSS/惯导 协议分析仪', 
        url=html_path,  # 加载本地文件
        js_api=api, 
        width=1280, 
        height=800
    )
    webview.start(debug=False)
