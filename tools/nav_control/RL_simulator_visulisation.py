import pygame
import numpy as np
import gymnasium as gym
from gymnasium import spaces
from scipy.interpolate import splprep, splev
from stable_baselines3 import PPO
from stable_baselines3.common.callbacks import BaseCallback
import torch
import multiprocessing as mp
import time
print(f"PyTorch 版本: {torch.__version__}")
print(f"CUDA 是否可用 (编译时支持): {torch.cuda.is_available()}")
# 即使上面是 False，也可以尝试查看编译时的 CUDA 版本
print(f"编译支持的 CUDA 版本: {torch.version.cuda}")

# ========================= 全局配置 =========================
WIDTH, HEIGHT = 1000, 800
CAR_MAX_STEER = 40.0
LOOKAHEAD_COUNT = 10
LOOKAHEAD_GAP = 12

# ========================= 1. 强化学习环境 =========================
class CarEnv(gym.Env):
    def __init__(self, shared_data):
        super().__init__()
        self.shared_data = shared_data # 共享内存字典
        self.action_space = spaces.Box(low=-1, high=1, shape=(1,), dtype=np.float32)
        # 观察空间: [CTE, AngleErr, 10个前瞻点(x,y)] = 22维
        self.observation_space = spaces.Box(low=-np.inf, high=np.inf, shape=(2+LOOKAHEAD_COUNT*2,), dtype=np.float32)
        
        self.x, self.y, self.yaw = 0, 0, 0
        self.speed = 220.0
        self.reset()

    def reset(self, seed=None, options=None):
        # 从共享内存获取最新的路径
        if self.shared_data.get('path_ready', False):
            px, py = self.shared_data['path_x'], self.shared_data['path_y']
            self.x, self.y = px[0], py[0]
            self.yaw = self.shared_data['path_yaws'][0]
        else:
            self.x, self.y, self.yaw = 200, 200, 0
            
        return self._get_obs(), {}

    def _get_obs(self):
        if not self.shared_data.get('path_ready', False):
            return np.zeros(self.observation_space.shape, dtype=np.float32)
        
        # 实时计算距离路径最近的点
        px, py = self.shared_data['path_x'], self.shared_data['path_y']
        yaws = self.shared_data['path_yaws']
        
        dists = np.hypot(px - self.x, py - self.y)
        idx = np.argmin(dists)
        cte = dists[idx]
        
        target_yaw = yaws[idx]
        angle_err = (target_yaw - self.yaw + 180) % 360 - 180
        
        obs = [cte / 100.0, angle_err / 45.0]
        rad = np.radians(-self.yaw)
        for i in range(1, LOOKAHEAD_COUNT + 1):
            f_idx = min(idx + i * LOOKAHEAD_GAP, len(px) - 1)
            dx, dy = px[f_idx] - self.x, py[f_idx] - self.y
            rx = (dx * np.cos(rad) - dy * np.sin(rad)) / 200.0
            ry = (dx * np.sin(rad) + dy * np.cos(rad)) / 200.0
            obs.extend([rx, ry])
            
        return np.array(obs, dtype=np.float32)

    def step(self, action):
        dt = 0.05
        self.yaw += (self.speed * np.tan(np.radians(action[0]*CAR_MAX_STEER)) / 50.0) * np.degrees(dt)
        self.x += self.speed * np.cos(np.radians(self.yaw)) * dt
        self.y += self.speed * np.sin(np.radians(self.yaw)) * dt
        
        # 更新共享内存，让UI进程看到小车在动
        self.shared_data['car_x'] = self.x
        self.shared_data['car_y'] = self.y
        self.shared_data['car_yaw'] = self.yaw
        
        idx, cte = 0, 0
        if self.shared_data.get('path_ready', False):
            px, py = self.shared_data['path_x'], self.shared_data['path_y']
            dists = np.hypot(px - self.x, py - self.y)
            idx = np.argmin(dists)
            cte = dists[idx]

        reward = 1.0 - (cte / 80.0)
        done = cte > 200 or (self.shared_data.get('path_ready') and idx >= len(px) - 5)
        
        return self._get_obs(), reward, done, False, {}

# ========================= 2. 训练进程函数 =========================
def trainer_process(shared_data):
    # 初始化环境
    env = CarEnv(shared_data)
    
    # 检测 GPU
    device = "cuda" if torch.cuda.is_available() else "cpu"
    print(f"训练进程启动。设备: {device}")
    
    # 初始化模型
    model = PPO("MlpPolicy", env, verbose=1, device=device, tensorboard_log="./ppo_tensorboard/")
    
    while True:
        # 检查是否收到开始训练指令
        if shared_data.get('cmd_train', False):
            print("收到训练指令，开始 GPU 训练...")
            model.learn(total_timesteps=30000)
            shared_data['cmd_train'] = False
            model.save("gpu_model")
            print("训练完成，模型已保存。")
        
        # 检查是否收到测试指令
        if shared_data.get('cmd_test', False):
            obs, _ = env.reset()
            while shared_data.get('cmd_test'):
                action, _ = model.predict(obs, deterministic=True)
                obs, _, done, _, _ = env.step(action)
                if done: obs, _ = env.reset()
                time.sleep(0.02)
        
        time.sleep(0.1)

# ========================= 3. UI 交互进程函数 =========================
def ui_process(shared_data):
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("UI 进程 - 手动路径编辑器")
    clock = pygame.time.Clock()
    font = pygame.font.SysFont("SimHei", 20)
    
    raw_points = []
    
    while True:
        screen.fill((30, 30, 30))
        
        # 绘制 UI 提示
        msgs = [
            "【GPU 多进程模式】",
            "1. 左键点击：绘制路径点",
            "2. 按 T：发送指令给 GPU 进程训练",
            "3. 按 S：开启/关闭 演示测试",
            "4. 按 C：清空地图",
            f"GPU 状态: {'正在训练' if shared_data.get('cmd_train') else '空闲'}"
        ]
        for i, m in enumerate(msgs):
            screen.blit(font.render(m, True, (200, 200, 200)), (20, 20 + i*25))

        # 事件处理
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit(); return
            
            if event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1:
                    raw_points.append(event.pos)
                    # 实时计算样条曲线并存入共享内存
                    if len(raw_points) >= 3:
                        pts = np.array(raw_points)
                        tck, u = splprep([pts[:,0], pts[:,1]], s=0, k=min(3, len(pts)-1))
                        u_fine = np.linspace(0, 1, 800)
                        x, y = splev(u_fine, tck)
                        dx, dy = splev(u_fine, tck, der=1)
                        shared_data['path_x'] = x
                        shared_data['path_y'] = y
                        shared_data['path_yaws'] = np.degrees(np.arctan2(dy, dx))
                        shared_data['path_ready'] = True
            
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_t: shared_data['cmd_train'] = True
                if event.key == pygame.K_s: shared_data['cmd_test'] = not shared_data.get('cmd_test', False)
                if event.key == pygame.K_c:
                    raw_points = []
                    shared_data['path_ready'] = False
                    shared_data['cmd_test'] = False

        # 绘制路径
        for p in raw_points:
            pygame.draw.circle(screen, (255, 0, 0), p, 5)
        if shared_data.get('path_ready'):
            pts = list(zip(shared_data['path_x'], shared_data['path_y']))
            pygame.draw.lines(screen, (0, 150, 255), False, pts, 2)
            
        # 绘制小车（位置来源于训练进程的实时更新）
        if 'car_x' in shared_data:
            cx, cy, cyaw = shared_data['car_x'], shared_data['car_y'], shared_data['car_yaw']
            car_surf = pygame.Surface((30, 15), pygame.SRCALPHA)
            car_surf.fill((255, 255, 0))
            rot_car = pygame.transform.rotate(car_surf, -cyaw)
            screen.blit(rot_car, (cx - rot_car.get_width()//2, cy - rot_car.get_height()//2))

        pygame.display.flip()
        clock.tick(60)

# ========================= 4. 主入口 =========================
# ========================= 4. 主入口 =========================
if __name__ == "__main__":
    # [关键修改] 在 Windows 上强制使用 'spawn' 模式
    # 这能确保子进程拥有独立的 Python 解释器环境，避免 CUDA 上下文冲突
    try:
        mp.set_start_method('spawn')
    except RuntimeError:
        # 防止多次调用 set_start_method 导致报错
        pass

    # [建议修改] 在主进程中预先检测 GPU，打印信息以便调试
    print(f"主进程检测到 CUDA 可用: {torch.cuda.is_available()}")
    if torch.cuda.is_available():
        print(f"GPU 设备名称: {torch.cuda.get_device_name(0)}")

    # 使用 Manager 实现进程间数据共享
    manager = mp.Manager()
    shared_data = manager.dict()
    
    # 默认状态
    shared_data['path_ready'] = False
    shared_data['cmd_train'] = False
    shared_data['cmd_test'] = False
    
    # 启动 UI 进程
    p_ui = mp.Process(target=ui_process, args=(shared_data,))
    p_ui.start()
    
    # 启动训练进程 (GPU)
    p_trainer = mp.Process(target=trainer_process, args=(shared_data,))
    p_trainer.start()
    
    p_ui.join()
    p_trainer.terminate()
