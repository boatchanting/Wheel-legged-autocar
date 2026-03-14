import pygame
import numpy as np
from scipy.interpolate import splprep, splev

# --- 1. 全局配置和常量 ---
# 窗口设置
SCREEN_WIDTH = 1280
SCREEN_HEIGHT = 720
FPS = 60

# 颜色定义
COLOR_BACKGROUND = (30, 30, 30)
COLOR_GRID = (50, 50, 50)
COLOR_RAW_PATH = (100, 100, 100)
COLOR_RAW_POINT = (255, 0, 0)
COLOR_SPLINE_PATH = (0, 150, 255)
COLOR_CAR = (255, 255, 0)
COLOR_PREDICTION_OK = (0, 255, 0, 100)  # 半透明
COLOR_PREDICTION_FAIL = (255, 100, 0, 100) # 半透明
COLOR_LOOKAHEAD = (255, 0, 255)
COLOR_TEXT = (255, 255, 255)

# 小车物理属性 (单位: 像素, 度)
CAR_WIDTH = 20
CAR_LENGTH = 40
CAR_MAX_SPEED = 200.0  # 像素/秒
CAR_MAX_STEER = 40.0   # 最大转向角 (度)


# --- 2. 小车类 ---
class Car:
    def __init__(self, x, y, yaw=0.0):
        self.x = x
        self.y = y
        self.yaw = yaw  # 角度制
        self.speed = 0.0

    def update(self, steer, dt):
        """
        使用简单的自行车运动学模型更新小车状态
        steer: 转向角 (度)
        dt: 时间步长 (秒)
        """
        steer = np.clip(steer, -CAR_MAX_STEER, CAR_MAX_STEER)
        
        # 将角度转为弧度
        yaw_rad = np.radians(self.yaw)
        steer_rad = np.radians(steer)
        
        # 运动学模型
        self.x += self.speed * np.cos(yaw_rad) * dt
        self.y += self.speed * np.sin(yaw_rad) * dt
        # omega = v * tan(delta) / L
        self.yaw += np.degrees(self.speed * np.tan(steer_rad) / CAR_LENGTH * dt)
        self.yaw = self.normalize_angle(self.yaw)

    def draw(self, screen, camera):
        """绘制小车"""
        car_points = [
            (-CAR_LENGTH / 2, -CAR_WIDTH / 2),
            (CAR_LENGTH / 2, -CAR_WIDTH / 2),
            (CAR_LENGTH / 2, CAR_WIDTH / 2),
            (-CAR_LENGTH / 2, CAR_WIDTH / 2),
        ]
        
        rotated_points = []
        for x, y in car_points:
            x_rot = x * np.cos(np.radians(self.yaw)) - y * np.sin(np.radians(self.yaw)) + self.x
            y_rot = x * np.sin(np.radians(self.yaw)) + y * np.cos(np.radians(self.yaw)) + self.y
            rotated_points.append(camera.world_to_screen(x_rot, y_rot))
        
        pygame.draw.polygon(screen, COLOR_CAR, rotated_points)
        # 绘制车头方向
        front_x = self.x + CAR_LENGTH / 2 * np.cos(np.radians(self.yaw))
        front_y = self.y + CAR_LENGTH / 2 * np.sin(np.radians(self.yaw))
        start_pos = camera.world_to_screen(self.x, self.y)
        end_pos = camera.world_to_screen(front_x, front_y)
        pygame.draw.line(screen, (255,0,0), start_pos, end_pos, 2)

    @staticmethod
    def normalize_angle(angle):
        while angle > 180: angle -= 360
        while angle < -180: angle += 360
        return angle

# --- 3. 路径类 ---
class Path:
    def __init__(self):
        self.raw_points = []
        self.spline_x = None
        self.spline_y = None
        self.spline_yaw = None
        self.total_length = 0
        self.path_resolution = 200 # 路径插值的点数，越多越平滑

    def add_point(self, x, y):
        self.raw_points.append([x, y])
        if len(self.raw_points) > 1:
            self.interpolate()

    def clear(self):
        self.__init__()

    def interpolate(self):
        """使用三次样条插值生成平滑路径"""
        if len(self.raw_points) < 2:
            return

        points = np.array(self.raw_points).T
        
        # k=3 表示三次样条
        tck, u = splprep([points[0], points[1]], s=0, k=min(3, len(self.raw_points)-1))
        
        u_fine = np.linspace(0, 1, self.path_resolution)
        x_fine, y_fine = splev(u_fine, tck)
        
        # 计算每个点的朝向角 (航向角)
        dx, dy = splev(u_fine, tck, der=1)
        yaws = np.degrees(np.arctan2(dy, dx))

        self.spline_x = x_fine
        self.spline_y = y_fine
        self.spline_yaw = yaws
        
        # 计算路径总长度
        distances = np.sqrt(np.diff(x_fine)**2 + np.diff(y_fine)**2)
        self.total_length = np.sum(distances)

    def find_closest_point_index(self, car_x, car_y):
        """找到离散化路径上离小车最近的点的索引"""
        if self.spline_x is None:
            return 0
        dx = self.spline_x - car_x
        dy = self.spline_y - car_y
        distances = np.hypot(dx, dy)
        return np.argmin(distances)

    def draw(self, screen, camera):
        # 绘制原始点
        for p in self.raw_points:
            pygame.draw.circle(screen, COLOR_RAW_POINT, camera.world_to_screen(*p), 5)
        
        # 绘制原始连线
        if len(self.raw_points) > 1:
            points_screen = [camera.world_to_screen(*p) for p in self.raw_points]
            pygame.draw.lines(screen, COLOR_RAW_PATH, False, points_screen, 1)

        # 绘制插值后的平滑路径
        if self.spline_x is not None and len(self.spline_x) > 1:
            points_screen = [camera.world_to_screen(self.spline_x[i], self.spline_y[i]) for i in range(len(self.spline_x))]
            pygame.draw.lines(screen, COLOR_SPLINE_PATH, False, points_screen, 2)


# --- 4. MPC 控制器类 ---
class MPCController:
    def __init__(self):
        # --- 可调参数 ---
        self.horizon = 10         # N: 预测步长
        self.dt = 0.1             # 预测时间步长 (秒)
        self.w_cte = 1.0          # 权重: 横向误差
        self.w_heading = 1.5      # 权重: 航向误差
        self.w_steer_rate = 0.1   # 权重: 转向变化率 (为了平滑)
        self.lookahead_dist = 100.0 # 预瞄距离 (像素)
        
        # 候选控制量 (转向角)
        self.num_samples = 11
        self.steer_options = np.linspace(-CAR_MAX_STEER, CAR_MAX_STEER, self.num_samples)
        
        # 用于可视化
        self.predicted_paths = []
        self.best_path_idx = -1

    def compute_control(self, car, path):
        self.predicted_paths = [] # 清空上次的预测
        self.best_path_idx = -1

        if path.spline_x is None:
            return 0.0

        # --- 步骤1: 找到当前位置和目标 ---
        # 1a: 找到路径上离小车最近的点 (投影点)
        closest_idx = path.find_closest_point_index(car.x, car.y)
        
        # 1b: 计算横向误差 (CTE - Cross Track Error)
        # 使用叉乘判断小车在路径左侧还是右侧
        path_dx = path.spline_x[(closest_idx + 1) % len(path.spline_x)] - path.spline_x[closest_idx]
        path_dy = path.spline_y[(closest_idx + 1) % len(path.spline_x)] - path.spline_y[closest_idx]
        vec_path = np.array([path_dx, path_dy])
        vec_car = np.array([car.x - path.spline_x[closest_idx], car.y - path.spline_y[closest_idx]])
        
        cte = np.hypot(vec_car[0], vec_car[1])
        cross_product = np.cross(vec_path, vec_car)
        if cross_product > 0:
            cte = -cte # 在右侧

        # 1c: 计算预瞄点 (Look-ahead Point)
        lookahead_idx = closest_idx
        dist_sum = 0
        while dist_sum < self.lookahead_dist and lookahead_idx + 1 < len(path.spline_x):
            dist_sum += np.hypot(path.spline_x[lookahead_idx+1] - path.spline_x[lookahead_idx],
                                 path.spline_y[lookahead_idx+1] - path.spline_y[lookahead_idx])
            lookahead_idx += 1
        
        target_x = path.spline_x[lookahead_idx]
        target_y = path.spline_y[lookahead_idx]
        target_yaw = path.spline_yaw[lookahead_idx]

        # --- 步骤2: 迭代所有候选控制，找到代价最小的 ---
        min_cost = float('inf')
        best_steer = 0.0
        
        for i, steer in enumerate(self.steer_options):
            sim_car = Car(car.x, car.y, car.yaw)
            sim_car.speed = car.speed
            
            cost = 0.0
            current_predicted_path = [(sim_car.x, sim_car.y)]
            
            # --- 步骤3: 在预测时域内模拟 ---
            for _ in range(self.horizon):
                sim_car.update(steer, self.dt)
                current_predicted_path.append((sim_car.x, sim_car.y))

                # 计算代价
                # 1. 横向误差代价 (与预瞄点的距离)
                cost_cte = np.hypot(sim_car.x - target_x, sim_car.y - target_y)
                
                # 2. 航向误差代价
                cost_heading = abs(Car.normalize_angle(sim_car.yaw - target_yaw))
                
                cost += self.w_cte * cost_cte + self.w_heading * cost_heading

            # 3. 转向变化率代价 (可选，使转向更平滑)
            cost += self.w_steer_rate * (steer**2)
            
            self.predicted_paths.append(current_predicted_path)
            
            if cost < min_cost:
                min_cost = cost
                best_steer = steer
                self.best_path_idx = i
        
        # 用于调试的可视化数据
        self.lookahead_point = (target_x, target_y)

        return best_steer

    def draw_debug(self, screen, camera):
        # 绘制预瞄点
        if hasattr(self, 'lookahead_point'):
            pygame.draw.circle(screen, COLOR_LOOKAHEAD, camera.world_to_screen(*self.lookahead_point), 8)
            
        # 绘制所有预测轨迹
        for i, p_path in enumerate(self.predicted_paths):
            if len(p_path) < 2: continue
            
            color = COLOR_PREDICTION_OK if i == self.best_path_idx else COLOR_PREDICTION_FAIL
            points_screen = [camera.world_to_screen(*p) for p in p_path]
            
            # 创建一个带alpha通道的surface来绘制半透明线条
            line_surface = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT), pygame.SRCALPHA)
            pygame.draw.lines(line_surface, color, False, points_screen, 2)
            screen.blit(line_surface, (0, 0))

# --- 5. 视图/相机类 ---
class Camera:
    def __init__(self):
        self.zoom = 1.0
        self.offset_x = 0
        self.offset_y = 0

    def world_to_screen(self, x, y):
        screen_x = int((x - self.offset_x) * self.zoom + SCREEN_WIDTH / 2)
        screen_y = int((y - self.offset_y) * self.zoom + SCREEN_HEIGHT / 2)
        return screen_x, screen_y

    def screen_to_world(self, sx, sy):
        world_x = (sx - SCREEN_WIDTH / 2) / self.zoom + self.offset_x
        world_y = (sy - SCREEN_HEIGHT / 2) / self.zoom + self.offset_y
        return world_x, world_y
        
    def handle_event(self, event, mouse_is_panning):
        if event.type == pygame.MOUSEWHEEL:
            mouse_pos_world_before = self.screen_to_world(*pygame.mouse.get_pos())
            if event.y > 0:
                self.zoom *= 1.1
            else:
                self.zoom *= 0.9
            self.zoom = np.clip(self.zoom, 0.1, 10)
            mouse_pos_world_after = self.screen_to_world(*pygame.mouse.get_pos())
            
            # 调整偏移量以实现鼠标中心缩放
            self.offset_x += mouse_pos_world_before[0] - mouse_pos_world_after[0]
            self.offset_y += mouse_pos_world_before[1] - mouse_pos_world_after[1]

        if mouse_is_panning:
            dx, dy = event.rel
            self.offset_x -= dx / self.zoom
            self.offset_y -= dy / self.zoom

    def reset(self):
        self.zoom = 1.0
        self.offset_x = 0
        self.offset_y = 0

# --- 6. 主程序 ---
def main():
    pygame.init()
    screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
    pygame.display.set_caption("MPC 2D Car Simulation")
    clock = pygame.time.Clock()
    font = pygame.font.SysFont("Consolas", 18)

    car = Car(200, 300, 0)
    path = Path()
    mpc = MPCController()
    camera = Camera()
    
    running = True
    simulation_running = False
    mouse_panning = False
    
    # --- 主循环 ---
    while running:
        dt = clock.tick(FPS) / 1000.0  # 转换为秒
        
        # --- 事件处理 ---
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            
            # 鼠标交互
            if event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1: # 左键添加路径点
                    if not simulation_running:
                        world_x, world_y = camera.screen_to_world(*event.pos)
                        path.add_point(world_x, world_y)
                if event.button == 2: # 中键拖拽
                    mouse_panning = True
            if event.type == pygame.MOUSEBUTTONUP:
                if event.button == 2:
                    mouse_panning = False
            if event.type == pygame.MOUSEMOTION and mouse_panning:
                camera.handle_event(event, True)
            else:
                 camera.handle_event(event, False)

            # 键盘交互
            if event.type == pygame.KEYDOWN:
                # 开始/暂停仿真
                if event.key == pygame.K_SPACE:
                    if len(path.raw_points) > 1:
                        simulation_running = not simulation_running
                        if simulation_running: # 重置小车到起点
                           car.x, car.y = path.raw_points[0]
                           car.yaw = path.spline_yaw[0]
                           car.speed = CAR_MAX_SPEED
                        else:
                           car.speed = 0

                # 清除路径
                if event.key == pygame.K_c:
                    if not simulation_running:
                        path.clear()
                
                # 重置视角
                if event.key == pygame.K_r:
                    camera.reset()

                # --- MPC参数动态调整 ---
                if event.key == pygame.K_UP: mpc.horizon += 1
                if event.key == pygame.K_DOWN and mpc.horizon > 1: mpc.horizon -= 1
                if event.key == pygame.K_RIGHT: mpc.lookahead_dist += 10
                if event.key == pygame.K_LEFT and mpc.lookahead_dist > 10: mpc.lookahead_dist -= 10
                if event.key == pygame.K_w: mpc.w_cte *= 1.2
                if event.key == pygame.K_s: mpc.w_cte *= 0.8
                if event.key == pygame.K_d: mpc.w_heading *= 1.2
                if event.key == pygame.K_a: mpc.w_heading *= 0.8


        # --- 更新状态 ---
        if simulation_running:
            steer_angle = mpc.compute_control(car, path)
            car.update(steer_angle, dt)
            
            # 到达终点则停止
            dist_to_end = np.hypot(car.x - path.raw_points[-1][0], car.y - path.raw_points[-1][1])
            if dist_to_end < CAR_LENGTH:
                simulation_running = False
                car.speed = 0

        # --- 渲染/绘制 ---
        screen.fill(COLOR_BACKGROUND)
        
        # 绘制网格
        for x in range(0, SCREEN_WIDTH, 50):
            pygame.draw.line(screen, COLOR_GRID, (x, 0), (x, SCREEN_HEIGHT))
        for y in range(0, SCREEN_HEIGHT, 50):
            pygame.draw.line(screen, COLOR_GRID, (0, y), (SCREEN_WIDTH, y))

        path.draw(screen, camera)
        if simulation_running:
            mpc.draw_debug(screen, camera)
        car.draw(screen, camera)
        
        # 绘制UI文本
        ui_texts = [
            "--- CONTROLS ---",
            "Left Click: Add Path Point",
            "Mouse Wheel: Zoom",
            "Middle Mouse Drag: Pan",
            "SPACE: Start/Pause Simulation",
            "C: Clear Path",
            "R: Reset View",
            "--- MPC PARAMS (use keys to adjust) ---",
            f"Horizon (UP/DOWN): {mpc.horizon}",
            f"Lookahead (LEFT/RIGHT): {mpc.lookahead_dist:.0f}",
            f"CTE Weight (W/S): {mpc.w_cte:.2f}",
            f"Heading Weight (A/D): {mpc.w_heading:.2f}",
            f"--- STATUS ---",
            f"Simulation: {'RUNNING' if simulation_running else 'PAUSED'}"
        ]
        for i, text in enumerate(ui_texts):
            text_surface = font.render(text, True, COLOR_TEXT)
            screen.blit(text_surface, (10, 10 + i * 22))

        pygame.display.flip()

    pygame.quit()

if __name__ == '__main__':
    main()