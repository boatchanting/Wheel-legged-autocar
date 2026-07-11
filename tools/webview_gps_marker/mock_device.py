import socket
import struct
import time
import math
import random

HOST = '127.0.0.1'
PORT = 8086

def checksum(data):
    s = 0
    for b in data:
        s = (s + b) & 0xFF
    return s

def run_mock():
    print("请选择模拟轨迹的半径尺寸：")
    print("1. 3米 (3000mm) - 默认")
    print("2. 5米 (5000mm)")
    print("3. 10米 (10000mm)")
    choice = input("请输入 1, 2 或 3: ").strip()
    if choice == '2':
        radius = 5000.0
    elif choice == '3':
        radius = 10000.0
    else:
        radius = 3000.0

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect((HOST, PORT))
        print(f"Connected to {HOST}:{PORT}")
    except ConnectionRefusedError:
        print(f"Make sure gps_marker_host.py is running on port {PORT}")
        return

    loop = 0
    jump_count = 0
    
    # Fusion state
    offset_x = 0.0
    offset_y = 0.0
    
    last_ground_x = radius
    last_ground_y = 0.0
    
    ground_x = last_ground_x
    ground_y = last_ground_y

    virtual_t = 0.0

    print("Sending mock data. Press Ctrl+C to stop.")
    try:
        while True:
            t = loop * 0.01  # Global wall clock time (100Hz)
            
            # Physics engine time: pauses during ZUPT
            if not (15.0 <= t <= 18.0):
                virtual_t += 0.01
                
            # 1. Base circular trajectory
            angle = virtual_t * 0.5  # rad
            base_x = radius * math.cos(angle)
            base_y = radius * math.sin(angle)
            
            # INS naturally drifts based on global time (100Hz)
            drift_x = t * 10.0
            drift_y = t * -5.0
            ins_x = base_x + drift_x
            ins_y = base_y + drift_y
            
            # Default state
            k_pos = 0.02
            special = 0
            zupt = 0
            
            # --- Scenario Triggers ---
            
            # [T=5~10s]: Special Element
            if 5.0 <= t <= 10.0:
                special = 1
                k_pos = 0.0
                
            # [T=15~18s]: ZUPT (stop)
            elif 15.0 <= t <= 18.0:
                zupt = 1
                k_pos = 0.0
                # Vehicle physical movement is stopped because virtual_t is paused above.
                # However, INS still drifts, but we'll freeze the INS coordinates too to fully mock ZUPT hardware behavior
                ins_x = base_x + 150.0  
                ins_y = base_y - 75.0
                
            # GPS Update (10Hz = every 10 loops)
            if loop % 10 == 0:
                ground_x = base_x + random.uniform(-100, 100)
                ground_y = base_y + random.uniform(-100, 100)
                
                # [T=25s]: GPS Jump (10Hz)
                if 25.0 <= t <= 25.1:
                    ground_x += 2000.0  # 2m jump
                    ground_y += 2000.0
                
                # Jump reject logic (1.5m threshold)
                dx_gps = ground_x - last_ground_x
                dy_gps = ground_y - last_ground_y
                delta_gps = math.hypot(dx_gps, dy_gps)
                
                if delta_gps > 1500.0:
                    jump_count += 1
                else:
                    # Execute Complementary Filter (10Hz)
                    last_ground_x = ground_x
                    last_ground_y = ground_y
                    
                    fuse_x = ins_x + offset_x
                    fuse_y = ins_y + offset_y
                    
                    err_x = ground_x - fuse_x
                    err_y = ground_y - fuse_y
                    
                    offset_x += k_pos * err_x
                    offset_y += k_pos * err_y
            
            # Calculate final fusion output (100Hz)
            fuse_x = ins_x + offset_x
            fuse_y = ins_y + offset_y
            
            # Pack payload
            payload = struct.pack(
                "<IffffHBBBBBBHHHHHHddbbffBfBfffBBffBBffffffffffBBB",
                loop,                   # loop
                ins_x, ins_y, 0.0, 0.0, # nav_x, nav_y, vx, vy
                2026, 7, 11, 12, 0, 0,  # time
                1,                      # state
                0,0,0, 0,0,0,           # lat/lon deg
                0.0, 0.0,               # lat/lon double
                0, 0,                   # ns, ew
                0.0, 0.0,               # speed, dir
                0, 0.0,                 # ant_state, ant_dir
                15,                     # sat_used (15 = good)
                0.0, 0.0, 0.0,          # height, hdg, rel_yaw
                0, 0,                   # mark_trigger, pt_type
                0.0, 0.0, 1, 1,         # gps_x, gps_y, valid, origin
                fuse_x, fuse_y, 0.0, offset_x, offset_y,  # fuse (5)
                ins_x, ins_y, ground_x, ground_y,         # traj (4)
                k_pos, jump_count, zupt, special          # status (4)
            )
            
            # Build frame
            frame = bytearray([0x5A, 0xA5, 0x01, len(payload)])
            frame.extend(payload)
            frame.append(checksum(frame))
            frame.append(0xED)
            
            s.sendall(frame)
            
            if loop % 100 == 0:
                print(f"Sent loop {loop}, k_pos={k_pos}, jump={jump_count}")
            
            loop += 1
            time.sleep(0.01)  # 100Hz
            
    except KeyboardInterrupt:
        print("\nMock stopped.")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        s.close()

if __name__ == "__main__":
    run_mock()
