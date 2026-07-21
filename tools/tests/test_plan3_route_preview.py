"""科目三补点完成路线回显的回归测试。"""

from pathlib import Path
import sys
import tempfile
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
PLAN3_DIR = REPO_ROOT / "tools" / "webview_nav_marker速度规划_科目三"
sys.path.insert(0, str(PLAN3_DIR))

from plan3_route_builder import generate_route_artifacts  # noqa: E402


class Plan3RoutePreviewTests(unittest.TestCase):
    def test_generated_artifacts_return_completed_route_for_preview(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_path = Path(temp_dir)
            anchors_csv = temp_path / "anchors.csv"
            anchors_csv.write_text(
                "total_count,start_heading,index,x,y,relative_yaw,heading,point_type\n"
                "2,120.0,0,0.0,0.0,0.0,120.0,0\n"
                "2,120.0,1,600.0,0.0,-65.0,88.0,4\n",
                encoding="utf-8-sig",
            )

            result = generate_route_artifacts(
                anchors_csv,
                300.0,
                temp_path / "completed.csv",
                temp_path / "nav_replay_route_table.h",
            )

        self.assertEqual(
            [(point["x"], point["point_type"]) for point in result["route_points"]],
            [(0.0, 0), (300.0, 0), (600.0, 4)],
        )

    def test_page_draws_completed_route_returned_by_host(self):
        host = (PLAN3_DIR / "nav_marker_host.py").read_text(encoding="utf-8-sig")
        page = (PLAN3_DIR / "nav_marker.html").read_text(encoding="utf-8-sig")

        self.assertIn('"route_points"', host)
        self.assertIn("drawGeneratedRoute", page)
        self.assertIn("result.route_points", page)
        self.assertIn("if(p.y>maxY)maxY=p.y", page)
        self.assertGreaterEqual(page.count("clearGeneratedRoute()"), 6)


if __name__ == "__main__":
    unittest.main(verbosity=2)
