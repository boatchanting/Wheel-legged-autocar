import pygame
import numpy as np
import gymnasium as gym
from gymnasium import spaces
from scipy.interpolate import splprep, splev
from stable_baselines3 import PPO
import os

# ========================= 配置参数 =========================
SCREEN_WIDTH, SCREEN_HEIGHT = 1200, 800
CAR_MAX_STEER = 40.0        # 最大转向角
LOOKAHEAD_COUNT = 10        # 前瞻点数量
LOOKAHEAD_GAP = 12          # 采样间隔（每隔12个样条点取一个前瞻点）
MODEL_NAME = "ppo_car_expert"

# ========================= 路径处理 =========================
class DynamicPath:
    def __init__(self):
        self.raw_points = []
        self.x, self.y, self.yaws = [], [], []
        self.is_ready = False

    def add_point(self, pos):
        self.raw_points.append(pos)
        self.update_spline()

    def clear(self):
        self.raw_points = []
        self.is_ready = False

    def update_spline(self):
        if len(self.raw_points) < 3:
            self.is_ready = False
            return
        try:
            pts = np.array(self.raw_points)
            # s=0表示拟合曲线必须经过原点
            tck, u = splprep([pts[:,0], pts[:,1]], s=0, k=min(3, len(pts)-1))
            u_fine = np.linspace(0, 1, 1000) # 高密度采样点
            self.x, self.y = splev(u_fine, tck)
            dx, dy = splev(u_fine, tck, der=1)
            self.yaws = np.degrees(np.arctan2(dy, dx))
            self.is_ready = True
        except Exception as e:
            print(f"拟合失败: {e}")
            self.is_ready = False

    def get_closest_info(self, tx, ty):
        if not self.is_ready: return 0, 0
        dists = np.hypot(np.array(self.x) - tx, np.array(self.y) - ty)
        idx = np.argmin(dists)
        return idx, dists[idx]

# ========================= 强化学习环境 =========================
class CarEnv(gym.Env):
    def __init__(self, path_obj):
        super().__init__()
        self.path = path_obj
        # 动作空间：转向角 [-1, 1]
        self.action_space = spaces.Box(low=-1, high=1, shape=(1,), dtype=np.float32)
        
        # 观测空间：[横向误差, 角度误差, 10个前瞻点的相对坐标(x,y)]
        # 总维度：2 + 10 * 2 = 22
        obs_dim = 2 + LOOKAHEAD_COUNT * 2
        self.observation_space = spaces.Box(low=-np.inf, high=np.inf, shape=(obs_dim,), dtype=np.float32)
        
        self.x, self.y, self.yaw = 0, 0, 0
        self.speed = 200.0 # 像素/秒
        self.last_action = 0

    def reset(self, seed=None, options=None):
        if self.path.is_ready:
            self.x, self.y = self.path.x[0], self.path.y[0]
            self.yaw = self.path.yaws[0]
        else:
            self.x, self.y, self.yaw = 100, 100, 0
        self.last_action = 0
        return self._get_obs(), {}

    def _get_obs(self):
        if not self.path.is_ready:
            return np.zeros(self.observation_space.shape, dtype=np.float32)
        
        idx, cte = self.path.get_closest_info(self.x, self.y)
        
        # 1. 基础误差计算
        target_yaw = self.path.yaws[idx]
        angle_err = (target_yaw - self.yaw + 180) % 360 - 180
        
        obs = [cte / 100.0, angle_err / 45.0]
        
        # 2. 多点前瞻感知（局部坐标系）
        rad = np.radians(-self.yaw)
        cos_yaw = np.cos(rad)
        sin_yaw = np.sin(rad)
        
        for i in range(1, LOOKAHEAD_COUNT + 1):
            f_idx = min(idx + i * LOOKAHEAD_GAP, len(self.path.x) - 1)
            dx = self.path.x[f_idx] - self.x
            dy = self.path.y[f_idx] - self.y
            
            # 旋转到车体坐标系：X向前，Y向左
            rx = (dx * cos_yaw - dy * sin_yaw) / 200.0
            ry = (dx * sin_yaw + dy * cos_yaw) / 200.0
            obs.extend([rx, ry])
            
        return np.array(obs, dtype=np.float32)

    def step(self, action):
        dt = 0.05
        steer = action[0] * CAR_MAX_STEER
        
        # 运动学更新
        self.yaw += (self.speed * np.tan(np.radians(steer)) / 50.0) * np.degrees(dt)
        self.x += self.speed * np.cos(np.radians(self.yaw)) * dt
        self.y += self.speed * np.sin(np.radians(self.yaw)) * dt
        
        idx, cte = self.path.get_closest_info(self.x, self.y)
        
        # --- 奖励函数设计 ---
        # 1. 距离惩罚
        reward = 1.0 - (cte / 100.0)
        # 2. 丝滑度惩罚：惩罚剧烈的转向变化
        reward -= 0.1 * abs(action[0] - self.last_action)
        # 3. 速度奖励：鼓励往前跑（索引增加）
        reward += 0.2 * (idx / len(self.path.x))
        
        self.last_action = action[0]
        
        # 结束条件：离线太远 或 到达终点
        done = cte > 250 or idx >= len(self.path.x) - 5
        
        return self._get_obs(), reward, done, False, {}

# ========================= 主程序 =========================
def main():
    pygame.init()
    screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
    pygame.display.set_caption("强化学习多点前瞻丝滑控制仿真")
    clock = pygame.time.Clock()
    font = pygame.font.SysFont("SimHei", 20)

    path = DynamicPath()
    env = CarEnv(path)
    # 创建 PPO 模型
    model = PPO("MlpPolicy", env, verbose=1, learning_rate=3e-4)
    
    is_simulating = False
    is_training = False
    show_lookahead = True

    while True:
        screen.fill((25, 25, 25))
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit(); return
            
            if event.type == pygame.MOUSEBUTTONDOWN and not is_simulating:
                if event.button == 1: path.add_point(event.pos)
            
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_c: # 清除
                    path.clear(); is_simulating = False
                if event.key == pygame.K_t and path.is_ready: # 训练
                    is_training = True
                if event.key == pygame.K_s and path.is_ready: # 运行
                    is_simulating = not is_simulating
                    if is_simulating: obs, _ = env.reset()
                if event.key == pygame.K_v: # 保存
                    model.save(MODEL_NAME); print("模型已保存")
                if event.key == pygame.K_l: # 加载
                    if os.path.exists(MODEL_NAME + ".zip"):
                        model = PPO.load(MODEL_NAME, env=env)
                        print("模型已加载")

        # --- 训练逻辑 ---
        if is_training:
            txt = font.render("AI 正在疯狂闭眼练习中... 请稍候", True, (255, 255, 0))
            screen.blit(txt, (SCREEN_WIDTH//2-150, SCREEN_HEIGHT//2))
            pygame.display.flip()
            model.learn(total_timesteps=10000) # 训练1万步
            is_training = False

        # --- 仿真逻辑 ---
        if is_simulating and path.is_ready:
            current_obs = env._get_obs()
            action, _ = model.predict(current_obs, deterministic=True)
            _, _, done, _, _ = env.step(action)
            if done: is_simulating = False

        # --- 绘制路径 ---
        for p in path.raw_points:
            pygame.draw.circle(screen, (200, 50, 50), p, 5)
        if path.is_ready:
            points = [(path.x[i], path.y[i]) for i in range(len(path.x))]
            pygame.draw.lines(screen, (0, 180, 255), False, points, 3)

        # --- 绘制小车和前瞻点 ---
        if is_simulating:
            # 绘制小车
            car_surf = pygame.Surface((40, 20), pygame.SRCALPHA)
            car_surf.fill((255, 215, 0))
            rotated_car = pygame.transform.rotate(car_surf, -env.yaw)
            screen.blit(rotated_car, (env.x - rotated_car.get_width()//2, env.y - rotated_car.get_height()//2))
            
            # 绘制前瞻采样点（可视化 AI 正在看哪里）
            if show_lookahead:
                idx, _ = path.get_closest_info(env.x, env.y)
                for i in range(1, LOOKAHEAD_COUNT + 1):
                    f_idx = min(idx + i * LOOKAHEAD_GAP, len(path.x) - 1)
                    pygame.draw.circle(screen, (0, 255, 100), (int(path.x[f_idx]), int(path.y[f_idx])), 3)

        # --- UI 文字渲染 ---
        info = [
            f"路径点数: {len(path.raw_points)} (左键点击添加，至少3点)",
            "键盘 [T]: 针对当前路径训练AI",
            "键盘 [S]: 开启/关闭 AI 驾驶仿真",
            "键盘 [C]: 清空地图",
            "键盘 [V]: 保存模型 | [L]: 加载上次模型",
            "----------------",
            "状态: " + ("正在运行" if is_simulating else "待机"),
            "10点前瞻模式: 已开启 (绿色小点即AI的视野)"
        ]
        for i, t in enumerate(info):
            screen.blit(font.render(t, True, (220, 220, 220)), (20, 20 + i*25))

        pygame.display.flip()
        clock.tick(60)

if __name__ == "__main__":
    main()