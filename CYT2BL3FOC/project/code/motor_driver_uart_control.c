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
* 文件名称          motor_driver_uart_control
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2025-01-03       pudding            first version
********************************************************************************************************************/

#include "motor_driver_uart_control.h"

small_device_value_struct motor_value;

fifo_struct     motor_driver_fifo;              // 串口通讯的 FIFO 结构体

uint8           driver_fifo_buffer[128];        // FIFO 指向的缓冲数组

uint8           read_buffer[128];               // 解析时，读取数据的缓冲数组

int16           receive_enter_flag = 0;         // 回车标志位


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     串口通讯 FIFO 擦除指定长度的数据
// 参数说明     clear_length        要擦除的长度
// 返回参数     void
// 使用示例     motor_driver_fifo_clear(1); 
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_driver_fifo_clear(uint32 clear_length)
{
    fifo_read_buffer(&motor_driver_fifo, read_buffer, &clear_length, FIFO_READ_AND_CLEAN);
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     串口通讯 数据包解析函数
// 参数说明     device_value        通讯参数结构体
// 参数说明     statement_buffer    一整包数据
// 返回参数     void
// 使用示例     motor_driver_control_loop(&motor_value, read_buffer); 
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_driver_parse_statement(small_device_value_struct *device_value, uint8 *statement_buffer)
{
    if(BYTE_TYPE == device_value->cmd_type)                                     // 判断待解析内容类型是否为 字节通讯 类型
    {
        switch(statement_buffer[1])                                             // 判断 字节通讯 的功能字 
        {
            case 0x01:{                                                         // 功能字 0x01 设置占空比
  
                device_value->receive_left_duty_data   = (((int)statement_buffer[2] << 8) | (int)statement_buffer[3]);  // 左侧电机占空比拟合
                
                device_value->receive_right_duty_data  = (((int)statement_buffer[4] << 8) | (int)statement_buffer[5]);  // 右侧电机占空比拟合
                
                device_value->immediate_command        = SET_DUTY;              // 即时指令(仅响应一次)状态更新为 SET_DUTY 
                
                device_value->refresh_flag             = CMD_FORTHWITH;         // 即时指令刷新标志位修正  需要马上执行命令
            }break;
            
            case 0x02:{                                                         // 功能字 0x02 获取到速度信息
              
                device_value->continue_command         = GET_SPEED;             // 持续指令(串口持续输出)状态更新为 GET_SPEED 
                
            }break;
            
            case 0x03:{                                                         // 功能字 0x03 自矫正零点
              
                device_value->immediate_command        = SET_ZERO;              // 即时指令(仅响应一次)状态更新为 SET_ZERO 
                
                device_value->refresh_flag             = CMD_LOOP;              // 即时指令刷新标志位修正  无需马上执行
            }break;
            
            default:break;
        }
    }
    else if(STRING_TYPE == device_value->cmd_type)                              // 判断待解析内容类型是否为 字符通讯 类型
    {
        char *location1 = NULL;                                                 // 定义两个位置指针 用于解析数据
        char *location2 = NULL;
        
        char data_string[10];                                                   // 定义一个缓存数组 用于 字符串 转 数据
        
        if(statement_buffer[0] == 'S')                                          // 判断 字符 指令的首字符是否为 'S'
        {
            if(strstr((const char *)statement_buffer, "SET-DUTY"))             // 搜索 待解析数组内是否有 "SET-DUTY" 关键词  
            {                                                                   // SET-DUTY指令格式为："SET-DUTY,num_1,num_2\r\n" 其中 num_1 为左侧电机占空比， num_2 为右侧电机占空比 范围限制 -10000 ~ 10000
              
                location1 = strchr((char *)statement_buffer, ',');              // 搜索第一个逗号的位置 
                
                if(location1 != 0x00)                                           // 如果成功搜索到 则继续  否则默认将占空比设置为 0
                {
                    location1 ++;                                               // 位置自增 越过第一个逗号
                    
                    location2 = strchr(location1, ',');                         // 搜索第二个逗号的位置
                    
                    if(location2 != 0x00)                                       // 如果成功搜索到 则继续
                    {
                        memset(data_string, 0x00, 10);                          // 清空缓存数组
                      
                        memcpy(data_string, location1, location2 - location1);  // 取出两个逗号之间的数据
                        
                        device_value->receive_left_duty_data = func_str_to_int(data_string);    // 将字符串数据转为整形 第一个数据为左侧电机占空比
                        
                        device_value->receive_left_duty_data = func_limit_ab(device_value->receive_left_duty_data, -10000, 10000);  // 数据限幅
                        
                        location2 ++;                                           // 位置自增 越过第二个逗号
                        
                        location1 = strchr(location2, '\n');                    // 搜索回车的位置
                        
                        memset(data_string, 0x00, 10);                          // 清空缓存数组
                        
                        memcpy(data_string, location2, location1 - location2);  // 取出第二个逗号到回车之间的数据
                        
                        device_value->receive_right_duty_data = func_str_to_int(data_string);   // 将字符串数据转为整形 第二个数据为右侧电机占空比
                        
                        device_value->receive_right_duty_data = func_limit_ab(device_value->receive_right_duty_data, -10000, 10000);  // 数据限幅
                    }                                                                  
                    else                                                        // 未搜索到第二个逗号 则搜索回车 有一个逗号 可解析左侧占空比
                    {
                        location2 = strchr(location1, '\n');                    // 搜索回车的位置
                        
                        memset(data_string, 0x00, 10);                          // 清空缓存数组
                                                                                
                        memcpy(data_string, location1, location2 - location1);  // 取出第一个逗号到回车之间的数据
                        
                        device_value->receive_left_duty_data = func_str_to_int(data_string);    // 将字符串数据转为整形 第一个数据为左侧电机占空比
                        
                        device_value->receive_left_duty_data = func_limit_ab(device_value->receive_left_duty_data, -10000, 10000);  // 数据限幅
                      
                        device_value->receive_right_duty_data = 0;              // 没有第二个逗号 右侧占空比数据解析失败 设置占空比为 0 
                    }
                    
                }
                else
                {
                    device_value->receive_left_duty_data  = 0;                  // 没有逗号 数据指令解析异常 设置占空比为 0 
                    
                    device_value->receive_right_duty_data = 0;                  // 没有逗号 数据指令解析异常 设置占空比为 0 
                }
                
                device_value->immediate_command        = SET_DUTY;              // 即时指令(仅响应一次)状态更新为 SET_DUTY 
                        
                device_value->refresh_flag             = CMD_FORTHWITH;         // 即时指令刷新标志位修正  需要马上执行命令
            }
            else if(strstr((const char *)statement_buffer, "SET-ZERO"))        // 搜索 待解析数组内是否有 "SET-ZERO" 关键词
            {   
                device_value->immediate_command        = SET_ZERO;              // 即时指令(仅响应一次)状态更新为 SET_ZERO 
                
                device_value->refresh_flag             = CMD_LOOP;              // 即时指令刷新标志位修正  无需马上执行命令
            } 
            else if(strstr((const char *)statement_buffer, "SET-PHASE"))       // 搜索 待解析数组内是否有 "SET-PHASE" 关键词
            {
                device_value->refresh_flag             = CMD_LOOP;              // 即时指令刷新标志位修正  无需马上执行命令
            }
            else if(strstr((const char *)statement_buffer, "STOP-SEND"))       // 搜索 待解析数组内是否有 "STOP-SEND" 关键词
            {
                device_value->continue_command         = NULL_CMD;              // 持续指令(串口持续输出)状态更新为 NULL_CMD 
            } 
            else                                                                // 当前指令非协议指令 输出提示 error cmd
            {
                device_value->immediate_command        = ERROR_CMD;             // 即时指令(仅响应一次)状态更新为 ERROR_CMD 
                
                device_value->refresh_flag             = CMD_LOOP;              // 即时指令刷新标志位修正  无需马上执行命令
            }
        }
        else if(statement_buffer[0] == 'G')                                    // 判断 字符 指令的首字符是否为 'G'
        {
            if(strstr((const char *)statement_buffer, "GET-SPEED"))            // 搜索 待解析数组内是否有 "GET-SPEED" 关键词
            {
                device_value->continue_command         = GET_SPEED;             // 持续指令(串口持续输出)状态更新为 GET_SPEED 
            } 

            else if(strstr((const char *)statement_buffer, "GET-PHASE-DUTY"))  // 搜索 待解析数组内是否有 "GET-PHASE-DUTY" 关键词
            {                                                                   // GET-PHASE-DUTY指令格式为："GET-PHASE-DUTY,num\r\n" 其中 num 为选择的电机， 0 为左侧电机  1为右侧电机 不填 为左右互相切换
              
                location1 = strchr((char *)statement_buffer, ',');              // 搜索第一个逗号的位置 
                
                if(location1 != 0x00)                                           // 如果成功搜索到 则继续  否则默认 左右电机 互相切换
                {
                    location1 ++;                                               // 位置自增 越过第一个逗号
                    
                    location2 = strchr(location1, '\n');                        // 搜索回车的位置
                    
                    memset(data_string, 0x00, 10);                              // 清空缓存数组
                                                                                
                    memcpy(data_string, location2, location2 - location1);      // 取出第一个逗号到回车之间的数据
                    
                    device_value->select_motor = func_str_to_int(data_string);  // 将字符串数据转为整形 第一个数据为左侧电机占空比
                    
                    if(device_value->select_motor != LEFT_MOTOR)                // 数据限幅 防止用户输入异常数据  数据类型仅可选择左侧电机(O) 或者右侧电机(1)
                    {
                        device_value->select_motor = RIGHT_MOTOR;               
                    }
                }
                else
                {
                    if(device_value->select_motor == LEFT_MOTOR)                // 判断当前选择的电机类型 并且切换到另外一个电机 
                    {
                        device_value->select_motor = RIGHT_MOTOR;
                    }
                    else
                    {
                        device_value->select_motor = LEFT_MOTOR;
                    }
                }
                device_value->continue_command         = GET_PHASE_DUTY;        // 持续指令(串口持续输出)状态更新为 GET_PHASE_DUTY 
            } 
            else if(strstr((const char *)statement_buffer, "GET-ENCODER"))     // 搜索 待解析数组内是否有 "GET-ENCODER" 关键词
            {
                device_value->continue_command         = GET_ENCODER;           // 持续指令(串口持续输出)状态更新为 GET_ENCODER 
            }
            else if(strstr((const char *)statement_buffer, "GET-ANGLE"))       // 搜索 待解析数组内是否有 "GET-ANGLE" 关键词
            {
                device_value->continue_command         = GET_ANGLE;             // 持续指令(串口持续输出)状态更新为 GET_ANGLE 
            }  
            else if(strstr((const char *)statement_buffer, "GET-VOLTAGE"))     // 搜索 待解析数组内是否有 "GET-VOLTAGE" 关键词
            {
                device_value->continue_command         = GET_VOLTAGE;           // 持续指令(串口持续输出)状态更新为 GET_VOLTAGE 
            }  
            else                                                                // 当前指令非协议指令 输出提示 error cmd
            {   
                device_value->immediate_command        = ERROR_CMD;             // 即时指令(仅响应一次)状态更新为 ERROR_CMD 
                
                device_value->refresh_flag             = CMD_LOOP;              // 即时指令刷新标志位修正  无需马上执行命令
            }                                                                   
        }
        else if(statement_buffer[0] == 'T')                                     // 判断 字符 指令的首字符是否为 'T'
        {
            if(strstr((const char *)statement_buffer, "TEST-PHASE"))           // 搜索 待解析数组内是否有 "TEST-PHASE" 关键词
            {
                device_value->refresh_flag             = CMD_LOOP;              // 即时指令刷新标志位修正  无需马上执行命令
            } 
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     串口通讯控制 循环解析函数 
// 参数说明     device_value        通讯参数结构体
// 返回参数     void
// 使用示例     motor_driver_control_loop(&motor_value); 
// 备注信息     可放置在串口接收中断、PIT周期中断、主循环  
//-------------------------------------------------------------------------------------------------------------------
void motor_driver_control_loop(small_device_value_struct *device_value)
{    
    uint32 read_length = 0;                                                     // 定义读取长度变量
    
    uint8  check_data = 0;                                                      // 定义数据校验变量
    
    uint8  analysis_max = 20;                                                   // 定义数据校验变量
  
    if(fifo_used(&motor_driver_fifo) >= 7)                                      // 判断 FIFO 缓冲区长度是否大于7
    {
        if(receive_enter_flag >= 1)
        {
            while(receive_enter_flag >= 1)                                      // 判断是否收到 回车 字符
            {
                receive_enter_flag --;                                          // 清除一次 回车 字符标志位
                
                read_length = fifo_used(&motor_driver_fifo);                    // 获取现在 FIFO 中的字符总长度
                
                fifo_read_buffer(&motor_driver_fifo, read_buffer, &read_length, FIFO_READ_ONLY);        // 将所有数据 仅读取 出来
                
                for(int i = 0; i < read_length; i ++)                           // 查找 回车 字符位置 
                {
                    if(read_buffer[i] == '\n')                                  // 判断是否为 回车
                    {
                        read_length = i + 1;                                    // 获取到 回车的位置    
                        
                        memset(read_buffer, 0x00, 128);                         // 清空当前读出的数据
                        
                        fifo_read_buffer(&motor_driver_fifo, read_buffer, &read_length, FIFO_READ_AND_CLEAN);   // 将 回车 之前的所有字符从 FIFO 缓冲区读取并且擦除
                        
                        break;                                                  // 跳出循环
                    }   
                }
                
                if(read_length > 8)
                {
                    if((read_buffer[0] == 'S' || read_buffer[0] == 'G'|| read_buffer[0] == 'T') && (read_buffer[1] == 'E' || read_buffer[1] == 'T'))
                    {
                        device_value->cmd_type = STRING_TYPE;                       // 设置通讯类型为 字符通讯
                    
                        motor_driver_parse_statement(device_value, read_buffer);    // 调用数据解析    
                    }
                }
                
                if((-- analysis_max) == 0)                                      // 允许最多一次性连续解析10条指令
                {
                    break;
                }
            }
        }
        else 
        {
            if(fifo_used(&motor_driver_fifo) >= 64)                                             // 未收到回车 但fifo占用超过 64字节 则直接仅保留最后 17 字节
            {
                read_length = fifo_used(&motor_driver_fifo) - 17;                               // 计算读取长度
                
                fifo_read_buffer(&motor_driver_fifo, read_buffer, &read_length, FIFO_READ_AND_CLEAN);           // 读取无效数据
                
                memset(read_buffer, 0x00, 128);                                                 // 清空当前读出的数据
            }
            
            while(fifo_used(&motor_driver_fifo) >= 7)
            {      
                read_length = 2;
                
                fifo_read_buffer(&motor_driver_fifo, read_buffer, &read_length, FIFO_READ_ONLY);// 读取前两个数据
                
                if(read_buffer[0] == 0XA5)                                                      // 判断是否为帧头 若不是字节通讯的帧头 则判断是否为字符串通讯
                {
                    read_length = 7;                                                            // 成功判断到帧头 仅读取一包数据(长度为7位)
                    
                    fifo_read_buffer(&motor_driver_fifo, read_buffer, &read_length, FIFO_READ_ONLY);
                    
                    for(int i = 0; i < 6; i ++)                                                 // 和校验拟合
                    {
                        check_data += read_buffer[i];
                    }
                    if(check_data == read_buffer[6])                                            // 判断和校验
                    {   
                        read_length = 7;                                                        // 成功通过和校验 从 FIFO 读取并擦除 一包数据(长度为7位)
                        
                        fifo_read_buffer(&motor_driver_fifo, read_buffer, &read_length, FIFO_READ_AND_CLEAN);
                        
                        device_value->cmd_type = BYTE_TYPE;                                     // 设置通讯类型为 字节通讯
                        
                        motor_driver_parse_statement(device_value, read_buffer);                // 调用数据解析      
                    }
                    else
                    {
                        motor_driver_fifo_clear(1);                                             // 和校验未通过 清除一个缓冲区数据（状态机机制）
                    }
                }
                else
                {
                    if((read_buffer[0] == 'S' || read_buffer[0] == 'G' || read_buffer[0] == 'T') && (read_buffer[1] == 'E' || read_buffer[1] == 'T'))  // 判断是否为字符串指令
                    {
                        break;                                                                  // 字符串指令首字母不做处理
                    }
                    else
                    {
                        motor_driver_fifo_clear(1);                                             // 字节包通讯和字符串通讯协议 均不符合 清除一个缓冲区数据（状态机机制）
                    }
                }   
                
                if((-- analysis_max) == 0)
                {
                    break;
                }
            }
        }
        driver_cmd_forthwith();
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     发送占空比信息  字节包类型
// 参数说明     device_value        通讯参数结构体
// 参数说明     left_speed          左侧电机占空比
// 参数说明     right_speed         右侧电机占空比
// 返回参数     void
// 使用示例     motor_driver_set_duty(&motor_value, 500, 500);   // 字节包通讯则按照协议打包输出 占空比范围 -10000 ~ 10000
// 备注信息       
//-------------------------------------------------------------------------------------------------------------------
void motor_driver_set_duty(small_device_value_struct *device_value, int left_duty, int right_duty)
{
    device_value->send_data_buffer[0] = 0xA5;
    device_value->send_data_buffer[1] = 0X01;
    device_value->send_data_buffer[2] = (uint8)((left_duty  & 0xFF00) >> 8);
    device_value->send_data_buffer[3] = (uint8)( left_duty  & 0x00FF);
    device_value->send_data_buffer[4] = (uint8)((right_duty & 0xFF00) >> 8);
    device_value->send_data_buffer[5] = (uint8)( right_duty & 0x00FF);
    device_value->send_data_buffer[6] = device_value->send_data_buffer[0] + 
                                        device_value->send_data_buffer[1] + 
                                        device_value->send_data_buffer[2] + 
                                        device_value->send_data_buffer[3] + 
                                        device_value->send_data_buffer[4] + 
                                        device_value->send_data_buffer[5];

    uart_write_buffer(MOTOR_DRIVER_UART, device_value->send_data_buffer, 7);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     发送速度信息  字节包类型
// 参数说明     device_value        通讯参数结构体
// 参数说明     left_speed          左侧电机速度
// 参数说明     right_speed         右侧电机速度
// 返回参数     void
// 使用示例     motor_driver_send_speed(&motor_value, motor_left.motor_speed_filter, motor_right.motor_speed_filter);   // 字节包通讯则按照协议打包输出
// 备注信息       
//-------------------------------------------------------------------------------------------------------------------
void motor_driver_send_speed(small_device_value_struct *device_value, int left_speed, int right_speed)
{
    device_value->send_data_buffer[0] = 0xA5;
    device_value->send_data_buffer[1] = 0X02;
    device_value->send_data_buffer[2] = (uint8)((left_speed  & 0xFF00) >> 8);
    device_value->send_data_buffer[3] = (uint8)( left_speed  & 0x00FF);
    device_value->send_data_buffer[4] = (uint8)((right_speed & 0xFF00) >> 8);
    device_value->send_data_buffer[5] = (uint8)( right_speed & 0x00FF);
    device_value->send_data_buffer[6] = device_value->send_data_buffer[0] + 
                                        device_value->send_data_buffer[1] + 
                                        device_value->send_data_buffer[2] + 
                                        device_value->send_data_buffer[3] + 
                                        device_value->send_data_buffer[4] + 
                                        device_value->send_data_buffer[5];
    uart_write_buffer(MOTOR_DRIVER_UART, device_value->send_data_buffer, 7);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     串口通讯控制 回调函数
// 参数说明     device_value        通讯参数结构体
// 返回参数     void
// 使用示例     motor_driver_control_callback(&motor_value);
// 备注信息       
//-------------------------------------------------------------------------------------------------------------------
void motor_driver_control_callback(small_device_value_struct *device_value)
{
    uint8 receive_data;                                                         // 定义串口接收变量
    
    uint8 receive_max = 10;                                                     // 定义单次接收允许读取的最大数据量
    
    while(uart_query_byte(MOTOR_DRIVER_UART, &receive_data) == 1)               // 判断是否接收到串口数据
    {
        fifo_write_element(&motor_driver_fifo, receive_data);                   // 将数据写入 fifo
        
        if(receive_data == '\n')                                                // 若接收到特殊指令 回车：'\n'  则将回车标志位置位 用于后续解析
        {
            receive_enter_flag ++;
        }
        
        motor_driver_control_loop(device_value);                                // 调用数据解析 该函数也可在其他位置调用
        
        if((-- receive_max) == 0)
        {
            break;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     驱动 串口通讯 初始化
// 参数说明     void
// 返回参数     void
// 使用示例     motor_driver_uart_init();
// 备注信息       
//-------------------------------------------------------------------------------------------------------------------
void motor_driver_uart_init(void)
{
    fifo_init(&motor_driver_fifo, FIFO_DATA_8BIT, driver_fifo_buffer, 128);     // 初始化 fifo 结构体
      
    uart_init(MOTOR_DRIVER_UART, 460800, MOTOR_DRIVER_RX, MOTOR_DRIVER_TX);     // 初始化 通讯调试串口 
    
    uart_rx_interrupt(MOTOR_DRIVER_UART, 1);                                    // 打开串口接收中断
}













