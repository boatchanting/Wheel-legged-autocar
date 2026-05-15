import csv
import os
import struct
import sys
import threading
import time

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
NAV_MARKER_DIR = os.path.normpath(os.path.join(CURRENT_DIR, "..", "webview_nav_marker"))
if NAV_MARKER_DIR not in sys.path:
    sys.path.insert(0, NAV_MARKER_DIR)

import nav_marker_host as base

PAYLOAD_SIZE_V3 = 96
PAYLOAD_SIZE_V4 = 106
BASE_DECODE_PAYLOAD = base._decode_payload


def _decode_payload(payload_bytes):
    data = BASE_DECODE_PAYLOAD(payload_bytes)
    if data is None:
        return None

    size = len(payload_bytes)

    if size >= PAYLOAD_SIZE_V3:
        gps_x, gps_y, gps_valid, gps_origin_set = struct.unpack_from("<ffBB", payload_bytes, base.PAYLOAD_SIZE_V2)
        data["gps_x"] = gps_x
        data["gps_y"] = gps_y
        data["gps_valid"] = gps_valid
        data["gps_origin_set"] = gps_origin_set
    else:
        data["gps_x"] = data["nav_x"]
        data["gps_y"] = data["nav_y"]
        data["gps_valid"] = int(bool(data.get("state", 0)))
        data["gps_origin_set"] = data["gps_valid"]

    if size >= PAYLOAD_SIZE_V4:
        cf_x, cf_y, cf_ready, cf_gnss_ready = struct.unpack_from("<ffBB", payload_bytes, PAYLOAD_SIZE_V3)
        data["cf_x"] = cf_x
        data["cf_y"] = cf_y
        data["cf_ready"] = cf_ready
        data["cf_gnss_ready"] = cf_gnss_ready
        if cf_ready:
            data["nav_x"] = cf_x
            data["nav_y"] = cf_y
    else:
        data["cf_x"] = data["nav_x"]
        data["cf_y"] = data["nav_y"]
        data["cf_ready"] = 0
        data["cf_gnss_ready"] = 0

    data["payload_size"] = size
    return data


base._decode_payload = _decode_payload


class Api(base.Api):
    def export_mark_points_csv(self, points):
        try:
            if not isinstance(points, list) or not points:
                return {"success": False, "msg": "No mark points to export."}

            count = len(points)
            filename = f"cf_mark_points_{time.strftime('%Y%m%d_%H%M%S')}.csv"
            filepath = os.path.join(CURRENT_DIR, filename)

            with open(filepath, "w", newline="", encoding="utf-8-sig") as f:
                writer = csv.writer(f)
                writer.writerow([
                    "total_count",
                    "start_heading",
                    "index",
                    "x",
                    "y",
                    "relative_yaw",
                    "heading",
                    "point_type",
                ])

                for i, item in enumerate(points):
                    idx = int(item.get("index", i))
                    x = float(item.get("x", 0.0))
                    y = float(item.get("y", 0.0))
                    relative_yaw = base._normalize_relative_yaw_deg(item.get("relative_yaw", 0.0))
                    heading_deg = base._normalize_heading_deg(item.get("heading", 0.0))
                    point_type = int(item.get("point_type", 0))
                    if relative_yaw is None:
                        relative_yaw = 0.0
                    if heading_deg is None:
                        heading_deg = 0.0

                    writer.writerow([
                        count,
                        "",
                        idx,
                        f"{x:.3f}",
                        f"{y:.3f}",
                        f"{relative_yaw:.3f}",
                        f"{heading_deg:.3f}",
                        point_type,
                    ])

            return {
                "success": True,
                "msg": f"Exported: {filepath} (start_heading=NA)",
                "path": filepath,
                "start_heading": None,
            }
        except Exception as exc:
            return {"success": False, "msg": f"Export failed: {exc}"}


if __name__ == "__main__":
    thread = threading.Thread(target=base.tcp_server_thread, daemon=True)
    thread.start()

    html_path = os.path.join(CURRENT_DIR, "cf_marker.html")
    if not os.path.exists(html_path):
        raise FileNotFoundError(f"HTML not found: {html_path}")

    api = Api()
    window = base.webview.create_window(
        title="互补滤波打点上位机 (WebView)",
        url=html_path,
        js_api=api,
        width=1480,
        height=920,
        min_size=(1120, 700),
    )
    base.webview.start(debug=False)
