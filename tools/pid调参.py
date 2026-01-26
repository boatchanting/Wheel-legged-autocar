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
        self.max_output = float(max_out)
        self.max_integral = float(max_i)
        self.compensation = float(comp)
        self.reset_data()

    def reset_data(self):
        self.error = 0.0
        self.last_error = 0.0
        self.prev_error = 0.0
        self.error_integral = 0.0  
        self.output = 0.0

# 初始化默认参数
# Initialize Default Parameters
pid_speed = PID_Param_t(kp=0.0, ki=0.0, kd=0.0, max_out=0.0, max_i=0.0, comp=0.0)
pid_angle = PID_Param_t(kp=0.0, ki=0.0, kd=0.0, max_out=0.0, max_i=0.0, comp=0.0)
pid_gyro  = PID_Param_t(kp=0.0, ki=0.0, kd=0.0, max_out=0.0, max_i=0.0, comp=0.0)


target_speed_set = 0.0

# ============================================================================
#  2. 控制算法
# ============================================================================
def Speed_Loop_Control(target_speed, actual_speed):
    pid_speed.error = target_speed - actual_speed
    pid_speed.error_integral += pid_speed.error
    pid_speed.error_integral = float_constrain(pid_speed.error_integral, -pid_speed.max_integral, pid_speed.max_integral)
    pid_speed.output = (pid_speed.kp * pid_speed.error) + (pid_speed.ki * pid_speed.error_integral)
    pid_speed.output = float_constrain(pid_speed.output, -pid_speed.max_output, pid_speed.max_output)
    pid_speed.prev_error = pid_speed.last_error
    pid_speed.last_error = pid_speed.error
    return pid_speed.output 

def Angle_Loop_Control(speed_loop_output, actual_angle):
    target_angle = pid_angle.compensation - speed_loop_output 
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
    return -pid_angle.output 

def Gyro_Loop_Control(angle_loop_output, actual_gyro):
    pid_gyro.error = angle_loop_output - actual_gyro
    if pid_gyro.ki != 0:
        pid_gyro.error_integral += pid_gyro.error
        pid_gyro.error_integral = float_constrain(pid_gyro.error_integral, -pid_gyro.max_integral, pid_gyro.max_integral)
    pid_gyro.output = (pid_gyro.kp * pid_gyro.error) + \
                      (pid_gyro.ki * pid_gyro.error_integral) + \
                      (pid_gyro.kd * (pid_gyro.error - pid_gyro.last_error))
    if pid_gyro.output > 0: pid_gyro.output += pid_gyro.compensation
    elif pid_gyro.output < 0: pid_gyro.output -= pid_gyro.compensation
    pid_gyro.output = float_constrain(pid_gyro.output, -pid_gyro.max_output, pid_gyro.max_output)
    pid_gyro.prev_error = pid_gyro.last_error
    pid_gyro.last_error = pid_gyro.error
    return pid_gyro.output

# ============================================================================
#  3. UI 类定义
# ============================================================================
pygame.init()
WIDTH, HEIGHT = 1400, 800 
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("PID Simulator - Fixed Target Speed Display")
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
        label = f"Constant Force (Wind): {self.val:.2f} N"
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
    CheckBox(1150, 490, "Pos X", (0, 200, 200), True), 
    CheckBox(950, 360, "Camera Lock", (0, 0, 0), False)
]
cb_pos, cb_cam = checkboxes[5], checkboxes[6]

bias_slider = BiasSlider(50, 650, 800, -5.0, 5.0)

# ============================================================================
#  5. 物理 & 主循环
# ============================================================================
x, v, theta, omega = 0.0, 0.0, -0.05, 0.0
DT = 0.01

def physics_step(motor_force, ext_force):
    global x, v, theta, omega
    M, m, l, g = 1.0, 0.5, 0.5, 9.8
    fric_x = 0.5 * v 
    fric_theta = 0.01 * omega 
    sin_t, cos_t = math.sin(theta), math.cos(theta)
    
    denom = l * (4.0/3.0 - (m * cos_t**2) / (M + m))
    total_torque = g * sin_t + (ext_force/m)*cos_t - (motor_force/(M+m))*cos_t
    alpha = (total_torque - fric_theta/l) / (4.0/3.0 * l)
    accel = (motor_force + ext_force - fric_x) / (M + m) 
    
    omega += alpha * DT
    theta += omega * DT
    v += accel * DT
    x += v * DT
    
    if theta > 1.5: theta = 1.5; omega = 0
    if theta < -1.5: theta = -1.5; omega = 0

history = {k: [] for k in ["angle", "pwm", "speed_i", "angle_i", "gyro_i", "pos"]}
running = True
cnt_loop = 0

mouse_force = 0.0
is_dragging_force = False
drag_start_pos = (0,0)

while running:
    screen.fill((240, 240, 240))
    mx, my = pygame.mouse.get_pos()
    
    for event in pygame.event.get():
        if event.type == pygame.QUIT: running = False
        for box in input_boxes: box.handle_event(event)
        for cb in checkboxes: cb.handle_event(event)
        bias_slider.handle_event(event)
        
        if event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1 and mx < 900 and my < 600: 
                is_dragging_force = True
                drag_start_pos = (mx, my)
        elif event.type == pygame.MOUSEBUTTONUP:
            if event.button == 1:
                is_dragging_force = False
                mouse_force = 0.0
                
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_r: 
                x, v, theta, omega = 0, 0, -0.05, 0
                pid_speed.reset_data(); pid_angle.reset_data(); pid_gyro.reset_data()
                for k in history: history[k] = []
                target_speed_set = 0
            if event.key == pygame.K_UP: target_speed_set += 20
            if event.key == pygame.K_DOWN: target_speed_set -= 20
            if event.key == pygame.K_SPACE: target_speed_set = 0

    if is_dragging_force:
        dx = mx - drag_start_pos[0]
        mouse_force = dx * 0.05 

    # --- 控制 ---
    sensor_spd, sensor_ang, sensor_gyr = v*100.0, math.degrees(theta), math.degrees(omega)
    cnt_loop += 1
    spd_out = pid_speed.output
    if cnt_loop >= 5: 
        cnt_loop = 0
        spd_out = Speed_Loop_Control(target_speed_set, sensor_spd)
    ang_out = Angle_Loop_Control(spd_out, sensor_ang)
    mot_pwm = Gyro_Loop_Control(ang_out, sensor_gyr)
    
    # --- 物理更新 ---
    total_ext_force = mouse_force + bias_slider.val
    physics_step(mot_pwm * 0.05, total_ext_force)

    # --- 数据记录 ---
    history["angle"].append(sensor_ang)
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
    
    visible_meter_min = int(camera_offset_x) - 10
    visible_meter_max = int(camera_offset_x) + 10
    for m in range(visible_meter_min, visible_meter_max + 1):
        sx, _ = world_to_screen(m, 0)
        pygame.draw.line(screen, (100,100,100), (sx, ground_y), (sx, ground_y + 10), 2)
        txt = font_tiny.render(f"{m}m", True, (100,100,100))
        screen.blit(txt, (sx - 5, ground_y + 12))

    # 小车绘制
    cart_sx, _ = world_to_screen(x, 0)
    cart_sy = 370 # 固定Y
    
    pygame.draw.circle(screen, (50,50,50), (cart_sx, cart_sy), 30, 4)
    spk_angle = x / 0.6
    spk_x = cart_sx + 30 * math.cos(spk_angle)
    spk_y = cart_sy + 30 * math.sin(spk_angle)
    pygame.draw.line(screen, (200,200,200), (cart_sx, cart_sy), (int(spk_x), int(spk_y)), 2)
    
    bx = cart_sx + 150 * math.sin(theta)
    by = cart_sy - 150 * math.cos(theta)
    col = (0,180,0) if abs(theta) < 0.2 else (200,0,0)
    pygame.draw.line(screen, col, (cart_sx, cart_sy), (bx, by), 8)
    
    if is_dragging_force and abs(mouse_force) > 0.1:
        end_x = bx + mouse_force * 20 
        pygame.draw.line(screen, (255, 0, 0), (bx, by), (end_x, by), 4)
        pygame.draw.circle(screen, (255, 0, 0), (int(end_x), int(by)), 5)
        screen.blit(font_small.render(f"F: {mouse_force:.1f}N", True, (255,0,0)), (bx, by-20))
    
    # 4. 波形绘制
    base_y = 200
    pygame.draw.line(screen, (200,200,200), (0, base_y), (900, base_y), 1)
    def draw_w(d, c, s):
        if len(d) > 1: pygame.draw.lines(screen, c, False, [(i+20, base_y-v*s) for i,v in enumerate(d)], 2)
    
    if checkboxes[0].checked: draw_w(history["angle"], checkboxes[0].color, 2.0)
    if checkboxes[1].checked: draw_w(history["pwm"], checkboxes[1].color, 0.01)
    if checkboxes[2].checked: draw_w(history["speed_i"], checkboxes[2].color, 0.05)
    if checkboxes[3].checked: draw_w(history["angle_i"], checkboxes[3].color, 1.0)
    if checkboxes[4].checked: draw_w(history["gyro_i"], checkboxes[4].color, 1.0)
    if checkboxes[5].checked: draw_w(history["pos"], checkboxes[5].color, 20.0)

    # 5. UI 面板
    pygame.draw.rect(screen, (220, 220, 220), (900, 0, WIDTH-900, HEIGHT))
    pygame.draw.line(screen, (100,100,100), (900, 0), (900, HEIGHT))
    for t in titles: screen.blit(font.render(t["text"], True, (0,0,100)), t["pos"])
    for box in input_boxes: box.draw(screen)
    for cb in checkboxes: cb.draw(screen)
    bias_slider.draw(screen)
    
    # 信息显示修复
    info_y = 550
    # 修复：显式显示 Target Speed
    screen.blit(font.render(f"Tgt Spd: {target_speed_set} | Real Spd: {sensor_spd:.1f}", True, (0,100,0)), (950, info_y))
    screen.blit(font.render(f"Position: {x:.2f} m", True, (0,100,0)), (950, info_y + 25))
    
    tips = ["Drag mouse on LEFT to push robot", "Check 'Camera Lock' to follow car", "R=Reset, UP/DOWN=Target Speed"]
    for i, tp in enumerate(tips): screen.blit(font_small.render(tp, True, (50,50,50)), (950, info_y+60+i*18))

    pygame.display.flip()
    clock.tick(60)

pygame.quit()