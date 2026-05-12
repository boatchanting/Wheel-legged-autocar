/*********************************************************************************************************************
* CYT4BB7CEE Opensourec Library 即（CYT4BB7CEE 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2023 SEEKFREE 逐飞科技
*
* 本文件是 CYT4BB7CEE 开源库的一部分
*
* CYT4BB7CEE 开源库 是免费软件
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
* 文件名称          zf_driver_encoder
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          MDK 5.38a
* 适用平台          CYT4BB7CEE
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2023-11-30       pudding            first version
* 2023-05-21       pudding            新增支持正交编码器
********************************************************************************************************************/

#ifndef _zf_driver_encoder_h_
#define _zf_driver_encoder_h_

#include "zf_common_typedef.h"

// 此枚举定义不允许用户修改
typedef enum // 枚举编码器引脚
{
    TC_CH07_ENCODER_CH1_P07_6,    // tcpwm模块的通道7  计数引脚可选范围
    TC_CH20_ENCODER_CH1_P08_1,    // tcpwm模块的通道20 计数引脚可选范围
    TC_CH27_ENCODER_CH1_P19_2,    // tcpwm模块的通道27 计数引脚可选范围	
    TC_CH58_ENCODER_CH1_P17_3,    // tcpwm模块的通道58 计数引脚可选范围
}encoder_channel1_enum;

// 此枚举定义不允许用户修改
typedef enum // 枚举编码器引脚
{
    TC_CH07_ENCODER_CH2_P07_7,    // tcpwm模块的通道7  计数引脚可选范围	 
    TC_CH20_ENCODER_CH2_P08_2,    // tcpwm模块的通道20 计数引脚可选范围
    TC_CH27_ENCODER_CH2_P19_3,    // tcpwm模块的通道27 计数引脚可选范围	
    TC_CH58_ENCODER_CH2_P17_4,    // tcpwm模块的通道58 计数引脚可选范围
}encoder_channel2_enum;

typedef enum  // 枚举 定时器编号
{
    TC_CH07_ENCODER,
    TC_CH20_ENCODER,
    TC_CH27_ENCODER,
    TC_CH58_ENCODER,
}encoder_index_enum;

//====================================================编码器 基础函数====================================================
int16 encoder_get_count     (encoder_index_enum encoder_n);
void  encoder_clear_count   (encoder_index_enum encoder_n);

void  encoder_quad_init     (encoder_index_enum encoder_n, encoder_channel1_enum count_pin, encoder_channel2_enum dir_pin);
void  encoder_dir_init      (encoder_index_enum encoder_n, encoder_channel1_enum ch1_pin, encoder_channel2_enum ch2_pin);
//====================================================编码器 基础函数====================================================

#endif
