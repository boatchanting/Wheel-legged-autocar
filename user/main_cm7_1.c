/*********************************************************************************************************************
* CYT4BB Opensourec Library 即（ CYT4BB 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 CYT4BB 开源库的一部分
*
* CYT4BB 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          main_cm7_1
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT4BB
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-4       pudding            first version
********************************************************************************************************************/

#include "zf_common_headfile.h"
#include "../code/config/config.h"
#include "../code1/tools/camera_menu.h"
#include "../code1/wifi.h"
#include "../code1/wifi_diff_stream.h"
#include "../code1/wifi_protocol.h"
#include "../code1/vision/bridge_detect.h"                                          // 新单边桥管线 (bridge_detect v11 对齐版, 2026-08-14 融合迁移)
#include "../code1/vision/bridge_fusion.h"                                          // 远近融合管线 (ref远处/脱出 + v8桥上, v13门控定版, 2026-08-14 接入)
#include "../code1/vision/bridge_v2_arbiter.h"                                       // 新管线仲裁层 (C21; 融合迁移后仅桥上 v8 阶段使用)
#include "../code1/vision/bridge_output_filter.h"                                    // 仲裁输出中值滤波层 (2026-08-14)
#include "../code1/vision/pvc_vision.h"
#include "../code1/vision/bumpy_vision.h"
#include "../code1/vision/vision_ipc_core1.h"
#include "../code1/vision/telemetry_ipc_core1.h"
// 打开新的工程或者工程移动了位置务必执行以下操作
// 第一步 关闭上面所有打开的文件
// 第二步 project->clean  等待下方进度条走完

// 本例程是开源库空工程 可用作移植或者测试各类内外设
// 本例程是开源库空工程 可用作移植或者测试各类内外设
// 本例程是开源库空工程 可用作移植或者测试各类内外设

// **************************** 代码区域 ****************************
#define VISION_IPC_PIT_NUM     (PIT_CH2)

/* ---- 单边桥远近融合管线 (bridge_fusion, v13门控定版) 接线状态 ---- */
#define BRIDGE_VISION_V2_PROFILE_TIMER   (TC_TIME2_CH1)  // 计时通道: 与 pvc_vision 共用
static bf_state_t  s_fusion_st;                      // 融合状态机 (~23KiB, 含 ref 工作区; ★严禁放任务栈)
static bf_result_t s_fusion_res;                     // 融合单帧结果 (统一中线 + 两引擎原始输出)
volatile runtime_profiler_t g_bridge_v2_cost_profiler = {0};  // 算法耗时统计

/* b2_mode 位掩码打包 (宏定义见 code/vision/vision_ipc.h):
   高4位=检测状态(按当前引擎取), 低3位=融合阶段(0=准备进入 1=桥上 2=准备脱出) */
static uint8 bridge_fusion_pack_mode(const bf_result_t *r)
{
    uint8 stage, det = 0;
    if (r->gate_top)            stage = B2M_STAGE_PREPARE_EXIT;
    else if (r->gate_bottom)    stage = B2M_STAGE_ON_BRIDGE;
    else                        stage = B2M_STAGE_PREPARE_ENTER;
    if (r->source == BF_SRC_V8)
    {
        det = (uint8)((r->v8.has_red   ? B2M_DET_RED   : 0)
                    | (r->v8.has_green ? B2M_DET_GREEN : 0)
                    | (r->v8.has_blue  ? B2M_DET_BLUE  : 0)
                    | (r->v8.has_top   ? B2M_DET_TOP   : 0));
    }
    else
    {
        det = (uint8)((r->ref.left_line.valid  ? B2M_DET_RED   : 0)
                    | (r->ref.bridge_found     ? B2M_DET_GREEN : 0)
                    | (r->ref.right_line.valid ? B2M_DET_BLUE  : 0));
    }
    return (uint8)(det | stage);
}

/* ref 阶段 (准备进入/准备脱出) 中线适配: ref 自己的稳定中线 (融合层 bf_center_from_ref
   已由 center_segment 转出, 即 PC 渲染那条青色统一中线) 直接填入仲裁结构喂滤波层;
   arbiter 只认 v8 结果类型, ref 阶段不参与。source 填 3/4 与桥上阶段 (0/1/2) 错开,
   使滤波层"source 切换清窗"自然防止跨阶段混求中值。 */
static void bridge_fusion_fill_ref_arb(const bf_result_t *r, bridge_v2_arb_t *out)
{
    float a, b;
    memset(out, 0, sizeof(*out));
    out->valid  = r->valid;
    out->source = (uint8)(r->gate_top ? 4 : 3);     /* 3=准备进入 4=准备脱出 */
    out->mode   = bridge_fusion_pack_mode(r);
    out->gate   = r->gate_bottom;
    /* has_top/top_a/top_b 置 0: 结束线只由桥上 v8 阶段提供 (与现状同源) */
    if (r->valid)
    {
        a = r->center.a * 1000.0f;
        b = r->center.b * 100.0f;
        out->line_a_x1000 = (int16)(a > 32767.0f ? 32767.0f : (a < -32768.0f ? -32768.0f : a));
        out->line_b_x100  = (int16)(b > 32767.0f ? 32767.0f : (b < -32768.0f ? -32768.0f : b));
        if (r->ref.center_segment.valid)
        {
            out->u_lo = (uint8)((r->ref.center_segment.y0 < r->ref.center_segment.y1) ? r->ref.center_segment.y0 : r->ref.center_segment.y1);
            out->u_hi = (uint8)((r->ref.center_segment.y0 > r->ref.center_segment.y1) ? r->ref.center_segment.y0 : r->ref.center_segment.y1);
        }
    }
}

int main(void)
{
    clock_init(SYSTEM_CLOCK_250M); 	// 时钟配置及系统初始化<务必保留>
    debug_info_init();                  // 调试串口信息初始化

    // 此处编写用户代码 例如外设初始化代码等

     // 初始化 WiFi 模块
    //wifi_init();                                                                // 初始化WIFI模块

    // 连接TCP服务器
    //wifi_connect_tcp_server();                                                  // 连接TCP服务器

    // 初始化摄像头和逐飞助手
    //wifi_camera_init();                                                         // 初始化摄像头和逐飞助手
    //wifi_diff_stream_init(PVC_IMAGE_W, PVC_IMAGE_H, 30U, 2U);                  // init realtime diff stream (keyframe interval = 30)
#if DEBUG_DISPLAY_CORE1
    CameraMenu_Init();
#endif
#if WIFI_CORE1_USE
    wifi_init();
    wifi_connect_tcp_server();
    #if WIFI_CORE1_ASSISTANT
    wifi_camera_init();
    #endif
    #if WIFI_CORE1_CUSTOM_IMAGE
    wifi_diff_stream_init(WIFI_CAMERA_SEND_W, WIFI_CAMERA_SEND_H, 30U, 2U);
    #endif
#endif
#if !(WIFI_CORE1_USE && WIFI_CORE1_ASSISTANT)
    mt9v03x_init();//初始化摄像头
#endif
    pvc_vision_init();                                                          // 初始化 PVC 入口视觉检测与帧率/耗时统计
    timer_init(BRIDGE_VISION_V2_PROFILE_TIMER, TIMER_US);                       // 新单边桥管线: 计时初始化
    timer_start(BRIDGE_VISION_V2_PROFILE_TIMER);
    RUNTIME_PROFILE_RESET(&g_bridge_v2_cost_profiler);
    bridge_fusion_init(&s_fusion_st);                                           // 初始化单边桥远近融合管线 (清门控/ref缓存/v8状态)
    bumpy_vision_init();                                                        // 初始化颠簸路段视觉检测
    VisionIpc_Core1_Init();                                                     // 初始化1核视觉共享内存结果发布
    pit_ms_init(VISION_IPC_PIT_NUM, 2);                                          // 2ms 中断中处理0/1核视觉通信
    interrupt_global_enable(0);


    // 此处编写用户代码 例如外设初始化代码等
    // 跳帧控制：摄像头 ~100fps，WiFi 目标设定为 25fps 左右，防止 TCP 队列在远距离下阻塞
    #define CAMERA_FPS          100U
    #define WIFI_TARGET_FPS     50U
    #define FRAME_SKIP_RATIO    ((CAMERA_FPS + WIFI_TARGET_FPS - 1) / WIFI_TARGET_FPS)
    static uint32_t frame_skip_counter = 0U;
    
    extern uint8 wifi_needs_reconnect;

    while(true)
    {
        static uint32_t reconnect_cooldown = 0;
        if(reconnect_cooldown > 0)
        {
            reconnect_cooldown--;
        }
        else if(wifi_needs_reconnect)
        {
            wifi_needs_reconnect = 0;
            wifi_reconnect_tcp_server();
            reconnect_cooldown = 50; // 冷却期(假如主循环50Hz，约1秒)
        }
        #if WIFI_CORE1_USE && WIFI_CORE1_CUSTOM_IMAGE
        wifi_protocol_poll_rx();
        wifi_protocol_send_oscilloscope();   // C29: 1核 示波器帧 → 网页上位机 (tools/05/视频网页上位机)
        #endif
        // 此处编写需要循环执行的代码
                // 处理摄像头图像数据
        if(mt9v03x_finish_flag)
        {
            mt9v03x_finish_flag = 0;
            // 在发送前将图像备份再进行发送，这样可以避免图像出现撕裂的问题
            memcpy(image_copy[0], mt9v03x_image[0], MT9V03X_IMAGE_SIZE);

#if (WIFI_CAMERA_SEND_MODE == WIFI_CAMERA_SEND_MODE_COMPRESSED) // 配置选择在 code1\wifi.h中，不要和图像在屏幕显示同时开，没测试过，仅测试逐飞助手图像传输正常
            compress_image_to_target();// 将原图压缩至 compressed_image_copy (94*60)

            if(VisionIpc_Core1_TakePvcResetRequest())
            {
                pvc_vision_reset_filter();
            }
            if(VisionIpc_Core1_TakeBridgeResetRequest())
            {
                bridge_fusion_init(&s_fusion_st);                               // 重置=融合状态机全量复位 (清门控/ref帧缓存/v8先验)
                bridge_output_filter_reset();                                   // 同步清空中值滤波层 (门控/滑窗)
            }
            if(VisionIpc_Core1_TakeBumpyResetRequest())
            {
                bumpy_vision_reset_filter();
            }

            if(VisionIpc_Core1_ShouldRunPvc()) // VisionIpc_Core1_ShouldRunPvc() ，测试时用 1 0
            {

                pvc_vision_process_camera_frame(compressed_image_copy[0]);//将压缩图像输入到 PVC 检测算法中

                // 4. 将 PVC 检测框直接画在 compressed_image_copy[0] 上，供 WIFI 发送显示
                render_pvc_vision_to_image();//算法执行完毕后，将 PVC 检测框画在 image_copy 上,必须放在这！如果放在算法前面，画的黑线会破坏算法寻找白色的逻辑
            }
            if(VisionIpc_Core1_ShouldRunBridge()) // 正式比赛逻辑: 0核 enable 门控 (2026-08-14 融合迁移时恢复; 调试可临时改 if(1))
            {
                /* 单边桥远近融合管线 (v13门控定版): 融合检测 (每帧只跑 ref/v8 一个引擎)
                   → 桥上阶段走 arbiter (与现状一致) / ref 阶段 ref 中线直供
                   → 中值滤波 → b2_* 供 IPC 发布 */
                static bridge_v2_arb_t s_fusion_arb;                            // 适配输出 (两阶段共用, 喂滤波层)
                RUNTIME_PROFILE_BEGIN(g_bridge_v2_cost_profiler, BRIDGE_VISION_V2_PROFILE_TIMER);
                bridge_fusion_frame(compressed_image_copy[0], &s_fusion_st, &s_fusion_res);
                RUNTIME_PROFILE_END(&g_bridge_v2_cost_profiler, BRIDGE_VISION_V2_PROFILE_TIMER);
                if(s_fusion_res.source == BF_SRC_V8)
                {
                    bridge_v2_arbiter_process(&s_fusion_res.v8);                // 桥上: 仲裁 (红蓝中点/绿线/失能)
                    s_fusion_arb = *bridge_v2_arbiter_get();
                    s_fusion_arb.mode = bridge_fusion_pack_mode(&s_fusion_res); // mode 改发位掩码 (高4检测+低3阶段)
                    s_fusion_arb.gate = s_fusion_res.gate_bottom;               // gate 以融合层锁存为准 (与 v8 内部 gate 同步)
                }
                else
                {
                    bridge_fusion_fill_ref_arb(&s_fusion_res, &s_fusion_arb);   // ref 阶段: ref 中线直供, arbiter 不参与
                }
                bridge_output_filter_update(&s_fusion_arb);                     // 中值滤波+多帧门控 → b2_* 唯一发布口径
                render_bridge_vision_to_image();                                // 渲染 b2 控制线/退出线 (画在图像上, 不影响算法输入)
                #if DEBUG_LOG_ENABLE
                {
                    static uint32 v2_log_div = 0U;
                    if ((v2_log_div++ % 50U) == 0U)     // 100fps 下约 0.5s 一条
                    {
                        printf("[BridgeFusion] stage=%d det=0x%02X gb=%d gt=%d valid=%d src=%d cost=%lu us avg=%lu max=%lu cnt=%lu\r\n",
                               (int)(s_fusion_arb.mode & B2M_STAGE_MASK),
                               (int)((s_fusion_arb.mode >> 4) & 0x0F),
                               (int)s_fusion_res.gate_bottom,
                               (int)s_fusion_res.gate_top,
                               (int)s_fusion_res.valid,
                               (int)s_fusion_res.source,
                               (unsigned long)g_bridge_v2_cost_profiler.last_us,
                               (unsigned long)g_bridge_v2_cost_profiler.avg_us,
                               (unsigned long)g_bridge_v2_cost_profiler.max_us,
                               (unsigned long)g_bridge_v2_cost_profiler.count);
                    }
                }
                #endif
            }
            if(VisionIpc_Core1_ShouldRunBumpy()) //  ，测试时用 1 0
            {
                bumpy_vision_process_camera_frame(image_copy[0]);
                render_bumpy_vision_to_image();
            }
#endif

            // 跳帧控制：视觉算法满帧运行(100fps)，只限制 WiFi 发送速率(~25fps)
            if((frame_skip_counter % FRAME_SKIP_RATIO) == 0U)
            {
                // 发送图像
                #if WIFI_CORE1_USE && WIFI_CORE1_ASSISTANT
                seekfree_assistant_camera_send();
                #endif
                #if WIFI_CORE1_USE && WIFI_CORE1_CUSTOM_IMAGE
                wifi_diff_stream_send_gray_frame((const uint8 *)WIFI_CAMERA_SEND_IMAGE_PTR);
                #endif

            #if DEBUG_DISPLAY_CORE1
                CameraMenu_Update();
            #endif
            
            }
            frame_skip_counter++;
            // 如果使用UDP协议传输数据则推荐在数据全部发送到模块之后立即调用wifi_spi_udp_send_now()函数，以告知模块立即将收到的数据发送到网络上
            // 如果没有立即调用则模块会在持续2毫秒未收到数据后，将数据发送到网络上
            // 调用wifi_spi_udp_send_now()前传输给模块的数据数量建议不要超过40960字节
            // wifi_spi_udp_send_now();
        }
        // 此处编写需要循环执行的代码
    }
}

// **************************** 代码区域 ****************************

