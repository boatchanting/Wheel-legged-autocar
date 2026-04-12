#ifndef _WIFI_PROTOCOL_H_
#define _WIFI_PROTOCOL_H_

#include "zf_common_headfile.h"

// Frame constants
#define WIFI_FRAME_HEAD1        0x5A
#define WIFI_FRAME_HEAD2        0xA5
#define WIFI_FRAME_TAIL         0xED

// Frame command IDs
#define WIFI_CMD_DATA_PACKET    0x01    // MCU -> host telemetry
#define WIFI_CMD_HOST_CONTROL   0x10    // Host -> MCU control

// Host control payload command IDs
#define WIFI_HOST_CTRL_CLEAR_TRAJECTORY   0x01
#define WIFI_HOST_CTRL_START_CAR          0x02

#define WIFI_TX_BUFFER_SIZE     256

// Send telemetry frame to host
void wifi_protocol_send_data(void);

// Poll host->MCU command frames and apply command
void wifi_protocol_poll_rx(void);

#endif // _WIFI_PROTOCOL_H_
