import cv2
import numpy as np

def process_diff_video(input_file, quality=90, keyframe_interval=100):
    """
    模拟帧间差异图传
    :param input_file: 输入视频路径
    :param quality: JPEG 画质 (为了不损失画质，设为90)
    :param keyframe_interval: 关键帧间隔 (每隔多少帧发送一次全图)
    """
    cap = cv2.VideoCapture(input_file)
    if not cap.isOpened(): return

    # 编码参数
    encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), quality]

    frame_count = 0
    total_size_full = 0  # 假设全发原图的体积
    total_size_diff = 0  # 采用差异传输的体积

    # 用于保存上一帧的画面（发送端和接收端各保存一份）
    sender_prev_frame = None
    receiver_current_frame = None

    while True:
        ret, frame = cap.read()
        if not ret: break
        
        frame_count += 1
        is_keyframe = (frame_count % keyframe_interval == 1) # 判断是否为关键帧

        # ==================== 【发射端：压缩打包】 ====================
        send_data = None  # 准备发送的字节数据
        
        if is_keyframe or sender_prev_frame is None:
            # 1. 发送完整关键帧 (I帧)
            _, encoded = cv2.imencode('.jpg', frame, encode_param)
            # 打包格式：标志位(1字节) + 图像数据
            send_data = b'\x01' + encoded.tobytes() 
            sender_prev_frame = frame.copy()
        else:
            # 2. 计算差异帧 (P帧)
            # 将两帧转为灰度图并计算绝对差值
            gray1 = cv2.cvtColor(sender_prev_frame, cv2.COLOR_BGR2GRAY)
            gray2 = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            diff = cv2.absdiff(gray1, gray2)
            
            # 二值化，找到发生变化的区域
            _, thresh = cv2.threshold(diff, 25, 255, cv2.THRESH_BINARY)
            
            # 寻找变化区域的边界框 (Bounding Box)
            x, y, w, h = cv2.boundingRect(thresh)

            if w > 0 and h > 0:
                # 画面有变化，把变化的矩形区域裁剪下来
                changed_roi = frame[y:y+h, x:x+w]
                _, encoded_roi = cv2.imencode('.jpg', changed_roi, encode_param)
                
                # 打包格式：标志位(0) + x(2字节) + y(2字节) + 图像数据
                # 将 x, y 坐标转为2字节整数一起发过去，告诉接收端贴在哪个位置
                header = b'\x00' + int(x).to_bytes(2, 'big') + int(y).to_bytes(2, 'big')
                send_data = header + encoded_roi.tobytes()
            else:
                # 画面完全没变化，只发一个空包 (极度节省带宽)
                send_data = b'\x02' 

            sender_prev_frame = frame.copy()

        # 统计体积数据
        _, full_jpg = cv2.imencode('.jpg', frame, encode_param)
        total_size_full += len(full_jpg.tobytes())
        total_size_diff += len(send_data)
        # ==============================================================


        # ==================== 【接收端：解压还原】 ====================
        # 接收端收到 send_data
        
        frame_type = send_data[0] # 读取第一个字节，判断帧类型
        
        if frame_type == 1: # \x01: 完整关键帧
            nparr = np.frombuffer(send_data[1:], np.uint8)
            receiver_current_frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
            
        elif frame_type == 0: # \x00: 差异更新包
            # 解析坐标 x, y
            x = int.from_bytes(send_data[1:3], 'big')
            y = int.from_bytes(send_data[3:5], 'big')
            
            # 解析图像数据
            nparr = np.frombuffer(send_data[5:], np.uint8)
            roi = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
            
            # 将变化的小方块覆盖到上一帧的对应位置上
            h, w = roi.shape[:2]
            receiver_current_frame[y:y+h, x:x+w] = roi
            
        elif frame_type == 2: # \x02: 画面无变化
            pass # 保持 receiver_current_frame 不变即可
            
        # ==============================================================

        cv2.imshow('Receiver Window', receiver_current_frame)
        
        if frame_count % 100 == 0:
            print(f"帧 {frame_count} | 传统全量传输: {total_size_full/1024:.1f}KB | 差异传输: {total_size_diff/1024:.1f}KB | 节省了: {(1 - total_size_diff/total_size_full)*100:.1f}% 带宽")

        if cv2.waitKey(30) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    # 请准备一个稍微有点静止背景的视频测试效果最佳，比如监控视频
    process_diff_video(r"\data\2026_04_17_21_44_42_Video颠簸.avi" , quality=90, keyframe_interval=100)

