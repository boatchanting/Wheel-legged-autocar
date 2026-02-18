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
// 1	loop_counter	UInt32	4	系统ms级时间戳
// 2	nav.x	Float	4	惯性导航X轴位置，单位mm，小车向后为x正方向
// 3	nav.y	Float	4	惯性导航Y轴位置，单位mm，小车向右为y正方向
// 4	time.year	UInt16	2	年份，例如2024
// 5	time.month	UInt8	1	月份，1-12
// 6	time.day	UInt8	1	日期，1-31
// 7	time.hour	UInt8	1	小时，0-23
// 8	time.minute	UInt8	1	分钟，0-59
// 9	time.second	UInt8	1	秒，0-59
// 10	state	UInt8	1	导航状态，为1时表示导航数据有效
// 11	lat_degree	UInt16	2	纬度度数
// 12	lat_cent	UInt16	2	纬度分
// 13	lat_second	UInt16	2	纬度秒，    这里的秒是被放大了100倍的，主要是避免使用浮点数
// 14	lon_degree	UInt16	2	经度度数
// 15	lon_cent	UInt16	2	经度分
// 16	lon_second	UInt16	2	经度秒，    这里的秒是被放大了100倍的，主要是避免使用浮点数
// 17	latitude	Double	8	无损精度，纬度
// 18	longitude	Double	8	无损精度，经度
// 19	ns	Int8	1	  // 纬度半球 N（北半球）或 S（南半球）
// 20	ew	Int8	1	  // 经度半球 E（东半球）或 W（西半球）
// 21	speed	Float	4	 // 速度（公里/每小时）
// 22	direction	Float	4	     // 地面航向（000.0~359.9 度，以真北方为参考基准）+
// 23	ant_state	UInt8	1	   // 双天线测向有效状态 1：测向有效  0：测向无效，无效时antenna_direction数据是无效 
// 24	ant_dir	Float	4	      // 主天线指向从天线与真北构成的夹角（000.0~359.9 度）
// 25	sat_used	UInt8	1	  // 用于定位的卫星数量
// 26	height	Float	4	 // 高度（米）
