/*********************************************************************************************************************
* CYT2BL3 Opensourec Library 即（ CYT2BL3 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 CYT2BL3 开源库的一部分
*
* CYT2BL3 开源库 是免费软件
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
* 文件名称          zf_driver_ipc
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-11       pudding            first version
********************************************************************************************************************/

#include "ipc/cy_ipc_pipe.h"
#include "ipc/cy_ipc_config.h"
#include "zf_driver_delay.h"
#include "zf_driver_ipc.h"

vuint8 ipc_port_save; 
vuint8 ipc_released; 

static void ipc_default_callback (uint32 ipc_data);

ipc_callback_function user_callback = ipc_default_callback;
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     默认回调函数
// 参数说明     void
// 返回参数     void
// 使用示例     
// 备注信息     保护性冗余设计 防止在没有初始化设备的时候跑飞
//-------------------------------------------------------------------------------------------------------------------
static void ipc_default_callback (uint32 ipc_data)
{
    (void)ipc_data;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPC数据发送完成回调函数
// 参数说明     void
// 返回参数     void
// 使用示例     
// 备注信息     对方成功接收数据后将调用此函数置位释放标志位
//-------------------------------------------------------------------------------------------------------------------
void ipc_release_callback(void)
{
    ipc_released = 1;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPC数据接收回调函数
// 参数说明     ipc_msg           接收指针
// 返回参数     void
// 使用示例     
// 备注信息     将在此函数初步处理数据后调用用户的回调函数并传参
//-------------------------------------------------------------------------------------------------------------------
void ipc_received_callback(uint32_t *ipc_msg)
{
    communicate_data_t * data = (communicate_data_t *)ipc_msg;
    user_callback(data->data);
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPC数据发送函数
// 参数说明     send_data           需要发送给另外一个核心的uint32类型数据
// 返回参数     uint8               0：对方成功接收数据      1：对方无响应
// 使用示例     ipc_send_data(0x00000001);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint8 ipc_send_data(uint32 send_data)
{
    communicate_data_t pipedata;
    uint32 wait_time_us = 5000;
    uint8  return_data  = 1;
    pipedata.clientId   = 0x01;
    pipedata.data       = send_data;
    ipc_released        = 0;

    if (Cy_IPC_Pipe_SendMessage (ipc_port_save == 0 ? 1 : 0, &pipedata, ipc_release_callback) == CY_IPC_PIPE_SUCCESS)
    {
        while(wait_time_us)
        {
            if(ipc_released == 1)
            {
                return_data = 0;
                break;
            }
            else
            {
                system_delay_us(10);
                wait_time_us -= 10;
            }
        }
    }
    return return_data;
}



//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPC核心间通讯函数
// 参数说明     ipc_port            IPC端口 由于IPC只能两个核心连接 因此需要区分端口 比如核心7_0初始化 IPC_PORT_1 ，核心7_1初始化 IPC_PORT_2 
// 参数说明     ipc_callback        ipc通讯回调函数，如果另外一个核心发送数据 将调用此回调函数，用户可在回调函数中处理数据
// 返回参数     void
// 使用示例     ipc_communicate_init(IPC_PORT_1, my_ipc_callback);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void ipc_communicate_init(ipc_port_enum ipc_port, ipc_callback_function ipc_callback)
{
    cy_stc_ipc_pipe_config_t pipeConfig = 
    {
        .epIndexForThisCpu = (uint8)ipc_port,
        .epConfigData      = CY_IPC_PIPE_ENDPOINTS_DEFAULT_CONFIG,
    };
    
    Cy_IPC_Pipe_Init(&pipeConfig);
    Cy_IPC_Pipe_RegisterCallback(ipc_received_callback, 0x01u);
    NVIC_ClearPendingIRQ(pipeConfig.epConfigData[(uint8)ipc_port].ipcCpuIntIdx);
    NVIC_EnableIRQ(pipeConfig.epConfigData[(uint8)ipc_port].ipcCpuIntIdx);
    
    user_callback = ipc_callback;
    ipc_port_save = (uint8)ipc_port;
}

