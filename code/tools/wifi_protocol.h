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

// Host control ACK status
#define WIFI_HOST_ACK_ACCEPTED            0x00
#define WIFI_HOST_ACK_REJECTED            0x01
#define WIFI_HOST_ACK_UNKNOWN_CMD         0x02
#define WIFI_HOST_ACK_INVALID_PAYLOAD     0x03

#define WIFI_TX_BUFFER_SIZE     256

/* Telemetry payload layout (little-endian, 128 bytes):
 * loop, nav/fusion position and body velocity, heading, relative yaw,
 * marker fields, control flags, replay status, state-machine flags,
 * Euler/IMU values, RF/RR/LF/LR servo angles, then speed/PWM telemetry. */
#define WIFI_TELEMETRY_PAYLOAD_SIZE  128U

/* Updated by the control ISR after the final motor PWM limit is applied. */
extern volatile float g_wifi_target_speed_set;
extern volatile float g_wifi_speed_l;
extern volatile float g_wifi_speed_r;
extern volatile float g_wifi_pwm_left;
extern volatile float g_wifi_pwm_right;

// Send telemetry frame to host
void wifi_protocol_send_data(void);

// Poll host->MCU command frames and apply command
void wifi_protocol_poll_rx(void);

#endif // _WIFI_PROTOCOL_H_
