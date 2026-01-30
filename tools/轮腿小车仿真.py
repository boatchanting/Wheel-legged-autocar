import pygame
import pymunk
import pymunk.pygame_util
import math

# ==========================================
# 1. 配置参数与常量
# ==========================================
WIDTH, HEIGHT = 1000, 600
FPS = 60
# 物理单位转换 (Pymunk单位 -> 屏幕像素)
SCALE = 500.0  # 1米 = 500像素

# 机器人尺寸 (米)
BODY_WIDTH = 0.20
BODY_HEIGHT = 0.10
LEG_L1 = 0.12  # 大腿长度
LEG_L2 = 0.15  # 小腿长度
WHEEL_RADIUS = 0.05
SERVO_OFFSET_X = 0.06 # 舵机安装位置距离中心的X偏移

# 模拟舵机参数
SERVO_SPEED = 15.0      # 舵机最大转速 (rad/s)
SERVO_TORQUE = 5.0      # 舵机最大扭矩 (N*m)
PWM_CENTER = 1500       # 模拟中位
PWM_RANGE = 1000        # PWM摆动幅度对应角度

# C代码中的常量映射
JUMP_OFFSET_LAUNCH = 2000  # 伸腿 (增加Duty)
JUMP_OFFSET_FLIGHT = -1500 # 收腿 (减小Duty)
JUMP_OFFSET_LAND = 1000    # 落地准备
SERVO_HEIGHT = 500         # 基础身高

# 碰撞类型掩码 (防止腿部组件自我碰撞)
CAT_BODY = 0b0001
CAT_LEG = 0b0010
CAT_WHEEL = 0b0100
CAT_GROUND = 0b1000

# ==========================================
# 2. 辅助函数：模拟 C 代码中的数学工具
# ==========================================
def float_constrain(val, min_val, max_val):
    return max(min_val, min(val, max_val))

# ==========================================
# 3. 核心控制器 (移植你的 C 代码逻辑)
# ==========================================
class JumpController:
    def __init__(self):
        # 状态变量
        self.jump_flag = 0
        self.jump_start_time = 0
        self.current_time_ms = 0
        
        # 模拟 PWM 寄存器 (保存上一次的值)
        # 假设机器人是对称的，我们只模拟左侧的前后两个电机
        # CH1: Left Front (LF), CH4: Left Rear (LR)
        self.pwm_lf_last = PWM_CENTER
        self.pwm_lr_last = PWM_CENTER
        
        # 基础方向极性 (假设：Front向下为正，Rear向下为正)
        # 根据五连杆结构，通常前后舵机向外旋转是伸长
        self.dir_lf = 1 
        self.dir_lr = -1 # 后腿反向安装

    def trigger(self):
        if self.jump_flag == 0:
            self.jump_flag = 1
            self.jump_start_time = self.current_time_ms
            print(f"[CTRL] Jump Triggered at {self.current_time_ms}ms")

    def get_joint_target(self, base, direction, height_duty, offset_duty):
        # 对应 C: base_90 + (dir * (height_duty + offset_duty))
        return base + (direction * (height_duty + offset_duty))

    def update(self, dt_ms):
        self.current_time_ms += dt_ms
        
        if self.jump_flag == 0:
            # 非跳跃状态，保持站立高度
            target_h = SERVO_HEIGHT
            # 简单的站立维持
            tgt_lf = self.get_joint_target(PWM_CENTER, self.dir_lf, target_h, 0)
            tgt_lr = self.get_joint_target(PWM_CENTER, self.dir_lr, target_h, 0)
            
            # 使用较慢的斜率跟随
            slope = 30
            self.pwm_lf_last += float_constrain(tgt_lf - self.pwm_lf_last, -slope, slope)
            self.pwm_lr_last += float_constrain(tgt_lr - self.pwm_lr_last, -slope, slope)
            return self.pwm_lf_last, self.pwm_lr_last

        # === 以下完全复刻你的 servo_jump_executor ===
        time_elapsed = self.current_time_ms - self.jump_start_time
        
        target_lf = 0
        target_lr = 0
        dynamic_slope_limit = 0
        h_duty = SERVO_HEIGHT

        # --- 阶段 A: 爆发起跳 (0 - 100ms) ---
        if time_elapsed <= 100:
            dynamic_slope_limit = 10000 # 无视斜率，全速
            target_lf = self.get_joint_target(PWM_CENTER, self.dir_lf, h_duty, JUMP_OFFSET_LAUNCH)
            target_lr = self.get_joint_target(PWM_CENTER, self.dir_lr, h_duty, JUMP_OFFSET_LAUNCH)
        
        # --- 阶段 B: 空中收腿 (100 - 280ms) ---
        elif time_elapsed <= 280:
            dynamic_slope_limit = 10000
            target_lf = self.get_joint_target(PWM_CENTER, self.dir_lf, h_duty, JUMP_OFFSET_FLIGHT)
            target_lr = self.get_joint_target(PWM_CENTER, self.dir_lr, h_duty, JUMP_OFFSET_FLIGHT)
            
        # --- 阶段 C: 落地准备 (280 - 300ms) ---
        elif time_elapsed <= 300:
            dynamic_slope_limit = 10000
            target_lf = self.get_joint_target(PWM_CENTER, self.dir_lf, h_duty, JUMP_OFFSET_LAND)
            target_lr = self.get_joint_target(PWM_CENTER, self.dir_lr, h_duty, JUMP_OFFSET_LAND)
            
        # --- 阶段 D: 缓冲恢复 (300 - 360ms) ---
        elif time_elapsed <= 360:
            dynamic_slope_limit = 20 # 关键：阻尼效果
            target_lf = self.get_joint_target(PWM_CENTER, self.dir_lf, h_duty, 0)
            target_lr = self.get_joint_target(PWM_CENTER, self.dir_lr, h_duty, 0)
            
        else:
            self.jump_flag = 0
            print("[CTRL] Jump Finished")
            return self.pwm_lf_last, self.pwm_lr_last

        # 应用斜率限制
        self.pwm_lf_last += float_constrain(target_lf - self.pwm_lf_last, -dynamic_slope_limit, dynamic_slope_limit)
        self.pwm_lr_last += float_constrain(target_lr - self.pwm_lr_last, -dynamic_slope_limit, dynamic_slope_limit)

        return self.pwm_lf_last, self.pwm_lr_last

# ==========================================
# 4. 物理环境构建类
# ==========================================
class WheelLegRobot:
    def __init__(self, space, x, y):
        self.space = space
        
        # 1. 车身
        mass = 2.0
        moment = pymunk.moment_for_box(mass, (BODY_WIDTH*SCALE, BODY_HEIGHT*SCALE))
        self.body = pymunk.Body(mass, moment)
        self.body.position = (x, y)
        shape = pymunk.Poly.create_box(self.body, (BODY_WIDTH*SCALE, BODY_HEIGHT*SCALE))
        shape.color = (50, 50, 200, 255)
        shape.filter = pymunk.ShapeFilter(group=1) # 相同group不碰撞
        space.add(self.body, shape)
        
        # 2. 连杆结构 (五连杆的一侧)
        # 前腿 (Front)
        self.l1_f, self.l2_f = self._create_leg_linkage(x + SERVO_OFFSET_X*SCALE, y, -1)
        # 后腿 (Rear)
        self.l1_r, self.l2_r = self._create_leg_linkage(x - SERVO_OFFSET_X*SCALE, y, 1)
        
        # 3. 轮毂 (连接两条小腿)
        wheel_mass = 0.5
        wheel_moment = pymunk.moment_for_circle(wheel_mass, 0, WHEEL_RADIUS*SCALE)
        self.wheel = pymunk.Body(wheel_mass, wheel_moment)
        self.wheel.position = (x, y - 0.3*SCALE) # 初始在下方
        wheel_shape = pymunk.Circle(self.wheel, WHEEL_RADIUS*SCALE)
        wheel_shape.friction = 0.9
        wheel_shape.elasticity = 0.0
        wheel_shape.color = (50, 50, 50, 255)
        wheel_shape.filter = pymunk.ShapeFilter(group=1)
        space.add(self.wheel, wheel_shape)
        
        # 4. 关节连接
        # 大腿连车身 (这是我们要控制的舵机轴)
        self.motor_joint_f = pymunk.PivotJoint(self.body, self.l1_f, (SERVO_OFFSET_X*SCALE, 0), (0, LEG_L1*SCALE/2))
        self.motor_joint_r = pymunk.PivotJoint(self.body, self.l1_r, (-SERVO_OFFSET_X*SCALE, 0), (0, LEG_L1*SCALE/2))
        space.add(self.motor_joint_f, self.motor_joint_r)
        
        # 虚拟电机 (用于控制角度)
        self.motor_f = pymunk.SimpleMotor(self.body, self.l1_f, 0)
        self.motor_f.max_force = SERVO_TORQUE * 100000 # 放大以适应pymunk单位
        self.motor_r = pymunk.SimpleMotor(self.body, self.l1_r, 0)
        self.motor_r.max_force = SERVO_TORQUE * 100000
        space.add(self.motor_f, self.motor_r)

        # 膝盖 (大腿连小腿)
        knee_f = pymunk.PivotJoint(self.l1_f, self.l2_f, (0, -LEG_L1*SCALE/2), (0, LEG_L2*SCALE/2))
        knee_r = pymunk.PivotJoint(self.l1_r, self.l2_r, (0, -LEG_L1*SCALE/2), (0, LEG_L2*SCALE/2))
        space.add(knee_f, knee_r) # 修正: 膝盖无碰撞处理交给filter
        
        # 脚踝 (小腿连轮轴)
        ankle_f = pymunk.PivotJoint(self.l2_f, self.wheel, (0, -LEG_L2*SCALE/2), (0, 0))
        ankle_r = pymunk.PivotJoint(self.l2_r, self.wheel, (0, -LEG_L2*SCALE/2), (0, 0))
        space.add(ankle_f, ankle_r)

    def _create_leg_linkage(self, x, y, dir_mult):
        # 创建大腿
        mass = 0.2
        l1_h = LEG_L1 * SCALE
        body1 = pymunk.Body(mass, pymunk.moment_for_box(mass, (10, l1_h)))
        body1.position = (x, y - l1_h/2)
        shape1 = pymunk.Poly.create_box(body1, (10, l1_h))
        shape1.filter = pymunk.ShapeFilter(group=1)
        shape1.color = (200, 50, 50, 255) if dir_mult < 0 else (50, 200, 50, 255)
        self.space.add(body1, shape1)
        
        # 创建小腿
        l2_h = LEG_L2 * SCALE
        body2 = pymunk.Body(mass, pymunk.moment_for_box(mass, (8, l2_h)))
        body2.position = (x, y - l1_h - l2_h/2)
        shape2 = pymunk.Poly.create_box(body2, (8, l2_h))
        shape2.filter = pymunk.ShapeFilter(group=1)
        self.space.add(body2, shape2)
        
        return body1, body2

    def apply_control(self, pwm_lf, pwm_lr):
        # 将 PWM (1000-2000) 映射到 角度 (弧度)
        # 假设 1500 是水平(0度)，摆动范围 +-45度
        # 注意：Pymunk中角度，逆时针为正
        
        def pwm_to_angle(pwm, is_front):
            diff = pwm - PWM_CENTER
            # 简单映射：1000PWM = 90度 (PI/2)
            angle = (diff / 1000.0) * (math.pi / 2)
            # 几何修正：加上初始安装角度
            base_angle = -math.pi/4 if is_front else math.pi/4 
            # 前腿：PWM增 -> 向下伸 -> 顺时针转 (减小角度)
            # 这里的符号需要根据物理模型的初始状态调整
            if is_front:
                return base_angle - angle 
            else:
                return base_angle + angle 

        target_angle_f = pwm_to_angle(pwm_lf, True)
        target_angle_r = pwm_to_angle(pwm_lr, False)
        
        # Pymunk SimpleMotor 控制速率来达到位置 (模拟舵机P环)
        kp = 10.0
        
        curr_angle_f = self.l1_f.angle - self.body.angle
        curr_angle_r = self.l1_r.angle - self.body.angle
        
        # 计算需要的角速度
        rate_f = (target_angle_f - curr_angle_f) * kp
        rate_r = (target_angle_r - curr_angle_r) * kp
        
        self.motor_f.rate = rate_f
        self.motor_r.rate = rate_r


# ==========================================
# 5. 主程序
# ==========================================
def main():
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Wheel-Leg Jump Simulation (Side View)")
    clock = pygame.time.Clock()
    draw_options = pymunk.pygame_util.DrawOptions(screen)

    # 初始化物理空间
    space = pymunk.Space()
    space.gravity = (0.0, -9.81 * SCALE) # 重力向下

    # 地面
    ground = pymunk.Segment(space.static_body, (0, 50), (WIDTH, 50), 5)
    ground.friction = 1.0
    ground.elasticity = 0.5
    space.add(ground)

    # 生成机器人
    robot = WheelLegRobot(space, WIDTH/2, 300)
    controller = JumpController()

    # 模拟控制变量
    time_scale = 1.0
    sim_running = True
    
    font = pygame.font.SysFont("Consolas", 18)

    while sim_running:
        # 1. 输入处理
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                sim_running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE:
                    controller.trigger()
                elif event.key == pygame.K_r:
                    # 重置机器人位置
                    robot.body.position = (WIDTH/2, 300)
                    robot.body.angle = 0
                    robot.body.velocity = (0,0)
                    robot.wheel.velocity = (0,0)
                elif event.key == pygame.K_EQUALS or event.key == pygame.K_PLUS:
                    time_scale = min(time_scale * 1.5, 5.0)
                elif event.key == pygame.K_MINUS:
                    time_scale = max(time_scale / 1.5, 0.1)

        # 2. 控制逻辑更新 (C代码逻辑)
        # 将 Pygame 的 dt 转换为 毫秒
        dt_sec = 1.0/FPS
        sim_dt_ms = dt_sec * 1000 * time_scale
        
        # 获取目标PWM
        pwm_lf, pwm_lr = controller.update(sim_dt_ms)
        
        # 应用到物理模型
        robot.apply_control(pwm_lf, pwm_lr)

        # 3. 物理引擎步进
        # Pymunk 建议固定步长，这里为了慢动作效果，我们多次步进或改变dt
        steps = 5
        for _ in range(steps):
            space.step((dt_sec * time_scale) / steps)

        # 4. 渲染
        screen.fill((240, 240, 240))
        
        # 绘制物理对象
        # Pymunk 坐标系原点在左下角，Pygame在左上角，需要转换
        # pymunk.pygame_util 自动处理翻转，只需设置 transform
        # 这里手动处理一下背景，直接用 draw_options
        
        # 绘制地面线
        pygame.draw.line(screen, (0,0,0), (0, HEIGHT-50), (WIDTH, HEIGHT-50), 5)
        
        # 自定义绘制 (让它看起来更像3D一点)
        # 简单使用 debug draw
        space.debug_draw(draw_options)
        
        # UI 信息
        ui_texts = [
            f"Time Scale: {time_scale:.2f}x (Press +/-)",
            f"Jump State: {'ACTIVE' if controller.jump_flag else 'IDLE'}",
            f"Phase Time: {controller.current_time_ms - controller.jump_start_time:.0f} ms" if controller.jump_flag else "Phase Time: 0",
            f"PWM LF: {int(pwm_lf)} | PWM LR: {int(pwm_lr)}",
            "Controls: SPACE=Jump, R=Reset"
        ]
        
        for i, text in enumerate(ui_texts):
            surf = font.render(text, True, (0, 0, 0))
            screen.blit(surf, (10, 10 + i * 20))

        # 视觉指示灯 (模拟LED)
        if controller.jump_flag:
            pygame.draw.circle(screen, (255, 0, 0), (WIDTH - 50, 50), 10)
            
        pygame.display.flip()
        clock.tick(FPS)

    pygame.quit()

if __name__ == "__main__":
    main()