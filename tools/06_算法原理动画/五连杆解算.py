from manim import *
import numpy as np

class FiveBarKinematics(Scene):
    def construct(self):
        # ================= 全局水印 =================
        # 你的车队标识，常驻左上角
        watermark = Text("同济大学-锦鲤队", font_size=20, color=GRAY_B).to_corner(UL)
        self.add(watermark)

        # ================= 场景1：片头与变量定义 =================
        title = Text("五连杆逆运动学与查表解算", font_size=40, weight=BOLD).to_edge(UP)
        self.play(Write(title))

        # 参数定义区 (匹配 C 代码)，放置在水印下方
        params_text = VGroup(
            Text("L1 (左小腿): 6.0", font_size=24),
            Text("L2 (左大腿): 9.0", font_size=24),
            Text("L3 (右大腿): 9.0", font_size=24),
            Text("L4 (右小腿): 6.0", font_size=24),
            Text("L5 (舵机间距): 3.7", font_size=24)
        ).arrange(DOWN, aligned_edge=LEFT).next_to(watermark, DOWN, buff=0.8, aligned_edge=LEFT)
        
        self.play(FadeIn(params_text, shift=RIGHT))

        # ================= 场景2：五连杆物理模型构建 =================
        # 物理常量
        L1, L2, L3, L4, L5 = 6.0, 9.0, 9.0, 6.0, 3.7
        # 缩放比例，让其在屏幕上更好看
        scale = 0.35 
        
        # 舵机基准点
        O1 = np.array([-L5/2 * scale, 1.5, 0]) # 左舵机
        O2 = np.array([L5/2 * scale, 1.5, 0])  # 右舵机

        # 运动学追踪器 (P: 高度, A: 角度)
        P_tracker = ValueTracker(8.0) 
        A_tracker = ValueTracker(0.0)

        # 辅助 IK 求解函数 (用于动画绘制)
        def get_knee_pos(base, end, l_top, l_bot, is_left):
            dist = np.linalg.norm(end - base)
            dist = min(dist, l_top * scale + l_bot * scale - 0.01)
            theta = np.arctan2(end[1] - base[1], end[0] - base[0])
            cos_val = ((l_top*scale)**2 + dist**2 - (l_bot*scale)**2) / (2 * (l_top*scale) * dist)
            alpha = np.arccos(np.clip(cos_val, -1.0, 1.0))
            if is_left:
                angle = theta - alpha
            else:
                angle = theta + alpha
            return base + np.array([np.cos(angle), np.sin(angle), 0]) * (l_top * scale)

        # 末端点 (足端) updater
        foot_dot = Dot(color=RED, radius=0.1)
        foot_dot.add_updater(
            lambda m: m.move_to(
                np.array([
                    1.5 * np.sin(A_tracker.get_value() * DEGREES), 
                    1.5 - P_tracker.get_value() * scale * np.cos(A_tracker.get_value() * DEGREES), 
                    0
                ])
            )
        )

        # 绘制节点和连杆
        servo1_dot = Dot(O1, color=BLUE).scale(1.5)
        servo2_dot = Dot(O2, color=BLUE).scale(1.5)
        
        link1 = Line(color=YELLOW, stroke_width=6)
        link2 = Line(color=TEAL, stroke_width=6)
        link3 = Line(color=TEAL, stroke_width=6)
        link4 = Line(color=YELLOW, stroke_width=6)
        base_link = Line(O1, O2, color=GRAY, stroke_width=8)

        link1.add_updater(lambda m: m.put_start_and_end_on(O1, get_knee_pos(O1, foot_dot.get_center(), L1, L2, True)))
        link2.add_updater(lambda m: m.put_start_and_end_on(get_knee_pos(O1, foot_dot.get_center(), L1, L2, True), foot_dot.get_center()))
        link4.add_updater(lambda m: m.put_start_and_end_on(O2, get_knee_pos(O2, foot_dot.get_center(), L4, L3, False)))
        link3.add_updater(lambda m: m.put_start_and_end_on(get_knee_pos(O2, foot_dot.get_center(), L4, L3, False), foot_dot.get_center()))

        leg_group = VGroup(base_link, link1, link2, link3, link4, servo1_dot, servo2_dot, foot_dot)
        self.play(Create(leg_group), run_time=2)

        # UI 仪表盘 (放置在画面右上角)
        dashboard = VGroup(
            Text("控制输入", font_size=28, color=BLUE),
            DecimalNumber(0, unit=" cm", color=RED).add_updater(lambda m: m.set_value(P_tracker.get_value())),
            DecimalNumber(0, unit=r"^\circ", color=GREEN).add_updater(lambda m: m.set_value(A_tracker.get_value()))
        ).arrange(DOWN, aligned_edge=LEFT).to_corner(UR) # 紧贴右上角
        
        dash_labels = VGroup(
            Text(" ", font_size=28),
            Text("P (高度): ", font_size=24),
            Text("A (角度): ", font_size=24)
        ).arrange(DOWN, aligned_edge=RIGHT)
        dash_labels.next_to(dashboard, LEFT)
        
        self.play(FadeIn(dashboard, dash_labels))

        # 演示运动
        self.play(P_tracker.animate.set_value(14.5), run_time=1.5)
        self.play(P_tracker.animate.set_value(2.7), run_time=1.5)
        self.play(P_tracker.animate.set_value(10.0), run_time=1)
        self.play(A_tracker.animate.set_value(20.0), run_time=1.5)
        self.play(A_tracker.animate.set_value(-20.0), run_time=1.5)
        self.play(A_tracker.animate.set_value(0.0), run_time=1)

        # ================= 场景3：查表算法解算 =================
        self.play(
            # 将机械腿移动到左下角，为右侧的代码和图表让出空间
            leg_group.animate.scale(0.6).to_corner(DL).shift(UP*0.5 + RIGHT*0.5),
            FadeOut(params_text)
        )

        # 1. 首先构建查表网格 (放在右侧中间位置)
        grid = NumberPlane(
            x_range=[-2.5, 2.5, 1], 
            y_range=[-2.5, 2.5, 1],
            background_line_style={
                "stroke_color": BLUE_D,
                "stroke_width": 2,
                "stroke_opacity": 0.5
            }
        ).scale(0.5).to_edge(RIGHT).shift(LEFT*1) # 贴靠右边缘，高度居中

        grid_title = Text("二维查表: pwm_table_1", font_size=20).next_to(grid, UP)
        x_label = Text("Angle Index", font_size=16, color=GREEN).next_to(grid, DOWN)
        y_label = Text("P Index", font_size=16, color=RED).next_to(grid, LEFT).rotate(90*DEGREES)

        # 2. 然后构建代码框 (放在右侧最底部)
        code_str = """int p_index = round((p - P_min) / (P_max - P_min) * (num_legs - 1));
int a_index = round((degree - A_min) / (A_max - A_min) * (num_angles - 1));
int16 raw_val = pwm_table_1[p_index][a_index];
pwm_angle = raw_val - pwm_high;"""

        code_text = Text(
            code_str,
            font="Consolas",
            font_size=18,
            t2c={
                "int ": ORANGE,
                "int16 ": ORANGE,
                "round": BLUE,
                "pwm_table_1": GREEN_C,
                "P_min": RED_B,
                "P_max": RED_B,
                "A_min": TEAL_C,
                "A_max": TEAL_C,
                "num_legs": PURPLE_B,
                "num_angles": PURPLE_B
            }
        )
        code_bg = SurroundingRectangle(code_text, color=DARK_GRAY, fill_color="#272822", fill_opacity=1, buff=0.3)
        # 放置在右下角，确保在网格下方
        code_group = VGroup(code_bg, code_text).to_corner(DR).shift(UP*0.2 + LEFT*0.2)
        
        # 将代码和图表一起显示出来
        self.play(
            FadeIn(grid, grid_title, x_label, y_label),
            FadeIn(code_bg), 
            Write(code_text), 
            run_time=2
        )

        # 游标高亮查表过程
        cursor_box = Square(side_length=grid.x_axis.unit_size, color=YELLOW, fill_opacity=0.5)
        
        def update_cursor(mob):
            y_val = interpolate(-2, 2, (P_tracker.get_value() - 2.7) / (14.5 - 2.7))
            x_val = interpolate(-2, 2, (A_tracker.get_value() - (-20)) / (40))
            y_val_discrete = round(y_val)
            x_val_discrete = round(x_val)
            mob.move_to(grid.c2p(x_val_discrete, y_val_discrete))

        cursor_box.add_updater(update_cursor)
        self.play(FadeIn(cursor_box))

        # 最终综合联动演示
        self.play(P_tracker.animate.set_value(14.0), A_tracker.animate.set_value(15.0), run_time=2)
        self.play(P_tracker.animate.set_value(4.0), A_tracker.animate.set_value(-15.0), run_time=2)
        self.play(P_tracker.animate.set_value(8.0), A_tracker.animate.set_value(0.0), run_time=1.5)

        self.wait(2)