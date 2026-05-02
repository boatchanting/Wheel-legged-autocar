#ifndef __CODE1_WIFI_PROTOCOL_H__
#define __CODE1_WIFI_PROTOCOL_H__

#include "zf_common_headfile.h"

#define C1_WIFI_FRAME_HEAD1        0x5A
#define C1_WIFI_FRAME_HEAD2        0xA5
#define C1_WIFI_FRAME_TAIL         0xED

#define C1_WIFI_CMD_HOST_CONTROL   0x10
#define C1_WIFI_CMD_HOST_ACK       0x11

#define C1_WIFI_HOST_CTRL_SET_TCP_TARGET     0x03
#define C1_WIFI_HOST_CTRL_SET_LOCAL_PORT     0x04
#define C1_WIFI_HOST_CTRL_RECONNECT_TCP      0x05

#define C1_WIFI_HOST_ACK_ACCEPTED            0x00
#define C1_WIFI_HOST_ACK_INVALID_PAYLOAD     0x03
#define C1_WIFI_HOST_ACK_CONNECT_FAILED      0x04
#define C1_WIFI_HOST_ACK_UNKNOWN_CMD         0x05
#define C1_WIFI_HOST_ACK_UNSUPPORTED         0x06

void wifi_protocol_poll_rx(void);

#endif
