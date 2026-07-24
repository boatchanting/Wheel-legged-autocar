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
#define WIFI_CMD_HOST_ACK       0x11    // MCU -> host control ACK

// Host control payload command IDs
#define WIFI_HOST_CTRL_CLEAR_TRAJECTORY   0x01
#define WIFI_HOST_CTRL_START_CAR          0x02
#define WIFI_HOST_CTRL_START_GPS_REPLAY   0x03
#define WIFI_HOST_CTRL_STOP_GPS_REPLAY    0x04
#define WIFI_HOST_CTRL_START_LOG          0x05
#define WIFI_HOST_CTRL_STOP_LOG           0x06
#define WIFI_HOST_CTRL_SET_TARGET_SPEED   0x20
#define WIFI_HOST_CTRL_ARM_DIRECT_SPEED   0x21
#define WIFI_HOST_CTRL_STOP_DIRECT_SPEED  0x22

// Host control ACK status
#define WIFI_HOST_ACK_ACCEPTED            0x00
#define WIFI_HOST_ACK_REJECTED            0x01
#define WIFI_HOST_ACK_UNKNOWN_CMD         0x02
#define WIFI_HOST_ACK_INVALID_PAYLOAD     0x03

#define WIFI_TX_BUFFER_SIZE     320

extern volatile uint8_t g_wifi_host_beep_request;

uint8_t WifiHostSpeedTest_IsActive(void);
uint8_t WifiHostSpeedTest_Arm(void);
uint8_t WifiHostSpeedTest_SetTarget(float speed_cmd, uint8_t mode);
void WifiHostSpeedTest_Stop(void);
void WifiHostSpeedTest_Update1ms(void);
uint8_t WifiHostSpeedTest_AccelFeedforwardActive(void);

// Send telemetry frame to host
void wifi_protocol_send_data(void);

// Poll host->MCU command frames and apply command
void wifi_protocol_poll_rx(void);

#endif // _WIFI_PROTOCOL_H_
