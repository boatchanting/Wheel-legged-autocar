import math
import pygame
from pygame.locals import *

from OpenGL.GL import *
from OpenGL.GLU import *


# =========================
# 全局配置
# =========================
TARGET_WIDTH = 752
TARGET_HEIGHT = 480
SSAA_FACTOR = 2  # 2倍超采样抗锯齿
WINDOW_WIDTH = TARGET_WIDTH * SSAA_FACTOR
WINDOW_HEIGHT = TARGET_HEIGHT * SSAA_FACTOR

FOV = 45.0
NEAR_CLIP = 50.0
FAR_CLIP = 8000.0

MOVE_STEP = 10.0   # mm
ROT_STEP = 1.0     # deg

# 颜色定义
COLORS = {
    "rgb": {
        "ground": (0.75, 0.10, 0.10),   # 红色操场
        "white":  (0.95, 0.95, 0.95),
        "blue":   (0.20, 0.30, 0.85),
        "dark":   (0.20, 0.20, 0.20),
    },
    "gray": {
        "ground": (0.15, 0.15, 0.15),
        "white":  (0.85, 0.85, 0.85),
        "blue":   (0.45, 0.45, 0.45),
        "dark":   (0.20, 0.20, 0.20),
    }
}

# 浅灰色边框线
OUTLINE_COLOR = (0.80, 0.80, 0.80)
OUTLINE_WIDTH = 2.0


# =========================
# 尺寸（单位：mm）
# =========================
W_WIDTH = 500.0  # 全部结构宽度（x方向）

# 结构长度分段（z方向）
L_LEFT_TOTAL = 625.0
L_BLUE = 500.0
L_RIGHT = 500.0
L_TOTAL = L_LEFT_TOTAL + L_BLUE + L_RIGHT  # 1625

# 左段内部：顶平台 250mm，斜坡水平投影 375mm
L_LEFT_TOP = 250.0
L_LEFT_SLOPE_RUN = L_LEFT_TOTAL - L_LEFT_TOP  # 375

# 高度（三级：50/100/150）
H_RIGHT_TOP = 50.0
H_BLUE_TOP = 100.0
H_LEFT_TOP = 150.0

# 斜坡角（由 run/rise 推出，约 21.8° ~ 22°）
SLOPE_ANGLE_DEG = math.degrees(math.atan2(H_LEFT_TOP, L_LEFT_SLOPE_RUN))


# =========================
# 文本渲染
# =========================
class TextRenderer:
    def __init__(self):
        pygame.font.init()
        self.font = pygame.font.SysFont("Consolas", int(24 * SSAA_FACTOR), bold=True)

    def render(self, text_lines, x, y):
        # 2D 正交投影绘制文字
        glMatrixMode(GL_PROJECTION)
        glPushMatrix()
        glLoadIdentity()
        gluOrtho2D(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0)

        glMatrixMode(GL_MODELVIEW)
        glPushMatrix()
        glLoadIdentity()

        glDisable(GL_DEPTH_TEST)
        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

        current_y = y
        for line in text_lines:
            surf = self.font.render(line, True, (255, 255, 0, 255))
            text_data = pygame.image.tostring(surf, "RGBA", True)
            w, h = surf.get_size()
            glRasterPos2d(x, current_y + h)
            glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, text_data)
            current_y += h + 5

        glDisable(GL_BLEND)
        glEnable(GL_DEPTH_TEST)

        glMatrixMode(GL_PROJECTION)
        glPopMatrix()
        glMatrixMode(GL_MODELVIEW)
        glPopMatrix()


# =========================
# 相机
# =========================
class Camera:
    def __init__(self):
        # 初始位置（单位mm）
        self.x, self.y, self.z = 0.0, 500.0, 2200.0
        self.pitch, self.yaw, self.roll = -18.0, 0.0, 0.0

    def apply_transform(self):
        glRotatef(-self.roll,  0, 0, 1)
        glRotatef(-self.pitch, 1, 0, 0)
        glRotatef(-self.yaw,   0, 1, 0)
        glTranslatef(-self.x, -self.y, -self.z)

    def look_at(self, tx, ty, tz):
        """
        让相机朝向目标点（简化版），保持 roll=0
        """
        dx = tx - self.x
        dy = ty - self.y
        dz = tz - self.z

        self.yaw = math.degrees(math.atan2(dx, -dz))
        dist_xz = math.sqrt(dx * dx + dz * dz)
        self.pitch = -math.degrees(math.atan2(dy, dist_xz))
        self.roll = 0.0


# =========================
# OpenGL 初始化
# =========================
def init_gl():
    glEnable(GL_DEPTH_TEST)

    # 抗锯齿（MSAA + 多边形/线条平滑）
    glEnable(GL_MULTISAMPLE)
    glEnable(GL_POLYGON_SMOOTH)
    glEnable(GL_BLEND)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST)

    glEnable(GL_LINE_SMOOTH)
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST)

    glDisable(GL_CULL_FACE)  # 不裁剪，便于调试

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(FOV, (WINDOW_WIDTH / WINDOW_HEIGHT), NEAR_CLIP, FAR_CLIP)
    glMatrixMode(GL_MODELVIEW)


# =========================
# 绘制工具：盒子（带边框线）
# =========================
def draw_box_faces(xc, y0, zc, w, l, h, face_colors):
    hx = w * 0.5
    hz = l * 0.5
    y1 = y0 + h

    p = {
        "fbl": (xc - hx, y0, zc + hz),
        "fbr": (xc + hx, y0, zc + hz),
        "ftr": (xc + hx, y1, zc + hz),
        "ftl": (xc - hx, y1, zc + hz),

        "bbl": (xc - hx, y0, zc - hz),
        "bbr": (xc + hx, y0, zc - hz),
        "btr": (xc + hx, y1, zc - hz),
        "btl": (xc - hx, y1, zc - hz),
    }

    # ---------- 1) 实体（用 polygon offset 避免和线框Z-fighting） ----------
    glEnable(GL_POLYGON_OFFSET_FILL)
    glPolygonOffset(1.0, 1.0)

    glBegin(GL_QUADS)
    # top
    glColor3f(*face_colors.get("top", (1, 1, 1)))
    glVertex3f(*p["ftl"]); glVertex3f(*p["ftr"]); glVertex3f(*p["btr"]); glVertex3f(*p["btl"])
    # bottom
    glColor3f(*face_colors.get("bottom", (1, 1, 1)))
    glVertex3f(*p["fbl"]); glVertex3f(*p["bbl"]); glVertex3f(*p["bbl"]); glVertex3f(*p["fbr"])  # placeholder fix below
    glEnd()

    # 上面 bottom 面那行写错会导致渲染异常，这里重新正确绘制 bottom
    glBegin(GL_QUADS)
    glColor3f(*face_colors.get("bottom", (1, 1, 1)))
    glVertex3f(*p["fbl"]); glVertex3f(*p["bbl"]); glVertex3f(*p["bbr"]); glVertex3f(*p["fbr"])
    glEnd()

    # 其余面
    glBegin(GL_QUADS)
    # left
    glColor3f(*face_colors.get("left", (1, 1, 1)))
    glVertex3f(*p["fbl"]); glVertex3f(*p["ftl"]); glVertex3f(*p["btl"]); glVertex3f(*p["bbl"])
    # right
    glColor3f(*face_colors.get("right", (1, 1, 1)))
    glVertex3f(*p["fbr"]); glVertex3f(*p["bbr"]); glVertex3f(*p["btr"]); glVertex3f(*p["ftr"])
    # front
    glColor3f(*face_colors.get("front", (1, 1, 1)))
    glVertex3f(*p["fbl"]); glVertex3f(*p["fbr"]); glVertex3f(*p["ftr"]); glVertex3f(*p["ftl"])
    # back
    glColor3f(*face_colors.get("back", (1, 1, 1)))
    glVertex3f(*p["bbl"]); glVertex3f(*p["btl"]); glVertex3f(*p["btr"]); glVertex3f(*p["bbr"])
    glEnd()

    glDisable(GL_POLYGON_OFFSET_FILL)

    # ---------- 2) 线框边框 ----------
    glColor3f(*OUTLINE_COLOR)
    glLineWidth(OUTLINE_WIDTH)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)

    glBegin(GL_QUADS)
    # top
    glVertex3f(*p["ftl"]); glVertex3f(*p["ftr"]); glVertex3f(*p["btr"]); glVertex3f(*p["btl"])
    # bottom
    glVertex3f(*p["fbl"]); glVertex3f(*p["bbl"]); glVertex3f(*p["bbr"]); glVertex3f(*p["fbr"])
    # left
    glVertex3f(*p["fbl"]); glVertex3f(*p["ftl"]); glVertex3f(*p["btl"]); glVertex3f(*p["bbl"])
    # right
    glVertex3f(*p["fbr"]); glVertex3f(*p["bbr"]); glVertex3f(*p["btr"]); glVertex3f(*p["ftr"])
    # front
    glVertex3f(*p["fbl"]); glVertex3f(*p["fbr"]); glVertex3f(*p["ftr"]); glVertex3f(*p["ftl"])
    # back
    glVertex3f(*p["bbl"]); glVertex3f(*p["btl"]); glVertex3f(*p["btr"]); glVertex3f(*p["bbr"])
    glEnd()

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)
    glLineWidth(1.0)


# =========================
# 绘制工具：斜坡三棱柱（带边框线）
# =========================
def draw_ramp_prism(z_start, z_end, width, h_end, color):
    hx = width * 0.5

    A_bl = (-hx, 0.0, z_start)
    A_br = ( hx, 0.0, z_start)

    B_bl = (-hx, 0.0, z_end)
    B_br = ( hx, 0.0, z_end)

    B_tl = (-hx, h_end, z_end)
    B_tr = ( hx, h_end, z_end)

    # ---------- 1) 实体 ----------
    glEnable(GL_POLYGON_OFFSET_FILL)
    glPolygonOffset(1.0, 1.0)
    glColor3f(*color)

    # 顶面（斜面）
    glBegin(GL_QUADS)
    glVertex3f(*A_bl); glVertex3f(*A_br); glVertex3f(*B_tr); glVertex3f(*B_tl)
    glEnd()

    # 底面
    glBegin(GL_QUADS)
    glVertex3f(*A_bl); glVertex3f(*B_bl); glVertex3f(*B_br); glVertex3f(*A_br)
    glEnd()

    # 右侧面（三角）
    glBegin(GL_TRIANGLES)
    glVertex3f(*A_br); glVertex3f(*B_br); glVertex3f(*B_tr)
    glEnd()

    # 左侧面（三角）
    glBegin(GL_TRIANGLES)
    glVertex3f(*A_bl); glVertex3f(*B_tl); glVertex3f(*B_bl)
    glEnd()

    # 末端竖直面
    glBegin(GL_QUADS)
    glVertex3f(*B_bl); glVertex3f(*B_br); glVertex3f(*B_tr); glVertex3f(*B_tl)
    glEnd()

    glDisable(GL_POLYGON_OFFSET_FILL)

    # ---------- 2) 边框线 ----------
    glColor3f(*OUTLINE_COLOR)
    glLineWidth(OUTLINE_WIDTH)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)

    glBegin(GL_QUADS)
    glVertex3f(*A_bl); glVertex3f(*A_br); glVertex3f(*B_tr); glVertex3f(*B_tl)
    glEnd()

    glBegin(GL_QUADS)
    glVertex3f(*A_bl); glVertex3f(*B_bl); glVertex3f(*B_br); glVertex3f(*A_br)
    glEnd()

    glBegin(GL_TRIANGLES)
    glVertex3f(*A_br); glVertex3f(*B_br); glVertex3f(*B_tr)
    glEnd()

    glBegin(GL_TRIANGLES)
    glVertex3f(*A_bl); glVertex3f(*B_tl); glVertex3f(*B_bl)
    glEnd()

    glBegin(GL_QUADS)
    glVertex3f(*B_bl); glVertex3f(*B_br); glVertex3f(*B_tr); glVertex3f(*B_tl)
    glEnd()

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)
    glLineWidth(1.0)


# =========================
# 场景绘制：三级坡道
# =========================
def draw_scene(mode):
    p = COLORS[mode]
    c_ground = p["ground"]
    c_white = p["white"]
    c_blue = p["blue"]

    # 地面（用一个薄盒子模拟大平面）
    draw_box_faces(
        xc=0.0, y0=-5.0, zc=0.0,
        w=6000.0, l=6000.0, h=5.0,
        face_colors={"top": c_ground, "bottom": c_ground, "left": c_ground, "right": c_ground, "front": c_ground, "back": c_ground}
    )

    # 计算整体z布局：左->右
    z0 = -L_TOTAL * 0.5
    z_left_end = z0 + L_LEFT_TOTAL
    z_blue_end = z_left_end + L_BLUE
    z_right_end = z_blue_end + L_RIGHT

    # 左段：斜坡（0 -> 150） + 顶平台（250mm）
    z_slope_end = z0 + L_LEFT_SLOPE_RUN
    z_top_center = (z_slope_end + z_left_end) * 0.5

    # 斜坡（白色）
    draw_ramp_prism(
        z_start=z0,
        z_end=z_slope_end,
        width=W_WIDTH,
        h_end=H_LEFT_TOP,
        color=c_white
    )

    # 顶平台（白色盒子）：高度 150
    draw_box_faces(
        xc=0.0, y0=0.0, zc=z_top_center,
        w=W_WIDTH, l=L_LEFT_TOP, h=H_LEFT_TOP,
        face_colors={"top": c_white, "bottom": c_white, "left": c_white, "right": c_white, "front": c_white, "back": c_white}
    )

    # 中段：蓝色台阶（高度 100）
    z_blue_center = (z_left_end + z_blue_end) * 0.5
    draw_box_faces(
        xc=0.0, y0=0.0, zc=z_blue_center,
        w=W_WIDTH, l=L_BLUE, h=H_BLUE_TOP,
        face_colors={
            "top": c_blue,       # 顶面蓝
            "bottom": c_white,
            "left": c_blue,      # 侧面蓝
            "right": c_blue,
            "front": c_white,    # 正面白
            "back": c_white      # 后面白
        }
    )

    # 右段：白色平台（高度 50）
    z_right_center = (z_blue_end + z_right_end) * 0.5
    draw_box_faces(
        xc=0.0, y0=0.0, zc=z_right_center,
        w=W_WIDTH, l=L_RIGHT, h=H_RIGHT_TOP,
        face_colors={"top": c_white, "bottom": c_white, "left": c_white, "right": c_white, "front": c_white, "back": c_white}
    )


# =========================
# 截图（保存不含UI）
# =========================
def save_image_optimized(cam, mode):
    glPixelStorei(GL_PACK_ALIGNMENT, 1)
    raw = glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE)

    surface = pygame.image.fromstring(raw, (WINDOW_WIDTH, WINDOW_HEIGHT), "RGB")
    surface = pygame.transform.flip(surface, False, True)
    final_surface = pygame.transform.smoothscale(surface, (TARGET_WIDTH, TARGET_HEIGHT))

    fname = (f"img_{mode}_"
             f"X{int(cam.x)}_Y{int(cam.y)}_Z{int(cam.z)}_"
             f"P{int(cam.pitch)}_Y{int(cam.yaw)}_R{int(cam.roll)}.png")
    pygame.image.save(final_surface, fname)
    print(f"[已保存 - 无UI] {fname}")


# =========================
# 主循环
# =========================
def main():
    pygame.init()

    # 请求多重采样缓冲区（MSAA 4x）
    pygame.display.gl_set_attribute(pygame.GL_MULTISAMPLEBUFFERS, 1)
    pygame.display.gl_set_attribute(pygame.GL_MULTISAMPLESAMPLES, 4)

    pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT), DOUBLEBUF | OPENGL)
    pygame.display.set_caption("三级坡道仿真 - pygame + OpenGL（带浅灰边框线）")

    init_gl()
    cam = Camera()
    text_renderer = TextRenderer()

    mode = "rgb"
    running = True
    snapshot_request = False
    clock = pygame.time.Clock()

    # 初始朝向坡道中心
    cam.look_at(0.0, 80.0, 0.0)

    while running:
        # --- 事件处理 ---
        for event in pygame.event.get():
            if event.type == QUIT:
                running = False

            if event.type == KEYDOWN:
                if event.key == K_ESCAPE:
                    running = False
                if event.key == K_SPACE:
                    snapshot_request = True
                if event.key == K_m:
                    mode = "gray" if mode == "rgb" else "rgb"
                if event.key == K_r:
                    cam.x, cam.y, cam.z = 0.0, 500.0, 2200.0
                    cam.pitch, cam.yaw, cam.roll = -18.0, 0.0, 0.0
                    cam.look_at(0.0, 80.0, 0.0)

        # --- 运动控制 ---
        keys = pygame.key.get_pressed()

        # 平移
        if keys[K_a]: cam.x -= MOVE_STEP
        if keys[K_d]: cam.x += MOVE_STEP
        if keys[K_q]: cam.y += MOVE_STEP
        if keys[K_e]: cam.y -= MOVE_STEP
        if keys[K_w]: cam.z -= MOVE_STEP
        if keys[K_s]: cam.z += MOVE_STEP

        # 旋转
        if keys[K_i]: cam.pitch += ROT_STEP
        if keys[K_k]: cam.pitch -= ROT_STEP
        if keys[K_j]: cam.yaw += ROT_STEP
        if keys[K_l]: cam.yaw -= ROT_STEP
        if keys[K_u]: cam.roll += ROT_STEP
        if keys[K_o]: cam.roll -= ROT_STEP

        # --- 渲染 ---
        glClearColor(0.20, 0.20, 0.20, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glLoadIdentity()

        # 3D场景
        cam.apply_transform()
        draw_scene(mode)

        # 截图（在UI之前）
        if snapshot_request:
            save_image_optimized(cam, mode)
            snapshot_request = False

        # UI
        infos = [
            f"[MODE]: {mode.upper()}",
            f"POS(mm): {cam.x:.0f}, {cam.y:.0f}, {cam.z:.0f}",
            f"ROT(deg): P{cam.pitch:.0f}, Y{cam.yaw:.0f}, R{cam.roll:.0f}",
            f"Slope: run={L_LEFT_SLOPE_RUN:.0f}mm, rise={H_LEFT_TOP:.0f}mm, angle~{SLOPE_ANGLE_DEG:.1f}deg",
            "[WASD/QE] Move   [IJKL/UO] Rotate",
            "[M] Gray/RGB   [SPACE] Save   [R] Reset"
        ]
        text_renderer.render(infos, 20, 20)

        pygame.display.flip()
        clock.tick(60)

    pygame.quit()


if __name__ == "__main__":
    main()
