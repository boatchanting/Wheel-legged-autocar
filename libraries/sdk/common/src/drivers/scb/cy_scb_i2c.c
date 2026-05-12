/***************************************************************************//**
* \file cy_scb_i2c.c
* \version 1.0
*
* Provides I2C API implementation of the SCB driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_scb_i2c.h"

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
*        Function Prototypes
***************************************/

static void SlaveInterrupt         (volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);
static void SlaveHandleAddress     (volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);
static void SlaveHandleDataReceive (volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);
static void SlaveHandleDataTransmit(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);
static void SlaveHandleStop        (volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);

static void MasterInterrupt         (volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);
static void MasterHandleDataTransmit(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);
static void MasterHandleDataReceive (volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);
static void MasterHandleStop        (volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);
static void MasterHandleComplete    (volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);

static cy_en_scb_i2c_status_t HandleStatus(volatile stc_SCB_t *base, uint32_t status,
                                           cy_stc_scb_i2c_context_t *context);
static uint32_t WaitOneUnit(uint32_t *timeout);


/*******************************************************************************
* Function Name: Cy_SCB_I2C_Init
****************************************************************************//**
*
* Initializes the SCB for the I2C operation.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param config
* The pointer to the configuration structure \ref cy_stc_scb_i2c_config_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_scb_i2c_status_t
*
* \note
* Ensure that the SCB block is disabled before calling this function.
*
*******************************************************************************/
cy_en_scb_i2c_status_t Cy_SCB_I2C_Init(volatile stc_SCB_t *base, cy_stc_scb_i2c_config_t const *config, cy_stc_scb_i2c_context_t *context)
{
    cy_en_scb_i2c_status_t retStatus = CY_SCB_I2C_BAD_PARAM;
    un_SCB_CTRL_t         temp_CTRL         = {0ul};
    un_SCB_I2C_CTRL_t     temp_I2C_CTRL     = {0ul};
    un_SCB_RX_CTRL_t      temp_RX_CTRL      = {0ul};
    un_SCB_RX_MATCH_t     temp_RX_MATCH     = {0ul};
    un_SCB_TX_CTRL_t      temp_TX_CTRL      = {0ul};

    if ((NULL != base) && (NULL != config) && (NULL != context))
    {
        /* Configure the I2C interface */
        temp_CTRL.stcField.u1ADDR_ACCEPT = (config->acceptAddrInFifo ? 1ul : 0ul);
        temp_CTRL.stcField.u1EC_AM_MODE  = (config->enableWakeFromSleep ? 1ul : 0ul);
        temp_CTRL.stcField.u2MEM_WIDTH   = 0ul;
        temp_CTRL.stcField.u2MODE        = CY_SCB_CTRL_MODE_I2C;
        base->unCTRL.u32Register         = temp_CTRL.u32Register;

        temp_I2C_CTRL.stcField.u1S_GENERAL_IGNORE = (config->ackGeneralAddr ? 0ul : 1ul);
        temp_I2C_CTRL.stcField.u1SLAVE_MODE       = (config->i2cMode & 0x00000001ul);
        temp_I2C_CTRL.stcField.u1MASTER_MODE      = (config->i2cMode & 0x00000002ul) >> 1ul;
        base->unI2C_CTRL.u32Register              = temp_I2C_CTRL.u32Register;

        /* Configure the RX direction */
        temp_RX_CTRL.stcField.u5DATA_WIDTH = CY_SCB_I2C_DATA_WIDTH;
        temp_RX_CTRL.stcField.u1MSB_FIRST  = 1ul;
        base->unRX_CTRL.u32Register        = temp_RX_CTRL.u32Register;
        base->unRX_FIFO_CTRL.u32Register   = (config->useRxFifo ? (CY_SCB_I2C_FIFO_SIZE - 1ul) : 0ul);

        /* Set the default address and mask */
        temp_RX_MATCH.stcField.u8ADDR = ((uint32_t) config->slaveAddress << 1ul);
        temp_RX_MATCH.stcField.u8MASK = ((uint32_t) config->slaveAddressMask << 1ul);
        base->unRX_MATCH.u32Register  = temp_RX_MATCH.u32Register;

        /* Configure the TX direction */
        temp_TX_CTRL.stcField.u5DATA_WIDTH = CY_SCB_I2C_DATA_WIDTH;
        temp_TX_CTRL.stcField.u1MSB_FIRST  = 1ul;
        temp_TX_CTRL.stcField.u1OPEN_DRAIN = 1ul;
        base->unTX_CTRL.u32Register        = temp_TX_CTRL.u32Register;
        base->unTX_FIFO_CTRL.u32Register   = (config->useTxFifo ? CY_SCB_I2C_HALF_FIFO_SIZE : 1ul);

        /* Configure interrupt sources */
        base->unINTR_SPI_EC_MASK.u32Register = 0ul;
        base->unINTR_I2C_EC_MASK.u32Register = 0ul;
        base->unINTR_RX_MASK.u32Register     = 0ul;
        base->unINTR_TX_MASK.u32Register     = 0ul;
        base->unINTR_M_MASK.u32Register      = 0ul;

        base->unINTR_S_MASK.u32Register      = ((0ul != (CY_SCB_I2C_SLAVE & config->i2cMode)) ? CY_SCB_I2C_SLAVE_INTR : 0ul);

        /* Initialize the context */
        context->useRxFifo = config->useRxFifo;
        context->useTxFifo = config->useTxFifo;

        context->state = CY_SCB_I2C_IDLE;

        /* Master-specific */
        context->masterStatus     = 0ul;
        context->masterBufferIdx  = 0ul;

        /* Slave-specific */
        context->slaveStatus       = 0ul;

        context->slaveRxBufferIdx  = 0ul;
        context->slaveRxBufferSize = 0ul;

        context->slaveTxBufferIdx  = 0ul;
        context->slaveTxBufferSize = 0ul;

        /* Un-register callbacks */
        context->cbEvents = NULL;
        context->cbAddr   = NULL;

        retStatus = CY_SCB_I2C_SUCCESS;
    }

    return (retStatus);
}


/*******************************************************************************
*  Function Name: Cy_SCB_I2C_DeInit
****************************************************************************//**
*
* De-initializes the SCB block, returns register values to default.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \note
* Ensure that the SCB block is disabled before calling this function.
*
*******************************************************************************/
void Cy_SCB_I2C_DeInit(volatile stc_SCB_t *base)
{
    /* Returns block registers into the default state */
    base->unCTRL.u32Register             = CY_SCB_CTRL_DEF_VAL;
    base->unI2C_CTRL.u32Register         = CY_SCB_I2C_CTRL_DEF_VAL;
    base->unI2C_CFG.u32Register          = CY_SCB_I2C_CFG_DEF_VAL;

    base->unRX_CTRL.u32Register          = CY_SCB_RX_CTRL_DEF_VAL;
    base->unRX_FIFO_CTRL.u32Register     = 0ul;
    base->unRX_MATCH.u32Register         = 0ul;

    base->unTX_CTRL.u32Register          = CY_SCB_TX_CTRL_DEF_VAL;
    base->unTX_FIFO_CTRL.u32Register     = 0ul;

    base->unINTR_SPI_EC_MASK.u32Register = 0ul;
    base->unINTR_I2C_EC_MASK.u32Register = 0ul;
    base->unINTR_RX_MASK.u32Register     = 0ul;
    base->unINTR_TX_MASK.u32Register     = 0ul;
    base->unINTR_M_MASK.u32Register      = 0ul;
    base->unINTR_S_MASK.u32Register      = 0ul;
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_Disable
****************************************************************************//**
*
* Disables the SCB block and clears context statuses.
* Note that after the block is disabled, the TX and RX FIFOs and hardware
* statuses are cleared. Also, the hardware stops driving the output and
* ignores the input.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \note
* Calling this function when I2C is busy (master performs transaction or slave
* was address and communicates with master) may cause transaction corruption
* because the hardware stops driving the outputs and ignores the inputs.
* It is recommended to ensure that I2C is not busy before calling this function.
*
*******************************************************************************/
void Cy_SCB_I2C_Disable(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    base->unCTRL.stcField.u1ENABLED = 0ul;

    /* Set the state to default and clear statuses */
    context->state        = CY_SCB_I2C_IDLE;
    context->masterStatus = 0ul;
    context->slaveStatus  = 0ul;
}


/*******************************************************************************
* Function Name: Cy_I2C_DeepSleepCallback
****************************************************************************//**
*
* This function handles the transition of the I2C SCB into and out of 
* Deep-Sleep mode. It prevents the device from entering Deep-Sleep mode if 
* the I2C slave or master is actively communicating. 
* The following behavior of the I2C SCB depends on if the SCB block is  
* wakeup-capable or not:
* * The SCB wakeup-capable: on the incoming I2C slave address, the slave  
*   receives address and stretches the clock until the device is awoken from 
*   Deep-Sleep mode. If the slave address occurs before the device enters 
*   Deep-Sleep mode, the device will not enter Deep-Sleep mode.
*   Only the I2C slave can be configured to be a wakeup source from Deep-Sleep 
*   mode.
* * The SCB is not wakeup-capable: the I2C is disabled. It is enabled when the  
*   device failed to enter Deep-Sleep mode or it is awaken from Deep-Sleep mode.
*   During the I2C is disabled, it does stops driving the outputs and ignores 
*   the inputs. The slave NACKs all incoming addresses.
* 
* This function can be called during execution of \ref Cy_SysPm_DeepSleep,
* to do it, register this function as a callback before calling 
* \ref Cy_SysPm_DeepSleep : specify \ref CY_SYSPM_DEEPSLEEP as the callback 
* type and call \ref Cy_SysPm_RegisterCallback.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure
*
* \param mode
* \ref cy_en_syspm_callback_mode_t
*
* \return
* \ref cy_en_syspm_status_t
*
* \warning
* For proper operation, when the I2C slave is configured to be a wakeup source 
* from Deep-Sleep mode, this function must be copied and modified by the user. 
* The I2C clock disable code must be inserted in the \ref CY_SYSPM_BEFORE_TRANSITION  
* and enable code in the \ref CY_SYSPM_AFTER_TRANSITION mode processing.
*
*******************************************************************************/
cy_en_syspm_status_t Cy_I2C_DeepSleepCallback(void *base, void *context, cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t retStatus = CY_SYSPM_SUCCESS;

    volatile stc_SCB_t *locBase = (volatile stc_SCB_t *) base;
    cy_stc_scb_i2c_context_t *locContext = (cy_stc_scb_i2c_context_t *) context;

    switch(mode)
    {
        case CY_SYSPM_CHECK_READY:
        {
            /* Disable the slave interrupt sources to protect the state */
            Cy_SCB_SetSlaveInterruptMask(locBase, CY_SCB_CLEAR_ALL_INTR_SRC);

            /* If the I2C is in the IDLE state, it is ready for Deep-Sleep mode
            * (either the master or the slave is not busy),
            * otherwise return fail and restore the slave interrupt sources.
            */
            if (CY_SCB_I2C_IDLE == locContext->state)
            {
                if (locBase->unCTRL.stcField.u1EC_AM_MODE)
                {
                    /* The SCB is wakeup-capable: do not restore the address 
                    * match and general call interrupt sources. The next 
                    * transaction intended to the slave will be paused 
                    * (SCL is stretched) before the address is ACKed because 
                    * the corresponding interrupt source is disabled.
                    */
                    Cy_SCB_SetSlaveInterruptMask(locBase, CY_SCB_I2C_SLAVE_INTR_NO_ADDR);
                }
                else
                {
                    /* The SCB is NOT wakeup-capable: disable the I2C. The slave
                    * stops responding to the master and the master stops 
                    * driving the bus until the I2C is enabled. This happens 
                    * when the device failed to enter into Deep-Sleep mode or it 
                    * is awaken from Deep-Sleep mode.
                    */
                    Cy_SCB_I2C_Disable(locBase, locContext);
                    Cy_SCB_SetSlaveInterruptMask(locBase, CY_SCB_I2C_SLAVE_INTR);
                }
            }
            else
            {
                Cy_SCB_SetSlaveInterruptMask(locBase, CY_SCB_I2C_SLAVE_INTR);
                retStatus = CY_SYSPM_FAIL;
            }
        }
        break;

        case CY_SYSPM_CHECK_FAIL:
        {
            /* The other driver is not ready for Deep-Sleep mode. Restore 
            * Active mode configuration.
            */

            if (locBase->unCTRL.stcField.u1EC_AM_MODE)
            {
                /* The SCB is wakeup-capable: restore the slave interrupt sources */
                Cy_SCB_SetSlaveInterruptMask(locBase, CY_SCB_I2C_SLAVE_INTR);
            }
            else
            {
                /* The SCB is NOT wakeup-capable: enable the I2C to operate */
                Cy_SCB_I2C_Enable(locBase);
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

            if (locBase->unCTRL.stcField.u1EC_AM_MODE)
            {
                /* The SCB is wakeup-capable: enable the I2C wakeup interrupt 
                * source. If any transaction was paused, the I2C interrupt
                * becomes pending and prevents entering Deep-Sleep mode.
                * The transaction continues as soon as the global interrupts 
                * are enabled.
                */
                Cy_SCB_SetI2CInterruptMask(locBase, CY_SCB_I2C_INTR_WAKEUP);

                /* IMPORTANT (insert code below): for proper entering Deep
                * Sleep the I2C clock must be disabled. This code must be 
                * inserted by the user because the driver does not have access 
                * to the clock.
                */
            }
        }
        break;

        case CY_SYSPM_AFTER_EXIT:
        {
            if (locBase->unCTRL.stcField.u1EC_AM_MODE)
            {
                /* IMPORTANT (insert code below): for proper exiting Deep  
                * Sleep, the I2C clock must be enabled. This code must be 
                * inserted by the user because the driver does not have access 
                * to the clock.
                */

                /* The SCB is wakeup-capable: disable the I2C wakeup interrupt 
                * source and restore slave interrupt sources.
                */
                Cy_SCB_SetI2CInterruptMask(locBase, CY_SCB_CLEAR_ALL_INTR_SRC);
                Cy_SCB_SetSlaveInterruptMask(locBase, CY_SCB_I2C_SLAVE_INTR);
            }
            else
            {
                /* The SCB is NOT wakeup-capable: enable the I2C to operate */
                Cy_SCB_I2C_Enable(locBase);
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
* Function Name: Cy_I2C_HibernateCallback
****************************************************************************//**
*
* This function handles the transition of the I2C SCB block into Hibernate mode. 
* It prevents the device from entering Hibernate mode if the I2C slave or 
* master is actively communicating. 
* If the I2C is ready to enter Hibernate mode, it is disabled. If the device failed 
* to enter Hibernate mode the I2C is enabled. After the I2C is disabled, it stops  
* driving the outputs and ignores the inputs. The slave NACKs all incoming 
* addresses.
* 
* This function can be called during execution of \ref Cy_SysPm_Hibernate,
* to do it, register this function as a callback before calling 
* \ref Cy_SysPm_Hibernate : specify \ref CY_SYSPM_HIBERNATE as the callback 
* type and call \ref Cy_SysPm_RegisterCallback.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
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
cy_en_syspm_status_t Cy_I2C_HibernateCallback(void *base, void *context, cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t retStatus = CY_SYSPM_SUCCESS;

    volatile stc_SCB_t *locBase = (volatile stc_SCB_t *) base;
    cy_stc_scb_i2c_context_t *locContext = (cy_stc_scb_i2c_context_t *) context;

    switch(mode)
    {
        case CY_SYSPM_CHECK_READY:
        {
            /* Disable the slave interrupt sources to protect the state */
            Cy_SCB_SetSlaveInterruptMask(locBase, CY_SCB_CLEAR_ALL_INTR_SRC);

            /* If the I2C is in the IDLE state, it is ready for Hibernate mode
            * (either the master or the slave is not busy),
            * otherwise, return fail and restore the slave interrupt sources.
            */
            if (CY_SCB_I2C_IDLE == locContext->state)
            {
                /* Disable the I2C. The slave stops responding to the master and
                * the master stops driving the bus until the I2C is enabled.
                * This happens if the device failed to enter Hibernate mode.
                */
                Cy_SCB_I2C_Disable(locBase, locContext);
                Cy_SCB_SetSlaveInterruptMask(locBase, CY_SCB_I2C_SLAVE_INTR);
            }
            else
            {
                Cy_SCB_SetSlaveInterruptMask(locBase, CY_SCB_I2C_SLAVE_INTR);
                retStatus = CY_SYSPM_FAIL;
            }
        }
        break;

        case CY_SYSPM_CHECK_FAIL:
        {
            /* The other driver is not ready for Deep-Sleep mode. Restore the 
            * Active mode configuration.
            */

            /* Enable the I2C to operate */
            Cy_SCB_I2C_Enable(locBase);
        }
        break;

        case CY_SYSPM_BEFORE_ENTER:
        case CY_SYSPM_AFTER_EXIT:
            /* The SCB is not capable of waking up from Hibernate mode: do nothing */
        break;

        default:
            retStatus = CY_SYSPM_FAIL;
        break;
    }

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_SetDataRate
****************************************************************************//**
*
* Configures the SCB to work at the desired data rate.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param dataRateHz
* The desired data Rate in Hz.
*
* \param scbClockHz
* The frequency of the clock connected to the SCB in Hz.
*
* \return
* The achieved data rate in Hz.
*
* \note
* This function does not change the values of the clock divider connected
* to the SCB, it only changes the SCB clock oversample registers. If this
* function is not able to achieve the desired data rate, then the clock
* divider has to be adjusted. Call this function only while the SCB is
* disabled. For the slave, this function only checks that the attached clock is
* fast enough to meet the desired data rate, it does not change any registers.
*
*******************************************************************************/
uint32_t Cy_SCB_I2C_SetDataRate(volatile stc_SCB_t *base, uint32_t dataRateHz, uint32_t scbClockHz)
{
    uint32_t actualDataRate = 0ul;

    if ((base->unI2C_CTRL.stcField.u1SLAVE_MODE == 1ul) && (base->unI2C_CTRL.stcField.u1MASTER_MODE == 0ul))
    {
        actualDataRate = Cy_SCB_I2C_GetDataRate(base, scbClockHz);

        /* Use an analog filter for the slave */
        base->unRX_CTRL.stcField.u1MEDIAN = 0ul;
        base->unI2C_CFG.u32Register       = CY_SCB_I2C_ENABLE_ANALOG_FITLER;
    }
    else
    {
        if ((scbClockHz > 0u) && (dataRateHz > 0u))
        {
            uint32_t sclLow;
            uint32_t sclHigh;
            uint32_t lowPhase;
            uint32_t highPhase;

            /* Convert scb clock and data rate in kHz */
            uint32_t scbClockKHz = scbClockHz / 1000ul;
            uint32_t dataRateKHz = dataRateHz / 1000ul;

            /* Get period of the scb clock in ns */
            uint32_t period = 1000000000ul / scbClockHz;

            /* Get duration of SCL low and high for the selected data rate */
            if (dataRateHz <= CY_SCB_I2C_STD_DATA_RATE)
            {
                sclLow  = CY_SCB_I2C_MASTER_STD_SCL_LOW;
                sclHigh = CY_SCB_I2C_MASTER_STD_SCL_HIGH;
            }
            else if (dataRateHz <= CY_SCB_I2C_FST_DATA_RATE)
            {
                sclLow  = CY_SCB_I2C_MASTER_FST_SCL_LOW;
                sclHigh = CY_SCB_I2C_MASTER_FST_SCL_HIGH;
            }
            else
            {
                sclLow  = CY_SCB_I2C_MASTER_FSTP_SCL_LOW;
                sclHigh = CY_SCB_I2C_MASTER_FSTP_SCL_HIGH;
            }

            /* Get low phase minimum value in scb clocks */
            lowPhase = sclLow / period;
            while (((period * lowPhase) < sclLow) && (lowPhase < CY_SCB_I2C_LOW_PHASE_MAX))
            {
                ++lowPhase;
            }

            /* Get high phase minimum value in scb clocks */
            highPhase = sclHigh / period;
            while (((period * highPhase) < sclHigh) && (highPhase < CY_SCB_I2C_HIGH_PHASE_MAX))
            {
                ++highPhase;
            }

            /* Get actual data rate */
            actualDataRate = scbClockKHz / (lowPhase + highPhase);

            uint32_t idx = 0ul;
            while ((actualDataRate > dataRateKHz) &&
                   ((lowPhase + highPhase) < CY_SCB_I2C_DUTY_CYCLE_MAX))
            {
                /* Increase low and high phase to reach desired data rate */
                if (0ul != (idx & 0x1ul))
                {
                    if (highPhase < CY_SCB_I2C_HIGH_PHASE_MAX)
                    {
                        highPhase++;
                    }
                }
                else
                {
                    if (lowPhase < CY_SCB_I2C_LOW_PHASE_MAX)
                    {
                        lowPhase++;
                    }
                }

                idx++;

                /* Update actual data rate */
                actualDataRate = scbClockKHz / (lowPhase + highPhase);
            }

            /* Set filter configuration based on actual data rate */
            if (actualDataRate > CY_SCB_I2C_FST_DATA_RATE)
            {
                /* Use a digital filter */
                base->unRX_CTRL.stcField.u1MEDIAN = 1ul;
                base->unI2C_CFG.u32Register       = CY_SCB_I2C_DISABLE_ANALOG_FITLER;
            }
            else
            {
                /* Use an analog filter */
                base->unRX_CTRL.stcField.u1MEDIAN = 0ul;
                base->unI2C_CFG.u32Register       = CY_SCB_I2C_ENABLE_ANALOG_FITLER;
            }

            /* Set phase low and high */
            Cy_SCB_I2C_MasterSetLowPhaseDutyCycle (base, lowPhase);
            Cy_SCB_I2C_MasterSetHighPhaseDutyCycle(base, highPhase);

            /* Convert actual data rate in Hz */
            actualDataRate = scbClockHz / (lowPhase + highPhase);
        }
    }

    return (actualDataRate);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_GetDataRate
****************************************************************************//**
*
* Returns the data rate for the selected SCB block.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param scbClockHz
* The frequency of the clock connected to the SCB in Hz.
*
* \return
* The data rate in Hz.
*
*******************************************************************************/
uint32_t Cy_SCB_I2C_GetDataRate(volatile stc_SCB_t const *base, uint32_t scbClockHz)
{
    uint32_t actualDataRate;

    if ( (base->unI2C_CTRL.stcField.u1SLAVE_MODE == 1ul) && 
         (base->unI2C_CTRL.stcField.u1MASTER_MODE == 0ul) )
    {
        /* Check the clock frequency range to get maximum supported data rate */
        if ((scbClockHz >= CY_SCB_I2C_SLAVE_FST_CLK_MIN) && (scbClockHz <= CY_SCB_I2C_SLAVE_FST_CLK_MAX))
        {
            actualDataRate = CY_SCB_I2C_FST_DATA_RATE;
        }
        else if ((scbClockHz >= CY_SCB_I2C_SLAVE_STD_CLK_MIN) && (scbClockHz <= CY_SCB_I2C_SLAVE_STD_CLK_MAX))
        {
            actualDataRate = CY_SCB_I2C_STD_DATA_RATE;
        }
        else if ((scbClockHz >= CY_SCB_I2C_SLAVE_FSTP_CLK_MIN) && (scbClockHz <= CY_SCB_I2C_SLAVE_FSTP_CLK_MAX))
        {
            actualDataRate = CY_SCB_I2C_FSTP_DATA_RATE;
        }
        else
        {
            /* The clock frequency is too low or it gets to the gap between
            * Fast and Fast Plus data rates.
            */
            actualDataRate = 0ul;
        }
    }
    else
    {
        uint32_t dutyCycle;

        /* Get number of clocks in one SCL period */
        dutyCycle = base->unI2C_CTRL.stcField.u4LOW_PHASE_OVS  +
                    base->unI2C_CTRL.stcField.u4HIGH_PHASE_OVS +
                    2ul;

        /* Calculate the actual data rate */
        actualDataRate = (scbClockHz / dutyCycle);
    }

    return (actualDataRate);
}


/*******************************************************************************
*                         I2C Slave API
*******************************************************************************/

/*******************************************************************************
* Function Name: Cy_SCB_I2C_SlaveGetStatus
****************************************************************************//**
*
* Returns the current I2C slave status.
* This status is a bit mask and the value returned may have multiple bits set.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref group_scb_i2c_macro_slave_status.
*
*******************************************************************************/
uint32_t Cy_SCB_I2C_SlaveGetStatus(volatile stc_SCB_t const *base, cy_stc_scb_i2c_context_t const *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    return (context->slaveStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_SlaveConfigReadBuf
****************************************************************************//**
*
* Configures the buffer pointer and the read buffer size. This is the buffer
* that the master reads data from. After this function is called, data
* transfer from the read buffer to the master is handled by
* \ref Cy_SCB_I2C_Interrupt.
*
* When Read transaction is completed (master generated Stop, ReStart or
* error occurred), the \ref CY_SCB_I2C_SLAVE_RD_BUSY status is cleared and
* the \ref CY_SCB_I2C_SLAVE_RD_CMPLT is set, also
* the \ref CY_SCB_I2C_SLAVE_RD_CMPLT_EVENT event is generated.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param rdBuf
* The pointer to the buffer with data to be read by the master.
*
* \param size
* An amount of data the master can read from rdBuf.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \note
* * The Read buffer must not be modified and stay allocated until it has been
*   read by the master.
* * If this function has not been called, and the master tries to read data
*   from the slave a \ref CY_SCB_I2C_DEFAULT_TX is returned to the master.
* * If the master tries to read more bytes than available in the Read buffer,
*   a \ref CY_SCB_I2C_SLAVE_RD_BUF_EMPTY_EVENT event occurs. The
*   \ref CY_SCB_I2C_DEFAULT_TX is returned to the master if the buffer remains
*   empty after an event notification.
*
*******************************************************************************/
void Cy_SCB_I2C_SlaveConfigReadBuf(volatile stc_SCB_t const *base, uint8_t *rdBuf, uint32_t size,
                                cy_stc_scb_i2c_context_t *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    CY_ASSERT( ((NULL == rdBuf) && (0u == size)) || (NULL != rdBuf) );

    context->slaveTxBuffer     = rdBuf;
    context->slaveTxBufferSize = size;
    context->slaveTxBufferIdx  = 0ul;
    context->slaveTxBufferCnt  = 0ul;
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_SlaveAbortRead
****************************************************************************//**
*
* Aborts the configured slave read buffer to be read by the master.
* If the master reads and "abort operation" is requested, the
* \ref CY_SCB_I2C_SLAVE_RD_BUF_EMPTY_EVENT event occurs. The
* \ref CY_SCB_I2C_DEFAULT_TX is returned to the master if the buffer remains
* empty after the event notification.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \sideeffect
* If the TX FIFO is used, this function clears it.
* The TX FIFO clear operation also clears the shift register, thus
* the shifter can be cleared in the middle of a data element transfer,
* corrupting it. The data element corruption means that all bits which has
* not been transmitted are transmitted as "ones" on the bus.
*
*******************************************************************************/
void Cy_SCB_I2C_SlaveAbortRead(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    uint32_t intrState;

    /* Suppress a compiler warning about unused variables */
    (void) base;

    intrState = Cy_SysLib_EnterCriticalSection();

    /* Reset index to make write buffer empty */
    context->slaveTxBufferSize = 0ul;

    if ((context->useTxFifo) &&
        (0ul != (CY_SCB_I2C_SLAVE_RD_BUSY & context->slaveStatus)))
    {
        /* Clear TX FIFO from available data */
        Cy_SCB_ClearTxFifo(base);
    }

    Cy_SysLib_ExitCriticalSection(intrState);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_SlaveGetReadTransferCount
****************************************************************************//**
*
* Returns the number of bytes read by the master since the last time
* \ref Cy_SCB_I2C_SlaveConfigReadBuf is called.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* The number of bytes read by the master.
*
* \note
* * This function returns an invalid value if read transaction was
*   aborted or any of listed events occurred during the transaction:
*   \ref CY_SCB_I2C_SLAVE_ARB_LOST, \ref CY_SCB_I2C_SLAVE_BUS_ERR.
* * This number is updated only when a transaction completes, either through
*   an error or successfully.
*
*******************************************************************************/
uint32_t Cy_SCB_I2C_SlaveGetReadTransferCount(volatile stc_SCB_t const *base, cy_stc_scb_i2c_context_t const *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    return (context->slaveTxBufferCnt);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_SlaveClearReadStatus
****************************************************************************//**
*
* Clears the read status and error conditions flags and returns their values.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref group_scb_i2c_macro_slave_status.
*
* \note
* The \ref CY_SCB_I2C_SLAVE_RD_BUSY flag is not cleared.
*
*******************************************************************************/
uint32_t Cy_SCB_I2C_SlaveClearReadStatus(volatile stc_SCB_t const *base, cy_stc_scb_i2c_context_t *context)
{
    uint32_t retStatus;

    /* Suppress a compiler warning about unused variables */
    (void) base;

    retStatus = (context->slaveStatus & CY_SCB_I2C_SLAVE_RD_CLEAR);
    context->slaveStatus &= (uint32_t) ~CY_SCB_I2C_SLAVE_RD_CLEAR;

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_SlaveConfigWriteBuf
****************************************************************************//**
*
* Configures the buffer pointer and size of the write buffer. This is the buffer
* that the master writes data to. After this function is called data transfer
* from the master into the write buffer is handled by \ref Cy_SCB_I2C_Interrupt.
*
* When write transaction is completed (master generated Stop, ReStart or
* error occurred) the \ref CY_SCB_I2C_SLAVE_WR_BUSY status is cleared and
* the \ref CY_SCB_I2C_SLAVE_WR_CMPLT is set, also
* the \ref CY_SCB_I2C_SLAVE_WR_CMPLT_EVENT event is generated.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param wrBuf
* The pointer to buffer to store data written by the master.
*
* \param size
* Size of wrBuf.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \note
* * The write buffer must not be modified and stay allocated until it has been
*   written by the master.
* * If this function has not been called and the master tries to write data
*   the 1st byte is NAKed and discarded.
* * If the master writes more bytes than slave can stores in the write buffer
*   the \ref CY_SCB_I2C_SLAVE_WR_OVRFL status is set and slave will NACK last
*   byte, unless the RX FIFO is used. Then the slave will NAK only after
*   RX FIFO becomes full.
* * If the RX FIFO is used the minimum write buffer size is automatically
*   the size of the RX FIFO. If write buffer is less than RX FIFO size extra
*   bytes are ACKed and stored into RX FIFO but ignored by firmware.
*
*******************************************************************************/
void Cy_SCB_I2C_SlaveConfigWriteBuf(volatile stc_SCB_t const *base, uint8_t *wrBuf, uint32_t size,
                                 cy_stc_scb_i2c_context_t *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    CY_ASSERT( ((NULL == wrBuf) && (0u == size)) || (NULL != wrBuf) );

    context->slaveRxBuffer     = wrBuf;
    context->slaveRxBufferSize = size;
    context->slaveRxBufferIdx  = 0ul;
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_SlaveAbortWrite
****************************************************************************//**
*
* Aborts the configured slave write buffer to be written by the master.
* If master writes and "abort operation" is requested the next incoming byte will
* be NAKed.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \note
* If the RX FIFO is used the NAK will not be sent until RX FIFO
* becomes full however bytes accepted after abort request are ignored.
*
*******************************************************************************/
void Cy_SCB_I2C_SlaveAbortWrite(volatile stc_SCB_t *base,  cy_stc_scb_i2c_context_t *context)
{
    uint32_t intrState;

    /* Suppress a compiler warning about unused variables */
    (void) base;

    intrState = Cy_SysLib_EnterCriticalSection();

    /* Reset index to make read buffer empty */
    context->slaveRxBufferSize = 0ul;

    if ((context->useRxFifo) &&
        (0ul != (CY_SCB_I2C_SLAVE_WR_BUSY & context->slaveStatus)))
    {
        /* Configure to NACK when RX FIFO is full and disable RX level
        * interrupt sources to stop getting data from RX FIFO.
        */
        base->unI2C_CTRL.stcField.u1S_NOT_READY_DATA_NACK = 1ul;
        Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);
    }

    Cy_SysLib_ExitCriticalSection(intrState);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_SlaveGetWriteTransferCount
****************************************************************************//**
*
* Returns the number of bytes written by the master since the last time
* \ref Cy_SCB_I2C_SlaveConfigWriteBuf is called.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* Number of bytes written by the master.
*
* \note
* * This function returns an invalid value if write transaction was
*   aborted or any of listed events occurred during the transaction:
*   \ref CY_SCB_I2C_SLAVE_ARB_LOST, \ref CY_SCB_I2C_SLAVE_BUS_ERR.
* * This number is only updated when the transaction completes, either through
*   an error or successfully.
*
*******************************************************************************/
uint32_t Cy_SCB_I2C_SlaveGetWriteTransferCount(volatile stc_SCB_t const *base, cy_stc_scb_i2c_context_t const *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    return (context->slaveRxBufferIdx);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_SlaveClearWriteStatus
****************************************************************************//**
*
* Clears the write status flags and  error conditions flags returns and their
* values.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref group_scb_i2c_macro_slave_status.
*
* \note
* The \ref CY_SCB_I2C_SLAVE_WR_BUSY flag is not cleared.
*
*******************************************************************************/
uint32_t Cy_SCB_I2C_SlaveClearWriteStatus(volatile stc_SCB_t const *base, cy_stc_scb_i2c_context_t *context)
{
    uint32_t retStatus;

    /* Suppress a compiler warning about unused variables */
    (void) base;

    retStatus = (context->slaveStatus & CY_SCB_I2C_SLAVE_WR_CLEAR);
    context->slaveStatus &= (uint32_t) ~CY_SCB_I2C_SLAVE_WR_CLEAR;

    return (retStatus);
}


/*******************************************************************************
*                         I2C Master API: High level
*******************************************************************************/

/*******************************************************************************
* Function Name: Cy_SCB_I2C_MasterGetStatus
****************************************************************************//**
*
* Returns the current I2C master status.
* This status is a bit mask and the value returned may have multiple bits set.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref group_scb_i2c_macro_master_status.
* Note that not all I2C master statuses are returned by this function. Refer to
* more details of each status.
*
* \note
* Status is cleared by calling \ref Cy_SCB_I2C_MasterRead or
* \ref Cy_SCB_I2C_MasterWrite.
*
*******************************************************************************/
uint32_t Cy_SCB_I2C_MasterGetStatus(volatile stc_SCB_t const *base, cy_stc_scb_i2c_context_t const *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    return (context->masterStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_MasterRead
****************************************************************************//**
*
* This function configures the master to automatically read an entire buffer
* of data from the slave device. After the transaction is initiated by this
* function it returns and \ref Cy_SCB_I2C_Interrupt manages further data
* transfer.
*
* When a read transaction is completed (requested number of bytes are read or
* error occurred) the \ref CY_SCB_I2C_MASTER_BUSY status is cleared and
* the \ref CY_SCB_I2C_MASTER_RD_CMPLT_EVENT event is generated.
*
* Note that the master must read at least one byte.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param masterXferConfig
* Master transfer configuration structure
* \ref cy_stc_scb_i2c_master_xfer_config_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_scb_i2c_status_t
*
* \note
* The buffer must not be modified and stay allocated until read operation
* completion.
*
*******************************************************************************/
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterRead(volatile stc_SCB_t *base,
                            cy_stc_scb_i2c_master_xfer_config_t *masterXferConfig,
                            cy_stc_scb_i2c_context_t *context)
{
    cy_en_scb_i2c_status_t retStatus = CY_SCB_I2C_MASTER_NOT_READY;
    un_SCB_I2C_M_CMD_t temp_I2C_M_CMD;

    /* Disable I2C slave interrupt sources to protect state */
    Cy_SCB_SetSlaveInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

    if (0ul != (CY_SCB_I2C_IDLE_MASK & context->state))
    {
        uint32_t intrState;

        /* Setup context */
        context->masterStatus     = CY_SCB_I2C_MASTER_BUSY;

        context->masterBuffer     = masterXferConfig->buffer;
        context->masterBufferSize = masterXferConfig->bufferSize;
        context->masterBufferIdx  = 0ul;
        context->masterNumBytes   = 0ul;
        context->masterPause      = masterXferConfig->xferPending;
        context->masterRdDir      = true;

        /* Clean-up hardware before transfer. Note RX FIFO is empty at here */
        Cy_SCB_ClearMasterInterrupt(base, CY_SCB_I2C_MASTER_INTR_ALL);
        Cy_SCB_ClearTxFifo(base);

        if (CY_SCB_I2C_IDLE == context->state)
        {
            /* Generate Start condition */
            temp_I2C_M_CMD.u32Register                = 0ul;
            temp_I2C_M_CMD.stcField.u1M_START_ON_IDLE = 1ul;
            base->unI2C_M_CMD.u32Register             = temp_I2C_M_CMD.u32Register;
        }
        else
        {
            /* Generate ReStart condition. Note if previous transfer was read,
            * NACK is generated before ReStart to complete it.
            */
            temp_I2C_M_CMD.u32Register        = 0ul;
            temp_I2C_M_CMD.stcField.u1M_START = 1ul;
            temp_I2C_M_CMD.stcField.u1M_NACK  = ((base->unI2C_STATUS.stcField.u1M_READ) ? 1ul : 0ul);
            base->unI2C_M_CMD.u32Register     = temp_I2C_M_CMD.u32Register;
        }

        /* Put address in TX FIFO */
        Cy_SCB_WriteTxFifo(base, (CY_SCB_I2C_READ_XFER | _VAL2FLD(CY_SCB_I2C_ADDRESS, masterXferConfig->slaveAddress)));

        /* Configure interrupt for data reception */
        if ((context->useRxFifo) && (!context->masterPause) && (context->masterBufferSize >= 2ul))
        {
            /* Enable Auto data ACK */
            base->unI2C_CTRL.stcField.u1M_READY_DATA_ACK = 1ul;

            /* Adjust level in RX FIFO */
            Cy_SCB_SetRxFifoLevel(base, (context->masterBufferSize <= CY_SCB_I2C_FIFO_SIZE) ?
                                        (context->masterBufferSize - 2ul) : (CY_SCB_I2C_HALF_FIFO_SIZE - 1ul));

            context->state = CY_SCB_I2C_MASTER_RX1;
        }
        else
        {
            /* Adjust level in RX FIFO */
            Cy_SCB_SetRxFifoLevel(base, 0ul);

            context->state = CY_SCB_I2C_MASTER_RX0;
        }

        /* Enable interrupt sources to continue transfer.
        * Requires critical section to not cause race condition between RX and Master
        * interrupt sources.
        */
        intrState = Cy_SysLib_EnterCriticalSection();
        Cy_SCB_SetRxInterruptMask    (base, CY_SCB_RX_INTR_LEVEL);
        Cy_SCB_SetMasterInterruptMask(base, CY_SCB_I2C_MASTER_INTR);
        Cy_SysLib_ExitCriticalSection(intrState);

        retStatus = CY_SCB_I2C_SUCCESS;
    }

    /* Enable I2C slave interrupt sources */
    Cy_SCB_SetSlaveInterruptMask(base, CY_SCB_I2C_SLAVE_INTR);

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_MasterAbortRead
****************************************************************************//**
*
* This function requests master to abort read operation by NAKing the next byte
* and generating a Stop condition. The function does not wait until these
* actions are completed therefore next read operation can be initiated only
* after the \ref CY_SCB_I2C_MASTER_BUSY is cleared.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
******************************************************************************/
void Cy_SCB_I2C_MasterAbortRead(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    uint32_t intrState;
    un_SCB_I2C_M_CMD_t temp_I2C_M_CMD;

    intrState = Cy_SysLib_EnterCriticalSection();

    if (0ul != (CY_SCB_I2C_MASTER_BUSY & context->masterStatus))
    {
        if ((CY_SCB_I2C_MASTER_RX0 == context->state) || (CY_SCB_I2C_MASTER_RX1 == context->state))
        {
            if (context->useRxFifo)
            {
                /* Disable RX processing */
                Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

                /* Change state and request Stop generation */
                context->state = CY_SCB_I2C_MASTER_STOP;

                /* Enable ACK interrupt source to generate Stop after Start was generated */
                Cy_SCB_SetMasterInterruptMask(base, CY_SCB_I2C_MASTER_INTR_ALL);
            }
            else
            {
                /* Reduce buffer to minimum value and request Stop generation */
                context->masterPause = false;
                context->masterBufferSize = 1ul;
            }
        }
    }
    else
    {
        /* Master waits for ReStart */
        if (CY_SCB_I2C_MASTER_WAIT == context->state)
        {
            /* Generate NAK + Stop */
            temp_I2C_M_CMD.u32Register       = 0ul;
            temp_I2C_M_CMD.stcField.u1M_STOP = 1ul;
            temp_I2C_M_CMD.stcField.u1M_NACK = 1ul;
            base->unI2C_M_CMD.u32Register    = temp_I2C_M_CMD.u32Register;
        }
    }

    Cy_SysLib_ExitCriticalSection(intrState);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_MasterWrite
****************************************************************************//**
*
* This function configures the master to automatically write an entire buffer
* of data to a slave device. After the transaction is initiated by this
* function it returns and \ref Cy_SCB_I2C_Interrupt manages further data
* transfer.
*
* When a write transaction is completed (requested number of bytes are written
* or error occurred) the \ref CY_SCB_I2C_MASTER_BUSY status is cleared and
* the \ref CY_SCB_I2C_MASTER_WR_CMPLT_EVENT event is generated.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param masterXferConfig
* Master transfer configuration structure
* \ref cy_stc_scb_i2c_master_xfer_config_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_scb_i2c_status_t
*
* \note
* The buffer must not be modified and stay allocated until data has been
* copied into TX FIFO.
*
*******************************************************************************/
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterWrite(volatile stc_SCB_t *base,
                             cy_stc_scb_i2c_master_xfer_config_t *masterXferConfig,
                             cy_stc_scb_i2c_context_t *context)
{
    cy_en_scb_i2c_status_t retStatus = CY_SCB_I2C_MASTER_NOT_READY;
    un_SCB_I2C_M_CMD_t temp_I2C_M_CMD;

    /* Disable I2C slave interrupt sources to protect state */
    Cy_SCB_SetSlaveInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

    if (0ul != (CY_SCB_I2C_IDLE_MASK & context->state))
    {
        uint32_t intrState;

        /* Setup context */
        context->masterStatus     = CY_SCB_I2C_MASTER_BUSY;

        context->masterBuffer     = masterXferConfig->buffer;
        context->masterBufferSize = masterXferConfig->bufferSize;
        context->masterBufferIdx  = 0ul;
        context->masterNumBytes   = 0ul;
        context->masterPause      = masterXferConfig->xferPending;
        context->masterRdDir      = false;

        /* Clean-up hardware before transfer. Note RX FIFO is empty at here */
        Cy_SCB_ClearMasterInterrupt(base, CY_SCB_I2C_MASTER_INTR_ALL);
        Cy_SCB_ClearTxFifo(base);

        if (CY_SCB_I2C_IDLE == context->state)
        {
            /* Generate Start condition */
            temp_I2C_M_CMD.u32Register                = 0ul;
            temp_I2C_M_CMD.stcField.u1M_START_ON_IDLE = 1ul;
            base->unI2C_M_CMD.u32Register             = temp_I2C_M_CMD.u32Register;
        }
        else
        {
            /* Generate ReStart condition. Note if previous transfer was read,
            * NACK is generated before ReStart to complete it.
            */
            temp_I2C_M_CMD.u32Register        = 0ul;
            temp_I2C_M_CMD.stcField.u1M_START = 1ul;
            temp_I2C_M_CMD.stcField.u1M_NACK  = ((base->unI2C_STATUS.stcField.u1M_READ) ? 1ul : 0ul);
            base->unI2C_M_CMD.u32Register     = temp_I2C_M_CMD.u32Register;
        }

        /* Put address in TX FIFO */
        Cy_SCB_WriteTxFifo(base, _VAL2FLD(CY_SCB_I2C_ADDRESS, masterXferConfig->slaveAddress));

        context->state = CY_SCB_I2C_MASTER_TX;

        /* TX FIFO is empty. Set level to start transfer */
        Cy_SCB_SetTxFifoLevel(base, (context->useTxFifo) ? CY_SCB_I2C_HALF_FIFO_SIZE : (1ul));

        /* Enable interrupt sources to continue transfer.
        * Requires critical section to not cause race condition between TX and Master
        * interrupt sources.
        */
        intrState = Cy_SysLib_EnterCriticalSection();
        Cy_SCB_SetTxInterruptMask    (base, CY_SCB_TX_INTR_LEVEL);
        Cy_SCB_SetMasterInterruptMask(base, CY_SCB_I2C_MASTER_INTR);
        Cy_SysLib_ExitCriticalSection(intrState);

        retStatus = CY_SCB_I2C_SUCCESS;
    }

    /* Enable I2C slave interrupt sources */
    Cy_SCB_SetSlaveInterruptMask(base, CY_SCB_I2C_SLAVE_INTR);

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_MasterAbortWrite
****************************************************************************//**
*
* This function requests master to abort write operation by generating Stop
* condition. The function does not wait until this action is completed,
* therefore next write operation can be initiated only after the
* \ref CY_SCB_I2C_MASTER_BUSY is cleared.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \sideeffect
* If the TX FIFO is used it is cleared before Stop generation.
* The TX FIFO clear operation also clears shift register thus shifter
* could be cleared in the middle of a data element transfer, corrupting it.
* The remaining bits to transfer within corrupted data element are
* complemented with ones.
*
*******************************************************************************/
void Cy_SCB_I2C_MasterAbortWrite(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    uint32_t intrState;
    un_SCB_I2C_M_CMD_t temp_I2C_M_CMD;

    intrState = Cy_SysLib_EnterCriticalSection();

    if (0ul != (CY_SCB_I2C_MASTER_BUSY & context->masterStatus))
    {
        /* Disable TX processing */
        Cy_SCB_SetTxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

        if (context->useTxFifo)
        {
            /* Clear TX FIFO to allow Stop generation */
            Cy_SCB_ClearTxFifo(base);
        }

        if ((CY_SCB_I2C_MASTER_TX == context->state) || (CY_SCB_I2C_MASTER_TX_DONE == context->state))
        {
            /* Request Stop generation */
            context->state = CY_SCB_I2C_MASTER_STOP;

            /* Enable ACK interrupt source to trigger Stop generation */
            Cy_SCB_SetMasterInterruptMask(base, CY_SCB_I2C_MASTER_INTR_ALL);
        }
    }
    else
    {
        /* Master waits for ReStart */
        if (CY_SCB_I2C_MASTER_WAIT == context->state)
        {
            /* Generate Stop */
            temp_I2C_M_CMD.u32Register       = 0ul;
            temp_I2C_M_CMD.stcField.u1M_STOP = 1ul;
            temp_I2C_M_CMD.stcField.u1M_NACK = 1ul;
            base->unI2C_M_CMD.u32Register    = temp_I2C_M_CMD.u32Register;
        }
    }

    Cy_SysLib_ExitCriticalSection(intrState);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_MasterGetTransferCount
****************************************************************************//**
*
* Returns the number of bytes transferred since the last call of
* \ref Cy_SCB_I2C_MasterWrite or \ref Cy_SCB_I2C_MasterRead function.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* Number of bytes read or written by the master.
*
* \note
* * This function returns an invalid value if read or write transaction was
*   aborted or any of listed events occurred during the transaction:
*   \ref CY_SCB_I2C_MASTER_ARB_LOST, \ref CY_SCB_I2C_MASTER_BUS_ERR or
*   \ref CY_SCB_I2C_MASTER_ABORT_START.
*
* * This number is only updated when the transaction completes, either through
*   an error or successfully.
*
*******************************************************************************/
uint32_t Cy_SCB_I2C_MasterGetTransferCount(volatile stc_SCB_t const *base, cy_stc_scb_i2c_context_t const *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    return (context->masterNumBytes);
}


/*******************************************************************************
*                         I2C Master API: Low level
*******************************************************************************/


/*******************************************************************************
* Function Name: Cy_SCB_I2C_MasterSendStart
****************************************************************************//**
*
* Generates a Start condition and sends a slave address with the Read/Write bit.
* This function is blocking and it does not return until the Start condition
* and address byte are sent and a ACK/NAK is received, an error or timeout
* occurred.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param address
* 7 bit right justified slave address.
*
* \param bitRnW
* This sets the value of the Read/Write bit in the address, thus defining
* the direction of the following transfer.
* See \ref group_scb_i2c_macro_direction for the set of constants.
*
* \param timeoutMs
* Defines in milliseconds the time that this function can block for.
* If that time expires, the function returns. If a zero is passed,
* the function waits forever for the action to complete. If a timeout occurs,
* the SCB block is reset. Note The maximum value is sizeof(uint32_t)/1000.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_scb_i2c_status_t
*
* \note
* After a Read transaction is initiated and the slave ACKs the address, at
* least one byte must be read before completing the transaction or changing
* its direction.
*
*******************************************************************************/
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterSendStart(volatile stc_SCB_t *base, uint32_t address,
                                    uint32_t bitRnW, uint32_t timeoutMs,
                                    cy_stc_scb_i2c_context_t *context)
{
    cy_en_scb_i2c_status_t retStatus = CY_SCB_I2C_MASTER_NOT_READY;
    un_SCB_I2C_M_CMD_t temp_I2C_M_CMD;

    /* Disable the I2C slave interrupt sources to protect the state */
    Cy_SCB_SetSlaveInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

    if (CY_SCB_I2C_IDLE == context->state)
    {
        uint32_t locStatus;

        /* Convert the timeout to microseconds */
        uint32_t timeout = (timeoutMs * 1000ul);

        /* Set the read or write direction */
        context->state = CY_SCB_I2C_MASTER_ADDR;
        context->masterRdDir = (CY_SCB_I2C_READ_XFER == bitRnW);

        /* Clean up the hardware before a transfer. Note RX FIFO is empty at here */
        Cy_SCB_ClearMasterInterrupt(base, CY_SCB_I2C_MASTER_INTR_ALL);
        Cy_SCB_ClearRxInterrupt(base, CY_SCB_RX_INTR_NOT_EMPTY);
        Cy_SCB_ClearTxFifo(base);

        /* Generate a Start and send address byte */
        Cy_SCB_WriteTxFifo(base, (_VAL2FLD(CY_SCB_I2C_ADDRESS, address) | bitRnW));

        temp_I2C_M_CMD.u32Register                = 0ul;
        temp_I2C_M_CMD.stcField.u1M_START_ON_IDLE = 1ul;
        base->unI2C_M_CMD.u32Register             = temp_I2C_M_CMD.u32Register;

        /* Wait for a completion event from the master or slave */
        do
        {
            locStatus = ((CY_SCB_I2C_MASTER_TX_BYTE_DONE & Cy_SCB_GetMasterInterruptStatus(base)) |
                         (CY_SCB_I2C_SLAVE_ADDR_DONE & Cy_SCB_GetSlaveInterruptStatus(base)));

            locStatus |= WaitOneUnit(&timeout);

        } while (0ul == locStatus);

        retStatus = HandleStatus(base, locStatus, context);
    }

    /* Enable I2C slave interrupt sources */
    Cy_SCB_SetSlaveInterruptMask(base, CY_SCB_I2C_SLAVE_INTR);

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_MasterSendReStart
****************************************************************************//**
*
* Generates a ReStart condition and sends a slave address with the Read/Write
* bit.
* This function is blocking and it does not return until the ReStart condition
* and address byte are sent and an ACK/NAK is received, an error or timeout
* occurred.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param address
* A 7-bit right-justified slave address.
*
* \param bitRnW
* This sets the value of the Read/Write bit in the address, thus defining
* the direction of the following transfer.
* See \ref group_scb_i2c_macro_direction for the set of constants.
*
* \param timeoutMs
* Defines in milliseconds the time that this function can block for.
* If that time expires, the function returns. If a zero is passed,
* the function waits forever for the action to complete. If a timeout occurs,
* the SCB block is reset. Note The maximum value is sizeof(uint32_t)/1000.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_scb_i2c_status_t
*
* \note
* * A successful transaction must be initiated by \ref Cy_SCB_I2C_MasterSendStart
*   before calling this function. If this condition is not met, this function
*   does nothing and returns \ref CY_SCB_I2C_MASTER_NOT_READY.
* * After a Read transaction is initiated and the slave ACKs the address,
*   at least one byte must be read before completing the transaction or
*   changing its direction.
*
*******************************************************************************/
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterSendReStart(volatile stc_SCB_t *base, uint32_t address,
                                    uint32_t bitRnW, uint32_t timeoutMs,
                                    cy_stc_scb_i2c_context_t *context)
{
    cy_en_scb_i2c_status_t retStatus = CY_SCB_I2C_MASTER_NOT_READY;
    un_SCB_I2C_M_CMD_t temp_I2C_M_CMD;

    if (0ul != (CY_SCB_I2C_MASTER_ACTIVE & context->state))
    {
        uint32_t locStatus;

        /* Convert the timeout to microseconds */
        uint32_t timeout = (timeoutMs * 1000ul);

        /* Set the read or write direction */
        context->state = CY_SCB_I2C_MASTER_ADDR;
        context->masterRdDir = (CY_SCB_I2C_READ_XFER == bitRnW);

        /* Generate a restart (for write direction) and NACK plus restart for the Read direction */
        temp_I2C_M_CMD.u32Register        = 0ul;
        temp_I2C_M_CMD.stcField.u1M_START = 1ul;
        temp_I2C_M_CMD.stcField.u1M_NACK  = ((base->unI2C_STATUS.stcField.u1M_READ) ? 1ul : 0ul);
        base->unI2C_M_CMD.u32Register     = temp_I2C_M_CMD.u32Register;

        /* Send the address byte */
        Cy_SCB_WriteTxFifo(base, (_VAL2FLD(CY_SCB_I2C_ADDRESS, address) | bitRnW));

        /* Wait for a completion event from the or slave */
        do
        {
            locStatus = ((CY_SCB_I2C_MASTER_TX_BYTE_DONE & Cy_SCB_GetMasterInterruptStatus(base)) |
                         (CY_SCB_I2C_SLAVE_ADDR_DONE & Cy_SCB_GetSlaveInterruptStatus(base)));

            locStatus |= WaitOneUnit(&timeout);

        } while (0ul == locStatus);

        /* Convert the status from register plus timeout to the API status */
        retStatus = HandleStatus(base, locStatus, context);
    }

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_MasterSendStop
****************************************************************************//**
*
* Generates a Stop condition to complete the current transaction.
* This function is blocking and it does not return until the Stop condition
* is generated, an error occurred or timeout occurred.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param timeoutMs
* Defines in milliseconds the time that this function can block for.
* If that time expires, the function returns. If a zero is passed,
* the function waits forever for the action to complete. If a timeout occurs,
* the SCB block is reset. Note The maximum value is sizeof(uint32_t)/1000.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_scb_i2c_status_t
*
* \note
* * A successful transaction must be initiated by
*   \ref Cy_SCB_I2C_MasterSendStart or \ref Cy_SCB_I2C_MasterSendReStart
*   before calling this function. If this condition is not met, this function
*   does nothing and returns.
*   \ref CY_SCB_I2C_MASTER_NOT_READY.
* * Even after the slave NAKs the address, this function has to be called
*   to complete the transaction.
*
*******************************************************************************/
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterSendStop(volatile stc_SCB_t *base,uint32_t timeoutMs,
                                cy_stc_scb_i2c_context_t *context)
{
    cy_en_scb_i2c_status_t retStatus = CY_SCB_I2C_MASTER_NOT_READY;
    un_SCB_I2C_M_CMD_t temp_I2C_M_CMD;

    if (0ul != (CY_SCB_I2C_MASTER_ACTIVE & context->state))
    {
        uint32_t locStatus;

        /* Convert the timeout to microseconds */
        uint32_t timeout = (timeoutMs * 1000ul);

        /* Generate a stop (for Write direction) and NACK plus stop for the Read direction */
        temp_I2C_M_CMD.u32Register       = 0ul;
        temp_I2C_M_CMD.stcField.u1M_STOP = 1ul;
        temp_I2C_M_CMD.stcField.u1M_NACK = 1ul;
        base->unI2C_M_CMD.u32Register    = temp_I2C_M_CMD.u32Register;

        /* Wait for a completion event from the master or slave */
        do
        {
            locStatus = (CY_SCB_I2C_MASTER_STOP_DONE & Cy_SCB_GetMasterInterruptStatus(base));

            locStatus |= WaitOneUnit(&timeout);

        } while (0ul == locStatus);

        /* Convert the status from register plus timeout to the API status */
        retStatus = HandleStatus(base, locStatus, context);
    }

    return (retStatus);
}

cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterSendWriteStop(volatile stc_SCB_t *base,uint32_t timeoutMs,
                                cy_stc_scb_i2c_context_t *context)
{
    cy_en_scb_i2c_status_t retStatus = CY_SCB_I2C_MASTER_NOT_READY;

    if (0ul != (CY_SCB_I2C_MASTER_ACTIVE & context->state))
    {
        uint32_t locStatus;

        /* Convert the timeout to microseconds */
        uint32_t timeout = (timeoutMs * 1000ul);

        /* Generate a stop for Write direction */
        base->unI2C_M_CMD.stcField.u1M_STOP = 1ul;

        /* Wait for a completion event from the master or slave */
        do
        {
            locStatus = (CY_SCB_I2C_MASTER_STOP_DONE & Cy_SCB_GetMasterInterruptStatus(base));

            locStatus |= WaitOneUnit(&timeout);

        } while (0ul == locStatus);

        /* Convert the status from register plus timeout to the API status */
        retStatus = HandleStatus(base, locStatus, context);
    }

    return (retStatus);
}

cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterSendReadStop(volatile stc_SCB_t *base,uint32_t timeoutMs,
                                cy_stc_scb_i2c_context_t *context)
{
    cy_en_scb_i2c_status_t retStatus = CY_SCB_I2C_MASTER_NOT_READY;

    if (0ul != (CY_SCB_I2C_MASTER_ACTIVE & context->state))
    {
        uint32_t locStatus;

        /* Convert the timeout to microseconds */
        uint32_t timeout = (timeoutMs * 1000ul);

        /* Generate a NACK plus for the Read direction */
        base->unI2C_M_CMD.stcField.u1M_NACK = 1ul;

        /* Wait for a completion event from the master or slave */
        do
        {
            locStatus = (CY_SCB_I2C_MASTER_STOP_DONE & Cy_SCB_GetMasterInterruptStatus(base));

            locStatus |= WaitOneUnit(&timeout);

        } while (0ul == locStatus);

        /* Convert the status from register plus timeout to the API status */
        retStatus = HandleStatus(base, locStatus, context);
    }

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_MasterReadByte
****************************************************************************//**
*
* Reads one byte from a slave and generates an ACK or prepares to generate
* a NAK. The NAK will be generated before a Stop or ReStart condition by
* \ref Cy_SCB_I2C_MasterSendStop or \ref Cy_SCB_I2C_MasterSendReStart function
* appropriately.
* This function is blocking and it does not return until a byte is
* received, an error occurred or timeout occurred.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param ackNack
* A response to a received byte.
* See \ref group_scb_i2c_macro_ack_nack for the set of constants.
*
* \param byte
* The pointer to the location to store the Read byte.
*
* \param timeoutMs
* Defines in milliseconds the time that this function can block for.
* If that time expires, the function returns. If a zero is passed,
* the function waits forever for the action to complete. If a timeout occurs,
* the SCB block is reset. Note The maximum value is sizeof(uint32_t)/1000.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_scb_i2c_status_t
*
* \note
* A successful transaction must be initiated by \ref Cy_SCB_I2C_MasterSendStart
* or \ref Cy_SCB_I2C_MasterSendReStart before calling this function. If this
* condition is not met, this function does nothing and returns
* \ref CY_SCB_I2C_MASTER_NOT_READY.
*
*******************************************************************************/
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterReadByte(volatile stc_SCB_t *base, uint32_t ackNack,
                                uint8_t *byte, uint32_t timeoutMs,
                                cy_stc_scb_i2c_context_t *context)
{
    cy_en_scb_i2c_status_t retStatus = CY_SCB_I2C_MASTER_NOT_READY;
    un_SCB_I2C_M_CMD_t temp_I2C_M_CMD;

    if (CY_SCB_I2C_MASTER_RX0 == context->state)
    {
        bool rxEmpty;

        uint32_t locStatus;

        /* Convert the timeout to microseconds */
        uint32_t timeout = (timeoutMs * 1000ul);

        /* Wait for ACK/NAK transmission and data byte reception */
        do
        {
            locStatus = (CY_SCB_I2C_MASTER_RX_BYTE_DONE & Cy_SCB_GetMasterInterruptStatus(base));
            rxEmpty   = (0ul == (CY_SCB_RX_INTR_NOT_EMPTY & Cy_SCB_GetRxInterruptStatus(base)));

            locStatus |= WaitOneUnit(&timeout);

        } while ((rxEmpty) && (0ul == locStatus));

        /* The Read byte if available */
        if (!rxEmpty)
        {
            /* Get the received data byte */
            *byte = (uint8_t) Cy_SCB_ReadRxFifo(base);

            Cy_SCB_ClearRxInterrupt(base, CY_SCB_RX_INTR_NOT_EMPTY | CY_SCB_RX_INTR_LEVEL);
        }

        /* Convert the status from register plus timeout to the API status */
        retStatus = HandleStatus(base, locStatus, context);

        if (CY_SCB_I2C_SUCCESS == retStatus)
        {
            /* Generate ACK or wait for NAK generation */
            if (CY_SCB_I2C_ACK == ackNack)
            {
                temp_I2C_M_CMD.u32Register      = 0ul;
                temp_I2C_M_CMD.stcField.u1M_ACK = 1ul;
                base->unI2C_M_CMD.u32Register   = temp_I2C_M_CMD.u32Register;
            }
        }
    }

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_MasterWriteByte
****************************************************************************//**
*
* Sends one byte to a slave.
* This function is blocking and it does not return until a byte is
* transmitted, an error occurred or timeout occurred.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param theByte
* The byte to write to a slave.
*
* \param timeoutMs
* Defines in milliseconds the time that this function can block for.
* If that time expires, the function returns. If a zero is passed,
* the function waits forever for the action to complete. If a timeout occurs,
* the SCB block is reset. Note The maximum value is sizeof(uint32_t)/1000.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_scb_i2c_status_t
*
* \note
* A successful transaction must be initiated by \ref Cy_SCB_I2C_MasterSendStart
* or \ref Cy_SCB_I2C_MasterSendReStart before calling this function. If this
* condition is not met, this function does nothing and returns
* \ref CY_SCB_I2C_MASTER_NOT_READY.
*
*******************************************************************************/
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterWriteByte(volatile stc_SCB_t *base, uint8_t theByte,
                                    uint32_t timeoutMs,
                                    cy_stc_scb_i2c_context_t *context)
{
    cy_en_scb_i2c_status_t retStatus = CY_SCB_I2C_MASTER_NOT_READY;

    if (CY_SCB_I2C_MASTER_TX == context->state)
    {
        uint32_t locStatus;

        /* Convert the timeout to microseconds */
        uint32_t timeout = (timeoutMs * 1000ul);

        /* Send the data byte */
        Cy_SCB_WriteTxFifo(base, (uint32_t) theByte);

        /* Wait for a completion event from the master or slave */
        do
        {
            locStatus  = (CY_SCB_I2C_MASTER_TX_BYTE_DONE & Cy_SCB_GetMasterInterruptStatus(base));
            locStatus |= WaitOneUnit(&timeout);

        } while (0ul == locStatus);

        /* Convert the status from register plus timeout to the API status */
        retStatus = HandleStatus(base, locStatus, context);
    }

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_Interrupt
****************************************************************************//**
*
* This is the interrupt function for the SCB configured in the I2C mode.
* This function must be called inside the  user-defined interrupt service
* routine in order for higher-level functions to work:
* * Slave: Any of the slave functions.
* * Master: \ref Cy_SCB_I2C_MasterRead and \ref Cy_SCB_I2C_MasterWrite.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
void Cy_SCB_I2C_Interrupt(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    if (0ul != (CY_SCB_I2C_MASTER_ACTIVE & context->state))
    {
        /* Execute a transfer as the master */
        MasterInterrupt(base, context);
    }
    else
    {
        /* Execute a transfer as the slave */
        SlaveInterrupt(base, context);
    }
}


/*******************************************************************************
* Function Name: SlaveInterrupt
****************************************************************************//**
*
* This is the interrupt function to execute a slave transfer.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void SlaveInterrupt(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    uint32_t slaveIntrStatus;

    /* Handle an I2C wake-up event */
    if (0ul != (CY_SCB_I2C_INTR_WAKEUP & Cy_SCB_GetI2CInterruptStatusMasked(base)))
    {
        Cy_SCB_ClearI2CInterrupt(base, CY_SCB_I2C_INTR_WAKEUP);
    }

    /* Handle the slave interrupt sources */
    slaveIntrStatus = Cy_SCB_GetSlaveInterruptStatusMasked(base);

    /* Handle the error conditions */
    if (0ul != (CY_SCB_I2C_SLAVE_INTR_ERROR & slaveIntrStatus))
    {
        /* Update the status */
        context->slaveStatus |= (0ul != (CY_SCB_SLAVE_INTR_I2C_BUS_ERROR & slaveIntrStatus)) ?
                                            CY_SCB_I2C_SLAVE_BUS_ERR : CY_SCB_I2C_SLAVE_ARB_LOST;

        /* Disable the RX interrupt source to drop data into RX FIFO if any */
        Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

        /* Add the stop status to back into the default state and set completion statuses */
        slaveIntrStatus |= CY_SCB_SLAVE_INTR_I2C_STOP;
    }
    else
    {
        if (0ul != (CY_SCB_SLAVE_INTR_I2C_STOP & slaveIntrStatus))
        {
            /* Get data from the RX FIFO after a stop is generated if there is
            * space to store it.
            */
            if ((Cy_SCB_GetNumInRxFifo(base) > 0ul) && (context->slaveRxBufferSize > 0ul))
            {
                Cy_SCB_SetRxInterrupt    (base, CY_SCB_RX_INTR_LEVEL);
                Cy_SCB_SetRxInterruptMask(base, CY_SCB_RX_INTR_LEVEL);
            }
        }
    }

    /* Handle the receive direction (master writes data) */
    if (0ul != (CY_SCB_RX_INTR_LEVEL & Cy_SCB_GetRxInterruptStatusMasked(base)))
    {
        SlaveHandleDataReceive(base, context);

        Cy_SCB_ClearRxInterrupt(base, CY_SCB_RX_INTR_LEVEL);
    }

    /* Handle the transfer completion */
    if (0ul != (CY_SCB_SLAVE_INTR_I2C_STOP & slaveIntrStatus))
    {
        SlaveHandleStop(base, context);

        Cy_SCB_ClearSlaveInterrupt(base, CY_SCB_SLAVE_INTR_I2C_STOP);

        /* Update the slave interrupt status */
        slaveIntrStatus = Cy_SCB_GetSlaveInterruptStatusMasked(base);
    }

    /* Handle the address reception */
    if (0ul != (CY_SCB_I2C_SLAVE_INTR_ADDR & slaveIntrStatus))
    {
        SlaveHandleAddress(base, context);

        Cy_SCB_ClearI2CInterrupt(base, CY_SCB_I2C_INTR_WAKEUP);
        Cy_SCB_ClearSlaveInterrupt(base, CY_SCB_I2C_SLAVE_INTR_ADDR);
    }

    /* Handle the transmit direction (master reads data) */
    if (0ul != (CY_SCB_I2C_SLAVE_INTR_TX & Cy_SCB_GetTxInterruptStatusMasked(base)))
    {
        SlaveHandleDataTransmit(base, context);

        Cy_SCB_ClearTxInterrupt(base, CY_SCB_TX_INTR_LEVEL);
    }
}


/*******************************************************************************
* Function Name: SlaveHandleAddress
****************************************************************************//**
*
* Prepares the slave for the following Read or Write transfer after the
* matched address was received.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void SlaveHandleAddress(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    /* The default command is the ACK address. It can be overridden in an address callback */
    uint32_t cmd = CY_SCB_I2C_ACK;

    /* The callback for the address in RX FIFO or a general call */
    if (NULL != context->cbAddr)
    {
        uint32_t events = 0ul;

        /* Set an address in the FIFO event if the address accept is enabled */
        if (base->unCTRL.stcField.u1ADDR_ACCEPT)
        {
            events = (0ul != (CY_SCB_SLAVE_INTR_I2C_ADDR_MATCH & Cy_SCB_GetSlaveInterruptStatusMasked(base))) ?
                              CY_SCB_I2C_ADDR_IN_FIFO_EVENT : 0ul;
        }

        /* Set a general call event if "ignore general call" is disabled */
        if (!(base->unI2C_CTRL.stcField.u1S_GENERAL_IGNORE))
        {
            events |= (0ul != (CY_SCB_SLAVE_INTR_I2C_GENERAL_ADDR & Cy_SCB_GetSlaveInterruptStatusMasked(base))) ?
                               CY_SCB_I2C_GENERAL_CALL_EVENT : 0ul;
        }

        /* Check presence of events before involve callback */
        if (0ul != events)
        {
            /* Involve a callback for the address phase and get the ACK/NACK command */
            cmd = context->cbAddr(events);

            /* Clear RX level interrupt after address reception */
            Cy_SCB_ClearRxInterrupt(base, CY_SCB_RX_INTR_LEVEL);

            if (cmd == CY_SCB_I2C_ACK)
            {
                /* Clear the stall stop status and enable the stop interrupt source */
                Cy_SCB_ClearSlaveInterrupt(base, CY_SCB_SLAVE_INTR_I2C_STOP);
                Cy_SCB_SetSlaveInterruptMask(base, CY_SCB_I2C_SLAVE_INTR);
            }
            else
            {
                /* Disable the stop interrupt source */
                Cy_SCB_SetI2CInterruptMask(base, CY_SCB_I2C_SLAVE_INTR_NO_STOP);
            }
        }
    }

    /* Clear the TX FIFO before continue the transaction */
    Cy_SCB_ClearTxFifo(base);
                
    /* Set the command to an ACK or NACK address */
    base->unI2C_S_CMD.u32Register = cmd;

    if (cmd == CY_SCB_I2C_ACK)
    {
        bool readDirection = base->unI2C_STATUS.stcField.u1S_READ;

        /* Notify the user about start of transfer */
        if (NULL != context->cbEvents)
        {
            context->cbEvents(readDirection ? CY_SCB_I2C_SLAVE_READ_EVENT : CY_SCB_I2C_SLAVE_WRITE_EVENT);
        }

        /* Prepare for a transfer */
        if (readDirection)
        {
            context->state        = CY_SCB_I2C_SLAVE_TX;
            context->slaveStatus |= CY_SCB_I2C_SLAVE_RD_BUSY;

            /* Prepare to transmit data */
            context->slaveTxBufferIdx = context->slaveTxBufferCnt;
            context->slaveRdBufEmpty  = false;
            Cy_SCB_SetTxInterruptMask(base, CY_SCB_TX_INTR_LEVEL);
        }
        else
        {
            uint32_t level = 0ul;

            context->state        = CY_SCB_I2C_SLAVE_RX;
            context->slaveStatus |= CY_SCB_I2C_SLAVE_WR_BUSY;

            /* Prepare to receive data */
            Cy_SCB_SetRxInterruptMask(base, CY_SCB_RX_INTR_LEVEL);

            if (context->useRxFifo)
            {
                if (context->slaveRxBufferSize > 0ul)
                {
                    /* ACK data automatically until RX FIFO is full */
                    base->unI2C_CTRL.stcField.u1S_READY_DATA_ACK = 1ul;

                    if (context->slaveRxBufferSize > CY_SCB_I2C_FIFO_SIZE)
                    {
                        /* Set a level in RX FIFO to trigger the receive interrupt source */
                        level = (context->slaveRxBufferSize - CY_SCB_I2C_FIFO_SIZE);
                        level = (level > CY_SCB_I2C_FIFO_SIZE) ? ((CY_SCB_I2C_FIFO_SIZE / 2ul) - 1ul) : (level - 1ul);
                    }
                    else
                    {
                        /* Set a level in RX FIFO to read the number of bytes */
                        level = (context->slaveRxBufferSize - 1ul);

                        /* NACK when RX FIFO becomes full */
                        base->unI2C_CTRL.stcField.u1S_NOT_READY_DATA_NACK = 1ul;

                        /* Disable the RX level interrupt and wait until RX FIFO is full or stops */
                        Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);
                    }
                }
            }

            Cy_SCB_SetRxFifoLevel(base, level);
        }
    }
}


/*******************************************************************************
* Function Name: SlaveHandleDataReceive
****************************************************************************//**
*
* Reads data from RX FIFO into the buffer provided by
* \ref Cy_SCB_I2C_SlaveConfigWriteBuf.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void SlaveHandleDataReceive(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    un_SCB_I2C_S_CMD_t temp_I2C_S_CMD;

    /* Check if there is space to put data */
    if (context->slaveRxBufferSize > 0ul)
    {
        if (context->useRxFifo)
        {
            uint32_t level;

            /* Get the number of bytes to read from RX FIFO */
            uint32_t numToCopy = Cy_SCB_GetRxFifoLevel(base) + 1ul;

            /* Get data from RX FIFO */
            numToCopy = Cy_SCB_ReadArray(base, context->slaveRxBuffer, numToCopy);
            context->slaveRxBufferIdx  += numToCopy;
            context->slaveRxBufferSize -= numToCopy;
            context->slaveRxBuffer      = &context->slaveRxBuffer[numToCopy];

            /* Prepare to read a next chunk of data */
            if (context->slaveRxBufferSize > CY_SCB_I2C_FIFO_SIZE)
            {
                level = context->slaveRxBufferSize - CY_SCB_I2C_FIFO_SIZE;
                level = ((level > CY_SCB_I2C_FIFO_SIZE) ? (CY_SCB_I2C_FIFO_SIZE / 2ul) : level) - 1ul;
            }
            else
            {
                base->unI2C_CTRL.stcField.u1S_NOT_READY_DATA_NACK = 1ul;

                level = (context->slaveRxBufferSize == 0ul) ? (0ul) : (context->slaveRxBufferSize - 1ul);
                Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);
            }

            /* Set the RX level to trigger an interrupt */
            Cy_SCB_SetRxFifoLevel(base, level);
        }
        else
        {
            /* Continue the transfer: send an ACK */
            temp_I2C_S_CMD.u32Register      = 0ul;
            temp_I2C_S_CMD.stcField.u1S_ACK = 1ul;
            base->unI2C_S_CMD.u32Register   = temp_I2C_S_CMD.u32Register;

            /* Put data into the RX buffer */
            context->slaveRxBuffer[context->slaveRxBufferIdx] = (uint8_t) Cy_SCB_ReadRxFifo(base);
            ++context->slaveRxBufferIdx;
            --context->slaveRxBufferSize;
        }
    }
    else
    {
        /* Finish a transfer: send a NACK and discard the received byte */
        temp_I2C_S_CMD.u32Register       = 0ul;
        temp_I2C_S_CMD.stcField.u1S_NACK = 1ul;
        base->unI2C_S_CMD.u32Register    = temp_I2C_S_CMD.u32Register;
        Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);
    }
}


/*******************************************************************************
* Function Name: SlaveHandleDataTransmit
****************************************************************************//**
*
* Loads TX FIFO with data provided by \ref Cy_SCB_I2C_SlaveConfigReadBuf.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void SlaveHandleDataTransmit(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    uint32_t numToCopy;

    /* Notify the user that there is no data to send to the master.
    * This event triggers once in scope of a transfer.
    */
    if ((!context->slaveRdBufEmpty) && (0ul == context->slaveTxBufferSize))
    {
        /* Involve a callback if registered: no data to send */
        if (NULL != context->cbEvents)
        {
            context->cbEvents(CY_SCB_I2C_SLAVE_RD_BUF_EMPTY_EVENT);
        }

        /* Update the Read buffer empty status after a callback is involved */
        context->slaveRdBufEmpty = (0ul == context->slaveTxBufferSize);

        /* Enable the TX level interrupt source to continue sending data */
        Cy_SCB_SetTxInterruptMask(base, CY_SCB_TX_INTR_LEVEL);
    }

    /* Check if the Read buffer was updated in the callback */
    if (context->slaveRdBufEmpty)
    {
        /* The Read buffer is empty: copy CY_SCB_I2C_DEFAULT_TX into TX FIFO */
        numToCopy = (context->useTxFifo) ? Cy_SCB_GetFifoSize(base) : 1ul;

        numToCopy = Cy_SCB_WriteDefaultArray(base, CY_SCB_I2C_DEFAULT_TX, numToCopy);
        context->slaveTxBufferIdx += numToCopy;

        context->slaveStatus |= CY_SCB_I2C_SLAVE_RD_UNDRFL;
    }
    else
    {
        if (context->slaveTxBufferSize > 1ul)
        {
            /* Get the number of bytes to copy into TX FIFO */
            numToCopy = (context->useTxFifo) ? (context->slaveTxBufferSize - 1ul) : (1ul);

            /* Write data into TX FIFO */
            numToCopy = Cy_SCB_WriteArray(base, context->slaveTxBuffer, numToCopy);
            context->slaveTxBufferIdx  += numToCopy;
            context->slaveTxBufferSize -= numToCopy;
            context->slaveTxBuffer      = &context->slaveTxBuffer[numToCopy];
        }

        /* Put the last byte */
        if ((CY_SCB_I2C_FIFO_SIZE != Cy_SCB_GetNumInTxFifo(base)) && (1ul == context->slaveTxBufferSize))
        {
            uint32_t intrStatus;

            /* Put the last data element and make sure that "TX done" will happen for it */
            intrStatus = Cy_SysLib_EnterCriticalSection();

            Cy_SCB_WriteTxFifo(base, (uint32_t) context->slaveTxBuffer[0ul]);
            Cy_SCB_ClearTxInterrupt(base, CY_SCB_TX_INTR_UNDERFLOW);

            Cy_SysLib_ExitCriticalSection(intrStatus);

            /* Move the pointers */
            ++context->slaveTxBufferIdx;
            context->slaveTxBufferSize = 0ul;
            context->slaveTxBuffer     = &context->slaveTxBuffer[1ul];

            /* Enable the TX underflow interrupt to catch when there is no data to send */
            Cy_SCB_SetTxInterruptMask(base, CY_SCB_TX_INTR_UNDERFLOW);

            if (context->useTxFifo)
            {
                /* Data is copied into TX FIFO */
                context->slaveStatus |= CY_SCB_I2C_SLAVE_RD_IN_FIFO;

                /* Involve a callback if registered: data copied into TX FIFO */
                if (NULL != context->cbEvents)
                {
                    context->cbEvents(CY_SCB_I2C_SLAVE_RD_IN_FIFO_EVENT);
                }
            }
        }
    }
}


/*******************************************************************************
* Function Name: SlaveHandleStop
****************************************************************************//**
*
* Handles transfer completion. It is triggered by a stop or restart
* condition on the bus.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void SlaveHandleStop(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    uint32_t locEvents;
    un_SCB_I2C_CTRL_t temp_I2C_CTRL;

    if (CY_SCB_I2C_SLAVE_RX == context->state)
    {
        /* If any data is left in RX FIFO, this is an overflow */
        if (Cy_SCB_GetNumInRxFifo(base) > 0ul)
        {
            context->slaveStatus |= CY_SCB_I2C_SLAVE_WR_OVRFL;

            if (context->useRxFifo)
            {
                Cy_SCB_ClearRxFifo(base);
            }
            else
            {
                (void) Cy_SCB_ReadRxFifo(base);
            }
        }

        locEvents             = (uint32_t)  CY_SCB_I2C_SLAVE_WR_CMPLT_EVENT;
        context->slaveStatus |= (uint32_t)  CY_SCB_I2C_SLAVE_WR_CMPLT;
        context->slaveStatus &= (uint32_t) ~CY_SCB_I2C_SLAVE_WR_BUSY;

        /* Clean up the RX direction */
        temp_I2C_CTRL.u32Register                      = base->unI2C_CTRL.u32Register;
        temp_I2C_CTRL.stcField.u1S_READY_DATA_ACK      = 0ul;
        temp_I2C_CTRL.stcField.u1S_NOT_READY_DATA_NACK = 0ul;
        base->unI2C_CTRL.u32Register                   = temp_I2C_CTRL.u32Register;

        Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);
        Cy_SCB_ClearRxInterrupt(base, CY_SCB_RX_INTR_LEVEL);
    }
    else
    {
        /* The number of bytes left in TX FIFO */
        uint32_t size = Cy_SCB_GetNumInTxFifo(base) + Cy_SCB_GetTxSrValid(base);

        /* Get the number of bytes transferred from the read buffer */
        context->slaveTxBufferCnt = (context->slaveTxBufferIdx - size);

        /* Update buffer pointer and its size if there is no overflow */
        if (0ul == (CY_SCB_I2C_SLAVE_RD_UNDRFL & context->slaveStatus))
        {
            context->slaveTxBufferSize += size;
            context->slaveTxBuffer     -= size;
        }

        locEvents             = (uint32_t)  CY_SCB_I2C_SLAVE_RD_CMPLT_EVENT;
        context->slaveStatus |= (uint32_t)  CY_SCB_I2C_SLAVE_RD_CMPLT;
        context->slaveStatus &= (uint32_t) ~CY_SCB_I2C_SLAVE_RD_BUSY;

        /* Clean up the TX direction */
        Cy_SCB_SetTxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);
    }

    /* Return scb into the known state after an error */
    if (0ul != (CY_SCB_I2C_SLAVE_INTR_ERROR & Cy_SCB_GetSlaveInterruptStatusMasked(base)))
    {
        /* After scb IP is reset, the interrupt statuses are cleared */
        Cy_SCB_FwBlockReset(base);

        locEvents |= CY_SCB_I2C_SLAVE_ERR_EVENT;
    }

    /* After a stop or error, set the state to idle */
    context->state = CY_SCB_I2C_IDLE;

    /* Call a completion callback if registered */
    if (NULL != context->cbEvents)
    {
        context->cbEvents(locEvents);
    }
}


/*******************************************************************************
* Function Name: MasterInterrupt
****************************************************************************//**
*
* This is the interrupt function for the SCB configured in the I2C master.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void MasterInterrupt(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    uint32_t masterIntrStatus = Cy_SCB_GetMasterInterruptStatusMasked(base);
    uint32_t intrCause = Cy_SCB_GetInterruptCause(base);

    /* Check if the slave is active. It can be addressed during the master set-up transfer */
    if (0ul != (CY_SCB_SLAVE_INTR & intrCause))
    {
        /* Abort the transfer due to slave operation */
        if (0ul != base->unI2C_M_CMD.u32Register)
        {
            base->unI2C_M_CMD.u32Register = 0ul;

            context->masterStatus |= CY_SCB_I2C_MASTER_ABORT_START;
        }

        context->state = CY_SCB_I2C_MASTER_CMPLT;
    }

    /* Check for master error conditions */
    if (0ul != (CY_SCB_MASTER_INTR & intrCause))
    {
        /* Master interrupt source is other than ACK */
        if (0ul == (CY_SCB_MASTER_INTR_I2C_ACK & masterIntrStatus))
        {
            /* Skip TX processing */
            intrCause &= (uint32_t) ~CY_SCB_TX_INTR;

            /* The master has not received the acknowledgment for slave */
            if (0ul != (CY_SCB_MASTER_INTR_I2C_NACK & masterIntrStatus))
            {
                /* Clear NAK interrupt source */
                Cy_SCB_ClearMasterInterrupt(base, CY_SCB_MASTER_INTR_I2C_NACK);

                /* Update status to indicate address or data NAK */
                context->masterStatus |= (0ul == (CY_SCB_MASTER_INTR_I2C_ACK & Cy_SCB_GetMasterInterruptStatus(base))) ?
                                                    CY_SCB_I2C_MASTER_ADDR_NAK : CY_SCB_I2C_MASTER_DATA_NAK;

                /* Check if end transaction was not requested before */
                if (!((CY_SCB_I2C_MASTER_WAIT_STOP == context->state) || (CY_SCB_I2C_MASTER_STOP == context->state)))
                {
                    context->state = (context->masterPause) ? CY_SCB_I2C_MASTER_CMPLT : CY_SCB_I2C_MASTER_STOP;
                }
                else
                {
                    /* Wait for Stop generation because it was requested after all
                    * bytes were copied into the TX FIFO.
                    */
                }
            }

            /* Complete the transfer: stop, bus error or arbitration lost */
            if (0ul != (CY_SCB_I2C_MASTER_INTR_CMPLT & masterIntrStatus))
            {
                /* The master detected a bus error condition */
                if (0ul != (CY_SCB_MASTER_INTR_I2C_BUS_ERROR & masterIntrStatus))
                {
                    context->masterStatus |= CY_SCB_I2C_MASTER_BUS_ERR;
                }

                /* The master detected an arbitration lost condition */
                if (0ul != (CY_SCB_MASTER_INTR_I2C_ARB_LOST & masterIntrStatus))
                {
                    context->masterStatus |= CY_SCB_I2C_MASTER_ARB_LOST;
                }

                context->state = CY_SCB_I2C_MASTER_CMPLT;
            }
        }
    }

    if (0ul != (CY_SCB_RX_INTR & intrCause))
    {
        MasterHandleDataReceive(base, context);

        Cy_SCB_ClearRxInterrupt(base, CY_SCB_RX_INTR_LEVEL);
    }

    if (0ul != (CY_SCB_TX_INTR & intrCause))
    {
        MasterHandleDataTransmit(base, context);

        Cy_SCB_ClearTxInterrupt(base, CY_SCB_TX_INTR_LEVEL);
    }

    /* Complete the transfer */
    if (CY_SCB_I2C_MASTER_CMPLT == context->state)
    {
        MasterHandleComplete(base, context);
    }

    /* Generate stop to complete transfer */
    if (CY_SCB_I2C_MASTER_STOP == context->state)
    {
        MasterHandleStop(base, context);
    }
}


/*******************************************************************************
* Function Name: MasterHandleDataReceive
****************************************************************************//**
*
* Reads data from RX FIFO into the buffer provided by \ref Cy_SCB_I2C_MasterRead.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void MasterHandleDataReceive(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    un_SCB_I2C_M_CMD_t temp_I2C_M_CMD;
    un_SCB_I2C_CTRL_t  temp_I2C_CTRL;

    switch (context->state)
    {
        case CY_SCB_I2C_MASTER_RX0:
        {
            /* Put data into the component buffer */
            context->masterBuffer[0ul] = (uint8_t) Cy_SCB_ReadRxFifo(base);

            ++context->masterBufferIdx;
            --context->masterBufferSize;

            if (context->masterBufferSize > 0ul)
            {
                /* Continue the transaction: move pointer send an ACK */
                context->masterBuffer = &context->masterBuffer[1ul];
                temp_I2C_M_CMD.u32Register      = 0ul;
                temp_I2C_M_CMD.stcField.u1M_ACK = 1ul;
                base->unI2C_M_CMD.u32Register   = temp_I2C_M_CMD.u32Register;
            }
            else
            {
                /* Complete the transaction */
                context->state = (context->masterPause) ? CY_SCB_I2C_MASTER_CMPLT : CY_SCB_I2C_MASTER_STOP;
            }
        }
        break;

        case CY_SCB_I2C_MASTER_RX1:
        {
            uint32_t numToCopied;

            /* Get data from RX FIFO */
            numToCopied = Cy_SCB_ReadArray(base, context->masterBuffer, context->masterBufferSize);
            context->masterBufferIdx  += numToCopied;
            context->masterBufferSize -= numToCopied;
            context->masterBuffer      = &context->masterBuffer[numToCopied];

            if (context->masterBufferSize < 2ul)
            {
                /* Stop ACKing data */
                temp_I2C_CTRL.u32Register                 = base->unI2C_CTRL.u32Register;
                temp_I2C_CTRL.stcField.u1M_READY_DATA_ACK = 0ul;
                base->unI2C_CTRL.u32Register              = temp_I2C_CTRL.u32Register;

                if (1ul == context->masterBufferSize)
                {
                    /* Catch the last byte */
                    Cy_SCB_SetRxFifoLevel(base, 0ul);

                    context->state = CY_SCB_I2C_MASTER_RX0;
                }
                else
                {
                    /* Stop RX processing */
                    Cy_SCB_SetRxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

                    context->state = CY_SCB_I2C_MASTER_STOP;
                }
            }
            else
            {
                /* Continue the transfer: Adjust the level in RX FIFO */
                Cy_SCB_SetRxFifoLevel(base, (context->masterBufferSize <= CY_SCB_I2C_HALF_FIFO_SIZE) ?
                                            (context->masterBufferSize - 2ul) : (CY_SCB_I2C_HALF_FIFO_SIZE - 1ul));
            }
        }
        break;

        default:
            /* Do nothing: drop data into RX FIFO */
        break;
    }
}


/*******************************************************************************
* Function Name: MasterHandleDataTransmit
****************************************************************************//**
*
* Loads TX FIFO with data provided by \ref Cy_SCB_I2C_MasterWrite.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void MasterHandleDataTransmit(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    if (CY_SCB_I2C_MASTER_TX_DONE == context->state)
    {
        context->state = CY_SCB_I2C_MASTER_CMPLT;
    }
    else if (CY_SCB_I2C_MASTER_TX == context->state)
    {
        if (context->masterBufferSize > 1ul)
        {
            /* Get the number of bytes to copy into TX FIFO */
            uint32_t NumToCopy = (context->useTxFifo) ? (context->masterBufferSize - 1ul) : (1ul);

            /* Write data into TX FIFO */
            NumToCopy = Cy_SCB_WriteArray(base, context->masterBuffer, NumToCopy);
            context->masterBufferIdx  += NumToCopy;
            context->masterBufferSize -= NumToCopy;
            context->masterBuffer      = &context->masterBuffer[NumToCopy];
        }

        /* Put the last byte */
        if ((CY_SCB_I2C_FIFO_SIZE != Cy_SCB_GetNumInTxFifo(base)) && (1ul == context->masterBufferSize))
        {
            uint32_t intrStatus;

            /* Put the last data byte into TX FIFO and make sure that TX
            * underflow will happen after all data is transferred onto the bus.
            */
            intrStatus = Cy_SysLib_EnterCriticalSection();

            Cy_SCB_WriteTxFifo(base, (uint32_t) context->masterBuffer[0ul]);
            Cy_SCB_ClearTxInterrupt(base, CY_SCB_TX_INTR_UNDERFLOW);

            Cy_SysLib_ExitCriticalSection(intrStatus);

            ++context->masterBufferIdx;
            context->masterBufferSize = 0ul;
        }

        /* Complete the transfer */
        if (0ul == context->masterBufferSize)
        {
            if (context->masterPause)
            {
                /* Wait until data is transfered onto the bus */
                Cy_SCB_SetTxInterruptMask(base, CY_SCB_TX_INTR_UNDERFLOW);

                context->state = CY_SCB_I2C_MASTER_TX_DONE;
            }
            else
            {
                /* Disable TX processing */
                Cy_SCB_SetTxInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

                /* Request Stop generation */
                context->state = CY_SCB_I2C_MASTER_STOP;
            }

            if (context->useTxFifo)
            {
                /* Notify the user that data is in TX FIFO */
                context->masterStatus |= CY_SCB_I2C_MASTER_WR_IN_FIFO;

                if (NULL != context->cbEvents)
                {
                    context->cbEvents(CY_SCB_I2C_MASTER_WR_IN_FIFO_EVENT);
                }
            }
        }
    }
    else
    {
        /* Do nothing */
    }
}


/*******************************************************************************
* Function Name: MasterHandleComplete
****************************************************************************//**
*
* Handles the transfer completion on a stop or restart - the normal case or
* completion due to an error on the bus or lost arbitration.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void MasterHandleComplete(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    uint32_t masterIntrStatus = Cy_SCB_GetMasterInterruptStatusMasked(base);
    un_SCB_I2C_CTRL_t temp_I2C_CTRL;

    /* Clean-up hardware: disable interrupt sources */
    temp_I2C_CTRL.u32Register                 = base->unI2C_CTRL.u32Register;
    temp_I2C_CTRL.stcField.u1M_READY_DATA_ACK = 0ul;
    base->unI2C_CTRL.u32Register              = temp_I2C_CTRL.u32Register;

    /* Disable the interrupt source for master operation */
    Cy_SCB_SetRxInterruptMask    (base, CY_SCB_CLEAR_ALL_INTR_SRC);
    Cy_SCB_SetTxInterruptMask    (base, CY_SCB_CLEAR_ALL_INTR_SRC);
    Cy_SCB_SetMasterInterruptMask(base, CY_SCB_CLEAR_ALL_INTR_SRC);

    Cy_SCB_ClearMasterInterrupt(base, CY_SCB_I2C_MASTER_INTR_ALL);

    /* Operation complete - master is not busy anymore */
    context->masterStatus &= (uint32_t) ~CY_SCB_I2C_MASTER_BUSY;

    /* Get number of byte transferred on the bus */
    if (context->masterRdDir)
    {
        context->masterNumBytes = context->masterBufferIdx;
    }
    else
    {
        context->masterNumBytes = context->masterBufferIdx -
                    (Cy_SCB_GetNumInTxFifo(base) + Cy_SCB_GetTxSrValid(base));
    }

    /* Clean up after a not completed transfer */
    if (0ul != (CY_SCB_I2C_MASTER_INTR_ERR & masterIntrStatus))
    {
        /* Reset the scb IP block when:
        *  1. Master mode: Reset IP when arbitration is lost or a bus error occurs.
        *  2. Master-Slave mode: Reset IP if it is not the address phase (ACK is 0).
        *  Otherwise, reset only on a bus error. If "lost arbitration" happened, the slave
        *  can be addressed, so let the slave  accept the address.
        */

        bool resetIp = true;

        /* Check the Master-Slave address an ACK/NACK */
        if ((base->unI2C_CTRL.stcField.u1SLAVE_MODE == 1ul) && (base->unI2C_CTRL.stcField.u1MASTER_MODE == 1ul))
        {
            resetIp = ((0ul != (CY_SCB_MASTER_INTR_I2C_ACK & masterIntrStatus)) ? true :
                            ((0ul != (CY_SCB_MASTER_INTR_I2C_BUS_ERROR & masterIntrStatus)) ? true : false));
        }

        if (resetIp)
        {
            /* Reset to get it back in an known state */
            Cy_SCB_FwBlockReset(base);
        }

        /* Back to the idle state. The master is not active anymore */
        context->state = CY_SCB_I2C_IDLE;
    }
    else
    {
        if (context->useRxFifo)
        {
            /* Clear RX FIFO from remaining data and level interrupt source */
            Cy_SCB_ClearRxFifo(base);
            Cy_SCB_ClearRxInterrupt(base, CY_SCB_RX_INTR_LEVEL);
        }

        context->state = (context->masterPause) ? CY_SCB_I2C_MASTER_WAIT : CY_SCB_I2C_IDLE;
    }

    /* An operation completion callback */
    if (NULL != context->cbEvents)
    {
        /* Get completion events based on the hardware status */
        uint32_t locEvents = context->masterRdDir ? CY_SCB_I2C_MASTER_RD_CMPLT_EVENT : CY_SCB_I2C_MASTER_WR_CMPLT_EVENT;

        /* Add errors if any */
        locEvents |= (0ul != (CY_SCB_I2C_MASTER_ERR & context->masterStatus)) ? CY_SCB_I2C_MASTER_ERR_EVENT : 0ul;

        context->cbEvents(locEvents);
    }
}


/*******************************************************************************
* Function Name: MasterHandleComplete
****************************************************************************//**
*
* Handles the stop condition generation
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_scb_i2c_context_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void MasterHandleStop(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context)
{
    un_SCB_I2C_M_CMD_t temp_I2C_M_CMD;

    /* Stop RX and TX processing */
    Cy_SCB_SetRxInterruptMask (base, CY_SCB_CLEAR_ALL_INTR_SRC);
    Cy_SCB_SetTxInterruptMask (base, CY_SCB_CLEAR_ALL_INTR_SRC);

    if (0ul != base->unI2C_M_CMD.u32Register)
    {
        /* Enable ACK interrupt source: it triggers after Start or ReStart generation */
        Cy_SCB_SetMasterInterruptMask(base, CY_SCB_I2C_MASTER_INTR_ALL);
    }
    else
    {
        /* Disable ACK interrupt source */
        Cy_SCB_SetMasterInterruptMask(base, CY_SCB_I2C_MASTER_INTR);

        /* Complete transaction generating Stop */
        temp_I2C_M_CMD.u32Register = 0ul;
        temp_I2C_M_CMD.stcField.u1M_STOP = 1ul;
        temp_I2C_M_CMD.stcField.u1M_NACK = 1ul;
        base->unI2C_M_CMD.u32Register = temp_I2C_M_CMD.u32Register;
        context->state = CY_SCB_I2C_MASTER_WAIT_STOP;
    }
}


/******************************************************************************
* Function Name: WaitOneUnit
****************************************************************************//**
*
* Waits for one unit before unblock code execution.
* Note If a timeout value is 0, this function does nothing and returns 0.
*
* \param timeout
* The pointer to a timeout value.
*
* \return
* Returns 0 if a timeout does not expire or the timeout mask.
*
*******************************************************************************/
static uint32_t WaitOneUnit(uint32_t *timeout)
{
    uint32_t status = 0ul;

    /* If the timeout equal to 0. Ignore the timeout */
    if (*timeout > 0ul)
    {
        Cy_SysLib_DelayUs(CY_SCB_WAIT_1_UNIT);
        --(*timeout);

        if (0ul == *timeout)
        {
            status = CY_SCB_I2C_MASTER_TIMEOUT_DONE;
        }
    }

    return (status);
}


/******************************************************************************
* Function Name: HandleStatus
****************************************************************************//**
*
* Converts passed status into the cy_en_scb_i2c_status_t.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param status
* The status to covert.
*
* \return
* \ref cy_en_scb_i2c_status_t
*
*******************************************************************************/
static cy_en_scb_i2c_status_t HandleStatus(volatile stc_SCB_t *base, uint32_t status, cy_stc_scb_i2c_context_t *context)
{
    bool resetBlock = false;
    cy_en_scb_i2c_status_t retStatus = CY_SCB_I2C_MASTER_MANUAL_TIMEOUT;

    /* Convert the master status to the API status */
    if (0ul != (CY_SCB_I2C_MASTER_TIMEOUT_DONE & status))
    {
        resetBlock = true;
    }
    else if (0ul != (CY_SCB_I2C_SLAVE_ADDR_DONE & status))
    {
        if (0ul != base->unI2C_M_CMD.u32Register)
        {
            /* Abort the master operation, the slave was addressed first */
            retStatus = CY_SCB_I2C_MASTER_MANUAL_ABORT_START;

            base->unI2C_M_CMD.u32Register = 0ul;
            context->state = CY_SCB_I2C_IDLE;
        }
    }
    else if (0ul != (CY_SCB_MASTER_INTR_I2C_BUS_ERROR & status))
    {
        retStatus = CY_SCB_I2C_MASTER_MANUAL_BUS_ERR;
        resetBlock = true;
    }
    else if (0ul != (CY_SCB_MASTER_INTR_I2C_ARB_LOST & status))
    {
        retStatus = CY_SCB_I2C_MASTER_MANUAL_ARB_LOST;

        if (CY_SCB_I2C_MASTER_ADDR == context->state)
        {
            /* This is the address phase:
            *  1. Master mode: Reset IP when "arbitration lost" occurred.
            *  2. Master-Slave mode: If "lost arbitration" occurred, the slave
            *  can be addressed to let the slave accept the address; do not
            *  reset IP.
            */
            resetBlock = !(base->unI2C_CTRL.stcField.u1SLAVE_MODE);

            context->state = CY_SCB_I2C_IDLE;
        }
        else
        {
            resetBlock = true;
        }
    }
    else if (0ul != (CY_SCB_MASTER_INTR_I2C_NACK & status))
    {
        /* An address or data was NAKed */
        retStatus = (CY_SCB_I2C_MASTER_ADDR == context->state) ?
                     CY_SCB_I2C_MASTER_MANUAL_ADDR_NAK : CY_SCB_I2C_MASTER_MANUAL_NAK;
    }
    else if (0ul != (CY_SCB_MASTER_INTR_I2C_STOP & status))
    {
        /* No errors - end of transaction detected */
        retStatus = CY_SCB_I2C_SUCCESS;

        context->state = CY_SCB_I2C_IDLE;
    }
    else /* No errors - continue transaction */
    {
        retStatus = CY_SCB_I2C_SUCCESS;

        /* The change state */
        if (CY_SCB_I2C_MASTER_ADDR == context->state)
        {
            context->state = (context->masterRdDir) ?
                                CY_SCB_I2C_MASTER_RX0 : CY_SCB_I2C_MASTER_TX;
        }
    }

    if (resetBlock)
    {
        /* A block reset clears all interrupt sources */
        Cy_SCB_FwBlockReset(base);

        context->state = CY_SCB_I2C_IDLE;
    }
    else
    {
        Cy_SCB_ClearMasterInterrupt(base, CY_SCB_I2C_MASTER_INTR_ALL);
    }

    return (retStatus);
}


#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */

