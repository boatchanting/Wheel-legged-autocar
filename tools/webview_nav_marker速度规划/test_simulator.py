import socket
import struct
import time
import math
import random
import sys

# Target server config - local testing should prioritize localhost (127.0.0.1)
DEFAULT_IPs = ["127.0.0.1", "192.168.137.1"]
PORT = 8086

FRAME_HEAD1 = 0x5A
FRAME_HEAD2 = 0xA5
FRAME_TAIL = 0xED
CMD_TELEMETRY = 0x01

# Struct format for V2 (86 bytes)
STRUCT_FMT_V2 = "<IffffHBBBBBBHHHHHHddbbffBfBfffBB"

def build_frame(payload):
    payload_len = len(payload)
    frame = bytearray([FRAME_HEAD1, FRAME_HEAD2, CMD_TELEMETRY, payload_len])
    frame.extend(payload)
    check_sum = sum(frame) & 0xFF
    frame.append(check_sum)
    frame.append(FRAME_TAIL)
    return bytes(frame)

def run_simulator():
    s = None
    connected = False
    
    # Try connecting to available IPs
    for ip in DEFAULT_IPs:
        try:
            print(f"正在尝试连接到 {ip}:{PORT}...")
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(2.0)
            s.connect((ip, PORT))
            
            # Send an initial test frame to verify the socket is open and handles our protocol
            # (If it's another service like InfluxDB, it will immediately close the connection upon receiving this)
            dummy_payload = struct.pack(STRUCT_FMT_V2, *([0]*30 + [0, 0]))
            test_frame = build_frame(dummy_payload)
            s.sendall(test_frame)
            
            # Perform a quick non-blocking check to see if the server closed the socket
            s.setblocking(False)
            try:
                data = s.recv(1)
                if len(data) == 0:
                    # Connection closed by peer
                    raise ConnectionResetError("服务器关闭了连接")
            except BlockingIOError:
                # No data to read, meaning connection is still open and healthy
                pass
            
            s.setblocking(True)
            target_ip = ip
            print(f"成功连接并验证至 {ip}:{PORT}!")
            connected = True
            break
        except Exception as e:
            print(f"连接或测试 {ip}:{PORT} 失败: {e}")
            if s:
                s.close()
            continue

    if not connected:
        print("\n[错误] 无法建立与上位机主程序的有效连接。")
        print("请检查：")
        print("1. 上位机 (nav_marker_host.py) 是否已经在运行？")
        print("2. 端口 8086 是否被其他程序（如 InfluxDB 数据库）占用？")
        print("   如果被占用，您可以尝试在 nav_marker_host.py 和本脚本中修改 PORT 变量。")
        sys.exit(1)

    s.settimeout(None) # Set to blocking
    loop = 0
    start_time = time.time()
    
    # Trajectory center and radius
    cx, cy = 2000.0, 2000.0
    radius = 1500.0

    print("开始注入模拟数据（包含变化的 Heading 和环形轨迹）...")
    print("按下 Ctrl+C 停止注入。")

    try:
        while True:
            # Generate simulated values
            elapsed = time.time() - start_time
            
            # Straight line trajectory (moving along X axis)
            speed = 1000.0 # 1000 mm/s
            nav_x = cx + elapsed * speed
            nav_y = cy
            
            vx = speed
            vy = 0.0
            
            # Base heading is constant (e.g. 90 degrees if moving along positive X)
            base_heading = 90.0
            
            # Simulate interference: base noise + occasional random spikes
            noise = random.normalvariate(0, 1.5) # Gaussian noise (std_dev = 1.5 deg)
            if random.random() < 0.05: # 5% chance of a larger spike
                noise += random.uniform(-30, 30)
                
            heading = base_heading + noise
            heading = (heading + 360.0) % 360.0

            relative_yaw = (heading - 180.0) % 360.0 - 180.0

            # Pack struct
            payload_data = struct.pack(
                STRUCT_FMT_V2,
                loop,                   # loop (uint32)
                nav_x,                  # nav_x (float)
                nav_y,                  # nav_y (float)
                vx,                     # vx_body (float)
                vy,                     # vy_body (float)
                2026,                   # year (uint16)
                7,                      # month (uint8)
                6,                      # day (uint8)
                22,                     # hour (uint8)
                0,                      # minute (uint8)
                0,                      # second (uint8)
                2,                      # state (uint8)
                0, 0, 0,                # lat_deg, lat_cent, lat_sec (uint16)
                0, 0, 0,                # lon_deg, lon_cent, lon_sec (uint16)
                31.23,                  # latitude (double)
                121.47,                 # longitude (double)
                1,                      # ns (int8)
                1,                      # ew (int8)
                speed,                  # speed (float)
                heading,                # direction (float)
                1,                      # ant_state (uint8)
                heading,                # ant_direction (float)
                12,                     # sat_used (uint8)
                25.0,                   # height (float)
                heading,                # heading (float)
                relative_yaw,           # relative_yaw (float)
                0,                      # mark_trigger (uint8)
                0                       # point_type (uint8)
            )

            frame = build_frame(payload_data)
            s.sendall(frame)
            
            loop += 1
            time.sleep(0.05) # 20Hz refresh rate
    except KeyboardInterrupt:
        print("\n模拟数据注入停止。")
    except Exception as e:
        print(f"\n发送数据时发生错误: {e}")
    finally:
        s.close()

if __name__ == "__main__":
    run_simulator()
