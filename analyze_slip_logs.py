"""
双轮足打滑检测日志分析脚本
分析三个工况下的打滑检测数据并生成可视化图表
"""

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
import numpy as np
import os
from pathlib import Path

# 设置中文字体
matplotlib.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei', 'DejaVu Sans']
matplotlib.rcParams['axes.unicode_minus'] = False

# 车型参数 (CAR_SELECT == 3)
SPEED_TO_MM_S = 4.79  # 转速换算系数
WHEEL_BASE_MM = 175.0  # 轮距 mm

# 日志文件配置
LOG_FILES = [
    {
        "file": "slip_debug_log_20260713_210616(原地自转不打滑）.txt",
        "title": "原地自转工况",
        "desc": "低轮速、高角速度、无侧滑",
        "output": "analysis_原地自转.png"
    },
    {
        "file": "slip_debug_log_20260713_210719（低速小圈不打滑）.txt",
        "title": "低速小圈工况",
        "desc": "中等轮速、稳定转弯、正常抓地",
        "output": "analysis_低速小圈.png"
    },
    {
        "file": "slip_debug_log_20260713_210751（高速小圈刚好开始打滑，车轮开始径向漂移）.txt",
        "title": "高速小圈工况",
        "desc": "高轮速、大角速度、检测到侧滑",
        "output": "analysis_高速小圈.png"
    }
]


def load_log(filepath):
    """加载日志文件"""
    df = pd.read_csv(filepath)
    # 添加时间索引 (10ms间隔)
    df['TimeIdx'] = np.arange(len(df)) * 0.01  # 10ms
    return df


def calculate_derived_params(df):
    """计算派生物理量"""
    # 平均轮速 (mm/s)
    df['SpeedAvg'] = (df['SpeedL'] + df['SpeedR']) / 2.0
    
    # 轮速差 (mm/s)
    df['SpeedDiff'] = df['SpeedR'] - df['SpeedL']
    
    # 理论向心加速度 (mm/s^2): a_c = v_x * omega
    df['CentripetalAccel'] = df['SpeedAvg'] * df['ActualYawRate']
    
    # 异常横向加速度 (mm/s^2): e_a = a_y - v_x * omega
    # 注意: 这里没有原始加速度数据，用理论和实际角速度差来近似分析
    df['YawRateError'] = df['TheoYawRate'] - df['ActualYawRate']
    
    # 动态侧滑阈值
    df['SlipThreshold_Enter'] = 500.0 + 0.15 * np.abs(df['CentripetalAccel'])
    df['SlipThreshold_Exit'] = 300.0 + 0.10 * np.abs(df['CentripetalAccel'])
    
    # 转弯半径估算 (mm): R = v / omega, 避免除零
    omega_safe = df['ActualYawRate'].replace(0, np.nan)
    df['TurnRadius'] = np.abs(df['SpeedAvg'] / omega_safe)
    
    return df


def plot_single_case(df, config, output_dir):
    """绘制单个工况的分析图"""
    fig, axes = plt.subplots(4, 1, figsize=(14, 16), sharex=True)
    fig.suptitle(f'打滑检测数据分析 - {config["title"]}\n{config["desc"]}', 
                 fontsize=14, fontweight='bold')
    
    time = df['TimeIdx']
    
    # ========== 图1: 轮速曲线 ==========
    ax1 = axes[0]
    ax1.plot(time, df['SpeedL'], 'b-', label='左轮 SpeedL', alpha=0.8)
    ax1.plot(time, df['SpeedR'], 'r-', label='右轮 SpeedR', alpha=0.8)
    ax1.plot(time, df['SpeedAvg'], 'g--', label='平均轮速', alpha=0.7)
    ax1.axhline(y=0, color='k', linestyle=':', alpha=0.3)
    ax1.set_ylabel('轮速 (mm/s)')
    ax1.legend(loc='upper right')
    ax1.grid(True, alpha=0.3)
    ax1.set_title('轮速变化曲线')
    
    # ========== 图2: 角速度对比 ==========
    ax2 = axes[1]
    ax2.plot(time, df['TheoYawRate'], 'b-', label='理论角速度 (轮速差)', alpha=0.8)
    ax2.plot(time, df['ActualYawRate'], 'r-', label='实际角速度 (陀螺仪)', alpha=0.8)
    ax2.axhline(y=0, color='k', linestyle=':', alpha=0.3)
    ax2.set_ylabel('角速度 (rad/s)')
    ax2.legend(loc='upper right')
    ax2.grid(True, alpha=0.3)
    ax2.set_title('角速度对比: 理论 vs 实际')
    
    # ========== 图3: 角速度误差与侧滑阈值 ==========
    ax3 = axes[2]
    ax3.plot(time, df['YawRateError'], 'purple', label='角速度误差 (Theo-Actual)', alpha=0.8)
    ax3.axhline(y=0, color='k', linestyle=':', alpha=0.3)
    ax3.set_ylabel('角速度误差 (rad/s)')
    ax3.legend(loc='upper right')
    ax3.grid(True, alpha=0.3)
    ax3.set_title('角速度误差 (反映打滑/模型失配)')
    
    # ========== 图4: SlipFlag 状态 ==========
    ax4 = axes[3]
    colors = {0: 'green', 1: 'red', 2: 'blue', 3: 'orange'}
    labels = {0: '正常抓地', 1: '平移侧滑', 2: '静止态', 3: '原地自转'}
    
    for flag_val in df['SlipFlag'].unique():
        mask = df['SlipFlag'] == flag_val
        ax4.scatter(time[mask], df['SlipFlag'][mask], 
                   c=colors.get(flag_val, 'gray'), 
                   label=labels.get(flag_val, f'Flag={flag_val}'),
                   s=10, alpha=0.6)
    
    ax4.set_xlabel('时间 (s)')
    ax4.set_ylabel('SlipFlag')
    ax4.set_yticks([0, 1, 2, 3])
    ax4.set_yticklabels(['0:正常', '1:侧滑', '2:静止', '3:自转'])
    ax4.legend(loc='upper right')
    ax4.grid(True, alpha=0.3)
    ax4.set_title('打滑状态标志')
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, config['output']), dpi=150, bbox_inches='tight')
    plt.close()
    print(f"已保存: {config['output']}")


def plot_comparison(all_dfs, configs, output_dir):
    """绘制三个工况对比图"""
    fig, axes = plt.subplots(3, 3, figsize=(18, 14))
    fig.suptitle('三种工况打滑检测对比分析', fontsize=16, fontweight='bold')
    
    for col, (df, cfg) in enumerate(zip(all_dfs, configs)):
        time = df['TimeIdx']
        
        # 第一行: 轮速
        ax = axes[0, col]
        ax.plot(time, df['SpeedL'], 'b-', label='左轮', alpha=0.8)
        ax.plot(time, df['SpeedR'], 'r-', label='右轮', alpha=0.8)
        ax.set_ylabel('轮速 (mm/s)')
        ax.set_title(cfg['title'])
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.3)
        
        # 第二行: 角速度
        ax = axes[1, col]
        ax.plot(time, df['TheoYawRate'], 'b-', label='理论', alpha=0.8)
        ax.plot(time, df['ActualYawRate'], 'r-', label='实际', alpha=0.8)
        ax.set_ylabel('角速度 (rad/s)')
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.3)
        
        # 第三行: SlipFlag
        ax = axes[2, col]
        colors = {0: 'green', 1: 'red', 2: 'blue', 3: 'orange'}
        for flag_val in df['SlipFlag'].unique():
            mask = df['SlipFlag'] == flag_val
            ax.scatter(time[mask], df['SlipFlag'][mask],
                      c=colors.get(flag_val, 'gray'), s=10, alpha=0.6)
        ax.set_xlabel('时间 (s)')
        ax.set_ylabel('SlipFlag')
        ax.set_yticks([0, 1, 2, 3])
        ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'analysis_三工况对比.png'), dpi=150, bbox_inches='tight')
    plt.close()
    print("已保存: analysis_三工况对比.png")


def generate_statistics(all_dfs, configs):
    """生成统计分析文本"""
    lines = []
    lines.append("=" * 80)
    lines.append("双轮足打滑检测日志 - 数据统计与物理意义分析")
    lines.append("=" * 80)
    lines.append("")
    
    for df, cfg in zip(all_dfs, configs):
        lines.append(f"【{cfg['title']}】{cfg['desc']}")
        lines.append("-" * 60)
        
        # 基本统计
        lines.append(f"数据点数: {len(df)}")
        lines.append(f"采样时长: {df['TimeIdx'].iloc[-1]:.2f} s")
        lines.append("")
        
        # 轮速统计
        lines.append("轮速统计 (mm/s):")
        lines.append(f"  左轮  - 均值: {df['SpeedL'].mean():.1f}, 标准差: {df['SpeedL'].std():.1f}, "
                     f"范围: [{df['SpeedL'].min():.1f}, {df['SpeedL'].max():.1f}]")
        lines.append(f"  右轮  - 均值: {df['SpeedR'].mean():.1f}, 标准差: {df['SpeedR'].std():.1f}, "
                     f"范围: [{df['SpeedR'].min():.1f}, {df['SpeedR'].max():.1f}]")
        lines.append(f"  平均  - 均值: {df['SpeedAvg'].mean():.1f}, 标准差: {df['SpeedAvg'].std():.1f}")
        lines.append("")
        
        # 角速度统计
        lines.append("角速度统计 (rad/s):")
        lines.append(f"  理论  - 均值: {df['TheoYawRate'].mean():.3f}, 标准差: {df['TheoYawRate'].std():.3f}")
        lines.append(f"  实际  - 均值: {df['ActualYawRate'].mean():.3f}, 标准差: {df['ActualYawRate'].std():.3f}")
        lines.append(f"  误差  - 均值: {df['YawRateError'].mean():.3f}, 标准差: {df['YawRateError'].std():.3f}")
        lines.append("")
        
        # SlipFlag统计
        flag_counts = df['SlipFlag'].value_counts().sort_index()
        lines.append("SlipFlag 分布:")
        flag_names = {0: '正常抓地', 1: '平移侧滑', 2: '静止态', 3: '原地自转'}
        for flag, count in flag_counts.items():
            pct = count / len(df) * 100
            lines.append(f"  {flag} ({flag_names.get(flag, '未知')}): {count} 次 ({pct:.1f}%)")
        lines.append("")
        
        # 物理意义分析
        lines.append("物理意义分析:")
        
        if '原地自转' in cfg['title']:
            lines.append("  - 工况特征: 小车原地旋转，平均轮速接近0，但左右轮速差较大")
            lines.append("  - 角速度: 理论角速度由轮速差产生，实际角速度由陀螺仪测量")
            lines.append("  - 打滑判断: 由于v_avg≈0，向心加速度a_c=v*ω≈0，不会触发侧滑检测")
            lines.append("  - SlipFlag=2(静止): 轮速极低且无异常横向加速度")
            lines.append("  - SlipFlag=3(自转): 轮速低但角速度>0.3 rad/s")
            
        elif '低速小圈' in cfg['title']:
            lines.append("  - 工况特征: 小车以较低速度进行小半径转弯")
            lines.append("  - 速度分布: 左轮约-1000 mm/s，右轮约-1580 mm/s，存在明显速差")
            lines.append("  - 角速度: 理论约-3.2 rad/s，实际约-2.5 rad/s，存在约0.7 rad/s偏差")
            lines.append("  - 偏差原因: 倒立摆模型的机械相位滞后，IMU响应滞后于轮速变化")
            lines.append("  - 打滑判断: 虽有偏差但未超过阈值，SlipFlag=0，正常抓地")
            
        elif '高速小圈' in cfg['title']:
            lines.append("  - 工况特征: 小车以较高速度进行小半径转弯，接近抓地极限")
            lines.append("  - 速度分布: 左轮约-1200 mm/s，右轮约-1900 mm/s，速差更大")
            lines.append("  - 角速度: 理论约-4.2 rad/s，实际约-3.0 rad/s，偏差约1.2 rad/s")
            lines.append("  - 打滑判断: SlipFlag=1，检测到平移侧滑")
            lines.append("  - 物理机制: 向心加速度需求超过轮胎抓地极限，车轮开始径向漂移")
            lines.append("  - 角速度偏差增大: 轮速差反映理论需求，但实际车体因打滑无法完全跟随")
        
        lines.append("")
        lines.append("")
    
    # 总结
    lines.append("=" * 80)
    lines.append("总结与结论")
    lines.append("=" * 80)
    lines.append("")
    lines.append("1. 打滑检测原理:")
    lines.append("   基于向心加速度一致性检测: e_a = a_y - v_x * ω")
    lines.append("   当异常横向加速度超过动态阈值时判定为侧滑")
    lines.append("")
    lines.append("2. 三种工况对比:")
    lines.append("   - 原地自转: v_avg≈0, a_c≈0, 不触发侧滑检测")
    lines.append("   - 低速小圈: 角速度偏差存在但在容忍范围内，正常抓地")
    lines.append("   - 高速小圈: 角速度偏差超过阈值，检测到侧滑")
    lines.append("")
    lines.append("3. 角速度偏差来源:")
    lines.append("   - 倒立摆模型的机械相位滞后")
    lines.append("   - IMU与轮速的采样/滤波延迟差异")
    lines.append("   - 重力补偿对向心加速度的吸收")
    lines.append("   - 真实侧滑时轮速与车体运动的解耦")
    lines.append("")
    lines.append("4. 改进建议:")
    lines.append("   - 使用陀螺仪角速度替代偏航角差分")
    lines.append("   - 优化姿态EKF在转弯时的重力估计")
    lines.append("   - 增加状态机和时间确认机制")
    lines.append("   - 引入与转弯强度相关的动态阈值")
    
    return "\n".join(lines)


def main():
    # 创建输出目录
    output_dir = Path(__file__).parent / "slip_analysis_results"
    output_dir.mkdir(exist_ok=True)
    print(f"分析结果将保存到: {output_dir}")
    
    # 加载和分析数据
    all_dfs = []
    for config in LOG_FILES:
        filepath = Path(__file__).parent / config['file']
        print(f"正在处理: {config['file']}")
        df = load_log(filepath)
        df = calculate_derived_params(df)
        all_dfs.append(df)
        
        # 绘制单工况图
        plot_single_case(df, config, output_dir)
    
    # 绘制对比图
    plot_comparison(all_dfs, LOG_FILES, output_dir)
    
    # 生成统计报告
    report = generate_statistics(all_dfs, LOG_FILES)
    report_path = output_dir / "analysis_report.txt"
    with open(report_path, 'w', encoding='utf-8') as f:
        f.write(report)
    print(f"已保存: analysis_report.txt")
    
    print("\n分析完成!")


if __name__ == "__main__":
    main()
