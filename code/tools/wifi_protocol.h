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
#define WIFI_CMD_HOST_DRIVE     0x12    // Host -> MCU continuous drive/suspension command

#define WIFI_HOST_DRIVE_ENABLE  0x01U
#define WIFI_HOST_DRIVE_BRAKE   0x02U
#define WIFI_HOST_DRIVE_ROLL    0x04U
#define WIFI_HOST_DRIVE_HEIGHT  0x08U
#define WIFI_HOST_DRIVE_JUMP    0x10U
#define WIFI_HOST_DRIVE_KILL    0x20U  /* explicit motor-enable safety cut */

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

/* Host drive command, consumed by Remote_Control_Process(). Units: speed and
 * steering are the native control units; height/roll are cm/deg. */
extern volatile uint8_t g_wifi_host_drive_active;
extern volatile uint8_t g_wifi_host_drive_flags;
extern volatile float g_wifi_host_speed;
extern volatile float g_wifi_host_angle;
extern volatile float g_wifi_host_height;
extern volatile float g_wifi_host_roll;
extern volatile uint8_t g_wifi_host_disconnect_brake;
extern volatile uint8_t g_wifi_host_brake_latched;

// Send telemetry frame to host
void wifi_protocol_send_data(void);

// Poll host->MCU command frames and apply command
void wifi_protocol_poll_rx(void);

#endif // _WIFI_PROTOCOL_H_
