/***************************************************************************//**
* \file cy_scb_uart.c
* \version 1.0
*
* Provides UART API implementation of the SCB driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_scb_uart.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Static functions */
static void HandleDataReceive (volatile stc_SCB_t *base, cy_stc_scb_uart_context_t *context);
static void HandleRingBuffer  (volatile stc_SCB_t *base, cy_stc_scb_uart_context_t *context);
static void HandleDataTransmit(volatile stc_SCB_t *base, cy_stc_scb_uart_context_t *context);


/*******************************************************************************
* Function Name: Cy_SCB_UART_Init
****************************************************************************//**
*
* Initializes the SCB for UART operation.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param config
* The pointer to configuration structure \ref cy_stc_scb_uart_config_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
* If only UART functions which do not require context will be used pass NULL
* as pointer to context.
*
* \return
* \ref cy_en_scb_uart_status_t
*
* \note
* Ensure that the SCB block is disabled before calling this function.
*
*******************************************************************************/
cy_en_scb_uart_status_t Cy_SCB_UART_Init(volatile stc_SCB_t *base, cy_stc_scb_uart_config_t const *config, cy_stc_scb_uart_context_t *context)
{
    cy_en_scb_uart_status_t retStatus = CY_SCB_UART_BAD_PARAM;
    un_SCB_CTRL_t           temp_CTRL           = { 0ul };
    un_SCB_UART_CTRL_t      temp_UART_CTRL      = { 0ul };
    un_SCB_UART_RX_CTRL_t   temp_UART_RX_CTRL   = { 0ul };
    un_SCB_RX_CTRL_t        temp_RX_CTRL        = { 0ul };
    un_SCB_RX_MATCH_t       temp_RX_MATCH       = { 0ul };
    un_SCB_UART_TX_CTRL_t   temp_UART_TX_CTRL   = { 0ul };
    un_SCB_TX_CTRL_t        temp_TX_CTRL        = { 0ul };
    un_SCB_RX_FIFO_CTRL_t   temp_RX_FIFO_CTRL   = { 0ul };
    un_SCB_UART_FLOW_CTRL_t temp_UART_FLOW_CTRL = { 0ul };
    un_SCB_TX_FIFO_CTRL_t   temp_TX_FIFO_CTRL   = { 0ul };

    if ((NULL != base) && (NULL != config))
    {
        uint32_t ovs;

        if ((CY_SCB_UART_IRDA == config->uartMode) && (!config->irdaEnableLowPowerReceiver))
        {
            /* For Normal IrDA mode oversampling is always zero */
            ovs = 0ul;
        }
        else
        {
            ovs = (config->oversample - 1ul);
        }

        /* Configure the UART interface */
        temp_CTRL.stcField.u1ADDR_ACCEPT = (config->acceptAddrInFifo ? 1ul : 0ul);
        temp_CTRL.stcField.u2MEM_WIDTH   = 0ul;
        temp_CTRL.stcField.u4OVS         = ovs;
        temp_CTRL.stcField.u2MODE        = CY_SCB_CTRL_MODE_UART;
        base->unCTRL.u32Register         = temp_CTRL.u32Register;

        temp_UART_CTRL.stcField.u2MODE   = config->uartMode;
        base->unUART_CTRL.u32Register    = temp_UART_CTRL.u32Register;

        /* Configure the RX direction */
        temp_UART_RX_CTRL.stcField.u1POLARITY             = (config->irdaInvertRx ? 1ul : 0ul);
        temp_UART_RX_CTRL.stcField.u1MP_MODE              = (config->enableMutliProcessorMode ? 1ul : 0ul);
        temp_UART_RX_CTRL.stcField.u1DROP_ON_PARITY_ERROR = (config->dropOnParityError ? 1ul : 0ul);
        temp_UART_RX_CTRL.stcField.u1DROP_ON_FRAME_ERROR  = (config->dropOnFrameError ? 1ul : 0ul);
        temp_UART_RX_CTRL.stcField.u4BREAK_WIDTH          = (config->breakWidth - 1ul);
        temp_UART_RX_CTRL.stcField.u3STOP_BITS            = (config->stopBits - 1ul);
        temp_UART_RX_CTRL.stcField.u1PARITY               = (config->parity & 0x00000001ul);
        temp_UART_RX_CTRL.stcField.u1PARITY_ENABLED       = (config->parity & 0x00000002ul) >> 1;
        base->unUART_RX_CTRL.u32Register                  = temp_UART_RX_CTRL.u32Register;

        temp_RX_CTRL.stcField.u1MSB_FIRST  = (config->enableMsbFirst ? 1ul : 0ul);
        temp_RX_CTRL.stcField.u1MEDIAN     = (config->enableInputFilter ? 1ul : 0ul);
        temp_RX_CTRL.stcField.u5DATA_WIDTH = (config->dataWidth - 1ul);
        base->unRX_CTRL.u32Register        = temp_RX_CTRL.u32Register;

        temp_RX_MATCH.stcField.u8ADDR = config->receiverAddress;
        temp_RX_MATCH.stcField.u8MASK = config->receiverAddressMask;
        base->unRX_MATCH.u32Register  = temp_RX_MATCH.u32Register;

        /* Configure the TX direction */
        temp_UART_TX_CTRL.stcField.u1RETRY_ON_NACK  = (config->smartCardRetryOnNack ? 1ul : 0ul);
        temp_UART_TX_CTRL.stcField.u3STOP_BITS      = (config->stopBits - 1ul);
        temp_UART_TX_CTRL.stcField.u1PARITY         = (config->parity & 0x00000001ul);
        temp_UART_TX_CTRL.stcField.u1PARITY_ENABLED = (config->parity & 0x00000002ul) >> 1;
        base->unUART_TX_CTRL.u32Register            = temp_UART_TX_CTRL.u32Register;

        temp_TX_CTRL.stcField.u1MSB_FIRST  = (config->enableMsbFirst ? 1ul : 0ul);
        temp_TX_CTRL.stcField.u5DATA_WIDTH = (config->dataWidth - 1ul);
        temp_TX_CTRL.stcField.u1OPEN_DRAIN = ((config->uartMode == CY_SCB_UART_SMARTCARD) ? 1ul : 0ul);
        base->unTX_CTRL.u32Register        = temp_TX_CTRL.u32Register;

        temp_RX_FIFO_CTRL.stcField.u8TRIGGER_LEVEL = config->rxFifoTriggerLevel;
        base->unRX_FIFO_CTRL.u32Register           = temp_RX_FIFO_CTRL.u32Register;

        /* Configure the flow control */

        temp_UART_FLOW_CTRL.stcField.u1CTS_ENABLED   = (config->enableCts ? 1ul : 0ul);
        temp_UART_FLOW_CTRL.stcField.u1CTS_POLARITY  = ((CY_SCB_UART_ACTIVE_HIGH == config->ctsPolarity) ? 1ul : 0ul);
        temp_UART_FLOW_CTRL.stcField.u1RTS_POLARITY  = ((CY_SCB_UART_ACTIVE_HIGH == config->rtsPolarity) ? 1ul : 0ul);
        temp_UART_FLOW_CTRL.stcField.u8TRIGGER_LEVEL = config->rtsRxFifoLevel;
        base->unUART_FLOW_CTRL.u32Register           = temp_UART_FLOW_CTRL.u32Register;

        temp_TX_FIFO_CTRL.stcField.u8TRIGGER_LEVEL = config->txFifoTriggerLevel;
        base->unTX_FIFO_CTRL.u32Register           = temp_TX_FIFO_CTRL.u32Register;

        /* Set up interrupt sources */
        base->unINTR_TX_MASK.u32Register = config->txFifoIntEnableMask;
        base->unINTR_RX_MASK.u32Register = config->rxFifoIntEnableMask;

        /* Initialize context */
        if (NULL != context)
        {
            context->rxStatus  = 0ul;
            context->txStatus  = 0ul;

            context->rxRingBuf = NULL;
            context->rxRingBufSize = 0ul;

            context->rxBufIdx  = 0ul;
            context->txLeftToTransmit = 0ul;

            context->cbEvents = NULL;

        #if !defined(NDEBUG)
            /* Put an initialization key into the initKey variable to verify
            * context initialization in the transfer API.
            */
            context->initKey = CY_SCB_UART_INIT_KEY;
        #endif /* !(NDEBUG) */
        }

        retStatus = CY_SCB_UART_SUCCESS;
    }

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_UART_DeInit
****************************************************************************//**
*
* De-initializes the SCB block, returns the register values to default.
*
* \param base
* The pointer to the UART SCB instance.
*
* \note
* Ensure that the SCB block is disabled before calling this function.
*
*******************************************************************************/
void Cy_SCB_UART_DeInit(volatile stc_SCB_t *base)
{
    /* De-initialize the UART interface */
    base->unCTRL.u32Register      = CY_SCB_CTRL_DEF_VAL;
    base->unUART_CTRL.u32Register = CY_SCB_UART_CTRL_DEF_VAL;

    /* De-initialize the RX direction */
    base->unUART_RX_CTRL.u32Register = 0ul;
    base->unRX_CTRL.u32Register      = CY_SCB_RX_CTRL_DEF_VAL;
    base->unRX_FIFO_CTRL.u32Register = 0ul;
    base->unRX_MATCH.u32Register     = 0ul;

    /* De-initialize the TX direction */
    base->unUART_TX_CTRL.u32Register = 0ul;
    base->unTX_CTRL.u32Register      = CY_SCB_TX_CTRL_DEF_VAL;
    base->unTX_FIFO_CTRL.u32Register = 0ul;

    /* De-initialize the flow control */
    base->unUART_FLOW_CTRL.u32Register = 0ul;

    /* De-initialize the interrupt sources */
    base->unINTR_SPI_EC_MASK.u32Register = 0ul;
    base->unINTR_I2C_EC_MASK.u32Register = 0ul;
    base->unINTR_RX_MASK.u32Register     = 0ul;
    base->unINTR_TX_MASK.u32Register     = 0ul;
    base->unINTR_M_MASK.u32Register      = 0ul;
    base->unINTR_S_MASK.u32Register      = 0ul;
}


/*******************************************************************************
* Function Name: Cy_SCB_UART_Disable
****************************************************************************//**
*
* Disables the SCB block and clears context statuses.
* Note that after the block is disabled, the TX and RX FIFOs and
* hardware statuses are cleared. Also, the hardware stops driving the
* output and ignores the input.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
* If only UART functions which do not require context will be used pass NULL
* as pointer to context.
*
* \note
* Calling this function when the UART is busy (transmitter performs data
* transfer or receiver is in the middle of data reception) may result transfer
* corruption because the hardware stops driving the outputs and ignores
* the inputs.
* Ensure that the UART is not busy before calling this function.
*
*******************************************************************************/
void Cy_SCB_UART_Disable(volatile stc_SCB_t *base, cy_stc_scb_uart_context_t *context)
{
    base->unCTRL.stcField.u1ENABLED = 0ul;

    if (NULL != context)
    {
        context->rxStatus  = 0ul;
        context->txStatus  = 0ul;

        context->rxBufIdx  = 0ul;
        context->txLeftToTransmit = 0ul;
    }
}


/*******************************************************************************
* Function Name: Cy_UART_DeepSleepCallback
****************************************************************************//**
*
* This function handles the transition of the SCB UART into and out of 
* Deep-Sleep mode. It prevents the device from entering Deep-Sleep mode if 
* the UART is transmitting data or has any data in the RX FIFO. If the UART 
* is ready to enter Deep-Sleep mode, it is disabled. The UART is enabled 
* when the device failed to enter Deep-Sleep mode or it is awaken from 
* Deep-Sleep mode. During the UART is disabled, it stops driving the outputs 
* and ignores the inputs. Any incoming data is ignored.
* 
* This function can be called during execution of \ref Cy_SysPm_DeepSleep,
* to do it, register this function as a callback before calling 
* \ref Cy_SysPm_DeepSleep : specify \ref CY_SYSPM_DEEPSLEEP as the callback 
* type and call \ref Cy_SysPm_RegisterCallback.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \param mode
* \ref cy_en_syspm_callback_mode_t
*
* \return
* \ref cy_en_syspm_status_t
*
*******************************************************************************/
cy_en_syspm_status_t Cy_UART_DeepSleepCallback(void *base, void *context, cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t retStatus = CY_SYSPM_SUCCESS;

    volatile stc_SCB_t *locBase = (volatile stc_SCB_t *) base;
    cy_stc_scb_uart_context_t *locContext = (cy_stc_scb_uart_context_t *) context;

    switch(mode)
    {
        case CY_SYSPM_CHECK_READY:
        {
            /* Check if the High-level API is not busy executing the transmit 
            * or receive operation.
            */
            if ((0ul != (CY_SCB_UART_TRANSMIT_ACTIVE & Cy_SCB_UART_GetTransmitStatus(locBase, locContext))) ||
                (0ul != (CY_SCB_UART_RECEIVE_ACTIVE  & Cy_SCB_UART_GetReceiveStatus (locBase, locContext))))
            {
                retStatus = CY_SYSPM_FAIL;
            }

            if (CY_SYSPM_FAIL != retStatus)
            {
                /* If all data elements are transmitted from the TX FIFO and
                * shifter and the RX FIFO is empty: the UART is ready to enter
                * Deep-Sleep mode.
                */
                if (Cy_SCB_UART_IsTxComplete(locBase))
                {
                    if (0ul == Cy_SCB_UART_GetNumInRxFifo(locBase))
                    {
                        /* Disable the UART. The transmitter stops driving the 
                        * lines and the receiver stops receiving data until 
                        * the UART is enabled.
                        * This happens when the device failed to enter Deep 
                        * Sleep or it is awaken from Deep-Sleep mode.
                        */
                        Cy_SCB_UART_Disable(locBase, locContext);
                    }
                }
                else
                {
                    retStatus = CY_SYSPM_FAIL;
                }
            }
        }
        break;

        case CY_SYSPM_CHECK_FAIL:
        {
            /* The other driver is not ready for Deep-Sleep mode. Restore the 
            * Active mode configuration.
            */

            /* Enable the UART to operate */
            Cy_SCB_UART_Enable(locBase);
        }
        break;

        case CY_SYSPM_BEFORE_ENTER:
            /* Do noting: the UART is not capable of waking up from 
            * Deep-Sleep mode.
            */
        break;

        case CY_SYSPM_AFTER_EXIT:
        {
            /* Enable the UART to operate */
            Cy_SCB_UART_Enable(locBase);
        }
        break;

        default:
            retStatus = CY_SYSPM_FAIL;
            break;
    }

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_UART_HibernateCallback
****************************************************************************//**
*
* This function handles the transition of the SCB UART into Hibernate mode. 
* It prevents the device from entering Hibernate mode if the UART is 
* transmitting data or has any data in the RX FIFO. If the UART is ready 
* to enter Hibernate mode, it is disabled. If the device failed to enter 
* Hibernate mode, the UART is enabled. During the UART is disabled, it stops 
* driving the outputs and ignores the inputs. Any incoming data is ignored.
*
* This function can be called during execution of \ref Cy_SysPm_Hibernate,
* to do it, register this function as a callback before calling 
* \ref Cy_SysPm_Hibernate : specify \ref CY_SYSPM_HIBERNATE as the callback type
* and call \ref Cy_SysPm_RegisterCallback.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \param mode
* \ref cy_en_syspm_callback_mode_t
*
* \return
* \ref cy_en_syspm_status_t
*
*******************************************************************************/
cy_en_syspm_status_t Cy_UART_HibernateCallback(void *base, void *context, cy_en_syspm_callback_mode_t mode)
{
    return Cy_UART_DeepSleepCallback(base, context, mode);
}


/************************* High-Level Functions ********************************
* The following functions are considered high-level. They provide the layer of
* intelligence to the SCB. These functions require interrupts.
* Low-level and high-level functions must not be mixed because low-level API
* can adversely affect the operation of high-level functions.
*******************************************************************************/


/*******************************************************************************
* Function Name: Cy_SCB_UART_StartRingBuffer
****************************************************************************//**
*
* Starts the receive ring buffer operation.
* The RX interrupt source is configured to get data from the RX
* FIFO and put into the ring buffer.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param ringBuffer
* Pointer to the user defined ring buffer.
* The item size is defined by the data type which depends on the configured
* data width.
*
* \param size
* The size of the receive ring buffer.
* Note that one data element is used for internal use, so if the size is 32,
* then only 31 data elements are used for data storage.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \note
* * The buffer must not be modified and stay allocated while the ring buffer
*   operates.
* * This function overrides the RX interrupt sources and changes the
*   RX FIFO level.
*
*******************************************************************************/
void Cy_SCB_UART_StartRingBuffer(volatile stc_SCB_t *base, void *ringBuffer, uint32_t size, cy_stc_scb_uart_context_t *context)
{
#if !defined(NDEBUG)
    /* Check that the initialization key was set before using the context */
    CY_ASSERT(NULL != context);
    CY_ASSERT(CY_SCB_UART_INIT_KEY == context->initKey);
#endif /* !(NDEBUG) */

    if ((NULL != ringBuffer) && (size > 0ul))
    {
        uint32_t halfFifoSize =  (Cy_SCB_GetFifoSize(base) / 2ul);

        context->rxRingBuf     = ringBuffer;
        context->rxRingBufSize = size;
        context->rxRingBufHead = 0ul;
        context->rxRingBufTail = 0ul;

        /* Set up an RX interrupt to handle the ring buffer */
        Cy_SCB_SetRxFifoLevel(base, (size >= halfFifoSize) ? (halfFifoSize - 1ul) : (size - 1ul));

        Cy_SCB_SetRxInterruptMask(base, CY_SCB_RX_INTR_LEVEL);
    }
}


/*******************************************************************************
* Function Name: Cy_SCB_UART_StopRingBuffer
****************************************************************************//**
*
* Stops receiving data into the ring buffer, and clears the ring buffer.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
void Cy_SCB_UART_StopRingBuffer(volatile stc_SCB_t *base, cy_stc_scb_uart_context_t *context)
{
    Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);
    Cy_SCB_UART_ClearRingBuffer(base, context);

    context->rxRingBuf     = NULL;
    context->rxRingBufSize = 0ul;
}


/*******************************************************************************
* Function Name: Cy_SCB_UART_GetNumInRingBuffer
****************************************************************************//**
*
* Returns the number of data elements in the ring buffer.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* The number of data elements in the receive ring buffer.
*
* \note
* One data element is used for internal use, so when the buffer is full,
* this function returns (Ring Buffer size - 1).
*
*******************************************************************************/
uint32_t Cy_SCB_UART_GetNumInRingBuffer(volatile stc_SCB_t const *base, cy_stc_scb_uart_context_t const *context)
{
    uint32_t size;
    uint32_t locHead = context->rxRingBufHead;

    /* Suppress a compiler warning about unused variables */
    (void) base;

    if (locHead >= context->rxRingBufTail)
    {
        size = (locHead - context->rxRingBufTail);
    }
    else
    {
        size = (locHead + (context->rxRingBufSize - context->rxRingBufTail));
    }

    return (size);
}


/*******************************************************************************
* Function Name: Cy_SCB_UART_ClearRingBuffer
****************************************************************************//**
*
* Clears the ring buffer.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
void Cy_SCB_UART_ClearRingBuffer(volatile stc_SCB_t const *base, cy_stc_scb_uart_context_t *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    context->rxRingBufHead = context->rxRingBufTail;
}


/*******************************************************************************
* Function Name: Cy_SCB_UART_Receive
****************************************************************************//**
*
* This function starts a UART receive operation.
* It configures the receive interrupt sources to get data available in the
* receive FIFO and returns. The \ref Cy_SCB_UART_Interrupt manages the further
* data transfer.
*
* If the ring buffer is enabled, this function first reads data from the ring
* buffer. If there is more data to receive, it configures the receive interrupt
* sources to copy the remaining bytes from the RX FIFO when they arrive.
*
* When the receive operation is completed (requested number of data elements
* received) the \ref CY_SCB_UART_RECEIVE_ACTIVE status is cleared and
* the \ref CY_SCB_UART_RECEIVE_DONE_EVENT event is generated.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param rxBuf
* Pointer to buffer to store received data.
* The item size is defined by the data type which depends on the configured
* data width.
*
* \param size
* The number of data elements to receive.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_scb_uart_status_t
*
* \note
* * The buffer must not be modified and stay allocated until end of the
*   receive operation.
* * This function overrides the RX interrupt sources and changes the
*   RX FIFO level.
*
*******************************************************************************/
cy_en_scb_uart_status_t Cy_SCB_UART_Receive(volatile stc_SCB_t *base, void *rxBuf, uint32_t size, cy_stc_scb_uart_context_t *context)
{
    cy_en_scb_uart_status_t retStatus = CY_SCB_UART_RECEIVE_BUSY;

#if !defined(NDEBUG)
    /* Check that the initialization key was set before using the context */
    CY_ASSERT(NULL != context);
    CY_ASSERT(CY_SCB_UART_INIT_KEY == context->initKey);
#endif /* !(NDEBUG) */

    /* Check if there are no active transfer requests */
    if (0ul == (context->rxStatus & CY_SCB_UART_RECEIVE_ACTIVE))
    {
        uint8_t  *tmpBuf = (uint8_t *) rxBuf;
        uint32_t numToCopy = 0ul;

        /* Disable the RX interrupt source to stop the ring buffer update */
        Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

        if (NULL != context->rxRingBuf)
        {
            /* Get the items available in the ring buffer */
            numToCopy = Cy_SCB_UART_GetNumInRingBuffer(base, context);

            if (numToCopy > 0ul)
            {
                uint32_t idx;
                uint32_t locTail = context->rxRingBufTail;
                bool     byteMode = Cy_SCB_IsRxDataWidthByte(base);

                /* Adjust the number of items to be read */
                if (numToCopy > size)
                {
                    numToCopy = size;
                }

                /* Copy the data elements from the ring buffer */
                for (idx = 0ul; idx < numToCopy; ++idx)
                {
                    ++locTail;

                    if (locTail == context->rxRingBufSize)
                    {
                        locTail = 0ul;
                    }

                    if (byteMode)
                    {
                        uint8_t *buf = (uint8_t *) rxBuf;
                        buf[idx] = ((uint8_t *) context->rxRingBuf)[locTail];
                    }
                    else
                    {
                        uint16_t *buf = (uint16_t *) rxBuf;
                        buf[idx] = ((uint16_t *) context->rxRingBuf)[locTail];
                    }
                }

                /* Update the ring buffer tail after data has been copied */
                context->rxRingBufTail = locTail;

                /* Update with the copied bytes */
                size -= numToCopy;
                context->rxBufIdx = numToCopy;

                /* Check if all requested data has been read from the ring buffer */
                if (0ul == size)
                {
                    /* Enable the RX-error interrupt sources to update the error status */
                    Cy_SCB_SetRxInterruptMask(base, CY_SCB_UART_RECEIVE_ERR);

                    /* Call a completion callback if there was no abort receive called
                    * in the interrupt. The abort clears the number of the received bytes.
                    */
                    if (context->rxBufIdx > 0ul)
                    {
                        if (NULL != context->cbEvents)
                        {
                            context->cbEvents(CY_SCB_UART_RECEIVE_DONE_EVENT);
                        }
                    }

                    /* Continue receiving data in the ring buffer */
                    Cy_SCB_SetRxInterruptMask(base, CY_SCB_RX_INTR_LEVEL);
                }
                else
                {
                    tmpBuf = &tmpBuf[(byteMode) ? (numToCopy) : (2ul * numToCopy)];
                }
            }
        }

        /* Set up a direct RX FIFO receive */
        if (size > 0ul)
        {
            uint32_t halfFifoSize = Cy_SCB_GetFifoSize(base) / 2ul;

            /* Set up context */
            context->rxStatus  = CY_SCB_UART_RECEIVE_ACTIVE;

            context->rxBuf     = (void *) tmpBuf;
            context->rxBufSize = size;
            context->rxBufIdx =  numToCopy;

            /* Set the RX FIFO level to the trigger interrupt */
            Cy_SCB_SetRxFifoLevel(base, (size > halfFifoSize) ? (halfFifoSize - 1ul) : (size - 1ul));

            /* Enable the RX interrupt sources to continue data reading */
            Cy_SCB_SetRxInterruptMask(base, CY_SCB_UART_RX_INTR);
        }

        retStatus = CY_SCB_UART_SUCCESS;
    }

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_UART_AbortReceive
****************************************************************************//**
*
* Abort the current receive operation by clearing the receive status.
* * If the ring buffer is disabled, the receive interrupt sources are disabled.
* * If the ring buffer is enabled, the receive interrupt source is configured
*   to get data from the receive FIFO and put it into the ring buffer.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \note
* * The RX FIFO and ring buffer are not cleared after abort of receive
*   operation.
* * If after the abort of the receive operation the transmitter continues
*   sending data, it gets into the RX FIFO. To drop this data, the RX FIFO
*   and ring buffer (if enabled) have to be cleared when the transmitter
*   stops sending data. Otherwise, received data will be kept and copied
*   to the buffer when \ref Cy_SCB_UART_Receive is called.
*
*******************************************************************************/
void Cy_SCB_UART_AbortReceive(volatile stc_SCB_t *base, cy_stc_scb_uart_context_t *context)
{
    if (NULL == context->rxRingBuf)
    {
        Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);
    }

    context->rxBufSize = 0ul;
    context->rxBufIdx  = 0ul;

    context->rxStatus  = 0ul;
}


/*******************************************************************************
* Function Name: Cy_SCB_UART_GetNumReceived
****************************************************************************//**
*
* Returns the number of data elements received since the last call to \ref
* Cy_SCB_UART_Receive.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* The number of data elements received.
*
*******************************************************************************/
uint32_t Cy_SCB_UART_GetNumReceived(volatile stc_SCB_t const *base, cy_stc_scb_uart_context_t const *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    return (context->rxBufIdx);
}


/*******************************************************************************
* Function Name: Cy_SCB_UART_GetReceiveStatus
****************************************************************************//**
*
* Returns the status of the receive operation.
* This status is a bit mask and the value returned may have multiple bits set.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref group_scb_uart_macro_receive_status.
*
* \note
* The status is only cleared by calling \ref Cy_SCB_UART_Receive again.
*
*******************************************************************************/
uint32_t Cy_SCB_UART_GetReceiveStatus(volatile stc_SCB_t const *base, cy_stc_scb_uart_context_t const *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    return (context->rxStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_UART_Transmit
****************************************************************************//**
*
* This function starts a UART transmit operation.
* It configures the transmit interrupt sources and returns.
* The \ref Cy_SCB_UART_Interrupt manages the further data transfer.
*
* When the transmit operation is completed (requested number of data elements
* sent on the bus), the \ref CY_SCB_UART_TRANSMIT_ACTIVE status is cleared and
* the \ref CY_SCB_UART_TRANSMIT_DONE_EVENT event is generated.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param txBuf
* Pointer to user data to place in transmit buffer.
* The item size is defined by the data type which depends on the configured
* data width.
*
* \param size
* The number of data elements to transmit.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_scb_uart_status_t
*
* \note
* * The buffer must not be modified and stay allocated until its content is
*   copied into the TX FIFO.
* * This function overrides the TX FIFO interrupt sources and changes the
*   TX FIFO level.
*
*******************************************************************************/
cy_en_scb_uart_status_t Cy_SCB_UART_Transmit(volatile stc_SCB_t *base, void *txBuf, uint32_t size, cy_stc_scb_uart_context_t *context)
{
    cy_en_scb_uart_status_t retStatus = CY_SCB_UART_TRANSMIT_BUSY;

#if !defined(NDEBUG)
    /* Check that the initialization key was set before using the context */
    CY_ASSERT(NULL != context);
    CY_ASSERT(CY_SCB_UART_INIT_KEY == context->initKey);
#endif /* !(NDEBUG) */

    /* Check if there are no active transfer requests */
    if (0ul == (CY_SCB_UART_TRANSMIT_ACTIVE & context->txStatus))
    {
        /* Set up context */
        context->txStatus  = CY_SCB_UART_TRANSMIT_ACTIVE;

        context->txBuf     = txBuf;
        context->txBufSize = size;

        /* Set the level in TX FIFO to start a transfer */
        Cy_SCB_SetTxFifoLevel(base, (Cy_SCB_GetFifoSize(base) / 2ul));

        /* Enable the interrupt sources */
        if (CY_SCB_UART_SMARTCARD == base->unUART_CTRL.stcField.u2MODE)
        {
            /* Transfer data into TX FIFO and track SmartCard-specific errors */
            Cy_SCB_SetTxInterruptMask(base, CY_SCB_UART_TX_INTR);
        }
        else
        {
            /* Transfer data into TX FIFO */
            Cy_SCB_SetTxInterruptMask(base, CY_SCB_TX_INTR_LEVEL);
        }

        retStatus = CY_SCB_UART_SUCCESS;
    }

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_UART_AbortTransmit
****************************************************************************//**
*
* Aborts the current transmit operation.
* It disables the transmit interrupt sources and clears the transmit FIFO
* and status.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \sideeffect
* The transmit FIFO clear operation also clears the shift register, so that
* the shifter could be cleared in the middle of a data element transfer,
* corrupting it. The data element corruption means that all bits which has
* not been transmitted are transmitted as "ones" on the bus.
*
*******************************************************************************/
void Cy_SCB_UART_AbortTransmit(volatile stc_SCB_t *base, cy_stc_scb_uart_context_t *context)
{
    Cy_SCB_SetTxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

    Cy_SCB_UART_ClearTxFifo(base);

    context->txBufSize = 0ul;
    context->txLeftToTransmit = 0ul;

    context->txStatus  = 0ul;
}


/*******************************************************************************
* Function Name: Cy_SCB_UART_GetNumLeftToTransmit
****************************************************************************//**
*
* Returns the number of data elements left to transmit since the last call to
* \ref Cy_SCB_UART_Transmit.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* The number of data elements left to transmit.
*
*******************************************************************************/
uint32_t Cy_SCB_UART_GetNumLeftToTransmit(volatile stc_SCB_t const *base, cy_stc_scb_uart_context_t const *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    return (context->txLeftToTransmit);
}


/*******************************************************************************
* Function Name: Cy_SCB_UART_GetTransmitStatus
****************************************************************************//**
*
* Returns the status of the transmit operation.
* This status is a bit mask and the value returned may have multiple bits set.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref group_scb_uart_macro_transmit_status.
*
* \note
* The status is only cleared by calling \ref Cy_SCB_UART_Transmit or
* \ref Cy_SCB_UART_AbortTransmit.
*
*******************************************************************************/
uint32_t Cy_SCB_UART_GetTransmitStatus(volatile stc_SCB_t const *base, cy_stc_scb_uart_context_t const *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    return (context->txStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_UART_Interrupt
****************************************************************************//**
*
* This is the interrupt function for the SCB configured in the UART mode.
* This function must be called inside a user-defined interrupt service
* routine to make \ref Cy_SCB_UART_Transmit and \ref Cy_SCB_UART_Receive
* work.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
void Cy_SCB_UART_Interrupt(volatile stc_SCB_t *base, cy_stc_scb_uart_context_t *context)
{
    if (0ul != (CY_SCB_RX_INTR & Cy_SCB_GetInterruptCause(base)))
    {
        /* Get RX error events: a frame error, parity error, and overflow */
        uint32_t locRxErr = (CY_SCB_UART_RECEIVE_ERR & Cy_SCB_GetRxInterruptStatusMasked(base));

        /* Handle the error conditions */
        if (0ul != locRxErr)
        {
            context->rxStatus |= locRxErr;

            Cy_SCB_ClearRxInterrupt(base, locRxErr);

            if (NULL != context->cbEvents)
            {
                context->cbEvents(CY_SCB_UART_RECEIVE_ERR_EVENT);
            }
        }

        /* Break the detect */
        if (0ul != (CY_SCB_RX_INTR_UART_BREAK_DETECT & Cy_SCB_GetRxInterruptStatusMasked(base)))
        {
            context->rxStatus |= CY_SCB_UART_RECEIVE_BREAK_DETECT;

            Cy_SCB_ClearRxInterrupt(base, CY_SCB_RX_INTR_UART_BREAK_DETECT);
        }

        /* Copy the received data */
        if (0ul != (CY_SCB_RX_INTR_LEVEL & Cy_SCB_GetRxInterruptStatusMasked(base)))
        {
            if (context->rxBufSize > 0ul)
            {
                HandleDataReceive(base, context);
            }
            else
            {
                if (NULL != context->rxRingBuf)
                {
                    HandleRingBuffer(base, context);
                }
            }

            Cy_SCB_ClearRxInterrupt(base, CY_SCB_RX_INTR_LEVEL);
        }
    }

    if (0ul != (CY_SCB_TX_INTR & Cy_SCB_GetInterruptCause(base)))
    {
        uint32_t locTxErr = (CY_SCB_UART_TRANSMIT_ERR & Cy_SCB_GetTxInterruptStatusMasked(base));

        /* Handle the TX error conditions */
        if (0ul != locTxErr)
        {
            context->txStatus |= locTxErr;
            Cy_SCB_ClearTxInterrupt(base, locTxErr);

            if (NULL != context->cbEvents)
            {
                context->cbEvents(CY_SCB_UART_TRANSMIT_ERR_EVENT);
            }
        }

        /* Load data to transmit */
        if (0ul != (CY_SCB_TX_INTR_LEVEL & Cy_SCB_GetTxInterruptStatusMasked(base)))
        {
            HandleDataTransmit(base, context);

            Cy_SCB_ClearTxInterrupt(base, CY_SCB_TX_INTR_LEVEL);
        }

        /* Handle the TX complete */
        if (0ul != (CY_SCB_TX_INTR_UART_DONE & Cy_SCB_GetTxInterruptStatusMasked(base)))
        {
            /* Disable all TX interrupt sources */
            Cy_SCB_SetTxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

            context->txStatus &= (uint32_t) ~CY_SCB_UART_TRANSMIT_ACTIVE;
            context->txLeftToTransmit = 0ul;

            if (NULL != context->cbEvents)
            {
                context->cbEvents(CY_SCB_UART_TRANSMIT_DONE_EVENT);
            }
        }
    }
}



/*******************************************************************************
* Function Name: HandleDataReceive
****************************************************************************//**
*
* Reads data from the receive FIFO into the buffer provided by
* \ref Cy_SCB_UART_Receive.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void HandleDataReceive(volatile stc_SCB_t *base, cy_stc_scb_uart_context_t *context)
{
    uint32_t numCopied;
    uint32_t halfFifoSize = Cy_SCB_GetFifoSize(base) / 2ul;

    /* Get data from RX FIFO */
    numCopied = Cy_SCB_UART_GetArray(base, context->rxBuf, context->rxBufSize);

    /* Move the buffer */
    context->rxBufIdx  += numCopied;
    context->rxBufSize -= numCopied;

    if (0ul == context->rxBufSize)
    {
        if (NULL != context->rxRingBuf)
        {
            /* Adjust the level to proceed with the ring buffer */
            Cy_SCB_SetRxFifoLevel(base, (context->rxRingBufSize >= halfFifoSize) ?
                                            (halfFifoSize - 1ul) : (context->rxRingBufSize - 1ul));

            Cy_SCB_SetRxInterruptMask(base, CY_SCB_RX_INTR_LEVEL);
        }
        else
        {
            Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);
        }

        /* Update the status */
        context->rxStatus &= (uint32_t) ~CY_SCB_UART_RECEIVE_ACTIVE;

        /* Notify that receive is done in a callback */
        if (NULL != context->cbEvents)
        {
            context->cbEvents(CY_SCB_UART_RECEIVE_DONE_EVENT);
        }
    }
    else
    {
        uint8_t *buf = (uint8_t *) context->rxBuf;

        buf = &buf[(Cy_SCB_IsRxDataWidthByte(base) ? (numCopied) : (2ul * numCopied))];
        context->rxBuf = (void *) buf;

        if (context->rxBufSize < halfFifoSize)
        {
            /* Set the RX FIFO level to trigger an interrupt */
            Cy_SCB_SetRxFifoLevel(base, (context->rxBufSize - 1ul));
        }
    }
}


/*******************************************************************************
* Function Name: HandleRingBuffer
****************************************************************************//**
*
* Reads data from the receive FIFO into the receive ring buffer.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void HandleRingBuffer(volatile stc_SCB_t *base, cy_stc_scb_uart_context_t *context)
{
    uint32_t halfFifoSize = Cy_SCB_GetFifoSize(base) / 2ul;
    uint32_t numToCopy = Cy_SCB_GetNumInRxFifo(base);
    uint32_t locHead = context->rxRingBufHead;
    uint32_t rxData;

    /* Get data into the ring buffer */
    while (numToCopy > 0ul)
    {
        ++locHead;

        if (locHead == context->rxRingBufSize)
        {
            locHead = 0ul;
        }

        if (locHead == context->rxRingBufTail)
        {
            /* The ring buffer is full, trigger a callback */
            if (NULL != context->cbEvents)
            {
                context->cbEvents(CY_SCB_UART_RB_FULL_EVENT);
            }

            /* The ring buffer is still full. Disable the RX interrupt not to put data into the ring buffer.
            * The data is stored in the RX FIFO until it overflows. Revert the head index.
            */
            if (locHead == context->rxRingBufTail)
            {
                Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

                locHead = (locHead > 0ul) ? (locHead - 1ul) : (context->rxRingBufSize - 1ul);
                break;
            }
        }

        /* Get data from RX FIFO. */
        rxData = Cy_SCB_ReadRxFifo(base);

        /* Put a data item in the ring buffer */
        if (Cy_SCB_IsRxDataWidthByte(base))
        {
            ((uint8_t *) context->rxRingBuf)[locHead] = (uint8_t) rxData;
        }
        else
        {
            ((uint16_t *) context->rxRingBuf)[locHead] = (uint16_t) rxData;
        }

        --numToCopy;
    }

    /* Update the head index */
    context->rxRingBufHead = locHead;

    /* Get free entries in the ring buffer */
    numToCopy = context->rxRingBufSize - Cy_SCB_UART_GetNumInRingBuffer(base, context);

    if (numToCopy < halfFifoSize)
    {
        /* Adjust the level to copy to the ring buffer */
        uint32_t level = (numToCopy > 0ul) ? (numToCopy - 1ul) : 0ul;
        Cy_SCB_SetRxFifoLevel(base, level);
    }
}


/*******************************************************************************
* Function Name: HandleDataTransmit
****************************************************************************//**
*
* Loads the transmit FIFO with data provided by \ref Cy_SCB_UART_Transmit.
*
* \param base
* The pointer to the UART SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_uart_context_t allocated
* by the user. The structure is used during the UART operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void HandleDataTransmit(volatile stc_SCB_t *base, cy_stc_scb_uart_context_t *context)
{
    uint32_t numToCopy;
    uint32_t fifoSize = Cy_SCB_GetFifoSize(base);
    bool     byteMode = Cy_SCB_IsTxDataWidthByte(base);

    if (context->txBufSize > 1ul)
    {
        uint8_t *buf = (uint8_t *) context->txBuf;

        /* Get the number of items left for transmission */
        context->txLeftToTransmit = context->txBufSize;

        /* Put data into TX FIFO */
        numToCopy = Cy_SCB_UART_PutArray(base, context->txBuf, (context->txBufSize - 1ul));

        /* Move the buffer */
        context->txBufSize -= numToCopy;

        buf = &buf[(byteMode) ? (numToCopy) : (2ul * numToCopy)];
        context->txBuf = (void *) buf;
    }

    /* Put the last data item into TX FIFO */
    if ((fifoSize != Cy_SCB_GetNumInTxFifo(base)) && (1ul == context->txBufSize))
    {
        uint32_t txData;
        uint32_t intrStatus;

        context->txBufSize = 0ul;

        /* Get the last item from the buffer */
        txData = (uint32_t) ((byteMode) ? ((uint8_t *)  context->txBuf)[0ul] :
                                          ((uint16_t *) context->txBuf)[0ul]);

        /* Put the last data element and make sure that "TX done" will happen for it */
        intrStatus = Cy_SysLib_EnterCriticalSection();

        Cy_SCB_WriteTxFifo(base, txData);
        Cy_SCB_ClearTxInterrupt(base, CY_SCB_TX_INTR_UART_DONE);

        Cy_SysLib_ExitCriticalSection(intrStatus);

        /* Disable the level interrupt source and enable "transfer done" */
        Cy_SCB_SetTxInterruptMask(base, (CY_SCB_TX_INTR_UART_DONE |
                    (Cy_SCB_GetTxInterruptMask(base) & (uint32_t) ~CY_SCB_TX_INTR_LEVEL)));

        /* Data is copied into TX FIFO */
        context->txStatus |= CY_SCB_UART_TRANSMIT_IN_FIFO;

        if (NULL != context->cbEvents)
        {
            context->cbEvents(CY_SCB_UART_TRANSMIT_IN_FIFO_EVENT);
        }
    }
}


#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */

