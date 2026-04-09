from manim import *
import numpy as np

class InertialNavigation(Scene):
    def construct(self):
        # ================= 1. 全局水印与片头 =================
        watermark = Text("同济大学-锦鲤队", font_size=20, color=GRAY_B).to_corner(UL)
        self.add(watermark)

        title = Text("惯性导航原理展示", font_size=32, weight=BOLD).to_edge(UP)
        self.play(Write(title))

        # ================= 2. 核心数学代码框 (右侧) =================
        code_str = """// Inertial Nav: Coordinate Transform
void InertialNav_Update(...) {
    // 1. Slip Detection (Yaw Rate Diff)
    if (abs(w_theo - w_actual) > THRES) {
        nav.slip_flag = 1; // Drifting!
    } else {
        nav.slip_flag = 0;
    }

    // 2. Velocity Fusion & Lateral Accel
    nav.vx_body = alpha*v_wheel + (1-alpha)*v_pred;
    nav.vy_body = nav.vy_body*0.95 + acc_lat*dt;

    // 3. Body to World Frame (Rotation)
    float vx_w = vx*cos(yaw) - vy*sin(yaw);
    float vy_w = vx*sin(yaw) + vy*cos(yaw);

    // 4. Integral to Position
    nav.X += vx_w * dt;
    nav.Y += vy_w * dt;
}"""
        code_text = Text(
            code_str,
            font="Consolas", font_size=14,
            t2c={
                "void": ORANGE, "float": ORANGE, "if": ORANGE, "else": ORANGE,
                "nav.slip_flag": RED_B, "nav.vx_body": GREEN_C, "nav.vy_body": RED_C,
                "cos": BLUE, "sin": BLUE, "abs": BLUE,
                "nav.X": PURPLE_B, "nav.Y": PURPLE_B
            }
        )
        code_bg = SurroundingRectangle(code_text, color=DARK_GRAY, fill_color="#272822", fill_opacity=1, buff=0.3)
        code_group = VGroup(code_bg, code_text).to_edge(RIGHT).shift(DOWN*0.5)
        self.play(FadeIn(code_group))

        # ================= 3. 构造小车物理模型与世界网格 =================
        # 背景网格 (世界坐标系)
        grid = NumberPlane(
            x_range=[-10, 10, 1], y_range=[-10, 10, 1],
            background_line_style={"stroke_color": TEAL, "stroke_width": 1, "stroke_opacity": 0.3}
        ).scale(0.8)
        self.play(FadeIn(grid))

        chassis = RoundedRectangle(corner_radius=0.1, width=0.8, height=1.2, color=BLUE, fill_opacity=0.6)
        wheel_style = {"width": 0.2, "height": 0.4, "color": GRAY, "fill_opacity": 1}
        fl_wheel = Rectangle(**wheel_style).move_to(chassis.get_corner(UL) + RIGHT*0.1 + DOWN*0.2)
        fr_wheel = Rectangle(**wheel_style).move_to(chassis.get_corner(UR) + LEFT*0.1 + DOWN*0.2)
        rl_wheel = Rectangle(**wheel_style).move_to(chassis.get_corner(DL) + RIGHT*0.1 + UP*0.2)
        rr_wheel = Rectangle(**wheel_style).move_to(chassis.get_corner(DR) + LEFT*0.1 + UP*0.2)
        heading_arrow = Triangle(color=YELLOW, fill_opacity=1).scale(0.15).move_to(chassis.get_top() + DOWN*0.2)
        
        car = VGroup(chassis, fl_wheel, fr_wheel, rl_wheel, rr_wheel, heading_arrow)
        self.add(car) # 初始位置由 Updater 接管

        # ================= 4. 物理系统状态追踪器 =================
        # 将起点设置在真实世界坐标 (X=1500mm, Y=-1500mm)
        X_track = ValueTracker(1500.0) 
        Y_track = ValueTracker(-1500.0)
        Yaw_track = ValueTracker(90.0) # 90度代表车头朝左 (-X方向)
        
        Vx_body_track = ValueTracker(0.0)
        Vy_body_track = ValueTracker(0.0)
        Slip_track = ValueTracker(0)

        # ================= 5. 实时监控面板 (左侧严格对齐) =================
        monitor_title = Text("▶ 惯导解算寄存器", font_size=20, color=YELLOW)
        
        labels = VGroup(
            Text("World X =", font_size=18, color=PURPLE_B),
            Text("World Y =", font_size=18, color=PURPLE_B),
            Text("Yaw (deg) =", font_size=18),
            Text("vx_body =", font_size=18, color=GREEN_C),
            Text("vy_body =", font_size=18, color=RED_C),
            Text("slip_flag =", font_size=18, color=WHITE)
        ).arrange(DOWN, aligned_edge=RIGHT, buff=0.35) 
        
        values = VGroup(
            DecimalNumber(0, num_decimal_places=2, font_size=18, color=PURPLE_B),
            DecimalNumber(0, num_decimal_places=2, font_size=18, color=PURPLE_B),
            DecimalNumber(0, num_decimal_places=1, font_size=18),
            DecimalNumber(0, num_decimal_places=2, font_size=18, color=GREEN_C),
            DecimalNumber(0, num_decimal_places=2, font_size=18, color=RED_C),
            Integer(0, font_size=18, color=WHITE)
        )

        for i in range(len(labels)):
            values[i].next_to(labels[i], RIGHT, buff=0.4)
            values[i].align_to(labels[i], DOWN)

        monitor_panel = VGroup(labels, values)
        monitor_group = VGroup(monitor_title, monitor_panel).arrange(DOWN, aligned_edge=LEFT, buff=0.4).to_edge(LEFT, buff=0.5).align_to(code_group, UP)
        
        self.play(FadeIn(monitor_group))

        # ================= 6. 添加矢量箭头与轨迹 =================
        vx_arrow = Arrow(start=ORIGIN, end=UP, color=GREEN_C, buff=0, max_tip_length_to_length_ratio=0.2)
        vy_arrow = Arrow(start=ORIGIN, end=LEFT, color=RED_C, buff=0, max_tip_length_to_length_ratio=0.2)
        
        def update_vectors(v_grp):
            car_center = car.get_center()
            yaw_rad = Yaw_track.get_value() * DEGREES
            
            forward_vec = np.array([-np.sin(yaw_rad), np.cos(yaw_rad), 0]) 
            left_vec = np.array([-np.cos(yaw_rad), -np.sin(yaw_rad), 0])
            
            vx_val = Vx_body_track.get_value()
            vy_val = Vy_body_track.get_value()
            
            if abs(vx_val) > 0.1:
                vx_arrow.put_start_and_end_on(car_center, car_center + forward_vec * (vx_val / 500.0))
                vx_arrow.set_opacity(1)
            else:
                vx_arrow.set_opacity(0)
                
            if abs(vy_val) > 0.1:
                vy_arrow.put_start_and_end_on(car_center, car_center + left_vec * (vy_val / 500.0))
                vy_arrow.set_opacity(1)
            else:
                vy_arrow.set_opacity(0)

        vectors = VGroup(vx_arrow, vy_arrow)
        vectors.add_updater(update_vectors)
        self.add(vectors)

        trace = TracedPath(car.get_center, stroke_width=4, stroke_color=YELLOW)
        self.add(trace)

        # ================= 7. 物理积分引擎 (Updater) =================
        VISUAL_SCALE = 0.0015 # 完美缩放：1000mm 的真实距离对应屏幕上约 1.5 个格子

        def physics_engine(mob, dt):
            vx = Vx_body_track.get_value()
            vy = Vy_body_track.get_value()
            yaw_rad = Yaw_track.get_value() * DEGREES
            
            # 真实世界的航位推算解算
            dx_world = vx * (-np.sin(yaw_rad)) - vy * np.cos(yaw_rad)
            dy_world = vx * np.cos(yaw_rad) - vy * np.sin(yaw_rad)
            
            new_x = X_track.get_value() + dx_world * dt
            new_y = Y_track.get_value() + dy_world * dt
            
            X_track.set_value(new_x)
            Y_track.set_value(new_y)
            
            # 映射到屏幕坐标
            vis_x = new_x * VISUAL_SCALE
            vis_y = new_y * VISUAL_SCALE
            car.move_to(np.array([vis_x, vis_y, 0]))
            car.set_rotation(yaw_rad)

            # 更新数据面板
            values[0].set_value(X_track.get_value()) 
            values[1].set_value(Y_track.get_value())
            values[2].set_value(Yaw_track.get_value())
            values[3].set_value(vx)
            values[4].set_value(vy)
            values[5].set_value(Slip_track.get_value())
            
            if Slip_track.get_value() == 1:
                values[5].set_color(RED)
                labels[5].set_color(RED)
            else:
                values[5].set_color(WHITE)
                labels[5].set_color(WHITE)

        car.add_updater(physics_engine)

        # ================= 8. 场景动画控制 (曲线规划) =================
        
        # 【阶段 A】 起步直行 (朝左)
        self.play(Vx_body_track.animate.set_value(1000.0), run_time=1.0)
        
        # 【阶段 B】 平滑大曲线过弯 (向右转，Yaw角均匀变化)
        # rate_func=linear 保证角速度恒定，形成完美的圆弧轨迹
        self.play(
            Yaw_track.animate.set_value(0.0), # 90度变0度，车头转向朝上
            run_time=2.5, rate_func=linear
        )

        # 【阶段 C】 极速极限急转弯 -> 触发侧滑漂移 (Drift!)
        self.play(Slip_track.animate.set_value(1), run_time=0.1)
        self.play(
            Yaw_track.animate.set_value(-135.0),    # 极快地猛打方向盘到右下角
            Vy_body_track.animate.set_value(800.0), # 巨大的侧向离心速度 (甩尾)
            Vx_body_track.animate.set_value(600.0), # 纵向掉速
            run_time=1.5, rate_func=linear
        )
        
        # 【阶段 D】 修正反打方向，恢复抓地力
        self.play(Slip_track.animate.set_value(0), run_time=0.1)
        self.play(
            Yaw_track.animate.set_value(-180.0),    # 车头回正朝下
            Vy_body_track.animate.set_value(0.0),   # 侧滑恢复为0
            Vx_body_track.animate.set_value(1200.0),# 出弯加速
            run_time=1.5
        )
        self.wait(1)

        # 【阶段 E】 刹车停止
        self.play(Vx_body_track.animate.set_value(0.0), run_time=1)
        self.wait(2)