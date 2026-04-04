import pygame
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *
import time

# --- 全局配置 ---
TARGET_WIDTH = 752
TARGET_HEIGHT = 480
SSAA_FACTOR = 2  # 2倍超采样抗锯齿 (若显卡强劲可改为 4)
WINDOW_WIDTH = TARGET_WIDTH * SSAA_FACTOR
WINDOW_HEIGHT = TARGET_HEIGHT * SSAA_FACTOR

FOV = 45.0

# 【优化1】大幅增大近裁剪面
# 原因：Z-buffer是非线性的，精度主要集中在近处。
# 将近平面从 1.0 改为 50.0，可以显著提升远处的深度精度，消除Z-fighting。
NEAR_CLIP = 50.0  
FAR_CLIP = 5000.0

MOVE_STEP = 2.0
ROT_STEP = 1.0

# 颜色定义
COLORS = {
    'rgb': {
        'ground': (0.8, 0.0, 0.0),    
        'board':  (0.0, 0.2, 0.8),    
        'tape':   (1.0, 1.0, 1.0)     
    },
    'gray': {
        'ground': (0.1, 0.1, 0.1), 
        'board':  (0.4, 0.4, 0.4), 
        'tape':   (1.0, 1.0, 1.0)  
    }
}

class TextRenderer:
    def __init__(self):
        # 字体大小随分辨率缩放
        self.font = pygame.font.SysFont('Consolas', 24 * SSAA_FACTOR, bold=True)
    
    def render(self, text_lines, x, y):
        # 切换到 2D 投影绘制文字
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

class Camera:
    def __init__(self):
        self.x, self.y, self.z = 0.0, 40.0, 100.0
        self.pitch, self.yaw, self.roll = -30.0, 0.0, 0.0

    def apply_transform(self):
        glRotatef(-self.roll,  0, 0, 1)
        glRotatef(-self.pitch, 1, 0, 0)
        glRotatef(-self.yaw,   0, 1, 0)
        glTranslatef(-self.x, -self.y, -self.z)

    def look_at_center(self):
        self.yaw = 0
        self.roll = 0
        import math
        dist = math.sqrt(self.x**2 + self.z**2)
        if dist > 0:
            angle = math.degrees(math.atan2(self.y, dist))
            self.pitch = -angle

def init_gl():
    glEnable(GL_DEPTH_TEST)
    
    # 【优化2】开启多采样和多边形平滑
    # 这一步可以减少远处细长物体的锯齿感
    glEnable(GL_MULTISAMPLE) 
    glEnable(GL_POLYGON_SMOOTH)
    glEnable(GL_BLEND)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST)

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(FOV, (WINDOW_WIDTH / WINDOW_HEIGHT), NEAR_CLIP, FAR_CLIP)
    glMatrixMode(GL_MODELVIEW)

def draw_rect(x, y, z, w, l, color):
    glColor3f(*color)
    hw, hl = w/2, l/2
    glBegin(GL_QUADS)
    glNormal3f(0, 1, 0)
    glVertex3f(x - hw, y, z + hl)
    glVertex3f(x + hw, y, z + hl)
    glVertex3f(x + hw, y, z - hl)
    glVertex3f(x - hw, y, z - hl)
    glEnd()

def draw_scene(mode):
    p = COLORS[mode]
    draw_rect(0, 0, 0, 2000, 2000, p['ground']) # 地面
    draw_rect(0, 0.5, 0, 500, 500, p['board'])  # 蓝板 (高度 0.5)
    
    c = p['tape']
    
    # 【优化3】拉大层级间距
    # 原来 0.6，改为 2.0。物理上分开物体，GPU更容易判断遮挡关系。
    y = 2.0
    
    # 胶带 (回字形)
    draw_rect(0, y, -45, 100, 10, c)
    draw_rect(0, y, 45, 100, 10, c)
    draw_rect(-45, y, 0, 10, 80, c)
    draw_rect(45, y, 0, 10, 80, c)

def save_image_optimized(cam, mode):
    """只保存当前缓冲区的图像，不包含之后绘制的UI"""
    glPixelStorei(GL_PACK_ALIGNMENT, 1)
    raw_data = glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE)
    
    surface = pygame.image.fromstring(raw_data, (WINDOW_WIDTH, WINDOW_HEIGHT), "RGB")
    surface = pygame.transform.flip(surface, False, True)
    
    # 缩放消除摩尔纹 (Downsampling)
    final_surface = pygame.transform.smoothscale(surface, (TARGET_WIDTH, TARGET_HEIGHT))
    
    fname = (f"img_{mode}_"
             f"X{int(cam.x)}_Y{int(cam.y)}_Z{int(cam.z)}_"
             f"P{int(cam.pitch)}_Y{int(cam.yaw)}_R{int(cam.roll)}.png")
    
    pygame.image.save(final_surface, fname)
    print(f"[已保存 - 无UI] {fname}")

def main():
    pygame.init()
    # 请求多重采样缓冲区 (MSAA 4x)
    pygame.display.gl_set_attribute(pygame.GL_MULTISAMPLEBUFFERS, 1)
    pygame.display.gl_set_attribute(pygame.GL_MULTISAMPLESAMPLES, 4)
    
    pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT), DOUBLEBUF | OPENGL)
    pygame.display.set_caption("6DoF雷区仿真 - 优化抗锯齿版")
    
    init_gl()
    cam = Camera()
    text_renderer = TextRenderer()
    
    mode = 'rgb'
    running = True
    snapshot_request = False 
    clock = pygame.time.Clock()

    cam.look_at_center()

    while running:
        # --- 1. 事件处理 ---
        for event in pygame.event.get():
            if event.type == QUIT: running = False
            if event.type == KEYDOWN:
                if event.key == K_ESCAPE: running = False
                if event.key == K_SPACE: snapshot_request = True
                if event.key == K_m: mode = 'gray' if mode == 'rgb' else 'rgb'
                if event.key == K_r: cam.x=0; cam.z=100; cam.y=150; cam.look_at_center()

        # --- 2. 运动控制 ---
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

        # --- 3. 渲染核心流程 ---
        
        # A. 清除
        glClearColor(0.2, 0.2, 0.2, 1)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glLoadIdentity()
        
        # B. 3D场景
        cam.apply_transform()
        draw_scene(mode)
        
        # C. 截图 (在UI之前)
        if snapshot_request:
            save_image_optimized(cam, mode)
            snapshot_request = False
        
        # D. UI
        infos = [
            f"[MODE]: {mode.upper()}",
            f"POS: {cam.x:.0f}, {cam.y:.0f}, {cam.z:.0f}",
            f"ROT: {cam.pitch:.0f}, {cam.yaw:.0f}, {cam.roll:.0f}",
            f"[SPACE] Save"
        ]
        #text_renderer.render(infos, 20, 20)

        # E. 显示
        pygame.display.flip()
        clock.tick(60)

    pygame.quit()

if __name__ == "__main__":
    main()