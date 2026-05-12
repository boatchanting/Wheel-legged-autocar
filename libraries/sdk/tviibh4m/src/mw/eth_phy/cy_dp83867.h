/***************************************************************************//**
* \file cy_dp83867.h
*
* \brief Driver for Ethernet PHY DP83867
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef __CY_DP83867_H__
#define __CY_DP83867_H__


/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "cy_project.h"
#include "cy_device_headers.h"


/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/


/*****************************************************************************/
/* Global type definitions ('typedef')                                       */
/*****************************************************************************/

/* BMCR */
typedef struct
{
    uint16_t u6RESERVED                  : 6; /* RESERVED: Writes ignored, read as 0. */
    uint16_t u1SPEED_SELECTION_MSB       : 1; /* Speed Select: See description for bit 13. */
    uint16_t u1COLLISION_TEST            : 1; /* Collision Test:
                                                 1 = Collision test enabled.
                                                 0 = Normal operation.
                                                 When set, this bit will cause the COL signal to be asserted in
                                                 response to the assertion of TX_EN within 512-bit times. The COL
                                                 signal will be deasserted within 4-bit times in response to the
                                                 deassertion of TX_EN. */
    uint16_t u1DUPLEX_MODE               : 1; /* Duplex Mode:
                                                 When auto-negotiation is disabled writing to this bit allows the port
                                                 Duplex capability to be selected.
                                                 1 = Full Duplex operation.
                                                 0 = Half Duplex operation. */
    uint16_t u1RESTART_AUTO_NEGOTIATION  : 1; /* Restart Auto-Negotiation:
                                                 1 = Restart Auto-Negotiation. Reinitiates the Auto-Negotiation
                                                 process. If Auto-Negotiation is disabled (bit 12 = 0), this bit is
                                                 ignored. This bit is self-clearing and will return a value of 1 until
                                                 Auto-Negotiation is initiated, whereupon it will self-clear. Operation of
                                                 the Auto-Negotiation process is not affected by the management
                                                 entity clearing this bit.
                                                 0 = Normal operation. */
    uint16_t u1ISOLATE                   : 1; /* Isolate:
                                                 1 = Isolates the Port from the MII with the exception of the serial
                                                 management.
                                                 0 = Normal operation. */
    uint16_t u1POWER_DOWN                : 1; /* Power Down:
                                                 1 = Power down.
                                                 0 = Normal operation.
                                                 Setting this bit powers down the PHY. Only the register block is
                                                 enabled during a power down condition. This bit is ORd with the
                                                 input from the PWRDOWN_INT pin. When the active low
                                                 PWRDOWN_INT pin is asserted, this bit will be set. */
    uint16_t u1AUTO_NEGOTIATION_ENABLE   : 1; /* Auto-Negotiation Enable:
                                                 Strap controls initial value at reset.
                                                 1 = Auto-Negotiation Enabled - bits 8 and 13 of this register are
                                                 ignored when this bit is set.
                                                 0 = Auto-Negotiation Disabled - bits 8 and 13 determine the port
                                                 speed and duplex mode. */
    uint16_t u1SPEED_SELECTION_LSB       : 1; /* Speed Select (Bits 6, 13):
                                                 When auto-negotiation is disabled writing to this bit allows the port
                                                 speed to be selected.
                                                 11 = Reserved
                                                 10 = 1000 Mbps
                                                 1 = 100 Mbps
                                                 0 = 10 Mbps */
    uint16_t u1LOOPBACK                  : 1; /* Loopback:
                                                 1 = Loopback enabled.
                                                 0 = Normal operation.
                                                 The loopback function enables MII transmit data to be routed to the
                                                 MII receive data path.
                                                 Setting this bit may cause the descrambler to lose synchronization
                                                 and produce a 500-s dead time before any valid data will appear at
                                                 the MII receive outputs. */
    uint16_t u1RESET                     : 1; /* Reset:
                                                 1 = Initiate software Reset / Reset in Process.
                                                 0 = Normal operation.
                                                 This bit, which is self-clearing, returns a value of one until the reset
                                                 process is complete. The configuration is restrapped. */
} cy_stc_dp83867_bmcr_t;


typedef union
{
    cy_stc_dp83867_bmcr_t f;
    uint16_t u16;
} cy_un_dp83867_bmcr_t;


/* BMSR */
typedef struct
{
    uint16_t u1EXTENDED_CAPABILITY        : 1; /* Extended Capability:
                                                  1 = Extended register capabilities.
                                                  0 = Basic register set capabilities only. */
    uint16_t u1JABBER_DETECT              : 1; /* Jabber Detect: This bit only has meaning in 10-Mbps mode.
                                                  1 = Jabber condition detected.
                                                  0 = No Jabber.
                                                  This bit is implemented with a latching function, such that the
                                                  occurrence of a jabber condition causes it to set until it is cleared by
                                                  a read to this register by the management interface or by a reset. */
    uint16_t u1LINK_STATUS                : 1; /* Link Status:
                                                  1 = Valid link established.
                                                  0 = Link not established.
                                                  The criteria for link validity is implementation specific. The
                                                  occurrence of a link failure condition will causes the Link Status bit
                                                  to clear. Once cleared, this bit may only be set by establishing a
                                                  good link condition and a read through the management interface. */
    uint16_t u1AUTO_NEGOTIATION_ABILITY   : 1; /* Auto Negotiation Ability:
                                                  1 = Device is able to perform Auto-Negotiation.
                                                  0 = Device is not able to perform Auto-Negotiation. */
    uint16_t u1REMOTE_FAULT               : 1; /* Remote Fault:
                                                  1 = Remote Fault condition detected (cleared on read or by reset).
                                                  Fault criteria: Far-End Fault Indication or notification from Link
                                                  Partner of Remote Fault.
                                                  0 = No remote fault condition detected. */
    uint16_t u1AUTO_NEGOTIATION_COMPLETE  : 1; /* Auto-Negotiation Complete:
                                                  1 = Auto-Negotiation process complete.
                                                  0 = Auto-Negotiation process not complete. */
    uint16_t u1MF_PREAMBLE_SUPPRESSION    : 1; /* Preamble Suppression Capable:
                                                  1 = Device able to perform management transaction with preamble
                                                  suppressed, 32-bits of preamble needed only once after reset,
                                                  invalid opcode or invalid turnaround.
                                                  0 = Normal management operation. */
    uint16_t u1RESERVED                   : 1; /* RESERVED: Write as 0, read as 0. */
    uint16_t u1EXTENDED_STATUS            : 1; /* 1000BASE-T Extended Status Register:
                                                  1 = Device supports Extended Status Register 0x0F. */
    uint16_t u1_100BASE_T2_HALF_DUPLEX    : 1; /* 100BASE-T2 Half Duplex Capable:
                                                  0 = Device not able to perform 100BASE-T2 in half duplex mode. */
    uint16_t u1_100BASE_T2_FULL_DUPLEX    : 1; /* 100BASE-T2 Full Duplex Capable:
                                                  0 = Device not able to perform 100BASE-T2 in full duplex mode. */
    uint16_t u1_10BASE_Te_HALF_DUPLEX     : 1; /* 10BASE-Te Half Duplex Capable:
                                                  1 = Device able to perform 10BASE-Te in half duplex mode. */
    uint16_t u1_10BASE_Te_FULL_DUPLEX     : 1; /* 10BASE-Te Full Duplex Capable:
                                                  1 = Device able to perform 10BASE-Te in full duplex mode. */
    uint16_t u1_100BASE_TX_HALF_DUPLEX    : 1; /* 100BASE-TX Half Duplex Capable:
                                                  1 = Device able to perform 100BASE-TX in half duplex mode. */
    uint16_t u1_100BASE_TX_FULL_DUPLEX    : 1; /* 100BASE-TX Full Duplex Capable:
                                                  1 = Device able to perform 100BASE-TX in full duplex mode. */
    uint16_t u1_100BASE_T4                : 1; /* 100BASE-T4 Capable:
                                                  0 = Device not able to perform 100BASE-T4 mode. */
} cy_stc_dp83867_bmsr_t;


typedef union
{
    cy_stc_dp83867_bmsr_t f;
    uint16_t u16;
} cy_un_dp83867_bmsr_t;


/* PHYIDR1 */
typedef struct
{
    uint16_t u16OUI_MSB  : 16; /* OUI Most Significant Bits: Bits 3 to 18 of the OUI (080028h,) are
                                  stored in bits 15 to 0 of this register. The most significant two bits of
                                  the OUI are ignored (the IEEE standard refers to these as bits 1 and 2). */
} cy_stc_dp83867_phyidr1_t;


typedef union
{
    cy_stc_dp83867_phyidr1_t f;
    uint16_t u16;
} cy_un_dp83867_phyidr1_t;


/* PHYIDR2 */
typedef struct
{
    uint16_t u4MDL_REV   : 4; /* Model Revision Number:
                                 Four bits of the vendor model revision number are mapped from bits
                                 3 to 0 (most significant bit to bit 3). This field will be incremented for
                                 all major device changes. */
    uint16_t u6VNDR_MDL  : 6; /* Vendor Model Number:
                                 The six bits of vendor model number are mapped from bits 9 to 4
                                 (most significant bit to bit 9). */
    uint16_t u6OUI_LSB   : 6; /* OUI Least Significant Bits:
                                 Bits 19 to 24 of the OUI (080028h) are mapped from bits 15 to 10 of this register respectively. */
} cy_stc_dp83867_phyidr2_t;


typedef union
{
    cy_stc_dp83867_phyidr2_t f;
    uint16_t u16;
} cy_un_dp83867_phyidr2_t;


/* ANAR */
typedef struct
{
    uint16_t u5SELECTOR      : 5; /* Protocol Selection Bits:
                                     These bits contain the binary encoded protocol selector supported by this port.
                                     <00001> indicates that this device supports IEEE 802.3u. */
    uint16_t u1_10BASETe_EN  : 1; /* 10BASE-Te Support:
                                     1 = 10BASE-Te is supported by the local device.
                                     0 = 10BASE-Te not supported */
    uint16_t u1_10_FD        : 1; /* 10BASE-Te Full Duplex Support:
                                     1 = 10BASE-Te Full Duplex is supported by the local device.
                                     0 = 10BASE-Te Full Duplex not supported. */
    uint16_t u1TX            : 1; /* 100BASE-TX Support:
                                     1 = 100BASE-TX is supported by the local device.
                                     0 = 100BASE-TX not supported. */
    uint16_t u1TX_FD         : 1; /* 100BASE-TX Full Duplex Support:
                                     1 = 100BASE-TX Full Duplex is supported by the local device.
                                     0 = 100BASE-TX Full Duplex not supported. */
    uint16_t u1T4            : 1; /* 100BASE-T4 Support:
                                     1= 100BASE-T4 is supported by the local device.
                                     0 = 100BASE-T4 not supported. */
    uint16_t u1PAUSE         : 1; /* PAUSE Support for Full Duplex Links:
                                     The PAUSE bit indicates that the device is capable of providing the symmetric PAUSE
                                     functions as defined in Annex 31B.
                                     Encoding and resolution of PAUSE bits is defined in IEEE 802.3 Annex 28B, Tables
                                     28B-2 and 28B-3, respectively. Pause resolution status is reported in PHYCR[13:12].
                                     1 = Advertise that the DTE (MAC) has implemented both the optional MAC control
                                     sublayer and the pause function as specified in clause 31 and annex 31B of 802.3u.
                                     0= No MAC based full duplex flow control. */
    uint16_t u1ASM_DIR       : 1; /* Asymmetric PAUSE Support for Full Duplex Links:
                                     The ASM_DIR bit indicates that asymmetric PAUSE is supported.
                                     Encoding and resolution of PAUSE bits is defined in IEEE 802.3 Annex 28B, Tables
                                     28B-2 and 28B-3, respectively. Pause resolution status is reported in PHYCR[13:12].
                                     1 = Advertise that the DTE (MAC) has implemented both the optional MAC control
                                     sublayer and the pause function as specified in clause 31 and annex 31B of 802.3u.
                                     0= No MAC based full duplex flow control. */
    uint16_t u1RESERVED1     : 1; /* RESERVED for Future IEEE use: Write as 0, Read as 0 */
    uint16_t u1RF            : 1; /* Remote Fault:
                                     1 = Advertises that this device has detected a Remote Fault.
                                     0 = No Remote Fault detected. */
    uint16_t u1RESERVED0     : 1; /* RESERVED by IEEE: Writes ignored, read as 0. */
    uint16_t u1NP            : 1; /* Next Page Indication:
                                     0 = Next Page Transfer not desired.
                                     1 = Next Page Transfer desired. */
} cy_stc_dp83867_anar_t;


typedef union
{
    cy_stc_dp83867_anar_t f;
    uint16_t u16;
} cy_un_dp83867_anar_t;


/* ANLPAR */
typedef struct
{
    uint16_t u5SELECTOR   : 5; /* Protocol Selection Bits:
                                  Link Partners binary encoded protocol selector. */
    uint16_t u1_10        : 1; /* 10BASE-Te Support:
                                  1 = 10BASE-Te is supported by the Link Partner.
                                  0 = 10BASE-Te not supported by the Link Partner. */
    uint16_t u1_10_FD     : 1; /* 10BASE-Te Full Duplex Support:
                                  1 = 10BASE-Te Full Duplex is supported by the Link Partner.
                                  0 = 10BASE-Te Full Duplex not supported by the Link Partner. */
    uint16_t u1TX         : 1; /* 100BASE-TX Support:
                                  1 = 100BASE-TX is supported by the Link Partner.
                                  0 = 100BASE-TX not supported by the Link Partner. */
    uint16_t u1TX_FD      : 1; /* 100BASE-TX Full Duplex Support:
                                  1 = 100BASE-TX Full Duplex is supported by the Link Partner.
                                  0 = 100BASE-TX Full Duplex not supported by the Link Partner. */
    uint16_t u1T4         : 1; /* 100BASE-T4 Support:
                                  1 = 100BASE-T4 is supported by the Link Partner.
                                  0 = 100BASE-T4 not supported by the Link Partner. */
    uint16_t u1PAUSE      : 1; /* PAUSE:
                                  1 = Pause function is supported by the Link Partner.
                                  0 = Pause function is not supported by the Link Partner. */
    uint16_t u1ASM_DIR    : 1; /* ASYMMETRIC PAUSE:
                                  1 = Asymmetric pause is supported by the Link Partner.
                                  0 = Asymmetric pause is not supported by the Link Partner. */
    uint16_t u1RESERVED0  : 1; /* RESERVED for Future IEEE use:Write as 0, read as 0. */
    uint16_t u1RF         : 1; /* Remote Fault:
                                  1 = Remote Fault indicated by Link Partner.
                                  0 = No Remote Fault indicated by Link Partner. */
    uint16_t u1ACK        : 1; /* Acknowledge:
                                  1 = Link Partner acknowledges reception of the ability data word.
                                  0 = Not acknowledged.
                                  The Auto-Negotiation state machine will automatically control the this bit based on the
                                  incoming FLP bursts. */
    uint16_t u1NP         : 1; /* Next Page Indication:
                                  0 = Link Partner does not desire Next Page Transfer.
                                  1 = Link Partner desires Next Page Transfer. */
} cy_stc_dp83867_anlpar_t;


typedef union
{
    cy_stc_dp83867_anlpar_t f;
    uint16_t u16;
} cy_un_dp83867_anlpar_t;


/* ANER */
typedef struct
{
    uint16_t u1LP_AN_ABLE             : 1; /* Link Partner Auto-Negotiation Able:
                                              1 = indicates that the Link Partner supports Auto-Negotiation.
                                              0 = indicates that the Link Partner does not support Auto-Negotiation. */
    uint16_t u1PAGE_RX                : 1; /* Link Code Word Page Received:
                                              1 = Link Code Word has been received, cleared on a read.
                                              0 = Link Code Word has not been received. */
    uint16_t u1NP_ABLE                : 1; /* Next Page Able:
                                              1 = Indicates local device is able to send additional Next Pages. */
    uint16_t u1LP_NP_ABLE             : 1; /* Link Partner Next Page Able:
                                              1 = Link Partner does support Next Page.
                                              0 = Link Partner does not support Next Page. */
    uint16_t u1PDF                    : 1; /* Parallel Detection Fault:
                                              1 = A fault has been detected via the Parallel Detection function.
                                              0 = A fault has not been detected. */
    uint16_t u1RX_NEXT_PAGE_STOR_LOC  : 1; /* Receive Next Page Storage Location:
                                              1 = Link Partner Next Pages are stored in register 8.
                                              0 = Link Partner Next Pages are stored in register 5. */
    uint16_t u1RX_NEXT_PAGE_LOC_ABLE  : 1; /* Receive Next Page Location Able:
                                              1 = Received Next Page storage location is specified by bit 6.5.
                                              0 = Received Next Page storage location is not specified by bit 6.5. */
    uint16_t u9RESERVED               : 9; /* RESERVED: Writes ignored, read as 0. */
} cy_stc_dp83867_aner_t;


typedef union
{
    cy_stc_dp83867_aner_t f;
    uint16_t u16;
} cy_un_dp83867_aner_t;


/* ANNPTR */
typedef struct
{
    uint16_t u11CODE    : 11; /* Code:
                                 This field represents the code field of the next page transmission. If the MP bit is
                                 set (bit 13 of this register), then the code shall be interpreted as a "Message
                                 Page”, as defined in annex 28C of IEEE 802.3u. Otherwise, the code shall be
                                 interpreted as an "Unformatted Page”, and the interpretation is application
                                 specific.
                                 The default value of the CODE represents a Null Page as defined in Annex 28C
                                 of IEEE 802.3u. */
    uint16_t u1TOG_TX  : 1; /* Toggle:
                               1 = Value of toggle bit in previously transmitted Link Code Word was 0.
                               0 = Value of toggle bit in previously transmitted Link Code Word was 1.
                               Toggle is used by the Arbitration function within Auto-Negotiation to ensure
                               synchronization with the Link Partner during Next Page exchange. This bit shall
                               always take the opposite value of the Toggle bit in the previously exchanged Link
                               Code Word. */
    uint16_t u1ACK2    : 1; /* Acknowledge2:
                               1 = Will comply with message.
                               0 = Cannot comply with message.
                               Acknowledge2 is used by the next page function to indicate that Local Device has
                               the ability to comply with the message received. */
    uint16_t u1MP      : 1; /* Message Page:
                               1 = Message Page.
                               0 = Unformatted Page. */
    uint16_t u1ACK     : 1; /* Acknowledge:
                               1 = Acknowledge reception of link code word
                               0 = Do not acknowledge of link code word. */
    uint16_t u1NP      : 1; /* Next Page Indication:
                               0 = No other Next Page Transfer desired.
                               1 = Another Next Page desired. */
} cy_stc_dp83867_annptr_t;


typedef union
{
    cy_stc_dp83867_annptr_t f;
    uint16_t u16;
} cy_un_dp83867_annptr_t;


/* ANNPRR */
typedef struct
{
    uint16_t u11CODE    : 11; /* Code:
                                 This field represents the code field of the next page transmission. If
                                 the MP bit is set (bit 13 of this register), then the code shall be
                                 interpreted as a "Message Page”, as defined in Annex 28C of IEEE
                                 802.3u. Otherwise, the code shall be interpreted as an "Unformatted
                                 Page”, and the interpretation is application specific.
                                 The default value of the CODE represents a Null Page as defined in
                                 Annex 28C of IEEE 802.3u. */
    uint16_t u1TOG_TX  : 1; /* Toggle:
                               1 = Value of toggle bit in previously transmitted Link Code Word
                               was 0.
                               0 = Value of toggle bit in previously transmitted Link Code Word
                               was 1.
                               Toggle is used by the Arbitration function within Auto-Negotiation to
                               ensure synchronization with the Link Partner during Next Page
                               exchange. This bit shall always take the opposite value of the
                               Toggle bit in the previously exchanged Link Code Word. */
    uint16_t u1ACK2    : 1; /* Acknowledge2:
                               1 = Link partner sets the ACK2 bit.
                               0 = Link partner coes not set the ACK2 bit.
                               Acknowledge2 is used by the next page function to indicate that link
                               partner has the ability to comply with the message received. */
    uint16_t u1MP      : 1; /* Message Page:
                               1 = Received page is a Message Page.
                               0 = Received page is an Unformatted Page. */
    uint16_t u1ACK     : 1; /* Acknowledge:
                               1 = Acknowledge reception of link code word by the link partner.
                               0 = Link partner does not acknowledge reception of link code word. */
    uint16_t u1NP      : 1; /* Next Page Indication:
                               0 = No other Next Page Transfer desired by the link partner.
                               1 = Another Next Page desired by the link partner. */
} cy_stc_dp83867_annprr_t;


typedef union
{
    cy_stc_dp83867_annprr_t f;
    uint16_t u16;
} cy_un_dp83867_annprr_t;


/* CFG1 */
typedef struct
{
    uint16_t u7RESERVED                           : 7; /* RESERVED: Write ignored, read as 0. */
    uint16_t u1TDR_AUTO_RUN                       : 1; /* Automatic TDR on Link Down:
                                                          1 = Enable execution of TDR procedure after link down event.
                                                          0 = Disable automatic execution of TDR. */
    uint16_t u1_1000BASE_T_HALF_DUPLEX            : 1; /* Advertise 1000BASE-T Half Duplex Capable:
                                                          1 = Advertise 1000Base-T Half Duplex ability.
                                                          0 = Do not advertise 1000Base-T Half Duplex ability. */
    uint16_t u1_1000BASE_T_FULL_DUPLEX            : 1; /* Advertise 1000BASE-T Full Duplex Capable:
                                                          1 = Advertise 1000Base-T Full Duplex ability.
                                                          0 = Do not advertise 1000Base-T Full Duplex ability. */
    uint16_t u1PORT_TYPE                          : 1; /* Advertise Device Type: Multi or single port:
                                                          1 = Multi-port device.
                                                          0 = Single-port device. */
    uint16_t u1MASTER_SLAVE_CONFIGURATION_VALUE   : 1; /* Manual Master / Slave Configuration Value:
                                                          1 = Set PHY as MASTER when register 09h bit 12 = 1.
                                                          0 = Set PHY as SLAVE when register 09h bit 12 = 1.
                                                          Using the manual configuration feature may prevent the PHY from
                                                          establishing link in 1000Base-T mode if a conflict with the link
                                                          partner’s setting exists. */
    uint16_t u1MASTER_SLAVE_MANUAL_CONFIGURATION  : 1; /* Enable Manual Master / Slave Configuration:
                                                          1 = Enable Manual Master/Slave Configuration control.
                                                          0 = Disable Manual Master/Slave Configuration control.
                                                          Using the manual configuration feature may prevent the PHY from
                                                          establishing link in 1000Base-T mode if a conflict with the link
                                                          partner’s setting exists. */
    uint16_t u3TEST_MODE                          : 3; /* Test Mode Select:
                                                          111 = Test Mode 7 - Repetitive {Pulse, 63 zeros}
                                                          110 = Test Mode 6 - Repetitive 0001 sequence
                                                          101 = Test Mode 5 - Scrambled MLT3 Idles
                                                          100 = Test Mode 4 - Transmit Distortion Test
                                                          011 = Test Mode 3 - Transmit Jitter Test (Slave Mode)
                                                          010 = Test Mode 2 - Transmit Jitter Test (Master Mode)
                                                          001 = Test Mode 1 - Transmit Waveform Test
                                                          000 = Normal Mode */
} cy_stc_dp83867_cfg1_t;


typedef union
{
    cy_stc_dp83867_cfg1_t f;
    uint16_t u16;
} cy_un_dp83867_cfg1_t;


/* STS1 */
typedef struct
{
    uint16_t u8IDLE_ERROR_COUNTER                     : 8; /* 1000BASE-T Idle Error Counter */
    uint16_t u2RESERVED                               : 2; /* RESERVED by IEEE: Writes ignored, read as 0. */
    uint16_t u1_1000BASE_T_HALF_DUPLEX                : 1; /* Link Partner 1000BASE-T Half Duplex Capable:
                                                              1 = Link Partner capable of 1000Base-T Half Duplex.
                                                              0 = Link partner not capable of 1000Base-T Half Duplex. */
    uint16_t u1_1000BASE_T_FULL_DUPLEX                : 1; /* Link Partner 1000BASE-T Full Duplex Capable:
                                                              1 = Link Partner capable of 1000Base-T Full Duplex.
                                                              0 = Link partner not capable of 1000Base-T Full Duplex. */
    uint16_t u1REMOTE_RECEIVER_STATUS                 : 1; /* Remote Receiver Status:
                                                              1 = Remote receiver is OK.
                                                              0 = Remote receiver is not OK. */
    uint16_t u1LOCAL_RECEIVER_STATUS                  : 1; /* Local Receiver Status:
                                                              1 = Local receiver is OK.
                                                              0 = Local receiver is not OK. */
    uint16_t u1MASTER_SLAVE_CONFIGURATION_RESOLUTION  : 1; /* Master / Slave Configuration Results:
                                                              1 = Configuration resolved to MASTER.
                                                              0 = Configuration resolved to SLAVE. */
    uint16_t u1MASTER_SLAVE_CONFIGURATION_FAULT       : 1; /* Master / Slave Manual Configuration Fault Detected:
                                                              1 = Manual Master/Slave Configuration fault detected.
                                                              0 = No Manual Master/Slave Configuration fault detected. */
} cy_stc_dp83867_sts1_t;


typedef union
{
    cy_stc_dp83867_sts1_t f;
    uint16_t u16;
} cy_un_dp83867_sts1_t;


/* REGCR */
typedef struct
{
    uint16_t u5DEVAD     : 5; /* Device Address: In general, these bits [4:0] are the device address
                                 DEVAD that directs any accesses of ADDAR register (0x000E) to
                                 the appropriate MMD. Specifically, the DP83867 uses the vendor
                                 specific DEVAD [4:0] = 11111 for accesses. All accesses through
                                 registers REGCR and ADDAR should use this DEVAD.
                                 Transactions with other DEVAD are ignored. */
    uint16_t u9RESERVED  : 9; /* RESERVED: Writes ignored, read as 0. */
    uint16_t u2FUNCTION  : 2; /* 00 = Address
                                 01 = Data, no post increment
                                 10 = Data, post increment on read and write
                                 11 = Data, post increment on write only */
} cy_stc_dp83867_regcr_t;


typedef union
{
    cy_stc_dp83867_regcr_t f;
    uint16_t u16;
} cy_un_dp83867_regcr_t;


/* ADDAR */
typedef struct
{
    uint16_t u16ADDRESS_DATA  : 16; /* If REGCR register 15:14 = 00, holds the MMD DEVAD's address
                                       register, otherwise holds the MMD DEVAD's data register */
} cy_stc_dp83867_addar_t;


typedef union
{
    cy_stc_dp83867_addar_t f;
    uint16_t u16;
} cy_un_dp83867_addar_t;


/* _1KSCR */
typedef struct
{
    uint16_t u12RESERVED                     : 12; /* RESERVED by IEEE: Writes ignored, read as 0. */
    uint16_t u1_1000BASE_T_HALF_DUPLEX      : 1; /* 1000BASE-T Half Duplex Support:
                                                    1 = 1000BASE-T Half Duplex is supported by the local device.
                                                    0 = 1000BASE-T Half Duplex is not supported by the local device. */
    uint16_t u1_1000BASE_T_FULL_DUPLEX      : 1; /* 1000BASE-T Full Duplex Support:
                                                    1 = 1000BASE-T Full Duplex is supported by the local device.
                                                    0 = 1000BASE-T Full Duplex is not supported by the local device. */
    uint16_t u1_1000BASE_X_HALF_DUPLEX      : 1; /* 1000BASE-X Half Duplex Support:
                                                    1 = 1000BASE-X Half Duplex is supported by the local device.
                                                    0 = 1000BASE-X Half Duplex is not supported by the local device. */
    uint16_t u1_1000BASE_T_Status_Register  : 1; /* 1000BASE-X Full Duplex Support:
                                                    1 = 1000BASE-X Full Duplex is supported by the local device.
                                                    0 = 1000BASE-X Full Duplex is not supported by the local device. */
} cy_stc_dp83867__1kscr_t;


typedef union
{
    cy_stc_dp83867__1kscr_t f;
    uint16_t u16;
} cy_un_dp83867__1kscr_t;


/* PHYCR */
typedef struct
{
    uint16_t u1DISABLE_JABBER      : 1; /* Disable Jabber
                                           1 = Disable Jabber function.
                                           0 = Enable Jabber function. */
    uint16_t u1LINE_DRIVER_INV_EN  : 1; /* Line Driver Inversion Enable:
                                           1 = Invert Line Driver Transmission.
                                           0 = Normal operation. */
    uint16_t u1STANDBY_MODE        : 1; /* Standby Mode:
                                           1 = Enable standby mode. Digital and analog circuitry are powered
                                           up, but no link can be established.
                                           0 = Normal operation. */
    uint16_t u1RESERVED1           : 1; /* RESERVED: Writes ignored, read as 1. */
    uint16_t u1DISABLE_CLK_125     : 1; /* Disable 125MHz Clock:
                                           This bit may be used in conjunction with POWER_SAVE_MODE
                                           (bits 9:8 of this register).
                                           1 = Disable CLK125.
                                           0 = Enable CLK125. */
    uint16_t u2MDI_CROSSOVER       : 2; /* MDI Crosssover Mode:
                                           1x = Enable automatic crossover
                                           1 = Manual MDI-X configuration
                                           0 = Manual MDI configuration */
    uint16_t u1DEEP_POWER_DOWN_EN  : 1; /* Deep power-down mode enable
                                           1 = When power down is initiated through assertion of the external
                                           power-down pin or through the POWER_DOWN bit in the BMCR,
                                           the device enters a deep power-down mode.
                                           0 = Normal operation. */
    uint16_t u2POWER_SAVE_MODE     : 2; /* Power-Saving Modes:
                                           11 = Passive Sleep mode: Power down all digital and analog
                                           blocks.
                                           10 =Active Sleep mode: Power down all digital and analog blocks.
                                           Automatic power-up is performed when link partner is detected. Link
                                           pulses are transmitted approximately once per 1.4 Sec in this mode
                                           to wake up any potential link partner.
                                           01 = IEEE mode: power down all digital and analog blocks.
                                           Note: If DISABLE_CLK_125 (bit [4]of this register) is set to zero, the
                                           PLL is also powered down.
                                           00 = Normal mode */
    uint16_t u1FORCE_LINK_GOOD     : 1; /* Force Link Good:
                                           1 = Force link good according to the selected speed.
                                           0 = Normal operation */
    uint16_t u3RESERVED0           : 3; /* RESERVED */
    uint16_t u2TX_FIFO_DEPTH       : 2; /* TX FIFO Depth:
                                           11 = 8 bytes/nibbles (1000Mbps/Other Speeds)
                                           10 = 6 bytes/nibbles (1000Mbps/Other Speeds)
                                           01 = 4 bytes/nibbles (1000Mbps/Other Speeds)
                                           00 = 3 bytes/nibbles (1000Mbps/Other Speeds)
                                           Note: FIFO is enabled only in the following modes:
                                           1000BaseT + GMII */
} cy_stc_dp83867_phycr_t;


typedef union
{
    cy_stc_dp83867_phycr_t f;
    uint16_t u16;
} cy_un_dp83867_phycr_t;


/* PHYSTS */
typedef struct
{
    uint16_t u1JABBER_DETECT          : 1; /* Jabber Detect: This bit only has meaning in 10 Mbps mode.
                                              This bit is a duplicate of the Jabber Detect bit in the BMSR register,
                                              except that it is not cleared upon a read of the PHYSTS register.
                                              1 = Jabber condition detected.
                                              0 = No Jabber. */
    uint16_t u1POLARITY_STATUS        : 1; /* 10BASE-Te Polarity Status:
                                              1 = Correct Polarity detected.
                                              0 = Inverted Polarity detected. */
    uint16_t u4WIRE_CROSS             : 4; /* Crossed Wire Indication:
                                              Indicates channel polarity in 1000BASE-T linked status. Bits [5:2]
                                              correspond to channels [D,C,B,A], respectively.
                                              1 = Channel polarity is reversed.
                                              0 = Channel polarity is normal. */
    uint16_t u1SLEEP_MODE             : 1; /* Sleep Mode Status:
                                              1 = Device currently in sleep mode.
                                              0 = Device currently in active mode. */
    uint16_t u1SPEED_OPT_STATUS       : 1; /* Speed Optimization Status:
                                              1 = Auto-Negotiation is currently being performed with Speed
                                              Optimization masking 1000BaseT abilities (Valid only during Auto-
                                              Negotiation).
                                              0 = Auto-Negotiation is currently being performed without Speed
                                              Optimization. */
    uint16_t u1MDI_X_MODE_AB          : 1; /* MDI/MDIX Resolution Status for A and B Line Driver Pairs:
                                              1 = Resolved as MDIX
                                              0 = Resolved as MDI. */
    uint16_t u1MDI_X_MODE_CD          : 1; /* MDI/MDIX Resolution Status for C and D Line Driver Pairs:
                                              1 = Resolved as MDIX
                                              0 = Resolved as MDI. */
    uint16_t u1LINK_STATUS            : 1; /* Link Status:
                                              1 = Link is up.
                                              0 = Link is down. */
    uint16_t u1SPEED_DUPLEX_RESOLVED  : 1; /* Speed Duplex Resolution Status:
                                              1 = Auto-Negotiation has completed or is disabled.
                                              0 = Auto-Negotiation is enabled and has not completed. */
    uint16_t u1PAGE_RECEIVED          : 1; /* Page Received:
                                              This bit is latched high and will be cleared upon a read.
                                              1 = Page received.
                                              0 = No page received. */
    uint16_t u1DUPLEX_MODE            : 1; /* Duplex Mode Status:
                                              1 = Full Duplex
                                              0 = Half Duplex. */
    uint16_t u2SPEED_SELECTION        : 2; /* Speed Select Status:
                                              These two bits indicate the speed of operation as determined by
                                              Auto-Negotiation or as set by manual configuration.
                                              11 = Reserved
                                              10 = 1000 Mbps
                                              01 = 100 Mbps
                                              00 = 10 Mbps */
} cy_stc_dp83867_physts_t;


typedef union
{
    cy_stc_dp83867_physts_t f;
    uint16_t u16;
} cy_un_dp83867_physts_t;


/* MICR */
typedef struct
{
    uint16_t u1JABBER_INT_EN              : 1; /* Enable Jabber Interrupt:
                                                  1 = Enable Jabber interrupt.
                                                  0 = Disable Jabber interrupt. */
    uint16_t u1POLARITY_CHNG_INT_EN       : 1; /* Enable Polarity Change Interrupt:
                                                  1 = Enable Polarity Change interrupt.
                                                  0 = Disable Polarity Change interrupt. */
    uint16_t u1XGMII_ERR_INT_EN           : 1; /* Enable xGMII Error Interrupt:
                                                  1 = Enable xGMII Error Interrupt.
                                                  0 = Disable xGMII Error Interrupt */
    uint16_t u1WOL_INT_EN                 : 1; /* Enable Wake-on-LAN Interrupt:
                                                  1 = Enable Wake-on-LAN Interrupt.
                                                  0 = Disable Wake-on-LAN Interrupt. */
    uint16_t u1SLEEP_MODE_CHNG_INT_EN     : 1; /* Enable Sleep Mode Change Interrupt:
                                                  1 = Enable Sleep Mode Change Interrupt.
                                                  0 = Disable Sleep Mode Change Interrupt. */
    uint16_t u1SPEED_OPT_EVENT_INT_EN     : 1; /* Enable Speed Optimization Event Interrupt:
                                                  1 = Enable Speed Optimization Event Interrupt.
                                                  0 = Disable Speed Optimization Event Interrupt. */
    uint16_t u1MDI_CROSSOVER_CHNG_INT_EN  : 1; /* Enable MDI Crossover Change Interrupt:
                                                  1 = Enable MDI Crossover Change interrupt.
                                                  0 = Disable MDI Crossover Change interrupt. */
    uint16_t u1RESERVED1                  : 1; /* RESERVED */
    uint16_t u1FALSE_CARRIER_INT_EN       : 1; /* Enable False Carrier Interrupt:
                                                  1 = Enable False Carrier interrupt.
                                                  0 = Disable False Carrier interrupt. */
    uint16_t u1RESERVED0                  : 1; /* RESERVED */
    uint16_t u1LINK_STATUS_CHNG_INT_EN    : 1; /* Enable Link Status Change Interrupt:
                                                  1 = Enable Link Status Change interrupt.
                                                  0 = Disable Link Status Change interrupt. */
    uint16_t u1AUTONEG_COMP_INT_EN        : 1; /* Enable Auto-Negotiation Complete Interrupt:
                                                  1 = Enable Auto-Negotiation Complete Interrupt.
                                                  0 = Disable Auto-Negotiation Complete Interrupt. */
    uint16_t u1PAGE_RECEIVED_INT_EN       : 1; /* Enable Page Received Interrupt:
                                                  1 = Enable Page Received Interrupt.
                                                  0 = Disable Page Received Interrupt. */
    uint16_t u1DUPLEX_MODE_CHNG_INT_EN    : 1; /* Enable Duplex Mode Change Interrupt:
                                                  1 = Enable Duplex Mode Change interrupt.
                                                  0 = Disable Duplex Mode Change interrupt. */
    uint16_t u1SPEED_CHNG_INT_EN          : 1; /* Enable Speed Change Interrupt:
                                                  1 = Enable Speed Change interrupt.
                                                  0 = Disable Speed Change interrupt. */
    uint16_t u1AUTONEG_ERR_INT_EN         : 1; /* Enable Auto-Negotiation Error Interrupt:
                                                  1 = Enable Auto-Negotiation Error interrupt.
                                                  0 = Disable Auto-Negotiation Error interrupt. */
} cy_stc_dp83867_micr_t;


typedef union
{
    cy_stc_dp83867_micr_t f;
    uint16_t u16;
} cy_un_dp83867_micr_t;


/* ISR */
typedef struct
{
    uint16_t u1JABBER_INT              : 1; /* Jabber Interrupt:
                                               1 = Jabber interrupt is pending and is cleared by the current read.
                                               0 = No Jabber interrupt is pending. */
    uint16_t u1POLARITY_CHNG_INT       : 1; /* Polarity Change Interrupt:
                                               1 = Polarity Change interrupt is pending and is cleared by the
                                               current read.
                                               0 = No Polarity Change interrupt is pending. */
    uint16_t u1XGMII_ERR_INT           : 1; /* xGMII Error Interrupt:
                                               1 = xGMII Error Interrupt is pending and is cleared by the current
                                               read.
                                               0 = No xGMII Error Interrupt is pending. */
    uint16_t u1WOL_INT                 : 1; /* Wake-on-LAN Interrupt:
                                               1 = Wake-on-LAN Interrupt is pending.
                                               0 = No Wake-on-LAN Interrupt is pending. */
    uint16_t u1SLEEP_MODE_CHNG_INT     : 1; /* Sleep Mode Change Interrupt:
                                               1 = Sleep Mode Change Interrupt is pending and is cleared by the
                                               current read.
                                               0 = No Sleep Mode Change Interrupt is pending. */
    uint16_t u1SPEED_OPT_EVENT_INT     : 1; /* Speed Optimization Event Interrupt:
                                               1 = Speed Optimization Event Interrupt is pending and is cleared by
                                               the current read.
                                               0 = No Speed Optimization Event Interrupt is pending. */
    uint16_t u1MDI_CROSSOVER_CHNG_INT  : 1; /* MDI Crossover Change Interrupt:
                                               1 = MDI Crossover Change interrupt is pending and is cleared by
                                               the current read.
                                               0 = No MDI Crossover Change interrupt is pending. */
    uint16_t u1RESERVED1               : 1; /* RESERVED */
    uint16_t u1FALSE_CARRIER_INT       : 1; /* False Carrier Interrupt:
                                               1 = False Carrier interrupt is pending and is cleared by the current
                                               read.
                                               0 = No False Carrier interrupt is pending. */
    uint16_t u1RESERVED0               : 1; /* RESERVED */
    uint16_t u1LINK_STATUS_CHNG_INT    : 1; /* Link Status Change Interrupt:
                                               1 = Link Status Change interrupt is pending and is cleared by the
                                               current read.
                                               0 = No Link Status Change interrupt is pending. */
    uint16_t u1AUTONEG_COMP_INT        : 1; /* Auto-Negotiation Complete Interrupt:
                                               1 = Auto-Negotiation Complete Interrupt is pending and is cleared
                                               by the current read.
                                               0 = No Auto-Negotiation Complete Interrupt is pending. */
    uint16_t u1PAGE_RECEIVED_INT       : 1; /* Page Received Interrupt:
                                               1 = Page Received Interrupt is pending and is cleared by the
                                               current read.
                                               0 = No Page Received Interrupt is pending. */
    uint16_t u1DUPLEX_MODE_CHNG_INT    : 1; /* Duplex Mode Change Interrupt:
                                               1 = Duplex Mode Change interrupt is pending and is cleared by the
                                               current read.
                                               0 = No Duplex Mode Change interrupt. */
    uint16_t u1SPEED_CHNG_INT          : 1; /* Speed Change Interrupt:
                                               1 = Speed Change interrupt is pending and is cleared by the current
                                               read.
                                               0 = No Speed Change interrupt. */
    uint16_t u1AUTONEG_ERR_INT         : 1; /* Auto-Negotiation Error Interrupt:
                                               1 = Auto-Negotiation Error interrupt is pending and is cleared by the
                                               current read.
                                               0 = No Auto-Negotiation Error interrupt. */
} cy_stc_dp83867_isr_t;


typedef union
{
    cy_stc_dp83867_isr_t f;
    uint16_t u16;
} cy_un_dp83867_isr_t;


/* CFG2 */
typedef struct
{
    uint16_t u6RESERVED3              : 6; /* RESERVED */
    uint16_t u1SPEED_OPT_10M_EN       : 1; /* Enable Speed Optimization to 10BASE-Te:
                                              1 = Enable speed optimization to 10BASE-Te if link establishment
                                              fails in 1000BASE-T and 100BASE-TX .
                                              0 = Disable speed optimization to 10BASE-Te. */
    uint16_t u1RESERVED2              : 1; /* RESERVED */
    uint16_t u1SPEED_OPT_ENHANCED_EN  : 1; /* Speed Optimization Enhanced Mode Enable:
                                              In enhanced mode, speed is optimized if energy is not detected in
                                              channels C and D.
                                              1 = Enable Speed Optimization enhanced mode.
                                              0 = Disable Speed Optimization enhanced mode. */
    uint16_t u1SPEED_OPT_EN           : 1; /* Speed Optimization Enable:
                                              1 = Enable Speed Optimization.
                                              0 = Disable Speed Optimization. */
    uint16_t u2SPEED_OPT_ATTEMPT_CNT  : 2; /* Speed Optimization Attempt Count:
                                              Selects the number of 1000BASE-T link establishment attempt
                                              failures prior to performing Speed Optimization.
                                              11 = 8
                                              10 = 4
                                              01 = 2
                                              00 = 1 */
    uint16_t u1RESERVED1              : 1; /* RESERVED */
    uint16_t u1INTERRUPT_POLARITY     : 1; /* Configure Interrupt Polarity:
                                              1 = Interrupt pin is active low.
                                              0 = Interrupt pin is active high. */
    uint16_t u2RESERVED0              : 2; /* RESERVED: Writes ignored, read as 0. */
} cy_stc_dp83867_cfg2_t;


typedef union
{
    cy_stc_dp83867_cfg2_t f;
    uint16_t u16;
} cy_un_dp83867_cfg2_t;


/* RECR */
typedef struct
{
    uint16_t u16RXERCNT  : 16; /* RX_ER Counter:
                                  Receive error counter. This register saturates at the maximum value
                                  of 0xFFFF. It is cleared by dummy write to this register. */
} cy_stc_dp83867_recr_t;


typedef union
{
    cy_stc_dp83867_recr_t f;
    uint16_t u16;
} cy_un_dp83867_recr_t;


/* BISCR */
typedef struct
{
    uint16_t u2PCS_LOOPBACK           : 2; /* PCS Loopback Select:
                                              When configured for 100Base-TX:
                                              11: Loop after MLT3 encoder (full TX/RX path)
                                              10: Loop after scrambler, before MLT3 encoder
                                              01: Loop before scrambler
                                              When configured for 1000Base-T:
                                              x1: Loop before 1000Base-T signal processing. */
    uint16_t u4LOOPBACK_MODE          : 4; /* Loopback Mode Select:
                                              PCS Loopback must be disabled (Bits [1:0] =00) prior to selecting
                                              the loopback mode.
                                              1000: Reverse loop
                                              0100: External loop
                                              0010: Analog loop
                                              0001: Digital loop */
    uint16_t u1MII_LOOP_TX_DATA_CTRL  : 1; /* MII Loopback Transmit Data Control:
                                              This bit may only be set in MII Loopback mode.
                                              1 = Transmit data to MDI in MII loop
                                              0 = Suppress data to MDI in MII loop */
    uint16_t u1REV_LOOP_RX_DATA_CTRL  : 1; /* Reverse Loopback Receive Data Control:
                                              This bit may only be set in Reverse Loopback mode.
                                              1 = Send RX packets to MAC in reverse loop
                                              0 = Suppress RX packets to MAC in reverse loop */
    uint16_t u4RESERVED               : 4; /* RESERVED: Writes ignored, read as 0. */
    uint16_t u1PACKET_GEN_EN          : 1; /* BIST Packet Size:
                                              1 = Transmit 64 byte packets in packet generation mode.
                                              0 = Transmit 1518 byte packets in packet generation mode */
    uint16_t u1PACKET_GEN_64BIT_MODE  : 1; /* BIST Packet Size:
                                              1 = Transmit 64 byte packets in packet generation mode.
                                              0 = Transmit 1518 byte packets in packet generation mode */
    uint16_t u1GEN_PRBS_PACKET        : 1; /* Generated PRBS Packets:
                                              1 = When the packet generator is enabled, it will generate
                                              continuous packets with PRBS data. When the packet generator is
                                              disabled, the PRBS checker is still enabled.
                                              0 = When the packet generator is enabled, it will generate a single
                                              packet with constant data. PRBS generation and checking is
                                              disabled. */
    uint16_t u1PRBS_COUNT_MODE        : 1; /* PRBS Continuous Mode:
                                              1 = Continuous mode enabled. When one of the PRBS counters
                                              reaches the maximum value, a pulse is generated and the counter
                                              starts counting from zero again. This bit must be set for proper
                                              PRBS operation.
                                              0 = PRBS continuous mode disabled. PRBS operation is not
                                              supported for this setting. */
} cy_stc_dp83867_biscr_t;


typedef union
{
    cy_stc_dp83867_biscr_t f;
    uint16_t u16;
} cy_un_dp83867_biscr_t;


/* STS2 */
typedef struct
{
    uint16_t u6RESERVED1           : 6; /* RESERVED: Writes ignored, read as 0. */
    uint16_t u1CORE_PWR_MODE       : 1; /* Core Power Mode:
                                           1 = Core is in normal power mode.
                                           0 = Core is power-down mode or in sleep mode. */
    uint16_t u1SCR_MODE_SLAVE_1G   : 1; /* Gigabit Slave Scramble Mode:
                                           1 = 1G PCS (slave) is in legacy encoding mode.
                                           0 = 1G PCS (slave) is in normal encoding mode. */
    uint16_t u1SCR_MODE_MASTER_1G  : 1; /* Gigabit Master Scramble Mode:
                                           1 = 1G PCS (master) is in legacy encoding mode.
                                           0 = 1G PCS (master) is in normal encoding mode. */
    uint16_t u1PKT_GEN_BUSY        : 1; /* Packet Generator Busy:
                                           1 = Packet generation is in process.
                                           0 = Packet generation is not in process. */
    uint16_t u1PRBS_LOCK_LOST      : 1; /* PRBS Lock Lost:
                                           1 = PRBS checker has lost lock.
                                           0 = PRBS checker has not lost lock. */
    uint16_t u1PRBS_LOCK           : 1; /* PRBS Lock Status:
                                           1 = PRBS checker is locked to the received byte stream.
                                           0 = PRBS checker is not locked. */
    uint16_t u4RESERVED0           : 4; /* RESERVED: Writes ignored, read as 0. */
} cy_stc_dp83867_sts2_t;


typedef union
{
    cy_stc_dp83867_sts2_t f;
    uint16_t u16;
} cy_un_dp83867_sts2_t;


/* LEDCR1 */
typedef struct
{
    uint16_t u4LED_0_SEL     : 4; /* Source of LED_0:
                                     1111: Reserved
                                     1110: Receive Error
                                     1101: Receive Error or Transmit Error
                                     1100: RESERVED
                                     1011: Link established, blink for transmit or receive activity
                                     1010: Full duplex
                                     1001: 100/1000BT link established
                                     1000: 10/100BT link established
                                     0111: 10BT link established
                                     0110: 100 BTX link established
                                     0101: 1000BT link established
                                     0100: Collision detected
                                     0011: Receive activity
                                     0010: Transmit activity
                                     0001: Receive or Transmit activity
                                     0000: Link established */
    uint16_t u4LED_1_SEL     : 4; /* Source of LED_1:
                                     1111: Reserved
                                     1110: Receive Error
                                     1101: Receive Error or Transmit Error
                                     1100: RESERVED
                                     1011: Link established, blink for transmit or receive activity
                                     1010: Full duplex
                                     1001: 100/1000BT link established
                                     1000: 10/100BT link established
                                     0111: 10BT link established
                                     0110: 100 BTX link established
                                     0101: 1000BT link established
                                     0100: Collision detected
                                     0011: Receive activity
                                     0010: Transmit activity
                                     0001: Receive or Transmit activity
                                     0000: Link established */
    uint16_t u4LED_2_SEL     : 4; /* Source of LED_2:
                                     1111: Reserved
                                     1110: Receive Error
                                     1101: Receive Error or Transmit Error
                                     1100: RESERVED
                                     1011: Link established, blink for transmit or receive activity
                                     1010: Full duplex
                                     1001: 100/1000BT link established
                                     1000: 10/100BT link established
                                     0111: 10BT link established
                                     0110: 100 BTX link established
                                     0101: 1000BT link established
                                     0100: Collision detected
                                     0011: Receive activity
                                     0010: Transmit activity
                                     0001: Receive or Transmit activity
                                     0000: Link established */
    uint16_t u4LED_GPIO_SEL  : 4; /* Source of the GPIO LED_3:
                                     1111: Reserved
                                     1110: Receive Error
                                     1101: Receive Error or Transmit Error
                                     1100: RESERVED
                                     1011: Link established, blink for transmit or receive activity
                                     1010: Full duplex
                                     1001: 100/1000BT link established
                                     1000: 10/100BT link established
                                     0111: 10BT link established
                                     0110: 100 BTX link established
                                     0101: 1000BT link established
                                     0100: Collision detected
                                     0011: Receive activity
                                     0010: Transmit activity
                                     0001: Receive or Transmit activity
                                     0000: Link established */
} cy_stc_dp83867_ledcr1_t;


typedef union
{
    cy_stc_dp83867_ledcr1_t f;
    uint16_t u16;
} cy_un_dp83867_ledcr1_t;


/* LEDCR2 */
typedef struct
{
    uint16_t u1LED_0_DRV_EN       : 1; /* LED_0 Drive Enable:
                                          1 = Force the value of the LED_0_DRV_VAL bit onto LED_0.
                                          0 = Normal operation */
    uint16_t u1LED_0_DRV_VAL      : 1; /* LED_0 Drive Value:
                                          Value to force on LED_0
                                          This bit is only valid if enabled through LED_0_DRV_EN. */
    uint16_t u1LED_0_POLARITY     : 1; /* LED_0 Polarity:
                                          1 = Active high
                                          0 = Active low */
    uint16_t u1RESERVED3          : 1; /* RESERVED: Writes ignored, read as 0. */
    uint16_t u1LED_1_DRV_EN       : 1; /* LED_1 Drive Enable:
                                          1 = Force the value of the LED_1_DRV_VAL bit onto LED_1.
                                          0 = Normal operation */
    uint16_t u1LED_1_DRV_VAL      : 1; /* LED_1 Drive Value:
                                          Value to force on LED_1
                                          This bit is only valid if enabled through LED_1_DRV_EN. */
    uint16_t u1LED_1_POLARITY     : 1; /* LED_1 Polarity:
                                          1 = Active high
                                          0 = Active low */
    uint16_t u1RESERVED2          : 1; /* RESERVED: Writes ignored, read as 0. */
    uint16_t u1LED_2_DRV_EN       : 1; /* LED_2 Drive Enable:
                                          1 = Force the value of the LED_2_DRV_VAL bit onto LED_2.
                                          0 = Normal operation */
    uint16_t u1LED_2_DRV_VAL      : 1; /* LED_2 Drive Value:
                                          Value to force on LED_2
                                          This bit is only valid if enabled through LED_2_DRV_EN. */
    uint16_t u1LED_2_POLARITY     : 1; /* LED_2 Polarity:
                                          1 = Active high
                                          0 = Active low */
    uint16_t u1RESERVED1          : 1; /* RESERVED: Writes ignored, read as 0. */
    uint16_t u1LED_GPIO_DRV_EN    : 1; /* GPIO LED Drive Enable:
                                          1 = Force the value of the LED_GPIO_DRV_VAL bit onto the GPIO LED.
                                          0 = Normal operation */
    uint16_t u1LED_GPIO_DRV_VAL   : 1; /* GPIO LED Drive Value:
                                          Value to force on GPIO LED
                                          This bit is only valid if enabled through LED_GPIO_DRV_EN. */
    uint16_t u1LED_GPIO_POLARITY  : 1; /* GPIO LED Polarity:
                                          1 = Active high
                                          0 = Active low */
    uint16_t u1RESERVED0          : 1; /* RESERVED: Writes ignored, read as 0. */
} cy_stc_dp83867_ledcr2_t;


typedef union
{
    cy_stc_dp83867_ledcr2_t f;
    uint16_t u16;
} cy_un_dp83867_ledcr2_t;


/* LEDCR3 */
typedef struct
{
    uint16_t u2LEDS_BLINK_RATE         : 2; /* LED Blink Rate:
                                               11: 2 Hz (500 ms)
                                               10: 5 Hz (200 ms)
                                               01: 10 Hz (100 ms)
                                               00 = 20 Hz (50 ms) */
    uint16_t u1LEDS_BYPASS_STRETCHING  : 1; /* Bypass LED Stretching:
                                               1 = Bypass LED Stretching
                                               0 = Normal operation */
    uint16_t u13RESERVED                : 13; /* RESERVED: Writes ignored, read as 0. */
} cy_stc_dp83867_ledcr3_t;


typedef union
{
    cy_stc_dp83867_ledcr3_t f;
    uint16_t u16;
} cy_un_dp83867_ledcr3_t;


/* CFG3 */
typedef struct
{
    uint16_t u1TDR_START                        : 1; /* TDR Start:
                                                        1 = Start TDR.
                                                        0 = Normal operation */
    uint16_t u1TDR_DONE                         : 1; /* TDR Done:
                                                        1 = TDR has completed.
                                                        0 = TDR has not completed. */
    uint16_t u1TDR_FAIL                         : 1; /* TDR Failure:
                                                        1 = TDR failed.
                                                        0 = Normal TDR operation. */
    uint16_t u3RESERVED                         : 3; /* RESERVED: Writes ignored, read as 0. */
    uint16_t u1FORCE_INTERRUPT                  : 1; /* Force Interrupt:
                                                        1 = Assert interrupt pin.
                                                        0 = Normal interrupt mode. */
    uint16_t u1INT_OE                           : 1; /* Interrupt Output Enable:
                                                        1 = INTN/PWDNN Pad is an Interrupt Output.
                                                        0 = INTN/PWDNN Pad in a Power-Down Input. */
    uint16_t u1FAST_AUTO_MDIX                   : 1; /* Fast Auto MDI/MDIX:
                                                        1 = Enable Fast Auto MDI/MDIX mode
                                                        0 = Normal Auto MDI/MDIX mode
                                                        If both link partners are configured to work in Force 100Base-TX
                                                        mode (Auto-Negotiation is disabled), this mode enables Automatic
                                                        MDI/MDIX resolution in a short time. */
    uint16_t u1ROBUST_AUTO_MDIX                 : 1; /* Robust Auto-MDIX:
                                                        1 =Enable Robust Auto MDI/MDIX resolution
                                                        0 = Normal Auto MDI/MDIX mode
                                                        If link partners are configured to operational modes that are not
                                                        supported by normal Auto MDI/MDIX mode (like Auto-Neg versus
                                                        Force 100Base-TX or Force 100Base-TX versus Force 100Base-
                                                        TX), this Robust Auto MDI/MDIX mode allows MDI/MDIX
                                                        resolution and prevents deadlock. */
    uint16_t u1RESERVED                         : 1; /* RESERVED */
    uint16_t u1EXTENDED_FD_ABILITY              : 1; /* Extended Full-Duplex Ability:
                                                        1 = Force Full-Duplex while working with link partner in forced
                                                        100B-TX. When the PHY is set to Auto-Negotiation or Force 100B-
                                                        TX and the link partner is operated in Force 100B-TX, the link is
                                                        always Full Duplex
                                                        0 = Disable Extended Full Duplex Ability. Decision to work in Full
                                                        Duplex or Half Duplex mode follows IEEE specification. */
    uint16_t u2FAST_AN_SEL                      : 2; /* Fast Auto-Negotiation Select bits:
                                                        Fast AN Select: Break Link Timer(ms): Link Fall Inhibit Timer(ms): Auto-Neg Wait Timer(ms)
                                                        <00> 80 50 35
                                                        <01> 120 75 50
                                                        <10> 240 150 100
                                                        <11> NA NA NA
                                                        Adjusting these bits reduces the time it takes to auto-negotiate
                                                        between two PHYs. In Fast AN mode, both PHYs should be
                                                        configured to the same configuration. These 2 bits define the
                                                        duration for each state of the Auto-Negotiation process according
                                                        to the table above. The new duration time must be enabled by
                                                        setting Fast AN En - bit 4 of this register. Note: Using this mode in
                                                        cases where both link partners are not configured to the same
                                                        Fast Auto-Negotiation configuration might produce scenarios with
                                                        unexpected behavior. */
    uint16_t u1FAST_AN_ENABLE                   : 1; /* Fast Auto-Negotiation Enable:
                                                        1 = Enable Fast Auto-Negotiation mode  The PHY auto-
                                                        negotiates using Timer setting according to Fast AN Sel bits
                                                        0 = Disable Fast Auto-Negotiation mode  The PHY auto-
                                                        negotiates using normal Timer setting
                                                        Adjusting these bits reduces the time it takes to Auto-negotiate
                                                        between two PHYs. Note: When using this option care must be
                                                        taken to maintain proper operation of the system. While shortening
                                                        these timer intervals may not cause problems in normal operation,
                                                        there are certain situations where this may lead to problems. */
    uint16_t u1Fast_LINK_UP_IN_PARALLEL_DETECT  : 1; /* Fast Link-Up in Parallel Detect Mode:
                                                        1 = Enable Fast Link-Up time During Parallel Detection
                                                        0 = Normal Parallel Detection link establishment
                                                        In Fast Auto MDI-X this bit is automatically set. */
} cy_stc_dp83867_cfg3_t;


typedef union
{
    cy_stc_dp83867_cfg3_t f;
    uint16_t u16;
} cy_un_dp83867_cfg3_t;


/* CTRL */
typedef struct
{
    uint16_t u14RESERVED    : 14; /* RESERVED: Writes ignored, read as 0. */
    uint16_t u1SW_RESTART  : 1; /* Software Restart:
                                   1 = Perform a full reset, not including registers. .
                                   0 = Normal operation. */
    uint16_t u1SW_RESET    : 1; /* Software Reset:
                                   1 = Perform a full reset, including registers.
                                   0 = Normal operation. */
} cy_stc_dp83867_ctrl_t;


typedef union
{
    cy_stc_dp83867_ctrl_t f;
    uint16_t u16;
} cy_un_dp83867_ctrl_t;


/* TMCH_CTRL */
typedef struct
{
    uint16_t u5RESERVED   : 5; /* RESERVED */
    uint16_t u3TM_CH_SEL  : 3; /* Test mode Channel Select.
                                  If bit 7 is set then Test mode is driven on all 4 channels. If bit 7 is
                                  cleared then test modes are driven according to bits 6:5 as follows:
                                  00: Channel A
                                  01: Channel B
                                  10: Channel C
                                  11: Channel D */
    uint16_t u8RESERVED   : 8; /* RESERVED */
} cy_stc_dp83867_tmch_ctrl_t;


typedef union
{
    cy_stc_dp83867_tmch_ctrl_t f;
    uint16_t u16;
} cy_un_dp83867_tmch_ctrl_t;


/* FLD_CFG */
typedef struct
{
    uint16_t u5FLD_SRC_CFG  : 5; /* Fast Link Drop Source Configuration:
                                    The following FLD sources can be configured independently:
                                    Bit 4: Descrambler Loss Sync
                                    Bit 3: RX Errors
                                    Bit 2: MLT3 Errors
                                    Bit 1: SNR level
                                    Bit 0: Signal/Energy Lost */
    uint16_t u3RESERVED     : 3; /* RESERVED */
    uint16_t u5FLD_STS      : 5; /* Fast Link Drop Status:
                                    Status Registers that latch high each time a given Fast Link Down
                                    mode is activated and causes a link drop (assuming this criterion
                                    was enabled):
                                    Bit 12: Descrambler Loss Sync
                                    Bit 11: RX Errors
                                    Bit 10: MLT3 Errors
                                    Bit 9: SNR level
                                    Bit 8: Signal/Energy Lost */
    uint16_t u1RESERVED     : 1; /* RESERVED */
    uint16_t u1FLD_1G       : 1; /* 1000BASE-T Fast Link Drop:
                                    1 = Configure FLD for 1000BASE-T operation.
                                    0 = Normal operation. */
    uint16_t u1FLD_EN       : 1; /* Fast Link Drop Enable:
                                    1 = Enable FLD.
                                    0 = Normal operation. */
} cy_stc_dp83867_fld_cfg_t;


typedef union
{
    cy_stc_dp83867_fld_cfg_t f;
    uint16_t u16;
} cy_un_dp83867_fld_cfg_t;


/* CFG4 */
typedef struct
{
    uint16_t u1PORT_MIRROR_EN  : 1; /* Port Mirror Enable:
                                       1 = Enable port mirroring.
                                       0 = Normal operation */
    uint16_t u15RESERVED        : 15; /* RESERVED */
} cy_stc_dp83867_cfg4_t;


typedef union
{
    cy_stc_dp83867_cfg4_t f;
    uint16_t u16;
} cy_un_dp83867_cfg4_t;


/* RGMIICTL */
typedef struct
{
    uint16_t u1RGMII_RX_CLK_DELAY      : 1; /* RGMII Receive Clock Delay:
                                               1 = RGMII receive clock is shifted relative to receive data.
                                               0 = RGMII receive clock is aligned to receive data. */
    uint16_t u1RGMII_TX_CLK_DELAY      : 1; /* RGMII Transmit Clock Delay:
                                               1 = RGMII transmit clock is shifted relative to transmit data.
                                               0 = RGMII transmit clock is aligned to transmit data. */
    uint16_t u1RESERVED1               : 1; /* RESERVED: Writes ignored, read as 0. */
    uint16_t u2RGMII_TX_HALF_FULL_THR  : 2; /* RGMII Transmit FIFO Half Full Threshold:
                                               This field controls the RGMII transmit FIFO half full threshold. */
    uint16_t u2RGMII_RX_HALF_FULL_THR  : 2; /* RGMII Receive FIFO Half Full Threshold:
                                               This field controls the RGMII receive FIFO half full threshold. */
    uint16_t u1RGMII_EN                : 1; /* RGMII Enable:
                                               1 = Enable RGMII interface.
                                               0 = Disable RGMII interface. */
    uint16_t u8RESERVED0               : 8; /* RESERVED: Writes ignored, read as 0. */
} cy_stc_dp83867_rgmiictl_t;


typedef union
{
    cy_stc_dp83867_rgmiictl_t f;
    uint16_t u16;
} cy_un_dp83867_rgmiictl_t;


/* RGMIICTL2 */
typedef struct
{
    uint16_t u4RESERVED            : 4; /* RESERVED */
    uint16_t u1RGMII_AF_BYPASS_EN  : 1; /* RGMII Async FIFO Bypass Enable:
                                           1 = Enable RGMII Async FIFO Bypass.
                                           0 = Normal operation. */
    uint16_t u11RESERVED            : 11; /* RESERVED */
} cy_stc_dp83867_rgmiictl2_t;


typedef union
{
    cy_stc_dp83867_rgmiictl2_t f;
    uint16_t u16;
} cy_un_dp83867_rgmiictl2_t;


/* 100CR */
typedef struct
{
    uint16_t u1FAST_RX_DV           : 1; /* Fast RX_DV Enable:
                                            1 = Enable fast RX_DV.
                                            0 = Normal operation. */
    uint16_t u1ODD_NIBBLE_DETECT    : 1; /* Enable Odd Nibble Detection:
                                            1 = Detect when an odd number of nibbles is received.
                                            0 = Normal operation. */
    uint16_t u1SCR_DIS              : 1; /* Disable Scrambler:
                                            1 = Disable scrambler.
                                            0 = Normal operation. */
    uint16_t u1BYPASS_4B5B_RX       : 1; /* Bypass 4B/5B Receive Decoder:
                                            1 = Bypass 4B/5B decoder in receive path.
                                            0 = Normal operation. */
    uint16_t u1ENH_IPG_DET_EN       : 1; /* Enhanced Interpacket Gap Detection Enable:
                                            1 = Enable enhanced interpacket gap detection.
                                            0 = Normal operation. */
    uint16_t u1ENH_MLT3_DET_EN      : 1; /* Enhanced MLT-3 Detection Enable:
                                            1 = Enable enhanced MLT-3 Detection.
                                            0 = Normal operation. */
    uint16_t u1FORCE_100_OK         : 1; /* Force 100-Mbps Good Link:
                                            1 = Forces 100-Mbps good link.
                                            0 = Normal operation. */
    uint16_t u4DESCRAM_TIMEOUT      : 4; /* Descrambler Timeout:
                                            Adjust the descrambler time out value. This value refers to the
                                            recovery time due to descrambler unlock. Timer is in ms units. */
    uint16_t u1DESCRAM_TIMEOUT_DIS  : 1; /* Disable 100Base-TX Descrambler Timeout:
                                            1 = Disable packet reception when received packet violates the
                                            descrambler timeout. This occurs when the packet is longer than
                                            1.5 ms.
                                            0 = Stops packet reception when received packet violates the
                                            descrambler timeout. This occurs when the packet is longer than
                                            1.5 ms. */
    uint16_t u4RESERVED             : 4; /* RESERVED */
} cy_stc_dp83867_100cr_t;


typedef union
{
    cy_stc_dp83867_100cr_t f;
    uint16_t u16;
} cy_un_dp83867_100cr_t;


/* SKEW_FIFO */
typedef struct
{
    uint16_t u4CH_A_SKEW  : 4; /* Skew of RX channel A to align symbols in # of clock cycles. */
    uint16_t u4CH_B_SKEW  : 4; /* Skew of RX channel B to align symbols in # of clock cycles. */
    uint16_t u8RESERVED   : 8; /* RESERVED */
} cy_stc_dp83867_skew_fifo_t;


typedef union
{
    cy_stc_dp83867_skew_fifo_t f;
    uint16_t u16;
} cy_un_dp83867_skew_fifo_t;


/* STRAP_STS1_PAP */
typedef struct
{
    uint16_t u5STRAP_PHY_ADD_PAP            : 5; /* PHY Address Strap for PAP:
                                                    PHY address value from straps. */
    uint16_t u2STRAP_ANEG_SEL_PAP           : 2; /* Speed Select Strap:
                                                    SPEED_SEL[1:0] values from straps for PAP devices
                                                    See Speed Select Strap Details table. */
    uint16_t u1STRAP_ANEG_DIS               : 1; /* Auto-Negotiation Disable Strap:
                                                    1 = Auto-Negotiation strapped to disable.
                                                    0 = Auto-Negotiation strapped to enable. */
    uint16_t u1STRAP_HD_EN                  : 1; /* Half Duplex Enable Strap:
                                                    1 = Half Duplex strapped to enable.
                                                    0 = Full Duplex strapped to enable. */
    uint16_t u1STRAP_FORCE_MDI_X            : 1; /* Force MDI/X Strap:
                                                    1 = Force MDIX strapped to enable.
                                                    0 = Force MDI strapped to enable. */
    uint16_t u1STRAP_AMDIX_DIS              : 1; /* Auto-MDIX Disable Strap:
                                                    1 = Auto-MDIX strapped to disable.
                                                    0 = Auto-MDIX strapped to enable. */
    uint16_t u1RESERVED                     : 1; /* RESERVED */
    uint16_t u1STRAP_RGMII_DIS              : 1; /* RGMII Disable Strap:
                                                    1 = RGMII strapped to disable.
                                                    0 = RGMII strapped to enable. */
    uint16_t u1STRAP_CLK_OUT_DIS_PAP_ONLY   : 1; /* Clock Output Disable Strap:
                                                    1 = Clock output strapped to disable.
                                                    0 = Clock output strapped to enable. */
    uint16_t u1STRAP_LINK_DOWNSHIFT_EN      : 1; /* Link Downshift Enable Strap:
                                                    1 = Link Downshift strapped to enable.
                                                    0 = Link Downshift strapped to disable. */
    uint16_t u1STRAP_MIRROR_EN              : 1; /* Mirror Enable Strap:
                                                    1 = Port mirroring strapped to enable.
                                                    0 = Port mirroring strapped to disable. */
} cy_stc_dp83867_strap_sts1_pap_t;


typedef union
{
    cy_stc_dp83867_strap_sts1_pap_t f;
    uint16_t u16;
} cy_un_dp83867_strap_sts1_pap_t;


/* STRAP_STS1_RGZ */
typedef struct
{
    uint16_t u4STRAP_PHY_ADD                : 4; /* PHY Address Strap for RGZ:
                                                    PHY address value from straps. */
    uint16_t u1RESERVED_2                   : 1; /* RESERVED */
    uint16_t u1STRAP_SPEED_SEL              : 1; /* SPEED_SEL value from strap for RGZ devices
                                                    See Speed Select Strap Details table. */
    uint16_t u1RESERVED_1                   : 1; /* RESERVED */
    uint16_t u1STRAP_ANEG_DIS               : 1; /* Auto-Negotiation Disable Strap:
                                                    1 = Auto-Negotiation strapped to disable.
                                                    0 = Auto-Negotiation strapped to enable. */
    uint16_t u1STRAP_HD_EN                  : 1; /* Half Duplex Enable Strap:
                                                    1 = Half Duplex strapped to enable.
                                                    0 = Full Duplex strapped to enable. */
    uint16_t u1STRAP_FORCE_MDI_X            : 1; /* Force MDI/X Strap:
                                                    1 = Force MDIX strapped to enable.
                                                    0 = Force MDI strapped to enable. */
    uint16_t u1STRAP_AMDIX_DIS              : 1; /* Auto-MDIX Disable Strap:
                                                    1 = Auto-MDIX strapped to disable.
                                                    0 = Auto-MDIX strapped to enable. */
    uint16_t u1RESERVED                     : 1; /* RESERVED */
    uint16_t u1STRAP_RGMII_DIS              : 1; /* RGMII Disable Strap:
                                                    1 = RGMII strapped to disable.
                                                    0 = RGMII strapped to enable. */
    uint16_t u1STRAP_CLK_OUT_DIS_PAP_ONLY   : 1; /* Clock Output Disable Strap:
                                                    1 = Clock output strapped to disable.
                                                    0 = Clock output strapped to enable. */
    uint16_t u1STRAP_LINK_DOWNSHIFT_EN      : 1; /* Link Downshift Enable Strap:
                                                    1 = Link Downshift strapped to enable.
                                                    0 = Link Downshift strapped to disable. */
    uint16_t u1STRAP_MIRROR_EN              : 1; /* Mirror Enable Strap:
                                                    1 = Port mirroring strapped to enable.
                                                    0 = Port mirroring strapped to disable. */
} cy_stc_dp83867_strap_sts1_rgz_t;


typedef union
{
    cy_stc_dp83867_strap_sts1_rgz_t f;
    uint16_t u16;
} cy_un_dp83867_strap_sts1_rgz_t;


/* STRAP_STS2_PAP */
typedef struct
{
    uint16_t u10RESERVED    : 10; /* RESERVED */
    uint16_t u1STRAP_FLD    : 1; /* Fast Link Detect (FLD) Enable Strap:
                                   1 = FLD strapped to enable.
                                   0 = FLD strapped to disable. */
    uint16_t u5RESERVED    : 5; /* RESERVED */
} cy_stc_dp83867_strap_sts2_pap_t;


typedef union
{
    cy_stc_dp83867_strap_sts2_pap_t f;
    uint16_t u16;
} cy_un_dp83867_strap_sts2_pap_t;


/* STRAP_STS2_RGZ */
typedef struct
{
    uint16_t u3STRAP_RGMII_CLK_SKEW_RX  : 3; /* RGMII Receive Clock Skew Strap:
                                                RGMII_RX_DELAY_CTRL[2:0] values from straps.
                                                See RGMII Transmit Clock Skew Details table for more information. */
    uint16_t u1RESERVED                 : 1; /* RESERVED */
    uint16_t u3STRAP_RGMII_CLK_SKEW_TX  : 3; /* RGMII Transmit Clock Skew Strap:
                                                RGMII_TX_DELAY_CTRL[2:0] values from straps.
                                                See RGMII Transmit Clock Skew Details table for more information. */
    uint16_t u3RESERVED                 : 3; /* RESERVED */
    uint16_t u1STRAP_FLD                : 1; /* Fast Link Detect (FLD) Enable Strap:
                                                1 = FLD strapped to enable.
                                                0 = FLD strapped to disable. */
    uint16_t u5RESERVED                 : 5; /* RESERVED */
} cy_stc_dp83867_strap_sts2_rgz_t;


typedef union
{
    cy_stc_dp83867_strap_sts2_rgz_t f;
    uint16_t u16;
} cy_un_dp83867_strap_sts2_rgz_t;


/* BICSR1 */
typedef struct
{
    uint16_t u16PRBS_BYTE_CNT  : 16; /* Holds the number of total bytes received by the PRBS checker.
                                        Value in this register is locked when write is done to register
                                        BICSR2 bit[0] or bit[1].
                                        The count stops at 0xFFFF when PRBS_COUNT_MODE in BISCR
                                        register (0x0016) is set to 0. */
} cy_stc_dp83867_bicsr1_t;


typedef union
{
    cy_stc_dp83867_bicsr1_t f;
    uint16_t u16;
} cy_un_dp83867_bicsr1_t;


/* BICSR2 */
typedef struct
{
    uint16_t u8PRBS_ERR_CNT       : 8; /* Holds number of error bytes that are received by PRBS checker.
                                          Value in this register is locked when write is done to bit[0] or bit[1]
                                          When PRBS Count Mode set to zero, count stops on 0xFF (see
                                          register 0x0016)
                                          Notes: Writing bit 0 generates a lock signal for the PRBS counters.
                                          Writing bit 1 generates a lock and clear signal for the PRBS
                                          counters */
    uint16_t u1Reserved           : 1; /* Ignore on Read */
    uint16_t u1PRBS_BYTE_CNT_OVF  : 1; /* PRBS Byte Count Overflow
                                          If set, PRBS Byte counter has reached overflow. Overflow is cleared
                                          when PRBS counters are cleared by setting bit #1 of this register. */
    uint16_t u1PRBS_PKT_CNT_OVF   : 1; /* PRBS Checker Packet Count Overflow
                                          If set, PRBS Packet counter has reached overflow. Overflow is
                                          cleared when PRBS counters are cleared by setting bit #1 of this
                                          register. */
    uint16_t u5Reserved           : 5; /* Ignored on Read */
} cy_stc_dp83867_bicsr2_t;


typedef union
{
    cy_stc_dp83867_bicsr2_t f;
    uint16_t u16;
} cy_un_dp83867_bicsr2_t;


/* RGMIIDCTL */
typedef struct
{
    uint16_t u4RGMII_RX_DELAY_CTRL  : 4; /* RGMII Receive Clock Delay:
                                            1111: 4.00 ns
                                            1110: 3.75 ns
                                            1101: 3.50 ns
                                            1100: 3.25 ns
                                            1011: 3.00 ns
                                            1010: 2.75 ns
                                            1001: 2.50 ns
                                            1000: 2.25 ns
                                            0111: 2.00 ns
                                            0110: 1.75 ns
                                            0101: 1.50 ns
                                            0100: 1.25 ns
                                            0011: 1.00 ns
                                            0010: 0.75 ns
                                            0001: 0.50 ns
                                            0000: 0.25 ns */
    uint16_t u4RGMII_TX_DELAY_CTRL  : 4; /* RGMII Transmit Clock Delay:
                                            1111: 4.00 ns
                                            1110: 3.75 ns
                                            1101: 3.50 ns
                                            1100: 3.25 ns
                                            1011: 3.00 ns
                                            1010: 2.75 ns
                                            1001: 2.50 ns
                                            1000: 2.25 ns
                                            0111: 2.00 ns
                                            0110: 1.75 ns
                                            0101: 1.50 ns
                                            0100: 1.25 ns
                                            0011: 1.00 ns
                                            0010: 0.75 ns
                                            0001: 0.50 ns
                                            0000: 0.25 ns */
    uint16_t u8RESERVED             : 8; /* RESERVED: Writes ignored, read as 0. */
} cy_stc_dp83867_rgmiidctl_t;


typedef union
{
    cy_stc_dp83867_rgmiidctl_t f;
    uint16_t u16;
} cy_un_dp83867_rgmiidctl_t;


/* SYNC_FIFO_CTRL */
typedef struct
{
    uint16_t u16RESERVED  : 16; /* RESERVED */
} cy_stc_dp83867_sync_fifo_ctrl_t;


typedef union
{
    cy_stc_dp83867_sync_fifo_ctrl_t f;
    uint16_t u16;
} cy_un_dp83867_sync_fifo_ctrl_t;


/* LOOPCR */
typedef struct
{
    uint16_t u16LOOP_CFG_VAL  : 16; /* Loopback Configuration Value:
                                       1110 0111 0010 000: Configuration for loopback modes.
                                       A software reset through bit 14 of the Control Register (CTRL),
                                       address 0x001F, is required after changes to this register value.
                                       Other values for this register are not recommended. */
} cy_stc_dp83867_loopcr_t;


typedef union
{
    cy_stc_dp83867_loopcr_t f;
    uint16_t u16;
} cy_un_dp83867_loopcr_t;


/* RXFCFG */
typedef struct
{
    uint16_t u1WAKE_ON_MAGIC         : 1; /* Wake on Magic Packet:
                                             1 = Issue an interrupt upon reception of Magic packet.
                                             0 = Do not issue an interrupt upon reception of Magic packet. */
    uint16_t u1WAKE_ON_PATTERN       : 1; /* Wake on Pattern Match:
                                             1 = Issue an interrupt upon pattern match.
                                             0 = Do not issue an interrupt upon pattern match. */
    uint16_t u1WAKE_ON_BCAST         : 1; /* Wake on Broadcast Packet:
                                             1 = Issue an interrupt upon reception of Broadcast packet.
                                             0 = Do not issue an interrupt upon reception of Broadcast packet. */
    uint16_t u1RESERVED_2            : 1; /* RESERVED */
    uint16_t u1WAKE_ON_UCAST         : 1; /* Wake on Unicast Packet:
                                             1 = Issue an interrupt upon reception of Unicast packet.
                                             0 = Do not issue an interrupt upon reception of Unicast packet. */
    uint16_t u1SCRON_EN              : 1; /* Enable SecureOn Password:
                                             1 = SecureOn Password enabled.
                                             0 = SecureOn Password disabled. */
    uint16_t u1RESERVED_1            : 1; /* RESERVED */
    uint16_t u1ENHANCED_MAC_SUPPORT  : 1; /* Enable Enhanced Receive Features:
                                             1 = Enable for Wake-on-LAN, CRC check, and Receive 1588
                                             indication.
                                             0 = Normal operation. */
    uint16_t u1WOL_OUT_MODE          : 1; /* Wake-on-LAN Output Mode:
                                             1 = Level Mode. WoL is cleared by a write to WOL_OUT_CLEAR (bit 11).
                                             0 = Pulse Mode. Pulse width is configured via WOL_OUT_STRETCH (bits 10:9). */
    uint16_t u2WOL_OUT_STRETCH       : 2; /* Wake-on-LAN Output Stretch:
                                             If WoL out is configured for pulse mode, the pulse length is defined
                                             as the following number of 125-MHz clock cycles:
                                             11 = 64 clock cycles
                                             10 = 32 clock cycles
                                             01 = 16 clock cycles
                                             00 = 8 clock cycles */
    uint16_t u1WOL_OUT_CLEAR         : 1; /* Clear Wake-on-LAN Output:
                                             This bit is only applicable when configured for level mode.
                                             1 = Clear Wake-on-LAN output */
    uint16_t u4RESERVED              : 4; /* RESERVED */
} cy_stc_dp83867_rxfcfg_t;


typedef union
{
    cy_stc_dp83867_rxfcfg_t f;
    uint16_t u16;
} cy_un_dp83867_rxfcfg_t;


/* RXFSTS */
typedef struct
{
    uint16_t u1MAGIC_RCVD    : 1; /* Magic Packet Received:
                                     1 = A valid Magic packet was received.
                                     0 = No valid Magic packet was received. */
    uint16_t u1PATTERN_RCVD  : 1; /* Pattern Match Received:
                                     1 = A valid packet with the configured pattern was received.
                                     0 = No valid packet with the configured pattern was received. */
    uint16_t u1BCAST_RCVD    : 1; /* Broadcast Packet Received:
                                     1 = A valid Broadcast packet was received.
                                     0 = No valid Broadcast packet was received. */
    uint16_t u1RESERVED      : 1; /* RESERVED */
    uint16_t u1UCAST_RCVD    : 1; /* Unicast Packet Received:
                                     1 = A valid Unicast packet was received.
                                     0 = No valid Unicast packet was received. */
    uint16_t u1SCRON_HACK    : 1; /* SecureOn Hack Attempt Flag:
                                     1 = SecureOn Hack attempt seen.
                                     0 = No SecureOn Hack attempt seen. */
    uint16_t u1BAD_CRC       : 1; /* Bad CRC:
                                     1 = A packet with a bad CRC was received.
                                     0 = No bad CRC seen. */
    uint16_t u1SFD_ERR       : 1; /* SFD Error:
                                     1 = Packet with SFD error (without an 0x5D SFD byte) received.
                                     0 = No SFD error seen. */
    uint16_t u8RESERVED      : 8; /* RESERVED */
} cy_stc_dp83867_rxfsts_t;


typedef union
{
    cy_stc_dp83867_rxfsts_t f;
    uint16_t u16;
} cy_un_dp83867_rxfsts_t;


/* RXFPMD1 */
typedef struct
{
    uint16_t u16PMATCH_DATA_15_0  : 16; /* Bits 15:0 of Perfect Match Data - used for DA (destination address)
                                           match */
} cy_stc_dp83867_rxfpmd1_t;


typedef union
{
    cy_stc_dp83867_rxfpmd1_t f;
    uint16_t u16;
} cy_un_dp83867_rxfpmd1_t;


/* RXFPMD2 */
typedef struct
{
    uint16_t u16PMATCH_DATA_31_16  : 16; /* Bits 31:16 of Perfect Match Data - used for DA (destination address)
                                            match */
} cy_stc_dp83867_rxfpmd2_t;


typedef union
{
    cy_stc_dp83867_rxfpmd2_t f;
    uint16_t u16;
} cy_un_dp83867_rxfpmd2_t;


/* RXFPMD3 */
typedef struct
{
    uint16_t u16PMATCH_DATA_47_32   : 16; /* Bits 47:32 of Perfect Match Data - used for DA (destination address)
                                             match */
} cy_stc_dp83867_rxfpmd3_t;


typedef union
{
    cy_stc_dp83867_rxfpmd3_t f;
    uint16_t u16;
} cy_un_dp83867_rxfpmd3_t;


/* RXFSOP1 */
typedef struct
{
    uint16_t u16SCRON_PASSWORD_15_0  : 16; /* Bits 15:0 of secure-on password for magic packet */
} cy_stc_dp83867_rxfsop1_t;


typedef union
{
    cy_stc_dp83867_rxfsop1_t f;
    uint16_t u16;
} cy_un_dp83867_rxfsop1_t;


/* RXFSOP2 */
typedef struct
{
    uint16_t u16SCRON_PASSWORD_31_16  : 16; /* Bits 31:16 of secure-on password for magic packet */
} cy_stc_dp83867_rxfsop2_t;


typedef union
{
    cy_stc_dp83867_rxfsop2_t f;
    uint16_t u16;
} cy_un_dp83867_rxfsop2_t;


/* RXFSOP3 */
typedef struct
{
    uint16_t u16SCRON_PASSWORD_47_32  : 16; /* Bits 47:32 of secure-on password for magic packet */
} cy_stc_dp83867_rxfsop3_t;


typedef union
{
    cy_stc_dp83867_rxfsop3_t f;
    uint16_t u16;
} cy_un_dp83867_rxfsop3_t;


/* RXFPAT */
typedef struct
{
    uint16_t u16PATTERN_BYTES  : 16; /* Bytes (2xn-2) (Lsb) + (2xn-1) of the configured pattern. Each byte can be
                                        masked separately through the RXF_PATTERN_BYTE_MASK
                                        registers. */
} cy_stc_dp83867_rxfpat_t;


typedef union
{
    cy_stc_dp83867_rxfpat_t f;
    uint16_t u16;
} cy_un_dp83867_rxfpat_t;


/* RXFPBM1 */
typedef struct
{
    uint16_t u16PATTERN_BYTES_MASK_0_15  : 16; /* Masks for bytes 0 to 15 of the pattern. A 1 indicates a mask for the
                                                  associated byte. */
} cy_stc_dp83867_rxfpbm1_t;


typedef union
{
    cy_stc_dp83867_rxfpbm1_t f;
    uint16_t u16;
} cy_un_dp83867_rxfpbm1_t;


/* RXFPBM2 */
typedef struct
{
    uint16_t u16PATTERN_BYTES_MASK_16_31  : 16; /* Masks for bytes 16 to 31 of the pattern. A 1 indicates a mask for the
                                                   associated byte. */
} cy_stc_dp83867_rxfpbm2_t;


typedef union
{
    cy_stc_dp83867_rxfpbm2_t f;
    uint16_t u16;
} cy_un_dp83867_rxfpbm2_t;


/* RXFPBM3 */
typedef struct
{
    uint16_t u16PATTERN_BYTES_MASK_32_47  : 16; /* Masks for bytes 32 to 47 of the pattern. A 1 indicates a mask for the
                                                   associated byte. */
} cy_stc_dp83867_rxfpbm3_t;


typedef union
{
    cy_stc_dp83867_rxfpbm3_t f;
    uint16_t u16;
} cy_un_dp83867_rxfpbm3_t;


/* RXFPBM4 */
typedef struct
{
    uint16_t u16PATTERN_BYTES_MASK_48_63  : 16; /* Masks for bytes 48 to 63 of the pattern. A 1 indicates a mask for the
                                                   associated byte. */
} cy_stc_dp83867_rxfpbm4_t;


typedef union
{
    cy_stc_dp83867_rxfpbm4_t f;
    uint16_t u16;
} cy_un_dp83867_rxfpbm4_t;


/* RXFPATC */
typedef struct
{
    uint16_t u6PATTERN_START_POINT  : 6; /* Number of bytes after SFD where comparison of the RX packet to
                                            the configured pattern begins:
                                            111111 - Start compare in the 64th byte after SFD
                                            000000 - Start compare in the 1st byte after SFD */
    uint16_t u10RESERVED             : 10; /* RESERVED: Writes ignored, read as 0. */
} cy_stc_dp83867_rxfpatc_t;


typedef union
{
    cy_stc_dp83867_rxfpatc_t f;
    uint16_t u16;
} cy_un_dp83867_rxfpatc_t;


/* IO_MUX_CFG */
typedef struct
{
    uint16_t u5IO_IMPEDANCE_CTRL  : 5; /* Impedance Control for MAC I/Os:
                                          Output impedance approximate range from 35-70 [ohm] in 32 steps.
                                          Lowest being 11111 and highest being 00000. Range and Step size
                                          will vary with process.
                                          Default is set to 50 Ω by trim. But the default register value can vary
                                          by process. Non default values of MAC I/O impedance can be used
                                          based on trace impedance. Mismatch between device and trace
                                          impedance can cause voltage overshoot and undershoot. */
    uint16_t u1RESERVED_2         : 1; /* RESERVED */
    uint16_t u1CLK_O_DISABLE      : 1; /* Clock Output Disable:
                                          1 = Disable clock output on CLK_OUT pin.
                                          0 = Enable clock output on CLK_OUT pin. */
    uint16_t u1RESERVED_1         : 1; /* RESERVED */
    uint16_t u5CLK_O_SEL          : 5; /* Clock Output Select:
                                          01101 - 11111: RESERVED
                                          01100: Reference clock (synchronous to XI input clock)
                                          01011: Channel D transmit clock
                                          01010: Channel C transmit clock
                                          01001: Channel B transmit clock
                                          01000: Channel A transmit clock
                                          00111: Channel D receive clock divided by 5
                                          00110: Channel C receive clock divided by 5
                                          00101: Channel B receive clock divided by 5
                                          00100: Channel A receive clock divided by 5
                                          00011: Channel D receive clock
                                          00010: Channel C receive clock
                                          00001: Channel B receive clock
                                          00000: Channel A receive clock */
    uint16_t u3RESERVED           : 3; /* RESERVED */
} cy_stc_dp83867_io_mux_cfg_t;


typedef union
{
    cy_stc_dp83867_io_mux_cfg_t f;
    uint16_t u16;
} cy_un_dp83867_io_mux_cfg_t;


/* _GPIO_MUX_CTRL1 */
typedef struct
{
    uint16_t u4RX_D4_GPIO_CTRL  : 4; /* RX_D4 GPIO Control:
                                        1010 - 1111: RESERVED
                                        1001: Constant '1'
                                        1000: Constant '0'
                                        0111: PRBS Errors / Loss of Sync
                                        0110: LED_3
                                        0101: RESERVED
                                        0100: Energy Detect (1000Base-T and 100Base-TX only)
                                        0011: WOL
                                        0010: 1588 RX SFD
                                        0001: 1588 TX SFD
                                        0000: RX_D4 */
    uint16_t u4RX_D5_GPIO_CTRL  : 4; /* RX_D5 GPIO Control:
                                        1010 - 1111: RESERVED
                                        1001: Constant '1'
                                        1000: Constant '0'
                                        0111: PRBS Errors / Loss of Sync
                                        0110: LED_3
                                        0101: RESERVED
                                        0100: Energy Detect (1000Base-T and 100Base-TX only)
                                        0011: WOL
                                        0010: 1588 RX SFD
                                        0001: 1588 TX SFD
                                        0000: RX_D5 */
    uint16_t u4RX_D6_GPIO_CTRL  : 4; /* RX_D6 GPIO Control:
                                        1010 - 1111: RESERVED
                                        1001: Constant '1'
                                        1000: Constant '0'
                                        0111: PRBS Errors / Loss of Sync
                                        0110: LED_3
                                        0101: RESERVED
                                        0100: Energy Detect (1000Base-T and 100Base-TX only)
                                        0011: WOL
                                        0010: 1588 RX SFD
                                        0001: 1588 TX SFD
                                        0000: RX_D6 */
    uint16_t u4RX_D7_GPIO_CTRL  : 4; /* RX_D7 GPIO Control:
                                        1010 - 1111: RESERVED
                                        1001: Constant '1'
                                        1000: Constant '0'
                                        0111: PRBS Errors / Loss of Sync
                                        0110: LED_3
                                        0101: RESERVED
                                        0100: Energy Detect (1000Base-T and 100Base-TX only)
                                        0011: WOL
                                        0010: 1588 RX SFD
                                        0001: 1588 TX SFD
                                        0000: RX_D7 */
} cy_stc_dp83867__gpio_mux_ctrl1_t;


typedef union
{
    cy_stc_dp83867__gpio_mux_ctrl1_t f;
    uint16_t u16;
} cy_un_dp83867__gpio_mux_ctrl1_t;


/* GPIO_MUX_CTRL2 */
typedef struct
{
    uint16_t u4RX_ER_GPIO_CTRL  : 4; /* RX_ER GPIO Control:
                                        1010 - 1111: RESERVED
                                        1001: Constant '1'
                                        1000: Constant '0'
                                        0111: PRBS Errors / Loss of Sync
                                        0110: LED_3
                                        0101: RESERVED
                                        0100: Energy Detect (1000Base-T and 100Base-TX only)
                                        0011: WOL
                                        0010: 1588 RX SFD
                                        0001: 1588 TX SFD
                                        0000: RX_ER */
    uint16_t u4COL_GPIO_CTRL    : 4; /* COL GPIO Control:
                                        1010 - 1111: RESERVED
                                        1001: Constant '1'
                                        1000: Constant '0'
                                        0111: PRBS Errors / Loss of Sync
                                        0110: LED_3
                                        0101: RESERVED
                                        0100: Energy Detect (1000Base-T and 100Base-TX only)
                                        0011: WOL
                                        0010: 1588 RX SFD
                                        0001: 1588 TX SFD
                                        0000: COL */
    uint16_t u4CRS_GPIO_CTRL    : 4; /* CRS GPIO Control:
                                        1010 - 1111: RESERVED
                                        1001: Constant '1'
                                        1000: Constant '0'
                                        0111: PRBS Errors / Loss of Sync
                                        0110: LED_3
                                        0101: RESERVED
                                        0100: Energy Detect (1000Base-T and 100Base-TX only)
                                        0011: WOL
                                        0010: 1588 RX SFD
                                        0001: 1588 TX SFD
                                        0000: CRS */
    uint16_t u4RESERVED         : 4; /* RESERVED */
} cy_stc_dp83867_gpio_mux_ctrl2_t;


typedef union
{
    cy_stc_dp83867_gpio_mux_ctrl2_t f;
    uint16_t u16;
} cy_un_dp83867_gpio_mux_ctrl2_t;


/* GPIO_MUX_CTRL */
typedef struct
{
    uint16_t u4GPIO_0_CTRL  : 4; /* GPIO_0 Control:
                                    1010 - 1111: RESERVED
                                    1001: Constant 1
                                    1000: Constant 0
                                    0111: PRBS Errors / Loss of Sync
                                    0110: LED_3
                                    0101: RESERVED
                                    0100: Energy Detect (1000Base-T and 100Base-TX only)
                                    0011: WOL
                                    0010: 1588 RX SFD
                                    0001: 1588 TX SFD
                                    0000: RX_ER */
    uint16_t u4GPIO_1_CTRL  : 4; /* GPIO_1 Control:
                                    1010 - 1111: RESERVED
                                    1001: Constant 1
                                    1000: Constant 0
                                    0111: PRBS Errors / Loss of Sync
                                    0110: LED_3
                                    0101: RESERVED
                                    0100: Energy Detect (1000Base-T and 100Base-TX only)
                                    0011: WOL
                                    0010: 1588 RX SFD
                                    0001: 1588 TX SFD
                                    0000: COL */
    uint16_t u8RESERVED     : 8; /* RESERVED */
} cy_stc_dp83867_gpio_mux_ctrl_t;


typedef union
{
    cy_stc_dp83867_gpio_mux_ctrl_t f;
    uint16_t u16;
} cy_un_dp83867_gpio_mux_ctrl_t;


/* TDR_GEN_CFG1 */
typedef struct
{
    uint16_t u4TDR_CYCLE_TIME      : 4; /* Set the time for each TDR cycle. Value is measured in
                                           microseconds. */
    uint16_t u3TDR_SEG_NUM         : 3; /* Set the number of TDR segments to check. */
    uint16_t u3TDR_AVG_NUM         : 3; /* Number Of TDR Cycles to Average:
                                           111: RESERVED: Writes ignored, read as 0.
                                           110: 64 TDR cycles
                                           101: 32 TDR cycles
                                           100: 16 TDR cycles
                                           011: 8 TDR cycles
                                           010: 4 TDR cycles
                                           001: 2 TDR cycles
                                           000: 1 TDR cycle */
    uint16_t u1TDR_NLP_CHECK       : 1; /* TDR NLP Check:
                                           1 = Check for NLPs during silence.
                                           0 = Normal operation. */
    uint16_t u1TDR_CROSS_MODE_DIS  : 1; /* Disable TDR Cross Mode:
                                           1 = Disable cross mode option. Do not check cross channels. Only
                                           listen to the channel being used for transmit.
                                           0 = Normal operation. */
    uint16_t u1TDR_CH_CD_BYPASS    : 1; /* TDR Bypass for Channel C and D:
                                           1 = Bypass channel C and D in TDR tests.
                                           0 = Normal operation. */
    uint16_t u3RESERVED            : 3; /* RESERVED */
} cy_stc_dp83867_tdr_gen_cfg1_t;


typedef union
{
    cy_stc_dp83867_tdr_gen_cfg1_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_gen_cfg1_t;


/* TDR_PEAKS_LOC_1 */
typedef struct
{
    uint16_t u8TDR_PEAKS_LOC_A_0  : 8; /* Location of the first peak discovered by the TDR mechanism on
                                          channel A. The value of these bits is translated into distance from
                                          the PHY. */
    uint16_t u8TDR_PEAKS_LOC_A_1  : 8; /* Location of the second peak discovered by the TDR mechanism on
                                          channel A. The value of these bits is translated into distance from
                                          the PHY. */
} cy_stc_dp83867_tdr_peaks_loc_1_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_loc_1_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_loc_1_t;


/* TDR_PEAKS_LOC_2 */
typedef struct
{
    uint16_t u8TDR_PEAKS_LOC_A_2  : 8; /* Location of the third peak discovered by the TDR mechanism on
                                          channel A. The value of these bits is translated into distance from
                                          the PHY. */
    uint16_t u8TDR_PEAKS_LOC_A_3  : 8; /* Location of the fourth peak discovered by the TDR mechanism on
                                          channel A. The value of these bits is translated into distance from
                                          the PHY. */
} cy_stc_dp83867_tdr_peaks_loc_2_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_loc_2_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_loc_2_t;


/* TDR_PEAKS_LOC_3 */
typedef struct
{
    uint16_t u8TDR_PEAKS_LOC_A_4  : 8; /* Location of the fifth peak discovered by the TDR mechanism on
                                          channel A. The value of these bits is translated into distance from
                                          the PHY. */
    uint16_t u8TDR_PEAKS_LOC_B_0  : 8; /* Location of the first peak discovered by the TDR mechanism on
                                          channel B. The value of these bits is translated into distance from
                                          the PHY. */
} cy_stc_dp83867_tdr_peaks_loc_3_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_loc_3_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_loc_3_t;


/* TDR_PEAKS_LOC_4 */
typedef struct
{
    uint16_t u8TDR_PEAKS_LOC_B_1  : 8; /* Location of the second peak discovered by the TDR mechanism on
                                          channel B. The value of these bits is translated into distance from
                                          the PHY. */
    uint16_t u8TDR_PEAKS_LOC_B_2  : 8; /* Location of the third peak discovered by the TDR mechanism on
                                          channel B. The value of these bits is translated into distance from
                                          the PHY. */
} cy_stc_dp83867_tdr_peaks_loc_4_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_loc_4_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_loc_4_t;


/* TDR_PEAKS_LOC_5 */
typedef struct
{
    uint16_t u8TDR_PEAKS_LOC_B_3  : 8; /* Location of the fourth peak discovered by the TDR mechanism on
                                          channel B. The value of these bits is translated into distance from
                                          the PHY. */
    uint16_t u8TDR_PEAKS_LOC_B_4  : 8; /* Location of the fifth peak discovered by the TDR mechanism on
                                          channel B. The value of these bits is translated into distance from
                                          the PHY. */
} cy_stc_dp83867_tdr_peaks_loc_5_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_loc_5_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_loc_5_t;


/* TDR_PEAKS_LOC_6 */
typedef struct
{
    uint16_t u8TDR_PEAKS_LOC_C_0  : 8; /* Location of the first peak discovered by the TDR mechanism on
                                          channel C. The value of these bits is translated into distance from
                                          the PHY. */
    uint16_t u8TDR_PEAKS_LOC_C_1  : 8; /* Location of the second peak discovered by the TDR mechanism on
                                          channel C. The value of these bits is translated into distance from
                                          the PHY. */
} cy_stc_dp83867_tdr_peaks_loc_6_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_loc_6_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_loc_6_t;


/* TDR_PEAKS_LOC_7 */
typedef struct
{
    uint16_t u8TDR_PEAKS_LOC_C_2  : 8; /* Location of the third peak discovered by the TDR mechanism on
                                          channel C. The value of these bits is translated into distance from
                                          the PHY. */
    uint16_t u8TDR_PEAKS_LOC_C_3  : 8; /* Location of the fourth peak discovered by the TDR mechanism on
                                          channel C. The value of these bits is translated into distance from
                                          the PHY. */
} cy_stc_dp83867_tdr_peaks_loc_7_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_loc_7_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_loc_7_t;


/* TDR_PEAKS_LOC_8 */
typedef struct
{
    uint16_t u8TDR_PEAKS_LOC_C_4  : 8; /* Location of the fifth peak discovered by the TDR mechanism on
                                          channel C. The value of these bits is translated into distance from
                                          the PHY. */
    uint16_t u8TDR_PEAKS_LOC_D_0  : 8; /* Location of the first peak discovered by the TDR mechanism on
                                          channel D. The value of these bits is translated into distance from
                                          the PHY. */
} cy_stc_dp83867_tdr_peaks_loc_8_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_loc_8_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_loc_8_t;


/* TDR_PEAKS_LOC_9 */
typedef struct
{
    uint16_t u8TDR_PEAKS_LOC_D_1  : 8; /* Location of the second peak discovered by the TDR mechanism on
                                          channel D. The value of these bits is translated into distance from
                                          the PHY. */
    uint16_t u8TDR_PEAKS_LOC_D_2  : 8; /* Location of the third peak discovered by the TDR mechanism on
                                          channel D. The value of these bits is translated into distance from
                                          the PHY. */
} cy_stc_dp83867_tdr_peaks_loc_9_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_loc_9_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_loc_9_t;


/* TDR_PEAKS_LOC_10 */
typedef struct
{
    uint16_t u8TDR_PEAKS_LOC_D_3  : 8; /* Location of the fourth peak discovered by the TDR mechanism on
                                          channel D. The value of these bits is translated into distance from
                                          the PHY. */
    uint16_t u8TDR_PEAKS_LOC_D_4  : 8; /* Location of the fifth peak discovered by the TDR mechanism on
                                          channel D. The value of these bits is translated into distance from
                                          the PHY. */
} cy_stc_dp83867_tdr_peaks_loc_10_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_loc_10_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_loc_10_t;


/* TDR_PEAKS_AMP_1 */
typedef struct
{
    uint16_t u7TDR_PEAKS_AMP_A_0  : 7; /* Amplitude of the first peak discovered by the TDR mechanism on
                                          channel A. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED_1         : 1; /* RESERVED */
    uint16_t u7TDR_PEAKS_AMP_A_1  : 7; /* Amplitude of the second peak discovered by the TDR mechanism
                                          on channel A. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED           : 1; /* RESERVED */
} cy_stc_dp83867_tdr_peaks_amp_1_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_amp_1_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_amp_1_t;


/* TDR_PEAKS_AMP_2 */
typedef struct
{
    uint16_t u7TDR_PEAKS_AMP_A_2  : 7; /* Amplitude of the third peak discovered by the TDR mechanism on
                                          channel A. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED_1         : 1; /* RESERVED */
    uint16_t u7TDR_PEAKS_AMP_A_3  : 7; /* Amplitude of the fourth peak discovered by the TDR mechanism on
                                          channel A. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED           : 1; /* RESERVED */
} cy_stc_dp83867_tdr_peaks_amp_2_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_amp_2_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_amp_2_t;


/* TDR_PEAKS_AMP_3 */
typedef struct
{
    uint16_t u7TDR_PEAKS_AMP_A_4  : 7; /* Amplitude of the fifth peak discovered by the TDR mechanism on
                                          channel A. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED_1         : 1; /* RESERVED */
    uint16_t u7TDR_PEAKS_AMP_B_0  : 7; /* Amplitude of the first peak discovered by the TDR mechanism on
                                          channel B. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED           : 1; /* RESERVED */
} cy_stc_dp83867_tdr_peaks_amp_3_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_amp_3_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_amp_3_t;


/* TDR_PEAKS_AMP_4 */
typedef struct
{
    uint16_t u7TDR_PEAKS_AMP_B_1  : 7; /* Amplitude of the second peak discovered by the TDR mechanism
                                          on channel B. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED_1         : 1; /* RESERVED */
    uint16_t u7TDR_PEAKS_AMP_B_2  : 7; /* Amplitude of the third peak discovered by the TDR mechanism on
                                          channel B. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED           : 1; /* RESERVED */
} cy_stc_dp83867_tdr_peaks_amp_4_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_amp_4_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_amp_4_t;


/* TDR_PEAKS_AMP_5 */
typedef struct
{
    uint16_t u7TDR_PEAKS_AMP_B_3  : 7; /* Amplitude of the fourth peak discovered by the TDR mechanism on
                                          channel B. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED_1         : 1; /* RESERVED */
    uint16_t u7TDR_PEAKS_AMP_B_4  : 7; /* Amplitude of the fifth peak discovered by the TDR mechanism on
                                          channel B. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED           : 1; /* RESERVED */
} cy_stc_dp83867_tdr_peaks_amp_5_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_amp_5_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_amp_5_t;


/* TDR_PEAKS_AMP_6 */
typedef struct
{
    uint16_t u7TDR_PEAKS_AMP_C_0  : 7; /* Amplitude of the first peak discovered by the TDR mechanism on
                                          channel C. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED_1         : 1; /* RESERVED */
    uint16_t u7TDR_PEAKS_AMP_C_1  : 7; /* Amplitude of the second peak discovered by the TDR mechanism
                                          on channel C. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED           : 1; /* RESERVED */
} cy_stc_dp83867_tdr_peaks_amp_6_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_amp_6_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_amp_6_t;


/* TDR_PEAKS_AMP_7 */
typedef struct
{
    uint16_t u7TDR_PEAKS_AMP_C_2  : 7; /* Amplitude of the third peak discovered by the TDR mechanism on
                                          channel C. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED_1         : 1; /* RESERVED */
    uint16_t u7TDR_PEAKS_AMP_C_3  : 7; /* Amplitude of the fourth peak discovered by the TDR mechanism on
                                          channel C. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED           : 1; /* RESERVED */
} cy_stc_dp83867_tdr_peaks_amp_7_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_amp_7_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_amp_7_t;


/* TDR_PEAKS_AMP_8 */
typedef struct
{
    uint16_t u7TDR_PEAKS_AMP_C_4  : 7; /* Amplitude of the fifth peak discovered by the TDR mechanism on
                                          channel C. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED_1         : 1; /* RESERVED */
    uint16_t u7TDR_PEAKS_AMP_D_0  : 7; /* Amplitude of the first peak discovered by the TDR mechanism on
                                          channel D. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED           : 1; /* RESERVED */
} cy_stc_dp83867_tdr_peaks_amp_8_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_amp_8_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_amp_8_t;


/* TDR_PEAKS_AMP_9 */
typedef struct
{
    uint16_t u7TDR_PEAKS_AMP_D_1  : 7; /* Amplitude of the second peak discovered by the TDR mechanism
                                          on channel D. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED_1         : 1; /* RESERVED */
    uint16_t u7TDR_PEAKS_AMP_D_2  : 7; /* Amplitude of the third peak discovered by the TDR mechanism on
                                          channel D. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED           : 1; /* RESERVED */
} cy_stc_dp83867_tdr_peaks_amp_9_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_amp_9_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_amp_9_t;


/* TDR_PEAKS_AMP_10 */
typedef struct
{
    uint16_t u7TDR_PEAKS_AMP_D_3  : 7; /* Amplitude of the fourth peak discovered by the TDR mechanism on
                                          channel D. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED_1         : 1; /* RESERVED */
    uint16_t u7TDR_PEAKS_AMP_D_4  : 7; /* Amplitude of the fifth peak discovered by the TDR mechanism on
                                          channel D. The value of these bits is translated into type of cable
                                          fault and-or interference. */
    uint16_t u1RESERVED           : 1; /* RESERVED */
} cy_stc_dp83867_tdr_peaks_amp_10_t;


typedef union
{
    cy_stc_dp83867_tdr_peaks_amp_10_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_peaks_amp_10_t;


/* TDR_GEN_STATUS */
typedef struct
{
    uint16_t u4RESERVED_1              : 4; /* RESERVED */
    uint16_t u1TDR_P_LOC_OVERFLOW_A    : 1; /* Peak Overflow on Channel A:
                                               1 = More than 5 reflections were detected on channel A.
                                               0 = Normal operation. */
    uint16_t u1TDR_P_LOC_OVERFLOW_B    : 1; /* Peak Overflow on Channel B:
                                               1 = More than 5 reflections were detected on channel B.
                                               0 = Normal operation. */
    uint16_t u1TDR_P_LOC_OVERFLOW_C    : 1; /* Peak Overflow on Channel C:
                                               1 = More than 5 reflections were detected on channel C.
                                               0 = Normal operation. */
    uint16_t u1TDR_P_LOC_OVERFLOW_D    : 1; /* Peak Overflow on Channel D:
                                               1 = More than 5 reflections were detected on channel D.
                                               0 = Normal operation. */
    uint16_t u1TDR_P_LOC_CROSS_MODE_A  : 1; /* Cross Detect on Channel A:
                                               1 = Cross reflection detected on channel A. Indicates a short
                                               between channels.
                                               0 = No cross reflection detected. */
    uint16_t u1TDR_P_LOC_CROSS_MODE_B  : 1; /* Cross Detect on Channel B:
                                               1 = Cross reflection detected on channel B. Indicates a short
                                               between channels.
                                               0 = No cross reflection detected. */
    uint16_t u1TDR_P_LOC_CROSS_MODE_C  : 1; /* Cross Detect on Channel C:
                                               1 = Cross reflection detected on channel C. Indicates a short
                                               between channels.
                                               0 = No cross reflection detected. */
    uint16_t u1TDR_P_LOC_CROSS_MODE_D  : 1; /* Cross Detect on Channel D:
                                               1 = Cross reflection detected on channel D. Indicates a short
                                               between channels.
                                               0 = No cross reflection detected. */
    uint16_t u4RESERVED                : 4; /* RESERVED */
} cy_stc_dp83867_tdr_gen_status_t;


typedef union
{
    cy_stc_dp83867_tdr_gen_status_t f;
    uint16_t u16;
} cy_un_dp83867_tdr_gen_status_t;


/* ALCD_CTRL */
typedef struct
{
    uint16_t u4RESERVED     : 4; /* RESERVED */
    uint16_t u1ALCD_CLEAR     : 1; /* Clear ALCD:
                                      1 = Reset the ALCD results. */
    uint16_t u1ALCD_SUM_DONE  : 1; /* ALCD Complete:
                                      1 = ALCD process has completed.
                                      0 = ALCD process has not completed. */
    uint16_t u2RESERVED       : 2; /* RESERVED */
    uint16_t u8ALCD_SUM       : 8; /* ALCD result */
} cy_stc_dp83867_alcd_ctrl_t;


typedef union
{
    cy_stc_dp83867_alcd_ctrl_t f;
    uint16_t u16;
} cy_un_dp83867_alcd_ctrl_t;


/**
 *****************************************************************************
 ** \brief Addresses of normal PHY registers (addresses 0-31)
 ** 
 *****************************************************************************/
typedef enum
{
    CY_DP83867_REG_NORMAL_BMCR               = 0x0000, // Basic Mode Control Register  \ref cy_un_dp83867_bmcr_t
    CY_DP83867_REG_NORMAL_BMSR               = 0x0001, // Basic Mode Status Register  \ref cy_un_dp83867_bmsr_t
    CY_DP83867_REG_NORMAL_PHYIDR1            = 0x0002, // PHY Identifier Register #1  \ref cy_un_dp83867_phyidr1_t
    CY_DP83867_REG_NORMAL_PHYIDR2            = 0x0003, // PHY Identifier Register #2  \ref cy_un_dp83867_phyidr2_t
    CY_DP83867_REG_NORMAL_ANAR               = 0x0004, // Auto-Negotiation Advertisement Register  \ref cy_un_dp83867_anar_t
    CY_DP83867_REG_NORMAL_ANLPAR             = 0x0005, // Auto-Negotiation Link Partner Ability Register (Base Page)  \ref cy_un_dp83867_anlpar_t
    CY_DP83867_REG_NORMAL_ANER               = 0x0006, // Auto-Negotiation Expansion Register  \ref cy_un_dp83867_aner_t
    CY_DP83867_REG_NORMAL_ANNPTR             = 0x0007, // Auto-Negotiation Next Page TX  \ref cy_un_dp83867_annptr_t
    CY_DP83867_REG_NORMAL_ANNPRR             = 0x0008, // Auto-Negotiation Next Page Transmit Register  \ref cy_un_dp83867_annprr_t
    CY_DP83867_REG_NORMAL_CFG1               = 0x0009, // 1000BASE-T Configuration Register  \ref cy_un_dp83867_cfg1_t
    CY_DP83867_REG_NORMAL_STS1               = 0x000A, // Status Register 1  \ref cy_un_dp83867_sts1_t
    CY_DP83867_REG_NORMAL_REGCR              = 0x000D, // Register Control Register  \ref cy_un_dp83867_regcr_t
    CY_DP83867_REG_NORMAL_ADDAR              = 0x000E, // Address or Data Register  \ref cy_un_dp83867_addar_t
    CY_DP83867_REG_NORMAL_1KSCR              = 0x000F, // 1000BASE-T Status Register  \ref cy_un_dp83867__1kscr_t
    CY_DP83867_REG_NORMAL_PHYCR              = 0x0010, // PHY Control Register  \ref cy_un_dp83867_phycr_t
    CY_DP83867_REG_NORMAL_PHYSTS             = 0x0011, // PHY Status Register  \ref cy_un_dp83867_physts_t
    CY_DP83867_REG_NORMAL_MICR               = 0x0012, // MII Interrupt Control Register  \ref cy_un_dp83867_micr_t
    CY_DP83867_REG_NORMAL_ISR                = 0x0013, // Interrupt Status Register  \ref cy_un_dp83867_isr_t
    CY_DP83867_REG_NORMAL_CFG2               = 0x0014, // Configuration Register 2  \ref cy_un_dp83867_cfg2_t
    CY_DP83867_REG_NORMAL_RECR               = 0x0015, // Receiver Error Counter Register  \ref cy_un_dp83867_recr_t
    CY_DP83867_REG_NORMAL_BISCR              = 0x0016, // BIST Control Register  \ref cy_un_dp83867_biscr_t
    CY_DP83867_REG_NORMAL_STS2               = 0x0017, // Status Register 2  \ref cy_un_dp83867_sts2_t
    CY_DP83867_REG_NORMAL_LEDCR1             = 0x0018, // LED Configuration Register 1  \ref cy_un_dp83867_ledcr1_t
    CY_DP83867_REG_NORMAL_LEDCR2             = 0x0019, // LED Configuration Register 2  \ref cy_un_dp83867_ledcr2_t
    CY_DP83867_REG_NORMAL_LEDCR3             = 0x001A, //  LED Configuration Register 3  \ref cy_un_dp83867_ledcr3_t
    CY_DP83867_REG_NORMAL_CFG3               = 0x001E, // Configuration Register 3  \ref cy_un_dp83867_cfg3_t
    CY_DP83867_REG_NORMAL_CTRL               = 0x001F, // Control Register  \ref cy_un_dp83867_ctrl_t
} cy_en_dp83867_reg_info_t;

/**
 *****************************************************************************
 ** \brief Addresses of extended PHY registers (address > 31)
 ** 
 *****************************************************************************/
typedef enum
{
    CY_DP83867_REG_EXTENDED_TMCH_CTRL        = 0x0025, // Testmode Channel Control  \ref cy_un_dp83867_tmch_ctrl_t
    CY_DP83867_REG_EXTENDED_FLD_CFG          = 0x002D, // Fast Link Drop Configuration Register  \ref cy_un_dp83867_fld_cfg_t
    CY_DP83867_REG_EXTENDED_CFG4             = 0x0031, // Configuration Register 4  \ref cy_un_dp83867_cfg4_t
    CY_DP83867_REG_EXTENDED_RGMIICTL         = 0x0032, // RGMII Control Register  \ref cy_un_dp83867_rgmiictl_t
    CY_DP83867_REG_EXTENDED_RGMIICTL2        = 0x0033, //  RGMII Control Register 2  \ref cy_un_dp83867_rgmiictl2_t
    CY_DP83867_REG_EXTENDED_100CR            = 0x0043, // 100BASE-TX Configuration Register  \ref cy_un_dp83867_100cr_t
    CY_DP83867_REG_EXTENDED_SKEW_FIFO        = 0x0055, // Skew FIFO Status  \ref cy_un_dp83867_skew_fifo_t
    CY_DP83867_REG_EXTENDED_STRAP_STS1_PAP   = 0x006E, // Strap Configuration Status Register 1 for PAP device  \ref cy_un_dp83867_strap_sts1_pap_t
    CY_DP83867_REG_EXTENDED_STRAP_STS1_RGZ   = 0x006E, // Strap Configuration Status Register 1 for RGZ device  \ref cy_un_dp83867_strap_sts1_rgz_t
    CY_DP83867_REG_EXTENDED_STRAP_STS2_PAP   = 0x006F, // Strap Configuration Status Register 2 for PAP device  \ref cy_un_dp83867_strap_sts2_pap_t
    CY_DP83867_REG_EXTENDED_STRAP_STS2_RGZ   = 0x006F, // Strap Configuration Status Register 2 for RGZ device  \ref cy_un_dp83867_strap_sts2_rgz_t
    CY_DP83867_REG_EXTENDED_BICSR1           = 0x0071, // BIST Control and Status Register 1  \ref cy_un_dp83867_bicsr1_t
    CY_DP83867_REG_EXTENDED_BICSR2           = 0x0072, //  BIST Control and Status Register 2  \ref cy_un_dp83867_bicsr2_t
    CY_DP83867_REG_EXTENDED_RGMIIDCTL        = 0x0086, // RGMII Delay Control Register  \ref cy_un_dp83867_rgmiidctl_t
    CY_DP83867_REG_EXTENDED_SYNC_FIFO_CTRL   = 0x00E9, // Sync FIFO Control  \ref cy_un_dp83867_sync_fifo_ctrl_t
    CY_DP83867_REG_EXTENDED_LOOPCR           = 0x00FE, // Loopback Configuration Register   \ref cy_un_dp83867_loopcr_t
    CY_DP83867_REG_EXTENDED_RXFCFG           = 0x0134, // Receive Configuration Register  \ref cy_un_dp83867_rxfcfg_t
    CY_DP83867_REG_EXTENDED_RXFSTS           = 0x0135, // Receive Status Register  \ref cy_un_dp83867_rxfsts_t
    CY_DP83867_REG_EXTENDED_RXFPMD1          = 0x0136, // Pattern Match Data Register 1  \ref cy_un_dp83867_rxfpmd1_t
    CY_DP83867_REG_EXTENDED_RXFPMD2          = 0x0137, // Pattern Match Data Register 2   \ref cy_un_dp83867_rxfpmd2_t
    CY_DP83867_REG_EXTENDED_RXFPMD3          = 0x0138, // Pattern Match Data Register 3  \ref cy_un_dp83867_rxfpmd3_t
    CY_DP83867_REG_EXTENDED_RXFSOP1          = 0x0139, // SecureOn Pass Register 1  \ref cy_un_dp83867_rxfsop1_t
    CY_DP83867_REG_EXTENDED_RXFSOP2          = 0x013A, // SecureOn Pass Register 2  \ref cy_un_dp83867_rxfsop2_t
    CY_DP83867_REG_EXTENDED_RXFSOP3          = 0x013B, // SecureOn Pass Register 3  \ref cy_un_dp83867_rxfsop3_t
    CY_DP83867_REG_EXTENDED_RXFPAT           = 0x013C, // Receive Pattern Register n  \ref cy_un_dp83867_rxfpat_t
    CY_DP83867_REG_EXTENDED_RXFPBM1          = 0x015C, // Receive Pattern Byte Mask Register 1  \ref cy_un_dp83867_rxfpbm1_t
    CY_DP83867_REG_EXTENDED_RXFPBM2          = 0x015D, // Receive Pattern Byte Mask Register 2  \ref cy_un_dp83867_rxfpbm2_t
    CY_DP83867_REG_EXTENDED_RXFPBM3          = 0x015E, // Receive Pattern Byte Mask Register 3  \ref cy_un_dp83867_rxfpbm3_t
    CY_DP83867_REG_EXTENDED_RXFPBM4          = 0x015F, // Receive Pattern Byte Mask Register 4  \ref cy_un_dp83867_rxfpbm4_t
    CY_DP83867_REG_EXTENDED_RXFPATC          = 0x0161, // Receive Status Register  \ref cy_un_dp83867_rxfpatc_t
    CY_DP83867_REG_EXTENDED_IO_MUX_CFG       = 0x0170, //  I/O Configuration  \ref cy_un_dp83867_io_mux_cfg_t
    CY_DP83867_REG_EXTENDED__GPIO_MUX_CTRL1  = 0x0171, // GPIO Mux Control Register 1  \ref cy_un_dp83867__gpio_mux_ctrl1_t
    CY_DP83867_REG_EXTENDED_GPIO_MUX_CTRL2   = 0x0172, // GPIO Mux Control Register 2 for PAP device  \ref cy_un_dp83867_gpio_mux_ctrl2_t
    CY_DP83867_REG_EXTENDED_GPIO_MUX_CTRL    = 0x0172, // GPIO Mux Control Register for RGZ device  \ref cy_un_dp83867_gpio_mux_ctrl_t
    CY_DP83867_REG_EXTENDED_TDR_GEN_CFG1     = 0x0180, // TDR General Configuration Register 1  \ref cy_un_dp83867_tdr_gen_cfg1_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_LOC_1  = 0x0190, // TDR Peak Locations Register 1  \ref cy_un_dp83867_tdr_peaks_loc_1_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_LOC_2  = 0x0191, // TDR Peak Locations Register 2  \ref cy_un_dp83867_tdr_peaks_loc_2_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_LOC_3  = 0x0192, // TDR Peak Locations Register 3  \ref cy_un_dp83867_tdr_peaks_loc_3_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_LOC_4  = 0x0193, // TDR Peak Locations Register 4  \ref cy_un_dp83867_tdr_peaks_loc_4_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_LOC_5  = 0x0194, // TDR Peak Locations Register 5  \ref cy_un_dp83867_tdr_peaks_loc_5_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_LOC_6  = 0x0195, // TDR Peak Locations Register 6  \ref cy_un_dp83867_tdr_peaks_loc_6_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_LOC_7  = 0x0196, // TDR Peak Locations Register 7  \ref cy_un_dp83867_tdr_peaks_loc_7_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_LOC_8  = 0x0197, // TDR Peak Locations Register 8  \ref cy_un_dp83867_tdr_peaks_loc_8_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_LOC_9  = 0x0198, // TDR Peak Locations Register 9  \ref cy_un_dp83867_tdr_peaks_loc_9_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_LOC_10 = 0x0199, // TDR Peak Locations Register 10  \ref cy_un_dp83867_tdr_peaks_loc_10_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_AMP_1  = 0x019A, // TDR Peak Amplitudes Register 1  \ref cy_un_dp83867_tdr_peaks_amp_1_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_AMP_2  = 0x019B, // TDR Peak Amplitudes Register 2  \ref cy_un_dp83867_tdr_peaks_amp_2_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_AMP_3  = 0x019C, // TDR Peak Amplitudes Register 3  \ref cy_un_dp83867_tdr_peaks_amp_3_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_AMP_4  = 0x019D, // TDR Peak Amplitudes Register 4  \ref cy_un_dp83867_tdr_peaks_amp_4_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_AMP_5  = 0x019E, // TDR Peak Amplitudes Register 5  \ref cy_un_dp83867_tdr_peaks_amp_5_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_AMP_6  = 0x019F, // TDR Peak Amplitudes Register 6  \ref cy_un_dp83867_tdr_peaks_amp_6_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_AMP_7  = 0x01A0, // TDR Peak Amplitudes Register 7  \ref cy_un_dp83867_tdr_peaks_amp_7_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_AMP_8  = 0x01A1, // TDR Peak Amplitudes Register 8  \ref cy_un_dp83867_tdr_peaks_amp_8_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_AMP_9  = 0x01A2, // TDR Peak Amplitudes Register 9  \ref cy_un_dp83867_tdr_peaks_amp_9_t
    CY_DP83867_REG_EXTENDED_TDR_PEAKS_AMP_10 = 0x01A3, // TDR Peak Amplitudes Register 10  \ref cy_un_dp83867_tdr_peaks_amp_10_t
    CY_DP83867_REG_EXTENDED_TDR_GEN_STATUS   = 0x01A4, // TDR General Status  \ref cy_un_dp83867_tdr_gen_status_t
    CY_DP83867_REG_EXTENDED_ALCD_CTRL        = 0x01A7, //  Advanced Link Cable Diagnostics Control Register  \ref cy_un_dp83867_alcd_ctrl_t
} cy_en_dp83867_ex_reg_info_t;


/**
 *****************************************************************************
 ** \brief Driver Handle
 ** 
 *****************************************************************************/
typedef struct
{
    volatile stc_ETH_t* pEth;    /**  Pointer to ETH IP base */
    uint8_t             phyAddr; /**  PHY address (0-31) */
} cy_stc_dp83867_t;


/**
 *****************************************************************************
 ** \brief Eth Media Independent Interface mode
 ** 
 *****************************************************************************/
typedef enum 
{
    CY_DP83867_INTERFACE_MII,    /**  MII interface */
    CY_DP83867_INTERFACE_GMII,   /**  GMII interface */
    CY_DP83867_INTERFACE_RGMII,  /**  RGMII interface */
} cy_en_dp83867_interface_mode_t;

/**
 *****************************************************************************
 ** \brief Eth Speed mode
 ** 
 *****************************************************************************/
typedef enum 
{
    CY_DP83867_SPEED_10   = 10,   /**  10 MBit/s */
    CY_DP83867_SPEED_100  = 100,  /**  100 MBit/s */
    CY_DP83867_SPEED_1000 = 1000, /**  1000 MBit/s */
} cy_en_dp83867_speed_mode_t;

/**
 *****************************************************************************
 ** \brief PHY configuration
 ** 
 *****************************************************************************/
typedef struct 
{
    cy_en_dp83867_interface_mode_t ifMode;        /**  Interface mode between MAC and PHY */
    cy_en_dp83867_speed_mode_t     speedMode;     /**  Speed mode that shall be advertised (#enableAutoNeg == true) or configured */
    bool                           enableAutoNeg; /**  Selects between auto-negotation and fixed configuration */
} cy_stc_dp83867_phy_cfg_t;


/*****************************************************************************/
/* Global variable declarations ('extern', definition in C source)           */
/*****************************************************************************/


/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source)             */
/*****************************************************************************/

extern void     Cy_Dp83867_Init(volatile stc_ETH_t* pEth, uint8_t phyAddr, cy_stc_dp83867_t* handle);
extern uint16_t Cy_Dp83867_ReadReg(cy_en_dp83867_reg_info_t reg, cy_stc_dp83867_t* handle);
extern void     Cy_Dp83867_WriteReg(cy_en_dp83867_reg_info_t reg, uint16_t data, cy_stc_dp83867_t* handle);
extern uint16_t Cy_Dp83867_ReadExtendedReg(cy_en_dp83867_ex_reg_info_t reg, cy_stc_dp83867_t* handle);
extern void     Cy_Dp83867_WriteExtendedReg(cy_en_dp83867_ex_reg_info_t reg, uint16_t data, cy_stc_dp83867_t* handle);
extern void     Cy_Dp83867_ConfigurePhy(cy_stc_dp83867_phy_cfg_t * phyCfg, cy_stc_dp83867_t* handle);
extern bool     Cy_Dp83867_IsLinkUp(cy_stc_dp83867_t* handle);

#endif // __CY_DP83867_H__
