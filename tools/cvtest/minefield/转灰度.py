from PIL import Image, ImageSequence
import os
import sys

def convert_gif_to_grayscale(input_path: str, output_path: str = None) -> bool:
    """
    将GIF图像转换为灰度格式，保留所有帧及原始时序参数
    
    参数:
        input_path: 输入GIF文件路径
        output_path: 输出GIF文件路径（若为None，则在原文件名后添加'_gray'）
    
    返回:
        bool: 转换成功返回True，失败返回False
    """
    try:
        # 验证输入文件存在
        if not os.path.isfile(input_path):
            print(f"错误: 文件不存在 - {input_path}")
            return False
        
        # 设置默认输出路径
        if output_path is None:
            base, ext = os.path.splitext(input_path)
            output_path = f"{base}_gray{ext}"
        
        # 打开GIF图像
        with Image.open(input_path) as img:
            # 检查是否为GIF格式
            if img.format != 'GIF':
                print(f"警告: 文件 '{input_path}' 不是GIF格式 (实际格式: {img.format})")
            
            frames = []
            durations = []
            loop = img.info.get('loop', 0)  # 获取循环次数
            
            # 处理每一帧
            for frame in ImageSequence.Iterator(img):
                # 将当前帧转换为RGB再转灰度（确保兼容性）
                gray_frame = frame.convert('L')
                
                # 保留原始帧的持续时间
                duration = frame.info.get('duration', 100)
                durations.append(duration)
                
                frames.append(gray_frame)
            
            # 保存灰度GIF（多帧）
            frames[0].save(
                output_path,
                save_all=True,
                append_images=frames[1:],
                duration=durations,
                loop=loop,
                optimize=True
            )
            
            print(f"✓ 转换成功: '{input_path}' -> '{output_path}'")
            print(f"  帧数: {len(frames)} | 尺寸: {frames[0].size}")
            return True
            
    except Exception as e:
        print(f"✗ 转换失败: {type(e).__name__}: {e}")
        return False


if __name__ == "__main__":
    input_file = r"E:\github_projects\autocar1\ppt\屏幕录制 2026-02-08 162120.gif"
    output_file = r"E:\github_projects\autocar1\ppt\屏幕录制 2026-02-08 162120_gray.gif"

    success = convert_gif_to_grayscale(input_file, output_file)
    sys.exit(0 if success else 1)