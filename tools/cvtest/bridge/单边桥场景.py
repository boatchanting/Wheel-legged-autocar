import pygame
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *
import math
import random

# =========================
# 全局配置
# =========================
TARGET_WIDTH = 752
TARGET_HEIGHT = 480
SSAA_FACTOR = 2
WINDOW_WIDTH = TARGET_WIDTH * SSAA_FACTOR
WINDOW_HEIGHT = TARGET_HEIGHT * SSAA_FACTOR

FOV = 45.0
NEAR_CLIP = 10.0
FAR_CLIP = 10000.0

MOVE_STEP = 2.0
ROT_STEP = 1.0

# =========================
# 场景尺寸（单位：cm）
# =========================
BRIDGE_LEN = 300.0   # 3 x 100cm
BRIDGE_W   = 45.0
BRIDGE_THICK = 2.0

# 护栏：防“绕行”的视觉约束,0表示没有
RAIL_H = 0
RAIL_T = 0

# 楔形路障（满足：高度<=5cm，长度<=20cm，宽度>=22.5cm）
WEDGE_H = 5.0
WEDGE_L = 20.0
WEDGE_W = 27.0

# 间隔要求：10~25cm（指楔形之间的“空隙”，非中心距）
GAP_MIN = 10.0
GAP_MAX = 25.0

# 生成路障时的前后安全边距（避免贴桥头桥尾）
MARGIN_Z = 25.0

# 固定只要 3 个单边桥
WEDGE_COUNT = 3

# 颜色
COLORS = {
    'rgb': {
        'ground': (0.70, 0.10, 0.10),   # 红色地面
        'bridge': (0.90, 0.90, 0.90),   # 白色桥面
        'rail':   (0.85, 0.85, 0.85),   # 护栏略灰
        'wedge':  (0.05, 0.05, 0.05),   # 黑色楔形
        'mark':   (0.10, 0.10, 0.10),   # 分段线
    },
    'gray': {
        'ground': (0.15, 0.15, 0.15),
        'bridge': (0.55, 0.55, 0.55),
        'rail':   (0.50, 0.50, 0.50),
        'wedge':  (0.10, 0.10, 0.10),
        'mark':   (0.25, 0.25, 0.25),
    }
}

# =========================
# 文本渲染
# =========================
class TextRenderer:
    def __init__(self):
        self.font = pygame.font.SysFont('Consolas', 24 * SSAA_FACTOR, bold=True)

    def render(self, text_lines, x, y):
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
            text_surface = self.font.render(line, True, (255, 255, 0, 255))
            text_data = pygame.image.tostring(text_surface, "RGBA", True)
            w, h = text_surface.get_size()
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
        self.x, self.y, self.z = 0.0, 45.0, 320.0
        self.pitch, self.yaw, self.roll = -30.0, 0.0, 0.0

    def apply_transform(self):
        glRotatef(-self.roll,  0, 0, 1)
        glRotatef(-self.pitch, 1, 0, 0)
        glRotatef(-self.yaw,   0, 1, 0)
        glTranslatef(-self.x, -self.y, -self.z)

    def look_at_bridge_center(self):
        tx, ty, tz = 0.0, (0.5 + BRIDGE_THICK) + 5.0, 0.0
        dx = tx - self.x
        dy = ty - self.y
        dz = tz - self.z

        self.roll = 0.0
        self.yaw = math.degrees(math.atan2(dx, -dz))
        dist_xz = math.sqrt(dx*dx + dz*dz)
        if dist_xz > 1e-6:
            self.pitch = -math.degrees(math.atan2(dy, dist_xz))

# =========================
# OpenGL 初始化
# =========================
def init_gl():
    glEnable(GL_DEPTH_TEST)

    glEnable(GL_MULTISAMPLE)
    glEnable(GL_POLYGON_SMOOTH)
    glEnable(GL_BLEND)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST)

    glDisable(GL_CULL_FACE)

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(FOV, (WINDOW_WIDTH / WINDOW_HEIGHT), NEAR_CLIP, FAR_CLIP)
    glMatrixMode(GL_MODELVIEW)

# =========================
# 基础几何绘制
# =========================
def draw_box(cx, cy, cz, w, h, l, color):
    glColor3f(*color)
    hw, hh, hl = w/2, h/2, l/2

    x0, x1 = cx - hw, cx + hw
    y0, y1 = cy - hh, cy + hh
    z0, z1 = cz - hl, cz + hl

    glBegin(GL_QUADS)
    # +Y 顶
    glVertex3f(x0, y1, z1); glVertex3f(x1, y1, z1); glVertex3f(x1, y1, z0); glVertex3f(x0, y1, z0)
    # -Y 底
    glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0); glVertex3f(x1, y0, z1); glVertex3f(x0, y0, z1)
    # +Z
    glVertex3f(x0, y0, z1); glVertex3f(x1, y0, z1); glVertex3f(x1, y1, z1); glVertex3f(x0, y1, z1)
    # -Z
    glVertex3f(x0, y1, z0); glVertex3f(x1, y1, z0); glVertex3f(x1, y0, z0); glVertex3f(x0, y0, z0)
    # +X
    glVertex3f(x1, y0, z0); glVertex3f(x1, y1, z0); glVertex3f(x1, y1, z1); glVertex3f(x1, y0, z1)
    # -X
    glVertex3f(x0, y0, z1); glVertex3f(x0, y1, z1); glVertex3f(x0, y1, z0); glVertex3f(x0, y0, z0)
    glEnd()

def draw_ground(color):
    glColor3f(*color)
    size = 4000.0
    y = 0.0
    glBegin(GL_QUADS)
    glVertex3f(-size, y, -size)
    glVertex3f( size, y, -size)
    glVertex3f( size, y,  size)
    glVertex3f(-size, y,  size)
    glEnd()

def draw_bridge(mode_color):
    bridge_color = mode_color['bridge']
    rail_color   = mode_color['rail']
    mark_color   = mode_color['mark']

    base_y = 0.5
    top_y  = base_y + BRIDGE_THICK
    draw_box(0.0, base_y + BRIDGE_THICK/2, 0.0, BRIDGE_W, BRIDGE_THICK, BRIDGE_LEN, bridge_color)

    # 护栏
    rail_y = top_y + RAIL_H/2
    x_left  = -(BRIDGE_W/2 + RAIL_T/2)
    x_right = +(BRIDGE_W/2 + RAIL_T/2)
    draw_box(x_left,  rail_y, 0.0, RAIL_T, RAIL_H, BRIDGE_LEN, rail_color)
    draw_box(x_right, rail_y, 0.0, RAIL_T, RAIL_H, BRIDGE_LEN, rail_color)

    # 100cm 分段线
    glColor3f(*mark_color)
    glLineWidth(3.0)
    y_line = top_y + 0.05
    for k in [-100.0, 0.0, 100.0]:
        glBegin(GL_LINES)
        glVertex3f(-BRIDGE_W/2, y_line, k)
        glVertex3f( BRIDGE_W/2, y_line, k)
        glEnd()
    glLineWidth(1.0)

def draw_wedge(cx, base_y, cz, width, length, height, color):
    """
    “屋脊形”楔形：
    - 沿Z方向：长度 length
    - Z中心最高：height
    - 沿X方向挤出：width（>=22.5cm）
    """
    glColor3f(*color)

    xL = cx - width/2
    xR = cx + width/2
    zB = cz - length/2
    zF = cz + length/2
    zC = cz

    y0 = base_y
    y1 = base_y + height

    E = (xL, y1, zC)
    F = (xR, y1, zC)

    A = (xL, y0, zB)
    B = (xR, y0, zB)
    C = (xR, y0, zF)
    D = (xL, y0, zF)

    glBegin(GL_QUADS)
    glVertex3f(*A); glVertex3f(*B); glVertex3f(*F); glVertex3f(*E)  # 后坡
    glVertex3f(*E); glVertex3f(*F); glVertex3f(*C); glVertex3f(*D)  # 前坡
    glEnd()

    glBegin(GL_TRIANGLES)
    glVertex3f(*A); glVertex3f(*E); glVertex3f(*D)  # 左侧
    glVertex3f(*B); glVertex3f(*C); glVertex3f(*F)  # 右侧
    glEnd()

# =========================
# 只生成 3 个交错单边桥（左右交错 + 间隔10~25cm）
# =========================
def generate_three_wedges():
    wedges = []
    top_y = 0.5 + BRIDGE_THICK

    # 左右贴边摆放（单边桥）
    left_cx  = -(BRIDGE_W/2 - WEDGE_W/2)
    right_cx = +(BRIDGE_W/2 - WEDGE_W/2)

    # 可用范围
    z_min = -BRIDGE_LEN/2 + MARGIN_Z + WEDGE_L/2
    z_max =  BRIDGE_LEN/2 - MARGIN_Z - WEDGE_L/2

    # 两个间隔（空隙）随机在 10~25
    # gap1 = random.uniform(GAP_MIN, GAP_MAX)
    # gap2 = random.uniform(GAP_MIN, GAP_MAX)
    gap1 = 20.0
    gap2 = 20.0


    # 三个楔形的中心距 = WEDGE_L + gap
    total_span = (WEDGE_COUNT * WEDGE_L) + (gap1 + gap2)
    available = (z_max - z_min) - total_span
    if available < 0:
        # 极端情况下（参数被改小导致放不下），退化成均匀摆放
        gap1 = gap2 = max(0.0, (z_max - z_min - 3*WEDGE_L) / 2.0)
        available = 0.0

    # 让整体在可用范围内随机平移一点点，看起来更自然
    offset = random.uniform(0.0, max(0.0, available))

    # 计算三个中心 z
    z1 = z_min + offset
    z2 = z1 + WEDGE_L + gap1
    z3 = z2 + WEDGE_L + gap2

    # 左右交错：左-右-左（你也可以改成右-左-右）
    xs = [left_cx, right_cx, left_cx]
    zs = [z1, z2, z3]

    for cx, cz in zip(xs, zs):
        wedges.append({
            "cx": cx,
            "cz": cz,
            "base_y": top_y + 0.05,  # 与桥面略拉开防z-fighting
            "w": WEDGE_W,
            "l": WEDGE_L,
            "h": WEDGE_H
        })

    return wedges, (gap1, gap2)

def draw_scene(mode, wedges):
    p = COLORS[mode]
    draw_ground(p['ground'])
    draw_bridge(p)
    for w in wedges:
        draw_wedge(w["cx"], w["base_y"], w["cz"], w["w"], w["l"], w["h"], p['wedge'])

# =========================
# 截图（无UI）
# =========================
def save_image_optimized(cam, mode):
    glPixelStorei(GL_PACK_ALIGNMENT, 1)
    raw_data = glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE)

    surface = pygame.image.fromstring(raw_data, (WINDOW_WIDTH, WINDOW_HEIGHT), "RGB")
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
    pygame.display.gl_set_attribute(pygame.GL_MULTISAMPLEBUFFERS, 1)
    pygame.display.gl_set_attribute(pygame.GL_MULTISAMPLESAMPLES, 4)

    pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT), DOUBLEBUF | OPENGL)
    pygame.display.set_caption("交错单边桥（3个楔形）仿真 - pygame + OpenGL")

    init_gl()

    cam = Camera()
    cam.look_at_bridge_center()

    text_renderer = TextRenderer()
    mode = 'rgb'

    wedges, (gap1, gap2) = generate_three_wedges()

    running = True
    snapshot_request = False
    clock = pygame.time.Clock()

    while running:
        # --- 事件 ---
        for event in pygame.event.get():
            if event.type == QUIT:
                running = False
            if event.type == KEYDOWN:
                if event.key == K_ESCAPE:
                    running = False
                if event.key == K_SPACE:
                    snapshot_request = True
                if event.key == K_m:
                    mode = 'gray' if mode == 'rgb' else 'rgb'
                if event.key == K_r:
                    cam.x, cam.y, cam.z = 0.0, 120.0, 320.0
                    cam.pitch, cam.yaw, cam.roll = -25.0, 0.0, 0.0
                    cam.look_at_bridge_center()
                if event.key == K_g:
                    wedges, (gap1, gap2) = generate_three_wedges()

        # --- 控制 ---
        keys = pygame.key.get_pressed()
        if keys[K_a]: cam.x -= MOVE_STEP
        if keys[K_d]: cam.x += MOVE_STEP
        if keys[K_q]: cam.y += MOVE_STEP
        if keys[K_e]: cam.y -= MOVE_STEP
        if keys[K_w]: cam.z -= MOVE_STEP
        if keys[K_s]: cam.z += MOVE_STEP

        if keys[K_i]: cam.pitch += ROT_STEP
        if keys[K_k]: cam.pitch -= ROT_STEP
        if keys[K_j]: cam.yaw += ROT_STEP
        if keys[K_l]: cam.yaw -= ROT_STEP
        if keys[K_u]: cam.roll += ROT_STEP
        if keys[K_o]: cam.roll -= ROT_STEP

        # --- 渲染 ---
        glClearColor(0.2, 0.2, 0.2, 1)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glLoadIdentity()

        cam.apply_transform()
        draw_scene(mode, wedges)

        # 截图（无UI）
        if snapshot_request:
            save_image_optimized(cam, mode)
            snapshot_request = False

        # UI
        infos = [
            f"[MODE]: {mode.upper()}   (M toggle)   (G regen gaps)",
            f"POS: X={cam.x:.0f}, Y={cam.y:.0f}, Z={cam.z:.0f}",
            f"ROT: P={cam.pitch:.0f}, Y={cam.yaw:.0f}, R={cam.roll:.0f}",
            f"BRIDGE: {BRIDGE_LEN:.0f}x{BRIDGE_W:.0f}cm  RAIL_H={RAIL_H:.0f}cm",
            f"WEDGES: {len(wedges)} (fixed=3)  gaps: {gap1:.1f}cm, {gap2:.1f}cm",
            f"WEDGE: H={WEDGE_H:.0f}cm  L={WEDGE_L:.0f}cm  W={WEDGE_W:.1f}cm",
            f"[SPACE] Save(no UI)  [R] Reset cam  [ESC] Quit"
        ]
        text_renderer.render(infos, 20, 20)

        pygame.display.flip()
        clock.tick(60)

    pygame.quit()

if __name__ == "__main__":
    main()
