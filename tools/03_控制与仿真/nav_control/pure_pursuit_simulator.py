#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
===============================================================================
Pure Pursuit 2D 调参仿真器
Pure Pursuit 2D Parameter Tuning Simulator

完全对应您提供的C代码参数和算法逻辑
===============================================================================
"""

import pygame
import math
import json
from dataclasses import dataclass, field
from typing import List, Tuple, Optional

# =================================================================
# 【性能调优参数区】- 对应您的C代码宏定义
# =================================================================
@dataclass
class PurePursuitParams:
    """纯追踪算法参数 - 完全对应您的C代码"""
    # --- 1. 纯追踪 (Pure Pursuit) 导航参数 ---
    pp_ld_min_curve: float = 400.0      # PP_LD_MIN_CURVE - 弯道最小前瞻 (mm)
    pp_ld_min_straight: float = 1.1     # PP_LD_MIN_STRAIGHT - 直道前瞻倍率
    pp_ld_speed_gain: float = 0.5       # PP_LD_SPEED_GAIN - 速度增益系数
    curve_preview_dist: float = 600.0   # CURVE_PREVIEW_DIST - 曲率预判距离 (mm)
    
    # --- 2. 速度规划 (Speed Planning) 参数 ---
    spd_curve_deadzone: float = 0.2     # SPD_CURVE_DEADZONE - 曲率感应死区
    spd_curve_exponent: float = 2.0     # SPD_CURVE_EXPONENT - 曲率减速指数
    spd_angle_penalty: float = 0.2      # SPD_ANGLE_PENALTY - 转向角度惩罚权重
    spd_angle_tolerance: float = 60.0   # SPD_ANGLE_TOLERANCE - 转向角度容忍门槛 (度)
    
    # --- 3. 丝滑滤波 (Smoothness) 参数 ---
    filter_alpha_angle: float = 0.45    # FILTER_ALPHA_ANGLE - 角度滤波系数
    filter_alpha_speed: float = 0.80    # FILTER_ALPHA_SPEED - 速度滤波系数
    slew_rate_angle: float = 25.0       # SLEW_RATE_ANGLE - 单次周期最大转角变化 (度)
    
    # --- 4. 速度规划辅助参数 ---
    nav_speed_fast: float = 1000.0      # NAV_SPEED_FAST - 快速巡航速度 (mm/s)
    nav_speed_slow: float = 300.0       # NAV_SPEED_SLOW - 慢速速度 (mm/s)
    nav_dist_far: float = 2000.0        # NAV_DIST_FAR - 远距离阈值 (mm)
    nav_dist_near: float = 500.0        # NAV_DIST_NEAR - 近距离阈值 (mm)
    nav_dist_arrive: float = 100.0      # NAV_DIST_ARRIVE - 到达阈值 (mm)

@dataclass
class Waypoint:
    """路径点"""
    x: float
    y: float
    point_type: int = 0  # 0=PATH, 1=STOP, 2=SPECIAL

@dataclass
class VehicleState:
    """车辆状态"""
    x: float = 0.0
    y: float = 0.0
    yaw: float = 0.0  # 度
    speed: float = 0.0  # mm/s
    target_speed: float = 0.0
    err_degree: float = 0.0
    prev_err_degree: float = 0.0
    prev_speed: float = 0.0
    target_idx: int = 0
    lookahead_point: Optional[Tuple[float, float]] = None
    lookahead_dist: float = 0.0
    curve_factor: float = 0.0
    trajectory: List[Tuple[float, float]] = field(default_factory=list)

class PurePursuitSimulator:
    """Pure Pursuit算法仿真器"""
    
    def __init__(self, params: PurePursuitParams):
        self.params = params
        self.waypoints: List[Waypoint] = []
        self.vehicle = VehicleState()
        self.running = False
        self.sim_time = 0.0
        self.dt = 0.02  # 50Hz仿真频率
        
    def load_waypoints(self, waypoints: List[Waypoint]):
        """加载路径点"""
        self.waypoints = waypoints
        self.vehicle = VehicleState()
        if waypoints:
            self.vehicle.x = waypoints[0].x
            self.vehicle.y = waypoints[0].y
            self.vehicle.trajectory = [(self.vehicle.x, self.vehicle.y)]
        
    def calc_distance(self, x1: float, y1: float, x2: float, y2: float) -> float:
        """计算两点距离"""
        return math.sqrt((x2 - x1)**2 + (y2 - y1)**2)
    
    def normalize_angle(self, angle: float) -> float:
        """归一化角度到[-180, 180]"""
        while angle > 180:
            angle -= 360
        while angle < -180:
            angle += 360
        return angle
    
    def find_closest_point_index(self, current_idx: int, search_range: int = 20) -> int:
        """查找最近的路径点索引"""
        if not self.waypoints:
            return 0
        
        best_idx = current_idx
        best_dist = float('inf')
        
        start_idx = max(0, current_idx - search_range)
        end_idx = min(len(self.waypoints), current_idx + search_range)
        
        for i in range(start_idx, end_idx):
            dist = self.calc_distance(self.vehicle.x, self.vehicle.y,
                                     self.waypoints[i].x, self.waypoints[i].y)
            if dist < best_dist:
                best_dist = dist
                best_idx = i
                
        return best_idx
    
    def calculate_upcoming_curve_factor(self, base_idx: int, preview_dist: float) -> float:
        """计算前方曲率因子 (0-1, 1为最弯)"""
        if len(self.waypoints) < 3 or base_idx >= len(self.waypoints) - 2:
            return 0.0
        
        total_angle_change = 0.0
        total_dist = 0.0
        
        for i in range(base_idx, min(base_idx + 10, len(self.waypoints) - 2)):
            p1 = self.waypoints[i]
            p2 = self.waypoints[i + 1]
            p3 = self.waypoints[i + 2]
            
            seg_dist = self.calc_distance(p1.x, p1.y, p2.x, p2.y)
            if seg_dist < 10:
                continue
                
            v1 = (p2.x - p1.x, p2.y - p1.y)
            v2 = (p3.x - p2.x, p3.y - p2.y)
            
            len1 = math.sqrt(v1[0]**2 + v1[1]**2)
            len2 = math.sqrt(v2[0]**2 + v2[1]**2)
            
            if len1 < 1 or len2 < 1:
                continue
                
            dot = v1[0]*v2[0] + v1[1]*v2[1]
            angle = math.acos(max(-1, min(1, dot / (len1 * len2))))
            angle_deg = math.degrees(angle)
            
            total_angle_change += angle_deg
            total_dist += seg_dist
            
            if total_dist >= preview_dist:
                break
        
        if total_dist < 10:
            return 0.0
            
        curve_factor = min(1.0, total_angle_change / 90.0)
        return curve_factor
    
    def interpolate_lookahead_point(self, base_idx: int, lookahead_dist: float) -> Tuple[float, float]:
        """插值计算前瞻点"""
        if not self.waypoints or base_idx >= len(self.waypoints) - 1:
            return self.waypoints[-1].x if self.waypoints else 0, \\
                   self.waypoints[-1].y if self.waypoints else 0
        
        tx, ty = self.waypoints[base_idx].x, self.waypoints[base_idx].y
        
        for i in range(base_idx, len(self.waypoints) - 1):
            d_next_node = self.calc_distance(self.vehicle.x, self.vehicle.y,
                                            self.waypoints[i+1].x, self.waypoints[i+1].y)
            
            if d_next_node >= lookahead_dist:
                d_curr_node = self.calc_distance(self.vehicle.x, self.vehicle.y,
                                                self.waypoints[i].x, self.waypoints[i].y)
                seg_len = self.calc_distance(self.waypoints[i].x, self.waypoints[i].y,
                                            self.waypoints[i+1].x, self.waypoints[i+1].y)
                
                if seg_len > 0:
                    ratio = (lookahead_dist - d_curr_node) / seg_len
                    ratio = max(0, min(1, ratio))
                    tx = self.waypoints[i].x + ratio * (self.waypoints[i+1].x - self.waypoints[i].x)
                    ty = self.waypoints[i].y + ratio * (self.waypoints[i+1].y - self.waypoints[i].y)
                break
            else:
                tx, ty = self.waypoints[i+1].x, self.waypoints[i+1].y
                
            if self.waypoints[i+1].point_type != 0:
                break
                
        return tx, ty
    
    def step(self):
        """执行一步仿真"""
        if not self.waypoints or self.vehicle.target_idx >= len(self.waypoints):
            self.running = False
            self.vehicle.target_speed = 0
            return
        
        # 1. 定位当前基准
        base_idx = self.find_closest_point_index(self.vehicle.target_idx, 20)
        self.vehicle.target_idx = base_idx
        
        # 2. 动态前瞻距离计算
        seg_dist = 0
        if base_idx < len(self.waypoints) - 1:
            seg_dist = self.calc_distance(self.waypoints[base_idx].x, self.waypoints[base_idx].y,
                                         self.waypoints[base_idx+1].x, self.waypoints[base_idx+1].y)
        
        Ld_min = self.params.pp_ld_min_curve if seg_dist < self.params.pp_ld_min_curve \\
                 else seg_dist * self.params.pp_ld_min_straight
        lookahead_dist = Ld_min + abs(self.vehicle.prev_speed) * self.params.pp_ld_speed_gain
        self.vehicle.lookahead_dist = lookahead_dist
        
        # 3. 线段插值前瞻点
        tx, ty = self.interpolate_lookahead_point(base_idx, lookahead_dist)
        self.vehicle.lookahead_point = (tx, ty)
        
        # 4. 计算期望偏航角
        dx = tx - self.vehicle.x
        dy = ty - self.vehicle.y
        target_yaw = -math.degrees(math.atan2(dy, -dx))
        raw_err_degree = self.normalize_angle(target_yaw - self.vehicle.yaw)
        
        # 5. 高动力速度规划
        curve_f = self.calculate_upcoming_curve_factor(base_idx, self.params.curve_preview_dist)
        self.vehicle.curve_factor = curve_f
        
        if curve_f < self.params.spd_curve_deadzone:
            curve_f = 0.0
        else:
            curve_f = (curve_f - self.params.spd_curve_deadzone) / (1.0 - self.params.spd_curve_deadzone)
        curve_f = pow(curve_f, self.params.spd_curve_exponent)
        
        dist_stop = 9999.0
        stop_idx = -1
        for i in range(base_idx, min(base_idx + 10, len(self.waypoints))):
            if self.waypoints[i].point_type != 0 or i == len(self.waypoints) - 1:
                dist_stop = self.calc_distance(self.vehicle.x, self.vehicle.y,
                                              self.waypoints[i].x, self.waypoints[i].y)
                stop_idx = i
                break
        
        raw_spd = 0
        if dist_stop < self.params.nav_dist_far:
            if dist_stop <= self.params.nav_dist_arrive:
                raw_spd = 0
            elif dist_stop <= self.params.nav_dist_near:
                raw_spd = self.params.nav_speed_slow * (dist_stop / self.params.nav_dist_near)
            else:
                raw_spd = self.params.nav_speed_slow + \\
                         (self.params.nav_speed_fast - self.params.nav_speed_slow) * \\
                         ((dist_stop - self.params.nav_dist_near) / 
                          (self.params.nav_dist_far - self.params.nav_dist_near))
        else:
            raw_spd = self.params.nav_speed_fast - \\
                     (self.params.nav_speed_fast - self.params.nav_speed_slow) * curve_f
            ang_p = abs(raw_err_degree) / self.params.spd_angle_tolerance
            ang_p = min(1.0, ang_p)
            raw_spd *= (1.0 - self.params.spd_angle_penalty * ang_p)
        
        # 6. 丝滑滤波与输出
        diff = raw_err_degree - self.vehicle.prev_err_degree
        if diff > self.params.slew_rate_angle:
            raw_err_degree = self.vehicle.prev_err_degree + self.params.slew_rate_angle
        elif diff < -self.params.slew_rate_angle:
            raw_err_degree = self.vehicle.prev_err_degree - self.params.slew_rate_angle
        
        self.vehicle.err_degree = self.params.filter_alpha_angle * raw_err_degree + \\
                                 (1.0 - self.params.filter_alpha_angle) * self.vehicle.prev_err_degree
        self.vehicle.target_speed = self.params.filter_alpha_speed * raw_spd + \\
                                   (1.0 - self.params.filter_alpha_speed) * self.vehicle.prev_speed
        
        self.vehicle.prev_err_degree = self.vehicle.err_degree
        self.vehicle.prev_speed = self.vehicle.target_speed
        
        # 7. 车辆运动学更新
        wheelbase = 300.0
        steer_angle = math.radians(self.vehicle.err_degree)
        
        self.vehicle.x += self.vehicle.target_speed * self.dt * math.cos(math.radians(self.vehicle.yaw))
        self.vehicle.y += self.vehicle.target_speed * self.dt * math.sin(math.radians(self.vehicle.yaw))
        self.vehicle.yaw += math.degrees(self.vehicle.target_speed * self.dt * math.tan(steer_angle) / wheelbase)
        self.vehicle.yaw = self.normalize_angle(self.vehicle.yaw)
        
        self.vehicle.trajectory.append((self.vehicle.x, self.vehicle.y))
        
        # 8. 特殊点及终点触发
        if stop_idx != -1 and dist_stop < self.params.nav_dist_arrive:
            if self.waypoints[stop_idx].point_type != 0:
                self.vehicle.target_speed = 0
            elif stop_idx == len(self.waypoints) - 1:
                self.running = False
        
        self.sim_time += self.dt
    
    def reset(self):
        """重置仿真"""
        self.vehicle = VehicleState()
        if self.waypoints:
            self.vehicle.x = self.waypoints[0].x
            self.vehicle.y = self.waypoints[0].y
            self.vehicle.trajectory = [(self.vehicle.x, self.vehicle.y)]
        self.sim_time = 0.0


class PurePursuitVisualizer:
    """Pure Pursuit可视化器"""
    
    def __init__(self, simulator: PurePursuitSimulator):
        pygame.init()
        pygame.display.set_caption("Pure Pursuit 2D 调参仿真器")
        
        self.simulator = simulator
        self.screen_width = 1600
        self.screen_height = 900
        self.screen = pygame.display.set_mode((self.screen_width, self.screen_height))
        self.clock = pygame.time.Clock()
        
        self.font_small = pygame.font.Font(None, 20)
        self.font_medium = pygame.font.Font(None, 28)
        self.font_large = pygame.font.Font(None, 36)
        
        self.view_scale = 1.0
        self.view_offset_x = 100
        self.view_offset_y = 100
        
        self.param_groups = [
            ("纯追踪导航参数", [
                ("pp_ld_min_curve", "弯道最小前瞻(mm)", 100, 1000, 50),
                ("pp_ld_min_straight", "直道前瞻倍率", 0.5, 3.0, 0.1),
                ("pp_ld_speed_gain", "速度增益系数", 0.1, 2.0, 0.1),
                ("curve_preview_dist", "曲率预判距离(mm)", 200, 2000, 50),
            ]),
            ("速度规划参数", [
                ("spd_curve_deadzone", "曲率感应死区", 0.0, 0.5, 0.05),
                ("spd_curve_exponent", "曲率减速指数", 0.5, 4.0, 0.1),
                ("spd_angle_penalty", "转向角度惩罚", 0.0, 0.5, 0.05),
                ("spd_angle_tolerance", "转向角度容忍(度)", 30, 120, 5),
            ]),
            ("丝滑滤波参数", [
                ("filter_alpha_angle", "角度滤波系数", 0.1, 1.0, 0.05),
                ("filter_alpha_speed", "速度滤波系数", 0.3, 1.0, 0.05),
                ("slew_rate_angle", "最大转角变化(度/周期)", 5, 60, 1),
            ]),
            ("速度规划辅助", [
                ("nav_speed_fast", "快速巡航速度(mm/s)", 500, 2000, 50),
                ("nav_speed_slow", "慢速速度(mm/s)", 100, 800, 50),
                ("nav_dist_far", "远距离阈值(mm)", 1000, 5000, 100),
                ("nav_dist_near", "近距离阈值(mm)", 200, 1000, 50),
                ("nav_dist_arrive", "到达阈值(mm)", 50, 300, 25),
            ]),
        ]
        
        self.selected_param = None
        self.param_editors = {}
        
        for group_name, params in self.param_groups:
            for param_name, display_name, min_val, max_val, step in params:
                current_val = getattr(self.simulator.params, param_name)
                self.param_editors[param_name] = {
                    'value': current_val,
                    'min': min_val,
                    'max': max_val,
                    'step': step,
                    'display_name': display_name,
                    'rect': None,
                    'editing': False,
                }
        
        self.waypoint_edit_mode = False
        self.dragged_waypoint = -1
        
        self.running = True
        self.simulation_running = False
        
        self.auto_scale_view()
        
    def auto_scale_view(self):
        """自动调整视图比例"""
        if not self.simulator.waypoints:
            self.view_scale = 0.5
            return
            
        min_x = min(wp.x for wp in self.simulator.waypoints)
        max_x = max(wp.x for wp in self.simulator.waypoints)
        min_y = min(wp.y for wp in self.simulator.waypoints)
        max_y = max(wp.y for wp in self.simulator.waypoints)
        
        path_width = max_x - min_x + 200
        path_height = max_y - min_y + 200
        
        available_width = self.screen_width * 0.6
        available_height = self.screen_height * 0.7
        
        scale_x = available_width / path_width if path_width > 0 else 1.0
        scale_y = available_height / path_height if path_height > 0 else 1.0
        
        self.view_scale = min(scale_x, scale_y)
        self.view_scale = max(0.2, min(2.0, self.view_scale))
        
        self.view_offset_x = 50
        self.view_offset_y = 50
        
    def world_to_screen(self, x: float, y: float) -> Tuple[int, int]:
        """世界坐标转屏幕坐标"""
        sx = self.view_offset_x + x * self.view_scale
        sy = self.view_offset_y - y * self.view_scale
        return int(sx), int(sy)
    
    def screen_to_world(self, sx: int, sy: int) -> Tuple[float, float]:
        """屏幕坐标转世界坐标"""
        x = (sx - self.view_offset_x) / self.view_scale
        y = (self.view_offset_y - sy) / self.view_scale
        return x, y
    
    def draw_path(self):
        """绘制路径"""
        if not self.simulator.waypoints:
            return
            
        points = [self.world_to_screen(wp.x, wp.y) for wp in self.simulator.waypoints]
        if len(points) > 1:
            pygame.draw.lines(self.screen, (100, 100, 255), False, points, 3)
        
        for i, wp in enumerate(self.simulator.waypoints):
            x, y = self.world_to_screen(wp.x, wp.y)
            color = (255, 200, 100) if wp.point_type == 0 else (255, 100, 100)
            pygame.draw.circle(self.screen, color, (x, y), 6)
            
            text = self.font_small.render(str(i), True, (255, 255, 255))
            self.screen.blit(text, (x + 8, y - 8))
    
    def draw_vehicle(self):
        """绘制车辆"""
        vehicle = self.simulator.vehicle
        x, y = self.world_to_screen(vehicle.x, vehicle.y)
        
        angle = -vehicle.yaw
        length = 40 * self.view_scale
        width = 20 * self.view_scale
        
        rad = math.radians(angle)
        front_x = x + length * math.cos(rad)
        front_y = y - length * math.sin(rad)
        left_x = x + width * math.cos(rad + math.pi/2)
        left_y = y - width * math.sin(rad + math.pi/2)
        right_x = x + width * math.cos(rad - math.pi/2)
        right_y = y - width * math.sin(rad - math.pi/2)
        
        pygame.draw.polygon(self.screen, (100, 255, 100), [
            (front_x, front_y),
            (left_x, left_y),
            (right_x, right_y)
        ])
        
        end_x = x + 60 * self.view_scale * math.cos(rad)
        end_y = y - 60 * self.view_scale * math.sin(rad)
        pygame.draw.line(self.screen, (100, 255, 100), (x, y), (end_x, end_y), 2)
    
    def draw_trajectory(self):
        """绘制轨迹"""
        trajectory = self.simulator.vehicle.trajectory
        if len(trajectory) < 2:
            return
            
        points = [self.world_to_screen(x, y) for x, y in trajectory]
        pygame.draw.lines(self.screen, (255, 100, 255), False, points, 2)
    
    def draw_lookahead(self):
        """绘制前瞻可视化"""
        vehicle = self.simulator.vehicle
        
        if vehicle.lookahead_point:
            lx, ly = self.world_to_screen(vehicle.lookahead_point[0], vehicle.lookahead_point[1])
            pygame.draw.circle(self.screen, (255, 255, 0), (int(lx), int(ly)), 8)
            
            vx, vy = self.world_to_screen(vehicle.x, vehicle.y)
            pygame.draw.line(self.screen, (255, 255, 0), (int(vx), int(vy)), (int(lx), int(ly)), 2)
            
            arc_radius = vehicle.lookahead_dist * self.view_scale
            if arc_radius > 10 and arc_radius < 500:
                pygame.draw.circle(self.screen, (255, 255, 0), (int(vx), int(vy)), 
                                 int(arc_radius), 2)
    
    def draw_ui_panel(self):
        """绘制UI面板"""
        panel_x = self.screen_width - 420
        panel_y = 10
        panel_width = 410
        panel_height = self.screen_height - 20
        
        panel_surface = pygame.Surface((panel_width, panel_height), pygame.SRCALPHA)
        panel_surface.fill((30, 30, 50, 200))
        self.screen.blit(panel_surface, (panel_x, panel_y))
        
        title = self.font_large.render("参数调优面板", True, (255, 255, 255))
        self.screen.blit(title, (panel_x + 10, panel_y + 10))
        
        y_offset = panel_y + 60
        for group_name, params in self.param_groups:
            group_text = self.font_medium.render(group_name, True, (200, 200, 255))
            self.screen.blit(group_text, (panel_x + 10, y_offset))
            y_offset += 25
            
            for param_name, display_name, min_val, max_val, step in params:
                editor = self.param_editors[param_name]
                current_val = editor['value']
                
                name_text = self.font_small.render(f"{display_name}: {current_val:.2f}", 
                                                   True, (255, 255, 255))
                self.screen.blit(name_text, (panel_x + 15, y_offset))
                
                editor['rect'] = pygame.Rect(panel_x + 15, y_offset, 380, 20)
                y_offset += 22
            
            y_offset += 10
            
            if y_offset > panel_y + panel_height - 50:
                break
        
        y_offset = panel_y + panel_height - 120
        
        btn_text = "暂停仿真" if self.simulation_running else "启动仿真"
        btn_color = (255, 100, 100) if self.simulation_running else (100, 255, 100)
        pygame.draw.rect(self.screen, btn_color, (panel_x + 10, y_offset, 180, 40))
        btn_label = self.font_medium.render(btn_text, True, (0, 0, 0))
        self.screen.blit(btn_label, (panel_x + 40, y_offset + 8))
        
        pygame.draw.rect(self.screen, (100, 150, 255), (panel_x + 210, y_offset, 180, 40))
        reset_label = self.font_medium.render("重置仿真", True, (0, 0, 0))
        self.screen.blit(reset_label, (panel_x + 240, y_offset + 8))
        
        y_offset += 50
        pygame.draw.rect(self.screen, (255, 200, 100), (panel_x + 10, y_offset, 180, 40))
        load_label = self.font_medium.render("加载S型路径", True, (0, 0, 0))
        self.screen.blit(load_label, (panel_x + 30, y_offset + 8))
        
        pygame.draw.rect(self.screen, (255, 200, 100), (panel_x + 210, y_offset, 180, 40))
        load_label2 = self.font_medium.render("加载环形路径", True, (0, 0, 0))
        self.screen.blit(load_label2, (panel_x + 230, y_offset + 8))
        
        y_offset += 50
        pygame.draw.rect(self.screen, (200, 100, 255), (panel_x + 10, y_offset, 380, 40))
        save_label = self.font_medium.render("导出参数到JSON", True, (255, 255, 255))
        self.screen.blit(save_label, (panel_x + 100, y_offset + 8))
        
        y_offset += 50
        zoom_text = self.font_small.render(f"视图缩放：{self.view_scale:.2f} (鼠标滚轮调整)", 
                                           True, (200, 200, 200))
        self.screen.blit(zoom_text, (panel_x + 10, y_offset))
    
    def draw_status_panel(self):
        """绘制状态面板"""
        panel_x = 10
        panel_y = self.screen_height - 180
        panel_width = 500
        panel_height = 170
        
        panel_surface = pygame.Surface((panel_width, panel_height), pygame.SRCALPHA)
        panel_surface.fill((30, 30, 50, 200))
        self.screen.blit(panel_surface, (panel_x, panel_y))
        
        vehicle = self.simulator.vehicle
        
        status_items = [
            ("仿真时间", f"{self.simulator.sim_time:.2f} s"),
            ("车辆位置", f"({vehicle.x:.1f}, {vehicle.y:.1f}) mm"),
            ("车辆航向", f"{vehicle.yaw:.1f}°"),
            ("当前速度", f"{vehicle.target_speed:.1f} mm/s"),
            ("角度误差", f"{vehicle.err_degree:.2f}°"),
            ("前瞻距离", f"{vehicle.lookahead_dist:.1f} mm"),
            ("曲率因子", f"{vehicle.curve_factor:.3f}"),
            ("目标点索引", f"{vehicle.target_idx}"),
        ]
        
        for i, (label, value) in enumerate(status_items):
            text = self.font_small.render(f"{label}: {value}", True, (255, 255, 255))
            col = i % 2
            row = i // 2
            self.screen.blit(text, (panel_x + 15 + col * 250, panel_y + 15 + row * 25))
    
    def draw_help(self):
        """绘制帮助信息"""
        help_text = [
            "操作说明:",
            "  [S] 启动/暂停仿真",
            "  [R] 重置仿真",
            "  [1-4] 加载预设路径",
            "  [E] 路径点编辑模式",
            "  鼠标滚轮：视图缩放",
            "  左键拖动：平移视图",
            "  左键点击参数：编辑数值",
        ]
        
        panel_surface = pygame.Surface((250, 180), pygame.SRCALPHA)
        panel_surface.fill((30, 30, 50, 180))
        self.screen.blit(panel_surface, (10, 10))
        
        for i, line in enumerate(help_text):
            color = (255, 255, 255) if i == 0 else (200, 200, 200)
            text = self.font_small.render(line, True, color)
            self.screen.blit(text, (20, 15 + i * 20))
    
    def load_preset_path(self, path_type: int):
        """加载预设路径"""
        waypoints = []
        
        if path_type == 1:  # S型路径
            for i in range(50):
                x = i * 100
                y = 300 * math.sin(i * 0.2)
                waypoints.append(Waypoint(x, y))
        elif path_type == 2:  # 环形路径
            for i in range(60):
                angle = i * 2 * math.pi / 60
                x = 500 + 400 * math.cos(angle)
                y = 400 + 400 * math.sin(angle)
                waypoints.append(Waypoint(x, y))
        elif path_type == 3:  # 直线 + 弯道
            for i in range(20):
                waypoints.append(Waypoint(i * 100, 0))
            for i in range(20):
                angle = i * math.pi / 20
                x = 2000 + 300 * math.sin(angle)
                y = 300 * (1 - math.cos(angle))
                waypoints.append(Waypoint(x, y))
            for i in range(20):
                waypoints.append(Waypoint(2000 + 300 + i * 100, 300))
        elif path_type == 4:  # 8字形
            for i in range(80):
                angle = i * 2 * math.pi / 80
                x = 500 + 300 * math.sin(angle)
                y = 400 + 150 * math.sin(2 * angle)
                waypoints.append(Waypoint(x, y))
        
        if waypoints:
            self.simulator.load_waypoints(waypoints)
            self.auto_scale_view()
    
    def handle_events(self):
        """处理事件"""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
                
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_s:
                    self.simulation_running = not self.simulation_running
                    self.simulator.running = self.simulation_running
                elif event.key == pygame.K_r:
                    self.simulator.reset()
                elif event.key == pygame.K_1:
                    self.load_preset_path(1)
                elif event.key == pygame.K_2:
                    self.load_preset_path(2)
                elif event.key == pygame.K_3:
                    self.load_preset_path(3)
                elif event.key == pygame.K_4:
                    self.load_preset_path(4)
                elif event.key == pygame.K_e:
                    self.waypoint_edit_mode = not self.waypoint_edit_mode
                    
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1:
                    mx, my = event.pos
                    
                    for param_name, editor in self.param_editors.items():
                        if editor['rect'] and editor['rect'].collidepoint(mx, my):
                            editor['editing'] = True
                            self.selected_param = param_name
                            break
                    
                    panel_x = self.screen_width - 420
                    panel_y = 10
                    panel_height = self.screen_height - 20
                    
                    if pygame.Rect(panel_x + 10, panel_y + panel_height - 120, 180, 40).collidepoint(mx, my):
                        self.simulation_running = not self.simulation_running
                        self.simulator.running = self.simulation_running
                    
                    if pygame.Rect(panel_x + 210, panel_y + panel_height - 120, 180, 40).collidepoint(mx, my):
                        self.simulator.reset()
                    
                    if pygame.Rect(panel_x + 10, panel_y + panel_height - 70, 180, 40).collidepoint(mx, my):
                        self.load_preset_path(1)
                    
                    if pygame.Rect(panel_x + 210, panel_y + panel_height - 70, 180, 40).collidepoint(mx, my):
                        self.load_preset_path(2)
                    
                    if pygame.Rect(panel_x + 10, panel_y + panel_height - 20, 380, 40).collidepoint(mx, my):
                        self.export_params()
                        
                elif event.button == 4:
                    self.view_scale = min(2.0, self.view_scale * 1.1)
                elif event.button == 5:
                    self.view_scale = max(0.2, self.view_scale / 1.1)
                    
            elif event.type == pygame.MOUSEBUTTONUP:
                if event.button == 1:
                    for editor in self.param_editors.values():
                        editor['editing'] = False
                    self.selected_param = None
                    
            elif event.type == pygame.MOUSEMOTION:
                if self.selected_param:
                    mx, my = event.pos
                    editor = self.param_editors[self.selected_param]
                    dx = event.rel[0]
                    new_val = editor['value'] + dx * editor['step']
                    new_val = max(editor['min'], min(editor['max'], new_val))
                    editor['value'] = new_val
                    setattr(self.simulator.params, self.selected_param, new_val)
        
        keys = pygame.key.get_pressed()
        if self.selected_param:
            editor = self.param_editors[self.selected_param]
            if keys[pygame.K_LEFT]:
                new_val = editor['value'] - editor['step']
                new_val = max(editor['min'], min(editor['max'], new_val))
                editor['value'] = new_val
                setattr(self.simulator.params, self.selected_param, new_val)
            if keys[pygame.K_RIGHT]:
                new_val = editor['value'] + editor['step']
                new_val = max(editor['min'], min(editor['max'], new_val))
                editor['value'] = new_val
                setattr(self.simulator.params, self.selected_param, new_val)
    
    def export_params(self):
        """导出参数到 JSON"""
        params_dict = {}
        for param_name, editor in self.param_editors.items():
            params_dict[param_name] = editor['value']
        
        with open('pure_pursuit_params.json', 'w', encoding='utf-8') as f:
            json.dump(params_dict, f, indent=2, ensure_ascii=False)
        
        print("参数已导出到 pure_pursuit_params.json")
    
    def run(self):
        """主循环"""
        self.load_preset_path(1)
        
        while self.running:
            self.clock.tick(60)
            
            self.handle_events()
            
            if self.simulation_running and self.simulator.running:
                for _ in range(3):
                    self.simulator.step()
            
            self.screen.fill((20, 20, 35))
            
            self.draw_path()
            self.draw_trajectory()
            self.draw_lookahead()
            self.draw_vehicle()
            self.draw_ui_panel()
            self.draw_status_panel()
            self.draw_help()
            
            pygame.display.flip()
        
        pygame.quit()


def main():
    """主函数"""
    print("=" * 70)
    print("Pure Pursuit 2D 调参仿真器")
    print("=" * 70)
    print("\\n初始化参数...")
    
    params = PurePursuitParams(
        pp_ld_min_curve=400.0,
        pp_ld_min_straight=1.1,
        pp_ld_speed_gain=0.5,
        curve_preview_dist=600.0,
        spd_curve_deadzone=0.2,
        spd_curve_exponent=2.0,
        spd_angle_penalty=0.2,
        spd_angle_tolerance=60.0,
        filter_alpha_angle=0.45,
        filter_alpha_speed=0.80,
        slew_rate_angle=25.0,
        nav_speed_fast=1000.0,
        nav_speed_slow=300.0,
        nav_dist_far=2000.0,
        nav_dist_near=500.0,
        nav_dist_arrive=100.0,
    )
    
    print(f"创建仿真器...")
    simulator = PurePursuitSimulator(params)
    
    print(f"启动可视化器...")
    visualizer = PurePursuitVisualizer(simulator)
    
    print("\\n仿真器已启动！")
    print("按 [S] 启动/暂停仿真")
    print("按 [1-4] 加载不同预设路径")
    print("按 [E] 切换路径点编辑模式")
    print("=" * 70)
    
    visualizer.run()


if __name__ == "__main__":
    main()

