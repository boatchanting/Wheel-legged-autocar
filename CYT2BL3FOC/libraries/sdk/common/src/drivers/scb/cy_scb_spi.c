/***************************************************************************//**
* \file cy_scb_spi.c
* \version 1.0
*
* Provides SPI API implementation of the SCB driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_scb_spi.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Static functions */
static void HandleTransmit(volatile stc_SCB_t *base, cy_stc_scb_spi_context_t *context);
static void HandleReceive (volatile stc_SCB_t *base, cy_stc_scb_spi_context_t *context);
static void DiscardArrayNoCheck(volatile stc_SCB_t const *base, uint32_t size);

/*******************************************************************************
* Function Name: Cy_SCB_SPI_Init
****************************************************************************//**
*
* Initializes the SCB for SPI operation.
*
* \param base
* The pointer to the SPI SCB instance.
*
* \param config
* The pointer to the configuration structure \ref cy_stc_scb_spi_config_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_spi_context_t allocated
* by the user. The structure is used during the SPI operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
* If only SPI functions which do not require context will be used pass NULL
* as pointer to context.
*
* \return
* \ref cy_en_scb_spi_status_t
*
* \note
* Ensure that the SCB block is disabled before calling this function.
*
*******************************************************************************/
cy_en_scb_spi_status_t Cy_SCB_SPI_Init(volatile stc_SCB_t *base, cy_stc_scb_spi_config_t const *config, cy_stc_scb_spi_context_t *context)
{
    cy_en_scb_spi_status_t retStatus = CY_SCB_SPI_BAD_PARAM;
    un_SCB_CTRL_t     tscbCtrl       = { 0ul };
    un_SCB_SPI_CTRL_t tscbSpiCtrl    = { 0ul };
    un_SCB_TX_CTRL_t  tscbTxCtrl     = { 0ul };
    un_SCB_RX_CTRL_t  tscbRxCtrl     = { 0ul };
    uint32_t          locSclkMode    = 0ul;
    uint32_t          maxOfDataWidth = 0ul;

    if ((NULL != base) && (NULL != config))
    {
        /* Set SCLK mode for TI - CY_SCB_SPI_CPHA1_CPOL0, NS - CY_SCB_SPI_CPHA0_CPOL0, Motorola - take from config */
        if(CY_SCB_SPI_MOTOROLA == config->subMode)
        {
            locSclkMode = config->sclkMode;
        }
        else if(CY_SCB_SPI_NATIONAL == config->subMode)
        {
            locSclkMode = CY_SCB_SPI_CPHA0_CPOL0;
        }
        else
        {
            locSclkMode = CY_SCB_SPI_CPHA1_CPOL0;
        }

        maxOfDataWidth = (config->rxDataWidth >= config->txDataWidth) ? config->rxDataWidth : config->txDataWidth;
        if ( maxOfDataWidth <= CY_SCB_BYTE_WIDTH )
        {
            tscbCtrl.stcField.u2MEM_WIDTH = CY_SCB_SPI_MEM_WIDTH_BYTE;
        }
        else if ( maxOfDataWidth <= CY_SCB_HALFWORD_WIDTH )
        {
            tscbCtrl.stcField.u2MEM_WIDTH = CY_SCB_SPI_MEM_WIDTH_HALFWORD;
        }
        else if ( maxOfDataWidth <= CY_SCB_WORD_WIDTH )
        {
            tscbCtrl.stcField.u2MEM_WIDTH = CY_SCB_SPI_MEM_WIDTH_WORD;
        }
        else
        {
            return CY_SCB_SPI_BAD_PARAM;
        }

        if ( config->enableWakeFromSleep )
        {
            tscbCtrl.stcField.u1EC_AM_MODE = true;
        }
        else
        {
            tscbCtrl.stcField.u1EC_AM_MODE = false;
        }
        tscbCtrl.stcField.u4OVS = (config->oversample - 1ul);
        tscbCtrl.stcField.u2MODE = CY_SCB_CTRL_MODE_SPI;
        base->unCTRL.u32Register = tscbCtrl.u32Register;
     
        tscbSpiCtrl.stcField.u1SSEL_CONTINUOUS = config->enableTransferSeperation ? 0 : 1;
        tscbSpiCtrl.stcField.u1SELECT_PRECEDE = (0ul != (CY_SCB_SPI_TI_PRECEDE & config->subMode) ? 1ul : 0ul);
        tscbSpiCtrl.stcField.u1LATE_MISO_SAMPLE = config->enableMisoLateSample;
        tscbSpiCtrl.stcField.u1SCLK_CONTINUOUS = config->enableFreeRunSclk;
        tscbSpiCtrl.stcField.u1MASTER_MODE = ((CY_SCB_SPI_MASTER == config->spiMode) ? 1ul : 0ul);
        tscbSpiCtrl.stcField.u1CPHA = ((locSclkMode >> 1ul) & 0x01ul);
        tscbSpiCtrl.stcField.u1CPOL = (locSclkMode & 0x01ul);
        tscbSpiCtrl.stcField.u1SSEL_POLARITY0 = config->ssPolarity0;
        tscbSpiCtrl.stcField.u1SSEL_POLARITY1 = config->ssPolarity1;
        tscbSpiCtrl.stcField.u1SSEL_POLARITY2 = config->ssPolarity2;
        tscbSpiCtrl.stcField.u1SSEL_POLARITY3 = config->ssPolarity3;
        tscbSpiCtrl.stcField.u2MODE = config->subMode;
        base->unSPI_CTRL.u32Register = tscbSpiCtrl.u32Register;

        tscbRxCtrl.stcField.u1MSB_FIRST = config->enableMsbFirst;
        tscbRxCtrl.stcField.u1MEDIAN = config->enableInputFilter;
        tscbRxCtrl.stcField.u5DATA_WIDTH = (config->rxDataWidth - 1ul);
        base->unRX_CTRL.u32Register = tscbRxCtrl.u32Register;
        
        base->unRX_FIFO_CTRL.stcField.u8TRIGGER_LEVEL = config->rxFifoTriggerLevel;

        tscbTxCtrl.stcField.u1MSB_FIRST = config->enableMsbFirst;
        tscbTxCtrl.stcField.u5DATA_WIDTH = (config->txDataWidth - 1ul);
        base->unTX_CTRL.u32Register = tscbTxCtrl.u32Register;

        base->unTX_FIFO_CTRL.stcField.u8TRIGGER_LEVEL = config->txFifoTriggerLevel;

        /* Set up interrupt sources */
        base->unINTR_TX_MASK.u32Register = config->txFifoIntEnableMask;
        base->unINTR_RX_MASK.u32Register = config->rxFifoIntEnableMask;
        base->unINTR_M.stcField.u1SPI_DONE = config->enableSpiDoneInterrupt;
        base->unINTR_S.stcField.u1SPI_BUS_ERROR = config->enableSpiBusErrorInterrupt;
        base->unINTR_SPI_EC_MASK.u32Register = 0ul;

        /* Initialize the context */
        if (NULL != context)
        {
            context->status    = 0ul;
            context->txBufIdx  = 0ul;
            context->rxBufIdx  = 0ul;
            context->cbEvents = NULL;

        #if !defined(NDEBUG)
            /* Put an initialization key into the initKey variable to verify
            * context initialization in the transfer API.
            */
            context->initKey = CY_SCB_SPI_INIT_KEY;
        #endif /* !(NDEBUG) */
        }

        retStatus = CY_SCB_SPI_SUCCESS;
    }

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_SPI_DeInit
****************************************************************************//**
*
* De-initializes the SCB block, returns the register values to default.
*
* \param base
* The pointer to the SPI SCB instance.
*
* \note
* Ensure that the SCB block is disabled before calling this function.
*
*******************************************************************************/
void Cy_SCB_SPI_DeInit(volatile stc_SCB_t *base)
{
    /* SPI interface */
    base->unCTRL.u32Register              = CY_SCB_CTRL_DEF_VAL;
    base->unSPI_CTRL.u32Register          = CY_SCB_SPI_CTRL_DEF_VAL;

    /* RX direction */
    base->unRX_CTRL.u32Register           = CY_SCB_RX_CTRL_DEF_VAL;
    base->unRX_FIFO_CTRL.u32Register      = 0ul;

    /* TX direction */
    base->unTX_CTRL.u32Register           = CY_SCB_TX_CTRL_DEF_VAL;
    base->unTX_FIFO_CTRL.u32Register      = 0ul;

    /* Disable all interrupt sources */
    base->unINTR_SPI_EC_MASK.u32Register  = 0ul;
    base->unINTR_I2C_EC_MASK.u32Register  = 0ul;
    base->unINTR_RX_MASK.u32Register      = 0ul;
    base->unINTR_TX_MASK.u32Register      = 0ul;
    base->unINTR_M_MASK.u32Register       = 0ul;
    base->unINTR_S_MASK.u32Register       = 0ul;
}


/*******************************************************************************
* Function Name: Cy_SCB_SPI_Disable
****************************************************************************//**
*
* Disables the SCB block, clears context statuses and disables
* TX and RX interrupt sources.
* Note that after the block is disabled the TX and RX FIFOs and
* hardware statuses are cleared. Also, the hardware stops driving the output
* and ignores the input.
*
* \param base
* The pointer to the SPI SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_spi_context_t allocated
* by the user. The structure is used during the SPI operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
* If only SPI functions which do not require context will be used pass NULL
* as pointer to context.
*
* \note
* Calling this function when the SPI is busy (master preforms data transfer or
* slave communicates with the master) may cause transfer corruption because the
* hardware stops driving the outputs and ignores the inputs.
* It is recommenced to ensure that the SPI is not busy before calling this
* function.
*
*******************************************************************************/
void Cy_SCB_SPI_Disable(volatile stc_SCB_t *base, cy_stc_scb_spi_context_t *context)
{
    base->unCTRL.u32Register = 0ul;

    if (NULL != context)
    {
        context->status    = 0ul;
        context->rxBufIdx  = 0ul;
        context->txBufIdx  = 0ul;
    }

    /* Disable RX and TX interrupt sources for the slave because
    * RX overflow and TX underflow are kept enabled after the 1st call of
    * Cy_SCB_SPI_Transfer().
    */
    if ( !base->unSPI_CTRL.stcField.u1MASTER_MODE )
    {
        Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);
        Cy_SCB_SetTxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);
    }
}


/*******************************************************************************
* Function Name: Cy_SPI_DeepSleepCallback
****************************************************************************//**
*
* This function handles the transition of the SCB SPI into and out of 
* Deep-Sleep mode. It prevents the device from entering Deep-Sleep if the 
* SPI slave or master is actively communicating, or there is any data in the 
* TX or RX FIFOs. 
* The following behavior of the SPI SCB depends on if the SCB block is 
* wakeup-capable or not:
* * The SCB wakeup-capable: any transfer intended to the slave wakes up the 
*   device from Deep-Sleep mode. The slave responds with 0xFF to the transfer.
*   If the transfer occurs before the device enters Deep-Sleep mode, the device 
*   will not enter Deep-Sleep mode and incoming data is stored in the RX FIFO, 
*   otherwise incoming data is ignored.
*   Only the SPI slave can be configured to be a wakeup source from Deep-Sleep 
*   mode.
* * The SCB is not wakeup-capable: the SPI is disabled. It is enabled when the 
*   device failed to enter Deep-Sleep mode or it is awaken from Deep-Sleep mode. 
*   During the SPI is disabled, it stops driving the outputs and ignores the 
*   inputs. Any incoming data is ignored.
* 
* This function can be called during execution of \ref Cy_SysPm_DeepSleep,
* to do it, register this function as a callback before calling 
* \ref Cy_SysPm_DeepSleep : specify \ref CY_SYSPM_DEEPSLEEP as the callback 
* type and call \ref Cy_SysPm_RegisterCallback.
*
* \param base
* The pointer to the SPI SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_spi_context_t allocated
* by the user. The structure is used during the SPI operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure
*
* \param mode
* \ref cy_en_syspm_callback_mode_t
*
* \return
* \ref cy_en_syspm_status_t
*
*******************************************************************************/
cy_en_syspm_status_t Cy_SPI_DeepSleepCallback(void *base, void *context, cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t retStatus = CY_SYSPM_SUCCESS;

    volatile stc_SCB_t *locBase = (volatile stc_SCB_t *) base;
    cy_stc_scb_spi_context_t *locContext = (cy_stc_scb_spi_context_t *) context;

    switch(mode)
    {
    case CY_SYSPM_CHECK_READY:
    {
        /* Check if the High-level API is not busy executing the transfer 
        * operation.
        */
        if(0ul != (CY_SCB_SPI_TRANSFER_ACTIVE & Cy_SCB_SPI_GetTransferStatus(locBase, locContext)))
        {
            retStatus = CY_SYSPM_FAIL;
        }

        if(CY_SYSPM_FAIL != retStatus)
        {
            retStatus = CY_SYSPM_FAIL;

            /* If the SPI bus is not busy, all data elements are transferred 
            * on the bus from the TX FIFO and shifter and the RX FIFOs are
            * empty - the SPI is ready to enter Deep-Sleep mode.
            */
            if(!Cy_SCB_SPI_IsBusBusy(locBase))
            {
                if(Cy_SCB_SPI_IsTxComplete(locBase))
                {
                    if(0ul == Cy_SCB_SPI_GetNumInRxFifo(locBase))
                    {
                        // if (_FLD2BOOL(SCB_CTRL_EC_AM_MODE, locBase->CTRL))
                        if(locBase->unCTRL.stcField.u1EC_AM_MODE)
                        {
                            /* The SCB is wakeup-capable: clear the SPI 
                            * wakeup interrupt source because it triggers 
                            * during Activemode communication and make 
                            * sure that a new transfer is not started after 
                            * clearing.
                            */
                            Cy_SCB_ClearSpiInterrupt(locBase, CY_SCB_SPI_INTR_WAKEUP);

                            if(!Cy_SCB_SPI_IsBusBusy(locBase))
                            {
                                retStatus = CY_SYSPM_SUCCESS;
                            }
                        }
                        else
                        {
                            /* The SCB is NOT wakeup-capable: disable the 
                            * SPI. The master and slave stop driving the 
                            * bus until the SPI is enabled. This happens 
                            * when the device failed to enter Deep-Sleep  
                            * mode or it is awaken from Deep-Sleep mode.
                            */
                            Cy_SCB_SPI_Disable(locBase, locContext);

                            retStatus = CY_SYSPM_SUCCESS;
                        }
                    }
                }
            }
        }
    }
        break;

    case CY_SYSPM_CHECK_FAIL:
    {
        /* The other driver is not ready for Deep-Sleep mode. Restore 
        * Active mode configuration.
        */

        // if (!_FLD2BOOL(SCB_CTRL_EC_AM_MODE, locBase->CTRL))
        if(locBase->unCTRL.stcField.u1EC_AM_MODE)
        {
            /* The SCB is NOT wakeup-capable: enable the SPI to operate */
            Cy_SCB_SPI_Enable(locBase);
        }
    }
        break;

    case CY_SYSPM_BEFORE_ENTER:
    {
        /* This code executes inside the critical section and enabling the 
        * active interrupt source makes the interrupt pending in the NVIC.
        * However, the interrupt processing is delayed until the code exists 
        * the critical section. The pending interrupt force WFI instruction 
        * does nothing and the device remains in Active mode.
        */

        // if (_FLD2BOOL(SCB_CTRL_EC_AM_MODE, locBase->CTRL))
        if(locBase->unCTRL.stcField.u1EC_AM_MODE)
        {
            /* The SCB is wakeup-capable: enable the SPI wakeup interrupt 
            * source. If any transaction happened, the wakeup interrupt
            * becomes pending and prevents entering Deep-Sleep mode.
            */
            Cy_SCB_SetSpiInterruptMask(locBase, CY_SCB_I2C_INTR_WAKEUP);
        }
    }
        break;

    case CY_SYSPM_AFTER_EXIT:
    {
        // if (_FLD2BOOL(SCB_CTRL_EC_AM_MODE, locBase->CTRL))
        if(locBase->unCTRL.stcField.u1EC_AM_MODE)
        {
            /* The SCB is wakeup-capable: disable the SPI wakeup interrupt 
            * source 
            */
            Cy_SCB_SetSpiInterruptMask(locBase, CY_SCB_CLEAR_ALL_INTR_SRC);
        }
        else
        {
            /* The SCB is NOT wakeup-capable: enable the SPI to operate */
            Cy_SCB_SPI_Enable(locBase);
        }
    }
        break;

    default:
        retStatus = CY_SYSPM_FAIL;
        break;
    }

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SPI_HibernateCallback
****************************************************************************//**
*
* This function handles the transition of the SCB SPI into Hibernate mode. 
* It prevents the device from entering Hibernate mode if the SPI slave or master 
* is actively communicating, or there is any data in the TX or RX FIFOs. 
* If the SPI is ready to enter Hibernate mode, it is disabled. If the device 
* failed to enter Hibernate mode, the SPI is enabled. During the SPI is 
* disabled, it stops driving the outputs and ignores the inputs. 
* Any incoming data is ignored.
* 
* This function can be called during execution of \ref Cy_SysPm_Hibernate,
* to do it, register this function as a callback before calling 
* \ref Cy_SysPm_Hibernate : specify \ref CY_SYSPM_HIBERNATE as the callback 
* type and call \ref Cy_SysPm_RegisterCallback.
*
* \param base
* The pointer to the SPI SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_spi_context_t allocated
* by the user. The structure is used during the SPI operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure
*
* \param mode
* \ref cy_en_syspm_callback_mode_t
*
* \return
* \ref cy_en_syspm_status_t
*
*******************************************************************************/
cy_en_syspm_status_t Cy_SPI_HibernateCallback(void *base, void *context, cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t retStatus = CY_SYSPM_SUCCESS;

    volatile stc_SCB_t *locBase = (volatile stc_SCB_t *) base;
    cy_stc_scb_spi_context_t *locContext = (cy_stc_scb_spi_context_t *) context;

    switch(mode)
    {
    case CY_SYSPM_CHECK_READY:
    {
        /* Check if the High-level API is not busy executing the transfer 
        * operation.
        */
        if(0ul != (CY_SCB_SPI_TRANSFER_ACTIVE & Cy_SCB_SPI_GetTransferStatus(locBase, locContext)))
        {
            retStatus = CY_SYSPM_FAIL;
        }

        if(CY_SYSPM_FAIL != retStatus)
        {
            retStatus = CY_SYSPM_FAIL;

            /* If the SPI bus is not busy, all data elements are transferred 
            * on the bus from the TX FIFO and shifter and the RX FIFOs are
            * empty - the SPI is ready to enter Hibernate mode.
            */
            if(!Cy_SCB_SPI_IsBusBusy(locBase))
            {
                if(Cy_SCB_SPI_IsTxComplete(locBase))
                {
                    if(0ul == Cy_SCB_SPI_GetNumInRxFifo(locBase))
                    {
                        /* Disable the SPI. The master or slave stops 
                        * driving the bus until the SPI is enabled. 
                        * This happens if the device failed to enter 
                        * Hibernate mode.
                        */
                        Cy_SCB_SPI_Disable(locBase, locContext);

                        retStatus = CY_SYSPM_SUCCESS;
                    }
                }
            }
        }
    }
        break;

    case CY_SYSPM_CHECK_FAIL:
    {
        /* The other driver is not ready for Deep-Sleep mode. Restore Active
        * mode configuration.
        */

        /* Enable the SPI to operate */
        Cy_SCB_SPI_Enable(locBase);
    }
        break;

    case CY_SYSPM_BEFORE_ENTER:
    case CY_SYSPM_AFTER_EXIT:
        /* The SCB is not capable of waking up from Hibernate mode: 
        * do nothing.
        */
        break;

    default:
        retStatus = CY_SYSPM_FAIL;
        break;
    }

    return (retStatus);
}


/************************* High-Level Functions ********************************
* The following functions are considered high-level. They provide the layer of
* intelligence to the SCB. These functions require interrupts.
* Low-level and high-level functions must not be mixed because low-level API
* can adversely affect the operation of high-level functions.
*******************************************************************************/


/*******************************************************************************
* Function Name: Cy_SCB_SPI_Transfer
****************************************************************************//**
*
* This function starts an SPI transfer operation.
* It configures transmit and receive buffers for an SPI transfer.
* If the data that will be received is not important, pass NULL as rxBuf.
* If the data that will be transmitted is not important, pass NULL as txBuf
* and then the \ref CY_SCB_SPI_DEFAULT_TX is sent out as each data element.
*
* After the function configures TX and RX interrupt sources, it returns and
* \ref Cy_SCB_SPI_Interrupt manages further data transfer.
*
* * In the master mode, the transfer operation is started after calling this
*   function
* * In the slave mode, the transfer is registered and will be started when
*   the master request arrives.
*
* When the transfer operation is completed (requested number of data elements
* sent and received), the \ref CY_SCB_SPI_TRANSFER_ACTIVE status is cleared
* and the \ref CY_SCB_SPI_TRANSFER_CMPLT_EVENT event is generated.
*
* \param base
* The pointer to the SPI SCB instance.
*
* \param txBuf
* The pointer of the buffer with data to transmit.
* The item size is defined by the data type which depends on the configured
* TX data width.
*
* \param rxBuf
* The pointer to the buffer to store received data.
* The item size is defined by the data type which depends on the configured
* RX data width.
*
* \param size
* The number of data elements to transmit and receive.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_spi_context_t allocated
* by the user. The structure is used during the SPI operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_scb_spi_status_t
*
* \note
* * The buffers must not be modified and stay allocated until the end of the
*   transfer.
* * This function overrides all RX and TX FIFO interrupt sources and changes
*   the RX and TX FIFO level.
*
*******************************************************************************/
cy_en_scb_spi_status_t Cy_SCB_SPI_Transfer(volatile stc_SCB_t *base, void *txBuf, void *rxBuf, uint32_t size, cy_stc_scb_spi_context_t *context)
{
    cy_en_scb_spi_status_t retStatus = CY_SCB_SPI_TRANSFER_BUSY;

#if !defined(NDEBUG)
    /* Check that the initialization key was set before using the context */
    CY_ASSERT(NULL != context);
    CY_ASSERT(CY_SCB_SPI_INIT_KEY == context->initKey);
#endif /* !(NDEBUG) */

    /* Check if there are no active transfer requests */
    if (0ul == (CY_SCB_SPI_TRANSFER_ACTIVE & context->status))
    {
        uint32_t fifoSize = Cy_SCB_GetFifoSize(base);

        /* Set up the context */
        context->status    = CY_SCB_SPI_TRANSFER_ACTIVE;

        context->txBuf     = txBuf;
        context->txBufSize = size;
        context->txBufIdx =  0ul;

        context->rxBuf     = rxBuf;
        context->rxBufSize = size;
        context->rxBufIdx  = 0ul;

        /* Set the TX interrupt when half of FIFO was transmitted */
        Cy_SCB_SetTxFifoLevel(base, fifoSize / 2ul);

        if (base->unSPI_CTRL.stcField.u1MASTER_MODE)
        {
            /* Trigger an RX interrupt:
            * - If the transfer size is equal to or less than FIFO, trigger at the end of the transfer.
            * - If the transfer size is greater than FIFO, trigger 1 byte earlier than the TX interrupt.
            */
            Cy_SCB_SetRxFifoLevel(base, (size > fifoSize) ? ((fifoSize / 2ul) - 2ul) : (size - 1ul));

            Cy_SCB_SetMasterInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

            /* Enable interrupt sources to perform a transfer */
            Cy_SCB_SetRxInterruptMask(base, CY_SCB_RX_INTR_LEVEL);
            Cy_SCB_SetTxInterruptMask(base, CY_SCB_TX_INTR_LEVEL);
        }
        else
        {
            /* Trigger an RX interrupt:
            * - If the transfer size is equal to or less than half of FIFO, trigger ??at the end of the transfer.
            * - If the transfer size is greater than half of FIFO, trigger 1 byte earlier than a TX interrupt.
            */
            Cy_SCB_SetRxFifoLevel(base, (size > (fifoSize / 2ul)) ? ((fifoSize / 2ul) - 2ul) : (size - 1ul));

            Cy_SCB_SetSlaveInterruptMask(base, CY_SCB_SLAVE_INTR_SPI_BUS_ERROR);

            /* Enable interrupt sources to perform a transfer */
            Cy_SCB_SetRxInterruptMask(base, CY_SCB_RX_INTR_LEVEL | CY_SCB_RX_INTR_OVERFLOW);
            Cy_SCB_SetTxInterruptMask(base, CY_SCB_TX_INTR_LEVEL | CY_SCB_TX_INTR_UNDERFLOW);
        }

        retStatus = CY_SCB_SPI_SUCCESS;
    }

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_SPI_AbortTransfer
****************************************************************************//**
*
* Aborts the current SPI transfer.
* It disables the transmit and RX interrupt sources, clears the TX
* and RX FIFOs and the status.
*
* \param base
* The pointer to the SPI SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_spi_context_t allocated
* by the user. The structure is used during the SPI operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \note
* In the slave mode and after abort of transfer operation master continue
* sending data it gets into RX FIFO and TX FIFO is underflow as there is
* nothing to send. To drop this data, RX FIFO has to be cleared when
* the transfer is completed. Otherwise, received data will be kept and
* copied to the buffer when \ref Cy_SCB_SPI_Transfer is called.
*
* \sideeffect
* The transmit FIFO clear operation also clears the shift register, so that
* the shifter could be cleared in the middle of a data element transfer,
* corrupting it. The data element corruption means that all bits which has
* not been transmitted are transmitted as "ones" on the bus.
*
*******************************************************************************/
void Cy_SCB_SPI_AbortTransfer(volatile stc_SCB_t *base, cy_stc_scb_spi_context_t *context)
{
    /* Disable interrupt sources */
    Cy_SCB_SetSlaveInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

    if (base->unSPI_CTRL.stcField.u1MASTER_MODE)
    {
        Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);
        Cy_SCB_SetTxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);
    }
    else
    {
        Cy_SCB_SetRxInterruptMask(base, CY_SCB_RX_INTR_OVERFLOW);
        Cy_SCB_SetTxInterruptMask(base, CY_SCB_TX_INTR_UNDERFLOW);
    }

    /* Clear FIFOs */
    Cy_SCB_SPI_ClearTxFifo(base);
    Cy_SCB_SPI_ClearRxFifo(base);

    /* Clear the status to allow a new transfer */
    context->status = 0ul;
}


/*******************************************************************************
* Function Name: Cy_SCB_SPI_GetNumTransfered
****************************************************************************//**
*
* Returns the number of data elements transferred since the last call to \ref
* Cy_SCB_SPI_Transfer.
*
* \param base
* The pointer to the SPI SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_spi_context_t allocated
* by the user. The structure is used during the SPI operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* The number of data elements transferred.
*
*******************************************************************************/
uint32_t Cy_SCB_SPI_GetNumTransfered(volatile stc_SCB_t const *base, cy_stc_scb_spi_context_t const *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    return (context->rxBufIdx);
}


/*******************************************************************************
* Function Name: Cy_SCB_SPI_GetTransferStatus
****************************************************************************//**
*
* Returns the status of the transfer operation started by
* \ref Cy_SCB_SPI_Transfer.
* This status is a bit mask and the value returned may have a multiple-bit set.
*
* \param base
* The pointer to the SPI SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_spi_context_t allocated
* by the user. The structure is used during the SPI operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref group_scb_spi_macro_xfer_status.
*
* \note
* The status is cleared by calling \ref Cy_SCB_SPI_Transfer or
* \ref Cy_SCB_SPI_AbortTransfer.
*
*******************************************************************************/
uint32_t Cy_SCB_SPI_GetTransferStatus(volatile stc_SCB_t const *base, cy_stc_scb_spi_context_t const *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    return (context->status);
}


/*******************************************************************************
* Function Name: Cy_SCB_SPI_Interrupt
****************************************************************************//**
*
* This is the interrupt function for the SCB configured in the SPI mode.
* This function must be called inside the  user-defined interrupt service
* routine in order for \ref Cy_SCB_SPI_Transfer to work.
*
* \param base
* The pointer to the SPI SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_spi_context_t allocated
* by the user. The structure is used during the SPI operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
void Cy_SCB_SPI_Interrupt(volatile stc_SCB_t *base, cy_stc_scb_spi_context_t *context)
{
    en_transfer_error_t tTransferError;

    /* Wake up on the slave select condition */
    if( false != base->unINTR_SPI_EC_MASKED.stcField.u1WAKE_UP )
    {
        base->unINTR_SPI_EC.stcField.u1WAKE_UP = true;
        (void) base->unINTR_SPI_EC.u32Register;
    }

    /* The slave error condition */
    if( false != base->unINTR_S_MASKED.stcField.u1SPI_BUS_ERROR )
    {
        tTransferError = SpiTransferAllError;
        context->status |= SpiSlaveTransferError;

        base->unINTR_S.stcField.u1SPI_BUS_ERROR = false;
        (void) base->unINTR_S.u32Register;
    }

    /* The RX overflow error condition */
    if( false != base->unINTR_RX_MASKED.stcField.u1OVERFLOW )
    {
        tTransferError = SpiTransferAllError;
        context->status |= SpiTransferOverflow;

        base->unINTR_RX.stcField.u1OVERFLOW = false;
        (void) base->unINTR_RX.u32Register;
    }

    /* The TX underflow error condition or slave complete data transfer */
    if( false != base->unINTR_TX_MASKED.stcField.u1UNDERFLOW )
    {
        tTransferError = SpiTransferAllError;
        context->status |= SpiTransferUnderflow;

        base->unINTR_TX.stcField.u1UNDERFLOW = false;
        (void) base->unINTR_TX.u32Register;
    }

    /* Report an error, use a callback */
    if (0ul != tTransferError)
    {
        if (NULL != context->cbEvents)
        {
            context->cbEvents(tTransferError);
        }
    }

    /* RX direction */
    if ( false != base->unINTR_RX_MASKED.stcField.u1TRIGGER )
    {
        HandleReceive(base, context);

        Cy_SCB_ClearRxInterrupt(base, CY_SCB_RX_INTR_LEVEL);
    }

    /* TX direction */
    if ( false != base->unINTR_TX_MASKED.stcField.u1TRIGGER )
    {
        HandleTransmit(base, context);

        Cy_SCB_ClearTxInterrupt(base, CY_SCB_TX_INTR_LEVEL);
    }

    /* The transfer is complete: all data is loaded in the TX FIFO
    * and all data is read from the RX FIFO
    */
    if ((0ul != (context->status & CY_SCB_SPI_TRANSFER_ACTIVE)) &&
        (0ul == context->rxBufSize) && (0ul == context->txBufSize))
    {
        /* The transfer is complete */
        context->status &= (uint32_t) ~CY_SCB_SPI_TRANSFER_ACTIVE;

        if (NULL != context->cbEvents)
        {
            context->cbEvents(CY_SCB_SPI_TRANSFER_CMPLT_EVENT);
        }
    }
}


/*******************************************************************************
* Function Name: HandleReceive
****************************************************************************//**
*
* Reads data from RX FIFO into the buffer provided by \ref Cy_SCB_SPI_Transfer.
*
* \param base
* The pointer to the SPI SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_spi_context_t allocated
* by the user. The structure is used during the SPI operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void HandleReceive(volatile stc_SCB_t *base, cy_stc_scb_spi_context_t *context)
{
    /* Get data in RX FIFO */
    uint32_t numToCopy = Cy_SCB_GetNumInRxFifo(base);

    /* Adjust the number to read */
    if (numToCopy > context->rxBufSize)
    {
        numToCopy = context->rxBufSize;
    }

    /* Move the buffer */
    context->rxBufIdx  += numToCopy;
    context->rxBufSize -= numToCopy;

    /* Read data from RX FIFO */
    if (NULL != context->rxBuf)
    {
        uint8_t *buf = (uint8_t *) context->rxBuf;

        Cy_SCB_ReadArrayNoCheck(base, context->rxBuf, numToCopy);

        buf = &buf[(Cy_SCB_IsRxDataWidthByte(base) ? (numToCopy) : (2ul * numToCopy))];
        context->rxBuf = (void *) buf;
    }
    else
    {
        /* Discard read data. */
        DiscardArrayNoCheck(base, numToCopy);
    }

    if (0ul == context->rxBufSize)
    {
        /* Disable the RX level interrupt */
        base->unINTR_RX_MASK.stcField.u1TRIGGER = false;
    }
    else
    {
        uint32_t level = ( (base->unSPI_CTRL.stcField.u1MASTER_MODE) ?
                                    Cy_SCB_GetFifoSize(base) : (Cy_SCB_GetFifoSize(base) / 2ul) );

        if (context->rxBufSize < level)
        {
            Cy_SCB_SetRxFifoLevel(base, (context->rxBufSize - 1ul));
        }
    }
}


/*******************************************************************************
* Function Name: HandleTransmit
****************************************************************************//**
*
* Loads TX FIFO with data provided by \ref Cy_SCB_SPI_Transfer.
*
* \param base
* The pointer to the SPI SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_spi_context_t allocated
* by the user. The structure is used during the SPI operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void HandleTransmit(volatile stc_SCB_t *base, cy_stc_scb_spi_context_t *context)
{
    uint32_t numToCopy;
    uint32_t fifoSize = Cy_SCB_GetFifoSize(base);

    numToCopy = fifoSize - Cy_SCB_GetNumInTxFifo(base);

    /* Adjust the number to load */
    if (numToCopy > context->txBufSize)
    {
        numToCopy = context->txBufSize;
    }

    /* Move the buffer */
    context->txBufIdx  += numToCopy;
    context->txBufSize -= numToCopy;

    /* Load TX FIFO with data */
    if (NULL != context->txBuf)
    {
        uint8_t *buf = (uint8_t *) context->txBuf;

        Cy_SCB_WriteArrayNoCheck(base, context->txBuf, numToCopy);

        buf = &buf[(Cy_SCB_IsTxDataWidthByte(base) ? (numToCopy) : (2ul * numToCopy))];
        context->txBuf = (void *) buf;
    }
    else
    {
        Cy_SCB_WriteDefaultArrayNoCheck(base, CY_SCB_SPI_DEFAULT_TX, numToCopy);
    }

    if (0ul == context->txBufSize)
    {
        /* Data is transferred into TX FIFO */
        context->status |= CY_SCB_SPI_TRANSFER_IN_FIFO;

        /* Disable the TX level interrupt */
        base->unINTR_TX_MASK.stcField.u1TRIGGER = false;

        if (NULL != context->cbEvents)
        {
            context->cbEvents(CY_SCB_SPI_TRANSFER_IN_FIFO_EVENT);
        }
    }
}


/*******************************************************************************
* Function Name: DiscardArrayNoCheck
****************************************************************************//**
*
* Reads the number of data elements from the SPI RX FIFO. The read data is
* discarded. Before calling this function, make sure that RX FIFO has
* enough data elements to read.
*
* \param base
* The pointer to the SPI SCB instance.
*
* \param size
* The number of data elements to read.
*
*******************************************************************************/
static void DiscardArrayNoCheck(volatile stc_SCB_t const *base, uint32_t size)
{
    while (size > 0ul)
    {
        (void) Cy_SCB_SPI_Read(base);
        --size;
    }
}


#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */

