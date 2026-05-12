/***************************************************************************//**
* \file cy_ethif.c
* \version 1.0
*
* \brief
* Provides an API implementation of the Ethernet Interface driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include "cy_ethif.h"
#include "cy_project.h"
#include <string.h>

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
*       Local Variables
***************************************/
/** Cadence driver instantiation    */
static CEDI_OBJ *cyp_ethif_gemgxlobj;

/** Ethernet configurations */
static CEDI_Config  cy_ethif_cfg[CY_ETH_DEFINE_NUM_IP];

/** Cadence driver memory requirements */
static CEDI_SysReq  cy_ethif_sysreq;

/** Private data structures required for each instance of Ethernet IPs  */
static CEDI_PrivateData * cyp_ethif_pd[CY_ETH_DEFINE_NUM_IP];

/** Variables holding Statistics register values    */
static CEDI_Statistics  cyp_ethif_statistic[CY_ETH_DEFINE_NUM_IP];

/** Variables holding ethernet mac address    */
static uint8_t cyp_ethif_source_addr[CY_ETH_DEFINE_NUM_IP][6] = {0u};

/** 64 bits aligned Transmit Buffers along with their status    */
uint8_t u8TxBuf[CY_ETH_TOTAL_TX_BUF][CY_ETH_SIZE_MAX_FRAME] __ALIGNED(8) = {0};
cy_str_ethif_bufstatus cy_str_txBufStatus[CY_ETH_TOTAL_TX_BUF] = {0};       /** Buffer status will be CY_ETHIF_BUFFER_UNINIT */

/** 64 bits aligned Receive Buffers along with their status */
uint8_t u8RxBuf[CY_ETH_TOTAL_RX_BUF][CY_ETH_SIZE_MAX_FRAME] __ALIGNED(8) = {0};  /** This includes replacement buffers (requirement from cadence driver)     */
cy_str_ethif_bufstatus cy_str_rxBufStatus[CY_ETH_TOTAL_RX_BUF] = {0};            /** Buffer status will be CY_ETHIF_BUFFER_UNINIT */

/** Private data memory allocation  */
uint8_t cy_ethif_privatedata[CY_ETH_DEFINE_NUM_IP][1800] = {0};

/** Tx Descriptors */
/** CY_ETH_DEFINE_NUM_TXQS * CY_ETH_DEFINE_NUM_IP to avoid race condition in hardware; each BD list for particular queue must have 1 more BD */
uint8_t cy_ethif_tx_desc_list[CY_ETH_DEFINE_NUM_IP][CY_ETH_DEFINE_NUM_TXQS][CY_ETH_DEFINE_TOTAL_BD_PER_TXQUEUE + 1][CY_ETH_BD_SIZE] __ALIGNED(8) = {0};
 
/** Rx Descriptors  */
/** Cadence driver requires another set of buffers to replace the existing buffers after a frame has been received
* so, number of required BDs would not be same as number of buffers */
uint8_t cy_ethif_rx_desc_list[CY_ETH_DEFINE_NUM_IP][CY_ETH_DEFINE_NUM_RXQS][((CY_ETH_DEFINE_TOTAL_BD_PER_RXQUEUE + 1) * 2)][CY_ETH_BD_SIZE] __ALIGNED(8) = {0};

/** Statistics registers    */
uint8_t cy_ethif_statistic[CY_ETH_DEFINE_NUM_IP][120] = {0};

static cy_ethif_callbackfunctions stccallbackfunctions [CY_ETH_DEFINE_NUM_IP] = {0};
static cy_str_ethif_queueDisablestatus stcQueueDisStatus[CY_ETH_DEFINE_NUM_IP] = {0};

/*****************************************************************************
* Local function prototypes ('static')
*****************************************************************************/
static void Cy_EthIf_EnableInterrupts(cy_pstc_eth_type_t eth, cy_stc_ethif_interruptconfig_t * pstcInterruptList);
static void Cy_EthIf_PrepareConfiguration(cy_pstc_eth_type_t eth, cy_stc_ethif_configuration_t * pstcEthIfConfig);
static void Cy_EthIf_AssignMemory(cy_pstc_eth_type_t pstcEth);
static void Cy_EthIf_EventTx(void *pcy_privatedata, uint32_t u32event, uint8_t u8qnum);
static void Cy_EthIf_EventTxError(void *pcy_privatedata, uint32_t u32event, uint8_t u8qnum);
static void Cy_EthIf_EventRxFrame(void *pcy_privatedata, uint8_t u8qnum);
static void Cy_EthIf_EventRxError(void *pcy_privatedata, uint32_t a_event, uint8_t a_qnum);
static void Cy_EthIf_EventPhyManComplete(void *pcy_privatedata, uint8_t u8read, uint16_t u16ReadData);
static void Cy_EthIf_EventhrespError(void *pcy_privatedata, uint8_t u8qnum);
static void Cy_EthIf_EventLpPageRx(void* pcy_privatedata, struct CEDI_LpPageRx* pageRx);
static void Cy_EthIf_EventAn(void* pcy_privatedata, struct CEDI_NetAnStatus* netStat);
static void Cy_EthIf_EventLinkChange(void *pcy_privatedata, uint8_t a_linkstate);
static void Cy_EthIf_EventTsu(void *pcy_privatedata, uint32_t u32event);
static void Cy_EthIf_EventPauseFrame(void *pcy_privatedata, uint32_t u32event);
static void Cy_EthIf_EventPtp(void* pcy_privatedata, uint32_t u32type, struct CEDI_1588TimerVal* time);
static void Cy_EthIf_EventExternalInt(void * pcy_privatedata);
static void Cy_EthIf_EventWol(void * pcy_privatedata);
static void Cy_EthIf_EventLpi(void * pcy_privatedata);
static void Cy_EthIf_InitializeBuffers(void);
static cy_en_ethif_status_t Cy_EthIf_WrapperConfig(cy_pstc_eth_type_t eth, cy_str_ethif_wrapper_config * pstcWrapperConfig);
static cy_en_ethif_status_t Cy_EthIf_IPEnable(cy_pstc_eth_type_t eth);
static cy_en_ethif_status_t Cy_EthIf_IPDisable(cy_pstc_eth_type_t eth);
static cy_en_ethif_status_t Cy_EthIf_TSUInit(cy_pstc_eth_type_t eth, cy_str_ethif_TSU_Config * pstcTSUConfig);
static cy_en_ethif_status_t Cy_EthIf_DisableQueues(cy_pstc_eth_type_t eth, cy_stc_ethif_configuration_t * pstcEthIfConfig);
static bool Cy_EthIf_GetEthInstanceByIdx(uint32_t ethIdx, cy_pstc_eth_type_t* pEth);
static bool Cy_EthIf_GetEtherInstanceIndex(cy_pstc_eth_type_t eth, uint32_t* pEthIdx);
static uint32_t Cy_EthIf_GetPrivateDataIndex(CEDI_PrivateData* pPd, uint32_t* pEthIdx);
static uint32_t Cy_EthIf_GetFreeTxBuffer(uint32_t* freeTxBufferIdx);
static uint32_t Cy_EthIf_GetFreeRxBuffer(uint32_t* freeRxBufferIdx);

/*****************************************************************************
* Local Call back function supplied to Cadence driver
*****************************************************************************/
CEDI_Callbacks Cy_EthIf_Callbacks = {
    .phyManComplete = (CEDI_CbPhyManComplete)Cy_EthIf_EventPhyManComplete,
    .txEvent        = (CEDI_CbTxEvent)Cy_EthIf_EventTx,
    .txError        = (CEDI_CbTxError)Cy_EthIf_EventTxError,
    .rxFrame        = (CEDI_CbRxFrame)Cy_EthIf_EventRxFrame,
    .rxError        = (CEDI_CbRxError)Cy_EthIf_EventRxError,
    .hrespError     = (CEDI_CbHrespError)Cy_EthIf_EventhrespError,
    .lpPageRx       = (CEDI_CbLpPageRx)Cy_EthIf_EventLpPageRx,
    .anComplete     = (CEDI_CbAnComplete)Cy_EthIf_EventAn,
    .linkChange     = (CEDI_CbLinkChange)Cy_EthIf_EventLinkChange,
    .tsuEvent       = (CEDI_CbTsuEvent)Cy_EthIf_EventTsu,
    .pauseEvent     = (CEDI_CbPauseEvent)Cy_EthIf_EventPauseFrame,
    .ptpPriFrameTx  = (CEDI_CbPtpPriFrameTx)Cy_EthIf_EventPtp,
    .ptpPeerFrameTx = (CEDI_CbPtpPeerFrameTx)Cy_EthIf_EventPtp,
    .ptpPriFrameRx  = (CEDI_CbPtpPriFrameRx)Cy_EthIf_EventPtp,
    .ptpPeerFrameRx = (CEDI_CbPtpPeerFrameRx)Cy_EthIf_EventPtp,
    .lpiStatus      = (CEDI_CbLpiStatus)Cy_EthIf_EventLpi,
    .wolEvent       = (CEDI_CbWolEvent)Cy_EthIf_EventWol,
    .extInpIntr     = (CEDI_CbExtInpIntr)Cy_EthIf_EventExternalInt
};


/*******************************************************************************
* Function Name: Cy_EthIf_Init
****************************************************************************//**
*
* \brief Initializes the Ethernet MAC, Cadence Driver, EthIf and PHY 
*
* \param pstcEth
* Pointer to register area of Ethernet MAC
*
* \param pstcEthIfConfig
* Pointer to Ethernet configuration parameters
*
* \param pstcInterruptList
* List of Interrupts to enable
* 
* \return CY_ETHIF_SUCCESS      Ethernet MAC has been initialized along with Cadence driver and external PHY
* \return CY_ETHIF_BAD_PARAM    If following conditions are met:
*                               pstcEth == NULL
*                               pstcEthIfConfig == NULL
*                               pstcInterruptList == NULL
*                               GEM_GXL Object could not be created
*                               Memory assigned by Interface layer is not enough
*                               Cadence driver could not initialize Ethernet MAC
*         CY_ETHIF_MEMORY_NOT_ENOUGH
*                               Assigned memory for BDs or for Private data is not enough 
*         CY_ETHIF_LINK_DOWN    Link is not yet up
* 
* \ Note:
* This function Initializes the Ethernet MAC, Cadence driver, EthIf layer and external PHY with the provided parameters
* Port init for the Ethernet must be done before calling Cy_EthIf_Init function
* Buffer configuration parameters shall be done in EthIf.h file
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_Init(cy_pstc_eth_type_t pstcEth, cy_stc_ethif_configuration_t * pstcEthIfConfig, cy_stc_ethif_interruptconfig_t * pstcInterruptList)
{
    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Parameter checks        */
    if ((pstcEthIfConfig == NULL) || (pstcInterruptList == NULL))
    {
        return CY_ETHIF_BAD_PARAM;
    }
    else if (pstcEthIfConfig->pstcWrapperConfig == NULL)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Create GEM_GXL object  */
    cyp_ethif_gemgxlobj = CEDI_GetInstance();
    if (cyp_ethif_gemgxlobj == NULL)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Load Init parameter */
    Cy_EthIf_PrepareConfiguration(pstcEth, pstcEthIfConfig);

    if (pstcEthIfConfig->bintrEnable == true)
    {
        /** Configure Interrupts    */
        Cy_EthIf_EnableInterrupts(pstcEth, pstcInterruptList);

        /** Load Callback functions */
        stccallbackfunctions[ethIdx].rxframecb       = pstcInterruptList->rxframecb;
        stccallbackfunctions[ethIdx].txerrorcb       = pstcInterruptList->txerrorcb;
        stccallbackfunctions[ethIdx].txcompletecb    = pstcInterruptList->txcompletecb;
        stccallbackfunctions[ethIdx].tsuSecondInccb  = pstcInterruptList->tsuSecondInccb;
        stccallbackfunctions[ethIdx].tsuCompMatchcb  = pstcInterruptList->tsuCompMatchcb;
    }

    /** Initialize ENET MAC Wrapper */
    if (CY_ETHIF_BAD_PARAM == Cy_EthIf_WrapperConfig(pstcEth, pstcEthIfConfig->pstcWrapperConfig))
    {
        Cy_EthIf_IPDisable(pstcEth);
        return CY_ETHIF_BAD_PARAM;
    }

    /** Enable the IP to access EMAC registers set  */
    Cy_EthIf_IPEnable(pstcEth);

    /** Probe for checking configuration parameters and calculating memory size */
    (void)cyp_ethif_gemgxlobj->probe(&cy_ethif_cfg[ethIdx], &cy_ethif_sysreq);

    /** Check for assigned memory and required memory match */
    uint16_t u16SysReqTxBDLength = ((cy_ethif_sysreq.txDescListSize /  CY_ETH_DEFINE_NUM_TXQS)/(CY_ETH_DEFINE_TOTAL_BD_PER_TXQUEUE + 1));
    uint16_t u16SysReqRxBDLength = ((cy_ethif_sysreq.rxDescListSize /  CY_ETH_DEFINE_NUM_RXQS)/(CY_ETH_DEFINE_TOTAL_BD_PER_RXQUEUE + 1));
    if ((u16SysReqTxBDLength != CY_ETH_BD_SIZE) || (u16SysReqRxBDLength != CY_ETH_BD_SIZE))
    {
        /** Memory not enough   */
        return CY_ETHIF_MEMORY_NOT_ENOUGH;
    }

    /** assign starting addresses to local variable */
    Cy_EthIf_AssignMemory(pstcEth);

    if (sizeof(cy_ethif_privatedata[ethIdx])< cy_ethif_sysreq.privDataSize)
    {
        /** Memory not enough   */
        return CY_ETHIF_MEMORY_NOT_ENOUGH;
    }

    /** Initialization EMAC registers */
    uint32_t u32RetValue = cyp_ethif_gemgxlobj->init((void *)cyp_ethif_pd[ethIdx], &cy_ethif_cfg[ethIdx], &Cy_EthIf_Callbacks);       
    if (u32RetValue == EINVAL || u32RetValue == ENOTSUP)
    {
        Cy_EthIf_IPDisable(pstcEth);
        return CY_ETHIF_BAD_PARAM;
    }

    /** Disable Transmit and Receive Queues  */
    Cy_EthIf_DisableQueues(pstcEth, pstcEthIfConfig);

    /** TSU Initialization  */
    if (NULL != pstcEthIfConfig->pstcTSUConfig)
    {
        if (CY_ETHIF_BAD_PARAM == Cy_EthIf_TSUInit(pstcEth, pstcEthIfConfig->pstcTSUConfig))
        {
            Cy_EthIf_IPDisable(pstcEth);
            return CY_ETHIF_BAD_PARAM;
        }
    }

    /** Initialize Buffer status    */
    Cy_EthIf_InitializeBuffers();

    /** allocate assign buffers to RX BDs   */
    for (uint32_t rxQIdx = 0; rxQIdx < cy_ethif_cfg[ethIdx].rxQs; rxQIdx++)
    {
        for (uint32_t rxQElementIdx = 0; rxQElementIdx < cy_ethif_cfg[ethIdx].rxQLen[rxQIdx]; rxQElementIdx++)
        {            
            cyp_ethif_gemgxlobj->addRxBuf((void *)cyp_ethif_pd[ethIdx],
                                          rxQIdx, 
                                          (CEDI_BuffAddr *)&cy_str_rxBufStatus[(rxQElementIdx + (rxQIdx*CY_ETH_DEFINE_TOTAL_BD_PER_RXQUEUE))].cy_ethif_bufaddr.vAddr, 
                                          0);
            cy_str_rxBufStatus[(rxQElementIdx + (rxQIdx*CY_ETH_DEFINE_TOTAL_BD_PER_RXQUEUE))].enBufStatus = CY_ETHIF_BUFFER_OCCUPIED;
        }
    }

    /** additional Receive configurations   */
    cyp_ethif_gemgxlobj->setCopyAllFrames((void *)cyp_ethif_pd[ethIdx], CY_ETH_ENABLE_1);
    //cyp_ethif_gemgxlobj->setRxBadPreamble((void *)cyp_ethif_pd[ethIdx], CY_ETH_ENABLE_1);

    /** Do not drop frames with CRC error */
    //cyp_ethif_gemgxlobj->setIgnoreFcsRx((void *)cyp_ethif_pd[ethIdx], CY_ETH_ENABLE_1);

    // Optional: Setting Filter configuration
    // Optional: setting screen registers

    /** Enable MDIO */
    cyp_ethif_gemgxlobj->setMdioEnable((void *)(void *)cyp_ethif_pd[ethIdx], CY_ETH_ENABLE_1);

    /** driver start    */
    cyp_ethif_gemgxlobj->start((void *)cyp_ethif_pd[ethIdx]);

    return CY_ETHIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_EthIf_WrapperConfig
****************************************************************************//**
*
* \brief Function initializes Ethernet Wrapper to configure Interface mode, reference clock/divider etc
*
* \param u8EthIfInstance Ethernet Instance 
* \param pstcInterruptList pointer to structure list 
* 
*******************************************************************************/
static cy_en_ethif_status_t Cy_EthIf_WrapperConfig (cy_pstc_eth_type_t eth, cy_str_ethif_wrapper_config * pstcWrapperConfig)
{
    un_ETH_CTL_t unEthCtl = {0ul};
    unEthCtl.u32Register = eth->unCTL.u32Register;

    if (pstcWrapperConfig->stcInterfaceSel > CY_ETHIF_CTL_RMII_100)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    if ((pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_MII_10) ||
       (pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_MII_100))
    {
       unEthCtl.stcField.u2ETH_MODE = 0;
    }
    else if (pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_GMII_1000)
    {
        unEthCtl.stcField.u2ETH_MODE = 1;
    }
    else if ((pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_RGMII_10) ||
            (pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_RGMII_100) ||
            (pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_RGMII_1000))
    {
        unEthCtl.stcField.u2ETH_MODE = 2;
    }
    else if ((pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_RMII_10) ||
            (pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_RMII_100))
    {
        unEthCtl.stcField.u2ETH_MODE = 3;
    }

    unEthCtl.stcField.u1REFCLK_SRC_SEL =  pstcWrapperConfig->bRefClockSource;
    unEthCtl.stcField.u8REFCLK_DIV     = (pstcWrapperConfig->u8RefClkDiv - 1);

    eth->unCTL.u32Register = unEthCtl.u32Register;

    return CY_ETHIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_EthIf_IPEnable
****************************************************************************//**
*
* \brief Function enables Ethernet MAC
*
* \param u8EthIfInstance Ethernet Instance 
* 
*******************************************************************************/
static cy_en_ethif_status_t Cy_EthIf_IPEnable(cy_pstc_eth_type_t eth)
{
    eth->unCTL.stcField.u1ENABLED = CY_ETH_ENABLE_1;
    return CY_ETHIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_EthIf_IPDisable
****************************************************************************//**
*
* \brief Function Disables Ethernet MAC
*
* \param u8EthIfInstance Ethernet Instance 
* 
*******************************************************************************/
static cy_en_ethif_status_t Cy_EthIf_IPDisable(cy_pstc_eth_type_t eth)
{
    eth->unCTL.stcField.u1ENABLED = CY_ETH_DISABLE_0;
    return CY_ETHIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_EthIf_DisableQueues
****************************************************************************//**
* \brief By default all activated queues in the IP are enabled. However, only required 
* queues for tx and rx shall be enabled to make internal process faster. 
*
* \brief Function Disables Ethernet MAC
*
* \param u8EthIfInstance Ethernet Instance 
* 
*******************************************************************************/
static cy_en_ethif_status_t Cy_EthIf_DisableQueues(cy_pstc_eth_type_t eth, cy_stc_ethif_configuration_t * pstcEthIfConfig)
{
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(eth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Transmit Queue 0    */
    if (pstcEthIfConfig->btxq0enable == false)
    {
        /** Disable the queue pointer   */
        eth->unTRANSMIT_Q_PTR.stcField.u1DMA_TX_DIS_Q = 1;
        stcQueueDisStatus[ethIdx].bTxQueueDisable[0]  = true;
    }

    /** Transmit Queue 1    */
    if (pstcEthIfConfig->btxq1enable == false)
    {
        /** Disable the queue pointer   */
        eth->unTRANSMIT_Q1_PTR.stcField.u1DMA_TX_DIS_Q = 1;
        stcQueueDisStatus[ethIdx].bTxQueueDisable[1]   = true;
    }

    /** Transmit Queue 2    */
    if (pstcEthIfConfig->btxq2enable == false)
    {
        /** Disable the queue pointer   */
        eth->unTRANSMIT_Q2_PTR.stcField.u1DMA_TX_DIS_Q = 1;
        stcQueueDisStatus[ethIdx].bTxQueueDisable[2]   = true;
    }

    /** Receive Queue 0    */
    if (pstcEthIfConfig->brxq0enable == false)
    {
        /** Disable the queue pointer   */
        eth->unRECEIVE_Q_PTR.stcField.u1DMA_RX_DIS_Q = 1;
        stcQueueDisStatus[ethIdx].bRxQueueDisable[0] = true;
    }

    /** Receive Queue 1    */
    if (pstcEthIfConfig->brxq1enable == false)
    {
        /** Disable the queue pointer   */
        eth->unRECEIVE_Q1_PTR.stcField.u1DMA_RX_DIS_Q = 1;
        stcQueueDisStatus[ethIdx].bRxQueueDisable[1]  = true;
    }

    /** Receive Queue 2    */
    if (pstcEthIfConfig->brxq2enable == false)
    {
        /** Disable the queue pointer   */
        eth->unRECEIVE_Q2_PTR.stcField.u1DMA_RX_DIS_Q = 1;
        stcQueueDisStatus[ethIdx].bRxQueueDisable[2]  = true;
    }

    return CY_ETHIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_EthIf_TSUInit
****************************************************************************//**
*
* \brief Function enables Time stamp unit in EMAC
*
* \param u8EthIfInstance Ethernet Instance 
* \param pstcTSUConfig Pointer to TSU parameters 
*
* \return  
* 
*******************************************************************************/
static cy_en_ethif_status_t Cy_EthIf_TSUInit(cy_pstc_eth_type_t eth, cy_str_ethif_TSU_Config * pstcTSUConfig)
{
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(eth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** set 1588 timer value    */
    /** Load Timer Value        */
    if (EOK != cyp_ethif_gemgxlobj->set1588Timer((void *)cyp_ethif_pd[ethIdx], pstcTSUConfig->pstcTimerValue))
    {
        /** Reason could be Null pointer, hardware does not support TSU or pstcTimerValue.nanosecs>0x3FFFFFFF */
        return CY_ETHIF_BAD_PARAM;
    }

    /** Timer increment register to achieve 1 second as precise as possible  */
    if (EOK != cyp_ethif_gemgxlobj->set1588TimerInc((void *)cyp_ethif_pd[ethIdx], pstcTSUConfig->pstcTimerIncValue))
    {
        /** Reason could be Null pointer, hardware does not support TSU */
        return CY_ETHIF_BAD_PARAM;
    }

    /** one step sync enabled    */
    if (EOK != cyp_ethif_gemgxlobj->set1588OneStepTxSyncEnable((void *)cyp_ethif_pd[ethIdx], (uint8_t) pstcTSUConfig->bOneStepTxSyncEnable))
    {
        /** Reason could be Null pointer, hardware does not support TSU or bOneStepTxSyncEnable > 1 */
        return CY_ETHIF_BAD_PARAM;
    }

    /** Set the descriptor time stamp Mode   */
    if (EOK != cyp_ethif_gemgxlobj->setDescTimeStampMode((void *)cyp_ethif_pd[ethIdx], 
               pstcTSUConfig->enTxDescStoreTimeStamp, pstcTSUConfig->enRxDescStoreTimeStamp))
    {
        /** Reason could be Null pointer, hardware does not support TSU, enTxDescStoreTimeStamp > CEDI_TX_TS_ALL, enRxDescStoreTimeStamp > CEDI_RX_TS_ALL  */
        return CY_ETHIF_BAD_PARAM;
    }

    /** disabled storing nanosecond in CRC field of received frame   */
    if (EOK != cyp_ethif_gemgxlobj->setStoreRxTimeStamp((void *)cyp_ethif_pd[ethIdx], (uint8_t) pstcTSUConfig->bStoreNSinRxDesc))
    {
        /** Reason could be Null pointer, hardware does not support TSU or bStoreNSinRxDesc > 1 */
        return CY_ETHIF_BAD_PARAM;
    }

    return CY_ETHIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_EthIf_EnableInterrupts
****************************************************************************//**
*
* \brief Function initializes enabled Interrupt
*
* \param u8EthIfInstance Ethernet Instance 
* \param pstcInterruptList pointer to structure list 
* 
*******************************************************************************/
static void Cy_EthIf_EnableInterrupts(cy_pstc_eth_type_t eth, cy_stc_ethif_interruptconfig_t * pstcInterruptList)
{
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(eth, &ethIdx) == false)
    {
        /* invalid ether instance */
        CY_ASSERT(false); // should not reach here
    }

    uint32_t intrEn = 0ul;
    intrEn = (pstcInterruptList->bman_frame             == true) ? (intrEn|CEDI_EV_MAN_FRAME)             : intrEn;
    intrEn = (pstcInterruptList->brx_complete           == true) ? (intrEn|CEDI_EV_RX_COMPLETE)           : intrEn;
    intrEn = (pstcInterruptList->brx_used_read          == true) ? (intrEn|CEDI_EV_RX_USED_READ)          : intrEn;
    intrEn = (pstcInterruptList->btx_used_read          == true) ? (intrEn|CEDI_EV_TX_USED_READ)          : intrEn;
    intrEn = (pstcInterruptList->btx_underrun           == true) ? (intrEn|CEDI_EV_TX_UNDERRUN)           : intrEn;
    intrEn = (pstcInterruptList->btx_retry_ex_late_coll == true) ? (intrEn|CEDI_EV_TX_RETRY_EX_LATE_COLL) : intrEn;
    intrEn = (pstcInterruptList->btx_fr_corrupt         == true) ? (intrEn|CEDI_EV_TX_FR_CORRUPT)         : intrEn;
    intrEn = (pstcInterruptList->btx_complete           == true) ? (intrEn|CEDI_EV_TX_COMPLETE)           : intrEn;
    intrEn = (pstcInterruptList->brx_overrun            == true) ? (intrEn|CEDI_EV_RX_OVERRUN)            : intrEn;
    intrEn = (pstcInterruptList->bhresp_not_ok          == true) ? (intrEn|CEDI_EV_HRESP_NOT_OK)          : intrEn;
    intrEn = (pstcInterruptList->bpause_nz_qu_rx        == true) ? (intrEn|CEDI_EV_PAUSE_NZ_QU_RX)        : intrEn;
    intrEn = (pstcInterruptList->bpause_time_zero       == true) ? (intrEn|CEDI_EV_PAUSE_TIME_ZERO)       : intrEn;
    intrEn = (pstcInterruptList->bpause_frame_tx        == true) ? (intrEn|CEDI_EV_PAUSE_FRAME_TX)        : intrEn;
    intrEn = (pstcInterruptList->bptp_rx_dly_req        == true) ? (intrEn|CEDI_EV_PTP_RX_DLY_REQ)        : intrEn;
    intrEn = (pstcInterruptList->bptp_rx_sync           == true) ? (intrEn|CEDI_EV_PTP_RX_SYNC)           : intrEn;
    intrEn = (pstcInterruptList->bptp_tx_dly_req        == true) ? (intrEn|CEDI_EV_PTP_TX_DLY_REQ)        : intrEn;
    intrEn = (pstcInterruptList->bptp_tx_sync           == true) ? (intrEn|CEDI_EV_PTP_TX_SYNC)           : intrEn;
    intrEn = (pstcInterruptList->bptp_rx_pdly_req       == true) ? (intrEn|CEDI_EV_PTP_RX_PDLY_REQ)       : intrEn;
    intrEn = (pstcInterruptList->bptp_rx_pdly_rsp       == true) ? (intrEn|CEDI_EV_PTP_RX_PDLY_RSP)       : intrEn;
    intrEn = (pstcInterruptList->bptp_tx_pdly_req       == true) ? (intrEn|CEDI_EV_PTP_TX_PDLY_REQ)       : intrEn;
    intrEn = (pstcInterruptList->bptp_tx_pdly_rsp       == true) ? (intrEn|CEDI_EV_PTP_TX_PDLY_RSP)       : intrEn;
    intrEn = (pstcInterruptList->btsu_sec_inc           == true) ? (intrEn|CEDI_EV_TSU_SEC_INC)           : intrEn;
    intrEn = (pstcInterruptList->blpi_ch_rx             == true) ? (intrEn|CEDI_EV_LPI_CH_RX)             : intrEn;
    intrEn = (pstcInterruptList->bwol_rx                == true) ? (intrEn|CEDI_EV_WOL_RX)                : intrEn;
    intrEn = (pstcInterruptList->btsu_time_match        == true) ? (intrEn|CEDI_EV_TSU_TIME_MATCH)        : intrEn;

    cy_ethif_cfg[ethIdx].intrEnable = intrEn;
}

/*******************************************************************************
* Function Name: Cy_EthIf_PrepareConfiguration
****************************************************************************//**
*
* \brief prepares configuration based on the parameter passed to Cy_EthIf_Init function
* This prepared configuration which is dedicated for specific Ethernet instance would be used to initialize Ethernet MAC
*
* \param u8EthIfInstance Ethernet Instance 
* \param pstcEthIfConfig Pointer to Ethernet configuration passed from Application layer 
* 
* \Note:
*
*******************************************************************************/
static void Cy_EthIf_PrepareConfiguration(cy_pstc_eth_type_t eth, cy_stc_ethif_configuration_t * pstcEthIfConfig)
{
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(eth, &ethIdx) == false)
    {
        /* invalid ether instance */
        CY_ASSERT(false); // should not reach here
    }

    /** Clear configuration table   */
    memset((void *)&cy_ethif_cfg[ethIdx], 0, sizeof(cy_ethif_cfg[ethIdx]));

    /** Load GEM_GXL register base address  */
    cy_ethif_cfg[ethIdx].regBase = (uintptr_t)((uint32_t)eth + CY_ETH_GEMGXL_ADDR_OFFSET);

    /** Prepare Queues  */
    cy_ethif_cfg[ethIdx].rxQs = CY_ETH_DEFINE_NUM_RXQS;
    cy_ethif_cfg[ethIdx].txQs = CY_ETH_DEFINE_NUM_TXQS;
    
    for (uint32_t rxQIdx = 0ul; rxQIdx < cy_ethif_cfg[ethIdx].rxQs; rxQIdx += 1ul)
    {
        cy_ethif_cfg[ethIdx].rxQLen[rxQIdx] = CY_ETH_DEFINE_TOTAL_BD_PER_RXQUEUE;
    }

    for (uint32_t txQIdx = 0ul; txQIdx<cy_ethif_cfg[ethIdx].txQs; txQIdx += 1ul)
    {
        cy_ethif_cfg[ethIdx].txQLen[txQIdx] = CY_ETH_DEFINE_TOTAL_BD_PER_TXQUEUE;
    }

    /** Prepare Network control Register    */
    cy_ethif_cfg[ethIdx].pfcMultiQuantum   = 0; /* bit25    pfc_ctrl              */
    cy_ethif_cfg[ethIdx].enExtTsuPort      = 0; /* bit23    ext_tsu_port_enable, Not supported by hardware  */
    cy_ethif_cfg[ethIdx].storeUdpTcpOffset = 0; /* bit22    store_udp_offset      */
    cy_ethif_cfg[ethIdx].altSgmiiEn        = 0; /* bit21    alt_sgmii_mode, Not supported by hardware       */
    cy_ethif_cfg[ethIdx].enableMdio        = 0; /* bit4     man_port_en, Bit is separately enabled during MDIO operation  */

    /** Prepare Network Configuration Register    */               
    cy_ethif_cfg[ethIdx].uniDirEnable       = 0;                                       /* bit31    uni_direction_enable, Not supported by hardware    */
    cy_ethif_cfg[ethIdx].ignoreIpgRxEr      = 0;                                       /* bit30    ignore_ipg_rx_er, Not supported by hardware        */
    cy_ethif_cfg[ethIdx].enRxBadPreamble    = pstcEthIfConfig->u8enRxBadPreamble;      /* bit29    nsp_change           */
    /* bit27    sgmii_mode_enable  (reserved) : ifTypeSel will be set later. */
                                                                                       /*          (see the following)  */
    // don't care                                                                      /* bit26    ignore_rx_fcs        */
    cy_ethif_cfg[ethIdx].enRxHalfDupTx      = 0;                                       /* bit25    en_half_duplex_rx, not supported by hardware    */
    cy_ethif_cfg[ethIdx].chkSumOffEn        = pstcEthIfConfig->u8chkSumOffEn;          /* bit24    receive_checksum_offload_enable */
    cy_ethif_cfg[ethIdx].disCopyPause       = pstcEthIfConfig->u8disCopyPause;         /* bit23    disable_copy_of_pause_frames */
    if(ETH_AXI_MASTER_PRESENT == 1)
    {
        cy_ethif_cfg[ethIdx].dmaBusWidth        = CEDI_DMA_BUS_WIDTH_64;               /* bit22:21 data bus width        */
    }
    else
    {
        cy_ethif_cfg[ethIdx].dmaBusWidth        = CEDI_DMA_BUS_WIDTH_32;               /* bit22:21 data bus width        */
    }
    cy_ethif_cfg[ethIdx].mdcPclkDiv         = pstcEthIfConfig->mdcPclkDiv;             /* bit20:18 mdc_clock_division   */
    // don't care                                                                      /* bit17    fcs_remove           */
    cy_ethif_cfg[ethIdx].rxLenErrDisc       = pstcEthIfConfig->u8rxLenErrDisc;         /* bit16    length_field_error_frame_discard */
    cy_ethif_cfg[ethIdx].rxBufOffset        = 0;                                       /* bit15:14 receive_buffer_offset */
    // don't care                                                                      /* bit13    pause_enable         */
    // don't care                                                                      /* bit12    retry_test           */
    /* bit11    pcs_select (reserved) : ifTypeSel will be set later. */
    /* bit10    gigabit_mode_enable. : ifTypeSel will be set later. */
    cy_ethif_cfg[ethIdx].extAddrMatch       = 0;                                       /* bit9     external_address_match_enable, not supported by hardware */
    cy_ethif_cfg[ethIdx].rx1536ByteEn       = pstcEthIfConfig->u8rx1536ByteEn;         /* bit8     receive_1536_byte_frames */
    // don't care                                                                      /* bit7     unicast_hash_enable  */
    // don't care                                                                      /* bit6     multicast_hash_enable  */
    // don't care                                                                      /* bit5     no_broadcast         */
    // don't care                                                                      /* bit4     copy_all_frames      */
    cy_ethif_cfg[ethIdx].rxJumboFrEn        = pstcEthIfConfig->u8rxJumboFrEn;          /* bit3     jumbo_frames         */
    // don't care                                                                      /* bit2     discard_non_vlan_frames */
    cy_ethif_cfg[ethIdx].fullDuplex         = 1;                                       /* bit1     full_duplex          */
    /* bit0    speed : ifTypeSel will be set later. */
    
    /** configuration for cy_ethif_cfg[ethIdx].ifTypeSel    */
    if ((pstcEthIfConfig->pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_MII_10) ||
       (pstcEthIfConfig->pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_RGMII_10) ||
       (pstcEthIfConfig->pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_RMII_10))
    {
       cy_ethif_cfg[ethIdx].ifTypeSel = CEDI_IFSP_10M_MII;
    }
    else if ((pstcEthIfConfig->pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_MII_100) ||
        (pstcEthIfConfig->pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_RMII_100) ||
        (pstcEthIfConfig->pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_RGMII_100))
    {
        cy_ethif_cfg[ethIdx].ifTypeSel = CEDI_IFSP_100M_MII;
    }
    else if ((pstcEthIfConfig->pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_GMII_1000) ||
            (pstcEthIfConfig->pstcWrapperConfig->stcInterfaceSel == CY_ETHIF_CTL_RGMII_1000))
    {
        cy_ethif_cfg[ethIdx].ifTypeSel = CEDI_IFSP_1000M_GMII;
    }

    /** Prepare DMA Config register */
    cy_ethif_cfg[ethIdx].dmaAddrBusWidth    = 0;                                       /* bit30    dma_addr_bus_width_1    */
                                                                                       /* 0:32bit 1:64bit         */
    cy_ethif_cfg[ethIdx].enTxExtBD          = CY_ETH_DEFINE_BD;                        /* bit29  tx_bd_extended_mode_en  */
    cy_ethif_cfg[ethIdx].enRxExtBD          = CY_ETH_DEFINE_BD;                        /* bit28  rx_bd_extended_mode_en  */
    cy_ethif_cfg[ethIdx].dmaCfgFlags        = pstcEthIfConfig->u8dmaCfgFlags;          /* bit26  force_max_amba_burst_tx */
                                                                                       /* bit25  force_max_amba_burst_rx */
                                                                                       /* bit24  force_discard_on_err    */

    for (uint32_t rxQIdx = 0ul; rxQIdx < cy_ethif_cfg[ethIdx].rxQs; rxQIdx += 1ul)
    {
        cy_ethif_cfg[ethIdx].rxBufLength[rxQIdx] = CY_ETH_SIZE_BUF_TXQ_RXQ >> 6;            /* bit23:16 rx_buf_size       */
    }
    
    cy_ethif_cfg[ethIdx].txPktBufSize       = CY_ETH_TX_PBUF_SIZE;                     /* bit10  tx_pbuf_size            */
    cy_ethif_cfg[ethIdx].rxPktBufSize       = CY_ETH_RX_PBUF_SIZE;                     /* bit9:8 rx_pbuf_size            */
    cy_ethif_cfg[ethIdx].dmaEndianism       = 0;                                       /* bit7   endian_swap_packet  */
                                                                                       /*        0: little endian mode */
                                                                                       /*        1: endian swap mode enable for packet data (CEDI_END_SWAP_DATA) */
                                                                                       /* bit6   endian_swap_management   */
                                                                                       /*        0: little endian mode    */
                                                                                       /*        1: endian swap mode enable for management descriptor (CEDI_END_SWAP_DESC) */
    cy_ethif_cfg[ethIdx].dmaDataBurstLen    = pstcEthIfConfig->dmaDataBurstLen;        /* bit4:0   amba_burst_length                                         */
                                                                                       /* 1xxxx: attempt use burst up to 16 (CEDI_DMA_DBUR_LEN_16)  */
                                                                                       /* 01xxx: attempt use burst up to  8 (CEDI_DMA_DBUR_LEN_8)   */
                                                                                       /* 001xx: attempt use burst up to  4 (CEDI_DMA_DBUR_LEN_4)   */
                                                                                       /* 0001x: always use single burst                            */
                                                                                       /* 00001: always use single burst    (CEDI_AMBD_BURST_LEN_1) */
                                                                                       /* 00000: best AXI burst up to 256 beats                     */

    /** Prepare upper_tx_q_base_addr and upper_rx_q_base_addr register (0x4c8, 0x4D4)   */
    cy_ethif_cfg[ethIdx].upper32BuffTxQAddr = 0;                                       /* bit31:0  not used              */
    cy_ethif_cfg[ethIdx].upper32BuffRxQAddr = 0;                                       /* bit31:0  not used              */

    /** axi_max_pipeline register    */
    cy_ethif_cfg[ethIdx].aw2wMaxPipeline    = pstcEthIfConfig->u8aw2wMaxPipeline;      /* bit15:8  aw2w_max_pipeline     */
    cy_ethif_cfg[ethIdx].ar2rMaxPipeline    = pstcEthIfConfig->u8ar2rMaxPipeline;      /* bit 7:0  ar2r_max_pipeline     */
}

/*******************************************************************************
* Function Name: Cy_EthIf_PhyRegRead
****************************************************************************//**
*
* \brief Local function used by other APIs to read the PHY register
*
* \param pstcEth Ethernet instance
* \param u8RegNo Register to read
* \return read data from the register
*
* \Note: CY_ETH_PHY_ADDR must match with PHY. PHY data-sheet and hardware schematic shall be checked 
*
*******************************************************************************/
uint32_t Cy_EthIf_PhyRegRead(cy_pstc_eth_type_t pstcEth, uint8_t u8RegNo, uint8_t u8PHYAddr)
{
    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    cyp_ethif_gemgxlobj->phyStartMdioRead( (void *)cyp_ethif_pd[ethIdx], CY_ETHIF_PHY_FLAG, u8PHYAddr, u8RegNo);

    while (cyp_ethif_gemgxlobj->getMdioIdle((void *)cyp_ethif_pd[ethIdx]) == CY_ETH_MDIO_BUSY_0);

    uint16_t    u16ReadData;
    uint32_t u32result = cyp_ethif_gemgxlobj->getMdioReadData((void *)cyp_ethif_pd[ethIdx], &u16ReadData);

    /* Check is reading was no problem */
    CY_ASSERT(u32result == EOK);

    return (uint32_t)u16ReadData;
}

/*******************************************************************************
* Function Name: Cy_EthIf_PhyRegWrite
****************************************************************************//**
*
* \brief Local function used by other APIs to write the PHY register
*
* \param pstcEth Ethernet instance
* \param u8RegNo Register to write
* \param u16Data data to write
* \return 
*
* \Note: u8PHYAddr must match with PHY. PHY data-sheet and hardware schematic shall be checked 
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_PhyRegWrite(cy_pstc_eth_type_t pstcEth, uint8_t u8RegNo, uint16_t u16Data, uint8_t u8PHYAddr)
{
    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    cyp_ethif_gemgxlobj->phyStartMdioWrite( (void *)cyp_ethif_pd[ethIdx], CY_ETHIF_PHY_FLAG, u8PHYAddr, u8RegNo, u16Data);

    while(cyp_ethif_gemgxlobj->getMdioIdle((void *)cyp_ethif_pd[ethIdx]) == CY_ETH_MDIO_BUSY_0);

    return CY_ETHIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_EthIf_AssignMemory
****************************************************************************//**
*
* \brief Local function is used to initialize private data structure, tx and rx queue start address 
*
* \param u8EthIfInstance Ethernet Instance number 
*
*******************************************************************************/
static void Cy_EthIf_AssignMemory(cy_pstc_eth_type_t pstcEth)
{
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        /* invalid ether instance */
        CY_ASSERT(false); // should not reach here
    }

    cyp_ethif_pd[ethIdx]            = (CEDI_PrivateData *)&cy_ethif_privatedata[ethIdx];
    cy_ethif_cfg[ethIdx].rxQAddr    = (uintptr_t)cy_ethif_rx_desc_list[ethIdx];
    cy_ethif_cfg[ethIdx].txQAddr    = (uintptr_t)cy_ethif_tx_desc_list[ethIdx];
    cy_ethif_cfg[ethIdx].statsRegs  = (uintptr_t) &cyp_ethif_statistic[ethIdx];
    cy_ethif_cfg[ethIdx].rxQPhyAddr = cy_ethif_cfg[ethIdx].rxQAddr;             /** get the physical address */
    cy_ethif_cfg[ethIdx].txQPhyAddr = cy_ethif_cfg[ethIdx].txQAddr;             /** get the physical address */
}

/*******************************************************************************
* Function Name: Cy_EthIf_GetTimerValue
****************************************************************************//**
*
* \brief Returns the current timer value from TSU register
*
* \param pstcEth Ethernet Instance
* \param pstcRetTmrValue [out] pointer to data structure to return the values
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_GetTimerValue(cy_pstc_eth_type_t pstcEth, CEDI_1588TimerVal* pstcRetTmrValue)
{
    if (pstcRetTmrValue == NULL)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    cyp_ethif_gemgxlobj->get1588Timer((void *)cyp_ethif_pd[ethIdx], pstcRetTmrValue);

    return CY_ETHIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_EthIf_SetTimerValue
****************************************************************************//**
*
* \brief Setting the current timer value in TSU register
*
* \param pstcEth Ethernet Instance
* \param pstcTmrValue pointer to data structure to configure register with
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_SetTimerValue (cy_pstc_eth_type_t pstcEth, CEDI_1588TimerVal * pstcTmrValue)
{
    if (pstcTmrValue == NULL)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    if (EOK != cyp_ethif_gemgxlobj->set1588Timer((void *)cyp_ethif_pd[ethIdx], pstcTmrValue))
    {
        /** Reason could be Null pointer, hardware does not support TSU or pstcTimerValue.nanosecs>0x3FFFFFFF */
        return CY_ETHIF_BAD_PARAM;
    }

    return CY_ETHIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_EthIf_ConfigPause
****************************************************************************//**
*
* \brief Configures the Pause Frame transmission according to IEEE 802.3X standard
*
* \param pstcEth Ethernet Instance
* \param u16PauseQuanta Time Quanta
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_ConfigPause(cy_pstc_eth_type_t pstcEth, uint16_t u16PauseQuanta)
{
    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Set Tx Pause Quanta for Priority 0   */
    if (EOK != cyp_ethif_gemgxlobj->setTxPauseQuantum((void *)cyp_ethif_pd[ethIdx], u16PauseQuanta, CY_ETHIF_PAUSE_P0))
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Enable Receive Pause Frames */
    cyp_ethif_gemgxlobj->setCopyPauseDisable((void *)cyp_ethif_pd[ethIdx], CY_ETH_DISABLE_0);

    /** Enable Pause Frames */
    cyp_ethif_gemgxlobj->setPauseEnable((void *)cyp_ethif_pd[ethIdx], CY_ETH_ENABLE_1);

    return CY_ETHIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_EthIf00_DecodeEvent
****************************************************************************//**
*
* \brief Interrupt handler for Ethernet instance ETH0
* Function calls isr function to decode the event. 
*
* \param void
*
*******************************************************************************/
void Cy_EthIf00_DecodeEvent(void)
{
    cyp_ethif_gemgxlobj->isr((void *)cyp_ethif_pd[0]);
    return;
}

#ifdef ETH1
/*******************************************************************************
* Function Name: Cy_EthIf01_DecodeEvent
****************************************************************************//**
*
* \brief Interrupt handler for Ethernet instance ETH1
*
* \param void
* 
*
*******************************************************************************/
void Cy_EthIf01_DecodeEvent(void)
{
    cyp_ethif_gemgxlobj->isr((void *)cyp_ethif_pd[1]);
    return;
}
#endif

/*******************************************************************************
* Function Name: Cy_EthIf_GetTxBufferIndexFromAddress
****************************************************************************//**
*
* \brief Get TX buffer index the buffer address
*
* \param address the buffer address
*
* \return the index of the buffer
*
*******************************************************************************/
static uint32_t Cy_EthIf_GetTxBufferIndexFromAddress(uint32_t address)
{
    /* Note: This driver does not reserve a buffer dedicated to ethernet instances and queues.
            All ethernet instances and queues use a common area as a TX buffer.*/
    return ((address - (uint32_t)u8TxBuf) / CY_ETH_SIZE_MAX_FRAME);
}

/*******************************************************************************
* Function Name: Cy_EthIf_EventTx
****************************************************************************//**
*
* \brief Function called by cadence driver upon getting Tx Event
*
* \param pcy_privatedata    Instance specific private data
* \param u32event occurred event
* \param u8qnum Queue number 
*
*******************************************************************************/
static void Cy_EthIf_EventTx(void *pcy_privatedata, uint32_t u32event, uint8_t u8qnum)
{
    /** derive the instance */
    uint32_t           ethIdx;
    if(Cy_EthIf_GetPrivateDataIndex(pcy_privatedata, &ethIdx) == false)
    {
        /* invalid private data pointer */
        CY_ASSERT(false); // should not reach here
    }

    cy_pstc_eth_type_t eth;
    if(Cy_EthIf_GetEthInstanceByIdx(ethIdx, &eth) == false)
    {
        // invalid ether index
        CY_ASSERT(false); // should not reach here
    }

    if ((u32event&CEDI_EV_TX_COMPLETE) != 0) 
    {
        CEDI_TxDescData Tx_DescData;
        while(cyp_ethif_gemgxlobj->freeTxDesc((void *)cyp_ethif_pd[ethIdx], u8qnum, &Tx_DescData) == EOK)
        {
            /** application callback function     */
            uint32_t bufIdx = Cy_EthIf_GetTxBufferIndexFromAddress((uint32_t)Tx_DescData.bufAdd.pAddr);
            if (stccallbackfunctions[ethIdx].txcompletecb != NULL)
            {
                stccallbackfunctions[ethIdx].txcompletecb(eth, u8qnum, bufIdx);
            }
            cy_str_txBufStatus[bufIdx].enBufStatus = CY_ETHIF_BUFFER_RELEASED;
        }
    }
    return;
}

/*******************************************************************************
* Function Name: Cy_EthIf_EventTxError
****************************************************************************//**
*
* \brief Function called by cadence driver upon getting Tx Error Event
*
* \param pcy_privatedata    Instance specific private data
* \param u32event occurred event
* \param u8qnum Queue number 
*
*******************************************************************************/
static void Cy_EthIf_EventTxError(void *pcy_privatedata, uint32_t u32event, uint8_t u8qnum)
{
    /** derive the instance */
    uint32_t           ethIdx;
    if(Cy_EthIf_GetPrivateDataIndex(pcy_privatedata, &ethIdx) == false)
    {
        // invalid private data pointer
        CY_ASSERT(false); // should not reach here
    }

    cy_pstc_eth_type_t eth;
    if(Cy_EthIf_GetEthInstanceByIdx(ethIdx, &eth) == false)
    {
        // invalid ether index
        CY_ASSERT(false); // should not reach here
    }

    /** application callback function     */
    if (stccallbackfunctions[ethIdx].txerrorcb != NULL)
    {
        stccallbackfunctions[ethIdx].txerrorcb(eth, u8qnum);
    }

    /** Release the buffer  */
    CEDI_TxDescData Tx_DescData;
    cyp_ethif_gemgxlobj->freeTxDesc((void *)cyp_ethif_pd[ethIdx], u8qnum, &Tx_DescData);
    for (uint8_t txBufIdx = 0; txBufIdx < CY_ETH_TOTAL_TX_BUF; txBufIdx++)
    {
        if ((uint32_t*)Tx_DescData.bufAdd.pAddr == (uint32_t*)cy_str_txBufStatus[txBufIdx].cy_ethif_bufaddr.pAddr)
        {
            cy_str_txBufStatus[txBufIdx].enBufStatus = CY_ETHIF_BUFFER_RELEASED;
            break;
        }
    }

    return;
}

/*******************************************************************************
* Function Name: Cy_EthIf_EventRxFrame
****************************************************************************//**
*
* \brief Function called by cadence driver upon getting Rx Event
*
* \param pcy_privatedata    Instance specific private data
* \param u8qnum Queue number 
*
*******************************************************************************/
static void Cy_EthIf_EventRxFrame(void *pcy_privatedata, uint8_t u8qnum)
{
    /** derive the instance */
    uint32_t           ethIdx;
    if(Cy_EthIf_GetPrivateDataIndex(pcy_privatedata, &ethIdx) == false)
    {
        // invalid private data pointer
        CY_ASSERT(false); // should not reach here
    }

    cy_pstc_eth_type_t eth;
    if(Cy_EthIf_GetEthInstanceByIdx(ethIdx, &eth) == false)
    {
        // invalid ether index
        CY_ASSERT(false); // should not reach here
    }

    /** number of used buffers */
    uint32_t u32RxNum;
    u32RxNum = cyp_ethif_gemgxlobj->numRxUsed((void *)cyp_ethif_pd[ethIdx], u8qnum);

    /** read receive queue */
    while(u32RxNum!=0)
    {
        /** Get the new Buffer to replace the used one  */
        uint32_t rxBufferIndex;
        uint32_t bufStatus = Cy_EthIf_GetFreeRxBuffer(&rxBufferIndex);
        if(CY_ETHIF_NO_BUFFER_AVAILABLE == bufStatus)
        {
            /** check for first released buffer */
            bufStatus = Cy_EthIf_FreeOneReleasedRxBuffer(&rxBufferIndex);
            if (CY_ETHIF_NO_BUFFER_AVAILABLE == rxBufferIndex)
            {
                return;     // No buffer available
            }
        }

        CEDI_BuffAddr   TmpBufAddr;
        TmpBufAddr.vAddr = cy_str_rxBufStatus[rxBufferIndex].cy_ethif_bufaddr.vAddr;
        TmpBufAddr.pAddr = cy_str_rxBufStatus[rxBufferIndex].cy_ethif_bufaddr.pAddr;

        uint32_t u32Result;
        CEDI_RxDescData Rx_DescData;
        u32Result = cyp_ethif_gemgxlobj->readRxBuf((void *)cyp_ethif_pd[ethIdx],
                                          u8qnum,
                                          &TmpBufAddr,
                                          CY_ETHIF_BUFFER_CLEARED_0,
                                          &Rx_DescData);
        CY_ASSERT(u32Result == EOK);
        (void) u32Result;

        /** change buffer status to OCCUPIED    */
        cy_str_rxBufStatus[rxBufferIndex].enBufStatus = CY_ETHIF_BUFFER_OCCUPIED;
        
        switch (Rx_DescData.status)
        {
        case CEDI_RXDATA_SOF_EOF:     // 0
        {
            /** receive start and end frame */
            CEDI_RxDescStat Rx_DescStat;
            cyp_ethif_gemgxlobj->getRxDescStat((void *)cyp_ethif_pd[ethIdx],
                                         Rx_DescData.rxDescStat,
                                         &Rx_DescStat);

            /** application callback function     */
            if (stccallbackfunctions[ethIdx].rxframecb != NULL)
            {
                /** Invalidate D cache for received buffer */
                Cy_SysLib_InvalidateCoreDCacheByAddr((uint32_t*)TmpBufAddr.pAddr, Rx_DescStat.bufLen);
                stccallbackfunctions[ethIdx].rxframecb(eth, (uint8_t*)TmpBufAddr.pAddr, Rx_DescStat.bufLen);
            }

            /** Release the buffer  */
            for (uint32_t rxBufIdx = 0; rxBufIdx < CY_ETH_TOTAL_RX_BUF; rxBufIdx++)
            {
                if ((uint32_t*)TmpBufAddr.pAddr == (uint32_t*)cy_str_rxBufStatus[rxBufIdx].cy_ethif_bufaddr.pAddr)
                {
                    cy_str_rxBufStatus[rxBufIdx].enBufStatus = CY_ETHIF_BUFFER_RELEASED;
                    break;
                }
            }
            break;
        }
        case CEDI_RXDATA_SOF_ONLY:    // 1
            /* fragment start */
            break; //return;
        case CEDI_RXDATA_NO_FLAG:     // 2
            /* fragment */
            break; //return;
        case CEDI_RXDATA_EOF_ONLY:    // 3
            /* fragment end */
            break;
        case CEDI_RXDATA_NODATA:      // 4
            /* normal leaving */
            return;     /* from here it breaks while loop   */
        default:
            break;
        }
        u32RxNum--;
    }

    /* clear rx status (receive_status register) */
    // // TODO: Load the value in u32RxStatus
    //cyp_ethif_gemgxlobj->clearRxStatus((void *)cyp_ethif_pd[u8EthIfInstance], u32RxStatus);

    return;
}

/*******************************************************************************
* Function Name: Cy_EthIf_EventRxError
****************************************************************************//**
*
* \brief Function called by cadence driver upon getting Rx Error Event
*
* \param pcy_privatedata    Instance specific private data
* \param u32event occurred event
* \param u8qnum Queue number 
*
*******************************************************************************/
static void Cy_EthIf_EventRxError(void *pcy_privatedata, uint32_t a_event, uint8_t a_qnum)
{
    return;
}

/*******************************************************************************
* Function Name: Cy_EthIf_EventPhyManComplete
****************************************************************************//**
*
* \brief 
*
* \param 
* 
*
*******************************************************************************/
static void Cy_EthIf_EventPhyManComplete(void *pcy_privatedata, uint8_t u8read, uint16_t u16ReadData)
{
    return;
}

/*******************************************************************************
* Function Name: Cy_EthIf_EventhrespError
****************************************************************************//**
*
* \brief 
*
* \param 
* 
*
*******************************************************************************/
static void Cy_EthIf_EventhrespError(void *pcy_privatedata, uint8_t u8qnum)
{
    return;
}

/*******************************************************************************
* Function Name: Cy_EthIf_EventLpPageRx
****************************************************************************//**
*
* \brief 
*
* \param 
* 
*
*******************************************************************************/
static void Cy_EthIf_EventLpPageRx(void* pcy_privatedata, struct CEDI_LpPageRx* pageRx)
{
    return;
}

/*******************************************************************************
* Function Name: Cy_EthIf_EventAn
****************************************************************************//**
*
* \brief 
*
* \param 
* 
*
*******************************************************************************/
static void Cy_EthIf_EventAn(void* pcy_privatedata, struct CEDI_NetAnStatus* netStat)
{
    return;
}

/*******************************************************************************
* Function Name: Cy_EthIf_EventLinkChange
****************************************************************************//**
*
* \brief 
*
* \param 
* 
*
*******************************************************************************/
static void Cy_EthIf_EventLinkChange(void *pcy_privatedata, uint8_t a_linkstate)
{
    return;
}
/*******************************************************************************
* Function Name: Cy_EthIf_EventTsu
****************************************************************************//**
*
* \brief 
*
* \param 
* 
*
*******************************************************************************/
static void Cy_EthIf_EventTsu (void *pcy_privatedata, uint32_t u32event)
{
    /** derive the instance */
    uint32_t           ethIdx;
    if(Cy_EthIf_GetPrivateDataIndex(pcy_privatedata, &ethIdx) == false)
    {
        // invalid private data pointer
        CY_ASSERT(false); // should not reach here
    }

    cy_pstc_eth_type_t eth;
    if(Cy_EthIf_GetEthInstanceByIdx(ethIdx, &eth) == false)
    {
        // invalid ether index
        CY_ASSERT(false); // should not reach here
    }

    /** Event generated if second count of the timer has incremented */
    if (u32event & CEDI_EV_TSU_SEC_INC)
    {
        /** application callback function     */
        if (stccallbackfunctions[ethIdx].tsuSecondInccb != NULL)
        {
            stccallbackfunctions[ethIdx].tsuSecondInccb(eth);
        }
    }

    /** Timer count matched event    */
    if (u32event & CEDI_EV_TSU_TIME_MATCH)
    {
        /** application callback function     */
        if (stccallbackfunctions[ethIdx].tsuCompMatchcb != NULL)
        {
            stccallbackfunctions[ethIdx].tsuCompMatchcb(eth);
        }
    }

    return;
}
/*******************************************************************************
* Function Name: Cy_EthIf_EventPauseFrame
****************************************************************************//**
*
* \brief 
*
* \param 
* 
*
*******************************************************************************/
static void Cy_EthIf_EventPauseFrame(void *pcy_privatedata, uint32_t u32event)
{
    if (u32event & CEDI_EV_PAUSE_FRAME_TX)
    {
    }
    
    if (u32event & CEDI_EV_PAUSE_NZ_QU_RX)
    {
    }

    return;
}
/*******************************************************************************
* Function Name: Cy_EthIf_EventPtp
********************************************************************************
*
* \brief 
*
* \param 
* 
*
*******************************************************************************/
static void Cy_EthIf_EventPtp (void* pcy_privatedata, uint32_t u32type, struct CEDI_1588TimerVal* time)
{
    if (u32type & CEDI_EV_PTP_TX_SYNC)
    {
    }

    if (u32type & CEDI_EV_PTP_TX_PDLY_REQ)
    {
    }

    if (u32type & CEDI_EV_PTP_TX_PDLY_RSP)
    {
    }

    if (u32type & CEDI_EV_PTP_RX_SYNC)
    {
    }

    if (u32type & CEDI_EV_PTP_RX_PDLY_REQ)
    {
    }

    if (u32type & CEDI_EV_PTP_RX_PDLY_RSP)
    {
    }

    return;
}

/*******************************************************************************
* Function Name: Cy_EthIf_EventExternalInt
********************************************************************************
*
* \brief 
*
* \param 
* 
*
*******************************************************************************/
static void Cy_EthIf_EventExternalInt(void * pcy_privatedata)
{
   return;
}

/*******************************************************************************
* Function Name: Cy_EthIf_EventWol
********************************************************************************
*
* \brief 
*
* \param 
* 
*
*******************************************************************************/
static void Cy_EthIf_EventWol(void * pcy_privatedata)
{
    return;
}
/*******************************************************************************
* Function Name: Cy_EthIf_EventLpi
********************************************************************************
*
* \brief 
*
* \param 
* 
*
*******************************************************************************/
static void Cy_EthIf_EventLpi(void * pcy_privatedata)
{
    return;
}

/*******************************************************************************
* Function Name: Cy_EthIf_InitializeBuffers
****************************************************************************//**
*
* \brief Function initializes the buffer status.
*
* \param none
*
*******************************************************************************/
static void Cy_EthIf_InitializeBuffers(void)
{
    // Clear all TX buffers
    for (uint32_t txBufIdx = 0ul; txBufIdx < CY_ETH_TOTAL_TX_BUF; txBufIdx+=1ul)
    {
        cy_str_txBufStatus[txBufIdx].cy_ethif_bufaddr.vAddr = (uintptr_t)&u8TxBuf[txBufIdx][0];
        cy_str_txBufStatus[txBufIdx].cy_ethif_bufaddr.pAddr = (uintptr_t)&u8TxBuf[txBufIdx][0];
        cy_str_txBufStatus[txBufIdx].enBufStatus = CY_ETHIF_BUFFER_FREE;
    }

    // Clear all RX buffers
    for (uint32_t rxBufIdx = 0ul; rxBufIdx < CY_ETH_TOTAL_RX_BUF; rxBufIdx+=1ul)
    {
        cy_str_rxBufStatus[rxBufIdx].cy_ethif_bufaddr.vAddr = (uintptr_t)&u8RxBuf[rxBufIdx][0];
        cy_str_rxBufStatus[rxBufIdx].cy_ethif_bufaddr.pAddr = (uintptr_t)&u8RxBuf[rxBufIdx][0];
        cy_str_rxBufStatus[rxBufIdx].enBufStatus = CY_ETHIF_BUFFER_FREE;
    }
}

/*******************************************************************************
* Function Name: Cy_EthIf_GetFreeTxBuffer
****************************************************************************//**
*
* \brief returns the free TX buffer index which driver can start working with.
*
* \return the index of the free buffer instance.
*         CY_ETHIF_NO_BUFFER_AVAILABLE when no buffer is free.
*
*******************************************************************************/
static uint32_t Cy_EthIf_GetFreeTxBuffer(uint32_t* freeTxBufferIdx)
{
    static uint32_t txBufIdx = 0;

    for (uint32_t counter = 0; counter < CY_ETH_TOTAL_TX_BUF; counter += 1ul)
    {
        if (cy_str_txBufStatus[txBufIdx].enBufStatus == CY_ETHIF_BUFFER_FREE)
        {
            // Save the found free buffer index
            *freeTxBufferIdx = txBufIdx;

            // Increment the static TX buffer index for the next calls
            txBufIdx = (txBufIdx + 1ul) % CY_ETH_TOTAL_TX_BUF;

            // Return the status
            return CY_ETHIF_BUFFER_AVAILABLE;
        }

        // Increment the static TX buffer index for the search
        txBufIdx = (txBufIdx + 1ul) % CY_ETH_TOTAL_TX_BUF;
    }

    return CY_ETHIF_NO_BUFFER_AVAILABLE;
}

/*******************************************************************************
* Function Name: Cy_EthIf_GetFreeRxBuffer
****************************************************************************//**
*
* \brief returns the free RX buffer index which driver can start working with.
*
* \return the index of the free buffer instance.
*         CY_ETHIF_NO_BUFFER_AVAILABLE when no buffer is free.
*
*******************************************************************************/
static uint32_t Cy_EthIf_GetFreeRxBuffer(uint32_t* freeRxBufferIdx)
{
    static uint32_t rxBufIdx = 0;
    for (uint32_t counter = 0; counter < CY_ETH_TOTAL_RX_BUF; counter += 1ul)
    {
        if (cy_str_rxBufStatus[rxBufIdx].enBufStatus == CY_ETHIF_BUFFER_FREE)
        {
            // Save the found free buffer index
            *freeRxBufferIdx = rxBufIdx;

            // Increment the static RX buffer index for the next calls
            rxBufIdx = (rxBufIdx + 1ul) % CY_ETH_TOTAL_RX_BUF;

            // Return the status
            return CY_ETHIF_BUFFER_AVAILABLE;
        }

        // Increment the static RX buffer index for the search
        rxBufIdx = (rxBufIdx + 1ul) % CY_ETH_TOTAL_RX_BUF;
    }

    return CY_ETHIF_NO_BUFFER_AVAILABLE;
}

/*******************************************************************************
* Function Name: Cy_EthIf_FreeAllReleasedTxBuffer
****************************************************************************//**
*
* \brief Each buffer goes through cycle of FREE - OCCUPIED - RELEASED - FREE 
*        Function looks into status of all TX buffer and reset all RELEASED buffer
*        to FREE.
*
* \return CY_ETHIF_BUFFER_AVAILABLE when this freed at least one released buffer.
*         CY_ETHIF_NO_BUFFER_AVAILABLE No Buffer in a state of "CY_ETHIF_BUFFER_RELEASED"
*
*******************************************************************************/
uint32_t Cy_EthIf_FreeAllReleasedTxBuffer(void)
{
    bool bBufferReleased = false;

    for (uint32_t txBufIdx = 0; txBufIdx < CY_ETH_TOTAL_TX_BUF; txBufIdx += 1ul)
    {
        if (cy_str_txBufStatus[txBufIdx].enBufStatus == CY_ETHIF_BUFFER_RELEASED)
        {
            cy_str_txBufStatus[txBufIdx].enBufStatus = CY_ETHIF_BUFFER_FREE;
            bBufferReleased = true;
        }
    }

    if(bBufferReleased)
    {
        return CY_ETHIF_BUFFER_AVAILABLE;
    }
    else
    {
        return CY_ETHIF_NO_BUFFER_AVAILABLE;
    }
}

/*******************************************************************************
* Function Name: Cy_EthIf_FreeOneReleasedTxBuffer
****************************************************************************//**
*
* \brief Each buffer goes through cycle of FREE - OCCUPIED - RELEASED - FREE 
*        Function looks into status of each TX buffer and reset one of them 
*        to FREE if status is RELEASED.
*
* \param  pFreedTxBufIdx pointer to variable which saves the index of freed TX buffer.
*
* \return CY_ETHIF_BUFFER_AVAILABLE when this freed a released buffer.
*         CY_ETHIF_NO_BUFFER_AVAILABLE No Buffer in a state of "CY_ETHIF_BUFFER_RELEASED"
*
*******************************************************************************/
uint32_t Cy_EthIf_FreeOneReleasedTxBuffer(uint32_t* pFreedTxBufIdx)
{
    for (uint32_t txBufIdx = 0; txBufIdx < CY_ETH_TOTAL_TX_BUF; txBufIdx += 1ul)
    {
        if (cy_str_txBufStatus[txBufIdx].enBufStatus == CY_ETHIF_BUFFER_RELEASED)
        {
            cy_str_txBufStatus[txBufIdx].enBufStatus = CY_ETHIF_BUFFER_FREE;
            if(pFreedTxBufIdx != NULL)
            {
                *pFreedTxBufIdx = txBufIdx;
            }
            return CY_ETHIF_BUFFER_AVAILABLE;
        }
    }
    return CY_ETHIF_NO_BUFFER_AVAILABLE;
}

/*******************************************************************************
* Function Name: Cy_EthIf_FreeAllReleasedRxBuffer
****************************************************************************//**
*
* \brief Each buffer goes through cycle of FREE - OCCUPIED - RELEASED - FREE 
*        Function looks into status of all RX buffer and reset all RELEASED buffer
*        to FREE.
*
* \return CY_ETHIF_BUFFER_AVAILABLE when this freed at least one released buffer.
*         CY_ETHIF_NO_BUFFER_AVAILABLE No Buffer in a state of "CY_ETHIF_BUFFER_RELEASED"
*
*******************************************************************************/
uint32_t Cy_EthIf_FreeAllReleasedRxBuffer(void)
{
    bool bBufferReleased = false;

    for (uint32_t rxBufIdx = 0; rxBufIdx < CY_ETH_TOTAL_RX_BUF; rxBufIdx += 1ul)
    {
        if (cy_str_rxBufStatus[rxBufIdx].enBufStatus == CY_ETHIF_BUFFER_RELEASED)
        {
            cy_str_rxBufStatus[rxBufIdx].enBufStatus = CY_ETHIF_BUFFER_FREE;
            bBufferReleased = true;
        }
    }

    if(bBufferReleased)
    {
        return CY_ETHIF_BUFFER_AVAILABLE;
    }
    else
    {
        return CY_ETHIF_NO_BUFFER_AVAILABLE;
    }
}

/*******************************************************************************
* Function Name: Cy_EthIf_FreeOneReleasedRxBuffer
****************************************************************************//**
*
* \brief Each buffer goes through cycle of FREE - OCCUPIED - RELEASED - FREE 
*        Function looks into status of each RX buffer and reset one of them 
*        to FREE if status is RELEASED.
*
* \param  pFreedRxBufIdx pointer to variable which saves the index of freed RX buffer.
*
* \return CY_ETHIF_BUFFER_AVAILABLE when this freed a released buffer.
*         CY_ETHIF_NO_BUFFER_AVAILABLE No Buffer in a state of "CY_ETHIF_BUFFER_RELEASED"
*
*******************************************************************************/
uint32_t Cy_EthIf_FreeOneReleasedRxBuffer(uint32_t* pFreedRxBufIdx)
{
    for (uint32_t rxBufIdx = 0; rxBufIdx < CY_ETH_TOTAL_RX_BUF; rxBufIdx += 1ul)
    {
        if (cy_str_rxBufStatus[rxBufIdx].enBufStatus == CY_ETHIF_BUFFER_RELEASED)
        {
            cy_str_rxBufStatus[rxBufIdx].enBufStatus = CY_ETHIF_BUFFER_FREE;
            if(pFreedRxBufIdx != NULL)
            {
                *pFreedRxBufIdx = rxBufIdx;
            }
            return CY_ETHIF_BUFFER_AVAILABLE;
        }
    }
    return CY_ETHIF_NO_BUFFER_AVAILABLE;
}

/*******************************************************************************
* Function Name: Cy_EthIF_ClearReleasedBuf
****************************************************************************//**
*
* \brief Each buffer goes through cycle of FREE - OCCUPIED - RELEASED - FREE 
* Function looks into status of each buffer and reset them to default values if status is RELEASED
*
* \param bClearAll  True  - Free all buffers which have status "CY_ETHIF_BUFFER_RELEASED"
*                     False - Only first buffer found to be "CY_ETHIF_BUFFER_RELEASED" will be free
*        bTransmitBuf True  - Free buffer from Transmit pool
*                     False - Free buffer from Receive pool
*
* \return CY_ETHIF_BUFFER_AVAILABLE when Free all released buffers 
*         CY_ETHIF_NO_BUFFER_AVAILABLE No Buffer in a state of "CY_ETHIF_BUFFER_RELEASED"
*         when bClearALl == false, and if the buffer was freed, this returns the buffer index. 
*         
* \note  Return value of this function has mixed meaning that is "status" and "buffer index".
*        This is not good implementation. Using this function is not recommended.
*        Please use Cy_EthIf_FreeAllReleasedTxBuffer, Cy_EthIf_FreeOneReleasedTxBuffer,
*        Cy_EthIf_FreeAllReleasedRxBuffer, Cy_EthIf_FreeOneReleasedRxBuffer instead.
*
*******************************************************************************/
uint8_t Cy_EthIF_ClearReleasedBuf(bool bClearAll, bool bTransmitBuf)
{
    uint32_t status;
    uint32_t bufIdx;
    if (bTransmitBuf)
    {
        if (bClearAll)
        {
            // Clear all released TX Buffer
            status = Cy_EthIf_FreeAllReleasedTxBuffer();
        }
        else
        {
            // Clear one released TX Buffer
            status = Cy_EthIf_FreeOneReleasedTxBuffer(&bufIdx);
        }
    }
    else
    {
        if (bClearAll)
        {
            // Clear all released RX Buffer
            status = Cy_EthIf_FreeAllReleasedRxBuffer();
        }
        else
        {
            // Clear one released RX Buffer
            status = Cy_EthIf_FreeOneReleasedRxBuffer(&bufIdx);
        }
    }
    
    if (bClearAll)
    {
        // If clear all operation, return the status.
        return (uint8_t)status;
    }
    else
    {
        // If clear one buffer operation,
        // return status if no buffer was freed,
        // unless return the freed buffer index.
        if(status == CY_ETHIF_NO_BUFFER_AVAILABLE)
        {
            return (uint8_t)CY_ETHIF_NO_BUFFER_AVAILABLE;
        }
        else
        {
            return (uint8_t)bufIdx;
        }
    }
}

/*******************************************************************************
* Function Name: Cy_EthIf_TransmitFrame
********************************************************************************
*
* \brief Function initiates transmission based on passed arguments
*
* \param pstcEth Ethernet Instance
*        pu8TxBuffer pointer to Transmit source buffer
*        u16Length Length of data to transmit from source buffer, Length should include source
*                  and destination buffer address. CRC bytes shall not be included in the length
*        u8QueueIndex Queue to be used to transmit the frame
*        bEndBuffer True - Last buffer of the frame to be transmitted
*                   False - Other Buffers to be provided after function call
*                           IP will not start transmitting until it gets EndBuffer True
*
* \return CY_ETHIF_SUCCESS Frame transmission started
*         CY_ETHIF_BAD_PARAM Parameter passed contains invalid values
*         CY_ETHIF_BUFFER_NOT_AVAILABLE Buffer not available to load the source data
* 
* \note
* 
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_TransmitFrame (cy_pstc_eth_type_t pstcEth, uint8_t * pu8TxBuffer, uint16_t u16Length, uint8_t u8QueueIndex, bool bEndBuffer)
{
    /** check for arguments */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Check for arguments     */
    if ((pu8TxBuffer == NULL) || (u16Length > CY_ETH_SIZE_MAX_FRAME) || (u8QueueIndex > CY_ETH_QS3_3))
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Check requested queue enabled or not    */
    if (stcQueueDisStatus[ethIdx].bTxQueueDisable[u8QueueIndex] == true)
    {
        /** Requested Queue is disabled   */
        return CY_ETHIF_BAD_PARAM;
    } 

    /** Get available Tx Buffer from the Transmit buffer Pool   */
    uint32_t txBufIdx = 0;
    uint32_t bufStatus = Cy_EthIf_GetFreeTxBuffer(&txBufIdx);
    if (CY_ETHIF_NO_BUFFER_AVAILABLE == bufStatus)
    {
        /** check for first released buffer */
        bufStatus = Cy_EthIf_FreeOneReleasedTxBuffer(&txBufIdx);
        if (CY_ETHIF_NO_BUFFER_AVAILABLE == bufStatus)
        {
            return CY_ETHIF_BUFFER_NOT_AVAILABLE;     // No buffer available
        }
    }

    /** Load the buffer address of available Buffer */
    uint8_t* pu8LocalBuf = NULL;
    pu8LocalBuf = (uint8_t *)cy_str_txBufStatus[txBufIdx].cy_ethif_bufaddr.vAddr;

    /** change buffer's status to OCCUPIED, Buffer will be released in case of error or after Tx complete interrupt occurs  */
    cy_str_txBufStatus[txBufIdx].enBufStatus = CY_ETHIF_BUFFER_OCCUPIED;

    /* Copy data to un-cached aligned EthIf Tx buffer */
    memcpy((&pu8LocalBuf[0]), &pu8TxBuffer[0], u16Length);

    /** Clean Cache for the Tx Buffer area */
    Cy_SysLib_CleanCoreDCacheByAddr((uint32_t*)pu8LocalBuf, u16Length);

    /** Clear transmit status register before begin to transmit */
    cyp_ethif_gemgxlobj->clearTxStatus((void *)cyp_ethif_pd[ethIdx], CY_ETHIF_TX_STATUS_CLEAR);

    /** Typecast bEndBuffer to Flag type    */
    uint8_t u8flags = bEndBuffer ? CEDI_TXB_LAST_BUFF : 0;

    /** Trigger Internal transmit function  */
    uint32_t u32result;
    u32result = cyp_ethif_gemgxlobj->queueTxBuf((void *)cyp_ethif_pd[ethIdx],
                                                u8QueueIndex,
                                                &cy_str_txBufStatus[txBufIdx].cy_ethif_bufaddr,
                                                u16Length,
                                                u8flags);
    if (u32result != EOK)
    {
        /** error   */
        cy_str_txBufStatus[txBufIdx].enBufStatus = CY_ETHIF_BUFFER_RELEASED;
        return CY_ETHIF_BAD_PARAM;
    }

    return CY_ETHIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_EthIf_ProvideTxBuffer
********************************************************************************
*
* \brief Function provides a pointer to tx buffer
*
* \param bufIdxPtr output tx buffer index
*        bufPtr output top pointer to tx buffer
*
* \return CY_ETHIF_SUCCESS Frame transmission started
*         CY_ETHIF_BAD_PARAM Parameter passed contains invalid values
* 
* \note
* 
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_ProvideTxBuffer(uint8_t* bufIdxPtr, uint8_t** bufPtr)
{
    /************************/
    /***** Sanity Check *****/
    /************************/
    /** Check for arguments     */
    if ((bufIdxPtr == NULL) || (bufPtr == NULL))
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /***********************************/
    /***** Get Available Tx Buffer *****/
    /***********************************/
    /** Get available Tx Buffer index Pool */
    uint32_t freeTxBufferIdx;
    uint32_t bufStatus = Cy_EthIf_GetFreeTxBuffer(&freeTxBufferIdx);
    if (CY_ETHIF_NO_BUFFER_AVAILABLE == bufStatus)
    {
        /** check for first released buffer */
        bufStatus = Cy_EthIf_FreeOneReleasedTxBuffer(&freeTxBufferIdx);
        if (CY_ETHIF_NO_BUFFER_AVAILABLE == bufStatus)
        {
            return CY_ETHIF_BUFFER_NOT_AVAILABLE;     // No buffer available
        }
    }

    /** Load the buffer address and buffer index of available buffer */
    *bufIdxPtr = freeTxBufferIdx;
    uint8_t* pTopOfBuffer = (uint8_t*)cy_str_txBufStatus[freeTxBufferIdx].cy_ethif_bufaddr.vAddr;
    *bufPtr = &pTopOfBuffer[CY_ETH_HEADER_SIZE]; // destination address 6byte, source address 6byte, frame type = 2byte

    /** change buffer's status to OCCUPIED, Buffer will be released in case of error or after Tx complete interrupt occurs  */
    cy_str_txBufStatus[freeTxBufferIdx].enBufStatus = CY_ETHIF_BUFFER_OCCUPIED;

    return CY_ETHIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_EthIf_TransmitTrigger
********************************************************************************
*
* \brief Function triggers ethernet transmission
*
* \param pstcEth Ethernet Instance
*        bufIdx tx buffer index to be transmitted
*        payLoadLen payload length in byte
*        queueIndex queue index to be used
*        frameType ethernet frame type
*        physAddrPtr ethernet destination address
* 
* \return CY_ETHIF_SUCCESS Frame transmission started
*         CY_ETHIF_BAD_PARAM Parameter passed contains invalid values
* 
* \note Please call this function with "bufIdx" value
*       which Cy_EthIf_ProvideTxBuffer has output.
* 
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_TransmitTrigger(cy_pstc_eth_type_t pstcEth, uint8_t bufIdx, uint32_t payLoadLen, uint8_t queueIndex, uint16_t frameType, uint8_t* physAddrPtr)
{
    /************************/
    /***** Sanity Check *****/
    /************************/
    if(CY_ETH_TOTAL_TX_BUF <= bufIdx)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Check for arguments     */
    if (queueIndex > CY_ETH_QS3_3)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    if(cy_str_txBufStatus[bufIdx].enBufStatus != CY_ETHIF_BUFFER_OCCUPIED)
    {
        /* Call Cy_EthIf_ProvideTxBuffer before calling this function */
        return CY_ETHIF_BAD_PARAM;
    }

    /** Check requested queue enabled or not    */
    if (stcQueueDisStatus[ethIdx].bTxQueueDisable[queueIndex] == true)
    {
        /** Requested Queue is disabled   */
        return CY_ETHIF_BAD_PARAM;
    }

    /** Get static tx buffer  */
    uint8_t* pTxBuf = (uint8_t*)cy_str_txBufStatus[bufIdx].cy_ethif_bufaddr.vAddr;
    if(pTxBuf == NULL)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Get source MAC address   */
    uint8_t   pu8SourceMacAddr[6] = {0};
    Cy_EthIf_GetPhysAddr(pstcEth, &pu8SourceMacAddr[0]);

    /** Set ether header contents */
    const uint8_t destAddressOffset = 0u;
    const uint8_t srcAddressOffset  = 6u;
    const uint8_t frameTypeOffset   = 12u;
    memcpy(&pTxBuf[destAddressOffset],      physAddrPtr, 6);
    memcpy(&pTxBuf[srcAddressOffset],  pu8SourceMacAddr, 6);
    *(uint16_t*)&pTxBuf[frameTypeOffset] = ((frameType >> 8u) & 0x00FFu) | 
                                           ((frameType << 8u) & 0xFF00u); // Swapping because this is little endian machine

    /** Clear transmit status register before begin to transmit */
    cyp_ethif_gemgxlobj->clearTxStatus((void *)cyp_ethif_pd[ethIdx], CY_ETHIF_TX_STATUS_CLEAR);

    /** Clean Cache for the Tx Buffer area */
    Cy_SysLib_CleanCoreDCacheByAddr((uint32_t*)pTxBuf, (payLoadLen + CY_ETH_HEADER_SIZE));

    /** Trigger Internal transmit function  (emacQueueTxBuf)   */
    uint32_t u32result = cyp_ethif_gemgxlobj->queueTxBuf((void *)cyp_ethif_pd[ethIdx],
                                       queueIndex,
                                       &cy_str_txBufStatus[bufIdx].cy_ethif_bufaddr,
                                       payLoadLen + CY_ETH_HEADER_SIZE,
                                       CEDI_TXB_LAST_BUFF);
    if (u32result != 0)
    {
        /** error   */
        cy_str_txBufStatus[bufIdx].enBufStatus = CY_ETHIF_BUFFER_RELEASED;
        return CY_ETHIF_BAD_PARAM;
    }

    /* buffer status will be released in TX interrupt */

    return CY_ETHIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_EthIf_SetPhysAddr
********************************************************************************
*
* \brief Function sets MAC address for the interfaced Ethernet MAC
*
* \param uint8_t  pstcEth     Ethernet IP instance 
*        uint8_t* PhysAddrPtr Pointer to the address field from which addr is about to be copied
* 
* \return none
* 
* \note
* 
*******************************************************************************/
void Cy_EthIf_SetPhysAddr(cy_pstc_eth_type_t pstcEth, uint8_t * PhysAddrPtr)
{
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        /* invalid ether instance */
        CY_ASSERT(false); // should not reach here
    }

    for (uint8_t i = 0; i < 6; i++)
    {
        cyp_ethif_source_addr[ethIdx][i] = PhysAddrPtr[i];
    }
}

/*******************************************************************************
* Function Name: Cy_EthIf_GetPhysAddr
********************************************************************************
*
* \brief Function returns address of the buffer holding MAC address 
*
* \param uint8_t  CtrlIdx     Index of the Ethernet MAC
*        uint8_t* PhysAddrPtr out parameter to return address of the buffer
* 
* \return none
* 
* \note
* 
*******************************************************************************/
void Cy_EthIf_GetPhysAddr(cy_pstc_eth_type_t pstcEth, uint8_t * PhysAddrPtr)
{
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        /* invalid ether instance */
        CY_ASSERT(false); // should not reach here
    }

    for (uint8_t i = 0; i < 6; i++)
   {
        PhysAddrPtr[i] = cyp_ethif_source_addr[ethIdx][i];
   }
}

/*******************************************************************************
* Function Name: Cy_EthIf_TxPauseFrame
****************************************************************************//**
*
* \brief Transmits IEEE 802.3X standard Pause Frame 
*
* \param pstcEth  Ethernet Instance
*        bZeroTQ  True - Zero Time Quanta 
*        bZeroTQ  False - Transmit pause frame with set time quanta
*
* \return none
* \note: If interrupt enabled, interrupt will be triggered at the end of the transmission
*         
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_TxPauseFrame(cy_pstc_eth_type_t pstcEth, bool bZeroTQ)
{
    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    if (bZeroTQ == true)
    {
        /** trigger Pause frame with Zero Time Quanta */
        cyp_ethif_gemgxlobj->txZeroQPause((void *)cyp_ethif_pd[ethIdx]);
    }
    else
    {
        cyp_ethif_gemgxlobj->txPauseFrame((void *)cyp_ethif_pd[ethIdx]);
    }

    return CY_ETHIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_EthIf_SetNoBroadCast
****************************************************************************//**
*
* \brief
*
* \param
* 
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_SetNoBroadCast(cy_pstc_eth_type_t pstcEth, bool rejectBC)
{
    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    cyp_ethif_gemgxlobj->setNoBroadcast((void *)cyp_ethif_pd[ethIdx],rejectBC);
    
    return CY_ETHIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_EthIf_SetCopyAllFrames
****************************************************************************//**
*
* \brief 
*
* \param  
* 
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_SetCopyAllFrames(cy_pstc_eth_type_t pstcEth, bool toBeEnabled)
{
    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    cyp_ethif_gemgxlobj->setCopyAllFrames((void *)cyp_ethif_pd[ethIdx],toBeEnabled);
    
    return CY_ETHIF_SUCCESS;
}

/*******************************************************************************
* Function : Cy_EthIf_SetFilterAddress
****************************************************************************//**
*
* \brief This function set address to the filter specified by input index.
*
* \param pstcEth Ethernet Instance
*        cy_en_eth_filter_num_t filterNo Filter index to be set
*        cy_stc_eth_filter_config_t config filter configuration
* 
* \return cy_en_ethif_status_t
* 
* \note
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_SetFilterAddress(cy_pstc_eth_type_t pstcEth, cy_en_eth_filter_num_t filterNo, const cy_stc_eth_filter_config_t* config)
{
    /** check for arguments */
    if(filterNo >= CY_ETH_FILTER_NUM_INV)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    if(config == NULL)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /* add some address filters */
    if(cyp_ethif_gemgxlobj->setSpecificAddr((void *)cyp_ethif_pd[ethIdx],
                                        filterNo,
                                        (CEDI_MacAddress*)&config->filterAddr,
                                        config->typeFilter, 
                                        config->ignoreBytes) == EOK)
    {
        return CY_ETHIF_SUCCESS;
    }
    else
    {
        return CY_ETHIF_BAD_PARAM;
    }
}

/*******************************************************************************
* Function : Cy_EthIf_RemoveFilter
****************************************************************************//**
*
* \brief This function disables the filter specified by input index.
*
* \param pstcEth Ethernet Instance
*        cy_en_eth_filter_num_t filterNo Filter index to be disabled
* 
* \return cy_en_ethif_status_t
* 
* \note
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_RemoveFilter(cy_pstc_eth_type_t pstcEth, cy_en_eth_filter_num_t filterNo)
{
    /** check for arguments */
    if(filterNo >= CY_ETH_FILTER_NUM_INV)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /* add some address filters */
    if(cyp_ethif_gemgxlobj->disableSpecAddr((void *)cyp_ethif_pd[ethIdx], filterNo) == EOK)
    {
        return CY_ETHIF_SUCCESS;
    }
    else
    {
        return CY_ETHIF_BAD_PARAM;
    }

}

/*******************************************************************************
* Function : Cy_EthIf_RemoveFilterAddress
****************************************************************************//**
*
* \brief This function searches input address. If this function found the address,
*        it disables the filter.
*
* \param pstcEth Ethernet Instance
*        macAddr pointer to the memory which contains the MAC address
*                to be removed.
*        removedFilterIdx pointer to the memory to which
*                         this function will store the disabled filter index.
* 
* \return cy_en_ethif_status_t
* 
* \note
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_RemoveFilterAddress(cy_pstc_eth_type_t pstcEth, CEDI_MacAddress* macAddr, cy_en_eth_filter_num_t* removedFilterIdx)
{
    /** check for arguments */
    if(macAddr == NULL)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    const uint8_t filterIdxMax = CY_ETH_FILTER_NUM_INV-1;
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    bool fileterRemoved = false;
    for (cy_en_eth_filter_num_t filterIdx = CY_ETH_FILTER_NUM_1; filterIdx <= filterIdxMax; filterIdx++)
    {
        cy_stc_eth_filter_config_t getFilterConfg = {(cy_en_eth_filter_type_t)0};
        cyp_ethif_gemgxlobj->getSpecificAddr((void*)cyp_ethif_pd[ethIdx], filterIdx, &getFilterConfg.filterAddr, (uint8_t*)&getFilterConfg.typeFilter, &getFilterConfg.ignoreBytes);
        if(memcmp(macAddr,&getFilterConfg.filterAddr, 6) == 0)
        {
            Cy_EthIf_RemoveFilter(pstcEth, filterIdx);
            cyp_ethif_gemgxlobj->disableSpecAddr((void*)cyp_ethif_pd[ethIdx], filterIdx);
            fileterRemoved = true;
            *removedFilterIdx = filterIdx;
            break;
        }
    }

    if(fileterRemoved == true)
    {
        return CY_ETHIF_SUCCESS;
    }
    else
    {
        return CY_ETHIF_BAD_PARAM;
    }
    
}

/*******************************************************************************
* Function : Cy_EthIf_SetCorrectionTime
****************************************************************************//**
*
* \brief Adjust current Time stamp unit time by adjusting nanosecond field
*
* \param pstcEth Ethernet Instance
*        timeOffset time offset
*
* \return cy_en_ethif_status_t
* 
* \note
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_SetCorrectionTime(cy_pstc_eth_type_t pstcEth, int32_t timeOffset)
{
    /** check for arguments */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    if(cyp_ethif_gemgxlobj->adjust1588Timer((void*)cyp_ethif_pd[ethIdx], timeOffset)== EOK)
    {
        return CY_ETHIF_SUCCESS;
    }
    else
    {
        return CY_ETHIF_BAD_PARAM;
    }
}

/*******************************************************************************
* Function : Cy_EthIf_GetIngressTimestamp
****************************************************************************//**
*
* \brief Get PTP frame ingress time stamp value.
*
* \param pstcEth Ethernet Instance
*        pstcRetTmrValue ptp ingress time stamp value
*        eventType event type (primary or peer)
*
* \return cy_en_ethif_status_t
* 
* \note
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_GetIngressTimestamp(cy_pstc_eth_type_t pstcEth, CEDI_1588TimerVal* pstcRetTmrValue, cy_en_ethif_ptp_event_type_t eventType)
{
    /** check for arguments */
    if(pstcRetTmrValue == NULL)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    uint32_t status;
    if(eventType == CY_ETHIF_PTP_EVENT_PRIMARY)
    {
        status = cyp_ethif_gemgxlobj->getPtpFrameRxTime((void*)cyp_ethif_pd[ethIdx], pstcRetTmrValue);
    }
    else // eventType == CY_ETHIF_PTP_EVENT_PEER
    {
        status = cyp_ethif_gemgxlobj->getPtpPeerFrameRxTime((void*)cyp_ethif_pd[ethIdx], pstcRetTmrValue);
    }

    if(status == EOK)
    {
        return CY_ETHIF_SUCCESS;
    }
    else
    {
        return CY_ETHIF_BAD_PARAM;
    }
}

/*******************************************************************************
* Function : Cy_EthIf_GetEgressTimestamp
****************************************************************************//**
*
* \brief Get PTP frame egress time stamp value.
*
* \param pstcEth Ethernet Instance
*        pstcRetTmrValue ptp egress time stamp value
*        eventType event type (primary or peer)
*
* \return cy_en_ethif_status_t
* 
* \note
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_GetEgressTimestamp(cy_pstc_eth_type_t pstcEth, CEDI_1588TimerVal* pstcRetTmrValue, cy_en_ethif_ptp_event_type_t eventType)
{
    /** check for arguments */
    if(pstcRetTmrValue == NULL)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    uint32_t status = CY_ETHIF_BAD_PARAM;
    if(eventType == CY_ETHIF_PTP_EVENT_PRIMARY)
    {
        status = cyp_ethif_gemgxlobj->getPtpFrameTxTime((void*)cyp_ethif_pd[ethIdx], pstcRetTmrValue);
    }
    else // eventType == CY_ETHIF_PTP_EVENT_PEER
    {
        status = cyp_ethif_gemgxlobj->getPtpPeerFrameTxTime((void*)cyp_ethif_pd[ethIdx], pstcRetTmrValue);
    }

    if(status == EOK)
    {
        return CY_ETHIF_SUCCESS;
    }
    else
    {
        return CY_ETHIF_BAD_PARAM;
    }
}

/*******************************************************************************
* Function Name: Cy_EthIf_SetTimerCompValue
****************************************************************************//**
*
* \brief Setting the timer compare value in TSU register
*
* \param pstcEth Ethernet Instance
* \param pstcTmrValue pointer to data structure to configure register with
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_SetTimerCompValue(cy_pstc_eth_type_t pstcEth, CEDI_1588TimerVal* pstcTmrValue)
{
    /** check for arguments */
    if(pstcTmrValue == NULL)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    CEDI_TsuTimerVal compTimeValue;
    compTimeValue.nanosecs = (pstcTmrValue->nanosecs >> 8ul); // upper 22 bit of nano sec -> bits[29:8] of nano sec register.
    compTimeValue.secsLower = pstcTmrValue->secsLower;
    compTimeValue.secsUpper = pstcTmrValue->secsUpper;
    if(EOK != cyp_ethif_gemgxlobj->setTsuTimerCompVal((void*)cyp_ethif_pd[ethIdx], &compTimeValue))
    {
        return CY_ETHIF_BAD_PARAM;
    }

    return CY_ETHIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_EthIf_GetAndClearStats
****************************************************************************//**
*
* \brief Read stats from registers (which implicitly clears them) into struct
*
* \param pstcEth Ethernet Instance
* \param pstcStats Pointer to caller's stats variable
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_GetAndClearStats(cy_pstc_eth_type_t pstcEth, CEDI_Statistics* pstcStats)
{
    /** check for arguments */
    if(pstcStats == NULL)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    if(EOK != cyp_ethif_gemgxlobj->readStats((void*)cyp_ethif_pd[ethIdx]))
    {
        return CY_ETHIF_BAD_PARAM;
    }

    *pstcStats = cyp_ethif_statistic[ethIdx]; // Copy stats from driver to caller

    return CY_ETHIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_EthIf_ClearStats
****************************************************************************//**
*
* \brief Clear Stats
*
* \param pstcEth Ethernet Instance
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_ClearStats(cy_pstc_eth_type_t pstcEth)
{
    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    if(EOK != cyp_ethif_gemgxlobj->clearStats((void*)cyp_ethif_pd[ethIdx]))
    {
        return CY_ETHIF_BAD_PARAM;
    }

    return CY_ETHIF_SUCCESS;
}



/*******************************************************************************
* Function Name: Cy_EthIf_SetPauseEnable
****************************************************************************//**
*
* \brief Enable/Disable pause support
*
* \param pstcEth Ethernet Instance
* \param enable  Enable/Disable pause support
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_SetPauseEnable(cy_pstc_eth_type_t pstcEth, bool enable)
{
    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    cyp_ethif_gemgxlobj->setPauseEnable((void*)cyp_ethif_pd[ethIdx], enable ? 1 : 0);
    return CY_ETHIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_EthIf_SetPfcPriorityBasedPauseEnable
****************************************************************************//**
*
* \brief Enable/Disable pause support
*
* \param pstcEth Ethernet Instance
* \param enable  Enable/Disable pause support
*
*******************************************************************************/
cy_en_ethif_status_t Cy_EthIf_SetPfcPriorityBasedPauseEnable(cy_pstc_eth_type_t pstcEth, bool enable)
{
    /** Load the Ethernet instance */
    uint32_t ethIdx;
    if(Cy_EthIf_GetEtherInstanceIndex(pstcEth, &ethIdx) == false)
    {
        return CY_ETHIF_BAD_PARAM;
    }

    cyp_ethif_gemgxlobj->setPfcPriorityBasedPauseRx((void*)cyp_ethif_pd[ethIdx], enable ? 1 : 0);    
    return CY_ETHIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_EthIf_GetEthInstanceByIdx
****************************************************************************//**
*
* \brief Return ethernet IP instance by input index.
*
* \param ethIdx The ethernet IP index
*
* \param pEth pointer to a variable which saves ether instance if it is not NULL
*
* \return true if input ether exists, false otherwise.
*
*******************************************************************************/
static bool Cy_EthIf_GetEthInstanceByIdx(uint32_t ethIdx, cy_pstc_eth_type_t* pEth)
{
    if(CY_ETH_DEFINE_NUM_IP < ethIdx)
    {
        return false;
    }
    else
    {
        if(pEth != NULL)
        {
            *pEth = (cy_pstc_eth_type_t)(CY_ETH_ADDR_REG_BASE + CY_ETH_ADDR_REG_OFFSET * ethIdx);
        }
        return true;
    }
}

/*******************************************************************************
* Function Name: Cy_EthIf_GetEtherInstanceIndex
****************************************************************************//**
*
* \brief Return ethernet IP index of input instance.
*
* \param pstcEth Ethernet instance
*
* \param pEthIdx pointer to a variable which saves ethernet index if it is not NULL
*
* \return true if input ether exists, false otherwise.
*
*******************************************************************************/
static bool Cy_EthIf_GetEtherInstanceIndex(cy_pstc_eth_type_t eth, uint32_t* pEthIdx)
{
    for(uint32_t i = 0ul; i < CY_ETH_DEFINE_NUM_IP; i+=1ul)
    {
        if((uint32_t)eth == (CY_ETH_ADDR_REG_BASE + CY_ETH_ADDR_REG_OFFSET*i))
        {
            if(pEthIdx != NULL)
            {
                *pEthIdx = i;
            }
            return true;
        }
    }

    /* invalid input */
    return false;
}

/*******************************************************************************
* Function Name: Cy_EthIf_GetPrivateDataIndex
****************************************************************************//**
*
* \brief Return ethernet driver private data structure index by a pointer.
*        This index is corresponding to ethernet IP index.
*
* \param pPd Pointer to the private data.
*
* \param pEthIdx pointer to a variable which saves ethernet index if it is not NULL
*
* \return true if input ether exists, false otherwise.
*
*******************************************************************************/
static uint32_t Cy_EthIf_GetPrivateDataIndex(CEDI_PrivateData* pPd, uint32_t* pEthIdx)
{
    for(uint32_t i = 0ul; i < CY_ETH_DEFINE_NUM_IP; i+=1ul)
    {
        if(pPd == cyp_ethif_pd[i])
        {
            if(pEthIdx != NULL)
            {
                *pEthIdx = i;
            }
            return true;
        }
    }
    return false;
}

#if defined(__cplusplus)
}
#endif

/* [] END OF FILE */
