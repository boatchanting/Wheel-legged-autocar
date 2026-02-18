#ifndef _WIFI_PROTOCOL_H_
#define _WIFI_PROTOCOL_H_
#include "zf_common_headfile.h"

//此文件定义了自定义的WiFi通信协议的相关常量和函数，需要搭配上位机

// 协议常量定义
#define WIFI_FRAME_HEAD1        0x5A
#define WIFI_FRAME_HEAD2        0xA5
#define WIFI_FRAME_TAIL         0xED

// 功能字定义 (支持扩展)
#define WIFI_CMD_DATA_PACKET    0x01    // 组合数据包 (惯导+GNSS)

// 发送缓冲区大小定义 (根据数据量调整，目前估计约 100 字节)
#define WIFI_TX_BUFFER_SIZE     256

// 函数声明

/**
 * @brief 发送组合数据包 (包含 loop_counter, 惯导XY, GNSS全量数据)
 *        该函数会自动获取全局变量并打包发送
 */
void wifi_protocol_send_data(void);

#endif // _WIFI_PROTOCOL_H_

// 顺序	数据名称	类型	字节数	备注
// 1	loop_counter	UInt32	4	
// 2	nav.x	Float	4	
// 3	nav.y	Float	4	
// 4	time.year	UInt16	2	
// 5	time.month	UInt8	1	
// 6	time.day	UInt8	1	
// 7	time.hour	UInt8	1	
// 8	time.minute	UInt8	1	
// 9	time.second	UInt8	1	
// 10	state	UInt8	1	
// 11	lat_degree	UInt16	2	
// 12	lat_cent	UInt16	2	
// 13	lat_second	UInt16	2	
// 14	lon_degree	UInt16	2	
// 15	lon_cent	UInt16	2	
// 16	lon_second	UInt16	2	
// 17	latitude	Double	8	无损精度
// 18	longitude	Double	8	无损精度
// 19	ns	Int8	1	
// 20	ew	Int8	1	
// 21	speed	Float	4	
// 22	direction	Float	4	
// 23	ant_state	UInt8	1	
// 24	ant_dir	Float	4	
// 25	sat_used	UInt8	1	
// 26	height	Float	4	