"""科目一极速掉头路径生成的回归测试。

这个文件只在电脑端运行，用来验证 caculate_path.py 的离线路径生成逻辑：
原打点方式不变，第一个业务点是掉头点，后面的业务点仍然是桩桶点。
它不会被烧录到车上，也不会参与实际导航运行。
"""

import importlib.util
import os
from pathlib import Path
import sys
import tempfile
import unittest


os.environ.setdefault("MPLBACKEND", "Agg")

# 直接加载同目录下的 caculate_path.py，避免中文目录名影响普通 import。
SCRIPT_PATH = Path(__file__).with_name("caculate_path.py")
spec = importlib.util.spec_from_file_location("caculate_path", SCRIPT_PATH)
caculate_path = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = caculate_path
spec.loader.exec_module(caculate_path)


def rp(x, y):
    """构造测试用的普通路径点，减少每个用例里的重复样板代码。"""
    return caculate_path.RoutePoint(
        x=float(x),
        y=float(y),
        target_yaw_deg=0.0,
        heading_deg=0.0,
        target_speed=0.0,
        point_type=0,
    )


class FastUTurnPathTest(unittest.TestCase):
    """验证极速掉头路径拓扑、自动掉头线、动作点选择和速度方向。"""

    def setUp(self):
        # 每个用例开始前保存全局配置，避免一个测试影响另一个测试。
        self._old_fast_enable = caculate_path.PLAN1_FAST_UTURN_ENABLE
        self._old_fast_mode = caculate_path.PLAN1_FAST_UTURN_MODE
        self._old_fast_over = caculate_path.PLAN1_FAST_UTURN_LINE_OVER_MM
        self._old_mark_width = caculate_path.PLAN1_FAST_UTURN_MARK_WIDTH_MM
        self._old_width_factor = caculate_path.PLAN1_FAST_UTURN_LINE_WIDTH_FACTOR

    def tearDown(self):
        # 测试结束后恢复全局配置，保证用例之间互相隔离。
        caculate_path.PLAN1_FAST_UTURN_ENABLE = self._old_fast_enable
        caculate_path.PLAN1_FAST_UTURN_MODE = self._old_fast_mode
        caculate_path.PLAN1_FAST_UTURN_LINE_OVER_MM = self._old_fast_over
        caculate_path.PLAN1_FAST_UTURN_MARK_WIDTH_MM = self._old_mark_width
        caculate_path.PLAN1_FAST_UTURN_LINE_WIDTH_FACTOR = self._old_width_factor

    def test_fast_uturn_keeps_original_marking_semantics_and_builds_virtual_line(self):
        """第一个业务点仍是掉头点，后面业务点仍是桩桶；掉头线由程序自动生成。"""
        caculate_path.PLAN1_FAST_UTURN_MARK_WIDTH_MM = 1000.0
        caculate_path.PLAN1_FAST_UTURN_LINE_WIDTH_FACTOR = 1.25
        points = [rp(0, 0), rp(1000, 0), rp(1600, 300), rp(2400, 0)]

        layout = caculate_path.classify_fast_uturn_points(points)

        self.assertEqual(layout.start, points[0])
        self.assertEqual(layout.u_turn, points[1])
        self.assertEqual(layout.cones, [points[2]])
        self.assertEqual(layout.end_point, points[3])
        self.assertAlmostEqual(layout.line_center[0], 1000.0)
        self.assertAlmostEqual(layout.line_center[1], 0.0)

        line_len = caculate_path.math.hypot(
            layout.line_b.x - layout.line_a.x,
            layout.line_b.y - layout.line_a.y,
        )
        self.assertAlmostEqual(line_len, 1250.0)

    def test_fast_uturn_action_point_is_chosen_for_post_turn_path_not_line_center_only(self):
        """动作点应根据后续绕桩接入方向选择，不应固定在线中心正前方。"""
        caculate_path.PLAN1_FAST_UTURN_MARK_WIDTH_MM = 1000.0
        caculate_path.PLAN1_FAST_UTURN_LINE_WIDTH_FACTOR = 1.25
        points = [rp(0, 0), rp(1000, 0), rp(1600, 300), rp(2400, 0)]

        layout = caculate_path.classify_fast_uturn_points(points)

        self.assertAlmostEqual(layout.action_point[0], 1100.0)
        self.assertGreater(layout.action_point[1], 100.0)
        self.assertLessEqual(abs(layout.action_lateral_offset), 625.0)

    def test_fast_uturn_generated_path_drives_to_selected_action_point(self):
        """生成路径的起步直冲段应朝选出来的动作点走，而不是强行冲线中心。"""
        caculate_path.PLAN1_FAST_UTURN_MARK_WIDTH_MM = 1000.0
        caculate_path.PLAN1_FAST_UTURN_LINE_WIDTH_FACTOR = 1.25
        points = [rp(0, 0), rp(1000, 0), rp(1600, 300), rp(2400, 0)]

        control, x_vals, y_vals, _drop, action_idx = caculate_path.generate_fast_uturn_calculated_path(points)

        self.assertAlmostEqual(x_vals[action_idx], 1100.0, delta=1e-3)
        self.assertGreater(y_vals[action_idx], 100.0)
        self.assertIn((1100.0, round(y_vals[action_idx], 1)), [(round(x, 1), round(y, 1)) for x, y in control])

    def test_reverse_mode_uses_positive_speed_and_reverse_yaw_after_action_point(self):
        """急刹倒车模式下，动作点之后使用正速度指令，并把目标 yaw 反向 180 度。"""
        points = [rp(0, 0), rp(1000, 0), rp(1600, 300), rp(2400, 0)]
        _control, x_vals, y_vals, drop, action_idx = caculate_path.generate_fast_uturn_calculated_path(points)
        final_points = caculate_path.build_fast_uturn_final_points(points, x_vals, y_vals, drop, action_idx)

        caculate_path.apply_speed_plan(final_points, reverse_start_idx=action_idx)

        self.assertLess(final_points[max(0, action_idx - 1)].target_speed, 0.0)
        self.assertGreater(final_points[action_idx].target_speed, 0.0)

        forward_yaw = caculate_path.tangent_yaws(
            caculate_path.np.array([p.x for p in final_points]),
            caculate_path.np.array([p.y for p in final_points]),
        )[action_idx]
        expected_reverse_yaw = caculate_path.normalize_relative_yaw_deg(forward_yaw + 180.0)
        yaw_err = caculate_path.normalize_relative_yaw_deg(
            final_points[action_idx].target_yaw_deg - expected_reverse_yaw
        )
        self.assertLess(abs(yaw_err), 1e-3)

    def test_fast_uturn_config_reads_enable_mode_width_and_over_line_distance(self):
        """确认脚本能从 sys_options.h 风格的宏里读取极速掉头配置。"""
        config = "\n".join(
            [
                "#define PLAN1_FAST_UTURN_ENABLE 1",
                "#define PLAN1_FAST_UTURN_MODE_BRAKE_REVERSE 2",
                "#define PLAN1_FAST_UTURN_MODE PLAN1_FAST_UTURN_MODE_BRAKE_REVERSE",
                "#define PLAN1_FAST_UTURN_LINE_OVER_MM 150.0f",
                "#define PLAN1_FAST_UTURN_MARK_WIDTH_MM 1200.0f",
                "#define PLAN1_FAST_UTURN_LINE_WIDTH_FACTOR 1.25f",
            ]
        )

        with tempfile.NamedTemporaryFile("w", encoding="utf-8", delete=False) as tmp:
            tmp.write(config)
            tmp_path = tmp.name

        try:
            options = caculate_path.load_plan1_fast_uturn_options(Path(tmp_path))
        finally:
            os.unlink(tmp_path)

        self.assertEqual(options["enable"], 1)
        self.assertEqual(options["mode"], caculate_path.PLAN1_FAST_UTURN_MODE_BRAKE_REVERSE)
        self.assertAlmostEqual(options["line_over_mm"], 150.0)
        self.assertAlmostEqual(options["mark_width_mm"], 1200.0)
        self.assertAlmostEqual(options["line_width_factor"], 1.25)

    def test_generate_route_plan_uses_fast_uturn_and_adjusts_reverse_index_after_origin_insert(self):
        """输入未包含原点时，补原点后仍要把倒车起点映射到最终输出点序。"""
        points = [rp(1000, 0), rp(1600, 300), rp(2400, 0)]
        caculate_path.PLAN1_FAST_UTURN_ENABLE = 1
        caculate_path.PLAN1_FAST_UTURN_MODE = caculate_path.PLAN1_FAST_UTURN_MODE_BRAKE_REVERSE

        _control_points, final_points, method_name = caculate_path.generate_route_plan(points)

        action_indices = [
            idx
            for idx, point in enumerate(final_points)
            if point.point_type == caculate_path.POINT_TYPE_TOKENS["NAV_POINT_JUMP"]
        ]
        self.assertEqual(len(action_indices), 1)
        action_idx = action_indices[0]
        self.assertGreater(action_idx, 0)
        self.assertIn("急刹倒车", method_name)
        self.assertLess(final_points[action_idx - 1].target_speed, 0.0)
        self.assertGreater(final_points[action_idx].target_speed, 0.0)

    def test_fast_uturn_preview_cones_skip_only_the_uturn_point(self):
        """预览图里的桩桶安全圈应跳过第一个掉头点，从第二个业务点开始。"""
        points = [rp(0, 0), rp(1000, 0), rp(1600, 300), rp(2400, 0)]
        caculate_path.PLAN1_FAST_UTURN_ENABLE = 1

        cone_points = caculate_path.preview_cone_points(points)

        self.assertEqual(cone_points, points[2:])


if __name__ == "__main__":
    unittest.main()
