/*
 * FreeRTOS+TCP V2.4.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "list.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"

#include "cy_project.h"
#include "cy_device_headers.h"

/*****************************************************************************/
/* Local pre-processor symbols/macros ('#define')                            */
/*****************************************************************************/
#define RTOS_RX_Q_LEN             CY_MAX(ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS,CY_ETH_DEFINE_TOTAL_BD_PER_RXQUEUE)
#define ETH_FRAME_SIZE_FCS        (4)

// EMAC interface modes
#define EMAC_MII                  0
#define EMAC_RMII                 1
#define EMAC_GMII                 2
#define EMAC_RGMII                3

// EMAC speed modes
#define ETH_LINKSPEED_10          10
#define ETH_LINKSPEED_100         100
#define ETH_LINKSPEED_1000        1000

// Ethernet PHY types
#define ETH_PHY_DP83867           0
#define ETH_PHY_DP83848Q_Q1       1


/*************************************************************************************************************************************/
/*************************************************************************************************************************************/
/*************************************************************************************************************************************/
/** User Parameters      */

// Jumper settings (tviibh8m):
//                              MII      GMII/RGMII
// R149 (PHY_CLK_OUT)           -*       closed
// R127 (GTX-CLK: MAC->PHY)     open     closed
// R316 ( TX-CLK: PHY->MAC)     closed   open
//
// * don't care, open jumper if ETH_REF_CLK MCU pin shall be used for other purposes

// Jumper settings (tviic2d6m):
//                              MII      GMII/RGMII
// R235 (PHY_CLK_OUT)           -*       closed
// R241 (GTX-CLK: MAC->PHY)     open     closed
// R243 ( TX-CLK: PHY->MAC)     closed   open
//
// * don't care, open jumper if ETH_REF_CLK MCU pin shall be used for other purposes

#ifdef tviic2d4m  // PHY on eval board only connected by RMII
    #define EMAC_INTERFACE       EMAC_RMII
    #define EMAC_LINKSPEED       ETH_LINKSPEED_100
    #define ETH_PHY              ETH_PHY_DP83848Q_Q1
    #define PHY_ADDR             (1)                // Value depends on PHY and its hardware configuration
    #define AUTONEG_ENABLE       true               // Set 'true' or 'false' to select between auto negotation (only advertising the selected speed) or fixed mode
#else
    #define EMAC_INTERFACE       EMAC_RGMII
    #define EMAC_LINKSPEED       ETH_LINKSPEED_1000
    #define ETH_PHY              ETH_PHY_DP83867
    #define PHY_ADDR             (0)                // Value depends on PHY and its hardware configuration
    #define AUTONEG_ENABLE       true               // Set 'true' or 'false' to select between auto negotation (only advertising the selected speed) or fixed mode
#endif


#define OWN_MAC_ADDRESS          { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 }

/*************************************************************************************************************************************/
/*************************************************************************************************************************************/
/*************************************************************************************************************************************/

// Ethernet PHY abstraction
#if (ETH_PHY == ETH_PHY_DP83867)

    // Some error checks (combination of speed and interface mode is checked somewhere else)
    #if (EMAC_INTERFACE == EMAC_RMII)
        #error "RMII is not supported by this PHY"
    #endif

    #define ETH_PHY_INIT               Cy_Dp83867_Init
    #define ETH_PHY_CONFIGURE_PHY      Cy_Dp83867_ConfigurePhy
    #define ETH_PHY_IS_LINK_UP         Cy_Dp83867_IsLinkUp
    
    #define GET_ETH_PHY_INTERFACE_MODE_ENUM(ifModeMacro)     ((ifModeMacro == EMAC_MII) ? CY_DP83867_INTERFACE_MII : (ifModeMacro == EMAC_GMII) ? CY_DP83867_INTERFACE_GMII : CY_DP83867_INTERFACE_RGMII)
    #define GET_ETH_PHY_SPEED_MODE_ENUM(speedModeMacro)      ((speedModeMacro == ETH_LINKSPEED_10) ? CY_DP83867_SPEED_10 : (speedModeMacro == ETH_LINKSPEED_100) ? CY_DP83867_SPEED_100 : CY_DP83867_SPEED_1000)

#elif (ETH_PHY == ETH_PHY_DP83848Q_Q1)
    
    // Some error checks
    #if (EMAC_LINKSPEED == ETH_LINKSPEED_1000)
        #error "1000 MBit/s not supported by this PHY"
    #endif
    #if (EMAC_INTERFACE != EMAC_MII) && (EMAC_INTERFACE != EMAC_RMII)
        #error "Only MII and RMII are supported by this PHY"
    #endif


    #define ETH_PHY_INIT               Cy_Dp83848q_Q1_Init
    #define ETH_PHY_CONFIGURE_PHY      Cy_Dp83848q_Q1_ConfigurePhy
    #define ETH_PHY_IS_LINK_UP         Cy_Dp83848q_Q1_IsLinkUp
    
    #define GET_ETH_PHY_INTERFACE_MODE_ENUM(ifModeMacro)     ((ifModeMacro == EMAC_MII) ? CY_DP83848Q_Q1_INTERFACE_MII : CY_DP83848Q_Q1_INTERFACE_RMII)
    #define GET_ETH_PHY_SPEED_MODE_ENUM(speedModeMacro)      ((speedModeMacro == ETH_LINKSPEED_10) ? CY_DP83848Q_Q1_SPEED_10 : CY_DP83848Q_Q1_SPEED_100)

#endif


/********************************************************/
/* Device depending code */
#define HF_CLK_IDX_TO_TSU    (CY_SYSCLK_HFCLK_5)

#define ETH_REG_BASE         CY_GIG_ETH_TYPE

#define ETHx_TD0_PORT        CY_GIG_ETH_TD0_PORT
#define ETHx_TD0_PIN         CY_GIG_ETH_TD0_PIN
#define ETHx_TD0_PIN_MUX     CY_GIG_ETH_TD0_PIN_MUX

#define ETHx_TD1_PORT        CY_GIG_ETH_TD1_PORT
#define ETHx_TD1_PIN         CY_GIG_ETH_TD1_PIN
#define ETHx_TD1_PIN_MUX     CY_GIG_ETH_TD1_PIN_MUX

#define ETHx_TD2_PORT        CY_GIG_ETH_TD2_PORT
#define ETHx_TD2_PIN         CY_GIG_ETH_TD2_PIN
#define ETHx_TD2_PIN_MUX     CY_GIG_ETH_TD2_PIN_MUX

#define ETHx_TD3_PORT        CY_GIG_ETH_TD3_PORT
#define ETHx_TD3_PIN         CY_GIG_ETH_TD3_PIN
#define ETHx_TD3_PIN_MUX     CY_GIG_ETH_TD3_PIN_MUX

#define ETHx_TD4_PORT        CY_GIG_ETH_TD4_PORT
#define ETHx_TD4_PIN         CY_GIG_ETH_TD4_PIN
#define ETHx_TD4_PIN_MUX     CY_GIG_ETH_TD4_PIN_MUX

#define ETHx_TD5_PORT        CY_GIG_ETH_TD5_PORT
#define ETHx_TD5_PIN         CY_GIG_ETH_TD5_PIN
#define ETHx_TD5_PIN_MUX     CY_GIG_ETH_TD5_PIN_MUX

#define ETHx_TD6_PORT        CY_GIG_ETH_TD6_PORT
#define ETHx_TD6_PIN         CY_GIG_ETH_TD6_PIN
#define ETHx_TD6_PIN_MUX     CY_GIG_ETH_TD6_PIN_MUX

#define ETHx_TD7_PORT        CY_GIG_ETH_TD7_PORT
#define ETHx_TD7_PIN         CY_GIG_ETH_TD7_PIN
#define ETHx_TD7_PIN_MUX     CY_GIG_ETH_TD7_PIN_MUX

#define ETHx_TXER_PORT       CY_GIG_ETH_TXER_PORT
#define ETHx_TXER_PIN        CY_GIG_ETH_TXER_PIN
#define ETHx_TXER_PIN_MUX    CY_GIG_ETH_TXER_PIN_MUX

#define ETHx_TX_CTL_PORT     CY_GIG_ETH_TX_CTL_PORT
#define ETHx_TX_CTL_PIN      CY_GIG_ETH_TX_CTL_PIN
#define ETHx_TX_CTL_PIN_MUX  CY_GIG_ETH_TX_CTL_PIN_MUX

#define ETHx_RD0_PORT        CY_GIG_ETH_RD0_PORT
#define ETHx_RD0_PIN         CY_GIG_ETH_RD0_PIN
#define ETHx_RD0_PIN_MUX     CY_GIG_ETH_RD0_PIN_MUX

#define ETHx_RD1_PORT        CY_GIG_ETH_RD1_PORT
#define ETHx_RD1_PIN         CY_GIG_ETH_RD1_PIN
#define ETHx_RD1_PIN_MUX     CY_GIG_ETH_RD1_PIN_MUX

#define ETHx_RD2_PORT        CY_GIG_ETH_RD2_PORT
#define ETHx_RD2_PIN         CY_GIG_ETH_RD2_PIN
#define ETHx_RD2_PIN_MUX     CY_GIG_ETH_RD2_PIN_MUX

#define ETHx_RD3_PORT        CY_GIG_ETH_RD3_PORT
#define ETHx_RD3_PIN         CY_GIG_ETH_RD3_PIN
#define ETHx_RD3_PIN_MUX     CY_GIG_ETH_RD3_PIN_MUX

#define ETHx_RD4_PORT        CY_GIG_ETH_RD4_PORT
#define ETHx_RD4_PIN         CY_GIG_ETH_RD4_PIN
#define ETHx_RD4_PIN_MUX     CY_GIG_ETH_RD4_PIN_MUX

#define ETHx_RD5_PORT        CY_GIG_ETH_RD5_PORT
#define ETHx_RD5_PIN         CY_GIG_ETH_RD5_PIN
#define ETHx_RD5_PIN_MUX     CY_GIG_ETH_RD5_PIN_MUX

#define ETHx_RD6_PORT        CY_GIG_ETH_RD6_PORT
#define ETHx_RD6_PIN         CY_GIG_ETH_RD6_PIN
#define ETHx_RD6_PIN_MUX     CY_GIG_ETH_RD6_PIN_MUX

#define ETHx_RD7_PORT        CY_GIG_ETH_RD7_PORT
#define ETHx_RD7_PIN         CY_GIG_ETH_RD7_PIN
#define ETHx_RD7_PIN_MUX     CY_GIG_ETH_RD7_PIN_MUX

#define ETHx_RX_CTL_PORT     CY_GIG_ETH_RX_CTL_PORT
#define ETHx_RX_CTL_PIN      CY_GIG_ETH_RX_CTL_PIN
#define ETHx_RX_CTL_PIN_MUX  CY_GIG_ETH_RX_CTL_PIN_MUX

#define ETHx_RX_ER_PORT      CY_GIG_ETH_RX_ER_PORT
#define ETHx_RX_ER_PIN       CY_GIG_ETH_RX_ER_PIN
#define ETHx_RX_ER_PIN_MUX   CY_GIG_ETH_RX_ER_PIN_MUX

#define ETHx_TX_CLK_PORT     CY_GIG_ETH_TX_CLK_PORT
#define ETHx_TX_CLK_PIN      CY_GIG_ETH_TX_CLK_PIN
#define ETHx_TX_CLK_PIN_MUX  CY_GIG_ETH_TX_CLK_PIN_MUX

#define ETHx_RX_CLK_PORT     CY_GIG_ETH_RX_CLK_PORT
#define ETHx_RX_CLK_PIN      CY_GIG_ETH_RX_CLK_PIN
#define ETHx_RX_CLK_PIN_MUX  CY_GIG_ETH_RX_CLK_PIN_MUX

#define ETHx_REF_CLK_PORT    CY_GIG_ETH_REF_CLK_PORT
#define ETHx_REF_CLK_PIN     CY_GIG_ETH_REF_CLK_PIN
#define ETHx_REF_CLK_PIN_MUX CY_GIG_ETH_REF_CLK_PIN_MUX

#define ETHx_MDC_PORT        CY_GIG_ETH_MDC_PORT
#define ETHx_MDC_PIN         CY_GIG_ETH_MDC_PIN
#define ETHx_MDC_PIN_MUX     CY_GIG_ETH_MDC_PIN_MUX

#define ETHx_MDIO_PORT       CY_GIG_ETH_MDIO_PORT
#define ETHx_MDIO_PIN        CY_GIG_ETH_MDIO_PIN
#define ETHx_MDIO_PIN_MUX    CY_GIG_ETH_MDIO_PIN_MUX


// The different interface modes are specified with different input voltage threshold levels (MDIO interface is always TTL)
#if (EMAC_INTERFACE == EMAC_MII) || (EMAC_INTERFACE == EMAC_RMII)
    #define ETH_VTRIP_SEL     1
#elif (EMAC_INTERFACE == EMAC_RGMII)
    #define ETH_VTRIP_SEL     0
#elif (EMAC_INTERFACE == EMAC_GMII)
    #define ETH_VTRIP_SEL     3
#endif


/* If ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES is set to 1, then the Ethernet
 * driver will filter incoming packets and only pass the stack those packets it
 * considers need processing. */
#if ( ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES == 0 )
    #define ipCONSIDER_FRAME_FOR_PROCESSING( pucEthernetBuffer )    eProcessBuffer
#else
    #define ipCONSIDER_FRAME_FOR_PROCESSING( pucEthernetBuffer )    eConsiderFrameForProcessing( ( pucEthernetBuffer ) )
#endif

/*****************************************************************************/
/* Global variable definitions (declared in header file with 'extern')       */
/*****************************************************************************/
TaskHandle_t xEthRxTask;
QueueHandle_t xRxBufDescriptorQueue;

/*****************************************************************************/
/* Local type definitions ('typedef')                                        */
/*****************************************************************************/

/*****************************************************************************/
/* Local function prototypes ('static')                                      */
/*****************************************************************************/
static void EthernetRxFrameCallback( cy_pstc_eth_type_t pstcEth, uint8_t * u8RxBuffer, uint32_t u32Length );
static void prvEthRxTask( void *pvParameters );
static void CalcTSUIncValue(uint32_t sourceFreq, CEDI_TimerIncrement* incValue);

/*****************************************************************************/
/* Local variable definitions ('static')                                     */
/*****************************************************************************/

#if (ETH_PHY == ETH_PHY_DP83867)

    static cy_stc_dp83867_t m_ethPhyDrvHandle;

    static cy_stc_dp83867_phy_cfg_t m_ethPhyCfg = {
        .ifMode        = GET_ETH_PHY_INTERFACE_MODE_ENUM(EMAC_INTERFACE),
        .speedMode     = GET_ETH_PHY_SPEED_MODE_ENUM(EMAC_LINKSPEED),
        .enableAutoNeg = AUTONEG_ENABLE,
    };

#elif (ETH_PHY == ETH_PHY_DP83848Q_Q1)

    static cy_stc_dp83848q_q1_t m_ethPhyDrvHandle;

    static cy_stc_dp83848q_q1_phy_cfg_t m_ethPhyCfg = {
        .ifMode        = GET_ETH_PHY_INTERFACE_MODE_ENUM(EMAC_INTERFACE),
        .speedMode     = GET_ETH_PHY_SPEED_MODE_ENUM(EMAC_LINKSPEED),
        .enableAutoNeg = AUTONEG_ENABLE,
    };

#endif


/*****************************************************************************
 ** \brief Port pin configuration
 *****************************************************************************/
static const cy_stc_gpio_pin_prt_config_t m_astcEthPortPinCfg[] =
{
    { .portReg = ETHx_TD0_PORT,     .pinNum = ETHx_TD0_PIN,     .cfg = { .hsiom = ETHx_TD0_PIN_MUX,     .driveMode = CY_GPIO_DM_STRONG_IN_OFF, .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
    { .portReg = ETHx_TD1_PORT,     .pinNum = ETHx_TD1_PIN,     .cfg = { .hsiom = ETHx_TD1_PIN_MUX,     .driveMode = CY_GPIO_DM_STRONG_IN_OFF, .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
    { .portReg = ETHx_RD0_PORT,     .pinNum = ETHx_RD0_PIN,     .cfg = { .hsiom = ETHx_RD0_PIN_MUX,     .driveMode = CY_GPIO_DM_HIGHZ,         .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
    { .portReg = ETHx_RD1_PORT,     .pinNum = ETHx_RD1_PIN,     .cfg = { .hsiom = ETHx_RD1_PIN_MUX,     .driveMode = CY_GPIO_DM_HIGHZ,         .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
#if EMAC_INTERFACE != EMAC_RMII
    { .portReg = ETHx_TD2_PORT,     .pinNum = ETHx_TD2_PIN,     .cfg = { .hsiom = ETHx_TD2_PIN_MUX,     .driveMode = CY_GPIO_DM_STRONG_IN_OFF, .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
    { .portReg = ETHx_TD3_PORT,     .pinNum = ETHx_TD3_PIN,     .cfg = { .hsiom = ETHx_TD3_PIN_MUX,     .driveMode = CY_GPIO_DM_STRONG_IN_OFF, .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
    { .portReg = ETHx_RD2_PORT,     .pinNum = ETHx_RD2_PIN,     .cfg = { .hsiom = ETHx_RD2_PIN_MUX,     .driveMode = CY_GPIO_DM_HIGHZ,         .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
    { .portReg = ETHx_RD3_PORT,     .pinNum = ETHx_RD3_PIN,     .cfg = { .hsiom = ETHx_RD3_PIN_MUX,     .driveMode = CY_GPIO_DM_HIGHZ,         .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
#endif
#if EMAC_INTERFACE == EMAC_GMII
    { .portReg = ETHx_TD4_PORT,     .pinNum = ETHx_TD4_PIN,     .cfg = { .hsiom = ETHx_TD4_PIN_MUX,     .driveMode = CY_GPIO_DM_STRONG_IN_OFF, .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
    { .portReg = ETHx_TD5_PORT,     .pinNum = ETHx_TD5_PIN,     .cfg = { .hsiom = ETHx_TD5_PIN_MUX,     .driveMode = CY_GPIO_DM_STRONG_IN_OFF, .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
    { .portReg = ETHx_TD6_PORT,     .pinNum = ETHx_TD6_PIN,     .cfg = { .hsiom = ETHx_TD6_PIN_MUX,     .driveMode = CY_GPIO_DM_STRONG_IN_OFF, .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
    { .portReg = ETHx_TD7_PORT,     .pinNum = ETHx_TD7_PIN,     .cfg = { .hsiom = ETHx_TD7_PIN_MUX,     .driveMode = CY_GPIO_DM_STRONG_IN_OFF, .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
    { .portReg = ETHx_RD4_PORT,     .pinNum = ETHx_RD4_PIN,     .cfg = { .hsiom = ETHx_RD4_PIN_MUX,     .driveMode = CY_GPIO_DM_HIGHZ,         .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
    { .portReg = ETHx_RD5_PORT,     .pinNum = ETHx_RD5_PIN,     .cfg = { .hsiom = ETHx_RD5_PIN_MUX,     .driveMode = CY_GPIO_DM_HIGHZ,         .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
    { .portReg = ETHx_RD6_PORT,     .pinNum = ETHx_RD6_PIN,     .cfg = { .hsiom = ETHx_RD6_PIN_MUX,     .driveMode = CY_GPIO_DM_HIGHZ,         .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
    { .portReg = ETHx_RD7_PORT,     .pinNum = ETHx_RD7_PIN,     .cfg = { .hsiom = ETHx_RD7_PIN_MUX,     .driveMode = CY_GPIO_DM_HIGHZ,         .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
#endif
#if (EMAC_INTERFACE == EMAC_MII) || (EMAC_INTERFACE == EMAC_GMII)
    { .portReg = ETHx_TXER_PORT,    .pinNum = ETHx_TXER_PIN,    .cfg = { .hsiom = ETHx_TXER_PIN_MUX,    .driveMode = CY_GPIO_DM_STRONG_IN_OFF, .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
#endif
    { .portReg = ETHx_TX_CTL_PORT,  .pinNum = ETHx_TX_CTL_PIN,  .cfg = { .hsiom = ETHx_TX_CTL_PIN_MUX,  .driveMode = CY_GPIO_DM_STRONG_IN_OFF, .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },

#if EMAC_INTERFACE != EMAC_RGMII
    { .portReg = ETHx_RX_ER_PORT,   .pinNum = ETHx_RX_ER_PIN,   .cfg = { .hsiom = ETHx_RX_ER_PIN_MUX,   .driveMode = CY_GPIO_DM_HIGHZ,         .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
#endif
    { .portReg = ETHx_RX_CTL_PORT,  .pinNum = ETHx_RX_CTL_PIN,  .cfg = { .hsiom = ETHx_RX_CTL_PIN_MUX,  .driveMode = CY_GPIO_DM_HIGHZ,         .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
#if EMAC_INTERFACE != EMAC_RMII
    { .portReg = ETHx_RX_CLK_PORT,  .pinNum = ETHx_RX_CLK_PIN,  .cfg = { .hsiom = ETHx_RX_CLK_PIN_MUX,  .driveMode = CY_GPIO_DM_HIGHZ,         .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
#endif
#if EMAC_INTERFACE == EMAC_MII
    { .portReg = ETHx_TX_CLK_PORT,  .pinNum = ETHx_TX_CLK_PIN,  .cfg = { .hsiom = ETHx_TX_CLK_PIN_MUX,  .driveMode = CY_GPIO_DM_HIGHZ,         .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
#else
    #if EMAC_INTERFACE != EMAC_RMII
    { .portReg = ETHx_TX_CLK_PORT,  .pinNum = ETHx_TX_CLK_PIN,  .cfg = { .hsiom = ETHx_TX_CLK_PIN_MUX,  .driveMode = CY_GPIO_DM_STRONG_IN_OFF, .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
    #endif
    { .portReg = ETHx_REF_CLK_PORT, .pinNum = ETHx_REF_CLK_PIN, .cfg = { .hsiom = ETHx_REF_CLK_PIN_MUX, .driveMode = CY_GPIO_DM_HIGHZ,         .outVal = 0, .vtrip = ETH_VTRIP_SEL, .driveSel = 0 }, },
#endif
    { .portReg = ETHx_MDC_PORT,     .pinNum = ETHx_MDC_PIN,     .cfg = { .hsiom = ETHx_MDC_PIN_MUX,     .driveMode = CY_GPIO_DM_STRONG_IN_OFF, .outVal = 0, .vtrip = 1,             .driveSel = 0 }, },
    { .portReg = ETHx_MDIO_PORT,    .pinNum = ETHx_MDIO_PIN,    .cfg = { .hsiom = ETHx_MDIO_PIN_MUX,    .driveMode = CY_GPIO_DM_STRONG,        .outVal = 0, .vtrip = 1,             .driveSel = 0 }, },
};    

/** Wrapper configuration   */
static cy_str_ethif_wrapper_config m_stcWrapperConfig =
{

#if ((EMAC_INTERFACE == EMAC_MII) && (EMAC_LINKSPEED == ETH_LINKSPEED_10))
    .stcInterfaceSel = CY_ETHIF_CTL_MII_10,
#elif ((EMAC_INTERFACE == EMAC_MII) && (EMAC_LINKSPEED == ETH_LINKSPEED_100))
    .stcInterfaceSel = CY_ETHIF_CTL_MII_100,
#elif ((EMAC_INTERFACE == EMAC_GMII) && (EMAC_LINKSPEED == ETH_LINKSPEED_1000))
    .stcInterfaceSel = CY_ETHIF_CTL_GMII_1000,
#elif ((EMAC_INTERFACE == EMAC_RGMII) && (EMAC_LINKSPEED == ETH_LINKSPEED_10))
    .stcInterfaceSel = CY_ETHIF_CTL_RGMII_10,
#elif ((EMAC_INTERFACE == EMAC_RGMII) && (EMAC_LINKSPEED == ETH_LINKSPEED_100))
    .stcInterfaceSel = CY_ETHIF_CTL_RGMII_100,
#elif ((EMAC_INTERFACE == EMAC_RGMII) && (EMAC_LINKSPEED == ETH_LINKSPEED_1000))
    .stcInterfaceSel = CY_ETHIF_CTL_RGMII_1000,
#elif ((EMAC_INTERFACE == EMAC_RMII) && (EMAC_LINKSPEED == ETH_LINKSPEED_10))
    .stcInterfaceSel = CY_ETHIF_CTL_RMII_10,
#elif ((EMAC_INTERFACE == EMAC_RMII) && (EMAC_LINKSPEED == ETH_LINKSPEED_100))
    .stcInterfaceSel = CY_ETHIF_CTL_RMII_100,
#else
    #error "Error in configuration, not supported combination of EMAC_INTERFACE and/or EMAC_LINKSPEED"
#endif

    .bRefClockSource = CY_ETHIF_EXTERNAL_HSIO,  // Use external reference clock (not needed in all modes)

#if EMAC_LINKSPEED == ETH_LINKSPEED_10
  #if (EMAC_INTERFACE == EMAC_RMII)
    .u8RefClkDiv = 1u,                          // Expecting 50 MHz -> no division needed for 10 MBit/s RMII
  #else
    .u8RefClkDiv = 10u,                         // Expecting 25 MHz -> divide by 10 to get 2.5 MHz
  #endif
#elif EMAC_LINKSPEED == ETH_LINKSPEED_100
    .u8RefClkDiv = 1u,                          // Expecting 50 MHz (RMII) or 25 MHz -> no division needed
#elif EMAC_LINKSPEED == ETH_LINKSPEED_1000
    .u8RefClkDiv = 1u,                          // Expecting 125 MHz -> no division needed
#endif

};

/** Default Timer register values   */
static CEDI_1588TimerVal m_stc1588TimerValue = {
    .secsUpper = 0u,
    .secsLower = 0ul,
    .nanosecs  = 0ul,
};


/** TSU configuration   */
static CEDI_TimerIncrement m_stc1588TimerInc; // Will be updated in runtime

static cy_str_ethif_TSU_Config m_stcTSUConfig = {
    .pstcTimerValue         = &m_stc1588TimerValue,      /** start value for the counter     */
    .pstcTimerIncValue      = &m_stc1588TimerInc,
    .bOneStepTxSyncEnable   = false,                     /** useful when PTP protocol is in place    */
    .enTxDescStoreTimeStamp = CEDI_TX_TS_DISABLED,
    .enRxDescStoreTimeStamp = CEDI_RX_TS_DISABLED,
    .bStoreNSinRxDesc       = false,
};

/** General Ethernet configuration  */
static cy_stc_ethif_configuration_t m_stcENETConfig = {
    .bintrEnable         = true,                        /** Interrupt enable  */
    .dmaDataBurstLen     = CEDI_DMA_DBUR_LEN_4,
    .u8dmaCfgFlags       = CEDI_CFG_DMA_FRCE_TX_BRST,
    .mdcPclkDiv          = CEDI_MDC_DIV_BY_48,          /** source clock is 80 MHz and MDC must be less than 2.5MHz   */
    .u8rxLenErrDisc      = 0,                           /** Length error frame not discarded  */
    .u8disCopyPause      = 0,
    .u8chkSumOffEn       = 3,                           /** Checksum offloading for both Tx (bit0) and Rx (bit1) enabled    */
    .u8rx1536ByteEn      = 1,                           /** Enable receive frame up to 1536    */
    .u8rxJumboFrEn       = 0,
    .u8enRxBadPreamble   = 1,
    .u8ignoreIpgRxEr     = 0,
    .u8storeUdpTcpOffset = 0,
    .u8aw2wMaxPipeline   = 2,                           /** Value must be > 0   */
    .u8ar2rMaxPipeline   = 2,                           /** Value must be > 0   */
    .u8pfcMultiQuantum   = 0,
    .pstcWrapperConfig   = &m_stcWrapperConfig,
    .pstcTSUConfig       = &m_stcTSUConfig,             /** TSU settings    */
    .btxq0enable         = true,                        /** Tx Q0 Enabled   */
    .btxq1enable         = false,                       /** Tx Q1 Disabled  */
    .btxq2enable         = false,                       /** Tx Q2 Disabled  */
    .brxq0enable         = true,                        /** Rx Q0 Enabled   */
    .brxq1enable         = false,                       /** Rx Q1 Disabled  */
    .brxq2enable         = false,                       /** Rx Q2 Disabled  */
};

/** Interrupt configurations    */
static cy_stc_ethif_interruptconfig_t m_stcInterruptConfig = {
    .btsu_time_match        = false,          /** Time stamp unit time match event */
    .bwol_rx                = false,          /** Wake on LAN event received */
    .blpi_ch_rx             = false,          /** LPI indication status bit change received */
    .btsu_sec_inc           = false,          /** TSU seconds register increment */
    .bptp_tx_pdly_rsp       = false,          /** PTP pdelay_resp frame transmitted */
    .bptp_tx_pdly_req       = false,          /** PTP pdelay_req frame transmitted */
    .bptp_rx_pdly_rsp       = false,          /** PTP pdelay_resp frame received */
    .bptp_rx_pdly_req       = false,          /** PTP pdelay_req frame received */
    .bptp_tx_sync           = false,          /** PTP sync frame transmitted */
    .bptp_tx_dly_req        = false,          /** PTP delay_req frame transmitted */
    .bptp_rx_sync           = false,          /** PTP sync frame received */
    .bptp_rx_dly_req        = false,          /** PTP delay_req frame received */
    .bext_intr              = false,          /** External input interrupt detected */
    .bpause_frame_tx        = false,          /** Pause frame transmitted */
    .bpause_time_zero       = false,          /** Pause time reaches zero or zeroq pause frame received */
    .bpause_nz_qu_rx        = false,          /** Pause frame with non-zero quantum received */
    .bhresp_not_ok          = false,          /** DMA hresp not OK */
    .brx_overrun            = true,           /** Rx overrun error */
    .bpcs_link_change_det   = false,          /** Link status change detected by PCS */
    .btx_complete           = true,           /** Frame has been transmitted successfully */
    .btx_fr_corrupt         = true,           /** Tx frame corruption */
    .btx_retry_ex_late_coll = true,           /** Retry limit exceeded or late collision */
    .btx_underrun           = true,           /** Tx underrun */
    .btx_used_read          = true,           /** Used bit set has been read in Tx descriptor list */
    .brx_used_read          = true,           /** Used bit set has been read in Rx descriptor list */
    .brx_complete           = true,           /** Frame received successfully and stored */
    .bman_frame             = false,          /** Management Frame Sent */
    // call back functions
    .rxframecb              = EthernetRxFrameCallback,
    .txerrorcb              = NULL,
    .txcompletecb           = NULL,
    .tsuSecondInccb         = NULL,
    .tsuCompMatchcb         = NULL,
};


/*****************************************************************************/
/* Function implementation - global ('extern') and local ('static')          */
/*****************************************************************************/


/**
 *****************************************************************************
 ** Initialize the network interface
 **
 ** \return True if successful
 *****************************************************************************/
BaseType_t xNetworkInterfaceInitialise( void )
{
    xRxBufDescriptorQueue = xQueueCreate(RTOS_RX_Q_LEN, 4);

    xTaskCreate(prvEthRxTask, "ETH-RX", configMINIMAL_STACK_SIZE, NULL, (configMAX_PRIORITIES - 1), &xEthRxTask);

    // Port Pin Setup
    Cy_GPIO_Multi_Pin_Init(m_astcEthPortPinCfg, CY_GET_ARRAY_LEN(m_astcEthPortPinCfg));

    /** Initialize ENET MAC */
    uint32_t ClockFreqOfTSU;
    Cy_SysClk_GetHfClkFrequency(HF_CLK_IDX_TO_TSU, &ClockFreqOfTSU);
    CalcTSUIncValue(ClockFreqOfTSU, &m_stc1588TimerInc);
    if (CY_ETHIF_SUCCESS != Cy_EthIf_Init(ETH_REG_BASE, &m_stcENETConfig, &m_stcInterruptConfig))
    {
        CY_ASSERT(false);
    }

    cy_stc_eth_filter_config_t dstMacFilter =
    {
        .typeFilter = CY_ETH_FILTER_TYPE_DESTINATION,
        .filterAddr = (CEDI_MacAddress) { OWN_MAC_ADDRESS },
        .ignoreBytes = 0x00,
    };
    Cy_EthIf_SetFilterAddress(ETH_REG_BASE, CY_ETH_FILTER_NUM_1, &dstMacFilter);

    /** PHY initialization  */
    ETH_PHY_INIT(ETH_REG_BASE, PHY_ADDR, &m_ethPhyDrvHandle);
    ETH_PHY_CONFIGURE_PHY(&m_ethPhyCfg, &m_ethPhyDrvHandle);
    while(ETH_PHY_IS_LINK_UP(&m_ethPhyDrvHandle) == false)
    {        
    }

    return pdTRUE;
}


/**
 *****************************************************************************
 ** Callback from Ethernet driver ISR in case of received frame
 **
 ** \return none
 *****************************************************************************/
static void EthernetRxFrameCallback ( cy_pstc_eth_type_t pstcEth, uint8_t * u8RxBuffer, uint32_t u32Length )
{
    NetworkBufferDescriptor_t *pxBufferDescriptor;
    size_t xBytesReceived;

    xBytesReceived = u32Length - ETH_FRAME_SIZE_FCS; // no CRC?      CHEY

    if( xBytesReceived > 0 )
    {
        /* Allocate a network buffer descriptor that points to a buffer
        large enough to hold the received frame.  As this is the simple
        rather than efficient example the received data will just be copied
        into this buffer. */
        // pxBufferDescriptor = pxGetNetworkBufferWithDescriptor( xBytesReceived, 0 );
        pxBufferDescriptor = pxNetworkBufferGetFromISR( xBytesReceived );

        if( pxBufferDescriptor != NULL )
        {
            /* pxBufferDescriptor->pucEthernetBuffer now points to an Ethernet
            buffer large enough to hold the received data.  Copy the
            received data into pcNetworkBuffer->pucEthernetBuffer.  Here it
            is assumed ReceiveData() is a peripheral driver function that
            copies the received data into a buffer passed in as the function’s
            parameter.  Remember! While is is a simple robust technique –
            it is not efficient.  An example that uses a zero copy technique
            is provided further down this page. */
            // ReceiveData( pxBufferDescriptor->pucEthernetBuffer );

            memcpy(pxBufferDescriptor->pucEthernetBuffer, u8RxBuffer, xBytesReceived);

            pxBufferDescriptor->xDataLength = xBytesReceived;

            /* See if the data contained in the received Ethernet frame needs
            to be processed.  NOTE! It is preferable to do this in
            the interrupt service routine itself, which would remove the need
            to unblock this task for packets that don’t need processing. */
            if( eConsiderFrameForProcessing( pxBufferDescriptor->pucEthernetBuffer ) == eProcessBuffer )
            {
                BaseType_t xResult;
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                xResult = xQueueSendToBackFromISR(xRxBufDescriptorQueue, &pxBufferDescriptor, &xHigherPriorityTaskWoken);
                if(xResult == pdPASS)
                {
                    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
                }
                else
                {
                    // Queue full, drop packet
                    vNetworkBufferReleaseFromISR( pxBufferDescriptor );
                    iptraceETHERNET_RX_EVENT_LOST();
                }
            }
            else
            {
                /* The Ethernet frame can be dropped, but the Ethernet buffer
                must be released. */
                // vReleaseNetworkBufferAndDescriptor( pxBufferDescriptor );
                vNetworkBufferReleaseFromISR( pxBufferDescriptor );
            }
        }
        else
        {
            /* The event was lost because a network buffer was not available.
            Call the standard trace macro to log the occurrence. */
            iptraceETHERNET_RX_EVENT_LOST();
        }
    }
}


/**
 *****************************************************************************
 ** Deferred IRQ Handler Task for ETH RX event
 **
 ** \param pvParameters   Parameters for the task
 **
 ** \return none
 *****************************************************************************/
void prvEthRxTask( void *pvParameters )
{
    NetworkBufferDescriptor_t *pxBufferDescriptor;

    /* Used to indicate that xSendEventStructToIPTask() is being called because
    of an Ethernet receive event. */
    IPStackEvent_t xRxEvent;

    for( ;; )
    {
        /* Wait for the Ethernet MAC interrupt to indicate that another packet
        has been received.  The task notification is used in a similar way to a
        counting semaphore to count Rx events, but is a lot more efficient than
        a semaphore. */
        // ulTaskNotifyTake( pdFALSE, portMAX_DELAY );
        xQueueReceive(xRxBufDescriptorQueue, &pxBufferDescriptor, portMAX_DELAY);

        /* The event about to be sent to the TCP/IP is an Rx event. */
        xRxEvent.eEventType = eNetworkRxEvent;

        /* pvData is used to point to the network buffer descriptor that
        now references the received data. */
        xRxEvent.pvData = ( void * ) pxBufferDescriptor;

        /* Send the data to the TCP/IP stack. */
        if( xSendEventStructToIPTask( &xRxEvent, 0 ) == pdFALSE )
        {
            /* The buffer could not be sent to the IP task so the buffer
            must be released. */
            vReleaseNetworkBufferAndDescriptor( pxBufferDescriptor );

            /* Make a call to the standard trace macro to log the
            occurrence. */
            iptraceETHERNET_RX_EVENT_LOST();
        }
        else
        {
            /* The message was successfully sent to the TCP/IP stack.
            Call the standard trace macro to log the occurrence. */
            iptraceNETWORK_INTERFACE_RECEIVE();
        }

    }
}


/**
 *****************************************************************************
 ** Called from FreeRTOS network stack to transmit a frame
 **
 ** \return
 *****************************************************************************/
BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxDescriptor,
                                    BaseType_t xReleaseAfterSend )
{
    /* Simple network interfaces (as opposed to more efficient zero copy network
    interfaces) just use Ethernet peripheral driver library functions to copy
    data from the FreeRTOS+TCP buffer into the peripheral driver’s own buffer.
    This example assumes SendData() is a peripheral driver library function that
    takes a pointer to the start of the data to be sent and the length of the
    data to be sent as two separate parameters.  The start of the data is located
    by pxDescriptor->pucEthernetBuffer.  The length of the data is located
    by pxDescriptor->xDataLength. */
    // SendData( pxDescriptor->pucBuffer, pxDescriptor->xDataLength );

    Cy_EthIf_TransmitFrame(ETH_REG_BASE, pxDescriptor->pucEthernetBuffer, pxDescriptor->xDataLength, CY_ETH_QS0_0, true);

    /* Call the standard trace macro to log the send event. */
    iptraceNETWORK_INTERFACE_TRANSMIT();

    if( xReleaseAfterSend != pdFALSE )
    {
        /* It is assumed SendData() copies the data out of the FreeRTOS+TCP Ethernet
        buffer.  The Ethernet buffer is therefore no longer needed, and must be
        freed for re-use. */
        vReleaseNetworkBufferAndDescriptor( pxDescriptor );
    }

    return pdTRUE;
}



/* First statically allocate the buffers, ensuring an additional ipBUFFER_PADDING
bytes are allocated to each buffer.  This example makes no effort to align
the start of the buffers, but most hardware will have an alignment requirement.
If an alignment is required then the size of each buffer must be adjusted to
ensure it also ends on an alignment boundary.  Below shows an example assuming
the buffers must also end on an 8-byte boundary. */
#define BUFFER_ALIGNMENT  32 // Use 32-byte alignment to be inline with cacheline size (necessary so that clean & invalidate only impacts the targeted bufeer)
#define BUFFER_SIZE ( ipTOTAL_ETHERNET_FRAME_SIZE + ipBUFFER_PADDING )
#define BUFFER_SIZE_ROUNDED_UP ( ( BUFFER_SIZE + (BUFFER_ALIGNMENT -1) ) & ~(BUFFER_ALIGNMENT - 1))
static uint8_t ucBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ][ BUFFER_SIZE_ROUNDED_UP ]  __ALIGNED(BUFFER_ALIGNMENT);

/**
 *****************************************************************************
 ** Allocates RAM to the network buffers from the FreeRTOS network stack
 **
 ** \return
 *****************************************************************************/
void vNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )
{
    BaseType_t x;

    for( x = 0; x < ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS; x++ )
    {
        /* pucEthernetBuffer is set to point ipBUFFER_PADDING bytes in from the
        beginning of the allocated buffer. */
        pxNetworkBuffers[ x ].pucEthernetBuffer = &( ucBuffers[ x ][ ipBUFFER_PADDING ] );

        /* The following line is also required, but will not be required in
        future versions. */
        *( ( uint32_t * ) &ucBuffers[ x ][ 0 ] ) = ( uint32_t ) &( pxNetworkBuffers[ x ] );
    }
}


/**
 *****************************************************************************
 ** Return PHY link status
 **
 ** \return
 *****************************************************************************/
BaseType_t xGetPhyLinkStatus( void )
{
    bool bLinkUp = ETH_PHY_IS_LINK_UP(&m_ethPhyDrvHandle);
    return (bLinkUp == false) ? pdFALSE : pdTRUE;
}


/*******************************************************************************
* Function Name: CalcTSUIncValue
****************************************************************************//**
*
* \brief Calculate TSU Inc Value from source clock frequency of the TSU
*
* \param sourceFreq frequency of source clock of the TSU
* \param incValue pointer to the result
*
*******************************************************************************/
static void CalcTSUIncValue(uint32_t sourceFreq, CEDI_TimerIncrement* incValue)
{
    uint64_t secInNamo = 1000000000ull;
    uint64_t fractionBitCount = 24; // 24 bit fraction

    uint64_t incValueWithfixedPoint = (secInNamo << fractionBitCount)  / (uint64_t)sourceFreq;
    incValue->nanoSecsInc = (uint32_t)((incValueWithfixedPoint & 0xFFFFFFFFF000000ull) >> fractionBitCount);
    incValue->subNsInc    = (uint16_t)((incValueWithfixedPoint & 0x000000000FFFF00ull) >> 8ull);
    incValue->lsbSubNsInc = (uint8_t)(incValueWithfixedPoint & 0x0000000000000FFull);
    incValue->altIncCount = 0; // Don't care for now
    incValue->altNanoSInc = 0; // Don't care for now
}




/* [] END OF FILE */
