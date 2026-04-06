import pygame
import numpy as np
import math

# ============================================================================
#  1. PID 核心类
# ============================================================================
def float_constrain(val, min_val, max_val):
    if val > max_val: return max_val
    if val < min_val: return min_val
    return val

class PID_Param_t:
    def __init__(self, kp, ki, kd, max_out, max_i, comp):
        self.kp = float(kp)
        self.ki = float(ki)
        self.kd = float(kd)
        self.max_output = float(max_out) # 对于速度环，这限制了最大倾斜角度
        self.max_integral = float(max_i)
        self.compensation = float(comp)
        self.reset_data()

    def reset_data(self):
        self.error = 0.0
        self.last_error = 0.0
        self.prev_error = 0.0
        self.error_integral = 0.0  
        self.output = 0.0

# 初始化参数建议 (您可以手动在UI调整)
# 速度环输出的是角度，所以 max_out 不应太大 (例如 0.5 弧度 ≈ 28度)
pid_speed = PID_Param_t(kp=0.002, ki=0.0001, kd=0.0, max_out=0.4, max_i=0.1, comp=0.0)
# 角度环
pid_angle = PID_Param_t(kp=35.0, ki=0.0, kd=0.5, max_out=250.0, max_i=0.0, comp=0.0)
# 角速度环 (内环)
pid_gyro  = PID_Param_t(kp=2.0, ki=0.0, kd=0.0, max_out=255.0, max_i=0.0, comp=0.0)

target_speed_set = 0.0
global_debug_target_angle = 0.0 # 用于可视化：速度环要求的“腿”的角度

# ============================================================================
#  2. 控制算法
# ============================================================================

def Speed_Loop_Control(target_speed, actual_speed):
    """
    最外环：速度环
    输入：期望速度 vs 实际速度
    输出：期望的角度 (Target Angle) -> 即让“腿”倾斜
    """
    pid_speed.error = target_speed - actual_speed
    
    # 积分分离或限幅
    pid_speed.error_integral += pid_speed.error
    pid_speed.error_integral = float_constrain(pid_speed.error_integral, -pid_speed.max_integral, pid_speed.max_integral)
    
    pid_speed.output = (pid_speed.kp * pid_speed.error) + (pid_speed.ki * pid_speed.error_integral)
    
    # 【关键】：速度环的输出被限制在 max_output 内，防止要求小车倾斜过度而倒地
    pid_speed.output = float_constrain(pid_speed.output, -pid_speed.max_output, pid_speed.max_output)
    
    pid_speed.prev_error = pid_speed.last_error
    pid_speed.last_error = pid_speed.error
    
    return pid_speed.output 

def Angle_Loop_Control(speed_loop_output, actual_angle):
    """
    中间环：角度环
    输入：(机械零点 - 速度环输出) vs 实际角度
    输出：期望角速度 (Target Gyro)
    """
    global global_debug_target_angle
    
    # 【核心逻辑】：这里就是你说的“主动倾斜”。
    # 如果速度环输出正值（想加速），target_angle 变负（向前倾），利用重力分量加速。
    target_angle = pid_angle.compensation - speed_loop_output 
    global_debug_target_angle = target_angle # 保存用于绘图
    
    pid_angle.error = target_angle - actual_angle
    
    if pid_angle.ki != 0:
        pid_angle.error_integral += pid_angle.error
        pid_angle.error_integral = float_constrain(pid_angle.error_integral, -pid_angle.max_integral, pid_angle.max_integral)
        
    pid_angle.output = (pid_angle.kp * pid_angle.error) + \
                       (pid_angle.ki * pid_angle.error_integral) + \
                       (pid_angle.kd * (pid_angle.error - pid_angle.last_error))
                       
    pid_angle.output = float_constrain(pid_angle.output, -pid_angle.max_output, pid_angle.max_output)
    pid_angle.prev_error = pid_angle.last_error
    pid_angle.last_error = pid_angle.error
    
    # 这里通常返回负值，因为如果角度误差为正（后仰），我们需要向前加速（负力矩/速度）来纠正，
    # 或者取决于电机安装方向。模拟器中取反适配物理模型。
    return -pid_angle.output 

def Gyro_Loop_Control(angle_loop_output, actual_gyro):
    """
    最内环：角速度环
    输入：角度环输出 vs 实际角速度
    输出：PWM (电机电压/力)
    """
    pid_gyro.error = angle_loop_output - actual_gyro
    
    if pid_gyro.ki != 0:
        pid_gyro.error_integral += pid_gyro.error
        pid_gyro.error_integral = float_constrain(pid_gyro.error_integral, -pid_gyro.max_integral, pid_gyro.max_integral)
        
    pid_gyro.output = (pid_gyro.kp * pid_gyro.error) + \
                      (pid_gyro.ki * pid_gyro.error_integral) + \
                      (pid_gyro.kd * (pid_gyro.error - pid_gyro.last_error))
    
    # 死区/摩擦补偿
    if pid_gyro.output > 0: pid_gyro.output += pid_gyro.compensation
    elif pid_gyro.output < 0: pid_gyro.output -= pid_gyro.compensation
    
    pid_gyro.output = float_constrain(pid_gyro.output, -pid_gyro.max_output, pid_gyro.max_output)
    pid_gyro.prev_error = pid_gyro.last_error
    pid_gyro.last_error = pid_gyro.error
    return pid_gyro.output

# ============================================================================
#  3. UI 类定义 (保持不变)
# ============================================================================
pygame.init()
WIDTH, HEIGHT = 1400, 800 
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Cascade PID Simulator: Speed -> Angle -> Gyro")
clock = pygame.time.Clock()
font = pygame.font.SysFont("Courier New", 18, bold=True)
font_small = pygame.font.SysFont("Arial", 14)
font_tiny = pygame.font.SysFont("Arial", 12)

class InputBox:
    def __init__(self, x, y, w, h, obj, attr_name, label=""):
        self.rect = pygame.Rect(x, y, w, h)
        self.color = pygame.Color('lightskyblue3')
        self.text = str(getattr(obj, attr_name))
        self.obj = obj
        self.attr_name = attr_name
        self.label = label
        self.active = False
        self.txt_surf = font.render(self.text, True, pygame.Color('black'))

    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            if self.rect.collidepoint(event.pos): self.active = not self.active
            else: self.active = False
            self.color = pygame.Color('dodgerblue2') if self.active else pygame.Color('lightskyblue3')
        if event.type == pygame.KEYDOWN and self.active:
            if event.key == pygame.K_RETURN:
                self.apply_value()
                self.active = False
                self.color = pygame.Color('lightskyblue3')
            elif event.key == pygame.K_BACKSPACE: self.text = self.text[:-1]
            else: self.text += event.unicode
            self.txt_surf = font.render(self.text, True, pygame.Color('black'))

    def apply_value(self):
        try: setattr(self.obj, self.attr_name, float(self.text))
        except: self.text = str(getattr(self.obj, self.attr_name))
        self.txt_surf = font.render(self.text, True, pygame.Color('black'))

    def draw(self, screen):
        screen.blit(font_small.render(self.label, True, (50,50,50)), (self.rect.x - 70, self.rect.y + 5))
        pygame.draw.rect(screen, (255,255,255), self.rect)
        screen.blit(self.txt_surf, (self.rect.x+5, self.rect.y+5))
        pygame.draw.rect(screen, self.color, self.rect, 2)

class CheckBox:
    def __init__(self, x, y, label, color, checked=True):
        self.rect = pygame.Rect(x, y, 20, 20)
        self.label = label
        self.color = color
        self.checked = checked
    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN and self.rect.collidepoint(event.pos): self.checked = not self.checked
    def draw(self, screen):
        pygame.draw.rect(screen, (255,255,255), self.rect)
        pygame.draw.rect(screen, self.color, self.rect, 2)
        if self.checked: pygame.draw.rect(screen, self.color, (self.rect.x+4, self.rect.y+4, 12, 12))
        screen.blit(font.render(self.label, True, self.color), (self.rect.x+30, self.rect.y))

class BiasSlider:
    def __init__(self, x, y, w, min_v, max_v):
        self.rect = pygame.Rect(x, y, w, 20)
        self.min_v = min_v
        self.max_v = max_v
        self.val = 0.0
        self.dragging = False
    
    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN and self.rect.collidepoint(event.pos): self.dragging = True
        elif event.type == pygame.MOUSEBUTTONUP: self.dragging = False
        elif event.type == pygame.MOUSEMOTION and self.dragging:
            rel_x = event.pos[0] - self.rect.x
            ratio = max(0, min(1, rel_x / self.rect.width))
            self.val = self.min_v + ratio * (self.max_v - self.min_v)
            if abs(self.val) < (self.max_v - self.min_v)*0.05: self.val = 0 

    def draw(self, screen):
        pygame.draw.rect(screen, (200,200,200), self.rect)
        mid_x = self.rect.x + self.rect.width/2
        pygame.draw.line(screen, (150,150,150), (mid_x, self.rect.y), (mid_x, self.rect.y+20), 2)
        ratio = (self.val - self.min_v) / (self.max_v - self.min_v)
        h_x = self.rect.x + ratio * self.rect.width
        pygame.draw.circle(screen, (200,50,50), (int(h_x), self.rect.centery), 10)
        label = f"External Force (Wind/Push): {self.val:.2f} N"
        screen.blit(font_small.render(label, True, (0,0,0)), (self.rect.x, self.rect.y - 20))

# ============================================================================
#  4. UI 初始化
# ============================================================================
input_boxes = []
def create_group(sx, sy, t, obj):
    g, t_box = [], {"text": t, "pos": (sx, sy-25)}
    attrs = [('kp','KP'), ('ki','KI'), ('kd','KD'), ('max_output','MxOut'), ('max_integral','MxInt'), ('compensation','Comp')]
    for i, (a, l) in enumerate(attrs): g.append(InputBox(sx, sy+i*40, 80, 30, obj, a, l))
    return g, t_box

PX = 950
b_s, t_s = create_group(PX, 80, "SPEED (Outer)", pid_speed)
b_a, t_a = create_group(PX+150, 80, "ANGLE (Mid)", pid_angle)
b_g, t_g = create_group(PX+300, 80, "GYRO (Inner)", pid_gyro)
input_boxes = b_s + b_a + b_g
titles = [t_s, t_a, t_g]

checkboxes = [
    CheckBox(950, 400, "Angle", (255,0,0), True),
    CheckBox(950, 430, "PWM", (0,0,255), True),
    CheckBox(1150, 400, "Spd_I", (0,150,0), True),
    CheckBox(1150, 430, "Ang_I", (255,165,0), False),
    CheckBox(1150, 460, "Gyr_I", (128,0,128), False),
    CheckBox(1150, 490, "Pos X", (0, 200, 200), False), 
    CheckBox(950, 360, "Camera Lock", (0, 0, 0), True)
]
cb_pos, cb_cam = checkboxes[5], checkboxes[6]

bias_slider = BiasSlider(50, 650, 800, -5.0, 5.0)

# ============================================================================
#  5. 物理 & 主循环
# ============================================================================
x, v, theta, omega = 0.0, 0.0, -0.05, 0.0
DT = 0.01

def physics_step(motor_pwm_val, ext_force):
    global x, v, theta, omega
    # 物理参数
    M = 1.0  # 车轮质量 (近似)
    m = 2.0  # 摆杆(车身)质量
    l = 0.8  # 重心高度
    g = 9.8
    
    # 电机 PWM -> 力矩/力的简单映射
    # 假设 PWM 255 对应 10N 推力
    motor_force = (motor_pwm_val / 255.0) * 20.0 
    
    # 摩擦力
    fric_x = 1.0 * v 
    fric_theta = 0.1 * omega 
    
    sin_t, cos_t = math.sin(theta), math.cos(theta)
    
    # 简化的倒立摆动力学方程
    # 角加速度 alpha calculation
    # 分母项
    denom = l * (4.0/3.0 - (m * cos_t**2) / (M + m))
    
    # 引起转动的力矩: 重力 - 电机反作用力 + 外部推力
    # 注意：为了前进，电机向前转，车身会受到反向力矩后仰，
    # 但如果车身前倾，重力力矩会抵消它。
    
    term_gravity = g * sin_t
    term_ext = (ext_force/m) * cos_t
    term_motor = -(motor_force / (M + m)) * cos_t
    
    total_torque = term_gravity + term_ext + term_motor
    
    alpha = (total_torque - fric_theta/l) / (4.0/3.0 * l)
    
    # 线加速度
    accel = (motor_force + ext_force - fric_x) / (M + m) 
    
    omega += alpha * DT
    theta += omega * DT
    v += accel * DT
    x += v * DT
    
    # 地面限制
    if theta > 1.57: theta = 1.57; omega = 0
    if theta < -1.57: theta = -1.57; omega = 0

history = {k: [] for k in ["angle", "pwm", "speed_i", "angle_i", "gyro_i", "pos"]}
running = True
cnt_loop = 0

while running:
    screen.fill((240, 240, 240))
    mx, my = pygame.mouse.get_pos()
    
    for event in pygame.event.get():
        if event.type == pygame.QUIT: running = False
        for box in input_boxes: box.handle_event(event)
        for cb in checkboxes: cb.handle_event(event)
        bias_slider.handle_event(event)
                
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_r: 
                x, v, theta, omega = 0, 0, -0.05, 0
                pid_speed.reset_data(); pid_angle.reset_data(); pid_gyro.reset_data()
                target_speed_set = 0
                for k in history: history[k] = []
            if event.key == pygame.K_UP: target_speed_set += 1.0
            if event.key == pygame.K_DOWN: target_speed_set -= 1.0
            if event.key == pygame.K_SPACE: target_speed_set = 0

    # --- 控制 ---
    sensor_spd = v  # m/s
    sensor_ang = theta # rad
    sensor_gyr = omega # rad/s
    
    cnt_loop += 1
    
    # 模拟不同控制频率：速度环通常比直立环慢
    if cnt_loop >= 5: 
        cnt_loop = 0
        spd_out = Speed_Loop_Control(target_speed_set, sensor_spd)
    else:
        spd_out = pid_speed.output # 保持上一次输出
        
    ang_out = Angle_Loop_Control(spd_out, sensor_ang)
    mot_pwm = Gyro_Loop_Control(ang_out, sensor_gyr)
    
    # --- 物理更新 ---
    total_ext_force = bias_slider.val
    physics_step(mot_pwm, total_ext_force)

    # --- 数据记录 ---
    history["angle"].append(math.degrees(theta))
    history["pwm"].append(mot_pwm)
    history["speed_i"].append(pid_speed.error_integral)
    history["angle_i"].append(pid_angle.error_integral)
    history["gyro_i"].append(pid_gyro.error_integral)
    history["pos"].append(x)
    for k in history: 
        if len(history[k]) > 900: history[k].pop(0)

    # --- 绘图 (相机跟随) ---
    VIEW_CENTER_X = 450 
    SCALE_PIXELS_PER_METER = 50.0
    camera_offset_x = x if cb_cam.checked else 0.0
    
    def world_to_screen(world_x, world_y):
        screen_x = VIEW_CENTER_X + (world_x - camera_offset_x) * SCALE_PIXELS_PER_METER
        screen_y = 400 - world_y * SCALE_PIXELS_PER_METER 
        return int(screen_x), int(screen_y)

    ground_y = 400
    pygame.draw.line(screen, (0,0,0), (0, ground_y), (900, ground_y), 2)
    
    # 绘制网格
    visible_meter_min = int(camera_offset_x) - 10
    visible_meter_max = int(camera_offset_x) + 10
    for m in range(visible_meter_min, visible_meter_max + 1):
        sx, _ = world_to_screen(m, 0)
        pygame.draw.line(screen, (150,150,150), (sx, ground_y), (sx, ground_y + 10), 2)
        txt = font_tiny.render(f"{m}m", True, (100,100,100))
        screen.blit(txt, (sx - 5, ground_y + 12))

    # === 小车绘制 ===
    cart_sx, _ = world_to_screen(x, 0)
    cart_sy = 370 # 固定Y (轮子半径30px -> 0.6m)
    
    # 1. 绘制轮子
    pygame.draw.circle(screen, (50,50,50), (cart_sx, cart_sy), 30, 4)
    # 轮子辐条
    spk_angle = x / 0.6
    spk_x = cart_sx + 30 * math.cos(spk_angle)
    spk_y = cart_sy + 30 * math.sin(spk_angle)
    pygame.draw.line(screen, (200,200,200), (cart_sx, cart_sy), (int(spk_x), int(spk_y)), 2)
    
    # 2. 绘制“期望角度” (幽灵腿) - 这就是你想要的！
    # 速度环计算出的目标角度
    ghost_len = 150
    ghost_x = cart_sx + ghost_len * math.sin(global_debug_target_angle)
    ghost_y = cart_sy - ghost_len * math.cos(global_debug_target_angle)
    pygame.draw.line(screen, (0, 150, 255), (cart_sx, cart_sy), (ghost_x, ghost_y), 2)
    pygame.draw.circle(screen, (0, 150, 255), (int(ghost_x), int(ghost_y)), 5)
    
    # 3. 绘制实际车身
    bx = cart_sx + 150 * math.sin(theta)
    by = cart_sy - 150 * math.cos(theta)
    col = (0,180,0) if abs(theta) < 0.2 else (200,0,0)
    pygame.draw.line(screen, col, (cart_sx, cart_sy), (bx, by), 8)
    
    # 文字标注
    screen.blit(font_tiny.render("Real Angle", True, (0,150,0)), (bx+10, by))
    screen.blit(font_tiny.render("Target Angle (Speed Loop)", True, (0,100,255)), (ghost_x+10, ghost_y-15))

    
    # === 波形绘制 ===
    base_y = 200
    pygame.draw.line(screen, (200,200,200), (0, base_y), (900, base_y), 1)
    def draw_w(d, c, s):
        if len(d) > 1: pygame.draw.lines(screen, c, False, [(i+20, base_y-v*s) for i,v in enumerate(d)], 2)
    
    if checkboxes[0].checked: draw_w(history["angle"], checkboxes[0].color, 2.0)
    if checkboxes[1].checked: draw_w(history["pwm"], checkboxes[1].color, 0.5) # PWM scale
    if checkboxes[2].checked: draw_w(history["speed_i"], checkboxes[2].color, 5.0)
    if checkboxes[3].checked: draw_w(history["angle_i"], checkboxes[3].color, 1.0)
    if checkboxes[4].checked: draw_w(history["gyro_i"], checkboxes[4].color, 1.0)
    if checkboxes[5].checked: draw_w(history["pos"], checkboxes[5].color, 5.0)

    # === UI 面板 ===
    pygame.draw.rect(screen, (220, 220, 220), (900, 0, WIDTH-900, HEIGHT))
    pygame.draw.line(screen, (100,100,100), (900, 0), (900, HEIGHT))
    for t in titles: screen.blit(font.render(t["text"], True, (0,0,100)), t["pos"])
    for box in input_boxes: box.draw(screen)
    for cb in checkboxes: cb.draw(screen)
    bias_slider.draw(screen)
    
    # 信息显示
    info_y = 550
    screen.blit(font.render(f"Target Spd: {target_speed_set:.2f}", True, (0,0,0)), (950, info_y))
    screen.blit(font.render(f"Actual Spd: {v:.2f}", True, (0,100,0) if abs(v-target_speed_set)<0.5 else (200,0,0)), (1150, info_y))
    
    screen.blit(font.render(f"Target Ang: {math.degrees(global_debug_target_angle):.1f}°", True, (0,0,255)), (950, info_y + 30))
    screen.blit(font.render(f"Actual Ang: {math.degrees(theta):.1f}°", True, (0,100,0)), (1150, info_y + 30))

    tips = ["Blue Line = Speed Loop Output (Desired Lean)", "UP/DOWN=Set Speed, R=Reset", "Tune Speed KP to change lean aggression"]
    for i, tp in enumerate(tips): screen.blit(font_small.render(tp, True, (50,50,50)), (950, info_y+70+i*20))

    pygame.display.flip()
    clock.tick(60)

pygame.quit()