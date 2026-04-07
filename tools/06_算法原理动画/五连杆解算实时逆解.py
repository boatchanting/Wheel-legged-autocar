from manim import *
import numpy as np

class FiveBarRealTimeIK(Scene):
    def construct(self):
        # ================= 全局水印 =================
        watermark = Text("同济大学-锦鲤队", font_size=20, color=GRAY_B).to_corner(UL)
        self.add(watermark)

        # ================= 场景1：片头与变量定义 =================
        title = Text("五连杆逆运动学：浮点实时解算", font_size=36, weight=BOLD).to_edge(UP)
        self.play(Write(title))

        # 参数定义区
        params_text = VGroup(
            Text("L1 (左小腿): 6.0", font_size=24),
            Text("L2 (左大腿): 9.0", font_size=24),
            Text("L3 (右大腿): 9.0", font_size=24),
            Text("L4 (右小腿): 6.0", font_size=24),
            Text("L5 (舵机间距): 3.7", font_size=24)
        ).arrange(DOWN, aligned_edge=LEFT).next_to(watermark, DOWN, buff=0.8, aligned_edge=LEFT)
        
        self.play(FadeIn(params_text, shift=RIGHT))

        # ================= 场景2：五连杆物理模型构建 =================
        L1, L2, L3, L4, L5 = 6.0, 9.0, 9.0, 6.0, 3.7
        scale = 0.35 
        
        # 舵机基准点 (居中放置)
        O1 = np.array([-L5/2 * scale, 1.5, 0]) 
        O2 = np.array([L5/2 * scale, 1.5, 0])  

        P_tracker = ValueTracker(8.0) 
        A_tracker = ValueTracker(0.0)

        # 辅助 IK 求解函数
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

        # 足端 Updater
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

        servo1_dot = Dot(O1, color=BLUE).scale(1.5)
        servo2_dot = Dot(O2, color=BLUE).scale(1.5)
        
        link1 = Line(color=YELLOW, stroke_width=6)
        link2 = Line(color=TEAL, stroke_width=6)
        link3 = Line(color=TEAL, stroke_width=6)
        link4 = Line(color=YELLOW, stroke_width=6)
        base_link = Line(O1, O2, color=GRAY, stroke_width=8)

        # 连杆 Updater
        link1.add_updater(lambda m: m.put_start_and_end_on(O1, get_knee_pos(O1, foot_dot.get_center(), L1, L2, True)))
        link2.add_updater(lambda m: m.put_start_and_end_on(get_knee_pos(O1, foot_dot.get_center(), L1, L2, True), foot_dot.get_center()))
        link4.add_updater(lambda m: m.put_start_and_end_on(O2, get_knee_pos(O2, foot_dot.get_center(), L4, L3, False)))
        link3.add_updater(lambda m: m.put_start_and_end_on(get_knee_pos(O2, foot_dot.get_center(), L4, L3, False), foot_dot.get_center()))

        leg_group = VGroup(base_link, link1, link2, link3, link4, servo1_dot, servo2_dot, foot_dot)
        self.play(Create(leg_group), run_time=2)

        # 演示初始动作
        self.play(P_tracker.animate.set_value(14.5), run_time=1)
        self.play(P_tracker.animate.set_value(8.0), run_time=1)

        # ================= 场景3：进入解算展示界面 =================
        self.play(FadeOut(params_text))

        # 1. 绘制虚线辅助线 (物理涵义展示)
        dashed_line_L = DashedLine(O1, foot_dot.get_center(), color=WHITE, stroke_opacity=0.5)
        dashed_line_R = DashedLine(O2, foot_dot.get_center(), color=WHITE, stroke_opacity=0.5)
        dashed_line_L.add_updater(lambda m: m.put_start_and_end_on(servo1_dot.get_center(), foot_dot.get_center()))
        dashed_line_R.add_updater(lambda m: m.put_start_and_end_on(servo2_dot.get_center(), foot_dot.get_center()))
        self.play(Create(dashed_line_L), Create(dashed_line_R))

        # 2. C语言解算代码
        code_str = """// Real-Time IK (High CPU Load)
void calc_ik(float x, float y) {
    // 1. Pythagoras for diagonal dist
    float dx = x - SERVO_L_X;
    float dist_L = sqrt(dx*dx + y*y);
    
    // 2. Cosine rule for inner angle
    float cos_L = (L1*L1 + dist_L*dist_L - L2*L2) 
                  / (2 * L1 * dist_L);
    float alpha_L = acos(cos_L);
    
    // 3. Base angle and final theta
    float beta_L = atan2(y, dx);
    theta_L = beta_L - alpha_L;
}"""
        code_text = Text(
            code_str,
            font="Consolas", font_size=15,
            t2c={
                "float": ORANGE, "void": ORANGE,
                "sqrt": BLUE, "acos": BLUE, "atan2": BLUE,
                "dist_L": GREEN_C, "alpha_L": RED_B, "beta_L": RED_B, "theta_L": PURPLE_B
            }
        )
        code_bg = SurroundingRectangle(code_text, color=DARK_GRAY, fill_color="#272822", fill_opacity=1, buff=0.3)
        code_group = VGroup(code_bg, code_text).to_edge(RIGHT).shift(DOWN*0.5)
        self.play(FadeIn(code_group), run_time=1.5)

        # 3. 布局彻底修复：基于行的逐一严格对齐仪表盘
        monitor_title = Text("▶ 实时寄存器监控面板", font_size=20, color=YELLOW)
        
        # 将标签靠右排列 (aligned_edge=RIGHT)，让所有的等号完美对齐成一条竖线
        labels = VGroup(
            Text("Target X =", font_size=18),
            Text("Target Y =", font_size=18),
            Text("dist_L =", font_size=18),
            Text("alpha_L =", font_size=18),
            Text("theta_L =", font_size=18, color=PURPLE_B)
        ).arrange(DOWN, aligned_edge=RIGHT, buff=0.35) 
        
        values = VGroup(
            DecimalNumber(0, num_decimal_places=3, font_size=18),
            DecimalNumber(0, num_decimal_places=3, font_size=18),
            DecimalNumber(0, num_decimal_places=3, font_size=18),
            DecimalNumber(0, num_decimal_places=3, font_size=18),
            DecimalNumber(0, num_decimal_places=3, font_size=18, color=PURPLE_B)
        )

        # 逐行强制绑定：数字跟在等号后面，并且基准线(DOWN)完全对齐！不可能再错位了。
        for i in range(len(labels)):
            values[i].next_to(labels[i], RIGHT, buff=0.5)
            values[i].align_to(labels[i], DOWN)

        monitor_panel = VGroup(labels, values)
        
        # 将面板和标题组合，放在画面左侧
        monitor_group = VGroup(monitor_title, monitor_panel).arrange(DOWN, aligned_edge=LEFT, buff=0.4).to_edge(LEFT, buff=0.8).align_to(code_group, UP)
        
        self.play(FadeIn(monitor_group))

        # ================= 绑定实时解算的 Updater =================
        def update_values(mob):
            target_x = P_tracker.get_value() * np.sin(A_tracker.get_value() * DEGREES)
            target_y = P_tracker.get_value() * np.cos(A_tracker.get_value() * DEGREES)
            
            dx_l = target_x - (-L5/2)
            dy_l = target_y
            
            dist_l = np.sqrt(dx_l**2 + dy_l**2)
            dist_l = min(dist_l, L1 + L2 - 0.01) 
            
            cos_l = (L1**2 + dist_l**2 - L2**2) / (2 * L1 * dist_l)
            alpha_l = np.arccos(np.clip(cos_l, -1.0, 1.0))
            beta_l = np.arctan2(dy_l, dx_l)
            
            theta_l = beta_l - alpha_l

            values[0].set_value(target_x)
            values[1].set_value(target_y)
            values[2].set_value(dist_l)
            values[3].set_value(alpha_l * 180 / np.pi) 
            values[4].set_value(theta_l * 180 / np.pi) 

        values.add_updater(update_values)

        # ================= 最终高潮：疯狂的实时运动 =================
        self.play(
            P_tracker.animate.set_value(14.0),
            A_tracker.animate.set_value(15.0),
            run_time=2, rate_func=there_and_back
        )
        self.play(
            P_tracker.animate.set_value(5.0),
            A_tracker.animate.set_value(-15.0),
            run_time=2
        )
        self.play(
            P_tracker.animate.set_value(12.0),
            A_tracker.animate.set_value(20.0),
            run_time=1.5
        )
        self.play(
            P_tracker.animate.set_value(8.0),
            A_tracker.animate.set_value(0.0),
            run_time=1.5
        )

        self.wait(2)