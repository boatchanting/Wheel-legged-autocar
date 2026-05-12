/***************************************************************************//**
* \file cyip_eth_v2.h
*
* \brief
* ETH IP definitions
*
* \note
* Generator version: 1.6.0.481
* Database revision: TVIIBH4M_PR3_0
*
********************************************************************************
* \copyright
* Copyright 2016-2021, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef _CYIP_ETH_V2_H_
#define _CYIP_ETH_V2_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     ETH
*******************************************************************************/

#define ETH_SECTION_SIZE                        0x00010000UL

/**
  * \brief MXETH Control Register (ETH_CTL)
  */
typedef struct stc_ETH_CTL_field {
  uint32_t                         u2ETH_MODE:2;
  uint32_t                         u1REFCLK_SRC_SEL:1;
  uint32_t                         :5;
  uint32_t                         u8REFCLK_DIV:8;
  uint32_t                         :15;
  uint32_t                         u1ENABLED:1;
} stc_ETH_CTL_field_t;

typedef union un_ETH_CTL {
  uint32_t                         u32Register;
  stc_ETH_CTL_field_t              stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_CTL_t;

/**
  * \brief MXETH Status Register (ETH_STATUS)
  */
typedef struct stc_ETH_STATUS_field {
  uint32_t                         u1PFC_NEGOTIATE:1;
  uint32_t                         :7;
  uint32_t                         u8RX_PFC_PAUSED:8;
  uint32_t                         :16;
} stc_ETH_STATUS_field_t;

typedef union un_ETH_STATUS {
  uint32_t                         u32Register;
  stc_ETH_STATUS_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_STATUS_t;

/**
  * \brief The network control register contains general MAC control functions for both receiver and transmitter. (ETH_NETWORK_CONTROL)
  */
typedef struct stc_ETH_NETWORK_CONTROL_field {
  uint32_t                         u1LOOPBACK:1;
  uint32_t                         u1LOOPBACK_LOCAL:1;
  uint32_t                         u1ENABLE_RECEIVE:1;
  uint32_t                         u1ENABLE_TRANSMIT:1;
  uint32_t                         u1MAN_PORT_EN:1;
  uint32_t                         u1CLEAR_ALL_STATS_REGS:1;
  uint32_t                         u1INC_ALL_STATS_REGS:1;
  uint32_t                         u1STATS_WRITE_EN:1;
  uint32_t                         u1BACK_PRESSURE:1;
  uint32_t                         u1TX_START_PCLK:1;
  uint32_t                         u1TX_HALT_PCLK:1;
  uint32_t                         u1TX_PAUSE_FRAME_REQ:1;
  uint32_t                         u1TX_PAUSE_FRAME_ZERO:1;
  uint32_t                         u1REMOVED_13:1;
  uint32_t                         u1REMOVED_14:1;
  uint32_t                         u1STORE_RX_TS:1;
  uint32_t                         u1PFC_ENABLE:1;
  uint32_t                         u1TRANSMIT_PFC_PRIORITY_BASED_PAUSE_FRAME:1;
  uint32_t                         u1FLUSH_RX_PKT_PCLK:1;
  uint32_t                         u1TX_LPI_EN:1;
  uint32_t                         u1PTP_UNICAST_ENA:1;
  uint32_t                         u1ALT_SGMII_MODE:1;
  uint32_t                         u1STORE_UDP_OFFSET:1;
  uint32_t                         u1EXT_TSU_PORT_ENABLE:1;
  uint32_t                         u1ONE_STEP_SYNC_MODE:1;
  uint32_t                         u1PFC_CTRL:1;
  uint32_t                         u1EXT_RXQ_SEL_EN:1;
  uint32_t                         u1OSS_CORRECTION_FIELD:1;
  uint32_t                         u1SEL_MII_ON_RGMII:1;
  uint32_t                         u1TWO_PT_FIVE_GIG:1;
  uint32_t                         u1IFG_EATS_QAV_CREDIT:1;
  uint32_t                         u1EXT_RXQ_RESERVED_31:1;
} stc_ETH_NETWORK_CONTROL_field_t;

typedef union un_ETH_NETWORK_CONTROL {
  uint32_t                         u32Register;
  stc_ETH_NETWORK_CONTROL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_NETWORK_CONTROL_t;

/**
  * \brief The network configuration register contains functions for setting the mode of operation for the Gigabit Ethernet MAC. (ETH_NETWORK_CONFIG)
  */
typedef struct stc_ETH_NETWORK_CONFIG_field {
  uint32_t                         u1SPEED:1;
  uint32_t                         u1FULL_DUPLEX:1;
  uint32_t                         u1DISCARD_NON_VLAN_FRAMES:1;
  uint32_t                         u1JUMBO_FRAMES:1;
  uint32_t                         u1COPY_ALL_FRAMES:1;
  uint32_t                         u1NO_BROADCAST:1;
  uint32_t                         u1MULTICAST_HASH_ENABLE:1;
  uint32_t                         u1UNICAST_HASH_ENABLE:1;
  uint32_t                         u1RECEIVE_1536_BYTE_FRAMES:1;
  uint32_t                         u1EXTERNAL_ADDRESS_MATCH_ENABLE:1;
  uint32_t                         u1GIGABIT_MODE_ENABLE:1;
  uint32_t                         u1PCS_SELECT:1;
  uint32_t                         u1RETRY_TEST:1;
  uint32_t                         u1PAUSE_ENABLE:1;
  uint32_t                         u2RECEIVE_BUFFER_OFFSET:2;
  uint32_t                         u1LENGTH_FIELD_ERROR_FRAME_DISCARD:1;
  uint32_t                         u1FCS_REMOVE:1;
  uint32_t                         u3MDC_CLOCK_DIVISION:3;
  uint32_t                         u2DATA_BUS_WIDTH:2;
  uint32_t                         u1DISABLE_COPY_OF_PAUSE_FRAMES:1;
  uint32_t                         u1RECEIVE_CHECKSUM_OFFLOAD_ENABLE:1;
  uint32_t                         u1EN_HALF_DUPLEX_RX:1;
  uint32_t                         u1IGNORE_RX_FCS:1;
  uint32_t                         u1SGMII_MODE_ENABLE:1;
  uint32_t                         u1IPG_STRETCH_ENABLE:1;
  uint32_t                         u1NSP_CHANGE:1;
  uint32_t                         u1IGNORE_IPG_RX_ER:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_NETWORK_CONFIG_field_t;

typedef union un_ETH_NETWORK_CONFIG {
  uint32_t                         u32Register;
  stc_ETH_NETWORK_CONFIG_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_NETWORK_CONFIG_t;

/**
  * \brief The network status register returns status information with respect to the PHY management interface. (ETH_NETWORK_STATUS)
  */
typedef struct stc_ETH_NETWORK_STATUS_field {
  uint32_t                         u1PCS_LINK_STATE:1;
  uint32_t                         u1MDIO_IN:1;
  uint32_t                         u1MAN_DONE:1;
  uint32_t                         u1MAC_FULL_DUPLEX:1;
  uint32_t                         u2REMOVED_5_4:2;
  uint32_t                         u1PFC_NEGOTIATE_PCLK:1;
  uint32_t                         u1LPI_INDICATE_PCLK:1;
  uint32_t                         :24;
} stc_ETH_NETWORK_STATUS_field_t;

typedef union un_ETH_NETWORK_STATUS {
  uint32_t                         u32Register;
  stc_ETH_NETWORK_STATUS_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_NETWORK_STATUS_t;

/**
  * \brief Not presents. Access to the register will return AHB error. (ETH_USER_IO_REGISTER)
  */
typedef struct stc_ETH_USER_IO_REGISTER_field {
  uint32_t                         u32RESERVED_31_0:32;
} stc_ETH_USER_IO_REGISTER_field_t;

typedef union un_ETH_USER_IO_REGISTER {
  uint32_t                         u32Register;
  stc_ETH_USER_IO_REGISTER_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_USER_IO_REGISTER_t;

/**
  * \brief DMA Configuration Register (ETH_DMA_CONFIG)
  */
typedef struct stc_ETH_DMA_CONFIG_field {
  uint32_t                         u5AMBA_BURST_LENGTH:5;
  uint32_t                         u1HDR_DATA_SPLITTING_EN:1;
  uint32_t                         u1ENDIAN_SWAP_MANAGEMENT:1;
  uint32_t                         u1ENDIAN_SWAP_PACKET:1;
  uint32_t                         u2RX_PBUF_SIZE:2;
  uint32_t                         u1TX_PBUF_SIZE:1;
  uint32_t                         u1TX_PBUF_TCP_EN:1;
  uint32_t                         u1INFINITE_LAST_DBUF_SIZE_EN:1;
  uint32_t                         u1CRC_ERROR_REPORT:1;
  uint32_t                         :2;
  uint32_t                         u8RX_BUF_SIZE:8;
  uint32_t                         u1FORCE_DISCARD_ON_ERR:1;
  uint32_t                         u1FORCE_MAX_AMBA_BURST_RX:1;
  uint32_t                         u1FORCE_MAX_AMBA_BURST_TX:1;
  uint32_t                         :1;
  uint32_t                         u1RX_BD_EXTENDED_MODE_EN:1;
  uint32_t                         u1TX_BD_EXTENDED_MODE_EN:1;
  uint32_t                         u1DMA_ADDR_BUS_WIDTH_1:1;
} stc_ETH_DMA_CONFIG_field_t;

typedef union un_ETH_DMA_CONFIG {
  uint32_t                         u32Register;
  stc_ETH_DMA_CONFIG_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DMA_CONFIG_t;

/**
  * \brief This register, when read, provides details of the status of a transmit. Once read, individual bits may be cleared by writing 1 to them. It is not possible to set a bit to 1 by writing to the register. (ETH_TRANSMIT_STATUS)
  */
typedef struct stc_ETH_TRANSMIT_STATUS_field {
  uint32_t                         u1USED_BIT_READ:1;
  uint32_t                         u1COLLISION_OCCURRED:1;
  uint32_t                         u1RETRY_LIMIT_EXCEEDED:1;
  uint32_t                         u1TRANSMIT_GO:1;
  uint32_t                         u1AMBA_ERROR123:1;
  uint32_t                         u1TRANSMIT_COMPLETE123:1;
  uint32_t                         u1TRANSMIT_UNDER_RUN123:1;
  uint32_t                         u1LATE_COLLISION_OCCURRED:1;
  uint32_t                         u1RESP_NOT_OK123:1;
  uint32_t                         :23;
} stc_ETH_TRANSMIT_STATUS_field_t;

typedef union un_ETH_TRANSMIT_STATUS {
  uint32_t                         u32Register;
  stc_ETH_TRANSMIT_STATUS_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TRANSMIT_STATUS_t;

/**
  * \brief This register holds the start address of the receive buffer queue (receive buffers descriptor list). The receive buffer queue base address must be initialized before receive is enabled through bit 2 of the network control register. Once reception is enabled, any write to the receive buffer queue base address register is ignored. Reading this register returns the location of the descriptor currently being accessed. This value increments as buffers are used. Software should not use this register for determining where to remove received frames from the queue as it constantly changes as new frames are received. Software should instead work its way through the buffer descriptor queue checking the used bits. In terms of AMBA (AXI) operation, the receive descriptors are read from memory using a single 32bit AXI access. When the datapath is configured at 64bit, the receive descriptors should be aligned at 64-bit boundaries and each pair of 32-bit descriptors is written to using a single 64bit AXI access. (ETH_RECEIVE_Q_PTR)
  */
typedef struct stc_ETH_RECEIVE_Q_PTR_field {
  uint32_t                         u1DMA_RX_DIS_Q:1;
  uint32_t                         :1;
  uint32_t                         u30DMA_RX_Q_PTR:30;
} stc_ETH_RECEIVE_Q_PTR_field_t;

typedef union un_ETH_RECEIVE_Q_PTR {
  uint32_t                         u32Register;
  stc_ETH_RECEIVE_Q_PTR_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RECEIVE_Q_PTR_t;

/**
  * \brief This register holds the start address of the transmit buffer queue (transmit buffers descriptor list). The transmit buffer queue base address register must be initialized before transmit is started through bit 9 of the network control register. Once transmission has started, any write to the transmit buffer queue base address register is illegal and therefore ignored. Note that due to clock boundary synchronization, it takes a maximum of four pclk cycles from the writing of the transmit start bit before the transmitter is active. Writing to the transmit buffer queue base address register during this time may produce unpredictable results. Reading this register returns the location of the descriptor currently being accessed. Because the DMA can store data for multiple frames at once, this may not necessarily be pointing to the current frame being transmitted. In terms of AMBA AXI operation, the transmit descriptors are written to memory using a single 32bit AHB access. When the datapath is configured as 64bit, the transmit descriptors should be aligned at 64-bit boundaries and each pair of 32-bit descriptors is read from memory using a single AXI access. (ETH_TRANSMIT_Q_PTR)
  */
typedef struct stc_ETH_TRANSMIT_Q_PTR_field {
  uint32_t                         u1DMA_TX_DIS_Q:1;
  uint32_t                         :1;
  uint32_t                         u30DMA_TX_Q_PTR:30;
} stc_ETH_TRANSMIT_Q_PTR_field_t;

typedef union un_ETH_TRANSMIT_Q_PTR {
  uint32_t                         u32Register;
  stc_ETH_TRANSMIT_Q_PTR_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TRANSMIT_Q_PTR_t;

/**
  * \brief This register, when read provides details of the status of a receive. Once read, individual bits may be cleared by writing 1 to them. It is not possible to set a bit to 1 by writing to the register. (ETH_RECEIVE_STATUS)
  */
typedef struct stc_ETH_RECEIVE_STATUS_field {
  uint32_t                         u1BUFFER_NOT_AVAILABLE:1;
  uint32_t                         u1FRAME_RECEIVED:1;
  uint32_t                         u1RECEIVE_OVERRUN123:1;
  uint32_t                         u1RESP_NOT_OK1234:1;
  uint32_t                         :28;
} stc_ETH_RECEIVE_STATUS_field_t;

typedef union un_ETH_RECEIVE_STATUS {
  uint32_t                         u32Register;
  stc_ETH_RECEIVE_STATUS_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RECEIVE_STATUS_t;

/**
  * \brief If not configured for priority queueing, the GEM generates a single interrupt. This register indicates the source of this interrupt. The corresponding bit in the mask register must be clear for a bit to be set. If any bit is set in this register the ethernet_int signal will be asserted. For test purposes each bit can be set or reset by writing to the interrupt mask register. The default configuration is shown below whereby all bits are reset to zero on read. Changing the validity of the `gem_irq_read_clear define will instead require a one to be written to the appropriate bit in order to clear it. In this mode reading has no affect on the status of the bit. (ETH_INT_STATUS)
  */
typedef struct stc_ETH_INT_STATUS_field {
  uint32_t                         u1MANAGEMENT_FRAME_SENT:1;
  uint32_t                         u1RECEIVE_COMPLETE:1;
  uint32_t                         u1RX_USED_BIT_READ:1;
  uint32_t                         u1TX_USED_BIT_READ:1;
  uint32_t                         u1TRANSMIT_UNDER_RUN:1;
  uint32_t                         u1RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION:1;
  uint32_t                         u1AMBA_ERROR:1;
  uint32_t                         u1TRANSMIT_COMPLETE:1;
  uint32_t                         :1;
  uint32_t                         u1REMOVED_9:1;
  uint32_t                         u1RECEIVE_OVERRUN:1;
  uint32_t                         u1RESP_NOT_OK:1;
  uint32_t                         u1PAUSE_FRAME_WITH_NON_ZERO_PAUSE_QUANTUM_RECEIVED:1;
  uint32_t                         u1PAUSE_TIME_ELAPSED:1;
  uint32_t                         u1PAUSE_FRAME_TRANSMITTED:1;
  uint32_t                         u1REMOVED_15:1;
  uint32_t                         u1REMOVED_16:1;
  uint32_t                         u1REMOVED_17:1;
  uint32_t                         u1PTP_DELAY_REQ_FRAME_RECEIVED:1;
  uint32_t                         u1PTP_SYNC_FRAME_RECEIVED:1;
  uint32_t                         u1PTP_DELAY_REQ_FRAME_TRANSMITTED:1;
  uint32_t                         u1PTP_SYNC_FRAME_TRANSMITTED:1;
  uint32_t                         u1PTP_PDELAY_REQ_FRAME_RECEIVED:1;
  uint32_t                         u1PTP_PDELAY_RESP_FRAME_RECEIVED:1;
  uint32_t                         u1PTP_PDELAY_REQ_FRAME_TRANSMITTED:1;
  uint32_t                         u1PTP_PDELAY_RESP_FRAME_TRANSMITTED:1;
  uint32_t                         u1TSU_SECONDS_REGISTER_INCREMENT:1;
  uint32_t                         u1RECEIVE_LPI_INDICATION_STATUS_BIT_CHANGE:1;
  uint32_t                         u1REMOVED_28:1;
  uint32_t                         u1TSU_TIMER_COMPARISON_INTERRUPT:1;
  uint32_t                         :2;
} stc_ETH_INT_STATUS_field_t;

typedef union un_ETH_INT_STATUS {
  uint32_t                         u32Register;
  stc_ETH_INT_STATUS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_STATUS_t;

/**
  * \brief At reset all interrupts are disabled. Writing a one to the relevant bit location enables the required interrupt. This register is write only and when read will return zero. (ETH_INT_ENABLE)
  */
typedef struct stc_ETH_INT_ENABLE_field {
  uint32_t                         u1ENABLE_MANAGEMENT_DONE_INTERRUPT:1;
  uint32_t                         u1ENABLE_RECEIVE_COMPLETE_INTERRUPT:1;
  uint32_t                         u1ENABLE_RECEIVE_USED_BIT_READ_INTERRUPT:1;
  uint32_t                         u1ENABLE_TRANSMIT_USED_BIT_READ_INTERRUPT:1;
  uint32_t                         u1ENABLE_TRANSMIT_BUFFER_UNDER_RUN_INTERRUPT:1;
  uint32_t                         u1ENABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT:1;
  uint32_t                         u1ENABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT:1;
  uint32_t                         u1ENABLE_TRANSMIT_COMPLETE_INTERRUPT:1;
  uint32_t                         u1UNUSED_8:1;
  uint32_t                         u1UNUSED_9:1;
  uint32_t                         u1ENABLE_RECEIVE_OVERRUN_INTERRUPT:1;
  uint32_t                         u1ENABLE_RESP_NOT_OK_INTERRUPT:1;
  uint32_t                         u1ENABLE_PAUSE_FRAME_WITH_NON_ZERO_PAUSE_QUANTUM_INTERRUPT:1;
  uint32_t                         u1ENABLE_PAUSE_TIME_ZERO_INTERRUPT:1;
  uint32_t                         u1ENABLE_PAUSE_FRAME_TRANSMITTED_INTERRUPT:1;
  uint32_t                         u1UNUSED_15:1;
  uint32_t                         u1UNUSED_16:1;
  uint32_t                         u1UNUSED_17:1;
  uint32_t                         u1ENABLE_PTP_DELAY_REQ_FRAME_RECEIVED:1;
  uint32_t                         u1ENABLE_PTP_SYNC_FRAME_RECEIVED:1;
  uint32_t                         u1ENABLE_PTP_DELAY_REQ_FRAME_TRANSMITTED:1;
  uint32_t                         u1ENABLE_PTP_SYNC_FRAME_TRANSMITTED:1;
  uint32_t                         u1ENABLE_PTP_PDELAY_REQ_FRAME_RECEIVED:1;
  uint32_t                         u1ENABLE_PTP_PDELAY_RESP_FRAME_RECEIVED:1;
  uint32_t                         u1ENABLE_PTP_PDELAY_REQ_FRAME_TRANSMITTED:1;
  uint32_t                         u1ENABLE_PTP_PDELAY_RESP_FRAME_TRANSMITTED:1;
  uint32_t                         u1ENABLE_TSU_SECONDS_REGISTER_INCREMENT:1;
  uint32_t                         u1ENABLE_RX_LPI_INDICATION_INTERRUPT:1;
  uint32_t                         u1UNUSED_28:1;
  uint32_t                         u1ENABLE_TSU_TIMER_COMPARISON_INTERRUPT:1;
  uint32_t                         :2;
} stc_ETH_INT_ENABLE_field_t;

typedef union un_ETH_INT_ENABLE {
  uint32_t                         u32Register;
  stc_ETH_INT_ENABLE_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_ENABLE_t;

/**
  * \brief Writing a 1 to the relevant bit location disables that particular interrupt. This register is write only and when read will return zero. (ETH_INT_DISABLE)
  */
typedef struct stc_ETH_INT_DISABLE_field {
  uint32_t                         u1DISABLE_MANAGEMENT_DONE_INTERRUPT:1;
  uint32_t                         u1DISABLE_RECEIVE_COMPLETE_INTERRUPT:1;
  uint32_t                         u1DISABLE_RECEIVE_USED_BIT_READ_INTERRUPT:1;
  uint32_t                         u1DISABLE_TRANSMIT_USED_BIT_READ_INTERRUPT:1;
  uint32_t                         u1DISABLE_TRANSMIT_BUFFER_UNDER_RUN_INTERRUPT:1;
  uint32_t                         u1DISABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT:1;
  uint32_t                         u1DISABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT:1;
  uint32_t                         u1DISABLE_TRANSMIT_COMPLETE_INTERRUPT:1;
  uint32_t                         u1UNUSED_8:1;
  uint32_t                         u1UNUSED_9:1;
  uint32_t                         u1DISABLE_RECEIVE_OVERRUN_INTERRUPT:1;
  uint32_t                         u1DISABLE_RESP_NOT_OK_INTERRUPT:1;
  uint32_t                         u1DISABLE_PAUSE_FRAME_WITH_NON_ZERO_PAUSE_QUANTUM_INTERRUPT:1;
  uint32_t                         u1DISABLE_PAUSE_TIME_ZERO_INTERRUPT:1;
  uint32_t                         u1DISABLE_PAUSE_FRAME_TRANSMITTED_INTERRUPT:1;
  uint32_t                         u1UNUSED_15:1;
  uint32_t                         u1UNUSED_16:1;
  uint32_t                         u1UNUSED_17:1;
  uint32_t                         u1DISABLE_PTP_DELAY_REQ_FRAME_RECEIVED:1;
  uint32_t                         u1DISABLE_PTP_SYNC_FRAME_RECEIVED:1;
  uint32_t                         u1DISABLE_PTP_DELAY_REQ_FRAME_TRANSMITTED:1;
  uint32_t                         u1DISABLE_PTP_SYNC_FRAME_TRANSMITTED:1;
  uint32_t                         u1DISABLE_PTP_PDELAY_REQ_FRAME_RECEIVED:1;
  uint32_t                         u1DISABLE_PTP_PDELAY_RESP_FRAME_RECEIVED:1;
  uint32_t                         u1DISABLE_PTP_PDELAY_REQ_FRAME_TRANSMITTED:1;
  uint32_t                         u1DISABLE_PTP_PDELAY_RESP_FRAME_TRANSMITTED:1;
  uint32_t                         u1DISABLE_TSU_SECONDS_REGISTER_INCREMENT:1;
  uint32_t                         u1DISABLE_RX_LPI_INDICATION_INTERRUPT:1;
  uint32_t                         u1UNUSED_28:1;
  uint32_t                         u1DISABLE_TSU_TIMER_COMPARISON_INTERRUPT:1;
  uint32_t                         u1RESERVED_30_30:1;
  uint32_t                         u1RESERVED_31_31:1;
} stc_ETH_INT_DISABLE_field_t;

typedef union un_ETH_INT_DISABLE {
  uint32_t                         u32Register;
  stc_ETH_INT_DISABLE_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_DISABLE_t;

/**
  * \brief The interrupt mask register is a read only register indicating which interrupts are masked. All bits are set at reset and can be reset individually by writing to the interrupt enable register or set individually by writing to the interrupt disable register. Having separate address locations for enable and disable saves the need for performing a read modify write when updating the interrupt mask register. For test purposes there is a write-only function to this register that allows the bits in the interrupt status register to be set or cleared, regardless of the state of the mask register. (ETH_INT_MASK)
  */
typedef struct stc_ETH_INT_MASK_field {
  uint32_t                         u1MANAGEMENT_DONE_INTERRUPT_MASK:1;
  uint32_t                         u1RECEIVE_COMPLETE_INTERRUPT_MASK:1;
  uint32_t                         u1RECEIVE_USED_BIT_READ_INTERRUPT_MASK:1;
  uint32_t                         u1TRANSMIT_USED_BIT_READ_INTERRUPT_MASK:1;
  uint32_t                         u1TRANSMIT_BUFFER_UNDER_RUN_INTERRUPT_MASK:1;
  uint32_t                         u1RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_MASK:1;
  uint32_t                         u1AMBA_ERROR_INTERRUPT_MASK:1;
  uint32_t                         u1TRANSMIT_COMPLETE_INTERRUPT_MASK:1;
  uint32_t                         u1UNUSED_8:1;
  uint32_t                         u1UNUSED_9:1;
  uint32_t                         u1RECEIVE_OVERRUN_INTERRUPT_MASK:1;
  uint32_t                         u1RESP_NOT_OK_INTERRUPT_MASK:1;
  uint32_t                         u1PAUSE_FRAME_WITH_NON_ZERO_PAUSE_QUANTUM_INTERRUPT_MASK:1;
  uint32_t                         u1PAUSE_TIME_ZERO_INTERRUPT_MASK:1;
  uint32_t                         u1PAUSE_FRAME_TRANSMITTED_INTERRUPT_MASK:1;
  uint32_t                         u1UNUSED_15:1;
  uint32_t                         u1UNUSED_16:1;
  uint32_t                         u1UNUSED_17:1;
  uint32_t                         u1PTP_DELAY_REQ_FRAME_RECEIVED_MASK:1;
  uint32_t                         u1PTP_SYNC_FRAME_RECEIVED_MASK:1;
  uint32_t                         u1PTP_DELAY_REQ_FRAME_TRANSMITTED_MASK:1;
  uint32_t                         u1PTP_SYNC_FRAME_TRANSMITTED_MASK:1;
  uint32_t                         u1PTP_PDELAY_REQ_FRAME_RECEIVED_MASK:1;
  uint32_t                         u1PTP_PDELAY_RESP_FRAME_RECEIVED_MASK:1;
  uint32_t                         u1PTP_PDELAY_REQ_FRAME_TRANSMITTED_MASK:1;
  uint32_t                         u1PTP_PDELAY_RESP_FRAME_TRANSMITTED_MASK:1;
  uint32_t                         u1TSU_SECONDS_REGISTER_INCREMENT_MASK:1;
  uint32_t                         u1RX_LPI_INDICATION_MASK:1;
  uint32_t                         u1UNUSED_28:1;
  uint32_t                         u1TSU_TIMER_COMPARISON_MASK:1;
  uint32_t                         :2;
} stc_ETH_INT_MASK_field_t;

typedef union un_ETH_INT_MASK {
  uint32_t                         u32Register;
  stc_ETH_INT_MASK_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_MASK_t;

/**
  * \brief The PHY maintenance register is implemented as a shift register. Writing to the register starts a shift operation which is signalled as complete when bit-2 is set in the network status register. It takes about 2000 pclk cycles to complete, when MDC is set for pclk divide by 32 in the network configuration register. An interrupt is generated upon completion. During this time, the MSB of the register is output on the MDIO pin and the LSB updated from the MDIO pin with each MDC cycle. This causes transmission of a PHY management frame on MDIO. See Section 22.2.4.5 of the IEEE 802.3 standard. Reading during the shift operation will return the current contents of the shift register. At the end of management operation, the bits will have shifted back to their original locations. For a read operation, the data bits will be updated with data read from the PHY. It is important to write the correct values to the register to ensure a valid PHY management frame is produced. The MDIO interface can read IEEE 802.3 clause 45 PHYs as well as clause 22 PHYs. To read clause 45 PHYs, bit 30 should be written with a 0 rather than a 1. For a description of MDC generation, see Network Configuration Register. (ETH_PHY_MANAGEMENT)
  */
typedef struct stc_ETH_PHY_MANAGEMENT_field {
  uint32_t                         u16PHY_WRITE_READ_DATA:16;
  uint32_t                         u2WRITE10:2;
  uint32_t                         u5REGISTER_ADDRESS:5;
  uint32_t                         u5PHY_ADDRESS:5;
  uint32_t                         u2OPERATION:2;
  uint32_t                         u1WRITE1:1;
  uint32_t                         u1WRITE0:1;
} stc_ETH_PHY_MANAGEMENT_field_t;

typedef union un_ETH_PHY_MANAGEMENT {
  uint32_t                         u32Register;
  stc_ETH_PHY_MANAGEMENT_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_PHY_MANAGEMENT_t;

/**
  * \brief Received Pause Quantum Register (ETH_PAUSE_TIME)
  */
typedef struct stc_ETH_PAUSE_TIME_field {
  uint32_t                         u16QUANTUM:16;
  uint32_t                         :16;
} stc_ETH_PAUSE_TIME_field_t;

typedef union un_ETH_PAUSE_TIME {
  uint32_t                         u32Register;
  stc_ETH_PAUSE_TIME_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_PAUSE_TIME_t;

/**
  * \brief Transmit Pause Quantum Register (ETH_TX_PAUSE_QUANTUM)
  */
typedef struct stc_ETH_TX_PAUSE_QUANTUM_field {
  uint32_t                         u16QUANTUM:16;
  uint32_t                         u16QUANTUM_P1:16;
} stc_ETH_TX_PAUSE_QUANTUM_field_t;

typedef union un_ETH_TX_PAUSE_QUANTUM {
  uint32_t                         u32Register;
  stc_ETH_TX_PAUSE_QUANTUM_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TX_PAUSE_QUANTUM_t;

/**
  * \brief Partial store and forward is only applicable when using the DMA configured in SRAM based packet buffer mode. It is also not available when using multi buffer frames. TX Partial Store and Forward (ETH_PBUF_TXCUTTHRU)
  */
typedef struct stc_ETH_PBUF_TXCUTTHRU_field {
  uint32_t                         u9DMA_TX_CUTTHRU_THRESHOLD:9;
  uint32_t                         :22;
  uint32_t                         u1DMA_TX_CUTTHRU:1;
} stc_ETH_PBUF_TXCUTTHRU_field_t;

typedef union un_ETH_PBUF_TXCUTTHRU {
  uint32_t                         u32Register;
  stc_ETH_PBUF_TXCUTTHRU_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_PBUF_TXCUTTHRU_t;

/**
  * \brief RX Partial Store and Forward (ETH_PBUF_RXCUTTHRU)
  */
typedef struct stc_ETH_PBUF_RXCUTTHRU_field {
  uint32_t                         u8DMA_RX_CUTTHRU_THRESHOLD:8;
  uint32_t                         :23;
  uint32_t                         u1DMA_RX_CUTTHRU:1;
} stc_ETH_PBUF_RXCUTTHRU_field_t;

typedef union un_ETH_PBUF_RXCUTTHRU {
  uint32_t                         u32Register;
  stc_ETH_PBUF_RXCUTTHRU_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_PBUF_RXCUTTHRU_t;

/**
  * \brief Maximum Jumbo Frame Size. (ETH_JUMBO_MAX_LENGTH)
  */
typedef struct stc_ETH_JUMBO_MAX_LENGTH_field {
  uint32_t                         u14JUMBO_MAX_LENGTH:14;
  uint32_t                         :18;
} stc_ETH_JUMBO_MAX_LENGTH_field_t;

typedef union un_ETH_JUMBO_MAX_LENGTH {
  uint32_t                         u32Register;
  stc_ETH_JUMBO_MAX_LENGTH_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_JUMBO_MAX_LENGTH_t;

/**
  * \brief Not presents. (ETH_EXTERNAL_FIFO_INTERFACE)
  */
typedef struct stc_ETH_EXTERNAL_FIFO_INTERFACE_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_EXTERNAL_FIFO_INTERFACE_field_t;

typedef union un_ETH_EXTERNAL_FIFO_INTERFACE {
  uint32_t                         u32Register;
  stc_ETH_EXTERNAL_FIFO_INTERFACE_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_EXTERNAL_FIFO_INTERFACE_t;

/**
  * \brief Used to set the maximum amount of outstanding transactions on the AXI bus between AR / R channels and AW / W channels. Cannot be more than the depth of the configured AXI pipeline FIFO (defined in verilog defs.v) (ETH_AXI_MAX_PIPELINE)
  */
typedef struct stc_ETH_AXI_MAX_PIPELINE_field {
  uint32_t                         u8AR2R_MAX_PIPELINE:8;
  uint32_t                         u8AW2W_MAX_PIPELINE:8;
  uint32_t                         u1USE_AW2B_FILL:1;
  uint32_t                         :15;
} stc_ETH_AXI_MAX_PIPELINE_field_t;

typedef union un_ETH_AXI_MAX_PIPELINE {
  uint32_t                         u32Register;
  stc_ETH_AXI_MAX_PIPELINE_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_AXI_MAX_PIPELINE_t;

/**
  * \brief Not presents. Access to the register will return AHB error. (ETH_RSC_CONTROL)
  */
typedef struct stc_ETH_RSC_CONTROL_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_RSC_CONTROL_field_t;

typedef union un_ETH_RSC_CONTROL {
  uint32_t                         u32Register;
  stc_ETH_RSC_CONTROL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RSC_CONTROL_t;

/**
  * \brief Used to moderate the number of transmit and receive complete interrupts issued. With interrupt moderation enabled receive and transmit interrupts are not generated immediately a frame is transmitted or received. Instead when a receive or transmit event occurs a timer is started and the interrupt is asserted after it times out. This limits the frequency with which the CPU receives interrupts. When interrupt moderation is enabled interrupt status bit one is always used for receive and bit 7 is always used for transmit even when priority queuing is enabled. With interrupt moderation 800ns periods are counted. GEM determines what constitutes an 800ns period by looking at the tbi (bit 11), gigabit bit (10) and speed (bit 0) bits in the network configuration register and counting tx_clk cycles. Bit 0 needs to be set to 1 for 100M operation. (ETH_INT_MODERATION)
  */
typedef struct stc_ETH_INT_MODERATION_field {
  uint32_t                         u8RX_INT_MODERATION:8;
  uint32_t                         :8;
  uint32_t                         u8TX_INT_MODERATION:8;
  uint32_t                         :8;
} stc_ETH_INT_MODERATION_field_t;

typedef union un_ETH_INT_MODERATION {
  uint32_t                         u32Register;
  stc_ETH_INT_MODERATION_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_MODERATION_t;

/**
  * \brief Used to pause transmission after deassertion of tx_lpi_en. Each unit in this register corresponds to 64ns in gigabit mode, 320ns in 100M mode and 3200ns at 10M. After tx_lpi_en is deasserted transmission will pause for the set time. (ETH_SYS_WAKE_TIME)
  */
typedef struct stc_ETH_SYS_WAKE_TIME_field {
  uint32_t                         u16SYS_WAKE_TIME:16;
  uint32_t                         :16;
} stc_ETH_SYS_WAKE_TIME_field_t;

typedef union un_ETH_SYS_WAKE_TIME {
  uint32_t                         u32Register;
  stc_ETH_SYS_WAKE_TIME_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SYS_WAKE_TIME_t;

/**
  * \brief The unicast hash enable and the multicast hash enable bits in the network configuration register enable the reception of hash matched frames. Hash Register Bottom (31 to 0 bits) (ETH_HASH_BOTTOM)
  */
typedef struct stc_ETH_HASH_BOTTOM_field {
  uint32_t                         u32ADDRESS_HASH_B:32;
} stc_ETH_HASH_BOTTOM_field_t;

typedef union un_ETH_HASH_BOTTOM {
  uint32_t                         u32Register;
  stc_ETH_HASH_BOTTOM_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_HASH_BOTTOM_t;

/**
  * \brief Hash Register Top (63 to 32 bits) (ETH_HASH_TOP)
  */
typedef struct stc_ETH_HASH_TOP_field {
  uint32_t                         u32ADDRESS_HASH_T:32;
} stc_ETH_HASH_TOP_field_t;

typedef union un_ETH_HASH_TOP {
  uint32_t                         u32Register;
  stc_ETH_HASH_TOP_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_HASH_TOP_t;

/**
  * \brief The addresses stored in the specific address registers are deactivated at reset or when their corresponding specific address register bottom is written. They are activated when specific address register top is written. (ETH_SPEC_ADD1_BOTTOM)
  */
typedef struct stc_ETH_SPEC_ADD1_BOTTOM_field {
  uint32_t                         u32ADDRESS_ADD1_B:32;
} stc_ETH_SPEC_ADD1_BOTTOM_field_t;

typedef union un_ETH_SPEC_ADD1_BOTTOM {
  uint32_t                         u32Register;
  stc_ETH_SPEC_ADD1_BOTTOM_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_ADD1_BOTTOM_t;

/**
  * \brief Specific Address Top (ETH_SPEC_ADD1_TOP)
  */
typedef struct stc_ETH_SPEC_ADD1_TOP_field {
  uint32_t                         u16ADDRESS_TOP:16;
  uint32_t                         u1FILTER_TYPE:1;
  uint32_t                         :15;
} stc_ETH_SPEC_ADD1_TOP_field_t;

typedef union un_ETH_SPEC_ADD1_TOP {
  uint32_t                         u32Register;
  stc_ETH_SPEC_ADD1_TOP_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_ADD1_TOP_t;

/**
  * \brief The addresses stored in the specific address registers are deactivated at reset or when their corresponding specific address register bottom is written. They are activated when specific address register top is written. (ETH_SPEC_ADD2_BOTTOM)
  */
typedef struct stc_ETH_SPEC_ADD2_BOTTOM_field {
  uint32_t                         u32ADDRESS_BOTTOM:32;
} stc_ETH_SPEC_ADD2_BOTTOM_field_t;

typedef union un_ETH_SPEC_ADD2_BOTTOM {
  uint32_t                         u32Register;
  stc_ETH_SPEC_ADD2_BOTTOM_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_ADD2_BOTTOM_t;

/**
  * \brief Specific Address Top (ETH_SPEC_ADD2_TOP)
  */
typedef struct stc_ETH_SPEC_ADD2_TOP_field {
  uint32_t                         u16ADDRESS_TOP:16;
  uint32_t                         u1FILTER_TYPE:1;
  uint32_t                         :7;
  uint32_t                         u6FILTER_BYTE_MASK:6;
  uint32_t                         :2;
} stc_ETH_SPEC_ADD2_TOP_field_t;

typedef union un_ETH_SPEC_ADD2_TOP {
  uint32_t                         u32Register;
  stc_ETH_SPEC_ADD2_TOP_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_ADD2_TOP_t;

/**
  * \brief The addresses stored in the specific address registers are deactivated at reset or when their corresponding specific address register bottom is written. They are activated when specific address register top is written. (ETH_SPEC_ADD3_BOTTOM)
  */
typedef struct stc_ETH_SPEC_ADD3_BOTTOM_field {
  uint32_t                         u32ADDRESS_BOTTOM:32;
} stc_ETH_SPEC_ADD3_BOTTOM_field_t;

typedef union un_ETH_SPEC_ADD3_BOTTOM {
  uint32_t                         u32Register;
  stc_ETH_SPEC_ADD3_BOTTOM_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_ADD3_BOTTOM_t;

/**
  * \brief Specific Address Top (ETH_SPEC_ADD3_TOP)
  */
typedef struct stc_ETH_SPEC_ADD3_TOP_field {
  uint32_t                         u16ADDRESS_TOP:16;
  uint32_t                         u1FILTER_TYPE:1;
  uint32_t                         :7;
  uint32_t                         u6FILTER_BYTE_MASK:6;
  uint32_t                         :2;
} stc_ETH_SPEC_ADD3_TOP_field_t;

typedef union un_ETH_SPEC_ADD3_TOP {
  uint32_t                         u32Register;
  stc_ETH_SPEC_ADD3_TOP_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_ADD3_TOP_t;

/**
  * \brief The addresses stored in the specific address registers are deactivated at reset or when their corresponding specific address register bottom is written. They are activated when specific address register top is written. (ETH_SPEC_ADD4_BOTTOM)
  */
typedef struct stc_ETH_SPEC_ADD4_BOTTOM_field {
  uint32_t                         u32ADDRESS_BOTTOM:32;
} stc_ETH_SPEC_ADD4_BOTTOM_field_t;

typedef union un_ETH_SPEC_ADD4_BOTTOM {
  uint32_t                         u32Register;
  stc_ETH_SPEC_ADD4_BOTTOM_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_ADD4_BOTTOM_t;

/**
  * \brief Specific Address Top (ETH_SPEC_ADD4_TOP)
  */
typedef struct stc_ETH_SPEC_ADD4_TOP_field {
  uint32_t                         u16ADDRESS_TOP:16;
  uint32_t                         u1FILTER_TYPE:1;
  uint32_t                         :7;
  uint32_t                         u6FILTER_BYTE_MASK:6;
  uint32_t                         :2;
} stc_ETH_SPEC_ADD4_TOP_field_t;

typedef union un_ETH_SPEC_ADD4_TOP {
  uint32_t                         u32Register;
  stc_ETH_SPEC_ADD4_TOP_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_ADD4_TOP_t;

/**
  * \brief Type ID Match 1 (ETH_SPEC_TYPE1)
  */
typedef struct stc_ETH_SPEC_TYPE1_field {
  uint32_t                         u16MATCH:16;
  uint32_t                         :15;
  uint32_t                         u1ENABLE_COPY:1;
} stc_ETH_SPEC_TYPE1_field_t;

typedef union un_ETH_SPEC_TYPE1 {
  uint32_t                         u32Register;
  stc_ETH_SPEC_TYPE1_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_TYPE1_t;

/**
  * \brief Type ID Match 2 (ETH_SPEC_TYPE2)
  */
typedef struct stc_ETH_SPEC_TYPE2_field {
  uint32_t                         u16MATCH:16;
  uint32_t                         :15;
  uint32_t                         u1ENABLE_COPY:1;
} stc_ETH_SPEC_TYPE2_field_t;

typedef union un_ETH_SPEC_TYPE2 {
  uint32_t                         u32Register;
  stc_ETH_SPEC_TYPE2_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_TYPE2_t;

/**
  * \brief Type ID Match 3 (ETH_SPEC_TYPE3)
  */
typedef struct stc_ETH_SPEC_TYPE3_field {
  uint32_t                         u16MATCH:16;
  uint32_t                         :15;
  uint32_t                         u1ENABLE_COPY:1;
} stc_ETH_SPEC_TYPE3_field_t;

typedef union un_ETH_SPEC_TYPE3 {
  uint32_t                         u32Register;
  stc_ETH_SPEC_TYPE3_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_TYPE3_t;

/**
  * \brief Type ID Match 4 (ETH_SPEC_TYPE4)
  */
typedef struct stc_ETH_SPEC_TYPE4_field {
  uint32_t                         u16MATCH:16;
  uint32_t                         :15;
  uint32_t                         u1ENABLE_COPY:1;
} stc_ETH_SPEC_TYPE4_field_t;

typedef union un_ETH_SPEC_TYPE4 {
  uint32_t                         u32Register;
  stc_ETH_SPEC_TYPE4_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_TYPE4_t;

/**
  * \brief Wake on LAN Register. Presents in design, but feature is not supported. (ETH_WOL_REGISTER)
  */
typedef struct stc_ETH_WOL_REGISTER_field {
  uint32_t                         u16ADDR:16;
  uint32_t                         u1WOL_MASK_0:1;
  uint32_t                         u1WOL_MASK_1:1;
  uint32_t                         u1WOL_MASK_2:1;
  uint32_t                         u1WOL_MASK_3:1;
  uint32_t                         :12;
} stc_ETH_WOL_REGISTER_field_t;

typedef union un_ETH_WOL_REGISTER {
  uint32_t                         u32Register;
  stc_ETH_WOL_REGISTER_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_WOL_REGISTER_t;

/**
  * \brief IPG stretch register (ETH_STRETCH_RATIO)
  */
typedef struct stc_ETH_STRETCH_RATIO_field {
  uint32_t                         u16IPG_STRETCH:16;
  uint32_t                         :16;
} stc_ETH_STRETCH_RATIO_field_t;

typedef union un_ETH_STRETCH_RATIO {
  uint32_t                         u32Register;
  stc_ETH_STRETCH_RATIO_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_STRETCH_RATIO_t;

/**
  * \brief Stacked VLAN Register (ETH_STACKED_VLAN)
  */
typedef struct stc_ETH_STACKED_VLAN_field {
  uint32_t                         u16MATCH:16;
  uint32_t                         :15;
  uint32_t                         u1ENABLE_PROCESSING:1;
} stc_ETH_STACKED_VLAN_field_t;

typedef union un_ETH_STACKED_VLAN {
  uint32_t                         u32Register;
  stc_ETH_STACKED_VLAN_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_STACKED_VLAN_t;

/**
  * \brief Transmit PFC Pause Register (ETH_TX_PFC_PAUSE)
  */
typedef struct stc_ETH_TX_PFC_PAUSE_field {
  uint32_t                         u8VECTOR_ENABLE:8;
  uint32_t                         u8VECTOR:8;
  uint32_t                         :16;
} stc_ETH_TX_PFC_PAUSE_field_t;

typedef union un_ETH_TX_PFC_PAUSE {
  uint32_t                         u32Register;
  stc_ETH_TX_PFC_PAUSE_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TX_PFC_PAUSE_t;

/**
  * \brief Specific Address Mask 1 Bottom (31 to 0 bits) (ETH_MASK_ADD1_BOTTOM)
  */
typedef struct stc_ETH_MASK_ADD1_BOTTOM_field {
  uint32_t                         u32ADDRESS_MASK_BOTTOM:32;
} stc_ETH_MASK_ADD1_BOTTOM_field_t;

typedef union un_ETH_MASK_ADD1_BOTTOM {
  uint32_t                         u32Register;
  stc_ETH_MASK_ADD1_BOTTOM_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_MASK_ADD1_BOTTOM_t;

/**
  * \brief Specific Address Mask 1 Top (47 to 32 bits) (ETH_MASK_ADD1_TOP)
  */
typedef struct stc_ETH_MASK_ADD1_TOP_field {
  uint32_t                         u16ADDRESS_MASK_TOP:16;
  uint32_t                         :16;
} stc_ETH_MASK_ADD1_TOP_field_t;

typedef union un_ETH_MASK_ADD1_TOP {
  uint32_t                         u32Register;
  stc_ETH_MASK_ADD1_TOP_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_MASK_ADD1_TOP_t;

/**
  * \brief Receive DMA Data Buffer Address Mask (ETH_DMA_ADDR_OR_MASK)
  */
typedef struct stc_ETH_DMA_ADDR_OR_MASK_field {
  uint32_t                         u4MASK_ENABLE:4;
  uint32_t                         :24;
  uint32_t                         u4MASK_VALUE_DA:4;
} stc_ETH_DMA_ADDR_OR_MASK_field_t;

typedef union un_ETH_DMA_ADDR_OR_MASK {
  uint32_t                         u32Register;
  stc_ETH_DMA_ADDR_OR_MASK_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DMA_ADDR_OR_MASK_t;

/**
  * \brief PTP RX unicast IP destination address (ETH_RX_PTP_UNICAST)
  */
typedef struct stc_ETH_RX_PTP_UNICAST_field {
  uint32_t                         u32ADDRESS_UNICAST:32;
} stc_ETH_RX_PTP_UNICAST_field_t;

typedef union un_ETH_RX_PTP_UNICAST {
  uint32_t                         u32Register;
  stc_ETH_RX_PTP_UNICAST_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RX_PTP_UNICAST_t;

/**
  * \brief PTP TX unicast IP destination address (ETH_TX_PTP_UNICAST)
  */
typedef struct stc_ETH_TX_PTP_UNICAST_field {
  uint32_t                         u32ADDRESS_UNICAST:32;
} stc_ETH_TX_PTP_UNICAST_field_t;

typedef union un_ETH_TX_PTP_UNICAST {
  uint32_t                         u32Register;
  stc_ETH_TX_PTP_UNICAST_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TX_PTP_UNICAST_t;

/**
  * \brief TSU timer comparison value nanoseconds (ETH_TSU_NSEC_CMP)
  */
typedef struct stc_ETH_TSU_NSEC_CMP_field {
  uint32_t                         u22COMPARISON_NSEC:22;
  uint32_t                         :10;
} stc_ETH_TSU_NSEC_CMP_field_t;

typedef union un_ETH_TSU_NSEC_CMP {
  uint32_t                         u32Register;
  stc_ETH_TSU_NSEC_CMP_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_NSEC_CMP_t;

/**
  * \brief TSU timer comparison value seconds (31 to 0 bits) (ETH_TSU_SEC_CMP)
  */
typedef struct stc_ETH_TSU_SEC_CMP_field {
  uint32_t                         u32COMPARISON_SEC:32;
} stc_ETH_TSU_SEC_CMP_field_t;

typedef union un_ETH_TSU_SEC_CMP {
  uint32_t                         u32Register;
  stc_ETH_TSU_SEC_CMP_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_SEC_CMP_t;

/**
  * \brief TSU timer comparison value seconds (47 to 32 bits) (ETH_TSU_MSB_SEC_CMP)
  */
typedef struct stc_ETH_TSU_MSB_SEC_CMP_field {
  uint32_t                         u16COMPARISON_MSB_SEC:16;
  uint32_t                         :16;
} stc_ETH_TSU_MSB_SEC_CMP_field_t;

typedef union un_ETH_TSU_MSB_SEC_CMP {
  uint32_t                         u32Register;
  stc_ETH_TSU_MSB_SEC_CMP_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_MSB_SEC_CMP_t;

/**
  * \brief PTP Event Frame Transmitted Seconds Register (47 to 32 bits) (ETH_TSU_PTP_TX_MSB_SEC)
  */
typedef struct stc_ETH_TSU_PTP_TX_MSB_SEC_field {
  uint32_t                         u16TIMER_SECONDS:16;
  uint32_t                         :16;
} stc_ETH_TSU_PTP_TX_MSB_SEC_field_t;

typedef union un_ETH_TSU_PTP_TX_MSB_SEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_PTP_TX_MSB_SEC_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_PTP_TX_MSB_SEC_t;

/**
  * \brief PTP Event Frame Received Seconds Register (47 to 32 bits) (ETH_TSU_PTP_RX_MSB_SEC)
  */
typedef struct stc_ETH_TSU_PTP_RX_MSB_SEC_field {
  uint32_t                         u16TIMER_SECONDS:16;
  uint32_t                         :16;
} stc_ETH_TSU_PTP_RX_MSB_SEC_field_t;

typedef union un_ETH_TSU_PTP_RX_MSB_SEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_PTP_RX_MSB_SEC_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_PTP_RX_MSB_SEC_t;

/**
  * \brief PTP Peer Event Frame Transmitted Seconds Register (47 to 32 bits) (ETH_TSU_PEER_TX_MSB_SEC)
  */
typedef struct stc_ETH_TSU_PEER_TX_MSB_SEC_field {
  uint32_t                         u16TIMER_SECONDS:16;
  uint32_t                         :16;
} stc_ETH_TSU_PEER_TX_MSB_SEC_field_t;

typedef union un_ETH_TSU_PEER_TX_MSB_SEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_PEER_TX_MSB_SEC_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_PEER_TX_MSB_SEC_t;

/**
  * \brief PTP Peer Event Frame Received Seconds Register (47 to 32 bits) (ETH_TSU_PEER_RX_MSB_SEC)
  */
typedef struct stc_ETH_TSU_PEER_RX_MSB_SEC_field {
  uint32_t                         u16TIMER_SECONDS:16;
  uint32_t                         :16;
} stc_ETH_TSU_PEER_RX_MSB_SEC_field_t;

typedef union un_ETH_TSU_PEER_RX_MSB_SEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_PEER_RX_MSB_SEC_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_PEER_RX_MSB_SEC_t;

/**
  * \brief The fill levels for the TX & RX packet buffers can be read using this register, including the fill level for each queue in the TX direction. (ETH_DPRAM_FILL_DBG)
  */
typedef struct stc_ETH_DPRAM_FILL_DBG_field {
  uint32_t                         u1DMA_TX_RX_FILL_LEVEL_SELECT:1;
  uint32_t                         :3;
  uint32_t                         u4DMA_TX_Q_FILL_LEVEL_SELECT:4;
  uint32_t                         :8;
  uint32_t                         u16DMA_TX_RX_FILL_LEVEL:16;
} stc_ETH_DPRAM_FILL_DBG_field_t;

typedef union un_ETH_DPRAM_FILL_DBG {
  uint32_t                         u32Register;
  stc_ETH_DPRAM_FILL_DBG_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DPRAM_FILL_DBG_t;

/**
  * \brief This register indicates a Cadence module identification number and module revision. The value of this register is read only as defined by `gem_revision_reg_value (ETH_REVISION_REG)
  */
typedef struct stc_ETH_REVISION_REG_field {
  uint32_t                         u16MODULE_REVISION:16;
  uint32_t                         u12MODULE_IDENTIFICATION_NUMBER:12;
  uint32_t                         u4FIX_NUMBER:4;
} stc_ETH_REVISION_REG_field_t;

typedef union un_ETH_REVISION_REG {
  uint32_t                         u32Register;
  stc_ETH_REVISION_REG_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_REVISION_REG_t;

/**
  * \brief Octets Transmitted lower bits (31 to 0 bits) (ETH_OCTETS_TXED_BOTTOM)
  */
typedef struct stc_ETH_OCTETS_TXED_BOTTOM_field {
  uint32_t                         u32COUNT_BOTTOM:32;
} stc_ETH_OCTETS_TXED_BOTTOM_field_t;

typedef union un_ETH_OCTETS_TXED_BOTTOM {
  uint32_t                         u32Register;
  stc_ETH_OCTETS_TXED_BOTTOM_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_OCTETS_TXED_BOTTOM_t;

/**
  * \brief Octets Transmitted higher bits (47 to 32 bits) (ETH_OCTETS_TXED_TOP)
  */
typedef struct stc_ETH_OCTETS_TXED_TOP_field {
  uint32_t                         u16COUNT_TOP:16;
  uint32_t                         :16;
} stc_ETH_OCTETS_TXED_TOP_field_t;

typedef union un_ETH_OCTETS_TXED_TOP {
  uint32_t                         u32Register;
  stc_ETH_OCTETS_TXED_TOP_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_OCTETS_TXED_TOP_t;

/**
  * \brief Frames Transmitted (ETH_FRAMES_TXED_OK)
  */
typedef struct stc_ETH_FRAMES_TXED_OK_field {
  uint32_t                         u32COUNT_OK:32;
} stc_ETH_FRAMES_TXED_OK_field_t;

typedef union un_ETH_FRAMES_TXED_OK {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_TXED_OK_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_TXED_OK_t;

/**
  * \brief Broadcast Frames Transmitted (ETH_BROADCAST_TXED)
  */
typedef struct stc_ETH_BROADCAST_TXED_field {
  uint32_t                         u32COUNT_BROADCAST:32;
} stc_ETH_BROADCAST_TXED_field_t;

typedef union un_ETH_BROADCAST_TXED {
  uint32_t                         u32Register;
  stc_ETH_BROADCAST_TXED_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_BROADCAST_TXED_t;

/**
  * \brief Multicast Frames Transmitted (ETH_MULTICAST_TXED)
  */
typedef struct stc_ETH_MULTICAST_TXED_field {
  uint32_t                         u32COUNT_MULTICAST:32;
} stc_ETH_MULTICAST_TXED_field_t;

typedef union un_ETH_MULTICAST_TXED {
  uint32_t                         u32Register;
  stc_ETH_MULTICAST_TXED_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_MULTICAST_TXED_t;

/**
  * \brief Pause Frames Transmitted (ETH_PAUSE_FRAMES_TXED)
  */
typedef struct stc_ETH_PAUSE_FRAMES_TXED_field {
  uint32_t                         u16COUNT_PAUSE:16;
  uint32_t                         :16;
} stc_ETH_PAUSE_FRAMES_TXED_field_t;

typedef union un_ETH_PAUSE_FRAMES_TXED {
  uint32_t                         u32Register;
  stc_ETH_PAUSE_FRAMES_TXED_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_PAUSE_FRAMES_TXED_t;

/**
  * \brief 64 Byte Frames Transmitted (ETH_FRAMES_TXED_64)
  */
typedef struct stc_ETH_FRAMES_TXED_64_field {
  uint32_t                         u32COUNT_64:32;
} stc_ETH_FRAMES_TXED_64_field_t;

typedef union un_ETH_FRAMES_TXED_64 {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_TXED_64_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_TXED_64_t;

/**
  * \brief 65 to 127 Byte Frames Transmitted (ETH_FRAMES_TXED_65)
  */
typedef struct stc_ETH_FRAMES_TXED_65_field {
  uint32_t                         u32COUNT_65:32;
} stc_ETH_FRAMES_TXED_65_field_t;

typedef union un_ETH_FRAMES_TXED_65 {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_TXED_65_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_TXED_65_t;

/**
  * \brief 128 to 255 Byte Frames Transmitted (ETH_FRAMES_TXED_128)
  */
typedef struct stc_ETH_FRAMES_TXED_128_field {
  uint32_t                         u32COUNT_128:32;
} stc_ETH_FRAMES_TXED_128_field_t;

typedef union un_ETH_FRAMES_TXED_128 {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_TXED_128_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_TXED_128_t;

/**
  * \brief 256 to 511 Byte Frames Transmitted (ETH_FRAMES_TXED_256)
  */
typedef struct stc_ETH_FRAMES_TXED_256_field {
  uint32_t                         u32COUNT_256:32;
} stc_ETH_FRAMES_TXED_256_field_t;

typedef union un_ETH_FRAMES_TXED_256 {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_TXED_256_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_TXED_256_t;

/**
  * \brief 512 to 1023 Byte Frames Transmitted (ETH_FRAMES_TXED_512)
  */
typedef struct stc_ETH_FRAMES_TXED_512_field {
  uint32_t                         u32COUNT_512:32;
} stc_ETH_FRAMES_TXED_512_field_t;

typedef union un_ETH_FRAMES_TXED_512 {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_TXED_512_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_TXED_512_t;

/**
  * \brief 1024 to 1518 Byte Frames Transmitted (ETH_FRAMES_TXED_1024)
  */
typedef struct stc_ETH_FRAMES_TXED_1024_field {
  uint32_t                         u32COUNT_1024:32;
} stc_ETH_FRAMES_TXED_1024_field_t;

typedef union un_ETH_FRAMES_TXED_1024 {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_TXED_1024_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_TXED_1024_t;

/**
  * \brief Greater Than 1518 Byte Frames Transmitted (ETH_FRAMES_TXED_1519)
  */
typedef struct stc_ETH_FRAMES_TXED_1519_field {
  uint32_t                         u32COUNT_1519:32;
} stc_ETH_FRAMES_TXED_1519_field_t;

typedef union un_ETH_FRAMES_TXED_1519 {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_TXED_1519_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_TXED_1519_t;

/**
  * \brief Transmit Under Runs (ETH_TX_UNDERRUNS)
  */
typedef struct stc_ETH_TX_UNDERRUNS_field {
  uint32_t                         u10COUNT_UN:10;
  uint32_t                         :22;
} stc_ETH_TX_UNDERRUNS_field_t;

typedef union un_ETH_TX_UNDERRUNS {
  uint32_t                         u32Register;
  stc_ETH_TX_UNDERRUNS_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TX_UNDERRUNS_t;

/**
  * \brief Single Collision Frames. Presents in design but not support. (ETH_SINGLE_COLLISIONS)
  */
typedef struct stc_ETH_SINGLE_COLLISIONS_field {
  uint32_t                         u18COUNT14:18;
  uint32_t                         :14;
} stc_ETH_SINGLE_COLLISIONS_field_t;

typedef union un_ETH_SINGLE_COLLISIONS {
  uint32_t                         u32Register;
  stc_ETH_SINGLE_COLLISIONS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SINGLE_COLLISIONS_t;

/**
  * \brief Multiple Collision Frames. Presents in design but not support. (ETH_MULTIPLE_COLLISIONS)
  */
typedef struct stc_ETH_MULTIPLE_COLLISIONS_field {
  uint32_t                         u18COUNT15:18;
  uint32_t                         :14;
} stc_ETH_MULTIPLE_COLLISIONS_field_t;

typedef union un_ETH_MULTIPLE_COLLISIONS {
  uint32_t                         u32Register;
  stc_ETH_MULTIPLE_COLLISIONS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_MULTIPLE_COLLISIONS_t;

/**
  * \brief Excessive Collisions.  Presents in design but not support. (ETH_EXCESSIVE_COLLISIONS)
  */
typedef struct stc_ETH_EXCESSIVE_COLLISIONS_field {
  uint32_t                         u10COUNT16:10;
  uint32_t                         :22;
} stc_ETH_EXCESSIVE_COLLISIONS_field_t;

typedef union un_ETH_EXCESSIVE_COLLISIONS {
  uint32_t                         u32Register;
  stc_ETH_EXCESSIVE_COLLISIONS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_EXCESSIVE_COLLISIONS_t;

/**
  * \brief Late Collisions.  Presents in design but not support. (ETH_LATE_COLLISIONS)
  */
typedef struct stc_ETH_LATE_COLLISIONS_field {
  uint32_t                         u10COUNT17:10;
  uint32_t                         :22;
} stc_ETH_LATE_COLLISIONS_field_t;

typedef union un_ETH_LATE_COLLISIONS {
  uint32_t                         u32Register;
  stc_ETH_LATE_COLLISIONS_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_LATE_COLLISIONS_t;

/**
  * \brief Deferred Transmission Frames.  Presents in design but not support. (ETH_DEFERRED_FRAMES)
  */
typedef struct stc_ETH_DEFERRED_FRAMES_field {
  uint32_t                         u18COUNT18:18;
  uint32_t                         :14;
} stc_ETH_DEFERRED_FRAMES_field_t;

typedef union un_ETH_DEFERRED_FRAMES {
  uint32_t                         u32Register;
  stc_ETH_DEFERRED_FRAMES_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DEFERRED_FRAMES_t;

/**
  * \brief Carrier Sense Errors.  Presents in design but not support. (ETH_CRS_ERRORS)
  */
typedef struct stc_ETH_CRS_ERRORS_field {
  uint32_t                         u10COUNT19:10;
  uint32_t                         :22;
} stc_ETH_CRS_ERRORS_field_t;

typedef union un_ETH_CRS_ERRORS {
  uint32_t                         u32Register;
  stc_ETH_CRS_ERRORS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_CRS_ERRORS_t;

/**
  * \brief Octets Received (31 to 0 bits) (ETH_OCTETS_RXED_BOTTOM)
  */
typedef struct stc_ETH_OCTETS_RXED_BOTTOM_field {
  uint32_t                         u32COUNT_BOTTOM:32;
} stc_ETH_OCTETS_RXED_BOTTOM_field_t;

typedef union un_ETH_OCTETS_RXED_BOTTOM {
  uint32_t                         u32Register;
  stc_ETH_OCTETS_RXED_BOTTOM_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_OCTETS_RXED_BOTTOM_t;

/**
  * \brief Octets Received (47 to 32 bits) (ETH_OCTETS_RXED_TOP)
  */
typedef struct stc_ETH_OCTETS_RXED_TOP_field {
  uint32_t                         u16COUNT_TOP:16;
  uint32_t                         :16;
} stc_ETH_OCTETS_RXED_TOP_field_t;

typedef union un_ETH_OCTETS_RXED_TOP {
  uint32_t                         u32Register;
  stc_ETH_OCTETS_RXED_TOP_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_OCTETS_RXED_TOP_t;

/**
  * \brief Frames Received (ETH_FRAMES_RXED_OK)
  */
typedef struct stc_ETH_FRAMES_RXED_OK_field {
  uint32_t                         u32COUNT_OK:32;
} stc_ETH_FRAMES_RXED_OK_field_t;

typedef union un_ETH_FRAMES_RXED_OK {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_RXED_OK_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_RXED_OK_t;

/**
  * \brief Broadcast Frames Received (ETH_BROADCAST_RXED)
  */
typedef struct stc_ETH_BROADCAST_RXED_field {
  uint32_t                         u32COUNT_BROADCAST:32;
} stc_ETH_BROADCAST_RXED_field_t;

typedef union un_ETH_BROADCAST_RXED {
  uint32_t                         u32Register;
  stc_ETH_BROADCAST_RXED_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_BROADCAST_RXED_t;

/**
  * \brief Multicast Frames Received (ETH_MULTICAST_RXED)
  */
typedef struct stc_ETH_MULTICAST_RXED_field {
  uint32_t                         u32COUNT_MULTICAST:32;
} stc_ETH_MULTICAST_RXED_field_t;

typedef union un_ETH_MULTICAST_RXED {
  uint32_t                         u32Register;
  stc_ETH_MULTICAST_RXED_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_MULTICAST_RXED_t;

/**
  * \brief Pause Frames Received (ETH_PAUSE_FRAMES_RXED)
  */
typedef struct stc_ETH_PAUSE_FRAMES_RXED_field {
  uint32_t                         u16COUNT_PAUSE:16;
  uint32_t                         :16;
} stc_ETH_PAUSE_FRAMES_RXED_field_t;

typedef union un_ETH_PAUSE_FRAMES_RXED {
  uint32_t                         u32Register;
  stc_ETH_PAUSE_FRAMES_RXED_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_PAUSE_FRAMES_RXED_t;

/**
  * \brief 64 Byte Frames Received (ETH_FRAMES_RXED_64)
  */
typedef struct stc_ETH_FRAMES_RXED_64_field {
  uint32_t                         u32COUNT_64:32;
} stc_ETH_FRAMES_RXED_64_field_t;

typedef union un_ETH_FRAMES_RXED_64 {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_RXED_64_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_RXED_64_t;

/**
  * \brief 65 to 127 Byte Frames Received (ETH_FRAMES_RXED_65)
  */
typedef struct stc_ETH_FRAMES_RXED_65_field {
  uint32_t                         u32COUNT_65:32;
} stc_ETH_FRAMES_RXED_65_field_t;

typedef union un_ETH_FRAMES_RXED_65 {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_RXED_65_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_RXED_65_t;

/**
  * \brief 128 to 255 Byte Frames Received (ETH_FRAMES_RXED_128)
  */
typedef struct stc_ETH_FRAMES_RXED_128_field {
  uint32_t                         u32COUNT_128:32;
} stc_ETH_FRAMES_RXED_128_field_t;

typedef union un_ETH_FRAMES_RXED_128 {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_RXED_128_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_RXED_128_t;

/**
  * \brief 256 to 511 Byte Frames Received (ETH_FRAMES_RXED_256)
  */
typedef struct stc_ETH_FRAMES_RXED_256_field {
  uint32_t                         u32COUNT_256:32;
} stc_ETH_FRAMES_RXED_256_field_t;

typedef union un_ETH_FRAMES_RXED_256 {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_RXED_256_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_RXED_256_t;

/**
  * \brief 512 to 1023 Byte Frames Received (ETH_FRAMES_RXED_512)
  */
typedef struct stc_ETH_FRAMES_RXED_512_field {
  uint32_t                         u32COUNT_512:32;
} stc_ETH_FRAMES_RXED_512_field_t;

typedef union un_ETH_FRAMES_RXED_512 {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_RXED_512_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_RXED_512_t;

/**
  * \brief 1024 to 1518 Byte Frames Received (ETH_FRAMES_RXED_1024)
  */
typedef struct stc_ETH_FRAMES_RXED_1024_field {
  uint32_t                         u32COUNT_1024:32;
} stc_ETH_FRAMES_RXED_1024_field_t;

typedef union un_ETH_FRAMES_RXED_1024 {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_RXED_1024_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_RXED_1024_t;

/**
  * \brief 1519 to maximum Byte Frames Received (ETH_FRAMES_RXED_1519)
  */
typedef struct stc_ETH_FRAMES_RXED_1519_field {
  uint32_t                         u32COUNT_1519:32;
} stc_ETH_FRAMES_RXED_1519_field_t;

typedef union un_ETH_FRAMES_RXED_1519 {
  uint32_t                         u32Register;
  stc_ETH_FRAMES_RXED_1519_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FRAMES_RXED_1519_t;

/**
  * \brief Undersized Frames Received (ETH_UNDERSIZE_FRAMES)
  */
typedef struct stc_ETH_UNDERSIZE_FRAMES_field {
  uint32_t                         u10COUNT_UNDERSIZE:10;
  uint32_t                         :22;
} stc_ETH_UNDERSIZE_FRAMES_field_t;

typedef union un_ETH_UNDERSIZE_FRAMES {
  uint32_t                         u32Register;
  stc_ETH_UNDERSIZE_FRAMES_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_UNDERSIZE_FRAMES_t;

/**
  * \brief Oversize Frames Received (ETH_EXCESSIVE_RX_LENGTH)
  */
typedef struct stc_ETH_EXCESSIVE_RX_LENGTH_field {
  uint32_t                         u10COUNT_OVERSIZE:10;
  uint32_t                         :22;
} stc_ETH_EXCESSIVE_RX_LENGTH_field_t;

typedef union un_ETH_EXCESSIVE_RX_LENGTH {
  uint32_t                         u32Register;
  stc_ETH_EXCESSIVE_RX_LENGTH_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_EXCESSIVE_RX_LENGTH_t;

/**
  * \brief Jabbers Received (ETH_RX_JABBERS)
  */
typedef struct stc_ETH_RX_JABBERS_field {
  uint32_t                         u10COUNT_JABBERS:10;
  uint32_t                         :22;
} stc_ETH_RX_JABBERS_field_t;

typedef union un_ETH_RX_JABBERS {
  uint32_t                         u32Register;
  stc_ETH_RX_JABBERS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RX_JABBERS_t;

/**
  * \brief Frame Check Sequence Errors (ETH_FCS_ERRORS)
  */
typedef struct stc_ETH_FCS_ERRORS_field {
  uint32_t                         u10COUNT_FCS_ERR:10;
  uint32_t                         :22;
} stc_ETH_FCS_ERRORS_field_t;

typedef union un_ETH_FCS_ERRORS {
  uint32_t                         u32Register;
  stc_ETH_FCS_ERRORS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_FCS_ERRORS_t;

/**
  * \brief Length Field Frame Errors (ETH_RX_LENGTH_ERRORS)
  */
typedef struct stc_ETH_RX_LENGTH_ERRORS_field {
  uint32_t                         u10COUNT_LENGTH_ERR:10;
  uint32_t                         :22;
} stc_ETH_RX_LENGTH_ERRORS_field_t;

typedef union un_ETH_RX_LENGTH_ERRORS {
  uint32_t                         u32Register;
  stc_ETH_RX_LENGTH_ERRORS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RX_LENGTH_ERRORS_t;

/**
  * \brief Receive Symbol Errors (ETH_RX_SYMBOL_ERRORS)
  */
typedef struct stc_ETH_RX_SYMBOL_ERRORS_field {
  uint32_t                         u10COUNT_SYMBOL_ERR:10;
  uint32_t                         :22;
} stc_ETH_RX_SYMBOL_ERRORS_field_t;

typedef union un_ETH_RX_SYMBOL_ERRORS {
  uint32_t                         u32Register;
  stc_ETH_RX_SYMBOL_ERRORS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RX_SYMBOL_ERRORS_t;

/**
  * \brief Alignment Errors (ETH_ALIGNMENT_ERRORS)
  */
typedef struct stc_ETH_ALIGNMENT_ERRORS_field {
  uint32_t                         u10COUNT_ALIGNMENT_ERROR:10;
  uint32_t                         :22;
} stc_ETH_ALIGNMENT_ERRORS_field_t;

typedef union un_ETH_ALIGNMENT_ERRORS {
  uint32_t                         u32Register;
  stc_ETH_ALIGNMENT_ERRORS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_ALIGNMENT_ERRORS_t;

/**
  * \brief Receive Resource Errors (ETH_RX_RESOURCE_ERRORS)
  */
typedef struct stc_ETH_RX_RESOURCE_ERRORS_field {
  uint32_t                         u18COUNT_RESOURCE_ERR:18;
  uint32_t                         :14;
} stc_ETH_RX_RESOURCE_ERRORS_field_t;

typedef union un_ETH_RX_RESOURCE_ERRORS {
  uint32_t                         u32Register;
  stc_ETH_RX_RESOURCE_ERRORS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RX_RESOURCE_ERRORS_t;

/**
  * \brief Receive Overruns (ETH_RX_OVERRUNS)
  */
typedef struct stc_ETH_RX_OVERRUNS_field {
  uint32_t                         u10COUNT_OVERRUN:10;
  uint32_t                         :22;
} stc_ETH_RX_OVERRUNS_field_t;

typedef union un_ETH_RX_OVERRUNS {
  uint32_t                         u32Register;
  stc_ETH_RX_OVERRUNS_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RX_OVERRUNS_t;

/**
  * \brief IP Header Checksum Errors (ETH_RX_IP_CK_ERRORS)
  */
typedef struct stc_ETH_RX_IP_CK_ERRORS_field {
  uint32_t                         u8COUNT_IPCK_ERR:8;
  uint32_t                         :24;
} stc_ETH_RX_IP_CK_ERRORS_field_t;

typedef union un_ETH_RX_IP_CK_ERRORS {
  uint32_t                         u32Register;
  stc_ETH_RX_IP_CK_ERRORS_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RX_IP_CK_ERRORS_t;

/**
  * \brief TCP Checksum Errors (ETH_RX_TCP_CK_ERRORS)
  */
typedef struct stc_ETH_RX_TCP_CK_ERRORS_field {
  uint32_t                         u8COUNT_TCPCK_ERR:8;
  uint32_t                         :24;
} stc_ETH_RX_TCP_CK_ERRORS_field_t;

typedef union un_ETH_RX_TCP_CK_ERRORS {
  uint32_t                         u32Register;
  stc_ETH_RX_TCP_CK_ERRORS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RX_TCP_CK_ERRORS_t;

/**
  * \brief UDP Checksum Errors (ETH_RX_UDP_CK_ERRORS)
  */
typedef struct stc_ETH_RX_UDP_CK_ERRORS_field {
  uint32_t                         u8COUNT_UDPCK_ERR:8;
  uint32_t                         :24;
} stc_ETH_RX_UDP_CK_ERRORS_field_t;

typedef union un_ETH_RX_UDP_CK_ERRORS {
  uint32_t                         u32Register;
  stc_ETH_RX_UDP_CK_ERRORS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RX_UDP_CK_ERRORS_t;

/**
  * \brief Receive DMA Flushed Packets (ETH_AUTO_FLUSHED_PKTS)
  */
typedef struct stc_ETH_AUTO_FLUSHED_PKTS_field {
  uint32_t                         u16COUNT_FLUSHED:16;
  uint32_t                         :16;
} stc_ETH_AUTO_FLUSHED_PKTS_field_t;

typedef union un_ETH_AUTO_FLUSHED_PKTS {
  uint32_t                         u32Register;
  stc_ETH_AUTO_FLUSHED_PKTS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_AUTO_FLUSHED_PKTS_t;

/**
  * \brief 1588 Timer Increment Register sub nsec (ETH_TSU_TIMER_INCR_SUB_NSEC)
  */
typedef struct stc_ETH_TSU_TIMER_INCR_SUB_NSEC_field {
  uint32_t                         u16SUB_NS_INCR:16;
  uint32_t                         :8;
  uint32_t                         u8SUB_NS_INCR_LSB:8;
} stc_ETH_TSU_TIMER_INCR_SUB_NSEC_field_t;

typedef union un_ETH_TSU_TIMER_INCR_SUB_NSEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_TIMER_INCR_SUB_NSEC_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_TIMER_INCR_SUB_NSEC_t;

/**
  * \brief 1588 Timer Seconds Register (47 to 32 bits) (ETH_TSU_TIMER_MSB_SEC)
  */
typedef struct stc_ETH_TSU_TIMER_MSB_SEC_field {
  uint32_t                         u16TIMER_MSB_SEC:16;
  uint32_t                         :16;
} stc_ETH_TSU_TIMER_MSB_SEC_field_t;

typedef union un_ETH_TSU_TIMER_MSB_SEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_TIMER_MSB_SEC_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_TIMER_MSB_SEC_t;

/**
  * \brief 1588 Timer Sync Strobe Seconds Register (47 to 32 bits) (ETH_TSU_STROBE_MSB_SEC)
  */
typedef struct stc_ETH_TSU_STROBE_MSB_SEC_field {
  uint32_t                         u16STROBE_MSB_SEC:16;
  uint32_t                         :16;
} stc_ETH_TSU_STROBE_MSB_SEC_field_t;

typedef union un_ETH_TSU_STROBE_MSB_SEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_STROBE_MSB_SEC_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_STROBE_MSB_SEC_t;

/**
  * \brief 1588 Timer Sync Strobe Seconds Register (31 to 0 bits) (ETH_TSU_STROBE_SEC)
  */
typedef struct stc_ETH_TSU_STROBE_SEC_field {
  uint32_t                         u32STROBE_SEC:32;
} stc_ETH_TSU_STROBE_SEC_field_t;

typedef union un_ETH_TSU_STROBE_SEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_STROBE_SEC_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_STROBE_SEC_t;

/**
  * \brief 1588 Timer Sync Strobe Nanoseconds Register (ETH_TSU_STROBE_NSEC)
  */
typedef struct stc_ETH_TSU_STROBE_NSEC_field {
  uint32_t                         u30STROBE_NSEC:30;
  uint32_t                         :2;
} stc_ETH_TSU_STROBE_NSEC_field_t;

typedef union un_ETH_TSU_STROBE_NSEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_STROBE_NSEC_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_STROBE_NSEC_t;

/**
  * \brief 1588 Timer Seconds Register (31 to 0 bits) (ETH_TSU_TIMER_SEC)
  */
typedef struct stc_ETH_TSU_TIMER_SEC_field {
  uint32_t                         u32TIMER_SEC:32;
} stc_ETH_TSU_TIMER_SEC_field_t;

typedef union un_ETH_TSU_TIMER_SEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_TIMER_SEC_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_TIMER_SEC_t;

/**
  * \brief 1588 Timer Nanoseconds Register (ETH_TSU_TIMER_NSEC)
  */
typedef struct stc_ETH_TSU_TIMER_NSEC_field {
  uint32_t                         u30TIMER_NSEC:30;
  uint32_t                         :2;
} stc_ETH_TSU_TIMER_NSEC_field_t;

typedef union un_ETH_TSU_TIMER_NSEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_TIMER_NSEC_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_TIMER_NSEC_t;

/**
  * \brief This register is used to adjust the value of the timer in the TSU. It allows an integral number of nanoseconds to be added or subtracted from the timer in a one-off operation. This register returns all zeroes when read. (ETH_TSU_TIMER_ADJUST)
  */
typedef struct stc_ETH_TSU_TIMER_ADJUST_field {
  uint32_t                         u30INCREMENT_VALUE:30;
  uint32_t                         :1;
  uint32_t                         u1ADD_SUBTRACT:1;
} stc_ETH_TSU_TIMER_ADJUST_field_t;

typedef union un_ETH_TSU_TIMER_ADJUST {
  uint32_t                         u32Register;
  stc_ETH_TSU_TIMER_ADJUST_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_TIMER_ADJUST_t;

/**
  * \brief 1588 Timer Increment Register (ETH_TSU_TIMER_INCR)
  */
typedef struct stc_ETH_TSU_TIMER_INCR_field {
  uint32_t                         u8NS_INCREMENT:8;
  uint32_t                         u8ALT_NS_INCR:8;
  uint32_t                         u8NUM_INCS:8;
  uint32_t                         :8;
} stc_ETH_TSU_TIMER_INCR_field_t;

typedef union un_ETH_TSU_TIMER_INCR {
  uint32_t                         u32Register;
  stc_ETH_TSU_TIMER_INCR_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_TIMER_INCR_t;

/**
  * \brief PTP Event Frame Transmitted Seconds Register (31 to 0 bits) (ETH_TSU_PTP_TX_SEC)
  */
typedef struct stc_ETH_TSU_PTP_TX_SEC_field {
  uint32_t                         u32TIMER_PTP_SEC:32;
} stc_ETH_TSU_PTP_TX_SEC_field_t;

typedef union un_ETH_TSU_PTP_TX_SEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_PTP_TX_SEC_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_PTP_TX_SEC_t;

/**
  * \brief PTP Event Frame Transmitted Nanoseconds Register (ETH_TSU_PTP_TX_NSEC)
  */
typedef struct stc_ETH_TSU_PTP_TX_NSEC_field {
  uint32_t                         u30TIMER_PTP_NSEC:30;
  uint32_t                         :2;
} stc_ETH_TSU_PTP_TX_NSEC_field_t;

typedef union un_ETH_TSU_PTP_TX_NSEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_PTP_TX_NSEC_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_PTP_TX_NSEC_t;

/**
  * \brief PTP Event Frame Received Seconds Register (31 to 0 bits) (ETH_TSU_PTP_RX_SEC)
  */
typedef struct stc_ETH_TSU_PTP_RX_SEC_field {
  uint32_t                         u32TIMER_PTP_SEC:32;
} stc_ETH_TSU_PTP_RX_SEC_field_t;

typedef union un_ETH_TSU_PTP_RX_SEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_PTP_RX_SEC_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_PTP_RX_SEC_t;

/**
  * \brief PTP Event Frame Received Nanoseconds Register (ETH_TSU_PTP_RX_NSEC)
  */
typedef struct stc_ETH_TSU_PTP_RX_NSEC_field {
  uint32_t                         u30TIMER_PTP_NSEC:30;
  uint32_t                         :2;
} stc_ETH_TSU_PTP_RX_NSEC_field_t;

typedef union un_ETH_TSU_PTP_RX_NSEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_PTP_RX_NSEC_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_PTP_RX_NSEC_t;

/**
  * \brief PTP Peer Event Frame Transmitted Seconds Register (31 to 0 bits) (ETH_TSU_PEER_TX_SEC)
  */
typedef struct stc_ETH_TSU_PEER_TX_SEC_field {
  uint32_t                         u32TIMER_PEER_SEC:32;
} stc_ETH_TSU_PEER_TX_SEC_field_t;

typedef union un_ETH_TSU_PEER_TX_SEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_PEER_TX_SEC_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_PEER_TX_SEC_t;

/**
  * \brief PTP Peer Event Frame Transmitted Nanoseconds Register (ETH_TSU_PEER_TX_NSEC)
  */
typedef struct stc_ETH_TSU_PEER_TX_NSEC_field {
  uint32_t                         u30TIMER_PEER_NSEC:30;
  uint32_t                         :2;
} stc_ETH_TSU_PEER_TX_NSEC_field_t;

typedef union un_ETH_TSU_PEER_TX_NSEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_PEER_TX_NSEC_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_PEER_TX_NSEC_t;

/**
  * \brief PTP Peer Event Frame Received Seconds Register (31 to 0 bits) (ETH_TSU_PEER_RX_SEC)
  */
typedef struct stc_ETH_TSU_PEER_RX_SEC_field {
  uint32_t                         u32TIMER_PEER_SEC:32;
} stc_ETH_TSU_PEER_RX_SEC_field_t;

typedef union un_ETH_TSU_PEER_RX_SEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_PEER_RX_SEC_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_PEER_RX_SEC_t;

/**
  * \brief PTP Peer Event Frame Received Nanoseconds Register (ETH_TSU_PEER_RX_NSEC)
  */
typedef struct stc_ETH_TSU_PEER_RX_NSEC_field {
  uint32_t                         u30TIMER_PEER_NSEC:30;
  uint32_t                         :2;
} stc_ETH_TSU_PEER_RX_NSEC_field_t;

typedef union un_ETH_TSU_PEER_RX_NSEC {
  uint32_t                         u32Register;
  stc_ETH_TSU_PEER_RX_NSEC_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TSU_PEER_RX_NSEC_t;

/**
  * \brief Not presents. Access to the register returns AHB error. (ETH_PCS_CONTROL)
  */
typedef struct stc_ETH_PCS_CONTROL_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_PCS_CONTROL_field_t;

typedef union un_ETH_PCS_CONTROL {
  uint32_t                         u32Register;
  stc_ETH_PCS_CONTROL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_PCS_CONTROL_t;

/**
  * \brief Not presents. Access to the register returns AHB error. (ETH_PCS_STATUS)
  */
typedef struct stc_ETH_PCS_STATUS_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_PCS_STATUS_field_t;

typedef union un_ETH_PCS_STATUS {
  uint32_t                         u32Register;
  stc_ETH_PCS_STATUS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_PCS_STATUS_t;

/**
  * \brief Not presents. Access to the register returns AHB error. (ETH_PCS_AN_ADV)
  */
typedef struct stc_ETH_PCS_AN_ADV_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_PCS_AN_ADV_field_t;

typedef union un_ETH_PCS_AN_ADV {
  uint32_t                         u32Register;
  stc_ETH_PCS_AN_ADV_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_PCS_AN_ADV_t;

/**
  * \brief Not presents. Access to the register returns AHB error. (ETH_PCS_AN_LP_BASE)
  */
typedef struct stc_ETH_PCS_AN_LP_BASE_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_PCS_AN_LP_BASE_field_t;

typedef union un_ETH_PCS_AN_LP_BASE {
  uint32_t                         u32Register;
  stc_ETH_PCS_AN_LP_BASE_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_PCS_AN_LP_BASE_t;

/**
  * \brief Not presents. Access to the register returns AHB error. (ETH_PCS_AN_EXP)
  */
typedef struct stc_ETH_PCS_AN_EXP_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_PCS_AN_EXP_field_t;

typedef union un_ETH_PCS_AN_EXP {
  uint32_t                         u32Register;
  stc_ETH_PCS_AN_EXP_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_PCS_AN_EXP_t;

/**
  * \brief Not presents. Access to the register returns AHB error. (ETH_PCS_AN_NP_TX)
  */
typedef struct stc_ETH_PCS_AN_NP_TX_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_PCS_AN_NP_TX_field_t;

typedef union un_ETH_PCS_AN_NP_TX {
  uint32_t                         u32Register;
  stc_ETH_PCS_AN_NP_TX_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_PCS_AN_NP_TX_t;

/**
  * \brief Not presents. Access to the register returns AHB error. (ETH_PCS_AN_LP_NP)
  */
typedef struct stc_ETH_PCS_AN_LP_NP_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_PCS_AN_LP_NP_field_t;

typedef union un_ETH_PCS_AN_LP_NP {
  uint32_t                         u32Register;
  stc_ETH_PCS_AN_LP_NP_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_PCS_AN_LP_NP_t;

/**
  * \brief Not presents. Access to the register returns AHB error. (ETH_PCS_AN_EXT_STATUS)
  */
typedef struct stc_ETH_PCS_AN_EXT_STATUS_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_PCS_AN_EXT_STATUS_field_t;

typedef union un_ETH_PCS_AN_EXT_STATUS {
  uint32_t                         u32Register;
  stc_ETH_PCS_AN_EXT_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_PCS_AN_EXT_STATUS_t;

/**
  * \brief Transmit Pause Quantum Register 1 (ETH_TX_PAUSE_QUANTUM1)
  */
typedef struct stc_ETH_TX_PAUSE_QUANTUM1_field {
  uint32_t                         u16QUANTUM_P2:16;
  uint32_t                         u16QUANTUM_P3:16;
} stc_ETH_TX_PAUSE_QUANTUM1_field_t;

typedef union un_ETH_TX_PAUSE_QUANTUM1 {
  uint32_t                         u32Register;
  stc_ETH_TX_PAUSE_QUANTUM1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TX_PAUSE_QUANTUM1_t;

/**
  * \brief Transmit Pause Quantum Register 2 (ETH_TX_PAUSE_QUANTUM2)
  */
typedef struct stc_ETH_TX_PAUSE_QUANTUM2_field {
  uint32_t                         u16QUANTUM_P4:16;
  uint32_t                         u16QUANTUM_P5:16;
} stc_ETH_TX_PAUSE_QUANTUM2_field_t;

typedef union un_ETH_TX_PAUSE_QUANTUM2 {
  uint32_t                         u32Register;
  stc_ETH_TX_PAUSE_QUANTUM2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TX_PAUSE_QUANTUM2_t;

/**
  * \brief Transmit Pause Quantum Register 3 (ETH_TX_PAUSE_QUANTUM3)
  */
typedef struct stc_ETH_TX_PAUSE_QUANTUM3_field {
  uint32_t                         u16QUANTUM_P6:16;
  uint32_t                         u16QUANTUM_P7:16;
} stc_ETH_TX_PAUSE_QUANTUM3_field_t;

typedef union un_ETH_TX_PAUSE_QUANTUM3 {
  uint32_t                         u32Register;
  stc_ETH_TX_PAUSE_QUANTUM3_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TX_PAUSE_QUANTUM3_t;

/**
  * \brief Received LPI transitions (ETH_RX_LPI)
  */
typedef struct stc_ETH_RX_LPI_field {
  uint32_t                         u16COUNT_LPI:16;
  uint32_t                         :16;
} stc_ETH_RX_LPI_field_t;

typedef union un_ETH_RX_LPI {
  uint32_t                         u32Register;
  stc_ETH_RX_LPI_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RX_LPI_t;

/**
  * \brief Received LPI time (ETH_RX_LPI_TIME)
  */
typedef struct stc_ETH_RX_LPI_TIME_field {
  uint32_t                         u24LPI_TIME:24;
  uint32_t                         :8;
} stc_ETH_RX_LPI_TIME_field_t;

typedef union un_ETH_RX_LPI_TIME {
  uint32_t                         u32Register;
  stc_ETH_RX_LPI_TIME_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RX_LPI_TIME_t;

/**
  * \brief Transmit LPI transitions (ETH_TX_LPI)
  */
typedef struct stc_ETH_TX_LPI_field {
  uint32_t                         u16COUNT_LPI:16;
  uint32_t                         :16;
} stc_ETH_TX_LPI_field_t;

typedef union un_ETH_TX_LPI {
  uint32_t                         u32Register;
  stc_ETH_TX_LPI_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TX_LPI_t;

/**
  * \brief Transmit LPI time (ETH_TX_LPI_TIME)
  */
typedef struct stc_ETH_TX_LPI_TIME_field {
  uint32_t                         u24LPI_TIME:24;
  uint32_t                         :8;
} stc_ETH_TX_LPI_TIME_field_t;

typedef union un_ETH_TX_LPI_TIME {
  uint32_t                         u32Register;
  stc_ETH_TX_LPI_TIME_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TX_LPI_TIME_t;

/**
  * \brief The GEM_GXL(3PIP) has many parameterisation options to configure the IP during compilation stage. This is achieved using Verilog define compiler directives in an include file called mxeth_defs.v. (ETH_DESIGNCFG_DEBUG1)
  */
typedef struct stc_ETH_DESIGNCFG_DEBUG1_field {
  uint32_t                         u1NO_PCS:1;
  uint32_t                         u1EXCLUDE_QBV:1;
  uint32_t                         u2RESERVED_2:2;
  uint32_t                         u1INT_LOOPBACK:1;
  uint32_t                         u1RESERVED_5:1;
  uint32_t                         u1EXT_FIFO_INTERFACE:1;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1RESERVED_8:1;
  uint32_t                         u1USER_IO:1;
  uint32_t                         u5USER_OUT_WIDTH:5;
  uint32_t                         u5USER_IN_WIDTH:5;
  uint32_t                         u1RESERVED_20:1;
  uint32_t                         u1NO_STATS:1;
  uint32_t                         u1NO_SNAPSHOT:1;
  uint32_t                         u1IRQ_READ_CLEAR:1;
  uint32_t                         u1EXCLUDE_CBS:1;
  uint32_t                         u3DMA_BUS_WIDTH:3;
  uint32_t                         u4AXI_CACHE_VALUE:4;
} stc_ETH_DESIGNCFG_DEBUG1_field_t;

typedef union un_ETH_DESIGNCFG_DEBUG1 {
  uint32_t                         u32Register;
  stc_ETH_DESIGNCFG_DEBUG1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DESIGNCFG_DEBUG1_t;

/**
  * \brief Design Configuration Register 2 (ETH_DESIGNCFG_DEBUG2)
  */
typedef struct stc_ETH_DESIGNCFG_DEBUG2_field {
  uint32_t                         u14JUMBO_MAX_LENGTH:14;
  uint32_t                         :2;
  uint32_t                         u4HPROT_VALUE:4;
  uint32_t                         u1RX_PKT_BUFFER:1;
  uint32_t                         u1TX_PKT_BUFFER:1;
  uint32_t                         u4RX_PBUF_ADDR:4;
  uint32_t                         u4TX_PBUF_ADDR:4;
  uint32_t                         u1AXI:1;
  uint32_t                         u1SPRAM:1;
} stc_ETH_DESIGNCFG_DEBUG2_field_t;

typedef union un_ETH_DESIGNCFG_DEBUG2 {
  uint32_t                         u32Register;
  stc_ETH_DESIGNCFG_DEBUG2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DESIGNCFG_DEBUG2_t;

/**
  * \brief Design Configuration Register 3 (ETH_DESIGNCFG_DEBUG3)
  */
typedef struct stc_ETH_DESIGNCFG_DEBUG3_field {
  uint32_t                         :24;
  uint32_t                         u6NUM_SPEC_ADD_FILTERS:6;
  uint32_t                         :2;
} stc_ETH_DESIGNCFG_DEBUG3_field_t;

typedef union un_ETH_DESIGNCFG_DEBUG3 {
  uint32_t                         u32Register;
  stc_ETH_DESIGNCFG_DEBUG3_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DESIGNCFG_DEBUG3_t;

/**
  * \brief Design Configuration Register 4 (ETH_DESIGNCFG_DEBUG4)
  */
typedef struct stc_ETH_DESIGNCFG_DEBUG4_field {
  uint32_t                         u32RESERVED_31_0:32;
} stc_ETH_DESIGNCFG_DEBUG4_field_t;

typedef union un_ETH_DESIGNCFG_DEBUG4 {
  uint32_t                         u32Register;
  stc_ETH_DESIGNCFG_DEBUG4_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DESIGNCFG_DEBUG4_t;

/**
  * \brief Design Configuration Register 5 (ETH_DESIGNCFG_DEBUG5)
  */
typedef struct stc_ETH_DESIGNCFG_DEBUG5_field {
  uint32_t                         u4RX_FIFO_CNT_WIDTH:4;
  uint32_t                         u4TX_FIFO_CNT_WIDTH:4;
  uint32_t                         u1TSU:1;
  uint32_t                         u1PHY_IDENT:1;
  uint32_t                         u2DMA_BUS_WIDTH_DEF:2;
  uint32_t                         u3MDC_CLOCK_DIV:3;
  uint32_t                         u2ENDIAN_SWAP_DEF:2;
  uint32_t                         u2RX_PBUF_SIZE_DEF:2;
  uint32_t                         u1TX_PBUF_SIZE_DEF:1;
  uint32_t                         u8RX_BUFFER_LENGTH_DEF:8;
  uint32_t                         u1TSU_CLK:1;
  uint32_t                         u3AXI_PROT_VALUE:3;
} stc_ETH_DESIGNCFG_DEBUG5_field_t;

typedef union un_ETH_DESIGNCFG_DEBUG5 {
  uint32_t                         u32Register;
  stc_ETH_DESIGNCFG_DEBUG5_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DESIGNCFG_DEBUG5_t;

/**
  * \brief Design Configuration Register 6 (ETH_DESIGNCFG_DEBUG6)
  */
typedef struct stc_ETH_DESIGNCFG_DEBUG6_field {
  uint32_t                         :1;
  uint32_t                         u1DMA_PRIORITY_QUEUE1:1;
  uint32_t                         u1DMA_PRIORITY_QUEUE2:1;
  uint32_t                         u1DMA_PRIORITY_QUEUE3:1;
  uint32_t                         u1DMA_PRIORITY_QUEUE4:1;
  uint32_t                         u1DMA_PRIORITY_QUEUE5:1;
  uint32_t                         u1DMA_PRIORITY_QUEUE6:1;
  uint32_t                         u1DMA_PRIORITY_QUEUE7:1;
  uint32_t                         u1DMA_PRIORITY_QUEUE8:1;
  uint32_t                         u1DMA_PRIORITY_QUEUE9:1;
  uint32_t                         u1DMA_PRIORITY_QUEUE10:1;
  uint32_t                         u1DMA_PRIORITY_QUEUE11:1;
  uint32_t                         u1DMA_PRIORITY_QUEUE12:1;
  uint32_t                         u1DMA_PRIORITY_QUEUE13:1;
  uint32_t                         u1DMA_PRIORITY_QUEUE14:1;
  uint32_t                         u1DMA_PRIORITY_QUEUE15:1;
  uint32_t                         u4TX_PBUF_QUEUE_SEGMENT_SIZE:4;
  uint32_t                         u1EXT_TSU_TIMER:1;
  uint32_t                         u1TX_ADD_FIFO_IF:1;
  uint32_t                         u1HOST_IF_SOFT_SELECT:1;
  uint32_t                         u1DMA_ADDR_WIDTH_IS_64B:1;
  uint32_t                         u1PFC_MULTI_QUANTUM:1;
  uint32_t                         u1PBUF_CUTTHRU:1;
  uint32_t                         u1PBUF_RSC:1;
  uint32_t                         u1PBUF_LSO:1;
  uint32_t                         :4;
} stc_ETH_DESIGNCFG_DEBUG6_field_t;

typedef union un_ETH_DESIGNCFG_DEBUG6 {
  uint32_t                         u32Register;
  stc_ETH_DESIGNCFG_DEBUG6_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DESIGNCFG_DEBUG6_t;

/**
  * \brief Design Configuration Register 7 (ETH_DESIGNCFG_DEBUG7)
  */
typedef struct stc_ETH_DESIGNCFG_DEBUG7_field {
  uint32_t                         u4X_PBUF_NUM_SEGMENTS_Q0:4;
  uint32_t                         u4X_PBUF_NUM_SEGMENTS_Q1:4;
  uint32_t                         u4X_PBUF_NUM_SEGMENTS_Q2:4;
  uint32_t                         u4X_PBUF_NUM_SEGMENTS_Q3:4;
  uint32_t                         u4X_PBUF_NUM_SEGMENTS_Q4:4;
  uint32_t                         u4X_PBUF_NUM_SEGMENTS_Q5:4;
  uint32_t                         u4X_PBUF_NUM_SEGMENTS_Q6:4;
  uint32_t                         u4X_PBUF_NUM_SEGMENTS_Q7:4;
} stc_ETH_DESIGNCFG_DEBUG7_field_t;

typedef union un_ETH_DESIGNCFG_DEBUG7 {
  uint32_t                         u32Register;
  stc_ETH_DESIGNCFG_DEBUG7_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DESIGNCFG_DEBUG7_t;

/**
  * \brief Design Configuration Register 8 (ETH_DESIGNCFG_DEBUG8)
  */
typedef struct stc_ETH_DESIGNCFG_DEBUG8_field {
  uint32_t                         u8NUM_SCR2_COMPARE_REGS:8;
  uint32_t                         u8NUM_SCR2_ETHTYPE_REGS:8;
  uint32_t                         u8NUM_TYPE2_SCREENERS:8;
  uint32_t                         u8NUM_TYPE1_SCREENERS:8;
} stc_ETH_DESIGNCFG_DEBUG8_field_t;

typedef union un_ETH_DESIGNCFG_DEBUG8 {
  uint32_t                         u32Register;
  stc_ETH_DESIGNCFG_DEBUG8_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DESIGNCFG_DEBUG8_t;

/**
  * \brief Design Configuration Register 9 (ETH_DESIGNCFG_DEBUG9)
  */
typedef struct stc_ETH_DESIGNCFG_DEBUG9_field {
  uint32_t                         u4TX_PBUF_NUM_SEGMENTS_Q8:4;
  uint32_t                         u4TX_PBUF_NUM_SEGMENTS_Q9:4;
  uint32_t                         u4TX_PBUF_NUM_SEGMENTS_Q10:4;
  uint32_t                         u4TX_PBUF_NUM_SEGMENTS_Q11:4;
  uint32_t                         u4TX_PBUF_NUM_SEGMENTS_Q12:4;
  uint32_t                         u4TX_PBUF_NUM_SEGMENTS_Q13:4;
  uint32_t                         u4TX_PBUF_NUM_SEGMENTS_Q14:4;
  uint32_t                         u4TX_PBUF_NUM_SEGMENTS_Q15:4;
} stc_ETH_DESIGNCFG_DEBUG9_field_t;

typedef union un_ETH_DESIGNCFG_DEBUG9 {
  uint32_t                         u32Register;
  stc_ETH_DESIGNCFG_DEBUG9_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DESIGNCFG_DEBUG9_t;

/**
  * \brief Design Configuration Register 10 (ETH_DESIGNCFG_DEBUG10)
  */
typedef struct stc_ETH_DESIGNCFG_DEBUG10_field {
  uint32_t                         u4AXI_RX_DESCR_WR_BUFF_BITS:4;
  uint32_t                         u4AXI_TX_DESCR_WR_BUFF_BITS:4;
  uint32_t                         u4AXI_RX_DESCR_RD_BUFF_BITS:4;
  uint32_t                         u4AXI_TX_DESCR_RD_BUFF_BITS:4;
  uint32_t                         u4AXI_ACCESS_PIPELINE_BITS:4;
  uint32_t                         u4RX_PBUF_DATA:4;
  uint32_t                         u4TX_PBUF_DATA:4;
  uint32_t                         u4EMAC_BUS_WIDTH:4;
} stc_ETH_DESIGNCFG_DEBUG10_field_t;

typedef union un_ETH_DESIGNCFG_DEBUG10 {
  uint32_t                         u32Register;
  stc_ETH_DESIGNCFG_DEBUG10_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DESIGNCFG_DEBUG10_t;

/**
  * \brief Specific address registers 5 ~ 36 doesn't present. Access to the register returns AHB error. (ETH_SPEC_ADD5_BOTTOM)
  */
typedef struct stc_ETH_SPEC_ADD5_BOTTOM_field {
  uint32_t                         u32RESERVED_31_0:32;
} stc_ETH_SPEC_ADD5_BOTTOM_field_t;

typedef union un_ETH_SPEC_ADD5_BOTTOM {
  uint32_t                         u32Register;
  stc_ETH_SPEC_ADD5_BOTTOM_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_ADD5_BOTTOM_t;

/**
  * \brief Specific address registers 5 ~ 36 doesn't present. Access to the register returns AHB error. (ETH_SPEC_ADD5_TOP)
  */
typedef struct stc_ETH_SPEC_ADD5_TOP_field {
  uint32_t                         u32RESERVED_31_0:32;
} stc_ETH_SPEC_ADD5_TOP_field_t;

typedef union un_ETH_SPEC_ADD5_TOP {
  uint32_t                         u32Register;
  stc_ETH_SPEC_ADD5_TOP_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_ADD5_TOP_t;

/**
  * \brief Not presents. (ETH_SPEC_ADD36_BOTTOM)
  */
typedef struct stc_ETH_SPEC_ADD36_BOTTOM_field {
  uint32_t                         u32RESERVED_31_0:32;
} stc_ETH_SPEC_ADD36_BOTTOM_field_t;

typedef union un_ETH_SPEC_ADD36_BOTTOM {
  uint32_t                         u32Register;
  stc_ETH_SPEC_ADD36_BOTTOM_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_ADD36_BOTTOM_t;

/**
  * \brief Not presents. (ETH_SPEC_ADD36_TOP)
  */
typedef struct stc_ETH_SPEC_ADD36_TOP_field {
  uint32_t                         u32RESERVED_31_0:32;
} stc_ETH_SPEC_ADD36_TOP_field_t;

typedef union un_ETH_SPEC_ADD36_TOP {
  uint32_t                         u32Register;
  stc_ETH_SPEC_ADD36_TOP_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SPEC_ADD36_TOP_t;

/**
  * \brief Priority queue Interrupt Status Register (ETH_INT_Q1_STATUS)
  */
typedef struct stc_ETH_INT_Q1_STATUS_field {
  uint32_t                         :1;
  uint32_t                         u1RECEIVE_COMPLETE:1;
  uint32_t                         u1RX_USED_BIT_READ:1;
  uint32_t                         :2;
  uint32_t                         u1RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION:1;
  uint32_t                         u1AMBA_ERROR:1;
  uint32_t                         u1TRANSMIT_COMPLETE:1;
  uint32_t                         :3;
  uint32_t                         u1RESP_NOT_OK:1;
  uint32_t                         :20;
} stc_ETH_INT_Q1_STATUS_field_t;

typedef union un_ETH_INT_Q1_STATUS {
  uint32_t                         u32Register;
  stc_ETH_INT_Q1_STATUS_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q1_STATUS_t;

/**
  * \brief Priority queue Interrupt Status Register (ETH_INT_Q2_STATUS)
  */
typedef struct stc_ETH_INT_Q2_STATUS_field {
  uint32_t                         :1;
  uint32_t                         u1RECEIVE_COMPLETE:1;
  uint32_t                         u1RX_USED_BIT_READ:1;
  uint32_t                         :2;
  uint32_t                         u1RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION:1;
  uint32_t                         u1AMBA_ERROR:1;
  uint32_t                         u1TRANSMIT_COMPLETE:1;
  uint32_t                         :3;
  uint32_t                         u1RESP_NOT_OK:1;
  uint32_t                         :20;
} stc_ETH_INT_Q2_STATUS_field_t;

typedef union un_ETH_INT_Q2_STATUS {
  uint32_t                         u32Register;
  stc_ETH_INT_Q2_STATUS_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q2_STATUS_t;

/**
  * \brief int_q3_status to int_q15_status doesn't present.  Access to the register returns AHB error. (ETH_INT_Q3_STATUS)
  */
typedef struct stc_ETH_INT_Q3_STATUS_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_INT_Q3_STATUS_field_t;

typedef union un_ETH_INT_Q3_STATUS {
  uint32_t                         u32Register;
  stc_ETH_INT_Q3_STATUS_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q3_STATUS_t;

/**
  * \brief Not presents. (ETH_INT_Q15_STATUS)
  */
typedef struct stc_ETH_INT_Q15_STATUS_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_INT_Q15_STATUS_field_t;

typedef union un_ETH_INT_Q15_STATUS {
  uint32_t                         u32Register;
  stc_ETH_INT_Q15_STATUS_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q15_STATUS_t;

/**
  * \brief This register holds the start address of the transmit buffer queue (transmit buffers descriptor list). The transmit buffer queue base address register must be initialized before transmit is started through bit 9 of the network control register. Once transmission has started, any write to the transmit buffer queue base address register is illegal and therefore ignored. Note that due to clock boundary synchronization, it takes a maximum of four pclk cycles from the writing of the transmit start bit before the transmitter is active. Writing to the transmit buffer queue base address register during this time may produce unpredictable results. Reading this register returns the location of the descriptor currently being accessed. Because the DMA can store data for multiple frames at once, this may not necessarily be pointing to the current frame being transmitted. In terms of AMBA AXI operation, the transmit descriptors are written to memory using a single 32bit AHB access. When the datapath is configured as 64bit , the transmit descriptors should be aligned at 64-bit boundaries and each pair of 32-bit descriptors is read from memory using a single AXI access. (ETH_TRANSMIT_Q1_PTR)
  */
typedef struct stc_ETH_TRANSMIT_Q1_PTR_field {
  uint32_t                         u1DMA_TX_DIS_Q:1;
  uint32_t                         :1;
  uint32_t                         u30DMA_TX_Q_PTR:30;
} stc_ETH_TRANSMIT_Q1_PTR_field_t;

typedef union un_ETH_TRANSMIT_Q1_PTR {
  uint32_t                         u32Register;
  stc_ETH_TRANSMIT_Q1_PTR_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TRANSMIT_Q1_PTR_t;

/**
  * \brief This register holds the start address of the transmit buffer queue (transmit buffers descriptor list). The transmit buffer queue base address register must be initialized before transmit is started through bit 9 of the network control register. Once transmission has started, any write to the transmit buffer queue base address register is illegal and therefore ignored. Note that due to clock boundary synchronization, it takes a maximum of four pclk cycles from the writing of the transmit start bit before the transmitter is active. Writing to the transmit buffer queue base address register during this time may produce unpredictable results. Reading this register returns the location of the descriptor currently being accessed. Because the DMA can store data for multiple frames at once, this may not necessarily be pointing to the current frame being transmitted. In terms of AMBA AXI operation, the transmit descriptors are written to memory using a single 32bit AHB access. When the datapath is configured as 64bit , the transmit descriptors should be aligned at 64-bit boundaries and each pair of 32-bit descriptors is read from memory using a single AXI access. (ETH_TRANSMIT_Q2_PTR)
  */
typedef struct stc_ETH_TRANSMIT_Q2_PTR_field {
  uint32_t                         u1DMA_TX_DIS_Q:1;
  uint32_t                         :1;
  uint32_t                         u30DMA_TX_Q_PTR:30;
} stc_ETH_TRANSMIT_Q2_PTR_field_t;

typedef union un_ETH_TRANSMIT_Q2_PTR {
  uint32_t                         u32Register;
  stc_ETH_TRANSMIT_Q2_PTR_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TRANSMIT_Q2_PTR_t;

/**
  * \brief transmit_q3_ptr to transmit_q15_ptr doesn't present.  Access to the register returns AHB error. (ETH_TRANSMIT_Q3_PTR)
  */
typedef struct stc_ETH_TRANSMIT_Q3_PTR_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_TRANSMIT_Q3_PTR_field_t;

typedef union un_ETH_TRANSMIT_Q3_PTR {
  uint32_t                         u32Register;
  stc_ETH_TRANSMIT_Q3_PTR_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TRANSMIT_Q3_PTR_t;

/**
  * \brief Not presents. (ETH_TRANSMIT_Q15_PTR)
  */
typedef struct stc_ETH_TRANSMIT_Q15_PTR_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_TRANSMIT_Q15_PTR_field_t;

typedef union un_ETH_TRANSMIT_Q15_PTR {
  uint32_t                         u32Register;
  stc_ETH_TRANSMIT_Q15_PTR_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TRANSMIT_Q15_PTR_t;

/**
  * \brief This register holds the start address of the transmit buffer queue (transmit buffers descriptor list). The transmit buffer queue base address register must be initialized before transmit is started through bit 9 of the network control register. Once transmission has started, any write to the transmit buffer queue base address register is illegal and therefore ignored. Note that due to clock boundary synchronization, it takes a maximum of four pclk cycles from the writing of the transmit start bit before the transmitter is active. Writing to the transmit buffer queue base address register during this time may produce unpredictable results. Reading this register returns the location of the descriptor currently being accessed. Because the DMA can store data for multiple frames at once, this may not necessarily be pointing to the current frame being transmitted. In terms of AMBA AXI operation, the transmit descriptors are written to memory using a single 32bit AHB access. When the datapath is configured as 64bit , the transmit descriptors should be aligned at 64-bit boundaries and each pair of 32-bit descriptors is read from memory using a single AXI access. (ETH_RECEIVE_Q1_PTR)
  */
typedef struct stc_ETH_RECEIVE_Q1_PTR_field {
  uint32_t                         u1DMA_RX_DIS_Q:1;
  uint32_t                         :1;
  uint32_t                         u30DMA_RX_Q_PTR:30;
} stc_ETH_RECEIVE_Q1_PTR_field_t;

typedef union un_ETH_RECEIVE_Q1_PTR {
  uint32_t                         u32Register;
  stc_ETH_RECEIVE_Q1_PTR_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RECEIVE_Q1_PTR_t;

/**
  * \brief This register holds the start address of the transmit buffer queue (transmit buffers descriptor list). The transmit buffer queue base address register must be initialized before transmit is started through bit 9 of the network control register. Once transmission has started, any write to the transmit buffer queue base address register is illegal and therefore ignored. Note that due to clock boundary synchronization, it takes a maximum of four pclk cycles from the writing of the transmit start bit before the transmitter is active. Writing to the transmit buffer queue base address register during this time may produce unpredictable results. Reading this register returns the location of the descriptor currently being accessed. Because the DMA can store data for multiple frames at once, this may not necessarily be pointing to the current frame being transmitted. In terms of AMBA AXI operation, the transmit descriptors are written to memory using a single 32bit AHB access. When the datapath is configured as 64bit , the transmit descriptors should be aligned at 64-bit boundaries and each pair of 32-bit descriptors is read from memory using a single AXI access. (ETH_RECEIVE_Q2_PTR)
  */
typedef struct stc_ETH_RECEIVE_Q2_PTR_field {
  uint32_t                         u1DMA_RX_DIS_Q:1;
  uint32_t                         :1;
  uint32_t                         u30DMA_RX_Q_PTR:30;
} stc_ETH_RECEIVE_Q2_PTR_field_t;

typedef union un_ETH_RECEIVE_Q2_PTR {
  uint32_t                         u32Register;
  stc_ETH_RECEIVE_Q2_PTR_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RECEIVE_Q2_PTR_t;

/**
  * \brief Not presents.  Start address register doesn't present for queue3 ~ queue7. (ETH_RECEIVE_Q3_PTR)
  */
typedef struct stc_ETH_RECEIVE_Q3_PTR_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_RECEIVE_Q3_PTR_field_t;

typedef union un_ETH_RECEIVE_Q3_PTR {
  uint32_t                         u32Register;
  stc_ETH_RECEIVE_Q3_PTR_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RECEIVE_Q3_PTR_t;

/**
  * \brief Not presents. (ETH_RECEIVE_Q7_PTR)
  */
typedef struct stc_ETH_RECEIVE_Q7_PTR_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_RECEIVE_Q7_PTR_field_t;

typedef union un_ETH_RECEIVE_Q7_PTR {
  uint32_t                         u32Register;
  stc_ETH_RECEIVE_Q7_PTR_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RECEIVE_Q7_PTR_t;

/**
  * \brief Receive Buffer queue 1 Size (ETH_DMA_RXBUF_SIZE_Q1)
  */
typedef struct stc_ETH_DMA_RXBUF_SIZE_Q1_field {
  uint32_t                         u8DMA_RX_Q_BUF_SIZE:8;
  uint32_t                         :24;
} stc_ETH_DMA_RXBUF_SIZE_Q1_field_t;

typedef union un_ETH_DMA_RXBUF_SIZE_Q1 {
  uint32_t                         u32Register;
  stc_ETH_DMA_RXBUF_SIZE_Q1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DMA_RXBUF_SIZE_Q1_t;

/**
  * \brief Receive Buffer queue 2 Size (ETH_DMA_RXBUF_SIZE_Q2)
  */
typedef struct stc_ETH_DMA_RXBUF_SIZE_Q2_field {
  uint32_t                         u8DMA_RX_Q_BUF_SIZE:8;
  uint32_t                         :24;
} stc_ETH_DMA_RXBUF_SIZE_Q2_field_t;

typedef union un_ETH_DMA_RXBUF_SIZE_Q2 {
  uint32_t                         u32Register;
  stc_ETH_DMA_RXBUF_SIZE_Q2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DMA_RXBUF_SIZE_Q2_t;

/**
  * \brief dma_rxbuf_size_q3 to dma_rxbuf_size_q7 doesn't present. (ETH_DMA_RXBUF_SIZE_Q3)
  */
typedef struct stc_ETH_DMA_RXBUF_SIZE_Q3_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_DMA_RXBUF_SIZE_Q3_field_t;

typedef union un_ETH_DMA_RXBUF_SIZE_Q3 {
  uint32_t                         u32Register;
  stc_ETH_DMA_RXBUF_SIZE_Q3_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DMA_RXBUF_SIZE_Q3_t;

/**
  * \brief Not presents. (ETH_DMA_RXBUF_SIZE_Q7)
  */
typedef struct stc_ETH_DMA_RXBUF_SIZE_Q7_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_DMA_RXBUF_SIZE_Q7_field_t;

typedef union un_ETH_DMA_RXBUF_SIZE_Q7 {
  uint32_t                         u32Register;
  stc_ETH_DMA_RXBUF_SIZE_Q7_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DMA_RXBUF_SIZE_Q7_t;

/**
  * \brief The IdleSlope value is defined as the rate of change of credit when a packet is waiting to be sent. This must not exceed the portTransmitRate which is dependent on the speed of operation, eg, portTranmsitRate. 1Gb/s = 32'h07735940 (125 Mbytes/s), 100Mb/sec = 32'h017D7840 (25 Mnibbles/s), 10Mb/sec = 32'h002625A0 (2.5 Mnibbles/s). If 50 percent of bandwidth was to be allocated to a particular queue in 1Gb/sec mode then the IdleSlope value for that queue would be calculated as 32'h07735940/2. Note that Credit-Based Shaping should be disabled prior to updating the IdleSlope values. As another example, for a 1722 audio packet with a payload of 6 samples per channel, the packet size would be 7 (preamble) + 1 (SFD) + 50 (packet header) + 6x4x2(payload) + 4 (CRC) = 110 bytes. For a rate of 8000 packets per second, the desired rate would 110 x 8000 bytes per second, so the programmed idleSlope value would be 880000 for a 1Gb/s connection, or 1760000 for a 100Mb/s or 10Mbs connection. See Figure 6.3 in the IEEE 1722 standard. In practice, the actual transmission rate will be vary slightly from that calculated. In this case, the idleSlope value should be recalibrated based on the variance between the measured and expected rate, and in this case very accurate transmission rates can be achieved. (ETH_CBS_CONTROL)
  */
typedef struct stc_ETH_CBS_CONTROL_field {
  uint32_t                         u1CBS_ENABLE_QUEUE_A:1;
  uint32_t                         u1CBS_ENABLE_QUEUE_B:1;
  uint32_t                         :30;
} stc_ETH_CBS_CONTROL_field_t;

typedef union un_ETH_CBS_CONTROL {
  uint32_t                         u32Register;
  stc_ETH_CBS_CONTROL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_CBS_CONTROL_t;

/**
  * \brief queue A is the highest priority queue. This would be queue 8 in an 8 queue configuration. (ETH_CBS_IDLESLOPE_Q_A)
  */
typedef struct stc_ETH_CBS_IDLESLOPE_Q_A_field {
  uint32_t                         u32IDLESLOPE_A:32;
} stc_ETH_CBS_IDLESLOPE_Q_A_field_t;

typedef union un_ETH_CBS_IDLESLOPE_Q_A {
  uint32_t                         u32Register;
  stc_ETH_CBS_IDLESLOPE_Q_A_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_CBS_IDLESLOPE_Q_A_t;

/**
  * \brief queue B is the 2nd highest priority queue. This would be queue 7 in an 8 queue configuration. (ETH_CBS_IDLESLOPE_Q_B)
  */
typedef struct stc_ETH_CBS_IDLESLOPE_Q_B_field {
  uint32_t                         u32IDLESLOPE_B:32;
} stc_ETH_CBS_IDLESLOPE_Q_B_field_t;

typedef union un_ETH_CBS_IDLESLOPE_Q_B {
  uint32_t                         u32Register;
  stc_ETH_CBS_IDLESLOPE_Q_B_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_CBS_IDLESLOPE_Q_B_t;

/**
  * \brief Upper 32 bits of transmit buffer descriptor queue base address. (ETH_UPPER_TX_Q_BASE_ADDR)
  */
typedef struct stc_ETH_UPPER_TX_Q_BASE_ADDR_field {
  uint32_t                         u32UPPER_TX_Q_BASE_ADDR:32;
} stc_ETH_UPPER_TX_Q_BASE_ADDR_field_t;

typedef union un_ETH_UPPER_TX_Q_BASE_ADDR {
  uint32_t                         u32Register;
  stc_ETH_UPPER_TX_Q_BASE_ADDR_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_UPPER_TX_Q_BASE_ADDR_t;

/**
  * \brief TX BD control register (ETH_TX_BD_CONTROL)
  */
typedef struct stc_ETH_TX_BD_CONTROL_field {
  uint32_t                         :4;
  uint32_t                         u2TX_BD_TS_MODE:2;
  uint32_t                         :26;
} stc_ETH_TX_BD_CONTROL_field_t;

typedef union un_ETH_TX_BD_CONTROL {
  uint32_t                         u32Register;
  stc_ETH_TX_BD_CONTROL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TX_BD_CONTROL_t;

/**
  * \brief RX BD control register (ETH_RX_BD_CONTROL)
  */
typedef struct stc_ETH_RX_BD_CONTROL_field {
  uint32_t                         :4;
  uint32_t                         u2RX_BD_TS_MODE:2;
  uint32_t                         :26;
} stc_ETH_RX_BD_CONTROL_field_t;

typedef union un_ETH_RX_BD_CONTROL {
  uint32_t                         u32Register;
  stc_ETH_RX_BD_CONTROL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RX_BD_CONTROL_t;

/**
  * \brief Upper 32 bits of receive buffer descriptor queue base address. (ETH_UPPER_RX_Q_BASE_ADDR)
  */
typedef struct stc_ETH_UPPER_RX_Q_BASE_ADDR_field {
  uint32_t                         u32UPPER_RX_Q_BASE_ADDR:32;
} stc_ETH_UPPER_RX_Q_BASE_ADDR_field_t;

typedef union un_ETH_UPPER_RX_Q_BASE_ADDR {
  uint32_t                         u32Register;
  stc_ETH_UPPER_RX_Q_BASE_ADDR_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_UPPER_RX_Q_BASE_ADDR_t;

/**
  * \brief Hidden registers defined in edma_defs.v '`define gem_cbs_port_tx_rate_10m   12'h4e0   // 10M Port TX Rate *** HIDDEN Register ***'. Default value of cbs related hidden registers (0x14E0~0x14E8) are depicted in cbs_control register. (ETH_HIDDEN_REG0)
  */
typedef struct stc_ETH_HIDDEN_REG0_field {
  uint32_t                         u32HIDDEN0_FIELD:32;
} stc_ETH_HIDDEN_REG0_field_t;

typedef union un_ETH_HIDDEN_REG0 {
  uint32_t                         u32Register;
  stc_ETH_HIDDEN_REG0_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_HIDDEN_REG0_t;

/**
  * \brief Hidden registers defined in edma_defs.v '`define gem_cbs_port_tx_rate_100m  12'h4e4   // 100M Port TX Rate *** HIDDEN Register ***' (ETH_HIDDEN_REG1)
  */
typedef struct stc_ETH_HIDDEN_REG1_field {
  uint32_t                         u32HIDDEN1_FIELD:32;
} stc_ETH_HIDDEN_REG1_field_t;

typedef union un_ETH_HIDDEN_REG1 {
  uint32_t                         u32Register;
  stc_ETH_HIDDEN_REG1_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_HIDDEN_REG1_t;

/**
  * \brief Hidden registers defined in edma_defs.v '`define gem_cbs_port_tx_rate_1g    12'h4e8       // 1G Port TX Rate *** HIDDEN Register ***' (ETH_HIDDEN_REG2)
  */
typedef struct stc_ETH_HIDDEN_REG2_field {
  uint32_t                         u32HIDDEN2_FIELD:32;
} stc_ETH_HIDDEN_REG2_field_t;

typedef union un_ETH_HIDDEN_REG2 {
  uint32_t                         u32Register;
  stc_ETH_HIDDEN_REG2_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_HIDDEN_REG2_t;

/**
  * \brief Hidden registers defined in edma_defs.v '`define gem_wd_counter 12'h4ec    // *** HIDDEN Register ***'. (ETH_HIDDEN_REG3)
  */
typedef struct stc_ETH_HIDDEN_REG3_field {
  uint32_t                         u4HIDDEN3_FIELD:4;
  uint32_t                         :28;
} stc_ETH_HIDDEN_REG3_field_t;

typedef union un_ETH_HIDDEN_REG3 {
  uint32_t                         u32Register;
  stc_ETH_HIDDEN_REG3_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_HIDDEN_REG3_t;

/**
  * \brief Hidden registers defined in edma_defs.v '`define gem_axi_tx_full_threshold0 12'h4f8   // AXI full threshold setting *** HIDDEN Register ***'. Note. When using AXI mode with a single port ram ( gem_spram == 1) mode and a 32b dma bus width ( gem_dma_bus_width == 32 or bits 22 to 21 of the network_config register are set to 0) the AXI hidden registers (0x14F8 and 0x14FC) need to be updated (these registers are used for fine tuning AXI dma bursts and normally should not be touched). (ETH_HIDDEN_REG4)
  */
typedef struct stc_ETH_HIDDEN_REG4_field {
  uint32_t                         u9HIDDEN4_FIELD_L:9;
  uint32_t                         :7;
  uint32_t                         u9HIDDEN4_FIELD_H:9;
  uint32_t                         :7;
} stc_ETH_HIDDEN_REG4_field_t;

typedef union un_ETH_HIDDEN_REG4 {
  uint32_t                         u32Register;
  stc_ETH_HIDDEN_REG4_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_HIDDEN_REG4_t;

/**
  * \brief Hidden registers defined in edma_defs.v '`define gem_axi_tx_full_threshold1 12'h4fc   // AXI full threshold setting *** HIDDEN Register ***'. (ETH_HIDDEN_REG5)
  */
typedef struct stc_ETH_HIDDEN_REG5_field {
  uint32_t                         u9HIDDEN5_FIELD_L:9;
  uint32_t                         :7;
  uint32_t                         u9HIDDEN5_FIELD_H:9;
  uint32_t                         :7;
} stc_ETH_HIDDEN_REG5_field_t;

typedef union un_ETH_HIDDEN_REG5 {
  uint32_t                         u32Register;
  stc_ETH_HIDDEN_REG5_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_HIDDEN_REG5_t;

/**
  * \brief Screening type 1 registers are used to allocate up to 16 priority queues to received frames based on certain IP or UDP fields of incoming frames. Firstly, when DS/TC match enable is set (bit 28), the DS (Differentiated Services) field of the received IPv4 header or TCfield (traffic class) of IPv6 headers are matched against bits 11 to 4. Secondly, when UDP port match enable is set (bit 29), the UDP Destination Port of the received UDP frame is matched against bits 27 to 12. Both UDP and DS/TC matching can be enabled simultaneously or individually. If a match is successful, then the queue value programmed in bits 2 to 0 is allocated to the frame. The required number of Type 1 screening registers is configured in the gem defines file. Up to 16 type 1 screening registers have been allocated APB address space between 0x500 and 0x53C. (ETH_SCREENING_TYPE_1_REGISTER_0)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_0_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_0_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_0 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_0_t;

/**
  * \brief screening type 1 register 1, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_1)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_1_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_1_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_1 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_1_t;

/**
  * \brief screening type 1 register 2, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_2)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_2_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_2_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_2 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_2_t;

/**
  * \brief screening type 1 register 3, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_3)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_3_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_3_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_3 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_3_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_3_t;

/**
  * \brief screening type 1 register 4, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_4)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_4_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_4_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_4 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_4_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_4_t;

/**
  * \brief screening type 1 register 5, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_5)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_5_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_5_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_5 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_5_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_5_t;

/**
  * \brief screening type 1 register 6, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_6)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_6_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_6_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_6 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_6_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_6_t;

/**
  * \brief screening type 1 register 7, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_7)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_7_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_7_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_7 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_7_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_7_t;

/**
  * \brief screening type 1 register 8, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_8)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_8_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_8_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_8 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_8_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_8_t;

/**
  * \brief screening type 1 register 9, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_9)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_9_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_9_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_9 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_9_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_9_t;

/**
  * \brief screening type 1 register 10, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_10)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_10_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_10_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_10 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_10_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_10_t;

/**
  * \brief screening type 1 register 11, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_11)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_11_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_11_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_11 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_11_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_11_t;

/**
  * \brief screening type 1 register 12, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_12)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_12_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_12_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_12 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_12_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_12_t;

/**
  * \brief screening type 1 register 13, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_13)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_13_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_13_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_13 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_13_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_13_t;

/**
  * \brief screening type 1 register 14, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_14)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_14_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_14_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_14 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_14_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_14_t;

/**
  * \brief screening type 1 register 15, same as screening_type_1_register_0 (ETH_SCREENING_TYPE_1_REGISTER_15)
  */
typedef struct stc_ETH_SCREENING_TYPE_1_REGISTER_15_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u8DSTC_MATCH:8;
  uint32_t                         u16UDP_PORT_MATCH:16;
  uint32_t                         u1DSTC_ENABLE:1;
  uint32_t                         u1UDP_PORT_MATCH_ENABLE:1;
  uint32_t                         :2;
} stc_ETH_SCREENING_TYPE_1_REGISTER_15_field_t;

typedef union un_ETH_SCREENING_TYPE_1_REGISTER_15 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_1_REGISTER_15_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_1_REGISTER_15_t;

/**
  * \brief Screener Type 2 match registers operate independently of screener type 1 registers and offer additional match capabilities, extending the capabilities into vendor specific protocols. (ETH_SCREENING_TYPE_2_REGISTER_0)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_0_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_0_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_0 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_0_t;

/**
  * \brief screening type 2 register 1, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_1)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_1_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_1_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_1 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_1_t;

/**
  * \brief screening type 2 register 2, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_2)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_2_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_2_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_2 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_2_t;

/**
  * \brief screening type 2 register 3, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_3)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_3_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_3_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_3 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_3_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_3_t;

/**
  * \brief screening type 2 register 4, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_4)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_4_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_4_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_4 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_4_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_4_t;

/**
  * \brief screening type 2 register 5, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_5)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_5_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_5_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_5 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_5_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_5_t;

/**
  * \brief screening type 2 register 6, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_6)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_6_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_6_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_6 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_6_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_6_t;

/**
  * \brief screening type 2 register 7, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_7)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_7_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_7_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_7 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_7_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_7_t;

/**
  * \brief screening type 2 register 8, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_8)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_8_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_8_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_8 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_8_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_8_t;

/**
  * \brief screening type 2 register 9, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_9)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_9_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_9_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_9 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_9_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_9_t;

/**
  * \brief screening type 2 register 10, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_10)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_10_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_10_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_10 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_10_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_10_t;

/**
  * \brief screening type 2 register 11, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_11)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_11_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_11_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_11 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_11_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_11_t;

/**
  * \brief screening type 2 register 12, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_12)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_12_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_12_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_12 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_12_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_12_t;

/**
  * \brief screening type 2 register 13, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_13)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_13_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_13_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_13 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_13_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_13_t;

/**
  * \brief screening type 2 register 14, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_14)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_14_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_14_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_14 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_14_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_14_t;

/**
  * \brief screening type 2 register 15, same as screening_type_2_register_0 (ETH_SCREENING_TYPE_2_REGISTER_15)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_REGISTER_15_field {
  uint32_t                         u4QUEUE_NUMBER:4;
  uint32_t                         u3VLAN_PRIORITY:3;
  uint32_t                         u1RESERVED_7:1;
  uint32_t                         u1VLAN_ENABLE:1;
  uint32_t                         u3INDEX:3;
  uint32_t                         u1ETHERTYPE_ENABLE:1;
  uint32_t                         u5COMPARE_A:5;
  uint32_t                         u1COMPARE_A_ENABLE:1;
  uint32_t                         u5COMPARE_B:5;
  uint32_t                         u1COMPARE_B_ENABLE:1;
  uint32_t                         u5COMPARE_C:5;
  uint32_t                         u1COMPARE_C_ENABLE:1;
  uint32_t                         u1RESERVED_31:1;
} stc_ETH_SCREENING_TYPE_2_REGISTER_15_field_t;

typedef union un_ETH_SCREENING_TYPE_2_REGISTER_15 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_REGISTER_15_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_REGISTER_15_t;

/**
  * \brief This register controls the transmit scheduling algorithm the user can select for each active transmit queue. By default all queues are initialized to fixed priority, with the top indexed queue having overall priority (ETH_TX_SCHED_CTRL)
  */
typedef struct stc_ETH_TX_SCHED_CTRL_field {
  uint32_t                         u2TX_SCHED_Q0:2;
  uint32_t                         u2TX_SCHED_Q1:2;
  uint32_t                         u2TX_SCHED_Q2:2;
  uint32_t                         u2TX_SCHED_Q3:2;
  uint32_t                         u24REMOVED_31_8:24;
} stc_ETH_TX_SCHED_CTRL_field_t;

typedef union un_ETH_TX_SCHED_CTRL {
  uint32_t                         u32Register;
  stc_ETH_TX_SCHED_CTRL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TX_SCHED_CTRL_t;

/**
  * \brief This register holds the DWRR weighting value or the ETS bandwidth percentage value used by the transmit scheduler for queues 0 to 3. (ETH_BW_RATE_LIMIT_Q0TO3)
  */
typedef struct stc_ETH_BW_RATE_LIMIT_Q0TO3_field {
  uint32_t                         u8DWRR_ETS_WEIGHT_Q0:8;
  uint32_t                         u8DWRR_ETS_WEIGHT_Q1:8;
  uint32_t                         u8DWRR_ETS_WEIGHT_Q2:8;
  uint32_t                         u8DWRR_ETS_WEIGHT_Q3:8;
} stc_ETH_BW_RATE_LIMIT_Q0TO3_field_t;

typedef union un_ETH_BW_RATE_LIMIT_Q0TO3 {
  uint32_t                         u32Register;
  stc_ETH_BW_RATE_LIMIT_Q0TO3_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_BW_RATE_LIMIT_Q0TO3_t;

/**
  * \brief Not presents. MXETH has only 3 queues.  Access to the register returns AHB error. (ETH_BW_RATE_LIMIT_Q4TO7)
  */
typedef struct stc_ETH_BW_RATE_LIMIT_Q4TO7_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_BW_RATE_LIMIT_Q4TO7_field_t;

typedef union un_ETH_BW_RATE_LIMIT_Q4TO7 {
  uint32_t                         u32Register;
  stc_ETH_BW_RATE_LIMIT_Q4TO7_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_BW_RATE_LIMIT_Q4TO7_t;

/**
  * \brief Not presents. MXETH has only 3 queues.  Access to the register returns AHB error. (ETH_BW_RATE_LIMIT_Q8TO11)
  */
typedef struct stc_ETH_BW_RATE_LIMIT_Q8TO11_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_BW_RATE_LIMIT_Q8TO11_field_t;

typedef union un_ETH_BW_RATE_LIMIT_Q8TO11 {
  uint32_t                         u32Register;
  stc_ETH_BW_RATE_LIMIT_Q8TO11_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_BW_RATE_LIMIT_Q8TO11_t;

/**
  * \brief Not presents. MXETH has only 3 queues.  Access to the register returns AHB error. (ETH_BW_RATE_LIMIT_Q12TO15)
  */
typedef struct stc_ETH_BW_RATE_LIMIT_Q12TO15_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_BW_RATE_LIMIT_Q12TO15_field_t;

typedef union un_ETH_BW_RATE_LIMIT_Q12TO15 {
  uint32_t                         u32Register;
  stc_ETH_BW_RATE_LIMIT_Q12TO15_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_BW_RATE_LIMIT_Q12TO15_t;

/**
  * \brief This register allows the user to distribute the Transmit SRAM used by the DMA across the priority queues, for queues 0 to 7. The SRAM itself is split into a number of evenly sized segments (this is defined in the verilog configuration defs file - for the configuration used to generate this register description, the total number of segments was set to '16'). Those segments can then be freely distributed across the active queues, in powers of 2. I.e. a value of 0 would mean 1 segment has been allocated to the queue. A value of 1 would mean 2 segments, a value of 2 means 4 segments and so on. The reset values of these registers are defined in the configuration defs file. (ETH_TX_Q_SEG_ALLOC_Q0TO7)
  */
typedef struct stc_ETH_TX_Q_SEG_ALLOC_Q0TO7_field {
  uint32_t                         u3SEGMENT_ALLOC_Q0:3;
  uint32_t                         u1RESERVED_3_3:1;
  uint32_t                         u3SEGMENT_ALLOC_Q1:3;
  uint32_t                         u1RESERVED_7_7:1;
  uint32_t                         u3SEGMENT_ALLOC_Q2:3;
  uint32_t                         u1RESERVED_11_11:1;
  uint32_t                         u3REMOVED_14_12:3;
  uint32_t                         u1RESERVED_15_15:1;
  uint32_t                         u3REMOVED_18_16:3;
  uint32_t                         u1RESERVED_19_19:1;
  uint32_t                         u3REMOVED_22_20:3;
  uint32_t                         u1RESERVED_23:1;
  uint32_t                         u3REMOVED_26_24:3;
  uint32_t                         u1RESERVED_27_27:1;
  uint32_t                         u3REMOVED_30_28:3;
  uint32_t                         u1RESERVED_31_31:1;
} stc_ETH_TX_Q_SEG_ALLOC_Q0TO7_field_t;

typedef union un_ETH_TX_Q_SEG_ALLOC_Q0TO7 {
  uint32_t                         u32Register;
  stc_ETH_TX_Q_SEG_ALLOC_Q0TO7_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TX_Q_SEG_ALLOC_Q0TO7_t;

/**
  * \brief Not presents.  Access to the register returns AHB error. (ETH_TX_Q_SEG_ALLOC_Q8TO15)
  */
typedef struct stc_ETH_TX_Q_SEG_ALLOC_Q8TO15_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_TX_Q_SEG_ALLOC_Q8TO15_field_t;

typedef union un_ETH_TX_Q_SEG_ALLOC_Q8TO15 {
  uint32_t                         u32Register;
  stc_ETH_TX_Q_SEG_ALLOC_Q8TO15_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TX_Q_SEG_ALLOC_Q8TO15_t;

/**
  * \brief receive_q8_ptr to receive_q15_ptr doesn't present.  Access to the register returns AHB error. (ETH_RECEIVE_Q8_PTR)
  */
typedef struct stc_ETH_RECEIVE_Q8_PTR_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_RECEIVE_Q8_PTR_field_t;

typedef union un_ETH_RECEIVE_Q8_PTR {
  uint32_t                         u32Register;
  stc_ETH_RECEIVE_Q8_PTR_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RECEIVE_Q8_PTR_t;

/**
  * \brief Not presents. (ETH_RECEIVE_Q15_PTR)
  */
typedef struct stc_ETH_RECEIVE_Q15_PTR_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_RECEIVE_Q15_PTR_field_t;

typedef union un_ETH_RECEIVE_Q15_PTR {
  uint32_t                         u32Register;
  stc_ETH_RECEIVE_Q15_PTR_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_RECEIVE_Q15_PTR_t;

/**
  * \brief dma_rxbuf_size_q8 to dma_rxbuf_size_q15 doesn't present.  Access to the register returns AHB error. (ETH_DMA_RXBUF_SIZE_Q8)
  */
typedef struct stc_ETH_DMA_RXBUF_SIZE_Q8_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_DMA_RXBUF_SIZE_Q8_field_t;

typedef union un_ETH_DMA_RXBUF_SIZE_Q8 {
  uint32_t                         u32Register;
  stc_ETH_DMA_RXBUF_SIZE_Q8_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DMA_RXBUF_SIZE_Q8_t;

/**
  * \brief Not presents. (ETH_DMA_RXBUF_SIZE_Q15)
  */
typedef struct stc_ETH_DMA_RXBUF_SIZE_Q15_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_DMA_RXBUF_SIZE_Q15_field_t;

typedef union un_ETH_DMA_RXBUF_SIZE_Q15 {
  uint32_t                         u32Register;
  stc_ETH_DMA_RXBUF_SIZE_Q15_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_DMA_RXBUF_SIZE_Q15_t;

/**
  * \brief At reset all interrupts are disabled. Writing a one to the relevant bit location enables the required interrupt. This register is write only and when read will return zero. (ETH_INT_Q1_ENABLE)
  */
typedef struct stc_ETH_INT_Q1_ENABLE_field {
  uint32_t                         :1;
  uint32_t                         u1ENABLE_RECEIVE_COMPLETE_INTERRUPT:1;
  uint32_t                         u1ENABLE_RX_USED_BIT_READ_INTERRUPT:1;
  uint32_t                         :2;
  uint32_t                         u1ENABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT:1;
  uint32_t                         u1ENABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT:1;
  uint32_t                         u1ENABLE_TRANSMIT_COMPLETE_INTERRUPT:1;
  uint32_t                         :3;
  uint32_t                         u1ENABLE_RESP_NOT_OK_INTERRUPT:1;
  uint32_t                         :20;
} stc_ETH_INT_Q1_ENABLE_field_t;

typedef union un_ETH_INT_Q1_ENABLE {
  uint32_t                         u32Register;
  stc_ETH_INT_Q1_ENABLE_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q1_ENABLE_t;

/**
  * \brief At reset all interrupts are disabled. Writing a one to the relevant bit location enables the required interrupt. This register is write only and when read will return zero. (ETH_INT_Q2_ENABLE)
  */
typedef struct stc_ETH_INT_Q2_ENABLE_field {
  uint32_t                         :1;
  uint32_t                         u1ENABLE_RECEIVE_COMPLETE_INTERRUPT:1;
  uint32_t                         u1ENABLE_RX_USED_BIT_READ_INTERRUPT:1;
  uint32_t                         :2;
  uint32_t                         u1ENABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT:1;
  uint32_t                         u1ENABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT:1;
  uint32_t                         u1ENABLE_TRANSMIT_COMPLETE_INTERRUPT:1;
  uint32_t                         :3;
  uint32_t                         u1ENABLE_RESP_NOT_OK_INTERRUPT:1;
  uint32_t                         :20;
} stc_ETH_INT_Q2_ENABLE_field_t;

typedef union un_ETH_INT_Q2_ENABLE {
  uint32_t                         u32Register;
  stc_ETH_INT_Q2_ENABLE_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q2_ENABLE_t;

/**
  * \brief int_q3_enable to int_q7_enable doesn't present.  Access to the register returns AHB error. (ETH_INT_Q3_ENABLE)
  */
typedef struct stc_ETH_INT_Q3_ENABLE_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_INT_Q3_ENABLE_field_t;

typedef union un_ETH_INT_Q3_ENABLE {
  uint32_t                         u32Register;
  stc_ETH_INT_Q3_ENABLE_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q3_ENABLE_t;

/**
  * \brief Not presents. (ETH_INT_Q7_ENABLE)
  */
typedef struct stc_ETH_INT_Q7_ENABLE_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_INT_Q7_ENABLE_field_t;

typedef union un_ETH_INT_Q7_ENABLE {
  uint32_t                         u32Register;
  stc_ETH_INT_Q7_ENABLE_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q7_ENABLE_t;

/**
  * \brief Writing a 1 to the relevant bit location disables that particular interrupt. This register is write only and when read will return zero. (ETH_INT_Q1_DISABLE)
  */
typedef struct stc_ETH_INT_Q1_DISABLE_field {
  uint32_t                         :1;
  uint32_t                         u1DISABLE_RECEIVE_COMPLETE_INTERRUPT:1;
  uint32_t                         u1DISABLE_RX_USED_BIT_READ_INTERRUPT:1;
  uint32_t                         :2;
  uint32_t                         u1DISABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT:1;
  uint32_t                         u1DISABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT:1;
  uint32_t                         u1DISABLE_TRANSMIT_COMPLETE_INTERRUPT:1;
  uint32_t                         :3;
  uint32_t                         u1DISABLE_RESP_NOT_OK_INTERRUPT:1;
  uint32_t                         :20;
} stc_ETH_INT_Q1_DISABLE_field_t;

typedef union un_ETH_INT_Q1_DISABLE {
  uint32_t                         u32Register;
  stc_ETH_INT_Q1_DISABLE_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q1_DISABLE_t;

/**
  * \brief Writing a 1 to the relevant bit location disables that particular interrupt. This register is write only and when read will return zero. (ETH_INT_Q2_DISABLE)
  */
typedef struct stc_ETH_INT_Q2_DISABLE_field {
  uint32_t                         :1;
  uint32_t                         u1DISABLE_RECEIVE_COMPLETE_INTERRUPT:1;
  uint32_t                         u1DISABLE_RX_USED_BIT_READ_INTERRUPT:1;
  uint32_t                         :2;
  uint32_t                         u1DISABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT:1;
  uint32_t                         u1DISABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT:1;
  uint32_t                         u1DISABLE_TRANSMIT_COMPLETE_INTERRUPT:1;
  uint32_t                         :3;
  uint32_t                         u1DISABLE_RESP_NOT_OK_INTERRUPT:1;
  uint32_t                         :20;
} stc_ETH_INT_Q2_DISABLE_field_t;

typedef union un_ETH_INT_Q2_DISABLE {
  uint32_t                         u32Register;
  stc_ETH_INT_Q2_DISABLE_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q2_DISABLE_t;

/**
  * \brief int_q3_disable to int_q7_disable doesn't present.  Access to the register returns AHB error. (ETH_INT_Q3_DISABLE)
  */
typedef struct stc_ETH_INT_Q3_DISABLE_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_INT_Q3_DISABLE_field_t;

typedef union un_ETH_INT_Q3_DISABLE {
  uint32_t                         u32Register;
  stc_ETH_INT_Q3_DISABLE_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q3_DISABLE_t;

/**
  * \brief Not presents. (ETH_INT_Q7_DISABLE)
  */
typedef struct stc_ETH_INT_Q7_DISABLE_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_INT_Q7_DISABLE_field_t;

typedef union un_ETH_INT_Q7_DISABLE {
  uint32_t                         u32Register;
  stc_ETH_INT_Q7_DISABLE_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q7_DISABLE_t;

/**
  * \brief The interrupt mask register is a read only register indicating which interrupts are masked. All bits are set at reset and can be reset individually by writing to the interrupt enable register or set individually by writing to the interrupt disable register. Having separate address locations for enable and disable saves the need for performing a read modify write when updating the interrupt mask register. For test purposes there is a write-only function to this register that allows the bits in the interrupt status register to be set or cleared, regardless of the state of the mask register. (ETH_INT_Q1_MASK)
  */
typedef struct stc_ETH_INT_Q1_MASK_field {
  uint32_t                         :1;
  uint32_t                         u1RECEIVE_COMPLETE_INTERRUPT_MASK:1;
  uint32_t                         u1RX_USED_INTERRUPT_MASK:1;
  uint32_t                         :2;
  uint32_t                         u1RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_MASK:1;
  uint32_t                         u1AMBA_ERROR_INTERRUPT_MASK:1;
  uint32_t                         u1TRANSMIT_COMPLETE_INTERRUPT_MASK:1;
  uint32_t                         :3;
  uint32_t                         u1RESP_NOT_OK_INTERRUPT_MASK:1;
  uint32_t                         :20;
} stc_ETH_INT_Q1_MASK_field_t;

typedef union un_ETH_INT_Q1_MASK {
  uint32_t                         u32Register;
  stc_ETH_INT_Q1_MASK_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q1_MASK_t;

/**
  * \brief The interrupt mask register is a read only register indicating which interrupts are masked. All bits are set at reset and can be reset individually by writing to the interrupt enable register or set individually by writing to the interrupt disable register. Having separate address locations for enable and disable saves the need for performing a read modify write when updating the interrupt mask register. For test purposes there is a write-only function to this register that allows the bits in the interrupt status register to be set or cleared, regardless of the state of the mask register. (ETH_INT_Q2_MASK)
  */
typedef struct stc_ETH_INT_Q2_MASK_field {
  uint32_t                         :1;
  uint32_t                         u1RECEIVE_COMPLETE_INTERRUPT_MASK:1;
  uint32_t                         u1RX_USED_INTERRUPT_MASK:1;
  uint32_t                         :2;
  uint32_t                         u1RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_MASK:1;
  uint32_t                         u1AMBA_ERROR_INTERRUPT_MASK:1;
  uint32_t                         u1TRANSMIT_COMPLETE_INTERRUPT_MASK:1;
  uint32_t                         :3;
  uint32_t                         u1RESP_NOT_OK_INTERRUPT_MASK:1;
  uint32_t                         :20;
} stc_ETH_INT_Q2_MASK_field_t;

typedef union un_ETH_INT_Q2_MASK {
  uint32_t                         u32Register;
  stc_ETH_INT_Q2_MASK_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q2_MASK_t;

/**
  * \brief int_q3_mask to int_q7_mask doesn't present.  Access to the register returns AHB error. (ETH_INT_Q3_MASK)
  */
typedef struct stc_ETH_INT_Q3_MASK_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_INT_Q3_MASK_field_t;

typedef union un_ETH_INT_Q3_MASK {
  uint32_t                         u32Register;
  stc_ETH_INT_Q3_MASK_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q3_MASK_t;

/**
  * \brief Not presents. (ETH_INT_Q7_MASK)
  */
typedef struct stc_ETH_INT_Q7_MASK_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_INT_Q7_MASK_field_t;

typedef union un_ETH_INT_Q7_MASK {
  uint32_t                         u32Register;
  stc_ETH_INT_Q7_MASK_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q7_MASK_t;

/**
  * \brief int_q8_enable to int_q15_enable doesn't present.  Access to the register returns AHB error. (ETH_INT_Q8_ENABLE)
  */
typedef struct stc_ETH_INT_Q8_ENABLE_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_INT_Q8_ENABLE_field_t;

typedef union un_ETH_INT_Q8_ENABLE {
  uint32_t                         u32Register;
  stc_ETH_INT_Q8_ENABLE_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q8_ENABLE_t;

/**
  * \brief Not presents. (ETH_INT_Q15_ENABLE)
  */
typedef struct stc_ETH_INT_Q15_ENABLE_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_INT_Q15_ENABLE_field_t;

typedef union un_ETH_INT_Q15_ENABLE {
  uint32_t                         u32Register;
  stc_ETH_INT_Q15_ENABLE_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q15_ENABLE_t;

/**
  * \brief int_q8_disable to int_q15_disable doesn't present.  Access to the register returns AHB error. (ETH_INT_Q8_DISABLE)
  */
typedef struct stc_ETH_INT_Q8_DISABLE_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_INT_Q8_DISABLE_field_t;

typedef union un_ETH_INT_Q8_DISABLE {
  uint32_t                         u32Register;
  stc_ETH_INT_Q8_DISABLE_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q8_DISABLE_t;

/**
  * \brief Not presents. (ETH_INT_Q15_DISABLE)
  */
typedef struct stc_ETH_INT_Q15_DISABLE_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_INT_Q15_DISABLE_field_t;

typedef union un_ETH_INT_Q15_DISABLE {
  uint32_t                         u32Register;
  stc_ETH_INT_Q15_DISABLE_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q15_DISABLE_t;

/**
  * \brief int_q8_mask to int_q15_mask doesn't present.  Access to the register returns AHB error. (ETH_INT_Q8_MASK)
  */
typedef struct stc_ETH_INT_Q8_MASK_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_INT_Q8_MASK_field_t;

typedef union un_ETH_INT_Q8_MASK {
  uint32_t                         u32Register;
  stc_ETH_INT_Q8_MASK_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q8_MASK_t;

/**
  * \brief Not presents. (ETH_INT_Q15_MASK)
  */
typedef struct stc_ETH_INT_Q15_MASK_field {
  uint32_t                         u32REMOVED_31_0:32;
} stc_ETH_INT_Q15_MASK_field_t;

typedef union un_ETH_INT_Q15_MASK {
  uint32_t                         u32Register;
  stc_ETH_INT_Q15_MASK_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_INT_Q15_MASK_t;

/**
  * \brief Ethertype Register (ETH_SCREENING_TYPE_2_ETHERTYPE_REG_0)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_0_field {
  uint32_t                         u16COMPARE_VALUE:16;
  uint32_t                         u16RESERVED_31_16:16;
} stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_0_field_t;

typedef union un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_0 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_0_t;

/**
  * \brief Ethertype Register (ETH_SCREENING_TYPE_2_ETHERTYPE_REG_1)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_1_field {
  uint32_t                         u16COMPARE_VALUE:16;
  uint32_t                         u16RESERVED_31_16:16;
} stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_1_field_t;

typedef union un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_1 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_1_t;

/**
  * \brief Ethertype Register (ETH_SCREENING_TYPE_2_ETHERTYPE_REG_2)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_2_field {
  uint32_t                         u16COMPARE_VALUE:16;
  uint32_t                         u16RESERVED_31_16:16;
} stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_2_field_t;

typedef union un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_2 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_2_t;

/**
  * \brief Ethertype Register (ETH_SCREENING_TYPE_2_ETHERTYPE_REG_3)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_3_field {
  uint32_t                         u16COMPARE_VALUE:16;
  uint32_t                         u16RESERVED_31_16:16;
} stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_3_field_t;

typedef union un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_3 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_3_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_3_t;

/**
  * \brief Ethertype Register (ETH_SCREENING_TYPE_2_ETHERTYPE_REG_4)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_4_field {
  uint32_t                         u16COMPARE_VALUE:16;
  uint32_t                         u16RESERVED_31_16:16;
} stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_4_field_t;

typedef union un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_4 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_4_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_4_t;

/**
  * \brief Ethertype Register (ETH_SCREENING_TYPE_2_ETHERTYPE_REG_5)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_5_field {
  uint32_t                         u16COMPARE_VALUE:16;
  uint32_t                         u16RESERVED_31_16:16;
} stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_5_field_t;

typedef union un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_5 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_5_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_5_t;

/**
  * \brief Ethertype Register (ETH_SCREENING_TYPE_2_ETHERTYPE_REG_6)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_6_field {
  uint32_t                         u16COMPARE_VALUE:16;
  uint32_t                         u16RESERVED_31_16:16;
} stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_6_field_t;

typedef union un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_6 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_6_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_6_t;

/**
  * \brief Ethertype Register (ETH_SCREENING_TYPE_2_ETHERTYPE_REG_7)
  */
typedef struct stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_7_field {
  uint32_t                         u16COMPARE_VALUE:16;
  uint32_t                         u16RESERVED_31_16:16;
} stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_7_field_t;

typedef union un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_7 {
  uint32_t                         u32Register;
  stc_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_7_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_7_t;

/**
  * \brief 'Compare A, B and C fields of the screener type 2 match register are pointers to a pool of up to 32 compare registers. If enabled the compare is true if the data at the OFFSET into the frame, ANDed with the MASK Value if the mask is enabled, is equal to the COMPARE Value. Either a 16 bit comparison or a 32 bit comparison is done. This selection is made via the associated compare word1 register bit 9. If a 16 bit comparison is selected, then a 16 bit mask is also available to the user to select which bits should be compared. If the 32 bit compare option is selected, then no mask is available. The byte at the OFFSET number of bytes from the index start is compared thru bits 7 to 0 of the configured VALUE. The byte at the OFFSET number of bytes + 1 from the index start is compared thru bits 15 to 8 of the configured VALUE and so on. The OFFSET can be configured to be from 0 to 127 bytes from either the start of the frame, the byte following the therType field (last EtherType in the header if the frame is VLAN tagged), the byte following the IP header (IPv4 or IPv6) or from the byte following the start of the TCP/UDP header. The required number of Type 2 screening registers up to a maximum of 32 is configurable in the gem defines file and have been allocated APB address space between 0x700 and 0x7fc. Note. when using RX Partial Store and Forward mode and priority queues, the frame offset must be less than the Partial Store and Forward watermark. If the offset is higher than the watermark value it's not possible to identify the priority queue before the frame is sent to the AMBA interface, and an incorrect priority queue may be used. ' (ETH_TYPE2_COMPARE_0_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_0_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_0_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_0_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_0_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_0_WORD_0_t;

/**
  * \brief 'Type2 Compare Word 1' (ETH_TYPE2_COMPARE_0_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_0_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_0_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_0_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_0_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_0_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_1_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_1_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_1_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_1_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_1_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_1_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_1_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_1_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_1_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_1_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_1_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_1_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_2_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_2_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_2_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_2_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_2_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_2_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_2_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_2_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_2_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_2_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_2_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_2_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_3_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_3_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_3_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_3_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_3_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_3_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_3_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_3_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_3_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_3_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_3_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_3_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_4_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_4_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_4_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_4_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_4_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_4_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_4_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_4_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_4_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_4_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_4_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_4_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_5_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_5_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_5_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_5_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_5_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_5_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_5_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_5_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_5_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_5_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_5_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_5_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_6_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_6_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_6_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_6_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_6_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_6_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_6_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_6_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_6_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_6_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_6_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_6_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_7_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_7_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_7_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_7_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_7_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_7_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_7_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_7_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_7_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_7_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_7_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_7_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_8_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_8_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_8_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_8_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_8_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_8_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_8_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_8_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_8_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_8_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_8_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_8_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_9_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_9_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_9_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_9_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_9_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_9_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_9_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_9_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_9_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_9_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_9_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_9_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_10_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_10_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_10_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_10_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_10_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_10_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_10_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_10_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_10_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_10_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_10_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_10_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_11_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_11_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_11_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_11_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_11_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_11_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_11_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_11_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_11_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_11_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_11_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_11_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_12_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_12_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_12_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_12_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_12_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_12_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_12_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_12_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_12_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_12_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_12_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_12_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_13_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_13_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_13_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_13_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_13_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_13_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_13_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_13_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_13_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_13_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_13_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_13_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_14_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_14_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_14_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_14_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_14_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_14_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_14_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_14_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_14_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_14_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_14_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_14_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_15_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_15_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_15_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_15_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_15_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_15_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_15_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_15_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_15_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_15_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_15_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_15_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_16_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_16_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_16_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_16_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_16_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_16_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_16_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_16_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_16_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_16_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_16_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_16_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_17_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_17_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_17_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_17_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_17_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_17_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_17_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_17_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_17_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_17_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_17_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_17_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_18_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_18_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_18_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_18_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_18_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_18_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_18_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_18_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_18_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_18_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_18_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_18_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_19_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_19_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_19_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_19_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_19_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_19_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_19_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_19_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_19_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_19_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_19_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_19_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_20_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_20_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_20_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_20_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_20_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_20_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_20_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_20_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_20_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_20_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_20_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_20_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_21_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_21_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_21_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_21_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_21_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_21_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_21_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_21_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_21_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_21_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_21_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_21_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_22_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_22_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_22_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_22_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_22_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_22_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_22_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_22_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_22_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_22_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_22_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_22_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_23_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_23_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_23_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_23_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_23_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_23_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_23_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_23_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_23_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_23_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_23_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_23_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_24_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_24_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_24_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_24_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_24_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_24_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_24_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_24_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_24_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_24_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_24_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_24_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_25_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_25_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_25_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_25_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_25_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_25_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_25_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_25_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_25_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_25_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_25_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_25_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_26_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_26_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_26_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_26_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_26_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_26_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_26_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_26_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_26_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_26_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_26_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_26_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_27_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_27_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_27_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_27_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_27_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_27_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_27_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_27_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_27_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_27_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_27_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_27_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_28_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_28_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_28_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_28_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_28_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_28_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_28_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_28_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_28_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_28_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_28_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_28_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_29_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_29_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_29_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_29_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_29_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_29_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_29_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_29_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_29_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_29_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_29_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_29_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_30_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_30_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_30_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_30_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_30_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_30_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_30_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_30_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_30_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_30_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_30_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_30_WORD_1_t;

/**
  * \brief same as type2_compare_0_word_0 (ETH_TYPE2_COMPARE_31_WORD_0)
  */
typedef struct stc_ETH_TYPE2_COMPARE_31_WORD_0_field {
  uint32_t                         u16MASK_VALUE:16;
  uint32_t                         u16COMPARE_VALUE_TYPE2:16;
} stc_ETH_TYPE2_COMPARE_31_WORD_0_field_t;

typedef union un_ETH_TYPE2_COMPARE_31_WORD_0 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_31_WORD_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_31_WORD_0_t;

/**
  * \brief same as type2_compare_0_word_1 (ETH_TYPE2_COMPARE_31_WORD_1)
  */
typedef struct stc_ETH_TYPE2_COMPARE_31_WORD_1_field {
  uint32_t                         u7OFFSET_VALUE:7;
  uint32_t                         u2COMPARE_OFFSET:2;
  uint32_t                         u1DISABLE_MASK:1;
  uint32_t                         u22RESERVED_31_10:22;
} stc_ETH_TYPE2_COMPARE_31_WORD_1_field_t;

typedef union un_ETH_TYPE2_COMPARE_31_WORD_1 {
  uint32_t                         u32Register;
  stc_ETH_TYPE2_COMPARE_31_WORD_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_ETH_TYPE2_COMPARE_31_WORD_1_t;



/**
  * \brief Ethernet Interface (ETH)
  */
typedef struct stc_ETH {
  un_ETH_CTL_t                     unCTL;              /*!< 0x00000000 MXETH Control Register */
  un_ETH_STATUS_t                  unSTATUS;           /*!< 0x00000004 MXETH Status Register */
  uint32_t                         au32Reserved[1022];
  un_ETH_NETWORK_CONTROL_t         unNETWORK_CONTROL;  /*!< 0x00001000 The network control register contains general MAC
                                                                                control functions for both receiver and
                                                                                transmitter. */
  un_ETH_NETWORK_CONFIG_t          unNETWORK_CONFIG;   /*!< 0x00001004 The network configuration register contains functions
                                                                                for setting the mode of operation for the
                                                                                Gigabit Ethernet MAC. */
  un_ETH_NETWORK_STATUS_t          unNETWORK_STATUS;   /*!< 0x00001008 The network status register returns status information
                                                                                with respect to the PHY management interface. */
  un_ETH_USER_IO_REGISTER_t        unUSER_IO_REGISTER; /*!< 0x0000100C Not presents. Access to the register will return AHB
                                                                                error. */
  un_ETH_DMA_CONFIG_t              unDMA_CONFIG;       /*!< 0x00001010 DMA Configuration Register */
  un_ETH_TRANSMIT_STATUS_t         unTRANSMIT_STATUS;  /*!< 0x00001014 This register, when read, provides details of the status
                                                                                of a transmit. Once read, individual bits may be
                                                                                cleared by writing 1 to them. It is not possible
                                                                                to set a bit to 1 by writing to the register. */
  un_ETH_RECEIVE_Q_PTR_t           unRECEIVE_Q_PTR;    /*!< 0x00001018 This register holds the start address of the receive
                                                                                buffer queue (receive buffers descriptor list).
                                                                                The receive buffer queue base address must be
                                                                                initialized before receive is enabled through
                                                                                bit 2 of the network control register. Once
                                                                                reception is enabled, any write to the receive
                                                                                buffer queue base address register is ignored.
                                                                                Reading this register returns the location of
                                                                                the descriptor currently being accessed. This
                                                                                value increments as buffers are used. Software
                                                                                should not use this register for determining
                                                                                where to remove received frames from the queue
                                                                                as it constantly changes as new frames are
                                                                                received. Software should instead work its way
                                                                                through the buffer descriptor queue checking the
                                                                                used bits. In terms of AMBA (AXI) operation, the
                                                                                receive descriptors are read from memory using a
                                                                                single 32bit AXI access. When the datapath is
                                                                                configured at 64bit, the receive descriptors
                                                                                should be aligned at 64-bit boundaries and each
                                                                                pair of 32-bit descriptors is written to using a
                                                                                single 64bit AXI access. */
  un_ETH_TRANSMIT_Q_PTR_t          unTRANSMIT_Q_PTR;   /*!< 0x0000101C This register holds the start address of the transmit
                                                                                buffer queue (transmit buffers descriptor list).
                                                                                The transmit buffer queue base address register
                                                                                must be initialized before transmit is started
                                                                                through bit 9 of the network control register.
                                                                                Once transmission has started, any write to the
                                                                                transmit buffer queue base address register is
                                                                                illegal and therefore ignored. Note that due to
                                                                                clock boundary synchronization, it takes a
                                                                                maximum of four pclk cycles from the writing of
                                                                                the transmit start bit before the transmitter is
                                                                                active. Writing to the transmit buffer queue
                                                                                base address register during this time may
                                                                                produce unpredictable results. Reading this
                                                                                register returns the location of the descriptor
                                                                                currently being accessed. Because the DMA can
                                                                                store data for multiple frames at once, this may
                                                                                not necessarily be pointing to the current frame
                                                                                being transmitted. In terms of AMBA AXI
                                                                                operation, the transmit descriptors are written
                                                                                to memory using a single 32bit AHB access. When
                                                                                the datapath is configured as 64bit, the
                                                                                transmit descriptors should be aligned at 64-bit
                                                                                boundaries and each pair of 32-bit descriptors
                                                                                is read from memory using a single AXI access. */
  un_ETH_RECEIVE_STATUS_t          unRECEIVE_STATUS;   /*!< 0x00001020 This register, when read provides details of the status
                                                                                of a receive. Once read, individual bits may be
                                                                                cleared by writing 1 to them. It is not possible
                                                                                to set a bit to 1 by writing to the register. */
  un_ETH_INT_STATUS_t              unINT_STATUS;       /*!< 0x00001024 If not configured for priority queueing, the GEM
                                                                                generates a single interrupt. This register
                                                                                indicates the source of this interrupt. The
                                                                                corresponding bit in the mask register must be
                                                                                clear for a bit to be set. If any bit is set in
                                                                                this register the ethernet_int signal will be
                                                                                asserted. For test purposes each bit can be set
                                                                                or reset by writing to the interrupt mask
                                                                                register. The default configuration is shown
                                                                                below whereby all bits are reset to zero on
                                                                                read. Changing the validity of the
                                                                                `gem_irq_read_clear define will instead require
                                                                                a one to be written to the appropriate bit in
                                                                                order to clear it. In this mode reading has no
                                                                                affect on the status of the bit. */
  un_ETH_INT_ENABLE_t              unINT_ENABLE;       /*!< 0x00001028 At reset all interrupts are disabled. Writing a one to
                                                                                the relevant bit location enables the required
                                                                                interrupt. This register is write only and when
                                                                                read will return zero. */
  un_ETH_INT_DISABLE_t             unINT_DISABLE;      /*!< 0x0000102C Writing a 1 to the relevant bit location disables that
                                                                                particular interrupt. This register is write
                                                                                only and when read will return zero. */
  un_ETH_INT_MASK_t                unINT_MASK;         /*!< 0x00001030 The interrupt mask register is a read only register
                                                                                indicating which interrupts are masked. All bits
                                                                                are set at reset and can be reset individually
                                                                                by writing to the interrupt enable register or
                                                                                set individually by writing to the interrupt
                                                                                disable register. Having separate address
                                                                                locations for enable and disable saves the need
                                                                                for performing a read modify write when updating
                                                                                the interrupt mask register. For test purposes
                                                                                there is a write-only function to this register
                                                                                that allows the bits in the interrupt status
                                                                                register to be set or cleared, regardless of the
                                                                                state of the mask register. */
  un_ETH_PHY_MANAGEMENT_t          unPHY_MANAGEMENT;   /*!< 0x00001034 The PHY maintenance register is implemented as a shift
                                                                                register. Writing to the register starts a shift
                                                                                operation which is signalled as complete when
                                                                                bit-2 is set in the network status register. It
                                                                                takes about 2000 pclk cycles to complete, when
                                                                                MDC is set for pclk divide by 32 in the network
                                                                                configuration register. An interrupt is
                                                                                generated upon completion. During this time, the
                                                                                MSB of the register is output on the MDIO pin
                                                                                and the LSB updated from the MDIO pin with each
                                                                                MDC cycle. This causes transmission of a PHY
                                                                                management frame on MDIO. See Section 22.2.4.5
                                                                                of the IEEE 802.3 standard. Reading during the
                                                                                shift operation will return the current contents
                                                                                of the shift register. At the end of management
                                                                                operation, the bits will have shifted back to
                                                                                their original locations. For a read operation,
                                                                                the data bits will be updated with data read
                                                                                from the PHY. It is important to write the
                                                                                correct values to the register to ensure a valid
                                                                                PHY management frame is produced. The MDIO
                                                                                interface can read IEEE 802.3 clause 45 PHYs as
                                                                                well as clause 22 PHYs. To read clause 45 PHYs,
                                                                                bit 30 should be written with a 0 rather than a
                                                                                1. For a description of MDC generation, see
                                                                                Network Configuration Register. */
  un_ETH_PAUSE_TIME_t              unPAUSE_TIME;       /*!< 0x00001038 Received Pause Quantum Register */
  un_ETH_TX_PAUSE_QUANTUM_t        unTX_PAUSE_QUANTUM; /*!< 0x0000103C Transmit Pause Quantum Register */
  un_ETH_PBUF_TXCUTTHRU_t          unPBUF_TXCUTTHRU;   /*!< 0x00001040 Partial store and forward is only applicable when using
                                                                                the DMA configured in SRAM based packet buffer
                                                                                mode. It is also not available when using multi
                                                                                buffer frames. TX Partial Store and Forward */
  un_ETH_PBUF_RXCUTTHRU_t          unPBUF_RXCUTTHRU;   /*!< 0x00001044 RX Partial Store and Forward */
  un_ETH_JUMBO_MAX_LENGTH_t        unJUMBO_MAX_LENGTH; /*!< 0x00001048 Maximum Jumbo Frame Size. */
  un_ETH_EXTERNAL_FIFO_INTERFACE_t unEXTERNAL_FIFO_INTERFACE; /*!< 0x0000104C Not presents. */
  uint32_t                         u32Reserved1;
  un_ETH_AXI_MAX_PIPELINE_t        unAXI_MAX_PIPELINE; /*!< 0x00001054 Used to set the maximum amount of outstanding
                                                                                transactions on the AXI bus between AR / R
                                                                                channels and AW / W channels. Cannot be more
                                                                                than the depth of the configured AXI pipeline
                                                                                FIFO (defined in verilog defs.v) */
  un_ETH_RSC_CONTROL_t             unRSC_CONTROL;      /*!< 0x00001058 Not presents. Access to the register will return AHB
                                                                                error. */
  un_ETH_INT_MODERATION_t          unINT_MODERATION;   /*!< 0x0000105C Used to moderate the number of transmit and receive
                                                                                complete interrupts issued. With interrupt
                                                                                moderation enabled receive and transmit
                                                                                interrupts are not generated immediately a frame
                                                                                is transmitted or received. Instead when a
                                                                                receive or transmit event occurs a timer is
                                                                                started and the interrupt is asserted after it
                                                                                times out. This limits the frequency with which
                                                                                the CPU receives interrupts. When interrupt
                                                                                moderation is enabled interrupt status bit one
                                                                                is always used for receive and bit 7 is always
                                                                                used for transmit even when priority queuing is
                                                                                enabled. With interrupt moderation 800ns periods
                                                                                are counted. GEM determines what constitutes an
                                                                                800ns period by looking at the tbi (bit 11),
                                                                                gigabit bit (10) and speed (bit 0) bits in the
                                                                                network configuration register and counting
                                                                                tx_clk cycles. Bit 0 needs to be set to 1 for
                                                                                100M operation. */
  un_ETH_SYS_WAKE_TIME_t           unSYS_WAKE_TIME;    /*!< 0x00001060 Used to pause transmission after deassertion of
                                                                                tx_lpi_en. Each unit in this register
                                                                                corresponds to 64ns in gigabit mode, 320ns in
                                                                                100M mode and 3200ns at 10M. After tx_lpi_en is
                                                                                deasserted transmission will pause for the set
                                                                                time. */
  uint32_t                         au32Reserved2[7];
  un_ETH_HASH_BOTTOM_t             unHASH_BOTTOM;      /*!< 0x00001080 The unicast hash enable and the multicast hash enable
                                                                                bits in the network configuration register
                                                                                enable the reception of hash matched frames.
                                                                                Hash Register Bottom (31 to 0 bits) */
  un_ETH_HASH_TOP_t                unHASH_TOP;         /*!< 0x00001084 Hash Register Top (63 to 32 bits) */
  un_ETH_SPEC_ADD1_BOTTOM_t        unSPEC_ADD1_BOTTOM; /*!< 0x00001088 The addresses stored in the specific address registers
                                                                                are deactivated at reset or when their
                                                                                corresponding specific address register bottom
                                                                                is written. They are activated when specific
                                                                                address register top is written. */
  un_ETH_SPEC_ADD1_TOP_t           unSPEC_ADD1_TOP;    /*!< 0x0000108C Specific Address Top */
  un_ETH_SPEC_ADD2_BOTTOM_t        unSPEC_ADD2_BOTTOM; /*!< 0x00001090 The addresses stored in the specific address registers
                                                                                are deactivated at reset or when their
                                                                                corresponding specific address register bottom
                                                                                is written. They are activated when specific
                                                                                address register top is written. */
  un_ETH_SPEC_ADD2_TOP_t           unSPEC_ADD2_TOP;    /*!< 0x00001094 Specific Address Top */
  un_ETH_SPEC_ADD3_BOTTOM_t        unSPEC_ADD3_BOTTOM; /*!< 0x00001098 The addresses stored in the specific address registers
                                                                                are deactivated at reset or when their
                                                                                corresponding specific address register bottom
                                                                                is written. They are activated when specific
                                                                                address register top is written. */
  un_ETH_SPEC_ADD3_TOP_t           unSPEC_ADD3_TOP;    /*!< 0x0000109C Specific Address Top */
  un_ETH_SPEC_ADD4_BOTTOM_t        unSPEC_ADD4_BOTTOM; /*!< 0x000010A0 The addresses stored in the specific address registers
                                                                                are deactivated at reset or when their
                                                                                corresponding specific address register bottom
                                                                                is written. They are activated when specific
                                                                                address register top is written. */
  un_ETH_SPEC_ADD4_TOP_t           unSPEC_ADD4_TOP;    /*!< 0x000010A4 Specific Address Top */
  un_ETH_SPEC_TYPE1_t              unSPEC_TYPE1;       /*!< 0x000010A8 Type ID Match 1 */
  un_ETH_SPEC_TYPE2_t              unSPEC_TYPE2;       /*!< 0x000010AC Type ID Match 2 */
  un_ETH_SPEC_TYPE3_t              unSPEC_TYPE3;       /*!< 0x000010B0 Type ID Match 3 */
  un_ETH_SPEC_TYPE4_t              unSPEC_TYPE4;       /*!< 0x000010B4 Type ID Match 4 */
  un_ETH_WOL_REGISTER_t            unWOL_REGISTER;     /*!< 0x000010B8 Wake on LAN Register. Presents in design, but feature is
                                                                                not supported. */
  un_ETH_STRETCH_RATIO_t           unSTRETCH_RATIO;    /*!< 0x000010BC IPG stretch register */
  un_ETH_STACKED_VLAN_t            unSTACKED_VLAN;     /*!< 0x000010C0 Stacked VLAN Register */
  un_ETH_TX_PFC_PAUSE_t            unTX_PFC_PAUSE;     /*!< 0x000010C4 Transmit PFC Pause Register */
  un_ETH_MASK_ADD1_BOTTOM_t        unMASK_ADD1_BOTTOM; /*!< 0x000010C8 Specific Address Mask 1 Bottom (31 to 0 bits) */
  un_ETH_MASK_ADD1_TOP_t           unMASK_ADD1_TOP;    /*!< 0x000010CC Specific Address Mask 1 Top (47 to 32 bits) */
  un_ETH_DMA_ADDR_OR_MASK_t        unDMA_ADDR_OR_MASK; /*!< 0x000010D0 Receive DMA Data Buffer Address Mask */
  un_ETH_RX_PTP_UNICAST_t          unRX_PTP_UNICAST;   /*!< 0x000010D4 PTP RX unicast IP destination address */
  un_ETH_TX_PTP_UNICAST_t          unTX_PTP_UNICAST;   /*!< 0x000010D8 PTP TX unicast IP destination address */
  un_ETH_TSU_NSEC_CMP_t            unTSU_NSEC_CMP;     /*!< 0x000010DC TSU timer comparison value nanoseconds */
  un_ETH_TSU_SEC_CMP_t             unTSU_SEC_CMP;      /*!< 0x000010E0 TSU timer comparison value seconds (31 to 0 bits) */
  un_ETH_TSU_MSB_SEC_CMP_t         unTSU_MSB_SEC_CMP;  /*!< 0x000010E4 TSU timer comparison value seconds (47 to 32 bits) */
  un_ETH_TSU_PTP_TX_MSB_SEC_t      unTSU_PTP_TX_MSB_SEC; /*!< 0x000010E8 PTP Event Frame Transmitted Seconds Register (47 to 32
                                                                                bits) */
  un_ETH_TSU_PTP_RX_MSB_SEC_t      unTSU_PTP_RX_MSB_SEC; /*!< 0x000010EC PTP Event Frame Received Seconds Register (47 to 32
                                                                                bits) */
  un_ETH_TSU_PEER_TX_MSB_SEC_t     unTSU_PEER_TX_MSB_SEC; /*!< 0x000010F0 PTP Peer Event Frame Transmitted Seconds Register (47 to
                                                                                32 bits) */
  un_ETH_TSU_PEER_RX_MSB_SEC_t     unTSU_PEER_RX_MSB_SEC; /*!< 0x000010F4 PTP Peer Event Frame Received Seconds Register (47 to 32
                                                                                bits) */
  un_ETH_DPRAM_FILL_DBG_t          unDPRAM_FILL_DBG;   /*!< 0x000010F8 The fill levels for the TX & RX packet buffers can be
                                                                                read using this register, including the fill
                                                                                level for each queue in the TX direction. */
  un_ETH_REVISION_REG_t            unREVISION_REG;     /*!< 0x000010FC This register indicates a Cadence module identification
                                                                                number and module revision. The value of this
                                                                                register is read only as defined by
                                                                                `gem_revision_reg_value */
  un_ETH_OCTETS_TXED_BOTTOM_t      unOCTETS_TXED_BOTTOM; /*!< 0x00001100 Octets Transmitted lower bits (31 to 0 bits) */
  un_ETH_OCTETS_TXED_TOP_t         unOCTETS_TXED_TOP;  /*!< 0x00001104 Octets Transmitted higher bits (47 to 32 bits) */
  un_ETH_FRAMES_TXED_OK_t          unFRAMES_TXED_OK;   /*!< 0x00001108 Frames Transmitted */
  un_ETH_BROADCAST_TXED_t          unBROADCAST_TXED;   /*!< 0x0000110C Broadcast Frames Transmitted */
  un_ETH_MULTICAST_TXED_t          unMULTICAST_TXED;   /*!< 0x00001110 Multicast Frames Transmitted */
  un_ETH_PAUSE_FRAMES_TXED_t       unPAUSE_FRAMES_TXED; /*!< 0x00001114 Pause Frames Transmitted */
  un_ETH_FRAMES_TXED_64_t          unFRAMES_TXED_64;   /*!< 0x00001118 64 Byte Frames Transmitted */
  un_ETH_FRAMES_TXED_65_t          unFRAMES_TXED_65;   /*!< 0x0000111C 65 to 127 Byte Frames Transmitted */
  un_ETH_FRAMES_TXED_128_t         unFRAMES_TXED_128;  /*!< 0x00001120 128 to 255 Byte Frames Transmitted */
  un_ETH_FRAMES_TXED_256_t         unFRAMES_TXED_256;  /*!< 0x00001124 256 to 511 Byte Frames Transmitted */
  un_ETH_FRAMES_TXED_512_t         unFRAMES_TXED_512;  /*!< 0x00001128 512 to 1023 Byte Frames Transmitted */
  un_ETH_FRAMES_TXED_1024_t        unFRAMES_TXED_1024; /*!< 0x0000112C 1024 to 1518 Byte Frames Transmitted */
  un_ETH_FRAMES_TXED_1519_t        unFRAMES_TXED_1519; /*!< 0x00001130 Greater Than 1518 Byte Frames Transmitted */
  un_ETH_TX_UNDERRUNS_t            unTX_UNDERRUNS;     /*!< 0x00001134 Transmit Under Runs */
  un_ETH_SINGLE_COLLISIONS_t       unSINGLE_COLLISIONS; /*!< 0x00001138 Single Collision Frames. Presents in design but not
                                                                                support. */
  un_ETH_MULTIPLE_COLLISIONS_t     unMULTIPLE_COLLISIONS; /*!< 0x0000113C Multiple Collision Frames. Presents in design but not
                                                                                support. */
  un_ETH_EXCESSIVE_COLLISIONS_t    unEXCESSIVE_COLLISIONS; /*!< 0x00001140 Excessive Collisions. Presents in design but not
                                                                                support. */
  un_ETH_LATE_COLLISIONS_t         unLATE_COLLISIONS;  /*!< 0x00001144 Late Collisions.  Presents in design but not support. */
  un_ETH_DEFERRED_FRAMES_t         unDEFERRED_FRAMES;  /*!< 0x00001148 Deferred Transmission Frames. Presents in design but not
                                                                                support. */
  un_ETH_CRS_ERRORS_t              unCRS_ERRORS;       /*!< 0x0000114C Carrier Sense Errors. Presents in design but not
                                                                                support. */
  un_ETH_OCTETS_RXED_BOTTOM_t      unOCTETS_RXED_BOTTOM; /*!< 0x00001150 Octets Received (31 to 0 bits) */
  un_ETH_OCTETS_RXED_TOP_t         unOCTETS_RXED_TOP;  /*!< 0x00001154 Octets Received (47 to 32 bits) */
  un_ETH_FRAMES_RXED_OK_t          unFRAMES_RXED_OK;   /*!< 0x00001158 Frames Received */
  un_ETH_BROADCAST_RXED_t          unBROADCAST_RXED;   /*!< 0x0000115C Broadcast Frames Received */
  un_ETH_MULTICAST_RXED_t          unMULTICAST_RXED;   /*!< 0x00001160 Multicast Frames Received */
  un_ETH_PAUSE_FRAMES_RXED_t       unPAUSE_FRAMES_RXED; /*!< 0x00001164 Pause Frames Received */
  un_ETH_FRAMES_RXED_64_t          unFRAMES_RXED_64;   /*!< 0x00001168 64 Byte Frames Received */
  un_ETH_FRAMES_RXED_65_t          unFRAMES_RXED_65;   /*!< 0x0000116C 65 to 127 Byte Frames Received */
  un_ETH_FRAMES_RXED_128_t         unFRAMES_RXED_128;  /*!< 0x00001170 128 to 255 Byte Frames Received */
  un_ETH_FRAMES_RXED_256_t         unFRAMES_RXED_256;  /*!< 0x00001174 256 to 511 Byte Frames Received */
  un_ETH_FRAMES_RXED_512_t         unFRAMES_RXED_512;  /*!< 0x00001178 512 to 1023 Byte Frames Received */
  un_ETH_FRAMES_RXED_1024_t        unFRAMES_RXED_1024; /*!< 0x0000117C 1024 to 1518 Byte Frames Received */
  un_ETH_FRAMES_RXED_1519_t        unFRAMES_RXED_1519; /*!< 0x00001180 1519 to maximum Byte Frames Received */
  un_ETH_UNDERSIZE_FRAMES_t        unUNDERSIZE_FRAMES; /*!< 0x00001184 Undersized Frames Received */
  un_ETH_EXCESSIVE_RX_LENGTH_t     unEXCESSIVE_RX_LENGTH; /*!< 0x00001188 Oversize Frames Received */
  un_ETH_RX_JABBERS_t              unRX_JABBERS;       /*!< 0x0000118C Jabbers Received */
  un_ETH_FCS_ERRORS_t              unFCS_ERRORS;       /*!< 0x00001190 Frame Check Sequence Errors */
  un_ETH_RX_LENGTH_ERRORS_t        unRX_LENGTH_ERRORS; /*!< 0x00001194 Length Field Frame Errors */
  un_ETH_RX_SYMBOL_ERRORS_t        unRX_SYMBOL_ERRORS; /*!< 0x00001198 Receive Symbol Errors */
  un_ETH_ALIGNMENT_ERRORS_t        unALIGNMENT_ERRORS; /*!< 0x0000119C Alignment Errors */
  un_ETH_RX_RESOURCE_ERRORS_t      unRX_RESOURCE_ERRORS; /*!< 0x000011A0 Receive Resource Errors */
  un_ETH_RX_OVERRUNS_t             unRX_OVERRUNS;      /*!< 0x000011A4 Receive Overruns */
  un_ETH_RX_IP_CK_ERRORS_t         unRX_IP_CK_ERRORS;  /*!< 0x000011A8 IP Header Checksum Errors */
  un_ETH_RX_TCP_CK_ERRORS_t        unRX_TCP_CK_ERRORS; /*!< 0x000011AC TCP Checksum Errors */
  un_ETH_RX_UDP_CK_ERRORS_t        unRX_UDP_CK_ERRORS; /*!< 0x000011B0 UDP Checksum Errors */
  un_ETH_AUTO_FLUSHED_PKTS_t       unAUTO_FLUSHED_PKTS; /*!< 0x000011B4 Receive DMA Flushed Packets */
  uint32_t                         u32Reserved3;
  un_ETH_TSU_TIMER_INCR_SUB_NSEC_t unTSU_TIMER_INCR_SUB_NSEC; /*!< 0x000011BC 1588 Timer Increment Register sub nsec */
  un_ETH_TSU_TIMER_MSB_SEC_t       unTSU_TIMER_MSB_SEC; /*!< 0x000011C0 1588 Timer Seconds Register (47 to 32 bits) */
  un_ETH_TSU_STROBE_MSB_SEC_t      unTSU_STROBE_MSB_SEC; /*!< 0x000011C4 1588 Timer Sync Strobe Seconds Register (47 to 32 bits) */
  un_ETH_TSU_STROBE_SEC_t          unTSU_STROBE_SEC;   /*!< 0x000011C8 1588 Timer Sync Strobe Seconds Register (31 to 0 bits) */
  un_ETH_TSU_STROBE_NSEC_t         unTSU_STROBE_NSEC;  /*!< 0x000011CC 1588 Timer Sync Strobe Nanoseconds Register */
  un_ETH_TSU_TIMER_SEC_t           unTSU_TIMER_SEC;    /*!< 0x000011D0 1588 Timer Seconds Register (31 to 0 bits) */
  un_ETH_TSU_TIMER_NSEC_t          unTSU_TIMER_NSEC;   /*!< 0x000011D4 1588 Timer Nanoseconds Register */
  un_ETH_TSU_TIMER_ADJUST_t        unTSU_TIMER_ADJUST; /*!< 0x000011D8 This register is used to adjust the value of the timer
                                                                                in the TSU. It allows an integral number of
                                                                                nanoseconds to be added or subtracted from the
                                                                                timer in a one-off operation. This register
                                                                                returns all zeroes when read. */
  un_ETH_TSU_TIMER_INCR_t          unTSU_TIMER_INCR;   /*!< 0x000011DC 1588 Timer Increment Register */
  un_ETH_TSU_PTP_TX_SEC_t          unTSU_PTP_TX_SEC;   /*!< 0x000011E0 PTP Event Frame Transmitted Seconds Register (31 to 0
                                                                                bits) */
  un_ETH_TSU_PTP_TX_NSEC_t         unTSU_PTP_TX_NSEC;  /*!< 0x000011E4 PTP Event Frame Transmitted Nanoseconds Register */
  un_ETH_TSU_PTP_RX_SEC_t          unTSU_PTP_RX_SEC;   /*!< 0x000011E8 PTP Event Frame Received Seconds Register (31 to 0 bits) */
  un_ETH_TSU_PTP_RX_NSEC_t         unTSU_PTP_RX_NSEC;  /*!< 0x000011EC PTP Event Frame Received Nanoseconds Register */
  un_ETH_TSU_PEER_TX_SEC_t         unTSU_PEER_TX_SEC;  /*!< 0x000011F0 PTP Peer Event Frame Transmitted Seconds Register (31 to
                                                                                0 bits) */
  un_ETH_TSU_PEER_TX_NSEC_t        unTSU_PEER_TX_NSEC; /*!< 0x000011F4 PTP Peer Event Frame Transmitted Nanoseconds Register */
  un_ETH_TSU_PEER_RX_SEC_t         unTSU_PEER_RX_SEC;  /*!< 0x000011F8 PTP Peer Event Frame Received Seconds Register (31 to 0
                                                                                bits) */
  un_ETH_TSU_PEER_RX_NSEC_t        unTSU_PEER_RX_NSEC; /*!< 0x000011FC PTP Peer Event Frame Received Nanoseconds Register */
  un_ETH_PCS_CONTROL_t             unPCS_CONTROL;      /*!< 0x00001200 Not presents. Access to the register returns AHB error. */
  un_ETH_PCS_STATUS_t              unPCS_STATUS;       /*!< 0x00001204 Not presents. Access to the register returns AHB error. */
  uint32_t                         au32Reserved4[2];
  un_ETH_PCS_AN_ADV_t              unPCS_AN_ADV;       /*!< 0x00001210 Not presents. Access to the register returns AHB error. */
  un_ETH_PCS_AN_LP_BASE_t          unPCS_AN_LP_BASE;   /*!< 0x00001214 Not presents. Access to the register returns AHB error. */
  un_ETH_PCS_AN_EXP_t              unPCS_AN_EXP;       /*!< 0x00001218 Not presents. Access to the register returns AHB error. */
  un_ETH_PCS_AN_NP_TX_t            unPCS_AN_NP_TX;     /*!< 0x0000121C Not presents. Access to the register returns AHB error. */
  un_ETH_PCS_AN_LP_NP_t            unPCS_AN_LP_NP;     /*!< 0x00001220 Not presents. Access to the register returns AHB error. */
  uint32_t                         au32Reserved5[6];
  un_ETH_PCS_AN_EXT_STATUS_t       unPCS_AN_EXT_STATUS; /*!< 0x0000123C Not presents. Access to the register returns AHB error. */
  uint32_t                         au32Reserved6[8];
  un_ETH_TX_PAUSE_QUANTUM1_t       unTX_PAUSE_QUANTUM1; /*!< 0x00001260 Transmit Pause Quantum Register 1 */
  un_ETH_TX_PAUSE_QUANTUM2_t       unTX_PAUSE_QUANTUM2; /*!< 0x00001264 Transmit Pause Quantum Register 2 */
  un_ETH_TX_PAUSE_QUANTUM3_t       unTX_PAUSE_QUANTUM3; /*!< 0x00001268 Transmit Pause Quantum Register 3 */
  uint32_t                         u32Reserved7;
  un_ETH_RX_LPI_t                  unRX_LPI;           /*!< 0x00001270 Received LPI transitions */
  un_ETH_RX_LPI_TIME_t             unRX_LPI_TIME;      /*!< 0x00001274 Received LPI time */
  un_ETH_TX_LPI_t                  unTX_LPI;           /*!< 0x00001278 Transmit LPI transitions */
  un_ETH_TX_LPI_TIME_t             unTX_LPI_TIME;      /*!< 0x0000127C Transmit LPI time */
  un_ETH_DESIGNCFG_DEBUG1_t        unDESIGNCFG_DEBUG1; /*!< 0x00001280 The GEM_GXL(3PIP) has many parameterisation options to
                                                                                configure the IP during compilation stage. This
                                                                                is achieved using Verilog define compiler
                                                                                directives in an include file called
                                                                                mxeth_defs.v. */
  un_ETH_DESIGNCFG_DEBUG2_t        unDESIGNCFG_DEBUG2; /*!< 0x00001284 Design Configuration Register 2 */
  un_ETH_DESIGNCFG_DEBUG3_t        unDESIGNCFG_DEBUG3; /*!< 0x00001288 Design Configuration Register 3 */
  un_ETH_DESIGNCFG_DEBUG4_t        unDESIGNCFG_DEBUG4; /*!< 0x0000128C Design Configuration Register 4 */
  un_ETH_DESIGNCFG_DEBUG5_t        unDESIGNCFG_DEBUG5; /*!< 0x00001290 Design Configuration Register 5 */
  un_ETH_DESIGNCFG_DEBUG6_t        unDESIGNCFG_DEBUG6; /*!< 0x00001294 Design Configuration Register 6 */
  un_ETH_DESIGNCFG_DEBUG7_t        unDESIGNCFG_DEBUG7; /*!< 0x00001298 Design Configuration Register 7 */
  un_ETH_DESIGNCFG_DEBUG8_t        unDESIGNCFG_DEBUG8; /*!< 0x0000129C Design Configuration Register 8 */
  un_ETH_DESIGNCFG_DEBUG9_t        unDESIGNCFG_DEBUG9; /*!< 0x000012A0 Design Configuration Register 9 */
  un_ETH_DESIGNCFG_DEBUG10_t       unDESIGNCFG_DEBUG10; /*!< 0x000012A4 Design Configuration Register 10 */
  uint32_t                         au32Reserved8[22];
  un_ETH_SPEC_ADD5_BOTTOM_t        unSPEC_ADD5_BOTTOM; /*!< 0x00001300 Specific address registers 5 ~ 36 doesn't present.
                                                                                Access to the register returns AHB error. */
  un_ETH_SPEC_ADD5_TOP_t           unSPEC_ADD5_TOP;    /*!< 0x00001304 Specific address registers 5 ~ 36 doesn't present.
                                                                                Access to the register returns AHB error. */
  uint32_t                         au32Reserved9[60];
  un_ETH_SPEC_ADD36_BOTTOM_t       unSPEC_ADD36_BOTTOM; /*!< 0x000013F8 Not presents. */
  un_ETH_SPEC_ADD36_TOP_t          unSPEC_ADD36_TOP;   /*!< 0x000013FC Not presents. */
  un_ETH_INT_Q1_STATUS_t           unINT_Q1_STATUS;    /*!< 0x00001400 Priority queue Interrupt Status Register */
  un_ETH_INT_Q2_STATUS_t           unINT_Q2_STATUS;    /*!< 0x00001404 Priority queue Interrupt Status Register */
  un_ETH_INT_Q3_STATUS_t           unINT_Q3_STATUS;    /*!< 0x00001408 int_q3_status to int_q15_status doesn't present. Access
                                                                                to the register returns AHB error. */
  uint32_t                         au32Reserved10[11];
  un_ETH_INT_Q15_STATUS_t          unINT_Q15_STATUS;   /*!< 0x00001438 Not presents. */
  uint32_t                         u32Reserved11;
  un_ETH_TRANSMIT_Q1_PTR_t         unTRANSMIT_Q1_PTR;  /*!< 0x00001440 This register holds the start address of the transmit
                                                                                buffer queue (transmit buffers descriptor list).
                                                                                The transmit buffer queue base address register
                                                                                must be initialized before transmit is started
                                                                                through bit 9 of the network control register.
                                                                                Once transmission has started, any write to the
                                                                                transmit buffer queue base address register is
                                                                                illegal and therefore ignored. Note that due to
                                                                                clock boundary synchronization, it takes a
                                                                                maximum of four pclk cycles from the writing of
                                                                                the transmit start bit before the transmitter is
                                                                                active. Writing to the transmit buffer queue
                                                                                base address register during this time may
                                                                                produce unpredictable results. Reading this
                                                                                register returns the location of the descriptor
                                                                                currently being accessed. Because the DMA can
                                                                                store data for multiple frames at once, this may
                                                                                not necessarily be pointing to the current frame
                                                                                being transmitted. In terms of AMBA AXI
                                                                                operation, the transmit descriptors are written
                                                                                to memory using a single 32bit AHB access. When
                                                                                the datapath is configured as 64bit , the
                                                                                transmit descriptors should be aligned at 64-bit
                                                                                boundaries and each pair of 32-bit descriptors
                                                                                is read from memory using a single AXI access. */
  un_ETH_TRANSMIT_Q2_PTR_t         unTRANSMIT_Q2_PTR;  /*!< 0x00001444 This register holds the start address of the transmit
                                                                                buffer queue (transmit buffers descriptor list).
                                                                                The transmit buffer queue base address register
                                                                                must be initialized before transmit is started
                                                                                through bit 9 of the network control register.
                                                                                Once transmission has started, any write to the
                                                                                transmit buffer queue base address register is
                                                                                illegal and therefore ignored. Note that due to
                                                                                clock boundary synchronization, it takes a
                                                                                maximum of four pclk cycles from the writing of
                                                                                the transmit start bit before the transmitter is
                                                                                active. Writing to the transmit buffer queue
                                                                                base address register during this time may
                                                                                produce unpredictable results. Reading this
                                                                                register returns the location of the descriptor
                                                                                currently being accessed. Because the DMA can
                                                                                store data for multiple frames at once, this may
                                                                                not necessarily be pointing to the current frame
                                                                                being transmitted. In terms of AMBA AXI
                                                                                operation, the transmit descriptors are written
                                                                                to memory using a single 32bit AHB access. When
                                                                                the datapath is configured as 64bit , the
                                                                                transmit descriptors should be aligned at 64-bit
                                                                                boundaries and each pair of 32-bit descriptors
                                                                                is read from memory using a single AXI access. */
  un_ETH_TRANSMIT_Q3_PTR_t         unTRANSMIT_Q3_PTR;  /*!< 0x00001448 transmit_q3_ptr to transmit_q15_ptr doesn't present.
                                                                                Access to the register returns AHB error. */
  uint32_t                         au32Reserved12[11];
  un_ETH_TRANSMIT_Q15_PTR_t        unTRANSMIT_Q15_PTR; /*!< 0x00001478 Not presents. */
  uint32_t                         u32Reserved13;
  un_ETH_RECEIVE_Q1_PTR_t          unRECEIVE_Q1_PTR;   /*!< 0x00001480 This register holds the start address of the transmit
                                                                                buffer queue (transmit buffers descriptor list).
                                                                                The transmit buffer queue base address register
                                                                                must be initialized before transmit is started
                                                                                through bit 9 of the network control register.
                                                                                Once transmission has started, any write to the
                                                                                transmit buffer queue base address register is
                                                                                illegal and therefore ignored. Note that due to
                                                                                clock boundary synchronization, it takes a
                                                                                maximum of four pclk cycles from the writing of
                                                                                the transmit start bit before the transmitter is
                                                                                active. Writing to the transmit buffer queue
                                                                                base address register during this time may
                                                                                produce unpredictable results. Reading this
                                                                                register returns the location of the descriptor
                                                                                currently being accessed. Because the DMA can
                                                                                store data for multiple frames at once, this may
                                                                                not necessarily be pointing to the current frame
                                                                                being transmitted. In terms of AMBA AXI
                                                                                operation, the transmit descriptors are written
                                                                                to memory using a single 32bit AHB access. When
                                                                                the datapath is configured as 64bit , the
                                                                                transmit descriptors should be aligned at 64-bit
                                                                                boundaries and each pair of 32-bit descriptors
                                                                                is read from memory using a single AXI access. */
  un_ETH_RECEIVE_Q2_PTR_t          unRECEIVE_Q2_PTR;   /*!< 0x00001484 This register holds the start address of the transmit
                                                                                buffer queue (transmit buffers descriptor list).
                                                                                The transmit buffer queue base address register
                                                                                must be initialized before transmit is started
                                                                                through bit 9 of the network control register.
                                                                                Once transmission has started, any write to the
                                                                                transmit buffer queue base address register is
                                                                                illegal and therefore ignored. Note that due to
                                                                                clock boundary synchronization, it takes a
                                                                                maximum of four pclk cycles from the writing of
                                                                                the transmit start bit before the transmitter is
                                                                                active. Writing to the transmit buffer queue
                                                                                base address register during this time may
                                                                                produce unpredictable results. Reading this
                                                                                register returns the location of the descriptor
                                                                                currently being accessed. Because the DMA can
                                                                                store data for multiple frames at once, this may
                                                                                not necessarily be pointing to the current frame
                                                                                being transmitted. In terms of AMBA AXI
                                                                                operation, the transmit descriptors are written
                                                                                to memory using a single 32bit AHB access. When
                                                                                the datapath is configured as 64bit , the
                                                                                transmit descriptors should be aligned at 64-bit
                                                                                boundaries and each pair of 32-bit descriptors
                                                                                is read from memory using a single AXI access. */
  un_ETH_RECEIVE_Q3_PTR_t          unRECEIVE_Q3_PTR;   /*!< 0x00001488 Not presents. Start address register doesn't present for
                                                                                queue3 ~ queue7. */
  uint32_t                         au32Reserved14[3];
  un_ETH_RECEIVE_Q7_PTR_t          unRECEIVE_Q7_PTR;   /*!< 0x00001498 Not presents. */
  uint32_t                         u32Reserved15;
  un_ETH_DMA_RXBUF_SIZE_Q1_t       unDMA_RXBUF_SIZE_Q1; /*!< 0x000014A0 Receive Buffer queue 1 Size */
  un_ETH_DMA_RXBUF_SIZE_Q2_t       unDMA_RXBUF_SIZE_Q2; /*!< 0x000014A4 Receive Buffer queue 2 Size */
  un_ETH_DMA_RXBUF_SIZE_Q3_t       unDMA_RXBUF_SIZE_Q3; /*!< 0x000014A8 dma_rxbuf_size_q3 to dma_rxbuf_size_q7 doesn't present. */
  uint32_t                         au32Reserved16[3];
  un_ETH_DMA_RXBUF_SIZE_Q7_t       unDMA_RXBUF_SIZE_Q7; /*!< 0x000014B8 Not presents. */
  un_ETH_CBS_CONTROL_t             unCBS_CONTROL;      /*!< 0x000014BC The IdleSlope value is defined as the rate of change of
                                                                                credit when a packet is waiting to be sent. This
                                                                                must not exceed the portTransmitRate which is
                                                                                dependent on the speed of operation, eg,
                                                                                portTranmsitRate. 1Gb/s = 32'h07735940 (125
                                                                                Mbytes/s), 100Mb/sec = 32'h017D7840 (25
                                                                                Mnibbles/s), 10Mb/sec = 32'h002625A0 (2.5
                                                                                Mnibbles/s). If 50 percent of bandwidth was to
                                                                                be allocated to a particular queue in 1Gb/sec
                                                                                mode then the IdleSlope value for that queue
                                                                                would be calculated as 32'h07735940/2. Note that
                                                                                Credit-Based Shaping should be disabled prior to
                                                                                updating the IdleSlope values. As another
                                                                                example, for a 1722 audio packet with a payload
                                                                                of 6 samples per channel, the packet size would
                                                                                be 7 (preamble) + 1 (SFD) + 50 (packet header) +
                                                                                6x4x2(payload) + 4 (CRC) = 110 bytes. For a rate
                                                                                of 8000 packets per second, the desired rate
                                                                                would 110 x 8000 bytes per second, so the
                                                                                programmed idleSlope value would be 880000 for a
                                                                                1Gb/s connection, or 1760000 for a 100Mb/s or
                                                                                10Mbs connection. See Figure 6.3 in the IEEE
                                                                                1722 standard. In practice, the actual
                                                                                transmission rate will be vary slightly from
                                                                                that calculated. In this case, the idleSlope
                                                                                value should be recalibrated based on the
                                                                                variance between the measured and expected rate,
                                                                                and in this case very accurate transmission
                                                                                rates can be achieved. */
  un_ETH_CBS_IDLESLOPE_Q_A_t       unCBS_IDLESLOPE_Q_A; /*!< 0x000014C0 queue A is the highest priority queue. This would be
                                                                                queue 8 in an 8 queue configuration. */
  un_ETH_CBS_IDLESLOPE_Q_B_t       unCBS_IDLESLOPE_Q_B; /*!< 0x000014C4 queue B is the 2nd highest priority queue. This would be
                                                                                queue 7 in an 8 queue configuration. */
  un_ETH_UPPER_TX_Q_BASE_ADDR_t    unUPPER_TX_Q_BASE_ADDR; /*!< 0x000014C8 Upper 32 bits of transmit buffer descriptor queue base
                                                                                address. */
  un_ETH_TX_BD_CONTROL_t           unTX_BD_CONTROL;    /*!< 0x000014CC TX BD control register */
  un_ETH_RX_BD_CONTROL_t           unRX_BD_CONTROL;    /*!< 0x000014D0 RX BD control register */
  un_ETH_UPPER_RX_Q_BASE_ADDR_t    unUPPER_RX_Q_BASE_ADDR; /*!< 0x000014D4 Upper 32 bits of receive buffer descriptor queue base
                                                                                address. */
  uint32_t                         au32Reserved17[2];
  un_ETH_HIDDEN_REG0_t             unHIDDEN_REG0;      /*!< 0x000014E0 Hidden registers defined in edma_defs.v '`define
                                                                                gem_cbs_port_tx_rate_10m 12'h4e0 // 10M Port TX
                                                                                Rate *** HIDDEN Register ***'. Default value of
                                                                                cbs related hidden registers (0x14E0~0x14E8) are
                                                                                depicted in cbs_control register. */
  un_ETH_HIDDEN_REG1_t             unHIDDEN_REG1;      /*!< 0x000014E4 Hidden registers defined in edma_defs.v '`define
                                                                                gem_cbs_port_tx_rate_100m 12'h4e4 // 100M Port
                                                                                TX Rate *** HIDDEN Register ***' */
  un_ETH_HIDDEN_REG2_t             unHIDDEN_REG2;      /*!< 0x000014E8 Hidden registers defined in edma_defs.v '`define
                                                                                gem_cbs_port_tx_rate_1g 12'h4e8 // 1G Port TX
                                                                                Rate *** HIDDEN Register ***' */
  un_ETH_HIDDEN_REG3_t             unHIDDEN_REG3;      /*!< 0x000014EC Hidden registers defined in edma_defs.v '`define
                                                                                gem_wd_counter 12'h4ec // *** HIDDEN Register
                                                                                ***'. */
  uint32_t                         au32Reserved18[2];
  un_ETH_HIDDEN_REG4_t             unHIDDEN_REG4;      /*!< 0x000014F8 Hidden registers defined in edma_defs.v '`define
                                                                                gem_axi_tx_full_threshold0 12'h4f8 // AXI full
                                                                                threshold setting *** HIDDEN Register ***'.
                                                                                Note. When using AXI mode with a single port ram
                                                                                ( gem_spram == 1) mode and a 32b dma bus width (
                                                                                gem_dma_bus_width == 32 or bits 22 to 21 of the
                                                                                network_config register are set to 0) the AXI
                                                                                hidden registers (0x14F8 and 0x14FC) need to be
                                                                                updated (these registers are used for fine
                                                                                tuning AXI dma bursts and normally should not be
                                                                                touched). */
  un_ETH_HIDDEN_REG5_t             unHIDDEN_REG5;      /*!< 0x000014FC Hidden registers defined in edma_defs.v '`define
                                                                                gem_axi_tx_full_threshold1 12'h4fc // AXI full
                                                                                threshold setting *** HIDDEN Register ***'. */
  un_ETH_SCREENING_TYPE_1_REGISTER_0_t unSCREENING_TYPE_1_REGISTER_0; /*!< 0x00001500 Screening type 1 registers are used to allocate up to 16
                                                                                priority queues to received frames based on
                                                                                certain IP or UDP fields of incoming frames.
                                                                                Firstly, when DS/TC match enable is set (bit
                                                                                28), the DS (Differentiated Services) field of
                                                                                the received IPv4 header or TCfield (traffic
                                                                                class) of IPv6 headers are matched against bits
                                                                                11 to 4. Secondly, when UDP port match enable is
                                                                                set (bit 29), the UDP Destination Port of the
                                                                                received UDP frame is matched against bits 27 to
                                                                                12. Both UDP and DS/TC matching can be enabled
                                                                                simultaneously or individually. If a match is
                                                                                successful, then the queue value programmed in
                                                                                bits 2 to 0 is allocated to the frame. The
                                                                                required number of Type 1 screening registers is
                                                                                configured in the gem defines file. Up to 16
                                                                                type 1 screening registers have been allocated
                                                                                APB address space between 0x500 and 0x53C. */
  un_ETH_SCREENING_TYPE_1_REGISTER_1_t unSCREENING_TYPE_1_REGISTER_1; /*!< 0x00001504 screening type 1 register 1, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_1_REGISTER_2_t unSCREENING_TYPE_1_REGISTER_2; /*!< 0x00001508 screening type 1 register 2, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_1_REGISTER_3_t unSCREENING_TYPE_1_REGISTER_3; /*!< 0x0000150C screening type 1 register 3, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_1_REGISTER_4_t unSCREENING_TYPE_1_REGISTER_4; /*!< 0x00001510 screening type 1 register 4, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_1_REGISTER_5_t unSCREENING_TYPE_1_REGISTER_5; /*!< 0x00001514 screening type 1 register 5, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_1_REGISTER_6_t unSCREENING_TYPE_1_REGISTER_6; /*!< 0x00001518 screening type 1 register 6, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_1_REGISTER_7_t unSCREENING_TYPE_1_REGISTER_7; /*!< 0x0000151C screening type 1 register 7, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_1_REGISTER_8_t unSCREENING_TYPE_1_REGISTER_8; /*!< 0x00001520 screening type 1 register 8, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_1_REGISTER_9_t unSCREENING_TYPE_1_REGISTER_9; /*!< 0x00001524 screening type 1 register 9, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_1_REGISTER_10_t unSCREENING_TYPE_1_REGISTER_10; /*!< 0x00001528 screening type 1 register 10, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_1_REGISTER_11_t unSCREENING_TYPE_1_REGISTER_11; /*!< 0x0000152C screening type 1 register 11, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_1_REGISTER_12_t unSCREENING_TYPE_1_REGISTER_12; /*!< 0x00001530 screening type 1 register 12, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_1_REGISTER_13_t unSCREENING_TYPE_1_REGISTER_13; /*!< 0x00001534 screening type 1 register 13, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_1_REGISTER_14_t unSCREENING_TYPE_1_REGISTER_14; /*!< 0x00001538 screening type 1 register 14, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_1_REGISTER_15_t unSCREENING_TYPE_1_REGISTER_15; /*!< 0x0000153C screening type 1 register 15, same as
                                                                                screening_type_1_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_0_t unSCREENING_TYPE_2_REGISTER_0; /*!< 0x00001540 Screener Type 2 match registers operate independently of
                                                                                screener type 1 registers and offer additional
                                                                                match capabilities, extending the capabilities
                                                                                into vendor specific protocols. */
  un_ETH_SCREENING_TYPE_2_REGISTER_1_t unSCREENING_TYPE_2_REGISTER_1; /*!< 0x00001544 screening type 2 register 1, same as
                                                                                screening_type_2_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_2_t unSCREENING_TYPE_2_REGISTER_2; /*!< 0x00001548 screening type 2 register 2, same as
                                                                                screening_type_2_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_3_t unSCREENING_TYPE_2_REGISTER_3; /*!< 0x0000154C screening type 2 register 3, same as
                                                                                screening_type_2_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_4_t unSCREENING_TYPE_2_REGISTER_4; /*!< 0x00001550 screening type 2 register 4, same as
                                                                                screening_type_2_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_5_t unSCREENING_TYPE_2_REGISTER_5; /*!< 0x00001554 screening type 2 register 5, same as
                                                                                screening_type_2_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_6_t unSCREENING_TYPE_2_REGISTER_6; /*!< 0x00001558 screening type 2 register 6, same as
                                                                                screening_type_2_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_7_t unSCREENING_TYPE_2_REGISTER_7; /*!< 0x0000155C screening type 2 register 7, same as
                                                                                screening_type_2_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_8_t unSCREENING_TYPE_2_REGISTER_8; /*!< 0x00001560 screening type 2 register 8, same as
                                                                                screening_type_2_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_9_t unSCREENING_TYPE_2_REGISTER_9; /*!< 0x00001564 screening type 2 register 9, same as
                                                                                screening_type_2_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_10_t unSCREENING_TYPE_2_REGISTER_10; /*!< 0x00001568 screening type 2 register 10, same as
                                                                                screening_type_2_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_11_t unSCREENING_TYPE_2_REGISTER_11; /*!< 0x0000156C screening type 2 register 11, same as
                                                                                screening_type_2_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_12_t unSCREENING_TYPE_2_REGISTER_12; /*!< 0x00001570 screening type 2 register 12, same as
                                                                                screening_type_2_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_13_t unSCREENING_TYPE_2_REGISTER_13; /*!< 0x00001574 screening type 2 register 13, same as
                                                                                screening_type_2_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_14_t unSCREENING_TYPE_2_REGISTER_14; /*!< 0x00001578 screening type 2 register 14, same as
                                                                                screening_type_2_register_0 */
  un_ETH_SCREENING_TYPE_2_REGISTER_15_t unSCREENING_TYPE_2_REGISTER_15; /*!< 0x0000157C screening type 2 register 15, same as
                                                                                screening_type_2_register_0 */
  un_ETH_TX_SCHED_CTRL_t           unTX_SCHED_CTRL;    /*!< 0x00001580 This register controls the transmit scheduling algorithm
                                                                                the user can select for each active transmit
                                                                                queue. By default all queues are initialized to
                                                                                fixed priority, with the top indexed queue
                                                                                having overall priority */
  uint32_t                         au32Reserved19[3];
  un_ETH_BW_RATE_LIMIT_Q0TO3_t     unBW_RATE_LIMIT_Q0TO3; /*!< 0x00001590 This register holds the DWRR weighting value or the ETS
                                                                                bandwidth percentage value used by the transmit
                                                                                scheduler for queues 0 to 3. */
  un_ETH_BW_RATE_LIMIT_Q4TO7_t     unBW_RATE_LIMIT_Q4TO7; /*!< 0x00001594 Not presents. MXETH has only 3 queues. Access to the
                                                                                register returns AHB error. */
  un_ETH_BW_RATE_LIMIT_Q8TO11_t    unBW_RATE_LIMIT_Q8TO11; /*!< 0x00001598 Not presents. MXETH has only 3 queues. Access to the
                                                                                register returns AHB error. */
  un_ETH_BW_RATE_LIMIT_Q12TO15_t   unBW_RATE_LIMIT_Q12TO15; /*!< 0x0000159C Not presents. MXETH has only 3 queues. Access to the
                                                                                register returns AHB error. */
  un_ETH_TX_Q_SEG_ALLOC_Q0TO7_t    unTX_Q_SEG_ALLOC_Q0TO7; /*!< 0x000015A0 This register allows the user to distribute the Transmit
                                                                                SRAM used by the DMA across the priority queues,
                                                                                for queues 0 to 7. The SRAM itself is split into
                                                                                a number of evenly sized segments (this is
                                                                                defined in the verilog configuration defs file -
                                                                                for the configuration used to generate this
                                                                                register description, the total number of
                                                                                segments was set to '16'). Those segments can
                                                                                then be freely distributed across the active
                                                                                queues, in powers of 2. I.e. a value of 0 would
                                                                                mean 1 segment has been allocated to the queue.
                                                                                A value of 1 would mean 2 segments, a value of 2
                                                                                means 4 segments and so on. The reset values of
                                                                                these registers are defined in the configuration
                                                                                defs file. */
  un_ETH_TX_Q_SEG_ALLOC_Q8TO15_t   unTX_Q_SEG_ALLOC_Q8TO15; /*!< 0x000015A4 Not presents. Access to the register returns AHB error. */
  uint32_t                         au32Reserved20[6];
  un_ETH_RECEIVE_Q8_PTR_t          unRECEIVE_Q8_PTR;   /*!< 0x000015C0 receive_q8_ptr to receive_q15_ptr doesn't present.
                                                                                Access to the register returns AHB error. */
  uint32_t                         au32Reserved21[6];
  un_ETH_RECEIVE_Q15_PTR_t         unRECEIVE_Q15_PTR;  /*!< 0x000015DC Not presents. */
  un_ETH_DMA_RXBUF_SIZE_Q8_t       unDMA_RXBUF_SIZE_Q8; /*!< 0x000015E0 dma_rxbuf_size_q8 to dma_rxbuf_size_q15 doesn't present.
                                                                                Access to the register returns AHB error. */
  uint32_t                         au32Reserved22[6];
  un_ETH_DMA_RXBUF_SIZE_Q15_t      unDMA_RXBUF_SIZE_Q15; /*!< 0x000015FC Not presents. */
  un_ETH_INT_Q1_ENABLE_t           unINT_Q1_ENABLE;    /*!< 0x00001600 At reset all interrupts are disabled. Writing a one to
                                                                                the relevant bit location enables the required
                                                                                interrupt. This register is write only and when
                                                                                read will return zero. */
  un_ETH_INT_Q2_ENABLE_t           unINT_Q2_ENABLE;    /*!< 0x00001604 At reset all interrupts are disabled. Writing a one to
                                                                                the relevant bit location enables the required
                                                                                interrupt. This register is write only and when
                                                                                read will return zero. */
  un_ETH_INT_Q3_ENABLE_t           unINT_Q3_ENABLE;    /*!< 0x00001608 int_q3_enable to int_q7_enable doesn't present. Access
                                                                                to the register returns AHB error. */
  uint32_t                         au32Reserved23[3];
  un_ETH_INT_Q7_ENABLE_t           unINT_Q7_ENABLE;    /*!< 0x00001618 Not presents. */
  uint32_t                         u32Reserved24;
  un_ETH_INT_Q1_DISABLE_t          unINT_Q1_DISABLE;   /*!< 0x00001620 Writing a 1 to the relevant bit location disables that
                                                                                particular interrupt. This register is write
                                                                                only and when read will return zero. */
  un_ETH_INT_Q2_DISABLE_t          unINT_Q2_DISABLE;   /*!< 0x00001624 Writing a 1 to the relevant bit location disables that
                                                                                particular interrupt. This register is write
                                                                                only and when read will return zero. */
  un_ETH_INT_Q3_DISABLE_t          unINT_Q3_DISABLE;   /*!< 0x00001628 int_q3_disable to int_q7_disable doesn't present. Access
                                                                                to the register returns AHB error. */
  uint32_t                         au32Reserved25[3];
  un_ETH_INT_Q7_DISABLE_t          unINT_Q7_DISABLE;   /*!< 0x00001638 Not presents. */
  uint32_t                         u32Reserved26;
  un_ETH_INT_Q1_MASK_t             unINT_Q1_MASK;      /*!< 0x00001640 The interrupt mask register is a read only register
                                                                                indicating which interrupts are masked. All bits
                                                                                are set at reset and can be reset individually
                                                                                by writing to the interrupt enable register or
                                                                                set individually by writing to the interrupt
                                                                                disable register. Having separate address
                                                                                locations for enable and disable saves the need
                                                                                for performing a read modify write when updating
                                                                                the interrupt mask register. For test purposes
                                                                                there is a write-only function to this register
                                                                                that allows the bits in the interrupt status
                                                                                register to be set or cleared, regardless of the
                                                                                state of the mask register. */
  un_ETH_INT_Q2_MASK_t             unINT_Q2_MASK;      /*!< 0x00001644 The interrupt mask register is a read only register
                                                                                indicating which interrupts are masked. All bits
                                                                                are set at reset and can be reset individually
                                                                                by writing to the interrupt enable register or
                                                                                set individually by writing to the interrupt
                                                                                disable register. Having separate address
                                                                                locations for enable and disable saves the need
                                                                                for performing a read modify write when updating
                                                                                the interrupt mask register. For test purposes
                                                                                there is a write-only function to this register
                                                                                that allows the bits in the interrupt status
                                                                                register to be set or cleared, regardless of the
                                                                                state of the mask register. */
  un_ETH_INT_Q3_MASK_t             unINT_Q3_MASK;      /*!< 0x00001648 int_q3_mask to int_q7_mask doesn't present. Access to
                                                                                the register returns AHB error. */
  uint32_t                         au32Reserved27[3];
  un_ETH_INT_Q7_MASK_t             unINT_Q7_MASK;      /*!< 0x00001658 Not presents. */
  uint32_t                         u32Reserved28;
  un_ETH_INT_Q8_ENABLE_t           unINT_Q8_ENABLE;    /*!< 0x00001660 int_q8_enable to int_q15_enable doesn't present. Access
                                                                                to the register returns AHB error. */
  uint32_t                         au32Reserved29[6];
  un_ETH_INT_Q15_ENABLE_t          unINT_Q15_ENABLE;   /*!< 0x0000167C Not presents. */
  un_ETH_INT_Q8_DISABLE_t          unINT_Q8_DISABLE;   /*!< 0x00001680 int_q8_disable to int_q15_disable doesn't present.
                                                                                Access to the register returns AHB error. */
  uint32_t                         au32Reserved30[6];
  un_ETH_INT_Q15_DISABLE_t         unINT_Q15_DISABLE;  /*!< 0x0000169C Not presents. */
  un_ETH_INT_Q8_MASK_t             unINT_Q8_MASK;      /*!< 0x000016A0 int_q8_mask to int_q15_mask doesn't present. Access to
                                                                                the register returns AHB error. */
  uint32_t                         au32Reserved31[6];
  un_ETH_INT_Q15_MASK_t            unINT_Q15_MASK;     /*!< 0x000016BC Not presents. */
  uint32_t                         au32Reserved32[8];
  un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_0_t unSCREENING_TYPE_2_ETHERTYPE_REG_0; /*!< 0x000016E0 Ethertype Register */
  un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_1_t unSCREENING_TYPE_2_ETHERTYPE_REG_1; /*!< 0x000016E4 Ethertype Register */
  un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_2_t unSCREENING_TYPE_2_ETHERTYPE_REG_2; /*!< 0x000016E8 Ethertype Register */
  un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_3_t unSCREENING_TYPE_2_ETHERTYPE_REG_3; /*!< 0x000016EC Ethertype Register */
  un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_4_t unSCREENING_TYPE_2_ETHERTYPE_REG_4; /*!< 0x000016F0 Ethertype Register */
  un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_5_t unSCREENING_TYPE_2_ETHERTYPE_REG_5; /*!< 0x000016F4 Ethertype Register */
  un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_6_t unSCREENING_TYPE_2_ETHERTYPE_REG_6; /*!< 0x000016F8 Ethertype Register */
  un_ETH_SCREENING_TYPE_2_ETHERTYPE_REG_7_t unSCREENING_TYPE_2_ETHERTYPE_REG_7; /*!< 0x000016FC Ethertype Register */
  un_ETH_TYPE2_COMPARE_0_WORD_0_t  unTYPE2_COMPARE_0_WORD_0; /*!< 0x00001700 'Compare A, B and C fields of the screener type 2 match
                                                                                register are pointers to a pool of up to 32
                                                                                compare registers. If enabled the compare is
                                                                                true if the data at the OFFSET into the frame,
                                                                                ANDed with the MASK Value if the mask is
                                                                                enabled, is equal to the COMPARE Value. Either a
                                                                                16 bit comparison or a 32 bit comparison is
                                                                                done. This selection is made via the associated
                                                                                compare word1 register bit 9. If a 16 bit
                                                                                comparison is selected, then a 16 bit mask is
                                                                                also available to the user to select which bits
                                                                                should be compared. If the 32 bit compare option
                                                                                is selected, then no mask is available. The byte
                                                                                at the OFFSET number of bytes from the index
                                                                                start is compared thru bits 7 to 0 of the
                                                                                configured VALUE. The byte at the OFFSET number
                                                                                of bytes + 1 from the index start is compared
                                                                                thru bits 15 to 8 of the configured VALUE and so
                                                                                on. The OFFSET can be configured to be from 0 to
                                                                                127 bytes from either the start of the frame,
                                                                                the byte following the therType field (last
                                                                                EtherType in the header if the frame is VLAN
                                                                                tagged), the byte following the IP header (IPv4
                                                                                or IPv6) or from the byte following the start of
                                                                                the TCP/UDP header. The required number of Type
                                                                                2 screening registers up to a maximum of 32 is
                                                                                configurable in the gem defines file and have
                                                                                been allocated APB address space between 0x700
                                                                                and 0x7fc. Note. when using RX Partial Store and
                                                                                Forward mode and priority queues, the frame
                                                                                offset must be less than the Partial Store and
                                                                                Forward watermark. If the offset is higher than
                                                                                the watermark value it's not possible to
                                                                                identify the priority queue before the frame is
                                                                                sent to the AMBA interface, and an incorrect
                                                                                priority queue may be used. ' */
  un_ETH_TYPE2_COMPARE_0_WORD_1_t  unTYPE2_COMPARE_0_WORD_1; /*!< 0x00001704 'Type2 Compare Word 1' */
  un_ETH_TYPE2_COMPARE_1_WORD_0_t  unTYPE2_COMPARE_1_WORD_0; /*!< 0x00001708 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_1_WORD_1_t  unTYPE2_COMPARE_1_WORD_1; /*!< 0x0000170C same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_2_WORD_0_t  unTYPE2_COMPARE_2_WORD_0; /*!< 0x00001710 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_2_WORD_1_t  unTYPE2_COMPARE_2_WORD_1; /*!< 0x00001714 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_3_WORD_0_t  unTYPE2_COMPARE_3_WORD_0; /*!< 0x00001718 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_3_WORD_1_t  unTYPE2_COMPARE_3_WORD_1; /*!< 0x0000171C same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_4_WORD_0_t  unTYPE2_COMPARE_4_WORD_0; /*!< 0x00001720 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_4_WORD_1_t  unTYPE2_COMPARE_4_WORD_1; /*!< 0x00001724 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_5_WORD_0_t  unTYPE2_COMPARE_5_WORD_0; /*!< 0x00001728 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_5_WORD_1_t  unTYPE2_COMPARE_5_WORD_1; /*!< 0x0000172C same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_6_WORD_0_t  unTYPE2_COMPARE_6_WORD_0; /*!< 0x00001730 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_6_WORD_1_t  unTYPE2_COMPARE_6_WORD_1; /*!< 0x00001734 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_7_WORD_0_t  unTYPE2_COMPARE_7_WORD_0; /*!< 0x00001738 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_7_WORD_1_t  unTYPE2_COMPARE_7_WORD_1; /*!< 0x0000173C same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_8_WORD_0_t  unTYPE2_COMPARE_8_WORD_0; /*!< 0x00001740 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_8_WORD_1_t  unTYPE2_COMPARE_8_WORD_1; /*!< 0x00001744 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_9_WORD_0_t  unTYPE2_COMPARE_9_WORD_0; /*!< 0x00001748 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_9_WORD_1_t  unTYPE2_COMPARE_9_WORD_1; /*!< 0x0000174C same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_10_WORD_0_t unTYPE2_COMPARE_10_WORD_0; /*!< 0x00001750 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_10_WORD_1_t unTYPE2_COMPARE_10_WORD_1; /*!< 0x00001754 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_11_WORD_0_t unTYPE2_COMPARE_11_WORD_0; /*!< 0x00001758 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_11_WORD_1_t unTYPE2_COMPARE_11_WORD_1; /*!< 0x0000175C same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_12_WORD_0_t unTYPE2_COMPARE_12_WORD_0; /*!< 0x00001760 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_12_WORD_1_t unTYPE2_COMPARE_12_WORD_1; /*!< 0x00001764 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_13_WORD_0_t unTYPE2_COMPARE_13_WORD_0; /*!< 0x00001768 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_13_WORD_1_t unTYPE2_COMPARE_13_WORD_1; /*!< 0x0000176C same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_14_WORD_0_t unTYPE2_COMPARE_14_WORD_0; /*!< 0x00001770 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_14_WORD_1_t unTYPE2_COMPARE_14_WORD_1; /*!< 0x00001774 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_15_WORD_0_t unTYPE2_COMPARE_15_WORD_0; /*!< 0x00001778 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_15_WORD_1_t unTYPE2_COMPARE_15_WORD_1; /*!< 0x0000177C same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_16_WORD_0_t unTYPE2_COMPARE_16_WORD_0; /*!< 0x00001780 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_16_WORD_1_t unTYPE2_COMPARE_16_WORD_1; /*!< 0x00001784 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_17_WORD_0_t unTYPE2_COMPARE_17_WORD_0; /*!< 0x00001788 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_17_WORD_1_t unTYPE2_COMPARE_17_WORD_1; /*!< 0x0000178C same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_18_WORD_0_t unTYPE2_COMPARE_18_WORD_0; /*!< 0x00001790 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_18_WORD_1_t unTYPE2_COMPARE_18_WORD_1; /*!< 0x00001794 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_19_WORD_0_t unTYPE2_COMPARE_19_WORD_0; /*!< 0x00001798 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_19_WORD_1_t unTYPE2_COMPARE_19_WORD_1; /*!< 0x0000179C same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_20_WORD_0_t unTYPE2_COMPARE_20_WORD_0; /*!< 0x000017A0 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_20_WORD_1_t unTYPE2_COMPARE_20_WORD_1; /*!< 0x000017A4 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_21_WORD_0_t unTYPE2_COMPARE_21_WORD_0; /*!< 0x000017A8 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_21_WORD_1_t unTYPE2_COMPARE_21_WORD_1; /*!< 0x000017AC same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_22_WORD_0_t unTYPE2_COMPARE_22_WORD_0; /*!< 0x000017B0 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_22_WORD_1_t unTYPE2_COMPARE_22_WORD_1; /*!< 0x000017B4 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_23_WORD_0_t unTYPE2_COMPARE_23_WORD_0; /*!< 0x000017B8 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_23_WORD_1_t unTYPE2_COMPARE_23_WORD_1; /*!< 0x000017BC same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_24_WORD_0_t unTYPE2_COMPARE_24_WORD_0; /*!< 0x000017C0 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_24_WORD_1_t unTYPE2_COMPARE_24_WORD_1; /*!< 0x000017C4 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_25_WORD_0_t unTYPE2_COMPARE_25_WORD_0; /*!< 0x000017C8 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_25_WORD_1_t unTYPE2_COMPARE_25_WORD_1; /*!< 0x000017CC same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_26_WORD_0_t unTYPE2_COMPARE_26_WORD_0; /*!< 0x000017D0 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_26_WORD_1_t unTYPE2_COMPARE_26_WORD_1; /*!< 0x000017D4 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_27_WORD_0_t unTYPE2_COMPARE_27_WORD_0; /*!< 0x000017D8 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_27_WORD_1_t unTYPE2_COMPARE_27_WORD_1; /*!< 0x000017DC same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_28_WORD_0_t unTYPE2_COMPARE_28_WORD_0; /*!< 0x000017E0 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_28_WORD_1_t unTYPE2_COMPARE_28_WORD_1; /*!< 0x000017E4 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_29_WORD_0_t unTYPE2_COMPARE_29_WORD_0; /*!< 0x000017E8 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_29_WORD_1_t unTYPE2_COMPARE_29_WORD_1; /*!< 0x000017EC same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_30_WORD_0_t unTYPE2_COMPARE_30_WORD_0; /*!< 0x000017F0 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_30_WORD_1_t unTYPE2_COMPARE_30_WORD_1; /*!< 0x000017F4 same as type2_compare_0_word_1 */
  un_ETH_TYPE2_COMPARE_31_WORD_0_t unTYPE2_COMPARE_31_WORD_0; /*!< 0x000017F8 same as type2_compare_0_word_0 */
  un_ETH_TYPE2_COMPARE_31_WORD_1_t unTYPE2_COMPARE_31_WORD_1; /*!< 0x000017FC same as type2_compare_0_word_1 */
} stc_ETH_t;                                           /*!< Size = 6144 (0x1800) */


/* ETH.CTL */
#define ETH_CTL_ETH_MODE_Pos                    0UL
#define ETH_CTL_ETH_MODE_Msk                    0x3UL
#define ETH_CTL_REFCLK_SRC_SEL_Pos              2UL
#define ETH_CTL_REFCLK_SRC_SEL_Msk              0x4UL
#define ETH_CTL_REFCLK_DIV_Pos                  8UL
#define ETH_CTL_REFCLK_DIV_Msk                  0xFF00UL
#define ETH_CTL_ENABLED_Pos                     31UL
#define ETH_CTL_ENABLED_Msk                     0x80000000UL
/* ETH.STATUS */
#define ETH_STATUS_PFC_NEGOTIATE_Pos            0UL
#define ETH_STATUS_PFC_NEGOTIATE_Msk            0x1UL
#define ETH_STATUS_RX_PFC_PAUSED_Pos            8UL
#define ETH_STATUS_RX_PFC_PAUSED_Msk            0xFF00UL
/* ETH.NETWORK_CONTROL */
#define ETH_NETWORK_CONTROL_LOOPBACK_Pos        0UL
#define ETH_NETWORK_CONTROL_LOOPBACK_Msk        0x1UL
#define ETH_NETWORK_CONTROL_LOOPBACK_LOCAL_Pos  1UL
#define ETH_NETWORK_CONTROL_LOOPBACK_LOCAL_Msk  0x2UL
#define ETH_NETWORK_CONTROL_ENABLE_RECEIVE_Pos  2UL
#define ETH_NETWORK_CONTROL_ENABLE_RECEIVE_Msk  0x4UL
#define ETH_NETWORK_CONTROL_ENABLE_TRANSMIT_Pos 3UL
#define ETH_NETWORK_CONTROL_ENABLE_TRANSMIT_Msk 0x8UL
#define ETH_NETWORK_CONTROL_MAN_PORT_EN_Pos     4UL
#define ETH_NETWORK_CONTROL_MAN_PORT_EN_Msk     0x10UL
#define ETH_NETWORK_CONTROL_CLEAR_ALL_STATS_REGS_Pos 5UL
#define ETH_NETWORK_CONTROL_CLEAR_ALL_STATS_REGS_Msk 0x20UL
#define ETH_NETWORK_CONTROL_INC_ALL_STATS_REGS_Pos 6UL
#define ETH_NETWORK_CONTROL_INC_ALL_STATS_REGS_Msk 0x40UL
#define ETH_NETWORK_CONTROL_STATS_WRITE_EN_Pos  7UL
#define ETH_NETWORK_CONTROL_STATS_WRITE_EN_Msk  0x80UL
#define ETH_NETWORK_CONTROL_BACK_PRESSURE_Pos   8UL
#define ETH_NETWORK_CONTROL_BACK_PRESSURE_Msk   0x100UL
#define ETH_NETWORK_CONTROL_TX_START_PCLK_Pos   9UL
#define ETH_NETWORK_CONTROL_TX_START_PCLK_Msk   0x200UL
#define ETH_NETWORK_CONTROL_TX_HALT_PCLK_Pos    10UL
#define ETH_NETWORK_CONTROL_TX_HALT_PCLK_Msk    0x400UL
#define ETH_NETWORK_CONTROL_TX_PAUSE_FRAME_REQ_Pos 11UL
#define ETH_NETWORK_CONTROL_TX_PAUSE_FRAME_REQ_Msk 0x800UL
#define ETH_NETWORK_CONTROL_TX_PAUSE_FRAME_ZERO_Pos 12UL
#define ETH_NETWORK_CONTROL_TX_PAUSE_FRAME_ZERO_Msk 0x1000UL
#define ETH_NETWORK_CONTROL_REMOVED_13_Pos      13UL
#define ETH_NETWORK_CONTROL_REMOVED_13_Msk      0x2000UL
#define ETH_NETWORK_CONTROL_REMOVED_14_Pos      14UL
#define ETH_NETWORK_CONTROL_REMOVED_14_Msk      0x4000UL
#define ETH_NETWORK_CONTROL_STORE_RX_TS_Pos     15UL
#define ETH_NETWORK_CONTROL_STORE_RX_TS_Msk     0x8000UL
#define ETH_NETWORK_CONTROL_PFC_ENABLE_Pos      16UL
#define ETH_NETWORK_CONTROL_PFC_ENABLE_Msk      0x10000UL
#define ETH_NETWORK_CONTROL_TRANSMIT_PFC_PRIORITY_BASED_PAUSE_FRAME_Pos 17UL
#define ETH_NETWORK_CONTROL_TRANSMIT_PFC_PRIORITY_BASED_PAUSE_FRAME_Msk 0x20000UL
#define ETH_NETWORK_CONTROL_FLUSH_RX_PKT_PCLK_Pos 18UL
#define ETH_NETWORK_CONTROL_FLUSH_RX_PKT_PCLK_Msk 0x40000UL
#define ETH_NETWORK_CONTROL_TX_LPI_EN_Pos       19UL
#define ETH_NETWORK_CONTROL_TX_LPI_EN_Msk       0x80000UL
#define ETH_NETWORK_CONTROL_PTP_UNICAST_ENA_Pos 20UL
#define ETH_NETWORK_CONTROL_PTP_UNICAST_ENA_Msk 0x100000UL
#define ETH_NETWORK_CONTROL_ALT_SGMII_MODE_Pos  21UL
#define ETH_NETWORK_CONTROL_ALT_SGMII_MODE_Msk  0x200000UL
#define ETH_NETWORK_CONTROL_STORE_UDP_OFFSET_Pos 22UL
#define ETH_NETWORK_CONTROL_STORE_UDP_OFFSET_Msk 0x400000UL
#define ETH_NETWORK_CONTROL_EXT_TSU_PORT_ENABLE_Pos 23UL
#define ETH_NETWORK_CONTROL_EXT_TSU_PORT_ENABLE_Msk 0x800000UL
#define ETH_NETWORK_CONTROL_ONE_STEP_SYNC_MODE_Pos 24UL
#define ETH_NETWORK_CONTROL_ONE_STEP_SYNC_MODE_Msk 0x1000000UL
#define ETH_NETWORK_CONTROL_PFC_CTRL_Pos        25UL
#define ETH_NETWORK_CONTROL_PFC_CTRL_Msk        0x2000000UL
#define ETH_NETWORK_CONTROL_EXT_RXQ_SEL_EN_Pos  26UL
#define ETH_NETWORK_CONTROL_EXT_RXQ_SEL_EN_Msk  0x4000000UL
#define ETH_NETWORK_CONTROL_OSS_CORRECTION_FIELD_Pos 27UL
#define ETH_NETWORK_CONTROL_OSS_CORRECTION_FIELD_Msk 0x8000000UL
#define ETH_NETWORK_CONTROL_SEL_MII_ON_RGMII_Pos 28UL
#define ETH_NETWORK_CONTROL_SEL_MII_ON_RGMII_Msk 0x10000000UL
#define ETH_NETWORK_CONTROL_TWO_PT_FIVE_GIG_Pos 29UL
#define ETH_NETWORK_CONTROL_TWO_PT_FIVE_GIG_Msk 0x20000000UL
#define ETH_NETWORK_CONTROL_IFG_EATS_QAV_CREDIT_Pos 30UL
#define ETH_NETWORK_CONTROL_IFG_EATS_QAV_CREDIT_Msk 0x40000000UL
#define ETH_NETWORK_CONTROL_EXT_RXQ_RESERVED_31_Pos 31UL
#define ETH_NETWORK_CONTROL_EXT_RXQ_RESERVED_31_Msk 0x80000000UL
/* ETH.NETWORK_CONFIG */
#define ETH_NETWORK_CONFIG_SPEED_Pos            0UL
#define ETH_NETWORK_CONFIG_SPEED_Msk            0x1UL
#define ETH_NETWORK_CONFIG_FULL_DUPLEX_Pos      1UL
#define ETH_NETWORK_CONFIG_FULL_DUPLEX_Msk      0x2UL
#define ETH_NETWORK_CONFIG_DISCARD_NON_VLAN_FRAMES_Pos 2UL
#define ETH_NETWORK_CONFIG_DISCARD_NON_VLAN_FRAMES_Msk 0x4UL
#define ETH_NETWORK_CONFIG_JUMBO_FRAMES_Pos     3UL
#define ETH_NETWORK_CONFIG_JUMBO_FRAMES_Msk     0x8UL
#define ETH_NETWORK_CONFIG_COPY_ALL_FRAMES_Pos  4UL
#define ETH_NETWORK_CONFIG_COPY_ALL_FRAMES_Msk  0x10UL
#define ETH_NETWORK_CONFIG_NO_BROADCAST_Pos     5UL
#define ETH_NETWORK_CONFIG_NO_BROADCAST_Msk     0x20UL
#define ETH_NETWORK_CONFIG_MULTICAST_HASH_ENABLE_Pos 6UL
#define ETH_NETWORK_CONFIG_MULTICAST_HASH_ENABLE_Msk 0x40UL
#define ETH_NETWORK_CONFIG_UNICAST_HASH_ENABLE_Pos 7UL
#define ETH_NETWORK_CONFIG_UNICAST_HASH_ENABLE_Msk 0x80UL
#define ETH_NETWORK_CONFIG_RECEIVE_1536_BYTE_FRAMES_Pos 8UL
#define ETH_NETWORK_CONFIG_RECEIVE_1536_BYTE_FRAMES_Msk 0x100UL
#define ETH_NETWORK_CONFIG_EXTERNAL_ADDRESS_MATCH_ENABLE_Pos 9UL
#define ETH_NETWORK_CONFIG_EXTERNAL_ADDRESS_MATCH_ENABLE_Msk 0x200UL
#define ETH_NETWORK_CONFIG_GIGABIT_MODE_ENABLE_Pos 10UL
#define ETH_NETWORK_CONFIG_GIGABIT_MODE_ENABLE_Msk 0x400UL
#define ETH_NETWORK_CONFIG_PCS_SELECT_Pos       11UL
#define ETH_NETWORK_CONFIG_PCS_SELECT_Msk       0x800UL
#define ETH_NETWORK_CONFIG_RETRY_TEST_Pos       12UL
#define ETH_NETWORK_CONFIG_RETRY_TEST_Msk       0x1000UL
#define ETH_NETWORK_CONFIG_PAUSE_ENABLE_Pos     13UL
#define ETH_NETWORK_CONFIG_PAUSE_ENABLE_Msk     0x2000UL
#define ETH_NETWORK_CONFIG_RECEIVE_BUFFER_OFFSET_Pos 14UL
#define ETH_NETWORK_CONFIG_RECEIVE_BUFFER_OFFSET_Msk 0xC000UL
#define ETH_NETWORK_CONFIG_LENGTH_FIELD_ERROR_FRAME_DISCARD_Pos 16UL
#define ETH_NETWORK_CONFIG_LENGTH_FIELD_ERROR_FRAME_DISCARD_Msk 0x10000UL
#define ETH_NETWORK_CONFIG_FCS_REMOVE_Pos       17UL
#define ETH_NETWORK_CONFIG_FCS_REMOVE_Msk       0x20000UL
#define ETH_NETWORK_CONFIG_MDC_CLOCK_DIVISION_Pos 18UL
#define ETH_NETWORK_CONFIG_MDC_CLOCK_DIVISION_Msk 0x1C0000UL
#define ETH_NETWORK_CONFIG_DATA_BUS_WIDTH_Pos   21UL
#define ETH_NETWORK_CONFIG_DATA_BUS_WIDTH_Msk   0x600000UL
#define ETH_NETWORK_CONFIG_DISABLE_COPY_OF_PAUSE_FRAMES_Pos 23UL
#define ETH_NETWORK_CONFIG_DISABLE_COPY_OF_PAUSE_FRAMES_Msk 0x800000UL
#define ETH_NETWORK_CONFIG_RECEIVE_CHECKSUM_OFFLOAD_ENABLE_Pos 24UL
#define ETH_NETWORK_CONFIG_RECEIVE_CHECKSUM_OFFLOAD_ENABLE_Msk 0x1000000UL
#define ETH_NETWORK_CONFIG_EN_HALF_DUPLEX_RX_Pos 25UL
#define ETH_NETWORK_CONFIG_EN_HALF_DUPLEX_RX_Msk 0x2000000UL
#define ETH_NETWORK_CONFIG_IGNORE_RX_FCS_Pos    26UL
#define ETH_NETWORK_CONFIG_IGNORE_RX_FCS_Msk    0x4000000UL
#define ETH_NETWORK_CONFIG_SGMII_MODE_ENABLE_Pos 27UL
#define ETH_NETWORK_CONFIG_SGMII_MODE_ENABLE_Msk 0x8000000UL
#define ETH_NETWORK_CONFIG_IPG_STRETCH_ENABLE_Pos 28UL
#define ETH_NETWORK_CONFIG_IPG_STRETCH_ENABLE_Msk 0x10000000UL
#define ETH_NETWORK_CONFIG_NSP_CHANGE_Pos       29UL
#define ETH_NETWORK_CONFIG_NSP_CHANGE_Msk       0x20000000UL
#define ETH_NETWORK_CONFIG_IGNORE_IPG_RX_ER_Pos 30UL
#define ETH_NETWORK_CONFIG_IGNORE_IPG_RX_ER_Msk 0x40000000UL
#define ETH_NETWORK_CONFIG_RESERVED_31_Pos      31UL
#define ETH_NETWORK_CONFIG_RESERVED_31_Msk      0x80000000UL
/* ETH.NETWORK_STATUS */
#define ETH_NETWORK_STATUS_PCS_LINK_STATE_Pos   0UL
#define ETH_NETWORK_STATUS_PCS_LINK_STATE_Msk   0x1UL
#define ETH_NETWORK_STATUS_MDIO_IN_Pos          1UL
#define ETH_NETWORK_STATUS_MDIO_IN_Msk          0x2UL
#define ETH_NETWORK_STATUS_MAN_DONE_Pos         2UL
#define ETH_NETWORK_STATUS_MAN_DONE_Msk         0x4UL
#define ETH_NETWORK_STATUS_MAC_FULL_DUPLEX_Pos  3UL
#define ETH_NETWORK_STATUS_MAC_FULL_DUPLEX_Msk  0x8UL
#define ETH_NETWORK_STATUS_REMOVED_5_4_Pos      4UL
#define ETH_NETWORK_STATUS_REMOVED_5_4_Msk      0x30UL
#define ETH_NETWORK_STATUS_PFC_NEGOTIATE_PCLK_Pos 6UL
#define ETH_NETWORK_STATUS_PFC_NEGOTIATE_PCLK_Msk 0x40UL
#define ETH_NETWORK_STATUS_LPI_INDICATE_PCLK_Pos 7UL
#define ETH_NETWORK_STATUS_LPI_INDICATE_PCLK_Msk 0x80UL
/* ETH.USER_IO_REGISTER */
#define ETH_USER_IO_REGISTER_RESERVED_31_0_Pos  0UL
#define ETH_USER_IO_REGISTER_RESERVED_31_0_Msk  0xFFFFFFFFUL
/* ETH.DMA_CONFIG */
#define ETH_DMA_CONFIG_AMBA_BURST_LENGTH_Pos    0UL
#define ETH_DMA_CONFIG_AMBA_BURST_LENGTH_Msk    0x1FUL
#define ETH_DMA_CONFIG_HDR_DATA_SPLITTING_EN_Pos 5UL
#define ETH_DMA_CONFIG_HDR_DATA_SPLITTING_EN_Msk 0x20UL
#define ETH_DMA_CONFIG_ENDIAN_SWAP_MANAGEMENT_Pos 6UL
#define ETH_DMA_CONFIG_ENDIAN_SWAP_MANAGEMENT_Msk 0x40UL
#define ETH_DMA_CONFIG_ENDIAN_SWAP_PACKET_Pos   7UL
#define ETH_DMA_CONFIG_ENDIAN_SWAP_PACKET_Msk   0x80UL
#define ETH_DMA_CONFIG_RX_PBUF_SIZE_Pos         8UL
#define ETH_DMA_CONFIG_RX_PBUF_SIZE_Msk         0x300UL
#define ETH_DMA_CONFIG_TX_PBUF_SIZE_Pos         10UL
#define ETH_DMA_CONFIG_TX_PBUF_SIZE_Msk         0x400UL
#define ETH_DMA_CONFIG_TX_PBUF_TCP_EN_Pos       11UL
#define ETH_DMA_CONFIG_TX_PBUF_TCP_EN_Msk       0x800UL
#define ETH_DMA_CONFIG_INFINITE_LAST_DBUF_SIZE_EN_Pos 12UL
#define ETH_DMA_CONFIG_INFINITE_LAST_DBUF_SIZE_EN_Msk 0x1000UL
#define ETH_DMA_CONFIG_CRC_ERROR_REPORT_Pos     13UL
#define ETH_DMA_CONFIG_CRC_ERROR_REPORT_Msk     0x2000UL
#define ETH_DMA_CONFIG_RX_BUF_SIZE_Pos          16UL
#define ETH_DMA_CONFIG_RX_BUF_SIZE_Msk          0xFF0000UL
#define ETH_DMA_CONFIG_FORCE_DISCARD_ON_ERR_Pos 24UL
#define ETH_DMA_CONFIG_FORCE_DISCARD_ON_ERR_Msk 0x1000000UL
#define ETH_DMA_CONFIG_FORCE_MAX_AMBA_BURST_RX_Pos 25UL
#define ETH_DMA_CONFIG_FORCE_MAX_AMBA_BURST_RX_Msk 0x2000000UL
#define ETH_DMA_CONFIG_FORCE_MAX_AMBA_BURST_TX_Pos 26UL
#define ETH_DMA_CONFIG_FORCE_MAX_AMBA_BURST_TX_Msk 0x4000000UL
#define ETH_DMA_CONFIG_RX_BD_EXTENDED_MODE_EN_Pos 28UL
#define ETH_DMA_CONFIG_RX_BD_EXTENDED_MODE_EN_Msk 0x10000000UL
#define ETH_DMA_CONFIG_TX_BD_EXTENDED_MODE_EN_Pos 29UL
#define ETH_DMA_CONFIG_TX_BD_EXTENDED_MODE_EN_Msk 0x20000000UL
#define ETH_DMA_CONFIG_DMA_ADDR_BUS_WIDTH_1_Pos 30UL
#define ETH_DMA_CONFIG_DMA_ADDR_BUS_WIDTH_1_Msk 0x40000000UL
/* ETH.TRANSMIT_STATUS */
#define ETH_TRANSMIT_STATUS_USED_BIT_READ_Pos   0UL
#define ETH_TRANSMIT_STATUS_USED_BIT_READ_Msk   0x1UL
#define ETH_TRANSMIT_STATUS_COLLISION_OCCURRED_Pos 1UL
#define ETH_TRANSMIT_STATUS_COLLISION_OCCURRED_Msk 0x2UL
#define ETH_TRANSMIT_STATUS_RETRY_LIMIT_EXCEEDED_Pos 2UL
#define ETH_TRANSMIT_STATUS_RETRY_LIMIT_EXCEEDED_Msk 0x4UL
#define ETH_TRANSMIT_STATUS_TRANSMIT_GO_Pos     3UL
#define ETH_TRANSMIT_STATUS_TRANSMIT_GO_Msk     0x8UL
#define ETH_TRANSMIT_STATUS_AMBA_ERROR123_Pos   4UL
#define ETH_TRANSMIT_STATUS_AMBA_ERROR123_Msk   0x10UL
#define ETH_TRANSMIT_STATUS_TRANSMIT_COMPLETE123_Pos 5UL
#define ETH_TRANSMIT_STATUS_TRANSMIT_COMPLETE123_Msk 0x20UL
#define ETH_TRANSMIT_STATUS_TRANSMIT_UNDER_RUN123_Pos 6UL
#define ETH_TRANSMIT_STATUS_TRANSMIT_UNDER_RUN123_Msk 0x40UL
#define ETH_TRANSMIT_STATUS_LATE_COLLISION_OCCURRED_Pos 7UL
#define ETH_TRANSMIT_STATUS_LATE_COLLISION_OCCURRED_Msk 0x80UL
#define ETH_TRANSMIT_STATUS_RESP_NOT_OK123_Pos  8UL
#define ETH_TRANSMIT_STATUS_RESP_NOT_OK123_Msk  0x100UL
/* ETH.RECEIVE_Q_PTR */
#define ETH_RECEIVE_Q_PTR_DMA_RX_DIS_Q_Pos      0UL
#define ETH_RECEIVE_Q_PTR_DMA_RX_DIS_Q_Msk      0x1UL
#define ETH_RECEIVE_Q_PTR_DMA_RX_Q_PTR_Pos      2UL
#define ETH_RECEIVE_Q_PTR_DMA_RX_Q_PTR_Msk      0xFFFFFFFCUL
/* ETH.TRANSMIT_Q_PTR */
#define ETH_TRANSMIT_Q_PTR_DMA_TX_DIS_Q_Pos     0UL
#define ETH_TRANSMIT_Q_PTR_DMA_TX_DIS_Q_Msk     0x1UL
#define ETH_TRANSMIT_Q_PTR_DMA_TX_Q_PTR_Pos     2UL
#define ETH_TRANSMIT_Q_PTR_DMA_TX_Q_PTR_Msk     0xFFFFFFFCUL
/* ETH.RECEIVE_STATUS */
#define ETH_RECEIVE_STATUS_BUFFER_NOT_AVAILABLE_Pos 0UL
#define ETH_RECEIVE_STATUS_BUFFER_NOT_AVAILABLE_Msk 0x1UL
#define ETH_RECEIVE_STATUS_FRAME_RECEIVED_Pos   1UL
#define ETH_RECEIVE_STATUS_FRAME_RECEIVED_Msk   0x2UL
#define ETH_RECEIVE_STATUS_RECEIVE_OVERRUN123_Pos 2UL
#define ETH_RECEIVE_STATUS_RECEIVE_OVERRUN123_Msk 0x4UL
#define ETH_RECEIVE_STATUS_RESP_NOT_OK1234_Pos  3UL
#define ETH_RECEIVE_STATUS_RESP_NOT_OK1234_Msk  0x8UL
/* ETH.INT_STATUS */
#define ETH_INT_STATUS_MANAGEMENT_FRAME_SENT_Pos 0UL
#define ETH_INT_STATUS_MANAGEMENT_FRAME_SENT_Msk 0x1UL
#define ETH_INT_STATUS_RECEIVE_COMPLETE_Pos     1UL
#define ETH_INT_STATUS_RECEIVE_COMPLETE_Msk     0x2UL
#define ETH_INT_STATUS_RX_USED_BIT_READ_Pos     2UL
#define ETH_INT_STATUS_RX_USED_BIT_READ_Msk     0x4UL
#define ETH_INT_STATUS_TX_USED_BIT_READ_Pos     3UL
#define ETH_INT_STATUS_TX_USED_BIT_READ_Msk     0x8UL
#define ETH_INT_STATUS_TRANSMIT_UNDER_RUN_Pos   4UL
#define ETH_INT_STATUS_TRANSMIT_UNDER_RUN_Msk   0x10UL
#define ETH_INT_STATUS_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_Pos 5UL
#define ETH_INT_STATUS_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_Msk 0x20UL
#define ETH_INT_STATUS_AMBA_ERROR_Pos           6UL
#define ETH_INT_STATUS_AMBA_ERROR_Msk           0x40UL
#define ETH_INT_STATUS_TRANSMIT_COMPLETE_Pos    7UL
#define ETH_INT_STATUS_TRANSMIT_COMPLETE_Msk    0x80UL
#define ETH_INT_STATUS_REMOVED_9_Pos            9UL
#define ETH_INT_STATUS_REMOVED_9_Msk            0x200UL
#define ETH_INT_STATUS_RECEIVE_OVERRUN_Pos      10UL
#define ETH_INT_STATUS_RECEIVE_OVERRUN_Msk      0x400UL
#define ETH_INT_STATUS_RESP_NOT_OK_Pos          11UL
#define ETH_INT_STATUS_RESP_NOT_OK_Msk          0x800UL
#define ETH_INT_STATUS_PAUSE_FRAME_WITH_NON_ZERO_PAUSE_QUANTUM_RECEIVED_Pos 12UL
#define ETH_INT_STATUS_PAUSE_FRAME_WITH_NON_ZERO_PAUSE_QUANTUM_RECEIVED_Msk 0x1000UL
#define ETH_INT_STATUS_PAUSE_TIME_ELAPSED_Pos   13UL
#define ETH_INT_STATUS_PAUSE_TIME_ELAPSED_Msk   0x2000UL
#define ETH_INT_STATUS_PAUSE_FRAME_TRANSMITTED_Pos 14UL
#define ETH_INT_STATUS_PAUSE_FRAME_TRANSMITTED_Msk 0x4000UL
#define ETH_INT_STATUS_REMOVED_15_Pos           15UL
#define ETH_INT_STATUS_REMOVED_15_Msk           0x8000UL
#define ETH_INT_STATUS_REMOVED_16_Pos           16UL
#define ETH_INT_STATUS_REMOVED_16_Msk           0x10000UL
#define ETH_INT_STATUS_REMOVED_17_Pos           17UL
#define ETH_INT_STATUS_REMOVED_17_Msk           0x20000UL
#define ETH_INT_STATUS_PTP_DELAY_REQ_FRAME_RECEIVED_Pos 18UL
#define ETH_INT_STATUS_PTP_DELAY_REQ_FRAME_RECEIVED_Msk 0x40000UL
#define ETH_INT_STATUS_PTP_SYNC_FRAME_RECEIVED_Pos 19UL
#define ETH_INT_STATUS_PTP_SYNC_FRAME_RECEIVED_Msk 0x80000UL
#define ETH_INT_STATUS_PTP_DELAY_REQ_FRAME_TRANSMITTED_Pos 20UL
#define ETH_INT_STATUS_PTP_DELAY_REQ_FRAME_TRANSMITTED_Msk 0x100000UL
#define ETH_INT_STATUS_PTP_SYNC_FRAME_TRANSMITTED_Pos 21UL
#define ETH_INT_STATUS_PTP_SYNC_FRAME_TRANSMITTED_Msk 0x200000UL
#define ETH_INT_STATUS_PTP_PDELAY_REQ_FRAME_RECEIVED_Pos 22UL
#define ETH_INT_STATUS_PTP_PDELAY_REQ_FRAME_RECEIVED_Msk 0x400000UL
#define ETH_INT_STATUS_PTP_PDELAY_RESP_FRAME_RECEIVED_Pos 23UL
#define ETH_INT_STATUS_PTP_PDELAY_RESP_FRAME_RECEIVED_Msk 0x800000UL
#define ETH_INT_STATUS_PTP_PDELAY_REQ_FRAME_TRANSMITTED_Pos 24UL
#define ETH_INT_STATUS_PTP_PDELAY_REQ_FRAME_TRANSMITTED_Msk 0x1000000UL
#define ETH_INT_STATUS_PTP_PDELAY_RESP_FRAME_TRANSMITTED_Pos 25UL
#define ETH_INT_STATUS_PTP_PDELAY_RESP_FRAME_TRANSMITTED_Msk 0x2000000UL
#define ETH_INT_STATUS_TSU_SECONDS_REGISTER_INCREMENT_Pos 26UL
#define ETH_INT_STATUS_TSU_SECONDS_REGISTER_INCREMENT_Msk 0x4000000UL
#define ETH_INT_STATUS_RECEIVE_LPI_INDICATION_STATUS_BIT_CHANGE_Pos 27UL
#define ETH_INT_STATUS_RECEIVE_LPI_INDICATION_STATUS_BIT_CHANGE_Msk 0x8000000UL
#define ETH_INT_STATUS_REMOVED_28_Pos           28UL
#define ETH_INT_STATUS_REMOVED_28_Msk           0x10000000UL
#define ETH_INT_STATUS_TSU_TIMER_COMPARISON_INTERRUPT_Pos 29UL
#define ETH_INT_STATUS_TSU_TIMER_COMPARISON_INTERRUPT_Msk 0x20000000UL
/* ETH.INT_ENABLE */
#define ETH_INT_ENABLE_ENABLE_MANAGEMENT_DONE_INTERRUPT_Pos 0UL
#define ETH_INT_ENABLE_ENABLE_MANAGEMENT_DONE_INTERRUPT_Msk 0x1UL
#define ETH_INT_ENABLE_ENABLE_RECEIVE_COMPLETE_INTERRUPT_Pos 1UL
#define ETH_INT_ENABLE_ENABLE_RECEIVE_COMPLETE_INTERRUPT_Msk 0x2UL
#define ETH_INT_ENABLE_ENABLE_RECEIVE_USED_BIT_READ_INTERRUPT_Pos 2UL
#define ETH_INT_ENABLE_ENABLE_RECEIVE_USED_BIT_READ_INTERRUPT_Msk 0x4UL
#define ETH_INT_ENABLE_ENABLE_TRANSMIT_USED_BIT_READ_INTERRUPT_Pos 3UL
#define ETH_INT_ENABLE_ENABLE_TRANSMIT_USED_BIT_READ_INTERRUPT_Msk 0x8UL
#define ETH_INT_ENABLE_ENABLE_TRANSMIT_BUFFER_UNDER_RUN_INTERRUPT_Pos 4UL
#define ETH_INT_ENABLE_ENABLE_TRANSMIT_BUFFER_UNDER_RUN_INTERRUPT_Msk 0x10UL
#define ETH_INT_ENABLE_ENABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_Pos 5UL
#define ETH_INT_ENABLE_ENABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_Msk 0x20UL
#define ETH_INT_ENABLE_ENABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT_Pos 6UL
#define ETH_INT_ENABLE_ENABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT_Msk 0x40UL
#define ETH_INT_ENABLE_ENABLE_TRANSMIT_COMPLETE_INTERRUPT_Pos 7UL
#define ETH_INT_ENABLE_ENABLE_TRANSMIT_COMPLETE_INTERRUPT_Msk 0x80UL
#define ETH_INT_ENABLE_UNUSED_8_Pos             8UL
#define ETH_INT_ENABLE_UNUSED_8_Msk             0x100UL
#define ETH_INT_ENABLE_UNUSED_9_Pos             9UL
#define ETH_INT_ENABLE_UNUSED_9_Msk             0x200UL
#define ETH_INT_ENABLE_ENABLE_RECEIVE_OVERRUN_INTERRUPT_Pos 10UL
#define ETH_INT_ENABLE_ENABLE_RECEIVE_OVERRUN_INTERRUPT_Msk 0x400UL
#define ETH_INT_ENABLE_ENABLE_RESP_NOT_OK_INTERRUPT_Pos 11UL
#define ETH_INT_ENABLE_ENABLE_RESP_NOT_OK_INTERRUPT_Msk 0x800UL
#define ETH_INT_ENABLE_ENABLE_PAUSE_FRAME_WITH_NON_ZERO_PAUSE_QUANTUM_INTERRUPT_Pos 12UL
#define ETH_INT_ENABLE_ENABLE_PAUSE_FRAME_WITH_NON_ZERO_PAUSE_QUANTUM_INTERRUPT_Msk 0x1000UL
#define ETH_INT_ENABLE_ENABLE_PAUSE_TIME_ZERO_INTERRUPT_Pos 13UL
#define ETH_INT_ENABLE_ENABLE_PAUSE_TIME_ZERO_INTERRUPT_Msk 0x2000UL
#define ETH_INT_ENABLE_ENABLE_PAUSE_FRAME_TRANSMITTED_INTERRUPT_Pos 14UL
#define ETH_INT_ENABLE_ENABLE_PAUSE_FRAME_TRANSMITTED_INTERRUPT_Msk 0x4000UL
#define ETH_INT_ENABLE_UNUSED_15_Pos            15UL
#define ETH_INT_ENABLE_UNUSED_15_Msk            0x8000UL
#define ETH_INT_ENABLE_UNUSED_16_Pos            16UL
#define ETH_INT_ENABLE_UNUSED_16_Msk            0x10000UL
#define ETH_INT_ENABLE_UNUSED_17_Pos            17UL
#define ETH_INT_ENABLE_UNUSED_17_Msk            0x20000UL
#define ETH_INT_ENABLE_ENABLE_PTP_DELAY_REQ_FRAME_RECEIVED_Pos 18UL
#define ETH_INT_ENABLE_ENABLE_PTP_DELAY_REQ_FRAME_RECEIVED_Msk 0x40000UL
#define ETH_INT_ENABLE_ENABLE_PTP_SYNC_FRAME_RECEIVED_Pos 19UL
#define ETH_INT_ENABLE_ENABLE_PTP_SYNC_FRAME_RECEIVED_Msk 0x80000UL
#define ETH_INT_ENABLE_ENABLE_PTP_DELAY_REQ_FRAME_TRANSMITTED_Pos 20UL
#define ETH_INT_ENABLE_ENABLE_PTP_DELAY_REQ_FRAME_TRANSMITTED_Msk 0x100000UL
#define ETH_INT_ENABLE_ENABLE_PTP_SYNC_FRAME_TRANSMITTED_Pos 21UL
#define ETH_INT_ENABLE_ENABLE_PTP_SYNC_FRAME_TRANSMITTED_Msk 0x200000UL
#define ETH_INT_ENABLE_ENABLE_PTP_PDELAY_REQ_FRAME_RECEIVED_Pos 22UL
#define ETH_INT_ENABLE_ENABLE_PTP_PDELAY_REQ_FRAME_RECEIVED_Msk 0x400000UL
#define ETH_INT_ENABLE_ENABLE_PTP_PDELAY_RESP_FRAME_RECEIVED_Pos 23UL
#define ETH_INT_ENABLE_ENABLE_PTP_PDELAY_RESP_FRAME_RECEIVED_Msk 0x800000UL
#define ETH_INT_ENABLE_ENABLE_PTP_PDELAY_REQ_FRAME_TRANSMITTED_Pos 24UL
#define ETH_INT_ENABLE_ENABLE_PTP_PDELAY_REQ_FRAME_TRANSMITTED_Msk 0x1000000UL
#define ETH_INT_ENABLE_ENABLE_PTP_PDELAY_RESP_FRAME_TRANSMITTED_Pos 25UL
#define ETH_INT_ENABLE_ENABLE_PTP_PDELAY_RESP_FRAME_TRANSMITTED_Msk 0x2000000UL
#define ETH_INT_ENABLE_ENABLE_TSU_SECONDS_REGISTER_INCREMENT_Pos 26UL
#define ETH_INT_ENABLE_ENABLE_TSU_SECONDS_REGISTER_INCREMENT_Msk 0x4000000UL
#define ETH_INT_ENABLE_ENABLE_RX_LPI_INDICATION_INTERRUPT_Pos 27UL
#define ETH_INT_ENABLE_ENABLE_RX_LPI_INDICATION_INTERRUPT_Msk 0x8000000UL
#define ETH_INT_ENABLE_UNUSED_28_Pos            28UL
#define ETH_INT_ENABLE_UNUSED_28_Msk            0x10000000UL
#define ETH_INT_ENABLE_ENABLE_TSU_TIMER_COMPARISON_INTERRUPT_Pos 29UL
#define ETH_INT_ENABLE_ENABLE_TSU_TIMER_COMPARISON_INTERRUPT_Msk 0x20000000UL
/* ETH.INT_DISABLE */
#define ETH_INT_DISABLE_DISABLE_MANAGEMENT_DONE_INTERRUPT_Pos 0UL
#define ETH_INT_DISABLE_DISABLE_MANAGEMENT_DONE_INTERRUPT_Msk 0x1UL
#define ETH_INT_DISABLE_DISABLE_RECEIVE_COMPLETE_INTERRUPT_Pos 1UL
#define ETH_INT_DISABLE_DISABLE_RECEIVE_COMPLETE_INTERRUPT_Msk 0x2UL
#define ETH_INT_DISABLE_DISABLE_RECEIVE_USED_BIT_READ_INTERRUPT_Pos 2UL
#define ETH_INT_DISABLE_DISABLE_RECEIVE_USED_BIT_READ_INTERRUPT_Msk 0x4UL
#define ETH_INT_DISABLE_DISABLE_TRANSMIT_USED_BIT_READ_INTERRUPT_Pos 3UL
#define ETH_INT_DISABLE_DISABLE_TRANSMIT_USED_BIT_READ_INTERRUPT_Msk 0x8UL
#define ETH_INT_DISABLE_DISABLE_TRANSMIT_BUFFER_UNDER_RUN_INTERRUPT_Pos 4UL
#define ETH_INT_DISABLE_DISABLE_TRANSMIT_BUFFER_UNDER_RUN_INTERRUPT_Msk 0x10UL
#define ETH_INT_DISABLE_DISABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_Pos 5UL
#define ETH_INT_DISABLE_DISABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_Msk 0x20UL
#define ETH_INT_DISABLE_DISABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT_Pos 6UL
#define ETH_INT_DISABLE_DISABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT_Msk 0x40UL
#define ETH_INT_DISABLE_DISABLE_TRANSMIT_COMPLETE_INTERRUPT_Pos 7UL
#define ETH_INT_DISABLE_DISABLE_TRANSMIT_COMPLETE_INTERRUPT_Msk 0x80UL
#define ETH_INT_DISABLE_UNUSED_8_Pos            8UL
#define ETH_INT_DISABLE_UNUSED_8_Msk            0x100UL
#define ETH_INT_DISABLE_UNUSED_9_Pos            9UL
#define ETH_INT_DISABLE_UNUSED_9_Msk            0x200UL
#define ETH_INT_DISABLE_DISABLE_RECEIVE_OVERRUN_INTERRUPT_Pos 10UL
#define ETH_INT_DISABLE_DISABLE_RECEIVE_OVERRUN_INTERRUPT_Msk 0x400UL
#define ETH_INT_DISABLE_DISABLE_RESP_NOT_OK_INTERRUPT_Pos 11UL
#define ETH_INT_DISABLE_DISABLE_RESP_NOT_OK_INTERRUPT_Msk 0x800UL
#define ETH_INT_DISABLE_DISABLE_PAUSE_FRAME_WITH_NON_ZERO_PAUSE_QUANTUM_INTERRUPT_Pos 12UL
#define ETH_INT_DISABLE_DISABLE_PAUSE_FRAME_WITH_NON_ZERO_PAUSE_QUANTUM_INTERRUPT_Msk 0x1000UL
#define ETH_INT_DISABLE_DISABLE_PAUSE_TIME_ZERO_INTERRUPT_Pos 13UL
#define ETH_INT_DISABLE_DISABLE_PAUSE_TIME_ZERO_INTERRUPT_Msk 0x2000UL
#define ETH_INT_DISABLE_DISABLE_PAUSE_FRAME_TRANSMITTED_INTERRUPT_Pos 14UL
#define ETH_INT_DISABLE_DISABLE_PAUSE_FRAME_TRANSMITTED_INTERRUPT_Msk 0x4000UL
#define ETH_INT_DISABLE_UNUSED_15_Pos           15UL
#define ETH_INT_DISABLE_UNUSED_15_Msk           0x8000UL
#define ETH_INT_DISABLE_UNUSED_16_Pos           16UL
#define ETH_INT_DISABLE_UNUSED_16_Msk           0x10000UL
#define ETH_INT_DISABLE_UNUSED_17_Pos           17UL
#define ETH_INT_DISABLE_UNUSED_17_Msk           0x20000UL
#define ETH_INT_DISABLE_DISABLE_PTP_DELAY_REQ_FRAME_RECEIVED_Pos 18UL
#define ETH_INT_DISABLE_DISABLE_PTP_DELAY_REQ_FRAME_RECEIVED_Msk 0x40000UL
#define ETH_INT_DISABLE_DISABLE_PTP_SYNC_FRAME_RECEIVED_Pos 19UL
#define ETH_INT_DISABLE_DISABLE_PTP_SYNC_FRAME_RECEIVED_Msk 0x80000UL
#define ETH_INT_DISABLE_DISABLE_PTP_DELAY_REQ_FRAME_TRANSMITTED_Pos 20UL
#define ETH_INT_DISABLE_DISABLE_PTP_DELAY_REQ_FRAME_TRANSMITTED_Msk 0x100000UL
#define ETH_INT_DISABLE_DISABLE_PTP_SYNC_FRAME_TRANSMITTED_Pos 21UL
#define ETH_INT_DISABLE_DISABLE_PTP_SYNC_FRAME_TRANSMITTED_Msk 0x200000UL
#define ETH_INT_DISABLE_DISABLE_PTP_PDELAY_REQ_FRAME_RECEIVED_Pos 22UL
#define ETH_INT_DISABLE_DISABLE_PTP_PDELAY_REQ_FRAME_RECEIVED_Msk 0x400000UL
#define ETH_INT_DISABLE_DISABLE_PTP_PDELAY_RESP_FRAME_RECEIVED_Pos 23UL
#define ETH_INT_DISABLE_DISABLE_PTP_PDELAY_RESP_FRAME_RECEIVED_Msk 0x800000UL
#define ETH_INT_DISABLE_DISABLE_PTP_PDELAY_REQ_FRAME_TRANSMITTED_Pos 24UL
#define ETH_INT_DISABLE_DISABLE_PTP_PDELAY_REQ_FRAME_TRANSMITTED_Msk 0x1000000UL
#define ETH_INT_DISABLE_DISABLE_PTP_PDELAY_RESP_FRAME_TRANSMITTED_Pos 25UL
#define ETH_INT_DISABLE_DISABLE_PTP_PDELAY_RESP_FRAME_TRANSMITTED_Msk 0x2000000UL
#define ETH_INT_DISABLE_DISABLE_TSU_SECONDS_REGISTER_INCREMENT_Pos 26UL
#define ETH_INT_DISABLE_DISABLE_TSU_SECONDS_REGISTER_INCREMENT_Msk 0x4000000UL
#define ETH_INT_DISABLE_DISABLE_RX_LPI_INDICATION_INTERRUPT_Pos 27UL
#define ETH_INT_DISABLE_DISABLE_RX_LPI_INDICATION_INTERRUPT_Msk 0x8000000UL
#define ETH_INT_DISABLE_UNUSED_28_Pos           28UL
#define ETH_INT_DISABLE_UNUSED_28_Msk           0x10000000UL
#define ETH_INT_DISABLE_DISABLE_TSU_TIMER_COMPARISON_INTERRUPT_Pos 29UL
#define ETH_INT_DISABLE_DISABLE_TSU_TIMER_COMPARISON_INTERRUPT_Msk 0x20000000UL
#define ETH_INT_DISABLE_RESERVED_30_30_Pos      30UL
#define ETH_INT_DISABLE_RESERVED_30_30_Msk      0x40000000UL
#define ETH_INT_DISABLE_RESERVED_31_31_Pos      31UL
#define ETH_INT_DISABLE_RESERVED_31_31_Msk      0x80000000UL
/* ETH.INT_MASK */
#define ETH_INT_MASK_MANAGEMENT_DONE_INTERRUPT_MASK_Pos 0UL
#define ETH_INT_MASK_MANAGEMENT_DONE_INTERRUPT_MASK_Msk 0x1UL
#define ETH_INT_MASK_RECEIVE_COMPLETE_INTERRUPT_MASK_Pos 1UL
#define ETH_INT_MASK_RECEIVE_COMPLETE_INTERRUPT_MASK_Msk 0x2UL
#define ETH_INT_MASK_RECEIVE_USED_BIT_READ_INTERRUPT_MASK_Pos 2UL
#define ETH_INT_MASK_RECEIVE_USED_BIT_READ_INTERRUPT_MASK_Msk 0x4UL
#define ETH_INT_MASK_TRANSMIT_USED_BIT_READ_INTERRUPT_MASK_Pos 3UL
#define ETH_INT_MASK_TRANSMIT_USED_BIT_READ_INTERRUPT_MASK_Msk 0x8UL
#define ETH_INT_MASK_TRANSMIT_BUFFER_UNDER_RUN_INTERRUPT_MASK_Pos 4UL
#define ETH_INT_MASK_TRANSMIT_BUFFER_UNDER_RUN_INTERRUPT_MASK_Msk 0x10UL
#define ETH_INT_MASK_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_MASK_Pos 5UL
#define ETH_INT_MASK_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_MASK_Msk 0x20UL
#define ETH_INT_MASK_AMBA_ERROR_INTERRUPT_MASK_Pos 6UL
#define ETH_INT_MASK_AMBA_ERROR_INTERRUPT_MASK_Msk 0x40UL
#define ETH_INT_MASK_TRANSMIT_COMPLETE_INTERRUPT_MASK_Pos 7UL
#define ETH_INT_MASK_TRANSMIT_COMPLETE_INTERRUPT_MASK_Msk 0x80UL
#define ETH_INT_MASK_UNUSED_8_Pos               8UL
#define ETH_INT_MASK_UNUSED_8_Msk               0x100UL
#define ETH_INT_MASK_UNUSED_9_Pos               9UL
#define ETH_INT_MASK_UNUSED_9_Msk               0x200UL
#define ETH_INT_MASK_RECEIVE_OVERRUN_INTERRUPT_MASK_Pos 10UL
#define ETH_INT_MASK_RECEIVE_OVERRUN_INTERRUPT_MASK_Msk 0x400UL
#define ETH_INT_MASK_RESP_NOT_OK_INTERRUPT_MASK_Pos 11UL
#define ETH_INT_MASK_RESP_NOT_OK_INTERRUPT_MASK_Msk 0x800UL
#define ETH_INT_MASK_PAUSE_FRAME_WITH_NON_ZERO_PAUSE_QUANTUM_INTERRUPT_MASK_Pos 12UL
#define ETH_INT_MASK_PAUSE_FRAME_WITH_NON_ZERO_PAUSE_QUANTUM_INTERRUPT_MASK_Msk 0x1000UL
#define ETH_INT_MASK_PAUSE_TIME_ZERO_INTERRUPT_MASK_Pos 13UL
#define ETH_INT_MASK_PAUSE_TIME_ZERO_INTERRUPT_MASK_Msk 0x2000UL
#define ETH_INT_MASK_PAUSE_FRAME_TRANSMITTED_INTERRUPT_MASK_Pos 14UL
#define ETH_INT_MASK_PAUSE_FRAME_TRANSMITTED_INTERRUPT_MASK_Msk 0x4000UL
#define ETH_INT_MASK_UNUSED_15_Pos              15UL
#define ETH_INT_MASK_UNUSED_15_Msk              0x8000UL
#define ETH_INT_MASK_UNUSED_16_Pos              16UL
#define ETH_INT_MASK_UNUSED_16_Msk              0x10000UL
#define ETH_INT_MASK_UNUSED_17_Pos              17UL
#define ETH_INT_MASK_UNUSED_17_Msk              0x20000UL
#define ETH_INT_MASK_PTP_DELAY_REQ_FRAME_RECEIVED_MASK_Pos 18UL
#define ETH_INT_MASK_PTP_DELAY_REQ_FRAME_RECEIVED_MASK_Msk 0x40000UL
#define ETH_INT_MASK_PTP_SYNC_FRAME_RECEIVED_MASK_Pos 19UL
#define ETH_INT_MASK_PTP_SYNC_FRAME_RECEIVED_MASK_Msk 0x80000UL
#define ETH_INT_MASK_PTP_DELAY_REQ_FRAME_TRANSMITTED_MASK_Pos 20UL
#define ETH_INT_MASK_PTP_DELAY_REQ_FRAME_TRANSMITTED_MASK_Msk 0x100000UL
#define ETH_INT_MASK_PTP_SYNC_FRAME_TRANSMITTED_MASK_Pos 21UL
#define ETH_INT_MASK_PTP_SYNC_FRAME_TRANSMITTED_MASK_Msk 0x200000UL
#define ETH_INT_MASK_PTP_PDELAY_REQ_FRAME_RECEIVED_MASK_Pos 22UL
#define ETH_INT_MASK_PTP_PDELAY_REQ_FRAME_RECEIVED_MASK_Msk 0x400000UL
#define ETH_INT_MASK_PTP_PDELAY_RESP_FRAME_RECEIVED_MASK_Pos 23UL
#define ETH_INT_MASK_PTP_PDELAY_RESP_FRAME_RECEIVED_MASK_Msk 0x800000UL
#define ETH_INT_MASK_PTP_PDELAY_REQ_FRAME_TRANSMITTED_MASK_Pos 24UL
#define ETH_INT_MASK_PTP_PDELAY_REQ_FRAME_TRANSMITTED_MASK_Msk 0x1000000UL
#define ETH_INT_MASK_PTP_PDELAY_RESP_FRAME_TRANSMITTED_MASK_Pos 25UL
#define ETH_INT_MASK_PTP_PDELAY_RESP_FRAME_TRANSMITTED_MASK_Msk 0x2000000UL
#define ETH_INT_MASK_TSU_SECONDS_REGISTER_INCREMENT_MASK_Pos 26UL
#define ETH_INT_MASK_TSU_SECONDS_REGISTER_INCREMENT_MASK_Msk 0x4000000UL
#define ETH_INT_MASK_RX_LPI_INDICATION_MASK_Pos 27UL
#define ETH_INT_MASK_RX_LPI_INDICATION_MASK_Msk 0x8000000UL
#define ETH_INT_MASK_UNUSED_28_Pos              28UL
#define ETH_INT_MASK_UNUSED_28_Msk              0x10000000UL
#define ETH_INT_MASK_TSU_TIMER_COMPARISON_MASK_Pos 29UL
#define ETH_INT_MASK_TSU_TIMER_COMPARISON_MASK_Msk 0x20000000UL
/* ETH.PHY_MANAGEMENT */
#define ETH_PHY_MANAGEMENT_PHY_WRITE_READ_DATA_Pos 0UL
#define ETH_PHY_MANAGEMENT_PHY_WRITE_READ_DATA_Msk 0xFFFFUL
#define ETH_PHY_MANAGEMENT_WRITE10_Pos          16UL
#define ETH_PHY_MANAGEMENT_WRITE10_Msk          0x30000UL
#define ETH_PHY_MANAGEMENT_REGISTER_ADDRESS_Pos 18UL
#define ETH_PHY_MANAGEMENT_REGISTER_ADDRESS_Msk 0x7C0000UL
#define ETH_PHY_MANAGEMENT_PHY_ADDRESS_Pos      23UL
#define ETH_PHY_MANAGEMENT_PHY_ADDRESS_Msk      0xF800000UL
#define ETH_PHY_MANAGEMENT_OPERATION_Pos        28UL
#define ETH_PHY_MANAGEMENT_OPERATION_Msk        0x30000000UL
#define ETH_PHY_MANAGEMENT_WRITE1_Pos           30UL
#define ETH_PHY_MANAGEMENT_WRITE1_Msk           0x40000000UL
#define ETH_PHY_MANAGEMENT_WRITE0_Pos           31UL
#define ETH_PHY_MANAGEMENT_WRITE0_Msk           0x80000000UL
/* ETH.PAUSE_TIME */
#define ETH_PAUSE_TIME_QUANTUM_Pos              0UL
#define ETH_PAUSE_TIME_QUANTUM_Msk              0xFFFFUL
/* ETH.TX_PAUSE_QUANTUM */
#define ETH_TX_PAUSE_QUANTUM_QUANTUM_Pos        0UL
#define ETH_TX_PAUSE_QUANTUM_QUANTUM_Msk        0xFFFFUL
#define ETH_TX_PAUSE_QUANTUM_QUANTUM_P1_Pos     16UL
#define ETH_TX_PAUSE_QUANTUM_QUANTUM_P1_Msk     0xFFFF0000UL
/* ETH.PBUF_TXCUTTHRU */
#define ETH_PBUF_TXCUTTHRU_DMA_TX_CUTTHRU_THRESHOLD_Pos 0UL
#define ETH_PBUF_TXCUTTHRU_DMA_TX_CUTTHRU_THRESHOLD_Msk 0x1FFUL
#define ETH_PBUF_TXCUTTHRU_DMA_TX_CUTTHRU_Pos   31UL
#define ETH_PBUF_TXCUTTHRU_DMA_TX_CUTTHRU_Msk   0x80000000UL
/* ETH.PBUF_RXCUTTHRU */
#define ETH_PBUF_RXCUTTHRU_DMA_RX_CUTTHRU_THRESHOLD_Pos 0UL
#define ETH_PBUF_RXCUTTHRU_DMA_RX_CUTTHRU_THRESHOLD_Msk 0xFFUL
#define ETH_PBUF_RXCUTTHRU_DMA_RX_CUTTHRU_Pos   31UL
#define ETH_PBUF_RXCUTTHRU_DMA_RX_CUTTHRU_Msk   0x80000000UL
/* ETH.JUMBO_MAX_LENGTH */
#define ETH_JUMBO_MAX_LENGTH_JUMBO_MAX_LENGTH_Pos 0UL
#define ETH_JUMBO_MAX_LENGTH_JUMBO_MAX_LENGTH_Msk 0x3FFFUL
/* ETH.EXTERNAL_FIFO_INTERFACE */
#define ETH_EXTERNAL_FIFO_INTERFACE_REMOVED_31_0_Pos 0UL
#define ETH_EXTERNAL_FIFO_INTERFACE_REMOVED_31_0_Msk 0xFFFFFFFFUL
/* ETH.AXI_MAX_PIPELINE */
#define ETH_AXI_MAX_PIPELINE_AR2R_MAX_PIPELINE_Pos 0UL
#define ETH_AXI_MAX_PIPELINE_AR2R_MAX_PIPELINE_Msk 0xFFUL
#define ETH_AXI_MAX_PIPELINE_AW2W_MAX_PIPELINE_Pos 8UL
#define ETH_AXI_MAX_PIPELINE_AW2W_MAX_PIPELINE_Msk 0xFF00UL
#define ETH_AXI_MAX_PIPELINE_USE_AW2B_FILL_Pos  16UL
#define ETH_AXI_MAX_PIPELINE_USE_AW2B_FILL_Msk  0x10000UL
/* ETH.RSC_CONTROL */
#define ETH_RSC_CONTROL_REMOVED_31_0_Pos        0UL
#define ETH_RSC_CONTROL_REMOVED_31_0_Msk        0xFFFFFFFFUL
/* ETH.INT_MODERATION */
#define ETH_INT_MODERATION_RX_INT_MODERATION_Pos 0UL
#define ETH_INT_MODERATION_RX_INT_MODERATION_Msk 0xFFUL
#define ETH_INT_MODERATION_TX_INT_MODERATION_Pos 16UL
#define ETH_INT_MODERATION_TX_INT_MODERATION_Msk 0xFF0000UL
/* ETH.SYS_WAKE_TIME */
#define ETH_SYS_WAKE_TIME_SYS_WAKE_TIME_Pos     0UL
#define ETH_SYS_WAKE_TIME_SYS_WAKE_TIME_Msk     0xFFFFUL
/* ETH.HASH_BOTTOM */
#define ETH_HASH_BOTTOM_ADDRESS_HASH_B_Pos      0UL
#define ETH_HASH_BOTTOM_ADDRESS_HASH_B_Msk      0xFFFFFFFFUL
/* ETH.HASH_TOP */
#define ETH_HASH_TOP_ADDRESS_HASH_T_Pos         0UL
#define ETH_HASH_TOP_ADDRESS_HASH_T_Msk         0xFFFFFFFFUL
/* ETH.SPEC_ADD1_BOTTOM */
#define ETH_SPEC_ADD1_BOTTOM_ADDRESS_ADD1_B_Pos 0UL
#define ETH_SPEC_ADD1_BOTTOM_ADDRESS_ADD1_B_Msk 0xFFFFFFFFUL
/* ETH.SPEC_ADD1_TOP */
#define ETH_SPEC_ADD1_TOP_ADDRESS_TOP_Pos       0UL
#define ETH_SPEC_ADD1_TOP_ADDRESS_TOP_Msk       0xFFFFUL
#define ETH_SPEC_ADD1_TOP_FILTER_TYPE_Pos       16UL
#define ETH_SPEC_ADD1_TOP_FILTER_TYPE_Msk       0x10000UL
/* ETH.SPEC_ADD2_BOTTOM */
#define ETH_SPEC_ADD2_BOTTOM_ADDRESS_BOTTOM_Pos 0UL
#define ETH_SPEC_ADD2_BOTTOM_ADDRESS_BOTTOM_Msk 0xFFFFFFFFUL
/* ETH.SPEC_ADD2_TOP */
#define ETH_SPEC_ADD2_TOP_ADDRESS_TOP_Pos       0UL
#define ETH_SPEC_ADD2_TOP_ADDRESS_TOP_Msk       0xFFFFUL
#define ETH_SPEC_ADD2_TOP_FILTER_TYPE_Pos       16UL
#define ETH_SPEC_ADD2_TOP_FILTER_TYPE_Msk       0x10000UL
#define ETH_SPEC_ADD2_TOP_FILTER_BYTE_MASK_Pos  24UL
#define ETH_SPEC_ADD2_TOP_FILTER_BYTE_MASK_Msk  0x3F000000UL
/* ETH.SPEC_ADD3_BOTTOM */
#define ETH_SPEC_ADD3_BOTTOM_ADDRESS_BOTTOM_Pos 0UL
#define ETH_SPEC_ADD3_BOTTOM_ADDRESS_BOTTOM_Msk 0xFFFFFFFFUL
/* ETH.SPEC_ADD3_TOP */
#define ETH_SPEC_ADD3_TOP_ADDRESS_TOP_Pos       0UL
#define ETH_SPEC_ADD3_TOP_ADDRESS_TOP_Msk       0xFFFFUL
#define ETH_SPEC_ADD3_TOP_FILTER_TYPE_Pos       16UL
#define ETH_SPEC_ADD3_TOP_FILTER_TYPE_Msk       0x10000UL
#define ETH_SPEC_ADD3_TOP_FILTER_BYTE_MASK_Pos  24UL
#define ETH_SPEC_ADD3_TOP_FILTER_BYTE_MASK_Msk  0x3F000000UL
/* ETH.SPEC_ADD4_BOTTOM */
#define ETH_SPEC_ADD4_BOTTOM_ADDRESS_BOTTOM_Pos 0UL
#define ETH_SPEC_ADD4_BOTTOM_ADDRESS_BOTTOM_Msk 0xFFFFFFFFUL
/* ETH.SPEC_ADD4_TOP */
#define ETH_SPEC_ADD4_TOP_ADDRESS_TOP_Pos       0UL
#define ETH_SPEC_ADD4_TOP_ADDRESS_TOP_Msk       0xFFFFUL
#define ETH_SPEC_ADD4_TOP_FILTER_TYPE_Pos       16UL
#define ETH_SPEC_ADD4_TOP_FILTER_TYPE_Msk       0x10000UL
#define ETH_SPEC_ADD4_TOP_FILTER_BYTE_MASK_Pos  24UL
#define ETH_SPEC_ADD4_TOP_FILTER_BYTE_MASK_Msk  0x3F000000UL
/* ETH.SPEC_TYPE1 */
#define ETH_SPEC_TYPE1_MATCH_Pos                0UL
#define ETH_SPEC_TYPE1_MATCH_Msk                0xFFFFUL
#define ETH_SPEC_TYPE1_ENABLE_COPY_Pos          31UL
#define ETH_SPEC_TYPE1_ENABLE_COPY_Msk          0x80000000UL
/* ETH.SPEC_TYPE2 */
#define ETH_SPEC_TYPE2_MATCH_Pos                0UL
#define ETH_SPEC_TYPE2_MATCH_Msk                0xFFFFUL
#define ETH_SPEC_TYPE2_ENABLE_COPY_Pos          31UL
#define ETH_SPEC_TYPE2_ENABLE_COPY_Msk          0x80000000UL
/* ETH.SPEC_TYPE3 */
#define ETH_SPEC_TYPE3_MATCH_Pos                0UL
#define ETH_SPEC_TYPE3_MATCH_Msk                0xFFFFUL
#define ETH_SPEC_TYPE3_ENABLE_COPY_Pos          31UL
#define ETH_SPEC_TYPE3_ENABLE_COPY_Msk          0x80000000UL
/* ETH.SPEC_TYPE4 */
#define ETH_SPEC_TYPE4_MATCH_Pos                0UL
#define ETH_SPEC_TYPE4_MATCH_Msk                0xFFFFUL
#define ETH_SPEC_TYPE4_ENABLE_COPY_Pos          31UL
#define ETH_SPEC_TYPE4_ENABLE_COPY_Msk          0x80000000UL
/* ETH.WOL_REGISTER */
#define ETH_WOL_REGISTER_ADDR_Pos               0UL
#define ETH_WOL_REGISTER_ADDR_Msk               0xFFFFUL
#define ETH_WOL_REGISTER_WOL_MASK_0_Pos         16UL
#define ETH_WOL_REGISTER_WOL_MASK_0_Msk         0x10000UL
#define ETH_WOL_REGISTER_WOL_MASK_1_Pos         17UL
#define ETH_WOL_REGISTER_WOL_MASK_1_Msk         0x20000UL
#define ETH_WOL_REGISTER_WOL_MASK_2_Pos         18UL
#define ETH_WOL_REGISTER_WOL_MASK_2_Msk         0x40000UL
#define ETH_WOL_REGISTER_WOL_MASK_3_Pos         19UL
#define ETH_WOL_REGISTER_WOL_MASK_3_Msk         0x80000UL
/* ETH.STRETCH_RATIO */
#define ETH_STRETCH_RATIO_IPG_STRETCH_Pos       0UL
#define ETH_STRETCH_RATIO_IPG_STRETCH_Msk       0xFFFFUL
/* ETH.STACKED_VLAN */
#define ETH_STACKED_VLAN_MATCH_Pos              0UL
#define ETH_STACKED_VLAN_MATCH_Msk              0xFFFFUL
#define ETH_STACKED_VLAN_ENABLE_PROCESSING_Pos  31UL
#define ETH_STACKED_VLAN_ENABLE_PROCESSING_Msk  0x80000000UL
/* ETH.TX_PFC_PAUSE */
#define ETH_TX_PFC_PAUSE_VECTOR_ENABLE_Pos      0UL
#define ETH_TX_PFC_PAUSE_VECTOR_ENABLE_Msk      0xFFUL
#define ETH_TX_PFC_PAUSE_VECTOR_Pos             8UL
#define ETH_TX_PFC_PAUSE_VECTOR_Msk             0xFF00UL
/* ETH.MASK_ADD1_BOTTOM */
#define ETH_MASK_ADD1_BOTTOM_ADDRESS_MASK_BOTTOM_Pos 0UL
#define ETH_MASK_ADD1_BOTTOM_ADDRESS_MASK_BOTTOM_Msk 0xFFFFFFFFUL
/* ETH.MASK_ADD1_TOP */
#define ETH_MASK_ADD1_TOP_ADDRESS_MASK_TOP_Pos  0UL
#define ETH_MASK_ADD1_TOP_ADDRESS_MASK_TOP_Msk  0xFFFFUL
/* ETH.DMA_ADDR_OR_MASK */
#define ETH_DMA_ADDR_OR_MASK_MASK_ENABLE_Pos    0UL
#define ETH_DMA_ADDR_OR_MASK_MASK_ENABLE_Msk    0xFUL
#define ETH_DMA_ADDR_OR_MASK_MASK_VALUE_DA_Pos  28UL
#define ETH_DMA_ADDR_OR_MASK_MASK_VALUE_DA_Msk  0xF0000000UL
/* ETH.RX_PTP_UNICAST */
#define ETH_RX_PTP_UNICAST_ADDRESS_UNICAST_Pos  0UL
#define ETH_RX_PTP_UNICAST_ADDRESS_UNICAST_Msk  0xFFFFFFFFUL
/* ETH.TX_PTP_UNICAST */
#define ETH_TX_PTP_UNICAST_ADDRESS_UNICAST_Pos  0UL
#define ETH_TX_PTP_UNICAST_ADDRESS_UNICAST_Msk  0xFFFFFFFFUL
/* ETH.TSU_NSEC_CMP */
#define ETH_TSU_NSEC_CMP_COMPARISON_NSEC_Pos    0UL
#define ETH_TSU_NSEC_CMP_COMPARISON_NSEC_Msk    0x3FFFFFUL
/* ETH.TSU_SEC_CMP */
#define ETH_TSU_SEC_CMP_COMPARISON_SEC_Pos      0UL
#define ETH_TSU_SEC_CMP_COMPARISON_SEC_Msk      0xFFFFFFFFUL
/* ETH.TSU_MSB_SEC_CMP */
#define ETH_TSU_MSB_SEC_CMP_COMPARISON_MSB_SEC_Pos 0UL
#define ETH_TSU_MSB_SEC_CMP_COMPARISON_MSB_SEC_Msk 0xFFFFUL
/* ETH.TSU_PTP_TX_MSB_SEC */
#define ETH_TSU_PTP_TX_MSB_SEC_TIMER_SECONDS_Pos 0UL
#define ETH_TSU_PTP_TX_MSB_SEC_TIMER_SECONDS_Msk 0xFFFFUL
/* ETH.TSU_PTP_RX_MSB_SEC */
#define ETH_TSU_PTP_RX_MSB_SEC_TIMER_SECONDS_Pos 0UL
#define ETH_TSU_PTP_RX_MSB_SEC_TIMER_SECONDS_Msk 0xFFFFUL
/* ETH.TSU_PEER_TX_MSB_SEC */
#define ETH_TSU_PEER_TX_MSB_SEC_TIMER_SECONDS_Pos 0UL
#define ETH_TSU_PEER_TX_MSB_SEC_TIMER_SECONDS_Msk 0xFFFFUL
/* ETH.TSU_PEER_RX_MSB_SEC */
#define ETH_TSU_PEER_RX_MSB_SEC_TIMER_SECONDS_Pos 0UL
#define ETH_TSU_PEER_RX_MSB_SEC_TIMER_SECONDS_Msk 0xFFFFUL
/* ETH.DPRAM_FILL_DBG */
#define ETH_DPRAM_FILL_DBG_DMA_TX_RX_FILL_LEVEL_SELECT_Pos 0UL
#define ETH_DPRAM_FILL_DBG_DMA_TX_RX_FILL_LEVEL_SELECT_Msk 0x1UL
#define ETH_DPRAM_FILL_DBG_DMA_TX_Q_FILL_LEVEL_SELECT_Pos 4UL
#define ETH_DPRAM_FILL_DBG_DMA_TX_Q_FILL_LEVEL_SELECT_Msk 0xF0UL
#define ETH_DPRAM_FILL_DBG_DMA_TX_RX_FILL_LEVEL_Pos 16UL
#define ETH_DPRAM_FILL_DBG_DMA_TX_RX_FILL_LEVEL_Msk 0xFFFF0000UL
/* ETH.REVISION_REG */
#define ETH_REVISION_REG_MODULE_REVISION_Pos    0UL
#define ETH_REVISION_REG_MODULE_REVISION_Msk    0xFFFFUL
#define ETH_REVISION_REG_MODULE_IDENTIFICATION_NUMBER_Pos 16UL
#define ETH_REVISION_REG_MODULE_IDENTIFICATION_NUMBER_Msk 0xFFF0000UL
#define ETH_REVISION_REG_FIX_NUMBER_Pos         28UL
#define ETH_REVISION_REG_FIX_NUMBER_Msk         0xF0000000UL
/* ETH.OCTETS_TXED_BOTTOM */
#define ETH_OCTETS_TXED_BOTTOM_COUNT_BOTTOM_Pos 0UL
#define ETH_OCTETS_TXED_BOTTOM_COUNT_BOTTOM_Msk 0xFFFFFFFFUL
/* ETH.OCTETS_TXED_TOP */
#define ETH_OCTETS_TXED_TOP_COUNT_TOP_Pos       0UL
#define ETH_OCTETS_TXED_TOP_COUNT_TOP_Msk       0xFFFFUL
/* ETH.FRAMES_TXED_OK */
#define ETH_FRAMES_TXED_OK_COUNT_OK_Pos         0UL
#define ETH_FRAMES_TXED_OK_COUNT_OK_Msk         0xFFFFFFFFUL
/* ETH.BROADCAST_TXED */
#define ETH_BROADCAST_TXED_COUNT_BROADCAST_Pos  0UL
#define ETH_BROADCAST_TXED_COUNT_BROADCAST_Msk  0xFFFFFFFFUL
/* ETH.MULTICAST_TXED */
#define ETH_MULTICAST_TXED_COUNT_MULTICAST_Pos  0UL
#define ETH_MULTICAST_TXED_COUNT_MULTICAST_Msk  0xFFFFFFFFUL
/* ETH.PAUSE_FRAMES_TXED */
#define ETH_PAUSE_FRAMES_TXED_COUNT_PAUSE_Pos   0UL
#define ETH_PAUSE_FRAMES_TXED_COUNT_PAUSE_Msk   0xFFFFUL
/* ETH.FRAMES_TXED_64 */
#define ETH_FRAMES_TXED_64_COUNT_64_Pos         0UL
#define ETH_FRAMES_TXED_64_COUNT_64_Msk         0xFFFFFFFFUL
/* ETH.FRAMES_TXED_65 */
#define ETH_FRAMES_TXED_65_COUNT_65_Pos         0UL
#define ETH_FRAMES_TXED_65_COUNT_65_Msk         0xFFFFFFFFUL
/* ETH.FRAMES_TXED_128 */
#define ETH_FRAMES_TXED_128_COUNT_128_Pos       0UL
#define ETH_FRAMES_TXED_128_COUNT_128_Msk       0xFFFFFFFFUL
/* ETH.FRAMES_TXED_256 */
#define ETH_FRAMES_TXED_256_COUNT_256_Pos       0UL
#define ETH_FRAMES_TXED_256_COUNT_256_Msk       0xFFFFFFFFUL
/* ETH.FRAMES_TXED_512 */
#define ETH_FRAMES_TXED_512_COUNT_512_Pos       0UL
#define ETH_FRAMES_TXED_512_COUNT_512_Msk       0xFFFFFFFFUL
/* ETH.FRAMES_TXED_1024 */
#define ETH_FRAMES_TXED_1024_COUNT_1024_Pos     0UL
#define ETH_FRAMES_TXED_1024_COUNT_1024_Msk     0xFFFFFFFFUL
/* ETH.FRAMES_TXED_1519 */
#define ETH_FRAMES_TXED_1519_COUNT_1519_Pos     0UL
#define ETH_FRAMES_TXED_1519_COUNT_1519_Msk     0xFFFFFFFFUL
/* ETH.TX_UNDERRUNS */
#define ETH_TX_UNDERRUNS_COUNT_UN_Pos           0UL
#define ETH_TX_UNDERRUNS_COUNT_UN_Msk           0x3FFUL
/* ETH.SINGLE_COLLISIONS */
#define ETH_SINGLE_COLLISIONS_COUNT14_Pos       0UL
#define ETH_SINGLE_COLLISIONS_COUNT14_Msk       0x3FFFFUL
/* ETH.MULTIPLE_COLLISIONS */
#define ETH_MULTIPLE_COLLISIONS_COUNT15_Pos     0UL
#define ETH_MULTIPLE_COLLISIONS_COUNT15_Msk     0x3FFFFUL
/* ETH.EXCESSIVE_COLLISIONS */
#define ETH_EXCESSIVE_COLLISIONS_COUNT16_Pos    0UL
#define ETH_EXCESSIVE_COLLISIONS_COUNT16_Msk    0x3FFUL
/* ETH.LATE_COLLISIONS */
#define ETH_LATE_COLLISIONS_COUNT17_Pos         0UL
#define ETH_LATE_COLLISIONS_COUNT17_Msk         0x3FFUL
/* ETH.DEFERRED_FRAMES */
#define ETH_DEFERRED_FRAMES_COUNT18_Pos         0UL
#define ETH_DEFERRED_FRAMES_COUNT18_Msk         0x3FFFFUL
/* ETH.CRS_ERRORS */
#define ETH_CRS_ERRORS_COUNT19_Pos              0UL
#define ETH_CRS_ERRORS_COUNT19_Msk              0x3FFUL
/* ETH.OCTETS_RXED_BOTTOM */
#define ETH_OCTETS_RXED_BOTTOM_COUNT_BOTTOM_Pos 0UL
#define ETH_OCTETS_RXED_BOTTOM_COUNT_BOTTOM_Msk 0xFFFFFFFFUL
/* ETH.OCTETS_RXED_TOP */
#define ETH_OCTETS_RXED_TOP_COUNT_TOP_Pos       0UL
#define ETH_OCTETS_RXED_TOP_COUNT_TOP_Msk       0xFFFFUL
/* ETH.FRAMES_RXED_OK */
#define ETH_FRAMES_RXED_OK_COUNT_OK_Pos         0UL
#define ETH_FRAMES_RXED_OK_COUNT_OK_Msk         0xFFFFFFFFUL
/* ETH.BROADCAST_RXED */
#define ETH_BROADCAST_RXED_COUNT_BROADCAST_Pos  0UL
#define ETH_BROADCAST_RXED_COUNT_BROADCAST_Msk  0xFFFFFFFFUL
/* ETH.MULTICAST_RXED */
#define ETH_MULTICAST_RXED_COUNT_MULTICAST_Pos  0UL
#define ETH_MULTICAST_RXED_COUNT_MULTICAST_Msk  0xFFFFFFFFUL
/* ETH.PAUSE_FRAMES_RXED */
#define ETH_PAUSE_FRAMES_RXED_COUNT_PAUSE_Pos   0UL
#define ETH_PAUSE_FRAMES_RXED_COUNT_PAUSE_Msk   0xFFFFUL
/* ETH.FRAMES_RXED_64 */
#define ETH_FRAMES_RXED_64_COUNT_64_Pos         0UL
#define ETH_FRAMES_RXED_64_COUNT_64_Msk         0xFFFFFFFFUL
/* ETH.FRAMES_RXED_65 */
#define ETH_FRAMES_RXED_65_COUNT_65_Pos         0UL
#define ETH_FRAMES_RXED_65_COUNT_65_Msk         0xFFFFFFFFUL
/* ETH.FRAMES_RXED_128 */
#define ETH_FRAMES_RXED_128_COUNT_128_Pos       0UL
#define ETH_FRAMES_RXED_128_COUNT_128_Msk       0xFFFFFFFFUL
/* ETH.FRAMES_RXED_256 */
#define ETH_FRAMES_RXED_256_COUNT_256_Pos       0UL
#define ETH_FRAMES_RXED_256_COUNT_256_Msk       0xFFFFFFFFUL
/* ETH.FRAMES_RXED_512 */
#define ETH_FRAMES_RXED_512_COUNT_512_Pos       0UL
#define ETH_FRAMES_RXED_512_COUNT_512_Msk       0xFFFFFFFFUL
/* ETH.FRAMES_RXED_1024 */
#define ETH_FRAMES_RXED_1024_COUNT_1024_Pos     0UL
#define ETH_FRAMES_RXED_1024_COUNT_1024_Msk     0xFFFFFFFFUL
/* ETH.FRAMES_RXED_1519 */
#define ETH_FRAMES_RXED_1519_COUNT_1519_Pos     0UL
#define ETH_FRAMES_RXED_1519_COUNT_1519_Msk     0xFFFFFFFFUL
/* ETH.UNDERSIZE_FRAMES */
#define ETH_UNDERSIZE_FRAMES_COUNT_UNDERSIZE_Pos 0UL
#define ETH_UNDERSIZE_FRAMES_COUNT_UNDERSIZE_Msk 0x3FFUL
/* ETH.EXCESSIVE_RX_LENGTH */
#define ETH_EXCESSIVE_RX_LENGTH_COUNT_OVERSIZE_Pos 0UL
#define ETH_EXCESSIVE_RX_LENGTH_COUNT_OVERSIZE_Msk 0x3FFUL
/* ETH.RX_JABBERS */
#define ETH_RX_JABBERS_COUNT_JABBERS_Pos        0UL
#define ETH_RX_JABBERS_COUNT_JABBERS_Msk        0x3FFUL
/* ETH.FCS_ERRORS */
#define ETH_FCS_ERRORS_COUNT_FCS_ERR_Pos        0UL
#define ETH_FCS_ERRORS_COUNT_FCS_ERR_Msk        0x3FFUL
/* ETH.RX_LENGTH_ERRORS */
#define ETH_RX_LENGTH_ERRORS_COUNT_LENGTH_ERR_Pos 0UL
#define ETH_RX_LENGTH_ERRORS_COUNT_LENGTH_ERR_Msk 0x3FFUL
/* ETH.RX_SYMBOL_ERRORS */
#define ETH_RX_SYMBOL_ERRORS_COUNT_SYMBOL_ERR_Pos 0UL
#define ETH_RX_SYMBOL_ERRORS_COUNT_SYMBOL_ERR_Msk 0x3FFUL
/* ETH.ALIGNMENT_ERRORS */
#define ETH_ALIGNMENT_ERRORS_COUNT_ALIGNMENT_ERROR_Pos 0UL
#define ETH_ALIGNMENT_ERRORS_COUNT_ALIGNMENT_ERROR_Msk 0x3FFUL
/* ETH.RX_RESOURCE_ERRORS */
#define ETH_RX_RESOURCE_ERRORS_COUNT_RESOURCE_ERR_Pos 0UL
#define ETH_RX_RESOURCE_ERRORS_COUNT_RESOURCE_ERR_Msk 0x3FFFFUL
/* ETH.RX_OVERRUNS */
#define ETH_RX_OVERRUNS_COUNT_OVERRUN_Pos       0UL
#define ETH_RX_OVERRUNS_COUNT_OVERRUN_Msk       0x3FFUL
/* ETH.RX_IP_CK_ERRORS */
#define ETH_RX_IP_CK_ERRORS_COUNT_IPCK_ERR_Pos  0UL
#define ETH_RX_IP_CK_ERRORS_COUNT_IPCK_ERR_Msk  0xFFUL
/* ETH.RX_TCP_CK_ERRORS */
#define ETH_RX_TCP_CK_ERRORS_COUNT_TCPCK_ERR_Pos 0UL
#define ETH_RX_TCP_CK_ERRORS_COUNT_TCPCK_ERR_Msk 0xFFUL
/* ETH.RX_UDP_CK_ERRORS */
#define ETH_RX_UDP_CK_ERRORS_COUNT_UDPCK_ERR_Pos 0UL
#define ETH_RX_UDP_CK_ERRORS_COUNT_UDPCK_ERR_Msk 0xFFUL
/* ETH.AUTO_FLUSHED_PKTS */
#define ETH_AUTO_FLUSHED_PKTS_COUNT_FLUSHED_Pos 0UL
#define ETH_AUTO_FLUSHED_PKTS_COUNT_FLUSHED_Msk 0xFFFFUL
/* ETH.TSU_TIMER_INCR_SUB_NSEC */
#define ETH_TSU_TIMER_INCR_SUB_NSEC_SUB_NS_INCR_Pos 0UL
#define ETH_TSU_TIMER_INCR_SUB_NSEC_SUB_NS_INCR_Msk 0xFFFFUL
#define ETH_TSU_TIMER_INCR_SUB_NSEC_SUB_NS_INCR_LSB_Pos 24UL
#define ETH_TSU_TIMER_INCR_SUB_NSEC_SUB_NS_INCR_LSB_Msk 0xFF000000UL
/* ETH.TSU_TIMER_MSB_SEC */
#define ETH_TSU_TIMER_MSB_SEC_TIMER_MSB_SEC_Pos 0UL
#define ETH_TSU_TIMER_MSB_SEC_TIMER_MSB_SEC_Msk 0xFFFFUL
/* ETH.TSU_STROBE_MSB_SEC */
#define ETH_TSU_STROBE_MSB_SEC_STROBE_MSB_SEC_Pos 0UL
#define ETH_TSU_STROBE_MSB_SEC_STROBE_MSB_SEC_Msk 0xFFFFUL
/* ETH.TSU_STROBE_SEC */
#define ETH_TSU_STROBE_SEC_STROBE_SEC_Pos       0UL
#define ETH_TSU_STROBE_SEC_STROBE_SEC_Msk       0xFFFFFFFFUL
/* ETH.TSU_STROBE_NSEC */
#define ETH_TSU_STROBE_NSEC_STROBE_NSEC_Pos     0UL
#define ETH_TSU_STROBE_NSEC_STROBE_NSEC_Msk     0x3FFFFFFFUL
/* ETH.TSU_TIMER_SEC */
#define ETH_TSU_TIMER_SEC_TIMER_SEC_Pos         0UL
#define ETH_TSU_TIMER_SEC_TIMER_SEC_Msk         0xFFFFFFFFUL
/* ETH.TSU_TIMER_NSEC */
#define ETH_TSU_TIMER_NSEC_TIMER_NSEC_Pos       0UL
#define ETH_TSU_TIMER_NSEC_TIMER_NSEC_Msk       0x3FFFFFFFUL
/* ETH.TSU_TIMER_ADJUST */
#define ETH_TSU_TIMER_ADJUST_INCREMENT_VALUE_Pos 0UL
#define ETH_TSU_TIMER_ADJUST_INCREMENT_VALUE_Msk 0x3FFFFFFFUL
#define ETH_TSU_TIMER_ADJUST_ADD_SUBTRACT_Pos   31UL
#define ETH_TSU_TIMER_ADJUST_ADD_SUBTRACT_Msk   0x80000000UL
/* ETH.TSU_TIMER_INCR */
#define ETH_TSU_TIMER_INCR_NS_INCREMENT_Pos     0UL
#define ETH_TSU_TIMER_INCR_NS_INCREMENT_Msk     0xFFUL
#define ETH_TSU_TIMER_INCR_ALT_NS_INCR_Pos      8UL
#define ETH_TSU_TIMER_INCR_ALT_NS_INCR_Msk      0xFF00UL
#define ETH_TSU_TIMER_INCR_NUM_INCS_Pos         16UL
#define ETH_TSU_TIMER_INCR_NUM_INCS_Msk         0xFF0000UL
/* ETH.TSU_PTP_TX_SEC */
#define ETH_TSU_PTP_TX_SEC_TIMER_PTP_SEC_Pos    0UL
#define ETH_TSU_PTP_TX_SEC_TIMER_PTP_SEC_Msk    0xFFFFFFFFUL
/* ETH.TSU_PTP_TX_NSEC */
#define ETH_TSU_PTP_TX_NSEC_TIMER_PTP_NSEC_Pos  0UL
#define ETH_TSU_PTP_TX_NSEC_TIMER_PTP_NSEC_Msk  0x3FFFFFFFUL
/* ETH.TSU_PTP_RX_SEC */
#define ETH_TSU_PTP_RX_SEC_TIMER_PTP_SEC_Pos    0UL
#define ETH_TSU_PTP_RX_SEC_TIMER_PTP_SEC_Msk    0xFFFFFFFFUL
/* ETH.TSU_PTP_RX_NSEC */
#define ETH_TSU_PTP_RX_NSEC_TIMER_PTP_NSEC_Pos  0UL
#define ETH_TSU_PTP_RX_NSEC_TIMER_PTP_NSEC_Msk  0x3FFFFFFFUL
/* ETH.TSU_PEER_TX_SEC */
#define ETH_TSU_PEER_TX_SEC_TIMER_PEER_SEC_Pos  0UL
#define ETH_TSU_PEER_TX_SEC_TIMER_PEER_SEC_Msk  0xFFFFFFFFUL
/* ETH.TSU_PEER_TX_NSEC */
#define ETH_TSU_PEER_TX_NSEC_TIMER_PEER_NSEC_Pos 0UL
#define ETH_TSU_PEER_TX_NSEC_TIMER_PEER_NSEC_Msk 0x3FFFFFFFUL
/* ETH.TSU_PEER_RX_SEC */
#define ETH_TSU_PEER_RX_SEC_TIMER_PEER_SEC_Pos  0UL
#define ETH_TSU_PEER_RX_SEC_TIMER_PEER_SEC_Msk  0xFFFFFFFFUL
/* ETH.TSU_PEER_RX_NSEC */
#define ETH_TSU_PEER_RX_NSEC_TIMER_PEER_NSEC_Pos 0UL
#define ETH_TSU_PEER_RX_NSEC_TIMER_PEER_NSEC_Msk 0x3FFFFFFFUL
/* ETH.PCS_CONTROL */
#define ETH_PCS_CONTROL_REMOVED_31_0_Pos        0UL
#define ETH_PCS_CONTROL_REMOVED_31_0_Msk        0xFFFFFFFFUL
/* ETH.PCS_STATUS */
#define ETH_PCS_STATUS_REMOVED_31_0_Pos         0UL
#define ETH_PCS_STATUS_REMOVED_31_0_Msk         0xFFFFFFFFUL
/* ETH.PCS_AN_ADV */
#define ETH_PCS_AN_ADV_REMOVED_31_0_Pos         0UL
#define ETH_PCS_AN_ADV_REMOVED_31_0_Msk         0xFFFFFFFFUL
/* ETH.PCS_AN_LP_BASE */
#define ETH_PCS_AN_LP_BASE_REMOVED_31_0_Pos     0UL
#define ETH_PCS_AN_LP_BASE_REMOVED_31_0_Msk     0xFFFFFFFFUL
/* ETH.PCS_AN_EXP */
#define ETH_PCS_AN_EXP_REMOVED_31_0_Pos         0UL
#define ETH_PCS_AN_EXP_REMOVED_31_0_Msk         0xFFFFFFFFUL
/* ETH.PCS_AN_NP_TX */
#define ETH_PCS_AN_NP_TX_REMOVED_31_0_Pos       0UL
#define ETH_PCS_AN_NP_TX_REMOVED_31_0_Msk       0xFFFFFFFFUL
/* ETH.PCS_AN_LP_NP */
#define ETH_PCS_AN_LP_NP_REMOVED_31_0_Pos       0UL
#define ETH_PCS_AN_LP_NP_REMOVED_31_0_Msk       0xFFFFFFFFUL
/* ETH.PCS_AN_EXT_STATUS */
#define ETH_PCS_AN_EXT_STATUS_REMOVED_31_0_Pos  0UL
#define ETH_PCS_AN_EXT_STATUS_REMOVED_31_0_Msk  0xFFFFFFFFUL
/* ETH.TX_PAUSE_QUANTUM1 */
#define ETH_TX_PAUSE_QUANTUM1_QUANTUM_P2_Pos    0UL
#define ETH_TX_PAUSE_QUANTUM1_QUANTUM_P2_Msk    0xFFFFUL
#define ETH_TX_PAUSE_QUANTUM1_QUANTUM_P3_Pos    16UL
#define ETH_TX_PAUSE_QUANTUM1_QUANTUM_P3_Msk    0xFFFF0000UL
/* ETH.TX_PAUSE_QUANTUM2 */
#define ETH_TX_PAUSE_QUANTUM2_QUANTUM_P4_Pos    0UL
#define ETH_TX_PAUSE_QUANTUM2_QUANTUM_P4_Msk    0xFFFFUL
#define ETH_TX_PAUSE_QUANTUM2_QUANTUM_P5_Pos    16UL
#define ETH_TX_PAUSE_QUANTUM2_QUANTUM_P5_Msk    0xFFFF0000UL
/* ETH.TX_PAUSE_QUANTUM3 */
#define ETH_TX_PAUSE_QUANTUM3_QUANTUM_P6_Pos    0UL
#define ETH_TX_PAUSE_QUANTUM3_QUANTUM_P6_Msk    0xFFFFUL
#define ETH_TX_PAUSE_QUANTUM3_QUANTUM_P7_Pos    16UL
#define ETH_TX_PAUSE_QUANTUM3_QUANTUM_P7_Msk    0xFFFF0000UL
/* ETH.RX_LPI */
#define ETH_RX_LPI_COUNT_LPI_Pos                0UL
#define ETH_RX_LPI_COUNT_LPI_Msk                0xFFFFUL
/* ETH.RX_LPI_TIME */
#define ETH_RX_LPI_TIME_LPI_TIME_Pos            0UL
#define ETH_RX_LPI_TIME_LPI_TIME_Msk            0xFFFFFFUL
/* ETH.TX_LPI */
#define ETH_TX_LPI_COUNT_LPI_Pos                0UL
#define ETH_TX_LPI_COUNT_LPI_Msk                0xFFFFUL
/* ETH.TX_LPI_TIME */
#define ETH_TX_LPI_TIME_LPI_TIME_Pos            0UL
#define ETH_TX_LPI_TIME_LPI_TIME_Msk            0xFFFFFFUL
/* ETH.DESIGNCFG_DEBUG1 */
#define ETH_DESIGNCFG_DEBUG1_NO_PCS_Pos         0UL
#define ETH_DESIGNCFG_DEBUG1_NO_PCS_Msk         0x1UL
#define ETH_DESIGNCFG_DEBUG1_EXCLUDE_QBV_Pos    1UL
#define ETH_DESIGNCFG_DEBUG1_EXCLUDE_QBV_Msk    0x2UL
#define ETH_DESIGNCFG_DEBUG1_RESERVED_2_Pos     2UL
#define ETH_DESIGNCFG_DEBUG1_RESERVED_2_Msk     0xCUL
#define ETH_DESIGNCFG_DEBUG1_INT_LOOPBACK_Pos   4UL
#define ETH_DESIGNCFG_DEBUG1_INT_LOOPBACK_Msk   0x10UL
#define ETH_DESIGNCFG_DEBUG1_RESERVED_5_Pos     5UL
#define ETH_DESIGNCFG_DEBUG1_RESERVED_5_Msk     0x20UL
#define ETH_DESIGNCFG_DEBUG1_EXT_FIFO_INTERFACE_Pos 6UL
#define ETH_DESIGNCFG_DEBUG1_EXT_FIFO_INTERFACE_Msk 0x40UL
#define ETH_DESIGNCFG_DEBUG1_RESERVED_7_Pos     7UL
#define ETH_DESIGNCFG_DEBUG1_RESERVED_7_Msk     0x80UL
#define ETH_DESIGNCFG_DEBUG1_RESERVED_8_Pos     8UL
#define ETH_DESIGNCFG_DEBUG1_RESERVED_8_Msk     0x100UL
#define ETH_DESIGNCFG_DEBUG1_USER_IO_Pos        9UL
#define ETH_DESIGNCFG_DEBUG1_USER_IO_Msk        0x200UL
#define ETH_DESIGNCFG_DEBUG1_USER_OUT_WIDTH_Pos 10UL
#define ETH_DESIGNCFG_DEBUG1_USER_OUT_WIDTH_Msk 0x7C00UL
#define ETH_DESIGNCFG_DEBUG1_USER_IN_WIDTH_Pos  15UL
#define ETH_DESIGNCFG_DEBUG1_USER_IN_WIDTH_Msk  0xF8000UL
#define ETH_DESIGNCFG_DEBUG1_RESERVED_20_Pos    20UL
#define ETH_DESIGNCFG_DEBUG1_RESERVED_20_Msk    0x100000UL
#define ETH_DESIGNCFG_DEBUG1_NO_STATS_Pos       21UL
#define ETH_DESIGNCFG_DEBUG1_NO_STATS_Msk       0x200000UL
#define ETH_DESIGNCFG_DEBUG1_NO_SNAPSHOT_Pos    22UL
#define ETH_DESIGNCFG_DEBUG1_NO_SNAPSHOT_Msk    0x400000UL
#define ETH_DESIGNCFG_DEBUG1_IRQ_READ_CLEAR_Pos 23UL
#define ETH_DESIGNCFG_DEBUG1_IRQ_READ_CLEAR_Msk 0x800000UL
#define ETH_DESIGNCFG_DEBUG1_EXCLUDE_CBS_Pos    24UL
#define ETH_DESIGNCFG_DEBUG1_EXCLUDE_CBS_Msk    0x1000000UL
#define ETH_DESIGNCFG_DEBUG1_DMA_BUS_WIDTH_Pos  25UL
#define ETH_DESIGNCFG_DEBUG1_DMA_BUS_WIDTH_Msk  0xE000000UL
#define ETH_DESIGNCFG_DEBUG1_AXI_CACHE_VALUE_Pos 28UL
#define ETH_DESIGNCFG_DEBUG1_AXI_CACHE_VALUE_Msk 0xF0000000UL
/* ETH.DESIGNCFG_DEBUG2 */
#define ETH_DESIGNCFG_DEBUG2_JUMBO_MAX_LENGTH_Pos 0UL
#define ETH_DESIGNCFG_DEBUG2_JUMBO_MAX_LENGTH_Msk 0x3FFFUL
#define ETH_DESIGNCFG_DEBUG2_HPROT_VALUE_Pos    16UL
#define ETH_DESIGNCFG_DEBUG2_HPROT_VALUE_Msk    0xF0000UL
#define ETH_DESIGNCFG_DEBUG2_RX_PKT_BUFFER_Pos  20UL
#define ETH_DESIGNCFG_DEBUG2_RX_PKT_BUFFER_Msk  0x100000UL
#define ETH_DESIGNCFG_DEBUG2_TX_PKT_BUFFER_Pos  21UL
#define ETH_DESIGNCFG_DEBUG2_TX_PKT_BUFFER_Msk  0x200000UL
#define ETH_DESIGNCFG_DEBUG2_RX_PBUF_ADDR_Pos   22UL
#define ETH_DESIGNCFG_DEBUG2_RX_PBUF_ADDR_Msk   0x3C00000UL
#define ETH_DESIGNCFG_DEBUG2_TX_PBUF_ADDR_Pos   26UL
#define ETH_DESIGNCFG_DEBUG2_TX_PBUF_ADDR_Msk   0x3C000000UL
#define ETH_DESIGNCFG_DEBUG2_AXI_Pos            30UL
#define ETH_DESIGNCFG_DEBUG2_AXI_Msk            0x40000000UL
#define ETH_DESIGNCFG_DEBUG2_SPRAM_Pos          31UL
#define ETH_DESIGNCFG_DEBUG2_SPRAM_Msk          0x80000000UL
/* ETH.DESIGNCFG_DEBUG3 */
#define ETH_DESIGNCFG_DEBUG3_NUM_SPEC_ADD_FILTERS_Pos 24UL
#define ETH_DESIGNCFG_DEBUG3_NUM_SPEC_ADD_FILTERS_Msk 0x3F000000UL
/* ETH.DESIGNCFG_DEBUG4 */
#define ETH_DESIGNCFG_DEBUG4_RESERVED_31_0_Pos  0UL
#define ETH_DESIGNCFG_DEBUG4_RESERVED_31_0_Msk  0xFFFFFFFFUL
/* ETH.DESIGNCFG_DEBUG5 */
#define ETH_DESIGNCFG_DEBUG5_RX_FIFO_CNT_WIDTH_Pos 0UL
#define ETH_DESIGNCFG_DEBUG5_RX_FIFO_CNT_WIDTH_Msk 0xFUL
#define ETH_DESIGNCFG_DEBUG5_TX_FIFO_CNT_WIDTH_Pos 4UL
#define ETH_DESIGNCFG_DEBUG5_TX_FIFO_CNT_WIDTH_Msk 0xF0UL
#define ETH_DESIGNCFG_DEBUG5_TSU_Pos            8UL
#define ETH_DESIGNCFG_DEBUG5_TSU_Msk            0x100UL
#define ETH_DESIGNCFG_DEBUG5_PHY_IDENT_Pos      9UL
#define ETH_DESIGNCFG_DEBUG5_PHY_IDENT_Msk      0x200UL
#define ETH_DESIGNCFG_DEBUG5_DMA_BUS_WIDTH_DEF_Pos 10UL
#define ETH_DESIGNCFG_DEBUG5_DMA_BUS_WIDTH_DEF_Msk 0xC00UL
#define ETH_DESIGNCFG_DEBUG5_MDC_CLOCK_DIV_Pos  12UL
#define ETH_DESIGNCFG_DEBUG5_MDC_CLOCK_DIV_Msk  0x7000UL
#define ETH_DESIGNCFG_DEBUG5_ENDIAN_SWAP_DEF_Pos 15UL
#define ETH_DESIGNCFG_DEBUG5_ENDIAN_SWAP_DEF_Msk 0x18000UL
#define ETH_DESIGNCFG_DEBUG5_RX_PBUF_SIZE_DEF_Pos 17UL
#define ETH_DESIGNCFG_DEBUG5_RX_PBUF_SIZE_DEF_Msk 0x60000UL
#define ETH_DESIGNCFG_DEBUG5_TX_PBUF_SIZE_DEF_Pos 19UL
#define ETH_DESIGNCFG_DEBUG5_TX_PBUF_SIZE_DEF_Msk 0x80000UL
#define ETH_DESIGNCFG_DEBUG5_RX_BUFFER_LENGTH_DEF_Pos 20UL
#define ETH_DESIGNCFG_DEBUG5_RX_BUFFER_LENGTH_DEF_Msk 0xFF00000UL
#define ETH_DESIGNCFG_DEBUG5_TSU_CLK_Pos        28UL
#define ETH_DESIGNCFG_DEBUG5_TSU_CLK_Msk        0x10000000UL
#define ETH_DESIGNCFG_DEBUG5_AXI_PROT_VALUE_Pos 29UL
#define ETH_DESIGNCFG_DEBUG5_AXI_PROT_VALUE_Msk 0xE0000000UL
/* ETH.DESIGNCFG_DEBUG6 */
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE1_Pos 1UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE1_Msk 0x2UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE2_Pos 2UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE2_Msk 0x4UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE3_Pos 3UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE3_Msk 0x8UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE4_Pos 4UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE4_Msk 0x10UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE5_Pos 5UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE5_Msk 0x20UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE6_Pos 6UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE6_Msk 0x40UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE7_Pos 7UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE7_Msk 0x80UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE8_Pos 8UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE8_Msk 0x100UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE9_Pos 9UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE9_Msk 0x200UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE10_Pos 10UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE10_Msk 0x400UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE11_Pos 11UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE11_Msk 0x800UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE12_Pos 12UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE12_Msk 0x1000UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE13_Pos 13UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE13_Msk 0x2000UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE14_Pos 14UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE14_Msk 0x4000UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE15_Pos 15UL
#define ETH_DESIGNCFG_DEBUG6_DMA_PRIORITY_QUEUE15_Msk 0x8000UL
#define ETH_DESIGNCFG_DEBUG6_TX_PBUF_QUEUE_SEGMENT_SIZE_Pos 16UL
#define ETH_DESIGNCFG_DEBUG6_TX_PBUF_QUEUE_SEGMENT_SIZE_Msk 0xF0000UL
#define ETH_DESIGNCFG_DEBUG6_EXT_TSU_TIMER_Pos  20UL
#define ETH_DESIGNCFG_DEBUG6_EXT_TSU_TIMER_Msk  0x100000UL
#define ETH_DESIGNCFG_DEBUG6_TX_ADD_FIFO_IF_Pos 21UL
#define ETH_DESIGNCFG_DEBUG6_TX_ADD_FIFO_IF_Msk 0x200000UL
#define ETH_DESIGNCFG_DEBUG6_HOST_IF_SOFT_SELECT_Pos 22UL
#define ETH_DESIGNCFG_DEBUG6_HOST_IF_SOFT_SELECT_Msk 0x400000UL
#define ETH_DESIGNCFG_DEBUG6_DMA_ADDR_WIDTH_IS_64B_Pos 23UL
#define ETH_DESIGNCFG_DEBUG6_DMA_ADDR_WIDTH_IS_64B_Msk 0x800000UL
#define ETH_DESIGNCFG_DEBUG6_PFC_MULTI_QUANTUM_Pos 24UL
#define ETH_DESIGNCFG_DEBUG6_PFC_MULTI_QUANTUM_Msk 0x1000000UL
#define ETH_DESIGNCFG_DEBUG6_PBUF_CUTTHRU_Pos   25UL
#define ETH_DESIGNCFG_DEBUG6_PBUF_CUTTHRU_Msk   0x2000000UL
#define ETH_DESIGNCFG_DEBUG6_PBUF_RSC_Pos       26UL
#define ETH_DESIGNCFG_DEBUG6_PBUF_RSC_Msk       0x4000000UL
#define ETH_DESIGNCFG_DEBUG6_PBUF_LSO_Pos       27UL
#define ETH_DESIGNCFG_DEBUG6_PBUF_LSO_Msk       0x8000000UL
/* ETH.DESIGNCFG_DEBUG7 */
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q0_Pos 0UL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q0_Msk 0xFUL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q1_Pos 4UL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q1_Msk 0xF0UL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q2_Pos 8UL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q2_Msk 0xF00UL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q3_Pos 12UL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q3_Msk 0xF000UL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q4_Pos 16UL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q4_Msk 0xF0000UL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q5_Pos 20UL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q5_Msk 0xF00000UL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q6_Pos 24UL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q6_Msk 0xF000000UL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q7_Pos 28UL
#define ETH_DESIGNCFG_DEBUG7_X_PBUF_NUM_SEGMENTS_Q7_Msk 0xF0000000UL
/* ETH.DESIGNCFG_DEBUG8 */
#define ETH_DESIGNCFG_DEBUG8_NUM_SCR2_COMPARE_REGS_Pos 0UL
#define ETH_DESIGNCFG_DEBUG8_NUM_SCR2_COMPARE_REGS_Msk 0xFFUL
#define ETH_DESIGNCFG_DEBUG8_NUM_SCR2_ETHTYPE_REGS_Pos 8UL
#define ETH_DESIGNCFG_DEBUG8_NUM_SCR2_ETHTYPE_REGS_Msk 0xFF00UL
#define ETH_DESIGNCFG_DEBUG8_NUM_TYPE2_SCREENERS_Pos 16UL
#define ETH_DESIGNCFG_DEBUG8_NUM_TYPE2_SCREENERS_Msk 0xFF0000UL
#define ETH_DESIGNCFG_DEBUG8_NUM_TYPE1_SCREENERS_Pos 24UL
#define ETH_DESIGNCFG_DEBUG8_NUM_TYPE1_SCREENERS_Msk 0xFF000000UL
/* ETH.DESIGNCFG_DEBUG9 */
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q8_Pos 0UL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q8_Msk 0xFUL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q9_Pos 4UL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q9_Msk 0xF0UL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q10_Pos 8UL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q10_Msk 0xF00UL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q11_Pos 12UL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q11_Msk 0xF000UL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q12_Pos 16UL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q12_Msk 0xF0000UL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q13_Pos 20UL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q13_Msk 0xF00000UL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q14_Pos 24UL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q14_Msk 0xF000000UL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q15_Pos 28UL
#define ETH_DESIGNCFG_DEBUG9_TX_PBUF_NUM_SEGMENTS_Q15_Msk 0xF0000000UL
/* ETH.DESIGNCFG_DEBUG10 */
#define ETH_DESIGNCFG_DEBUG10_AXI_RX_DESCR_WR_BUFF_BITS_Pos 0UL
#define ETH_DESIGNCFG_DEBUG10_AXI_RX_DESCR_WR_BUFF_BITS_Msk 0xFUL
#define ETH_DESIGNCFG_DEBUG10_AXI_TX_DESCR_WR_BUFF_BITS_Pos 4UL
#define ETH_DESIGNCFG_DEBUG10_AXI_TX_DESCR_WR_BUFF_BITS_Msk 0xF0UL
#define ETH_DESIGNCFG_DEBUG10_AXI_RX_DESCR_RD_BUFF_BITS_Pos 8UL
#define ETH_DESIGNCFG_DEBUG10_AXI_RX_DESCR_RD_BUFF_BITS_Msk 0xF00UL
#define ETH_DESIGNCFG_DEBUG10_AXI_TX_DESCR_RD_BUFF_BITS_Pos 12UL
#define ETH_DESIGNCFG_DEBUG10_AXI_TX_DESCR_RD_BUFF_BITS_Msk 0xF000UL
#define ETH_DESIGNCFG_DEBUG10_AXI_ACCESS_PIPELINE_BITS_Pos 16UL
#define ETH_DESIGNCFG_DEBUG10_AXI_ACCESS_PIPELINE_BITS_Msk 0xF0000UL
#define ETH_DESIGNCFG_DEBUG10_RX_PBUF_DATA_Pos  20UL
#define ETH_DESIGNCFG_DEBUG10_RX_PBUF_DATA_Msk  0xF00000UL
#define ETH_DESIGNCFG_DEBUG10_TX_PBUF_DATA_Pos  24UL
#define ETH_DESIGNCFG_DEBUG10_TX_PBUF_DATA_Msk  0xF000000UL
#define ETH_DESIGNCFG_DEBUG10_EMAC_BUS_WIDTH_Pos 28UL
#define ETH_DESIGNCFG_DEBUG10_EMAC_BUS_WIDTH_Msk 0xF0000000UL
/* ETH.SPEC_ADD5_BOTTOM */
#define ETH_SPEC_ADD5_BOTTOM_RESERVED_31_0_Pos  0UL
#define ETH_SPEC_ADD5_BOTTOM_RESERVED_31_0_Msk  0xFFFFFFFFUL
/* ETH.SPEC_ADD5_TOP */
#define ETH_SPEC_ADD5_TOP_RESERVED_31_0_Pos     0UL
#define ETH_SPEC_ADD5_TOP_RESERVED_31_0_Msk     0xFFFFFFFFUL
/* ETH.SPEC_ADD36_BOTTOM */
#define ETH_SPEC_ADD36_BOTTOM_RESERVED_31_0_Pos 0UL
#define ETH_SPEC_ADD36_BOTTOM_RESERVED_31_0_Msk 0xFFFFFFFFUL
/* ETH.SPEC_ADD36_TOP */
#define ETH_SPEC_ADD36_TOP_RESERVED_31_0_Pos    0UL
#define ETH_SPEC_ADD36_TOP_RESERVED_31_0_Msk    0xFFFFFFFFUL
/* ETH.INT_Q1_STATUS */
#define ETH_INT_Q1_STATUS_RECEIVE_COMPLETE_Pos  1UL
#define ETH_INT_Q1_STATUS_RECEIVE_COMPLETE_Msk  0x2UL
#define ETH_INT_Q1_STATUS_RX_USED_BIT_READ_Pos  2UL
#define ETH_INT_Q1_STATUS_RX_USED_BIT_READ_Msk  0x4UL
#define ETH_INT_Q1_STATUS_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_Pos 5UL
#define ETH_INT_Q1_STATUS_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_Msk 0x20UL
#define ETH_INT_Q1_STATUS_AMBA_ERROR_Pos        6UL
#define ETH_INT_Q1_STATUS_AMBA_ERROR_Msk        0x40UL
#define ETH_INT_Q1_STATUS_TRANSMIT_COMPLETE_Pos 7UL
#define ETH_INT_Q1_STATUS_TRANSMIT_COMPLETE_Msk 0x80UL
#define ETH_INT_Q1_STATUS_RESP_NOT_OK_Pos       11UL
#define ETH_INT_Q1_STATUS_RESP_NOT_OK_Msk       0x800UL
/* ETH.INT_Q2_STATUS */
#define ETH_INT_Q2_STATUS_RECEIVE_COMPLETE_Pos  1UL
#define ETH_INT_Q2_STATUS_RECEIVE_COMPLETE_Msk  0x2UL
#define ETH_INT_Q2_STATUS_RX_USED_BIT_READ_Pos  2UL
#define ETH_INT_Q2_STATUS_RX_USED_BIT_READ_Msk  0x4UL
#define ETH_INT_Q2_STATUS_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_Pos 5UL
#define ETH_INT_Q2_STATUS_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_Msk 0x20UL
#define ETH_INT_Q2_STATUS_AMBA_ERROR_Pos        6UL
#define ETH_INT_Q2_STATUS_AMBA_ERROR_Msk        0x40UL
#define ETH_INT_Q2_STATUS_TRANSMIT_COMPLETE_Pos 7UL
#define ETH_INT_Q2_STATUS_TRANSMIT_COMPLETE_Msk 0x80UL
#define ETH_INT_Q2_STATUS_RESP_NOT_OK_Pos       11UL
#define ETH_INT_Q2_STATUS_RESP_NOT_OK_Msk       0x800UL
/* ETH.INT_Q3_STATUS */
#define ETH_INT_Q3_STATUS_REMOVED_31_0_Pos      0UL
#define ETH_INT_Q3_STATUS_REMOVED_31_0_Msk      0xFFFFFFFFUL
/* ETH.INT_Q15_STATUS */
#define ETH_INT_Q15_STATUS_REMOVED_31_0_Pos     0UL
#define ETH_INT_Q15_STATUS_REMOVED_31_0_Msk     0xFFFFFFFFUL
/* ETH.TRANSMIT_Q1_PTR */
#define ETH_TRANSMIT_Q1_PTR_DMA_TX_DIS_Q_Pos    0UL
#define ETH_TRANSMIT_Q1_PTR_DMA_TX_DIS_Q_Msk    0x1UL
#define ETH_TRANSMIT_Q1_PTR_DMA_TX_Q_PTR_Pos    2UL
#define ETH_TRANSMIT_Q1_PTR_DMA_TX_Q_PTR_Msk    0xFFFFFFFCUL
/* ETH.TRANSMIT_Q2_PTR */
#define ETH_TRANSMIT_Q2_PTR_DMA_TX_DIS_Q_Pos    0UL
#define ETH_TRANSMIT_Q2_PTR_DMA_TX_DIS_Q_Msk    0x1UL
#define ETH_TRANSMIT_Q2_PTR_DMA_TX_Q_PTR_Pos    2UL
#define ETH_TRANSMIT_Q2_PTR_DMA_TX_Q_PTR_Msk    0xFFFFFFFCUL
/* ETH.TRANSMIT_Q3_PTR */
#define ETH_TRANSMIT_Q3_PTR_REMOVED_31_0_Pos    0UL
#define ETH_TRANSMIT_Q3_PTR_REMOVED_31_0_Msk    0xFFFFFFFFUL
/* ETH.TRANSMIT_Q15_PTR */
#define ETH_TRANSMIT_Q15_PTR_REMOVED_31_0_Pos   0UL
#define ETH_TRANSMIT_Q15_PTR_REMOVED_31_0_Msk   0xFFFFFFFFUL
/* ETH.RECEIVE_Q1_PTR */
#define ETH_RECEIVE_Q1_PTR_DMA_RX_DIS_Q_Pos     0UL
#define ETH_RECEIVE_Q1_PTR_DMA_RX_DIS_Q_Msk     0x1UL
#define ETH_RECEIVE_Q1_PTR_DMA_RX_Q_PTR_Pos     2UL
#define ETH_RECEIVE_Q1_PTR_DMA_RX_Q_PTR_Msk     0xFFFFFFFCUL
/* ETH.RECEIVE_Q2_PTR */
#define ETH_RECEIVE_Q2_PTR_DMA_RX_DIS_Q_Pos     0UL
#define ETH_RECEIVE_Q2_PTR_DMA_RX_DIS_Q_Msk     0x1UL
#define ETH_RECEIVE_Q2_PTR_DMA_RX_Q_PTR_Pos     2UL
#define ETH_RECEIVE_Q2_PTR_DMA_RX_Q_PTR_Msk     0xFFFFFFFCUL
/* ETH.RECEIVE_Q3_PTR */
#define ETH_RECEIVE_Q3_PTR_REMOVED_31_0_Pos     0UL
#define ETH_RECEIVE_Q3_PTR_REMOVED_31_0_Msk     0xFFFFFFFFUL
/* ETH.RECEIVE_Q7_PTR */
#define ETH_RECEIVE_Q7_PTR_REMOVED_31_0_Pos     0UL
#define ETH_RECEIVE_Q7_PTR_REMOVED_31_0_Msk     0xFFFFFFFFUL
/* ETH.DMA_RXBUF_SIZE_Q1 */
#define ETH_DMA_RXBUF_SIZE_Q1_DMA_RX_Q_BUF_SIZE_Pos 0UL
#define ETH_DMA_RXBUF_SIZE_Q1_DMA_RX_Q_BUF_SIZE_Msk 0xFFUL
/* ETH.DMA_RXBUF_SIZE_Q2 */
#define ETH_DMA_RXBUF_SIZE_Q2_DMA_RX_Q_BUF_SIZE_Pos 0UL
#define ETH_DMA_RXBUF_SIZE_Q2_DMA_RX_Q_BUF_SIZE_Msk 0xFFUL
/* ETH.DMA_RXBUF_SIZE_Q3 */
#define ETH_DMA_RXBUF_SIZE_Q3_REMOVED_31_0_Pos  0UL
#define ETH_DMA_RXBUF_SIZE_Q3_REMOVED_31_0_Msk  0xFFFFFFFFUL
/* ETH.DMA_RXBUF_SIZE_Q7 */
#define ETH_DMA_RXBUF_SIZE_Q7_REMOVED_31_0_Pos  0UL
#define ETH_DMA_RXBUF_SIZE_Q7_REMOVED_31_0_Msk  0xFFFFFFFFUL
/* ETH.CBS_CONTROL */
#define ETH_CBS_CONTROL_CBS_ENABLE_QUEUE_A_Pos  0UL
#define ETH_CBS_CONTROL_CBS_ENABLE_QUEUE_A_Msk  0x1UL
#define ETH_CBS_CONTROL_CBS_ENABLE_QUEUE_B_Pos  1UL
#define ETH_CBS_CONTROL_CBS_ENABLE_QUEUE_B_Msk  0x2UL
/* ETH.CBS_IDLESLOPE_Q_A */
#define ETH_CBS_IDLESLOPE_Q_A_IDLESLOPE_A_Pos   0UL
#define ETH_CBS_IDLESLOPE_Q_A_IDLESLOPE_A_Msk   0xFFFFFFFFUL
/* ETH.CBS_IDLESLOPE_Q_B */
#define ETH_CBS_IDLESLOPE_Q_B_IDLESLOPE_B_Pos   0UL
#define ETH_CBS_IDLESLOPE_Q_B_IDLESLOPE_B_Msk   0xFFFFFFFFUL
/* ETH.UPPER_TX_Q_BASE_ADDR */
#define ETH_UPPER_TX_Q_BASE_ADDR_UPPER_TX_Q_BASE_ADDR_Pos 0UL
#define ETH_UPPER_TX_Q_BASE_ADDR_UPPER_TX_Q_BASE_ADDR_Msk 0xFFFFFFFFUL
/* ETH.TX_BD_CONTROL */
#define ETH_TX_BD_CONTROL_TX_BD_TS_MODE_Pos     4UL
#define ETH_TX_BD_CONTROL_TX_BD_TS_MODE_Msk     0x30UL
/* ETH.RX_BD_CONTROL */
#define ETH_RX_BD_CONTROL_RX_BD_TS_MODE_Pos     4UL
#define ETH_RX_BD_CONTROL_RX_BD_TS_MODE_Msk     0x30UL
/* ETH.UPPER_RX_Q_BASE_ADDR */
#define ETH_UPPER_RX_Q_BASE_ADDR_UPPER_RX_Q_BASE_ADDR_Pos 0UL
#define ETH_UPPER_RX_Q_BASE_ADDR_UPPER_RX_Q_BASE_ADDR_Msk 0xFFFFFFFFUL
/* ETH.HIDDEN_REG0 */
#define ETH_HIDDEN_REG0_HIDDEN0_FIELD_Pos       0UL
#define ETH_HIDDEN_REG0_HIDDEN0_FIELD_Msk       0xFFFFFFFFUL
/* ETH.HIDDEN_REG1 */
#define ETH_HIDDEN_REG1_HIDDEN1_FIELD_Pos       0UL
#define ETH_HIDDEN_REG1_HIDDEN1_FIELD_Msk       0xFFFFFFFFUL
/* ETH.HIDDEN_REG2 */
#define ETH_HIDDEN_REG2_HIDDEN2_FIELD_Pos       0UL
#define ETH_HIDDEN_REG2_HIDDEN2_FIELD_Msk       0xFFFFFFFFUL
/* ETH.HIDDEN_REG3 */
#define ETH_HIDDEN_REG3_HIDDEN3_FIELD_Pos       0UL
#define ETH_HIDDEN_REG3_HIDDEN3_FIELD_Msk       0xFUL
/* ETH.HIDDEN_REG4 */
#define ETH_HIDDEN_REG4_HIDDEN4_FIELD_L_Pos     0UL
#define ETH_HIDDEN_REG4_HIDDEN4_FIELD_L_Msk     0x1FFUL
#define ETH_HIDDEN_REG4_HIDDEN4_FIELD_H_Pos     16UL
#define ETH_HIDDEN_REG4_HIDDEN4_FIELD_H_Msk     0x1FF0000UL
/* ETH.HIDDEN_REG5 */
#define ETH_HIDDEN_REG5_HIDDEN5_FIELD_L_Pos     0UL
#define ETH_HIDDEN_REG5_HIDDEN5_FIELD_L_Msk     0x1FFUL
#define ETH_HIDDEN_REG5_HIDDEN5_FIELD_H_Pos     16UL
#define ETH_HIDDEN_REG5_HIDDEN5_FIELD_H_Msk     0x1FF0000UL
/* ETH.SCREENING_TYPE_1_REGISTER_0 */
#define ETH_SCREENING_TYPE_1_REGISTER_0_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_0_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_0_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_0_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_0_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_0_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_0_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_0_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_0_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_0_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_1 */
#define ETH_SCREENING_TYPE_1_REGISTER_1_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_1_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_1_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_1_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_1_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_1_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_1_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_1_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_1_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_1_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_2 */
#define ETH_SCREENING_TYPE_1_REGISTER_2_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_2_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_2_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_2_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_2_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_2_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_2_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_2_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_2_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_2_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_3 */
#define ETH_SCREENING_TYPE_1_REGISTER_3_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_3_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_3_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_3_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_3_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_3_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_3_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_3_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_3_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_3_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_4 */
#define ETH_SCREENING_TYPE_1_REGISTER_4_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_4_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_4_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_4_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_4_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_4_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_4_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_4_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_4_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_4_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_5 */
#define ETH_SCREENING_TYPE_1_REGISTER_5_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_5_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_5_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_5_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_5_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_5_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_5_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_5_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_5_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_5_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_6 */
#define ETH_SCREENING_TYPE_1_REGISTER_6_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_6_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_6_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_6_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_6_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_6_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_6_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_6_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_6_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_6_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_7 */
#define ETH_SCREENING_TYPE_1_REGISTER_7_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_7_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_7_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_7_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_7_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_7_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_7_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_7_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_7_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_7_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_8 */
#define ETH_SCREENING_TYPE_1_REGISTER_8_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_8_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_8_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_8_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_8_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_8_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_8_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_8_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_8_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_8_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_9 */
#define ETH_SCREENING_TYPE_1_REGISTER_9_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_9_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_9_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_9_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_9_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_9_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_9_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_9_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_9_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_9_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_10 */
#define ETH_SCREENING_TYPE_1_REGISTER_10_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_10_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_10_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_10_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_10_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_10_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_10_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_10_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_10_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_10_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_11 */
#define ETH_SCREENING_TYPE_1_REGISTER_11_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_11_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_11_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_11_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_11_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_11_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_11_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_11_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_11_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_11_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_12 */
#define ETH_SCREENING_TYPE_1_REGISTER_12_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_12_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_12_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_12_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_12_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_12_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_12_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_12_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_12_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_12_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_13 */
#define ETH_SCREENING_TYPE_1_REGISTER_13_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_13_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_13_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_13_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_13_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_13_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_13_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_13_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_13_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_13_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_14 */
#define ETH_SCREENING_TYPE_1_REGISTER_14_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_14_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_14_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_14_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_14_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_14_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_14_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_14_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_14_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_14_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_1_REGISTER_15 */
#define ETH_SCREENING_TYPE_1_REGISTER_15_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_1_REGISTER_15_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_1_REGISTER_15_DSTC_MATCH_Pos 4UL
#define ETH_SCREENING_TYPE_1_REGISTER_15_DSTC_MATCH_Msk 0xFF0UL
#define ETH_SCREENING_TYPE_1_REGISTER_15_UDP_PORT_MATCH_Pos 12UL
#define ETH_SCREENING_TYPE_1_REGISTER_15_UDP_PORT_MATCH_Msk 0xFFFF000UL
#define ETH_SCREENING_TYPE_1_REGISTER_15_DSTC_ENABLE_Pos 28UL
#define ETH_SCREENING_TYPE_1_REGISTER_15_DSTC_ENABLE_Msk 0x10000000UL
#define ETH_SCREENING_TYPE_1_REGISTER_15_UDP_PORT_MATCH_ENABLE_Pos 29UL
#define ETH_SCREENING_TYPE_1_REGISTER_15_UDP_PORT_MATCH_ENABLE_Msk 0x20000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_0 */
#define ETH_SCREENING_TYPE_2_REGISTER_0_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_0_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_0_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_1 */
#define ETH_SCREENING_TYPE_2_REGISTER_1_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_1_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_1_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_2 */
#define ETH_SCREENING_TYPE_2_REGISTER_2_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_2_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_2_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_3 */
#define ETH_SCREENING_TYPE_2_REGISTER_3_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_3_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_3_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_4 */
#define ETH_SCREENING_TYPE_2_REGISTER_4_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_4_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_4_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_5 */
#define ETH_SCREENING_TYPE_2_REGISTER_5_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_5_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_5_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_6 */
#define ETH_SCREENING_TYPE_2_REGISTER_6_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_6_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_6_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_7 */
#define ETH_SCREENING_TYPE_2_REGISTER_7_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_7_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_7_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_8 */
#define ETH_SCREENING_TYPE_2_REGISTER_8_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_8_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_8_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_9 */
#define ETH_SCREENING_TYPE_2_REGISTER_9_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_9_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_9_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_10 */
#define ETH_SCREENING_TYPE_2_REGISTER_10_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_10_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_10_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_11 */
#define ETH_SCREENING_TYPE_2_REGISTER_11_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_11_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_11_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_12 */
#define ETH_SCREENING_TYPE_2_REGISTER_12_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_12_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_12_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_13 */
#define ETH_SCREENING_TYPE_2_REGISTER_13_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_13_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_13_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_14 */
#define ETH_SCREENING_TYPE_2_REGISTER_14_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_14_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_14_RESERVED_31_Msk 0x80000000UL
/* ETH.SCREENING_TYPE_2_REGISTER_15 */
#define ETH_SCREENING_TYPE_2_REGISTER_15_QUEUE_NUMBER_Pos 0UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_QUEUE_NUMBER_Msk 0xFUL
#define ETH_SCREENING_TYPE_2_REGISTER_15_VLAN_PRIORITY_Pos 4UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_VLAN_PRIORITY_Msk 0x70UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_RESERVED_7_Pos 7UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_RESERVED_7_Msk 0x80UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_VLAN_ENABLE_Pos 8UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_VLAN_ENABLE_Msk 0x100UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_INDEX_Pos 9UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_INDEX_Msk 0xE00UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_ETHERTYPE_ENABLE_Pos 12UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_ETHERTYPE_ENABLE_Msk 0x1000UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_COMPARE_A_Pos 13UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_COMPARE_A_Msk 0x3E000UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_COMPARE_A_ENABLE_Pos 18UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_COMPARE_A_ENABLE_Msk 0x40000UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_COMPARE_B_Pos 19UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_COMPARE_B_Msk 0xF80000UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_COMPARE_B_ENABLE_Pos 24UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_COMPARE_B_ENABLE_Msk 0x1000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_COMPARE_C_Pos 25UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_COMPARE_C_Msk 0x3E000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_COMPARE_C_ENABLE_Pos 30UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_COMPARE_C_ENABLE_Msk 0x40000000UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_RESERVED_31_Pos 31UL
#define ETH_SCREENING_TYPE_2_REGISTER_15_RESERVED_31_Msk 0x80000000UL
/* ETH.TX_SCHED_CTRL */
#define ETH_TX_SCHED_CTRL_TX_SCHED_Q0_Pos       0UL
#define ETH_TX_SCHED_CTRL_TX_SCHED_Q0_Msk       0x3UL
#define ETH_TX_SCHED_CTRL_TX_SCHED_Q1_Pos       2UL
#define ETH_TX_SCHED_CTRL_TX_SCHED_Q1_Msk       0xCUL
#define ETH_TX_SCHED_CTRL_TX_SCHED_Q2_Pos       4UL
#define ETH_TX_SCHED_CTRL_TX_SCHED_Q2_Msk       0x30UL
#define ETH_TX_SCHED_CTRL_TX_SCHED_Q3_Pos       6UL
#define ETH_TX_SCHED_CTRL_TX_SCHED_Q3_Msk       0xC0UL
#define ETH_TX_SCHED_CTRL_REMOVED_31_8_Pos      8UL
#define ETH_TX_SCHED_CTRL_REMOVED_31_8_Msk      0xFFFFFF00UL
/* ETH.BW_RATE_LIMIT_Q0TO3 */
#define ETH_BW_RATE_LIMIT_Q0TO3_DWRR_ETS_WEIGHT_Q0_Pos 0UL
#define ETH_BW_RATE_LIMIT_Q0TO3_DWRR_ETS_WEIGHT_Q0_Msk 0xFFUL
#define ETH_BW_RATE_LIMIT_Q0TO3_DWRR_ETS_WEIGHT_Q1_Pos 8UL
#define ETH_BW_RATE_LIMIT_Q0TO3_DWRR_ETS_WEIGHT_Q1_Msk 0xFF00UL
#define ETH_BW_RATE_LIMIT_Q0TO3_DWRR_ETS_WEIGHT_Q2_Pos 16UL
#define ETH_BW_RATE_LIMIT_Q0TO3_DWRR_ETS_WEIGHT_Q2_Msk 0xFF0000UL
#define ETH_BW_RATE_LIMIT_Q0TO3_DWRR_ETS_WEIGHT_Q3_Pos 24UL
#define ETH_BW_RATE_LIMIT_Q0TO3_DWRR_ETS_WEIGHT_Q3_Msk 0xFF000000UL
/* ETH.BW_RATE_LIMIT_Q4TO7 */
#define ETH_BW_RATE_LIMIT_Q4TO7_REMOVED_31_0_Pos 0UL
#define ETH_BW_RATE_LIMIT_Q4TO7_REMOVED_31_0_Msk 0xFFFFFFFFUL
/* ETH.BW_RATE_LIMIT_Q8TO11 */
#define ETH_BW_RATE_LIMIT_Q8TO11_REMOVED_31_0_Pos 0UL
#define ETH_BW_RATE_LIMIT_Q8TO11_REMOVED_31_0_Msk 0xFFFFFFFFUL
/* ETH.BW_RATE_LIMIT_Q12TO15 */
#define ETH_BW_RATE_LIMIT_Q12TO15_REMOVED_31_0_Pos 0UL
#define ETH_BW_RATE_LIMIT_Q12TO15_REMOVED_31_0_Msk 0xFFFFFFFFUL
/* ETH.TX_Q_SEG_ALLOC_Q0TO7 */
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_SEGMENT_ALLOC_Q0_Pos 0UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_SEGMENT_ALLOC_Q0_Msk 0x7UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_3_3_Pos 3UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_3_3_Msk 0x8UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_SEGMENT_ALLOC_Q1_Pos 4UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_SEGMENT_ALLOC_Q1_Msk 0x70UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_7_7_Pos 7UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_7_7_Msk 0x80UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_SEGMENT_ALLOC_Q2_Pos 8UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_SEGMENT_ALLOC_Q2_Msk 0x700UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_11_11_Pos 11UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_11_11_Msk 0x800UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_REMOVED_14_12_Pos 12UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_REMOVED_14_12_Msk 0x7000UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_15_15_Pos 15UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_15_15_Msk 0x8000UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_REMOVED_18_16_Pos 16UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_REMOVED_18_16_Msk 0x70000UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_19_19_Pos 19UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_19_19_Msk 0x80000UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_REMOVED_22_20_Pos 20UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_REMOVED_22_20_Msk 0x700000UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_23_Pos 23UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_23_Msk 0x800000UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_REMOVED_26_24_Pos 24UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_REMOVED_26_24_Msk 0x7000000UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_27_27_Pos 27UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_27_27_Msk 0x8000000UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_REMOVED_30_28_Pos 28UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_REMOVED_30_28_Msk 0x70000000UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_31_31_Pos 31UL
#define ETH_TX_Q_SEG_ALLOC_Q0TO7_RESERVED_31_31_Msk 0x80000000UL
/* ETH.TX_Q_SEG_ALLOC_Q8TO15 */
#define ETH_TX_Q_SEG_ALLOC_Q8TO15_REMOVED_31_0_Pos 0UL
#define ETH_TX_Q_SEG_ALLOC_Q8TO15_REMOVED_31_0_Msk 0xFFFFFFFFUL
/* ETH.RECEIVE_Q8_PTR */
#define ETH_RECEIVE_Q8_PTR_REMOVED_31_0_Pos     0UL
#define ETH_RECEIVE_Q8_PTR_REMOVED_31_0_Msk     0xFFFFFFFFUL
/* ETH.RECEIVE_Q15_PTR */
#define ETH_RECEIVE_Q15_PTR_REMOVED_31_0_Pos    0UL
#define ETH_RECEIVE_Q15_PTR_REMOVED_31_0_Msk    0xFFFFFFFFUL
/* ETH.DMA_RXBUF_SIZE_Q8 */
#define ETH_DMA_RXBUF_SIZE_Q8_REMOVED_31_0_Pos  0UL
#define ETH_DMA_RXBUF_SIZE_Q8_REMOVED_31_0_Msk  0xFFFFFFFFUL
/* ETH.DMA_RXBUF_SIZE_Q15 */
#define ETH_DMA_RXBUF_SIZE_Q15_REMOVED_31_0_Pos 0UL
#define ETH_DMA_RXBUF_SIZE_Q15_REMOVED_31_0_Msk 0xFFFFFFFFUL
/* ETH.INT_Q1_ENABLE */
#define ETH_INT_Q1_ENABLE_ENABLE_RECEIVE_COMPLETE_INTERRUPT_Pos 1UL
#define ETH_INT_Q1_ENABLE_ENABLE_RECEIVE_COMPLETE_INTERRUPT_Msk 0x2UL
#define ETH_INT_Q1_ENABLE_ENABLE_RX_USED_BIT_READ_INTERRUPT_Pos 2UL
#define ETH_INT_Q1_ENABLE_ENABLE_RX_USED_BIT_READ_INTERRUPT_Msk 0x4UL
#define ETH_INT_Q1_ENABLE_ENABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_Pos 5UL
#define ETH_INT_Q1_ENABLE_ENABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_Msk 0x20UL
#define ETH_INT_Q1_ENABLE_ENABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT_Pos 6UL
#define ETH_INT_Q1_ENABLE_ENABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT_Msk 0x40UL
#define ETH_INT_Q1_ENABLE_ENABLE_TRANSMIT_COMPLETE_INTERRUPT_Pos 7UL
#define ETH_INT_Q1_ENABLE_ENABLE_TRANSMIT_COMPLETE_INTERRUPT_Msk 0x80UL
#define ETH_INT_Q1_ENABLE_ENABLE_RESP_NOT_OK_INTERRUPT_Pos 11UL
#define ETH_INT_Q1_ENABLE_ENABLE_RESP_NOT_OK_INTERRUPT_Msk 0x800UL
/* ETH.INT_Q2_ENABLE */
#define ETH_INT_Q2_ENABLE_ENABLE_RECEIVE_COMPLETE_INTERRUPT_Pos 1UL
#define ETH_INT_Q2_ENABLE_ENABLE_RECEIVE_COMPLETE_INTERRUPT_Msk 0x2UL
#define ETH_INT_Q2_ENABLE_ENABLE_RX_USED_BIT_READ_INTERRUPT_Pos 2UL
#define ETH_INT_Q2_ENABLE_ENABLE_RX_USED_BIT_READ_INTERRUPT_Msk 0x4UL
#define ETH_INT_Q2_ENABLE_ENABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_Pos 5UL
#define ETH_INT_Q2_ENABLE_ENABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_Msk 0x20UL
#define ETH_INT_Q2_ENABLE_ENABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT_Pos 6UL
#define ETH_INT_Q2_ENABLE_ENABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT_Msk 0x40UL
#define ETH_INT_Q2_ENABLE_ENABLE_TRANSMIT_COMPLETE_INTERRUPT_Pos 7UL
#define ETH_INT_Q2_ENABLE_ENABLE_TRANSMIT_COMPLETE_INTERRUPT_Msk 0x80UL
#define ETH_INT_Q2_ENABLE_ENABLE_RESP_NOT_OK_INTERRUPT_Pos 11UL
#define ETH_INT_Q2_ENABLE_ENABLE_RESP_NOT_OK_INTERRUPT_Msk 0x800UL
/* ETH.INT_Q3_ENABLE */
#define ETH_INT_Q3_ENABLE_REMOVED_31_0_Pos      0UL
#define ETH_INT_Q3_ENABLE_REMOVED_31_0_Msk      0xFFFFFFFFUL
/* ETH.INT_Q7_ENABLE */
#define ETH_INT_Q7_ENABLE_REMOVED_31_0_Pos      0UL
#define ETH_INT_Q7_ENABLE_REMOVED_31_0_Msk      0xFFFFFFFFUL
/* ETH.INT_Q1_DISABLE */
#define ETH_INT_Q1_DISABLE_DISABLE_RECEIVE_COMPLETE_INTERRUPT_Pos 1UL
#define ETH_INT_Q1_DISABLE_DISABLE_RECEIVE_COMPLETE_INTERRUPT_Msk 0x2UL
#define ETH_INT_Q1_DISABLE_DISABLE_RX_USED_BIT_READ_INTERRUPT_Pos 2UL
#define ETH_INT_Q1_DISABLE_DISABLE_RX_USED_BIT_READ_INTERRUPT_Msk 0x4UL
#define ETH_INT_Q1_DISABLE_DISABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_Pos 5UL
#define ETH_INT_Q1_DISABLE_DISABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_Msk 0x20UL
#define ETH_INT_Q1_DISABLE_DISABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT_Pos 6UL
#define ETH_INT_Q1_DISABLE_DISABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT_Msk 0x40UL
#define ETH_INT_Q1_DISABLE_DISABLE_TRANSMIT_COMPLETE_INTERRUPT_Pos 7UL
#define ETH_INT_Q1_DISABLE_DISABLE_TRANSMIT_COMPLETE_INTERRUPT_Msk 0x80UL
#define ETH_INT_Q1_DISABLE_DISABLE_RESP_NOT_OK_INTERRUPT_Pos 11UL
#define ETH_INT_Q1_DISABLE_DISABLE_RESP_NOT_OK_INTERRUPT_Msk 0x800UL
/* ETH.INT_Q2_DISABLE */
#define ETH_INT_Q2_DISABLE_DISABLE_RECEIVE_COMPLETE_INTERRUPT_Pos 1UL
#define ETH_INT_Q2_DISABLE_DISABLE_RECEIVE_COMPLETE_INTERRUPT_Msk 0x2UL
#define ETH_INT_Q2_DISABLE_DISABLE_RX_USED_BIT_READ_INTERRUPT_Pos 2UL
#define ETH_INT_Q2_DISABLE_DISABLE_RX_USED_BIT_READ_INTERRUPT_Msk 0x4UL
#define ETH_INT_Q2_DISABLE_DISABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_Pos 5UL
#define ETH_INT_Q2_DISABLE_DISABLE_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_Msk 0x20UL
#define ETH_INT_Q2_DISABLE_DISABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT_Pos 6UL
#define ETH_INT_Q2_DISABLE_DISABLE_TRANSMIT_FRAME_CORRUPTION_DUE_TO_AMBA_ERROR_INTERRUPT_Msk 0x40UL
#define ETH_INT_Q2_DISABLE_DISABLE_TRANSMIT_COMPLETE_INTERRUPT_Pos 7UL
#define ETH_INT_Q2_DISABLE_DISABLE_TRANSMIT_COMPLETE_INTERRUPT_Msk 0x80UL
#define ETH_INT_Q2_DISABLE_DISABLE_RESP_NOT_OK_INTERRUPT_Pos 11UL
#define ETH_INT_Q2_DISABLE_DISABLE_RESP_NOT_OK_INTERRUPT_Msk 0x800UL
/* ETH.INT_Q3_DISABLE */
#define ETH_INT_Q3_DISABLE_REMOVED_31_0_Pos     0UL
#define ETH_INT_Q3_DISABLE_REMOVED_31_0_Msk     0xFFFFFFFFUL
/* ETH.INT_Q7_DISABLE */
#define ETH_INT_Q7_DISABLE_REMOVED_31_0_Pos     0UL
#define ETH_INT_Q7_DISABLE_REMOVED_31_0_Msk     0xFFFFFFFFUL
/* ETH.INT_Q1_MASK */
#define ETH_INT_Q1_MASK_RECEIVE_COMPLETE_INTERRUPT_MASK_Pos 1UL
#define ETH_INT_Q1_MASK_RECEIVE_COMPLETE_INTERRUPT_MASK_Msk 0x2UL
#define ETH_INT_Q1_MASK_RX_USED_INTERRUPT_MASK_Pos 2UL
#define ETH_INT_Q1_MASK_RX_USED_INTERRUPT_MASK_Msk 0x4UL
#define ETH_INT_Q1_MASK_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_MASK_Pos 5UL
#define ETH_INT_Q1_MASK_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_MASK_Msk 0x20UL
#define ETH_INT_Q1_MASK_AMBA_ERROR_INTERRUPT_MASK_Pos 6UL
#define ETH_INT_Q1_MASK_AMBA_ERROR_INTERRUPT_MASK_Msk 0x40UL
#define ETH_INT_Q1_MASK_TRANSMIT_COMPLETE_INTERRUPT_MASK_Pos 7UL
#define ETH_INT_Q1_MASK_TRANSMIT_COMPLETE_INTERRUPT_MASK_Msk 0x80UL
#define ETH_INT_Q1_MASK_RESP_NOT_OK_INTERRUPT_MASK_Pos 11UL
#define ETH_INT_Q1_MASK_RESP_NOT_OK_INTERRUPT_MASK_Msk 0x800UL
/* ETH.INT_Q2_MASK */
#define ETH_INT_Q2_MASK_RECEIVE_COMPLETE_INTERRUPT_MASK_Pos 1UL
#define ETH_INT_Q2_MASK_RECEIVE_COMPLETE_INTERRUPT_MASK_Msk 0x2UL
#define ETH_INT_Q2_MASK_RX_USED_INTERRUPT_MASK_Pos 2UL
#define ETH_INT_Q2_MASK_RX_USED_INTERRUPT_MASK_Msk 0x4UL
#define ETH_INT_Q2_MASK_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_MASK_Pos 5UL
#define ETH_INT_Q2_MASK_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION_INTERRUPT_MASK_Msk 0x20UL
#define ETH_INT_Q2_MASK_AMBA_ERROR_INTERRUPT_MASK_Pos 6UL
#define ETH_INT_Q2_MASK_AMBA_ERROR_INTERRUPT_MASK_Msk 0x40UL
#define ETH_INT_Q2_MASK_TRANSMIT_COMPLETE_INTERRUPT_MASK_Pos 7UL
#define ETH_INT_Q2_MASK_TRANSMIT_COMPLETE_INTERRUPT_MASK_Msk 0x80UL
#define ETH_INT_Q2_MASK_RESP_NOT_OK_INTERRUPT_MASK_Pos 11UL
#define ETH_INT_Q2_MASK_RESP_NOT_OK_INTERRUPT_MASK_Msk 0x800UL
/* ETH.INT_Q3_MASK */
#define ETH_INT_Q3_MASK_REMOVED_31_0_Pos        0UL
#define ETH_INT_Q3_MASK_REMOVED_31_0_Msk        0xFFFFFFFFUL
/* ETH.INT_Q7_MASK */
#define ETH_INT_Q7_MASK_REMOVED_31_0_Pos        0UL
#define ETH_INT_Q7_MASK_REMOVED_31_0_Msk        0xFFFFFFFFUL
/* ETH.INT_Q8_ENABLE */
#define ETH_INT_Q8_ENABLE_REMOVED_31_0_Pos      0UL
#define ETH_INT_Q8_ENABLE_REMOVED_31_0_Msk      0xFFFFFFFFUL
/* ETH.INT_Q15_ENABLE */
#define ETH_INT_Q15_ENABLE_REMOVED_31_0_Pos     0UL
#define ETH_INT_Q15_ENABLE_REMOVED_31_0_Msk     0xFFFFFFFFUL
/* ETH.INT_Q8_DISABLE */
#define ETH_INT_Q8_DISABLE_REMOVED_31_0_Pos     0UL
#define ETH_INT_Q8_DISABLE_REMOVED_31_0_Msk     0xFFFFFFFFUL
/* ETH.INT_Q15_DISABLE */
#define ETH_INT_Q15_DISABLE_REMOVED_31_0_Pos    0UL
#define ETH_INT_Q15_DISABLE_REMOVED_31_0_Msk    0xFFFFFFFFUL
/* ETH.INT_Q8_MASK */
#define ETH_INT_Q8_MASK_REMOVED_31_0_Pos        0UL
#define ETH_INT_Q8_MASK_REMOVED_31_0_Msk        0xFFFFFFFFUL
/* ETH.INT_Q15_MASK */
#define ETH_INT_Q15_MASK_REMOVED_31_0_Pos       0UL
#define ETH_INT_Q15_MASK_REMOVED_31_0_Msk       0xFFFFFFFFUL
/* ETH.SCREENING_TYPE_2_ETHERTYPE_REG_0 */
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_0_COMPARE_VALUE_Pos 0UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_0_COMPARE_VALUE_Msk 0xFFFFUL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_0_RESERVED_31_16_Pos 16UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_0_RESERVED_31_16_Msk 0xFFFF0000UL
/* ETH.SCREENING_TYPE_2_ETHERTYPE_REG_1 */
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_1_COMPARE_VALUE_Pos 0UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_1_COMPARE_VALUE_Msk 0xFFFFUL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_1_RESERVED_31_16_Pos 16UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_1_RESERVED_31_16_Msk 0xFFFF0000UL
/* ETH.SCREENING_TYPE_2_ETHERTYPE_REG_2 */
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_2_COMPARE_VALUE_Pos 0UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_2_COMPARE_VALUE_Msk 0xFFFFUL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_2_RESERVED_31_16_Pos 16UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_2_RESERVED_31_16_Msk 0xFFFF0000UL
/* ETH.SCREENING_TYPE_2_ETHERTYPE_REG_3 */
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_3_COMPARE_VALUE_Pos 0UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_3_COMPARE_VALUE_Msk 0xFFFFUL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_3_RESERVED_31_16_Pos 16UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_3_RESERVED_31_16_Msk 0xFFFF0000UL
/* ETH.SCREENING_TYPE_2_ETHERTYPE_REG_4 */
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_4_COMPARE_VALUE_Pos 0UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_4_COMPARE_VALUE_Msk 0xFFFFUL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_4_RESERVED_31_16_Pos 16UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_4_RESERVED_31_16_Msk 0xFFFF0000UL
/* ETH.SCREENING_TYPE_2_ETHERTYPE_REG_5 */
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_5_COMPARE_VALUE_Pos 0UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_5_COMPARE_VALUE_Msk 0xFFFFUL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_5_RESERVED_31_16_Pos 16UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_5_RESERVED_31_16_Msk 0xFFFF0000UL
/* ETH.SCREENING_TYPE_2_ETHERTYPE_REG_6 */
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_6_COMPARE_VALUE_Pos 0UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_6_COMPARE_VALUE_Msk 0xFFFFUL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_6_RESERVED_31_16_Pos 16UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_6_RESERVED_31_16_Msk 0xFFFF0000UL
/* ETH.SCREENING_TYPE_2_ETHERTYPE_REG_7 */
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_7_COMPARE_VALUE_Pos 0UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_7_COMPARE_VALUE_Msk 0xFFFFUL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_7_RESERVED_31_16_Pos 16UL
#define ETH_SCREENING_TYPE_2_ETHERTYPE_REG_7_RESERVED_31_16_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_0_WORD_0 */
#define ETH_TYPE2_COMPARE_0_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_0_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_0_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_0_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_0_WORD_1 */
#define ETH_TYPE2_COMPARE_0_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_0_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_0_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_0_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_0_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_0_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_0_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_0_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_1_WORD_0 */
#define ETH_TYPE2_COMPARE_1_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_1_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_1_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_1_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_1_WORD_1 */
#define ETH_TYPE2_COMPARE_1_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_1_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_1_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_1_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_1_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_1_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_1_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_1_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_2_WORD_0 */
#define ETH_TYPE2_COMPARE_2_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_2_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_2_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_2_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_2_WORD_1 */
#define ETH_TYPE2_COMPARE_2_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_2_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_2_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_2_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_2_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_2_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_2_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_2_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_3_WORD_0 */
#define ETH_TYPE2_COMPARE_3_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_3_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_3_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_3_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_3_WORD_1 */
#define ETH_TYPE2_COMPARE_3_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_3_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_3_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_3_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_3_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_3_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_3_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_3_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_4_WORD_0 */
#define ETH_TYPE2_COMPARE_4_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_4_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_4_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_4_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_4_WORD_1 */
#define ETH_TYPE2_COMPARE_4_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_4_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_4_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_4_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_4_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_4_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_4_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_4_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_5_WORD_0 */
#define ETH_TYPE2_COMPARE_5_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_5_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_5_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_5_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_5_WORD_1 */
#define ETH_TYPE2_COMPARE_5_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_5_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_5_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_5_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_5_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_5_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_5_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_5_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_6_WORD_0 */
#define ETH_TYPE2_COMPARE_6_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_6_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_6_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_6_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_6_WORD_1 */
#define ETH_TYPE2_COMPARE_6_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_6_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_6_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_6_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_6_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_6_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_6_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_6_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_7_WORD_0 */
#define ETH_TYPE2_COMPARE_7_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_7_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_7_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_7_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_7_WORD_1 */
#define ETH_TYPE2_COMPARE_7_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_7_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_7_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_7_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_7_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_7_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_7_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_7_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_8_WORD_0 */
#define ETH_TYPE2_COMPARE_8_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_8_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_8_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_8_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_8_WORD_1 */
#define ETH_TYPE2_COMPARE_8_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_8_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_8_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_8_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_8_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_8_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_8_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_8_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_9_WORD_0 */
#define ETH_TYPE2_COMPARE_9_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_9_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_9_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_9_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_9_WORD_1 */
#define ETH_TYPE2_COMPARE_9_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_9_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_9_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_9_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_9_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_9_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_9_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_9_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_10_WORD_0 */
#define ETH_TYPE2_COMPARE_10_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_10_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_10_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_10_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_10_WORD_1 */
#define ETH_TYPE2_COMPARE_10_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_10_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_10_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_10_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_10_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_10_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_10_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_10_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_11_WORD_0 */
#define ETH_TYPE2_COMPARE_11_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_11_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_11_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_11_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_11_WORD_1 */
#define ETH_TYPE2_COMPARE_11_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_11_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_11_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_11_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_11_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_11_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_11_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_11_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_12_WORD_0 */
#define ETH_TYPE2_COMPARE_12_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_12_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_12_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_12_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_12_WORD_1 */
#define ETH_TYPE2_COMPARE_12_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_12_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_12_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_12_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_12_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_12_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_12_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_12_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_13_WORD_0 */
#define ETH_TYPE2_COMPARE_13_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_13_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_13_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_13_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_13_WORD_1 */
#define ETH_TYPE2_COMPARE_13_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_13_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_13_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_13_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_13_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_13_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_13_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_13_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_14_WORD_0 */
#define ETH_TYPE2_COMPARE_14_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_14_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_14_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_14_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_14_WORD_1 */
#define ETH_TYPE2_COMPARE_14_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_14_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_14_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_14_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_14_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_14_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_14_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_14_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_15_WORD_0 */
#define ETH_TYPE2_COMPARE_15_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_15_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_15_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_15_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_15_WORD_1 */
#define ETH_TYPE2_COMPARE_15_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_15_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_15_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_15_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_15_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_15_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_15_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_15_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_16_WORD_0 */
#define ETH_TYPE2_COMPARE_16_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_16_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_16_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_16_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_16_WORD_1 */
#define ETH_TYPE2_COMPARE_16_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_16_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_16_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_16_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_16_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_16_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_16_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_16_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_17_WORD_0 */
#define ETH_TYPE2_COMPARE_17_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_17_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_17_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_17_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_17_WORD_1 */
#define ETH_TYPE2_COMPARE_17_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_17_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_17_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_17_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_17_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_17_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_17_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_17_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_18_WORD_0 */
#define ETH_TYPE2_COMPARE_18_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_18_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_18_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_18_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_18_WORD_1 */
#define ETH_TYPE2_COMPARE_18_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_18_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_18_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_18_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_18_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_18_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_18_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_18_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_19_WORD_0 */
#define ETH_TYPE2_COMPARE_19_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_19_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_19_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_19_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_19_WORD_1 */
#define ETH_TYPE2_COMPARE_19_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_19_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_19_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_19_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_19_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_19_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_19_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_19_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_20_WORD_0 */
#define ETH_TYPE2_COMPARE_20_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_20_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_20_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_20_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_20_WORD_1 */
#define ETH_TYPE2_COMPARE_20_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_20_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_20_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_20_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_20_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_20_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_20_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_20_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_21_WORD_0 */
#define ETH_TYPE2_COMPARE_21_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_21_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_21_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_21_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_21_WORD_1 */
#define ETH_TYPE2_COMPARE_21_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_21_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_21_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_21_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_21_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_21_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_21_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_21_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_22_WORD_0 */
#define ETH_TYPE2_COMPARE_22_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_22_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_22_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_22_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_22_WORD_1 */
#define ETH_TYPE2_COMPARE_22_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_22_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_22_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_22_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_22_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_22_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_22_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_22_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_23_WORD_0 */
#define ETH_TYPE2_COMPARE_23_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_23_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_23_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_23_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_23_WORD_1 */
#define ETH_TYPE2_COMPARE_23_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_23_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_23_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_23_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_23_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_23_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_23_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_23_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_24_WORD_0 */
#define ETH_TYPE2_COMPARE_24_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_24_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_24_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_24_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_24_WORD_1 */
#define ETH_TYPE2_COMPARE_24_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_24_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_24_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_24_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_24_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_24_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_24_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_24_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_25_WORD_0 */
#define ETH_TYPE2_COMPARE_25_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_25_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_25_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_25_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_25_WORD_1 */
#define ETH_TYPE2_COMPARE_25_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_25_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_25_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_25_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_25_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_25_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_25_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_25_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_26_WORD_0 */
#define ETH_TYPE2_COMPARE_26_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_26_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_26_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_26_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_26_WORD_1 */
#define ETH_TYPE2_COMPARE_26_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_26_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_26_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_26_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_26_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_26_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_26_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_26_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_27_WORD_0 */
#define ETH_TYPE2_COMPARE_27_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_27_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_27_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_27_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_27_WORD_1 */
#define ETH_TYPE2_COMPARE_27_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_27_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_27_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_27_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_27_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_27_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_27_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_27_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_28_WORD_0 */
#define ETH_TYPE2_COMPARE_28_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_28_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_28_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_28_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_28_WORD_1 */
#define ETH_TYPE2_COMPARE_28_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_28_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_28_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_28_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_28_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_28_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_28_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_28_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_29_WORD_0 */
#define ETH_TYPE2_COMPARE_29_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_29_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_29_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_29_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_29_WORD_1 */
#define ETH_TYPE2_COMPARE_29_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_29_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_29_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_29_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_29_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_29_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_29_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_29_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_30_WORD_0 */
#define ETH_TYPE2_COMPARE_30_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_30_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_30_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_30_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_30_WORD_1 */
#define ETH_TYPE2_COMPARE_30_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_30_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_30_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_30_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_30_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_30_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_30_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_30_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL
/* ETH.TYPE2_COMPARE_31_WORD_0 */
#define ETH_TYPE2_COMPARE_31_WORD_0_MASK_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_31_WORD_0_MASK_VALUE_Msk 0xFFFFUL
#define ETH_TYPE2_COMPARE_31_WORD_0_COMPARE_VALUE_TYPE2_Pos 16UL
#define ETH_TYPE2_COMPARE_31_WORD_0_COMPARE_VALUE_TYPE2_Msk 0xFFFF0000UL
/* ETH.TYPE2_COMPARE_31_WORD_1 */
#define ETH_TYPE2_COMPARE_31_WORD_1_OFFSET_VALUE_Pos 0UL
#define ETH_TYPE2_COMPARE_31_WORD_1_OFFSET_VALUE_Msk 0x7FUL
#define ETH_TYPE2_COMPARE_31_WORD_1_COMPARE_OFFSET_Pos 7UL
#define ETH_TYPE2_COMPARE_31_WORD_1_COMPARE_OFFSET_Msk 0x180UL
#define ETH_TYPE2_COMPARE_31_WORD_1_DISABLE_MASK_Pos 9UL
#define ETH_TYPE2_COMPARE_31_WORD_1_DISABLE_MASK_Msk 0x200UL
#define ETH_TYPE2_COMPARE_31_WORD_1_RESERVED_31_10_Pos 10UL
#define ETH_TYPE2_COMPARE_31_WORD_1_RESERVED_31_10_Msk 0xFFFFFC00UL


#endif /* _CYIP_ETH_V2_H_ */


/* [] END OF FILE */
