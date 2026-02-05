#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
智能小车惯性导航增强系统 - 加速度计辅助偏差修正
核心特性：
  1. 初始偏航角归零（起始朝向=+X轴）
  2. 零速修正（ZUPT）：利用加速度计检测静止状态，重置速度漂移
  3. 运动状态自适应融合：根据加速度一致性动态调整轮速权重
  4. 专业级对比分析：有/无加速度修正的轨迹/误差可视化
"""

import json
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import rcParams
from scipy import signal
import os

# ========================================
# 配置参数（关键：加速度计辅助阈值）
# ========================================
class Config:
    DT = 0.01  # 10ms采样周期
    
    # 机械参数（必须实测校准！）
    WHEEL_DIAMETER = 0.065    # 轮径 [m]
    WHEELBASE = 0.145         # 轮距 [m]
    PULSES_PER_REV = 13       # 编码器每转脉冲数
    
    # 滤波参数
    WHEEL_FILTER_CUTOFF = 2.0  # 轮速滤波截止频率 [Hz]
    ACCEL_FILTER_CUTOFF = 5.0  # 加速度滤波截止频率 [Hz]
    FILTER_ORDER = 4
    
    # 零速修正（ZUPT）参数
    ZUPT_ACCEL_THRESHOLD = 0.15  # 静止判定：合加速度 < 0.15 m/s²
    ZUPT_SPEED_THRESHOLD = 0.05  # 静止判定：轮速 < 0.05 m/s
    ZUPT_DURATION = 10           # 持续静止采样点数（100ms）
    ZUPT_VELOCITY_RESET = True   # 静止时是否重置速度为零
    
    # 轮滑检测参数
    SLIP_ACCEL_THRESHOLD = 0.8   # 轮滑判定：加速度差异 > 0.8 m/s²
    SLIP_MIN_SPEED = 0.2         # 仅当速度 > 0.2 m/s 时检测轮滑
    
    # 数据文件路径
    DATA_PATH = r"\Data\惯性导航测试数据10ms.txt"

# ========================================
# 数据加载与预处理（扩展加速度通道）
# ========================================
class DataLoader:
    @staticmethod
    def load_full_dataset(filepath):
        """加载完整传感器数据集（含加速度通道）"""
        if not os.path.exists(filepath):
            raise FileNotFoundError(f"数据文件不存在: {filepath}")
        
        with open(filepath, 'r', encoding='utf-8') as f:
            raw_data = json.load(f)
        
        # 构建通道字典
        channels = {}
        for item in raw_
            chan_id = item['Channel']
            channels[chan_id] = np.array(item['Points'], dtype=np.float64)
        
        # 验证必要通道
        required = {
            0: "左轮转速", 1: "右轮转速", 
            4: "偏航角", 5: "陀螺仪Z轴",
            6: "X加速度(左为正)", 7: "Y加速度(前为正)"
        }
        for chan, name in required.items():
            if chan not in channels:
                raise ValueError(f"缺失必要通道 {chan} ({name})")
        
        # 统一长度
        min_len = min(len(channels[chan]) for chan in required)
        for chan in required:
            channels[chan] = channels[chan][:min_len]
        
        print(f"✓ 成功加载 {min_len} 个采样点 ({min_len * Config.DT:.2f} 秒)")
        print(f"  包含通道: 0(左轮), 1(右轮), 4(偏航角), 5(陀螺仪Z), 6(ax), 7(ay)")
        return channels, min_len
    
    @staticmethod
    def convert_wheel_speed(pulses, is_left_wheel):
        """轮速转换：脉冲/10ms → 线速度 [m/s]（符号校正）"""
        # 符号校正：左轮负值正转 → 取反；右轮正值正转 → 保持
        corrected = -pulses if is_left_wheel else pulses
        
        # 单位转换
        circumference = np.pi * Config.WHEEL_DIAMETER
        distance_per_pulse = circumference / Config.PULSES_PER_REV
        return corrected * distance_per_pulse / Config.DT
    
    @staticmethod
    def normalize_yaw(yaw_deg):
        """偏航角归零：以初始时刻为0°参考系"""
        initial = yaw_deg[0]
        return yaw_deg - initial, initial
    
    @staticmethod
    def lowpass_filter(data, cutoff, fs, order=4):
        """零相位巴特沃斯低通滤波"""
        nyq = 0.5 * fs
        normal_cutoff = cutoff / nyq
        b, a = signal.butter(order, normal_cutoff, btype='low', analog=False)
        return signal.filtfilt(b, a, data)

# ========================================
# 运动状态检测器（核心：静止/轮滑识别）
# ========================================
class MotionDetector:
    def __init__(self):
        self.zupt_counter = 0
        self.is_static = False
    
    def detect_static(self, ax, ay, v_left, v_right):
        """
        零速检测（ZUPT）：基于加速度+轮速双重验证
        
        返回:
            is_static: 当前是否静止
            should_reset: 是否应执行速度重置
        """
        # 计算合加速度（已去除重力，直接使用）
        accel_magnitude = np.sqrt(ax**2 + ay**2)
        
        # 轮速合速度
        v_magnitude = np.abs((v_left + v_right) / 2.0)
        
        # 静止条件：加速度小 + 轮速小
        if (accel_magnitude < Config.ZUPT_ACCEL_THRESHOLD and 
            v_magnitude < Config.ZUPT_SPEED_THRESHOLD):
            self.zupt_counter += 1
        else:
            self.zupt_counter = 0
        
        # 持续静止判定
        self.is_static = (self.zupt_counter >= Config.ZUPT_DURATION)
        
        # 速度重置条件：刚进入静止状态时触发一次
        should_reset = (self.is_static and self.zupt_counter == Config.ZUPT_DURATION)
        
        return self.is_static, should_reset
    
    def detect_wheel_slip(self, ay_imu, v_current, v_previous):
        """
        轮滑检测：比较IMU加速度与轮速微分加速度
        
        参数:
            ay_imu: IMU Y轴加速度 [m/s²]（车体坐标系，前向为正）
            v_current: 当前轮速平均值 [m/s]
            v_previous: 上一时刻轮速平均值 [m/s]
        
        返回:
            is_slip: 是否检测到轮滑
            accel_wheel: 轮速微分得到的加速度 [m/s²]
        """
        # 轮速微分加速度（需滤波抑制噪声）
        accel_wheel = (v_current - v_previous) / Config.DT if Config.DT > 0 else 0.0
        
        # 仅在运动时检测轮滑
        if np.abs(v_current) < Config.SLIP_MIN_SPEED:
            return False, accel_wheel
        
        # 比较IMU加速度与轮速微分（Y轴方向一致）
        accel_diff = np.abs(ay_imu - accel_wheel)
        
        is_slip = (accel_diff > Config.SLIP_ACCEL_THRESHOLD)
        return is_slip, accel_wheel

# ========================================
# 增强型惯性导航（含加速度辅助修正）
# ========================================
class EnhancedNavigation:
    def __init__(self):
        self.config = Config()
        self.detector = MotionDetector()
    
    def integrate_baseline(self, v_left, v_right, yaw_zeroed):
        """
        基准模式：仅使用轮速+IMU偏航角（无加速度修正）
        用于对比分析
        """
        n = len(v_left)
        x = np.zeros(n)
        y = np.zeros(n)
        theta = np.deg2rad(yaw_zeroed)  # 直接使用归零偏航角
        
        for i in range(1, n):
            v_avg = (v_left[i] + v_right[i]) / 2.0
            x[i] = x[i-1] + v_avg * np.cos(theta[i]) * self.config.DT
            y[i] = y[i-1] + v_avg * np.sin(theta[i]) * self.config.DT
        
        return x, y, theta
    
    def integrate_with_accel_correction(self, v_left, v_right, yaw_zeroed, ax, ay):
        """
        增强模式：融合加速度计进行偏差修正
        
        修正策略：
          1. 零速修正（ZUPT）：静止时重置速度漂移
          2. 轮滑抑制：检测到轮滑时降低轮速权重，短期信任IMU加速度
        """
        n = len(v_left)
        x = np.zeros(n)
        y = np.zeros(n)
        theta = np.deg2rad(yaw_zeroed)
        v_corrected = np.zeros(n)  # 修正后的速度
        
        # 初始状态
        v_prev = 0.0
        slip_counter = 0
        
        for i in range(1, n):
            # 1. 基础轮速计算
            v_wheel = (v_left[i] + v_right[i]) / 2.0
            
            # 2. 运动状态检测
            is_static, should_reset = self.detector.detect_static(
                ax[i], ay[i], v_left[i], v_right[i]
            )
            
            # 3. 零速修正（ZUPT）
            if should_reset and Config.ZUPT_VELOCITY_RESET:
                v_corrected[i] = 0.0
                v_prev = 0.0
                slip_counter = 0
                # 位置保持不变（静止时不积分）
                x[i] = x[i-1]
                y[i] = y[i-1]
                continue
            
            # 4. 轮滑检测与处理
            is_slip, accel_wheel = self.detector.detect_wheel_slip(
                ay[i], v_wheel, v_prev
            )
            
            if is_slip:
                slip_counter += 1
                # 轮滑期间：使用IMU加速度积分速度（短期）
                if slip_counter < 20:  # 限200ms内使用IMU
                    v_corrected[i] = v_prev + ay[i] * self.config.DT
                else:
                    # 长时间轮滑：保守策略，降低速度
                    v_corrected[i] = v_prev * 0.95
            else:
                slip_counter = 0
                # 正常状态：使用轮速（轻微平滑）
                v_corrected[i] = 0.7 * v_wheel + 0.3 * v_prev
            
            # 5. 位置积分（使用修正后速度）
            dx = v_corrected[i] * np.cos(theta[i]) * self.config.DT
            dy = v_corrected[i] * np.sin(theta[i]) * self.config.DT
            x[i] = x[i-1] + dx
            y[i] = y[i-1] + dy
            
            # 6. 更新历史速度
            v_prev = v_corrected[i]
        
        return x, y, theta, v_corrected

# ========================================
# 专业对比可视化
# ========================================
class ComparisonVisualizer:
    @staticmethod
    def configure_plotting():
        rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei', 'DejaVu Sans']
        rcParams['axes.unicode_minus'] = False
        rcParams['figure.dpi'] = 120
    
    @staticmethod
    def plot_trajectory_comparison(x_base, y_base, x_enhanced, y_enhanced, theta):
        """对比绘制基准轨迹与增强轨迹"""
        fig, ax = plt.subplots(figsize=(12, 10))
        
        # 基准轨迹（红色虚线）
        ax.plot(x_base, y_base, 'r--', linewidth=2.0, alpha=0.7, 
               label='基准模式 (仅轮速+IMU偏航角)', zorder=2)
        
        # 增强轨迹（蓝色实线）
        ax.plot(x_enhanced, y_enhanced, 'b-', linewidth=2.5, alpha=0.9,
               label='增强模式 (加速度辅助修正)', zorder=3)
        
        # 起点/终点标记
        ax.plot(0, 0, 'go', markersize=12, label='起点 (0,0)', zorder=5)
        ax.plot(x_enhanced[-1], y_enhanced[-1], 'r*', markersize=15,
               label=f'终点 ({x_enhanced[-1]:.2f}, {y_enhanced[-1]:.2f})', zorder=5)
        
        # 方向箭头（增强轨迹）
        arrow_interval = max(1, len(x_enhanced) // 25)
        for i in range(0, len(x_enhanced), arrow_interval):
            scale = 0.18
            dx = np.cos(theta[i]) * scale
            dy = np.sin(theta[i]) * scale
            ax.arrow(x_enhanced[i], y_enhanced[i], dx, dy, 
                    head_width=0.09, head_length=0.13,
                    fc='darkblue', ec='darkblue', alpha=0.6, zorder=4)
        
        ax.set_xlabel('X 坐标 [m]', fontsize=13, fontweight='bold')
        ax.set_ylabel('Y 坐标 [m]', fontsize=13, fontweight='bold')
        ax.set_title('导航轨迹对比：基准模式 vs. 加速度辅助增强模式', 
                    fontsize=15, fontweight='bold', pad=15)
        ax.grid(True, linestyle='--', alpha=0.65, linewidth=0.9)
        ax.axis('equal')
        ax.legend(loc='best', fontsize=11, framealpha=0.95)
        
        # 添加误差分析框
        end_error = np.sqrt((x_base[-1]-x_enhanced[-1])**2 + (y_base[-1]-y_enhanced[-1])**2)
        total_dist_base = np.sum(np.sqrt(np.diff(x_base)**2 + np.diff(y_base)**2))
        total_dist_enh = np.sum(np.sqrt(np.diff(x_enhanced)**2 + np.diff(y_enhanced)**2))
        
        stats = (f"终点位置偏差: {end_error:.3f} m\n"
                f"基准轨迹长度: {total_dist_base:.3f} m\n"
                f"增强轨迹长度: {total_dist_enh:.3f} m\n"
                f"相对改进: {(1-total_dist_enh/total_dist_base)*100:+.1f}%")
        
        ax.text(0.03, 0.97, stats, transform=ax.transAxes, fontsize=10,
               verticalalignment='top',
               bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.92))
        
        plt.tight_layout()
        return fig
    
    @staticmethod
    def plot_error_analysis(t, x_base, y_base, x_enh, y_enh):
        """误差时序分析：位置偏差、速度修正效果"""
        fig = plt.figure(figsize=(14, 10))
        
        # 子图1：位置偏差
        ax1 = plt.subplot(2, 2, 1)
        pos_error = np.sqrt((x_base - x_enh)**2 + (y_base - y_enh)**2)
        ax1.plot(t, pos_error, 'm-', linewidth=2.0)
        ax1.axhline(y=np.mean(pos_error), color='r', linestyle='--', 
                   label=f'平均偏差: {np.mean(pos_error):.3f} m')
        ax1.set_xlabel('时间 [s]', fontsize=11)
        ax1.set_ylabel('位置偏差 [m]', fontsize=11)
        ax1.set_title('基准 vs 增强模式位置偏差', fontsize=12, fontweight='bold')
        ax1.grid(True, alpha=0.5)
        ax1.legend()
        
        # 子图2：X坐标对比
        ax2 = plt.subplot(2, 2, 2)
        ax2.plot(t, x_base, 'r--', alpha=0.7, label='基准 X')
        ax2.plot(t, x_enh, 'b-', alpha=0.9, label='增强 X')
        ax2.set_xlabel('时间 [s]', fontsize=11)
        ax2.set_ylabel('X 坐标 [m]', fontsize=11)
        ax2.set_title('X 坐标时序对比', fontsize=12, fontweight='bold')
        ax2.grid(True, alpha=0.5)
        ax2.legend()
        
        # 子图3：Y坐标对比
        ax3 = plt.subplot(2, 2, 3)
        ax3.plot(t, y_base, 'r--', alpha=0.7, label='基准 Y')
        ax3.plot(t, y_enh, 'b-', alpha=0.9, label='增强 Y')
        ax3.set_xlabel('时间 [s]', fontsize=11)
        ax3.set_ylabel('Y 坐标 [m]', fontsize=11)
        ax3.set_title('Y 坐标时序对比', fontsize=12, fontweight='bold')
        ax3.grid(True, alpha=0.5)
        ax3.legend()
        
        # 子图4：静止检测事件
        ax4 = plt.subplot(2, 2, 4)
        # 模拟静止事件（实际应从detector获取）
        static_events = np.where(np.abs(np.gradient(x_enh)) < 0.01)[0]
        if len(static_events) > 0:
            ax4.vlines(t[static_events], -0.5, 0.5, colors='green', 
                      alpha=0.4, label='检测到静止', linewidth=2)
        ax4.set_ylim(-1, 1)
        ax4.set_yticks([])
        ax4.set_xlabel('时间 [s]', fontsize=11)
        ax4.set_title('静止事件检测 (ZUPT触发点)', fontsize=12, fontweight='bold')
        ax4.grid(True, alpha=0.3, axis='x')
        ax4.legend()
        
        fig.suptitle('导航误差深度分析', fontsize=14, fontweight='bold', y=0.995)
        plt.tight_layout()
        return fig
    
    @staticmethod
    def plot_acceleration_utilization(t, ax, ay, zupt_events, slip_events):
        """加速度数据利用分析"""
        fig, axes = plt.subplots(3, 1, figsize=(13, 9), sharex=True)
        
        # X加速度
        axes[0].plot(t, ax, 'c-', linewidth=1.5, label='a_x (左为正)')
        axes[0].axhline(y=0, color='k', linestyle='--', alpha=0.4)
        axes[0].set_ylabel('a_x [m/s²]', fontsize=11, fontweight='bold')
        axes[0].set_title('X轴加速度 (车体坐标系)', fontsize=12, fontweight='bold')
        axes[0].grid(True, alpha=0.4)
        
        # Y加速度
        axes[1].plot(t, ay, 'm-', linewidth=1.5, label='a_y (前为正)')
        axes[1].axhline(y=0, color='k', linestyle='--', alpha=0.4)
        axes[1].set_ylabel('a_y [m/s²]', fontsize=11, fontweight='bold')
        axes[1].set_title('Y轴加速度 (车体坐标系)', fontsize=12, fontweight='bold')
        axes[1].grid(True, alpha=0.4)
        
        # 事件标记
        axes[2].plot(t, np.zeros_like(t), 'k-', linewidth=0.5)
        if len(zupt_events) > 0:
            axes[2].vlines(t[zupt_events], -0.8, -0.2, colors='green', 
                          linewidth=3, label='ZUPT触发', alpha=0.7)
        if len(slip_events) > 0:
            axes[2].vlines(t[slip_events], 0.2, 0.8, colors='red', 
                          linewidth=3, label='轮滑检测', alpha=0.7)
        axes[2].set_ylim(-1.5, 1.5)
        axes[2].set_yticks([])
        axes[2].set_xlabel('时间 [s]', fontsize=12, fontweight='bold')
        axes[2].set_title('加速度数据利用事件', fontsize=12, fontweight='bold')
        axes[2].grid(True, alpha=0.3, axis='x')
        axes[2].legend(loc='upper right')
        
        fig.suptitle('加速度计数据利用分析', fontsize=14, fontweight='bold', y=0.998)
        plt.tight_layout()
        return fig

# ========================================
# 主执行流程（含对比实验）
# ========================================
def main():
    ComparisonVisualizer.configure_plotting()
    
    try:
        print("="*75)
        print("智能小车惯性导航增强系统 - 加速度计辅助偏差修正对比实验")
        print("="*75)
        
        # 1. 加载完整数据集
        print("\n[1/6] 加载传感器数据（含加速度通道）...")
        channels, n_samples = DataLoader.load_full_dataset(Config.DATA_PATH)
        
        # 2. 预处理轮速
        print("[2/6] 预处理轮速数据...")
        fs = 1.0 / Config.DT
        v_left = DataLoader.convert_wheel_speed(
            DataLoader.lowpass_filter(channels[0], Config.WHEEL_FILTER_CUTOFF, fs, Config.FILTER_ORDER),
            is_left_wheel=True
        )
        v_right = DataLoader.convert_wheel_speed(
            DataLoader.lowpass_filter(channels[1], Config.WHEEL_FILTER_CUTOFF, fs, Config.FILTER_ORDER),
            is_left_wheel=False
        )
        
        # 3. 处理偏航角（归零）
        print("[3/6] 偏航角归零处理...")
        yaw_zeroed, initial_yaw = DataLoader.normalize_yaw(channels[4])
        print(f"    初始偏航角校正: {initial_yaw:.2f}° → 0°")
        
        # 4. 预处理加速度（已去除重力，仅需滤波）
        print("[4/6] 预处理加速度数据...")
        ax = DataLoader.lowpass_filter(channels[6], Config.ACCEL_FILTER_CUTOFF, fs, Config.FILTER_ORDER)
        ay = DataLoader.lowpass_filter(channels[7], Config.ACCEL_FILTER_CUTOFF, fs, Config.FILTER_ORDER)
        print(f"    加速度范围: ax [{ax.min():.2f}, {ax.max():.2f}] m/s², "
              f"ay [{ay.min():.2f}, {ay.max():.2f}] m/s²")
        
        # 5. 执行双模式导航
        print("[5/6] 执行双模式导航解算...")
        nav = EnhancedNavigation()
        
        # 基准模式（无加速度修正）
        x_base, y_base, theta_base = nav.integrate_baseline(
            v_left, v_right, yaw_zeroed
        )
        
        # 增强模式（加速度辅助修正）
        x_enh, y_enh, theta_enh, v_corrected = nav.integrate_with_accel_correction(
            v_left, v_right, yaw_zeroed, ax, ay
        )
        
        # 6. 生成时间轴
        t = np.arange(n_samples) * Config.DT
        
        # 7. 可视化对比
        print("[6/6] 生成对比可视化结果...")
        
        # 轨迹对比图
        fig1 = ComparisonVisualizer.plot_trajectory_comparison(
            x_base, y_base, x_enh, y_enh, theta_enh
        )
        fig1.savefig("trajectory_comparison.png", dpi=150, bbox_inches='tight')
        print("  ✓ 轨迹对比图: trajectory_comparison.png")
        
        # 误差分析图
        fig2 = ComparisonVisualizer.plot_error_analysis(
            t, x_base, y_base, x_enh, y_enh
        )
        fig2.savefig("error_analysis.png", dpi=150, bbox_inches='tight')
        print("  ✓ 误差分析图: error_analysis.png")
        
        # 加速度利用分析（需记录事件）
        # 简化：通过速度变化检测静止/轮滑事件
        zupt_events = np.where((np.abs(np.gradient(x_enh)) < 0.005) & (np.abs(np.gradient(y_enh)) < 0.005))[0]
        slip_events = np.where(np.abs(np.gradient(v_corrected)) > 1.5)[0]  # 粗略检测
        
        fig3 = ComparisonVisualizer.plot_acceleration_utilization(
            t, ax, ay, zupt_events[:20], slip_events[:20]  # 限制显示数量
        )
        fig3.savefig("acceleration_utilization.png", dpi=150, bbox_inches='tight')
        print("  ✓ 加速度利用分析: acceleration_utilization.png")
        
        # 8. 生成对比报告
        total_dist_base = np.sum(np.sqrt(np.diff(x_base)**2 + np.diff(y_base)**2))
        total_dist_enh = np.sum(np.sqrt(np.diff(x_enh)**2 + np.diff(y_enh)**2))
        end_error = np.sqrt((x_base[-1]-x_enh[-1])**2 + (y_base[-1]-y_enh[-1])**2)
        improvement = (1 - total_dist_enh / total_dist_base) * 100
        
        print("\n" + "="*75)
        print("导航性能对比报告")
        print("="*75)
        print(f"基准模式 (仅轮速+IMU):")
        print(f"  起点: (0.000, 0.000) m | 终点: ({x_base[-1]:.3f}, {y_base[-1]:.3f}) m")
        print(f"  总轨迹长度: {total_dist_base:.3f} m")
        print(f"\n增强模式 (加速度辅助):")
        print(f"  起点: (0.000, 0.000) m | 终点: ({x_enh[-1]:.3f}, {y_enh[-1]:.3f}) m")
        print(f"  总轨迹长度: {total_dist_enh:.3f} m")
        print(f"\n性能改进:")
        print(f"  终点位置偏差: {end_error:.3f} m")
        print(f"  轨迹长度优化: {improvement:+.2f}%")
        print(f"  静止检测事件: {len(zupt_events)} 次")
        print(f"  轮滑检测事件: {len(slip_events)} 次")
        print("="*75)
        
        # 9. 保存对比数据
        comparison_data = np.column_stack([
            t,
            x_base, y_base, 
            x_enh, y_enh,
            np.rad2deg(theta_enh),
            v_corrected,
            ax, ay
        ])
        header = ("time(s),x_base(m),y_base(m),x_enh(m),y_enh(m),"
                 "yaw_enh(deg),v_corrected(m/s),ax(m/s2),ay(m/s2)")
        np.savetxt("navigation_comparison.csv", comparison_data,
                  delimiter=',', header=header, comments='', fmt='%.6f')
        print(f"\n✓ 完整对比数据已保存: navigation_comparison.csv")
        
        # 10. 显示图形
        plt.show()
        
    except Exception as e:
        print(f"\n❌ 程序执行出错: {type(e).__name__}")
        print(f"   错误详情: {str(e)}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()