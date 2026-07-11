import socket
import struct
import time
import math
import random
import os

HOST = '127.0.0.1'
PORT = 8086

def checksum(data):
    s = 0
    for b in data:
        s = (s + b) & 0xFF
    return s

def normalize_angle_180(angle):
    return (angle + 180.0) % 360.0 - 180.0

def run_mock():
    print("请选择发车角获取模式：")
    print("1. 静态多帧磁力计滤波 (发车前2秒) - Mode 1")
    print("2. 动态轨迹最小二乘拟合 (发车后5秒) - Mode 2")
    mode_choice = input("请输入 1 或 2 (默认2): ").strip()
    mode = 1 if mode_choice == '1' else 2

    print("请选择模拟轨迹的半径尺寸：")
    print("1. 3米 (3000mm) - 默认")
    print("2. 5米 (5000mm)")
    print("3. 10米 (10000mm)")
    radius_choice = input("请输入 1, 2 或 3: ").strip()
    if radius_choice == '2':
        radius = 5000.0
    elif radius_choice == '3':
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

    # Initialize log file (overwrite previous)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    log_path = os.path.join(script_dir, "mock_log.csv")
    log_file = open(log_path, 'w', encoding='utf-8')
    log_file.write("loop,time_s,ins_x,ins_y,ground_x,ground_y,fuse_x,fuse_y,offset_x,offset_y,k_pos,jump_count,zupt,special,frame_hex\n")
    print(f"Logging to: {log_path}")

    loop = 0
    jump_count = 0
    consecutive_jump = 0
    fast_recovery_frames = 0
    
    # Fusion state
    offset_x = 0.0
    offset_y = 0.0
    
    # 模拟真实世界
    actual_heading_deg = 45.0  # 假设车头实际朝东北方向发车
    actual_heading_rad = math.radians(actual_heading_deg)
    
    # 初始化状态
    track_base_yaw = 0.0 # 初始不知道发车角
    heading_calculating = True
    
    # Mode 1 状态
    heading_sum_cos = 0.0
    heading_sum_sin = 0.0
    
    # Mode 2 状态
    ls_gps_x = []
    ls_gps_y = []
    ls_ins_x = []
    ls_ins_y = []
    LS_SAMPLES = 50
    
    just_finished_heading = False

    # 卫星数状态
    sat_used = 15
    next_sat_change = random.randint(100, 200)  # 1~2秒后首次跳变

    # 上一次有效投影坐标
    last_ground_x = 0.0
    last_ground_y = 0.0
    
    ground_x = 0.0
    ground_y = 0.0
    fuse_x = 0.0
    fuse_y = 0.0

    virtual_t = 0.0

    print(f"Sending mock data. Actual Heading = {actual_heading_deg} deg, Mode = {mode}. Press Ctrl+C to stop.")
    try:
        while True:
            t = loop * 0.01  # Global wall clock time (100Hz)

            # 卫星数随机跳变
            if loop >= next_sat_change:
                sat_used = random.randint(5, 20)
                next_sat_change = loop + random.randint(100, 200)

            # 车辆运动逻辑
            if mode == 1 and t < 2.0:
                # Mode 1: 前 2 秒车辆静止，用来采样磁力计
                pass
            elif not (15.0 <= t <= 18.0):
                # 避开 ZUPT 停顿区，正常行驶
                virtual_t += 0.01
                
            # 1. 真实地球系轨迹 (GPS)
            angle = virtual_t * 0.5  # rad
            earth_x = radius * math.cos(angle)
            earth_y = radius * math.sin(angle)
            
            # 2. 真实本地赛道系轨迹 (INS)
            local_x_true = -earth_x * math.sin(actual_heading_rad) - earth_y * math.cos(actual_heading_rad)
            local_y_true = earth_x * math.cos(actual_heading_rad) - earth_y * math.sin(actual_heading_rad)
            
            # INS naturally drifts based on global time (100Hz)
            drift_x = virtual_t * 10.0
            drift_y = virtual_t * -5.0
            ins_x = local_x_true + drift_x
            ins_y = local_y_true + drift_y
            
            # Default state
            k_pos = 0.02
            if fast_recovery_frames > 0:
                k_pos = 0.20
                
            special = 0
            zupt = 0
            
            # [T=15~18s]: ZUPT (stop)
            if 15.0 <= t <= 18.0:
                zupt = 1
                k_pos = 0.0
                
            # --- Mode 1: 静态多帧磁力计滤波 (100Hz) ---
            if mode == 1:
                if t < 2.0:
                    k_pos = 0.0 # 静止时屏蔽融合
                    # 模拟磁力计静态噪声 ±0.5度
                    mag_reading = actual_heading_deg + random.uniform(-0.5, 0.5)
                    mag_rad = math.radians(mag_reading)
                    heading_sum_cos += math.cos(mag_rad)
                    heading_sum_sin += math.sin(mag_rad)
                elif loop == 200: # t == 2.0 刚好结束
                    track_base_yaw = math.degrees(math.atan2(heading_sum_sin, heading_sum_cos))
                    track_base_yaw = normalize_angle_180(track_base_yaw)
                    print(f"[{loop}] Mode 1 Finished: Calculated track_base_yaw = {track_base_yaw:.3f} deg (Actual is {actual_heading_deg})")
                    heading_calculating = False
                    just_finished_heading = True
                
            # GPS Update (10Hz)
            if loop % 10 == 0:
                if fast_recovery_frames > 0:
                    fast_recovery_frames -= 1
                    
                # 添加 GPS 噪声（卫星数越少误差越大）
                if sat_used < 10:
                    gps_noise = 400.0
                elif sat_used < 15:
                    gps_noise = 200.0
                else:
                    gps_noise = 100.0
                gps_E = earth_x + random.uniform(-gps_noise, gps_noise)
                gps_N = earth_y + random.uniform(-gps_noise, gps_noise)
                
                # 模拟长时间漂移 (10s 到 13s)
                if 10.0 <= t <= 13.0:
                    gps_E += 3000.0
                    gps_N += 3000.0
                else:
                    # Random GPS jump (~2% chance)
                    if random.random() < 0.02 and not heading_calculating:
                        gps_E += random.uniform(-2000, 2000)
                        gps_N += random.uniform(-2000, 2000)
                
                # --- Mode 2: 动态推算发车角 ---
                if mode == 2 and heading_calculating:
                    k_pos = 0.0 # 推算期间屏蔽融合
                    if len(ls_gps_x) < LS_SAMPLES:
                        ls_gps_x.append(gps_E)
                        ls_gps_y.append(gps_N)
                        ls_ins_x.append(ins_x)
                        ls_ins_y.append(ins_y)
                    
                    if len(ls_gps_x) >= LS_SAMPLES:
                        # 执行最小二乘法拟合
                        mean_gps_x = sum(ls_gps_x) / LS_SAMPLES
                        mean_gps_y = sum(ls_gps_y) / LS_SAMPLES
                        mean_ins_x = sum(ls_ins_x) / LS_SAMPLES
                        mean_ins_y = sum(ls_ins_y) / LS_SAMPLES
                        
                        gps_num = sum((x - mean_gps_x) * (y - mean_gps_y) for x, y in zip(ls_gps_x, ls_gps_y))
                        gps_den = sum((x - mean_gps_x) ** 2 for x in ls_gps_x)
                        ins_num = sum((x - mean_ins_x) * (y - mean_ins_y) for x, y in zip(ls_ins_x, ls_ins_y))
                        ins_den = sum((x - mean_ins_x) ** 2 for x in ls_ins_x)
                        
                        if gps_den < 1e-3:
                            gps_angle_rad = math.pi/2.0 if ls_gps_y[-1] >= ls_gps_y[0] else -math.pi/2.0
                        else:
                            gps_angle_rad = math.atan(gps_num / gps_den)
                            if ls_gps_x[-1] < ls_gps_x[0]:
                                gps_angle_rad += -math.pi if gps_angle_rad > 0 else math.pi
                                
                        if ins_den < 1e-3:
                            ins_angle_rad = math.pi/2.0 if ls_ins_y[-1] >= ls_ins_y[0] else -math.pi/2.0
                        else:
                            ins_angle_rad = math.atan(ins_num / ins_den)
                            if ls_ins_x[-1] < ls_ins_x[0]:
                                ins_angle_rad += -math.pi if ins_angle_rad > 0 else math.pi
                                
                        rad = ins_angle_rad - gps_angle_rad - (math.pi / 2.0)
                        track_base_yaw = normalize_angle_180(math.degrees(rad))
                        
                        print(f"[{loop}] Mode 2 Finished: Calculated track_base_yaw = {track_base_yaw:.3f} deg (Actual is {actual_heading_deg})")
                        
                        heading_calculating = False
                        just_finished_heading = True
                        k_pos = 0.02 # 恢复融合
                
                # 若发车角计算完毕或不再计算，将地球系转为本地系
                if not heading_calculating:
                    yaw_rad = math.radians(track_base_yaw)
                    ground_x = -gps_E * math.sin(yaw_rad) - gps_N * math.cos(yaw_rad)
                    ground_y =  gps_E * math.cos(yaw_rad) - gps_N * math.sin(yaw_rad)
                    
                    # 使用当前惯导推算位置计算 Innovation
                    pred_fuse_x = ins_x + offset_x
                    pred_fuse_y = ins_y + offset_y
                    
                    dx_gps = ground_x - pred_fuse_x
                    dy_gps = ground_y - pred_fuse_y
                    delta_gps = math.hypot(dx_gps, dy_gps)
                    
                    if just_finished_heading:
                        just_finished_heading = False
                        delta_gps = 0.0
                        
                    is_rejected = False
                    if delta_gps > 1500.0:
                        jump_count += 1
                        consecutive_jump += 1
                        if consecutive_jump >= 50:
                            consecutive_jump = 0 # 强行接受新位置 (5秒超时兜底)
                            fast_recovery_frames = 30 # 开启 3 秒快速恢复期
                        else:
                            is_rejected = True
                    else:
                        consecutive_jump = 0
                        
                    if not is_rejected:
                        last_ground_x = ground_x
                        last_ground_y = ground_y
                        
                        fuse_x = pred_fuse_x
                        fuse_y = pred_fuse_y
                        
                        err_x = ground_x - fuse_x
                        err_y = ground_y - fuse_y
                        
                        offset_x += k_pos * err_x
                        offset_y += k_pos * err_y
            
            # Calculate final fusion output (100Hz)
            if heading_calculating:
                fuse_x = ins_x + offset_x
                fuse_y = ins_y + offset_y
            else:
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
                sat_used,               # sat_used (5~20)
                0.0, 0.0, 0.0,          # height, hdg, rel_yaw
                0, 0,                   # mark_trigger, pt_type
                ground_x, ground_y, 1, 1, # gps_x, gps_y, valid, origin
                fuse_x, fuse_y, 0.0, offset_x, offset_y,  # fuse (5)
                ins_x, ins_y, ground_x, ground_y,         # traj (4)
                k_pos, jump_count, zupt, special          # status (4)
            )
            
            # Build frame
            frame = bytearray([0x5A, 0xA5, 0x01, len(payload)])
            frame.extend(payload)
            frame.append(checksum(frame))
            frame.append(0xED)
            
            if loop % 5 == 0:
                s.sendall(frame)
                log_file.write(f"{loop},{t:.2f},{ins_x:.1f},{ins_y:.1f},{ground_x:.1f},{ground_y:.1f},{fuse_x:.1f},{fuse_y:.1f},{offset_x:.1f},{offset_y:.1f},{k_pos},{jump_count},{zupt},{special},{frame.hex()}\n")
            
            if loop % 100 == 0:
                print(f"Sent loop {loop}, k_pos={k_pos:.3f}, jump={jump_count}, yaw={track_base_yaw:.2f}")
            
            loop += 1
            time.sleep(0.01)  # 100Hz
            
    except KeyboardInterrupt:
        print("\nMock stopped.")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        log_file.close()
        s.close()

if __name__ == "__main__":
    run_mock()
