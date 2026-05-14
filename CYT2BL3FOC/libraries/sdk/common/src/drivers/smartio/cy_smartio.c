/***************************************************************************//**
* \file cy_smartio.c
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* \brief
* Provides an API implementation of the Smart I/O driver
*
********************************************************************************
* \copyright
* Copyright 2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_smartio.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
* Function Name: Cy_SmartIO_Init
****************************************************************************//**
*
* \brief Initializes the Smart I/O.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param config
* Pointer to the Smart I/O configuration structure
*
* \return
* Status of the initialization operation
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_Init(volatile stc_SMARTIO_PRT_t* base, const cy_stc_smartio_config_t* config)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;
    
    if(NULL != config)
    {
        un_SMARTIO_PRT_CTL_t workCTL = {.u32Register = 0ul};
        workCTL.stcField.u1ENABLED     = CY_SMARTIO_DISABLE;
        workCTL.stcField.u1HLD_OVR     = config->hldOvr;
        workCTL.stcField.u1PIPELINE_EN = CY_SMARTIO_ENABLE;
        workCTL.stcField.u5CLOCK_SRC   = config->clkSrc;
        workCTL.stcField.u8BYPASS      = config->bypassMask;
        base->unCTL.u32Register        = workCTL.u32Register;

        un_SMARTIO_PRT_SYNC_CTL_t workSYNC_CTL = {.u32Register = 0ul};
        workSYNC_CTL.stcField.u8IO_SYNC_EN   = config->ioSyncEn;
        workSYNC_CTL.stcField.u8CHIP_SYNC_EN = config->chipSyncEn;
        base->unSYNC_CTL.u32Register         = workSYNC_CTL.u32Register;

        /* LUT configurations - skip if lutCfg is a NULL pointer */
        for(uint32_t i = CY_SMARTIO_LUTMIN; i < CY_SMARTIO_LUTMAX; i++)
        {
            if(NULL != config->lutCfg[i])
            {
                un_SMARTIO_PRT_LUT_SEL_t workLUT_SET = { .u32Register = 0ul };
                workLUT_SET.stcField.u4LUT_TR0_SEL = config->lutCfg[i]->tr0;
                workLUT_SET.stcField.u4LUT_TR1_SEL = config->lutCfg[i]->tr1;
                workLUT_SET.stcField.u4LUT_TR2_SEL = config->lutCfg[i]->tr2;
                base->unLUT_SEL[i].u32Register     = workLUT_SET.u32Register;

                un_SMARTIO_PRT_LUT_CTL_t workLUT_CTL = { .u32Register = 0ul };
                workLUT_CTL.stcField.u2LUT_OPC = config->lutCfg[i]->opcode;
                workLUT_CTL.stcField.u8LUT     = config->lutCfg[i]->lutMap;
                base->unLUT_CTL[i].u32Register = workLUT_CTL.u32Register;
            }
        }

        /* DU Configuration - skip if duCfg is a NULL pointer */
        if(NULL != config->duCfg)
        {
            un_SMARTIO_PRT_DU_SEL_t workDU_SEL = {.u32Register = 0ul};
            workDU_SEL.stcField.u4DU_TR0_SEL   = config->duCfg->tr0;
            workDU_SEL.stcField.u4DU_TR1_SEL   = config->duCfg->tr1;
            workDU_SEL.stcField.u4DU_TR2_SEL   = config->duCfg->tr2;
            workDU_SEL.stcField.u2DU_DATA0_SEL = config->duCfg->data0;
            workDU_SEL.stcField.u2DU_DATA1_SEL = config->duCfg->data1;
            base->unDU_SEL.u32Register         = workDU_SEL.u32Register;

            un_SMARTIO_PRT_DU_CTL_t workDU_CTL = {.u32Register = 0ul};
            workDU_CTL.stcField.u3DU_SIZE = config->duCfg->size;
            workDU_CTL.stcField.u4DU_OPC  = config->duCfg->opcode;
            base->unDU_CTL.u32Register    = workDU_CTL.u32Register;

            base->unDATA.stcField.u8DATA = config->duCfg->dataReg;
        }
    }
    else
    {
        status = CY_SMARTIO_BAD_PARAM;
    }
    
    return(status);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_Deinit
****************************************************************************//**
*
* \brief Resets the Smart I/O to default values.
*
* \param base
* Pointer to the Smart I/O base address
*
*******************************************************************************/
void Cy_SmartIO_Deinit(volatile stc_SMARTIO_PRT_t* base)
{
    un_SMARTIO_PRT_CTL_t workCTL= {.u32Register = 0ul};
    workCTL.stcField.u1ENABLED     = CY_SMARTIO_DISABLE;
    workCTL.stcField.u1PIPELINE_EN = CY_SMARTIO_ENABLE;
    workCTL.stcField.u5CLOCK_SRC   = CY_SMARTIO_CLK_GATED;
    workCTL.stcField.u8BYPASS      = CY_SMARTIO_CHANNEL_ALL;
    base->unCTL.u32Register        = workCTL.u32Register;

    base->unSYNC_CTL.u32Register = CY_SMARTIO_DEINIT;
    for(uint8_t idx = CY_SMARTIO_LUTMIN; idx < CY_SMARTIO_LUTMAX; idx++)
    {
        base->unLUT_SEL[idx].u32Register = CY_SMARTIO_DEINIT;
        base->unLUT_CTL[idx].u32Register = CY_SMARTIO_DEINIT;
    }
    base->unDU_SEL.u32Register = CY_SMARTIO_DEINIT;
    base->unDU_CTL.u32Register = CY_SMARTIO_DEINIT;
    base->unDATA.u32Register = CY_SMARTIO_DEINIT;
}


/*******************************************************************************
* Function Name: Cy_SmartIO_Enable
****************************************************************************//**
*
* \brief Enables the Smart I/O.
*
* \param base
* Pointer to the Smart I/O base address
*
*******************************************************************************/
void Cy_SmartIO_Enable(volatile stc_SMARTIO_PRT_t* base)
{
    un_SMARTIO_PRT_CTL_t workCTL;
    workCTL.u32Register            = base->unCTL.u32Register;
    workCTL.stcField.u1ENABLED     = CY_SMARTIO_ENABLE;
    workCTL.stcField.u1PIPELINE_EN = CY_SMARTIO_DISABLE;
    base->unCTL.u32Register        = workCTL.u32Register;
}


/*******************************************************************************
* Function Name: Cy_SmartIO_Disable
****************************************************************************//**
*
* \brief Disables the Smart I/O.
*
* \param base
* Pointer to the Smart I/O base address
*
*******************************************************************************/
void Cy_SmartIO_Disable(volatile stc_SMARTIO_PRT_t* base)
{
    un_SMARTIO_PRT_CTL_t workCTL;
    workCTL.u32Register            = base->unCTL.u32Register;
    workCTL.stcField.u1ENABLED     = CY_SMARTIO_DISABLE;
    workCTL.stcField.u1PIPELINE_EN = CY_SMARTIO_ENABLE;
    base->unCTL.u32Register        = workCTL.u32Register;
}


/*******************************************************************************
* Function Name: Cy_SmartIO_GetChBypass
****************************************************************************//**
*
* \brief Gets the bypass/route state of all 8 channels in the Smart I/O
*
* <table class="doxtable">
*   <tr><th>Bypass bit</th><th>    Channel  </th></tr>
*   <tr><td>     0    </td><td> io0<->chip0 </td></tr>
*   <tr><td>     1    </td><td> io1<->chip1 </td></tr>
*   <tr><td>     2    </td><td> io2<->chip2 </td></tr>
*   <tr><td>     3    </td><td> io3<->chip3 </td></tr>
*   <tr><td>     4    </td><td> io4<->chip4 </td></tr>
*   <tr><td>     5    </td><td> io5<->chip5 </td></tr>
*   <tr><td>     6    </td><td> io6<->chip6 </td></tr>
*   <tr><td>     7    </td><td> io7<->chip7 </td></tr>
* </table>
*
* \param base
* Pointer to the Smart I/O base address
*
* \return
* Bypass/Route state of 8 io<->chip channels (bits [7:0]): 1=bypass, 0=routed.
*
*******************************************************************************/
uint8_t Cy_SmartIO_GetChBypass(volatile stc_SMARTIO_PRT_t* base)
{
    return ((uint8_t)base->unCTL.stcField.u8BYPASS);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_SetChBypass
****************************************************************************//**
*
* \brief Sets the bypass/route state of all 8 channels in the Smart I/O
*
* <table class="doxtable">
*   <tr><th>Bypass bit</th><th>    Channel  </th></tr>
*   <tr><td>     0    </td><td> io0<->chip0 </td></tr>
*   <tr><td>     1    </td><td> io1<->chip1 </td></tr>
*   <tr><td>     2    </td><td> io2<->chip2 </td></tr>
*   <tr><td>     3    </td><td> io3<->chip3 </td></tr>
*   <tr><td>     4    </td><td> io4<->chip4 </td></tr>
*   <tr><td>     5    </td><td> io5<->chip5 </td></tr>
*   <tr><td>     6    </td><td> io6<->chip6 </td></tr>
*   <tr><td>     7    </td><td> io7<->chip7 </td></tr>
* </table>
*
* \param base
* Pointer to the Smart I/O base address
*
* \param bypassMask
* Bypass/Route state of 8 io<->chip channels (bits [7:0]): 1=bypass, 0=routed.
*
* \return
* Status of the operation
*
* \note The Smart I/O block must be disabled before calling this function.
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_SetChBypass(volatile stc_SMARTIO_PRT_t* base, uint8_t bypassMask)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;

    if(base->unCTL.stcField.u1ENABLED == CY_SMARTIO_ENABLE)
    {
        status = CY_SMARTIO_LOCKED;
    }
    else
    {
        base->unCTL.stcField.u8BYPASS = bypassMask;
    }
    
    return(status);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_GetClock
****************************************************************************//**
*
* \brief Gets the clock source of the Smart I/O.
*
* \param base
* Pointer to the Smart I/O base address
*
* \return
* Clock source
*
*******************************************************************************/
cy_en_smartio_clksrc_t Cy_SmartIO_GetClock(volatile stc_SMARTIO_PRT_t* base)
{
    return ((cy_en_smartio_clksrc_t)base->unCTL.stcField.u5CLOCK_SRC);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_SetClock
****************************************************************************//**
*
* \brief Sets the clock source of the Smart I/O.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param clkSrc
* Pointer to the Smart I/O base address
*
* \return
* Status of the operation
*
* \note The Smart I/O block must be disabled before calling this function.
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_SetClock(volatile stc_SMARTIO_PRT_t* base, cy_en_smartio_clksrc_t clkSrc)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;

    if(base->unCTL.stcField.u1ENABLED == CY_SMARTIO_ENABLE)
    {
        status = CY_SMARTIO_LOCKED;
    }
    else
    {
        base->unCTL.stcField.u5CLOCK_SRC = clkSrc;
    }
    
    return(status);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_GetIoSync
****************************************************************************//**
*
* \brief Gets the synchronization mode of the 8 I/O terminals.
*
* <table class="doxtable">
*   <tr><th> Sync bit </th><th> I/O terminal </th></tr>
*   <tr><td>     0    </td><td>     io0      </td></tr>
*   <tr><td>     1    </td><td>     io1      </td></tr>
*   <tr><td>     2    </td><td>     io2      </td></tr>
*   <tr><td>     3    </td><td>     io3      </td></tr>
*   <tr><td>     4    </td><td>     io4      </td></tr>
*   <tr><td>     5    </td><td>     io5      </td></tr>
*   <tr><td>     6    </td><td>     io6      </td></tr>
*   <tr><td>     7    </td><td>     io7      </td></tr>
* </table>
*
* \param base
* Pointer to the Smart I/O base address
*
* \return
* Sync mode of 8 I/O terminals (bits [7:0]): 1=sync, 0=no sync.
*
*******************************************************************************/
uint8_t Cy_SmartIO_GetIoSync(volatile stc_SMARTIO_PRT_t* base)
{
    return((uint8_t)base->unSYNC_CTL.stcField.u8IO_SYNC_EN);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_SetIoSync
****************************************************************************//**
*
* \brief Sets the synchronization mode of the 8 I/O terminals.
*
* <table class="doxtable">
*   <tr><th> Sync bit </th><th> I/O terminal </th></tr>
*   <tr><td>     0    </td><td>     io0      </td></tr>
*   <tr><td>     1    </td><td>     io1      </td></tr>
*   <tr><td>     2    </td><td>     io2      </td></tr>
*   <tr><td>     3    </td><td>     io3      </td></tr>
*   <tr><td>     4    </td><td>     io4      </td></tr>
*   <tr><td>     5    </td><td>     io5      </td></tr>
*   <tr><td>     6    </td><td>     io6      </td></tr>
*   <tr><td>     7    </td><td>     io7      </td></tr>
* </table>
*
* \param base
* Pointer to the Smart I/O base address
*
* \param ioSyncEn
* Sync mode of 8 I/O terminals (bits [7:0]): 1=sync, 0=no sync.
*
* \return
* Status of the operation
*
* \note The Smart I/O block must be disabled before calling this function.
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_SetIoSync(volatile stc_SMARTIO_PRT_t* base, uint8_t ioSyncEn)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;

    if(base->unCTL.stcField.u1ENABLED == CY_SMARTIO_ENABLE)
    {
        status = CY_SMARTIO_LOCKED;
    }
    else
    {
        base->unSYNC_CTL.stcField.u8IO_SYNC_EN = ioSyncEn;
    }

    return(status);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_GetChipSync
****************************************************************************//**
*
* \brief Gets the synchronization mode of the 8 chip-side terminals.
*
* <table class="doxtable">
*   <tr><th> Sync bit </th><th> chip terminal </th></tr>
*   <tr><td>     0    </td><td>     chip0     </td></tr>
*   <tr><td>     1    </td><td>     chip1     </td></tr>
*   <tr><td>     2    </td><td>     chip2     </td></tr>
*   <tr><td>     3    </td><td>     chip3     </td></tr>
*   <tr><td>     4    </td><td>     chip4     </td></tr>
*   <tr><td>     5    </td><td>     chip5     </td></tr>
*   <tr><td>     6    </td><td>     chip6     </td></tr>
*   <tr><td>     7    </td><td>     chip7     </td></tr>
* </table>
*
* \param base
* Pointer to the Smart I/O base address
*
* \return
* Sync mode of 8 chip-side terminals (bits [7:0]): 1=sync, 0=no sync.
*
*******************************************************************************/
uint8_t Cy_SmartIO_GetChipSync(volatile stc_SMARTIO_PRT_t* base)
{
    return ((uint8_t)base->unSYNC_CTL.stcField.u8CHIP_SYNC_EN);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_SetChipSync
****************************************************************************//**
*
* \brief Sets the synchronization mode of the 8 chip-side terminals.
*
* <table class="doxtable">
*   <tr><th> Sync bit </th><th> chip terminal </th></tr>
*   <tr><td>     0    </td><td>     chip0     </td></tr>
*   <tr><td>     1    </td><td>     chip1     </td></tr>
*   <tr><td>     2    </td><td>     chip2     </td></tr>
*   <tr><td>     3    </td><td>     chip3     </td></tr>
*   <tr><td>     4    </td><td>     chip4     </td></tr>
*   <tr><td>     5    </td><td>     chip5     </td></tr>
*   <tr><td>     6    </td><td>     chip6     </td></tr>
*   <tr><td>     7    </td><td>     chip7     </td></tr>
* </table>
*
* \param base
* Pointer to the Smart I/O base address
*
* \param chipSyncEn
* Sync mode of 8 chip-side terminals (bits [7:0]): 1=sync, 0=no sync.
*
* \return
* Status of the operation
*
* \note The Smart I/O block must be disabled before calling this function.
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_SetChipSync(volatile stc_SMARTIO_PRT_t* base, uint8_t chipSyncEn)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;

    if(base->unCTL.stcField.u1ENABLED == CY_SMARTIO_ENABLE)
    {
        status = CY_SMARTIO_LOCKED;
    }
    else
    {
        base->unSYNC_CTL.stcField.u8CHIP_SYNC_EN = chipSyncEn;
    }

    return(status);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_HoldOverride
****************************************************************************//**
*
* \brief Configures the hold override mode.
*
* In Deep-Sleep power mode, the HSIOM holds the GPIO output and output enable
* signals for all signals that operate in chip active domain. Enabling the hold
* override allows the Smart I/O to deliver Deep-Sleep output functionality
* on these GPIO terminals. If the Smart I/O should not drive any of the GPIO 
* outputs, the hold override should be disabled.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param hldOvr
* true = Enabled: Smart I/O controls the port I/Os
* false = Disabled: HSIOM controls the port I/Os
*
* \return
* Status of the operation
*
* \note The Smart I/O block must be disabled before calling this function.
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_HoldOverride(volatile stc_SMARTIO_PRT_t* base, bool hldOvr)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;

    if(base->unCTL.stcField.u1ENABLED == CY_SMARTIO_ENABLE)
    {
        status = CY_SMARTIO_LOCKED;
    }
    else
    {
        base->unCTL.stcField.u1HLD_OVR = hldOvr;
    }

    return(status);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_GetLutTr
****************************************************************************//**
*
* \brief Gets the specified LUT input trigger source.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param lutNum
* LUT index number
*
* \param trNum
* Input trigger number
*
* \return
* LUT input trigger source
*
*******************************************************************************/
cy_en_smartio_luttr_t Cy_SmartIO_GetLutTr(volatile stc_SMARTIO_PRT_t* base, cy_en_smartio_lutnum_t lutNum, cy_en_smartio_trnum_t trNum)
{
    cy_en_smartio_luttr_t trSrc;

    switch(trNum)
    {
    case(CY_SMARTIO_TR0):
    {
        trSrc = (cy_en_smartio_luttr_t)base->unLUT_SEL[lutNum].stcField.u4LUT_TR0_SEL;
        break;
    }
    case(CY_SMARTIO_TR1):
    {
        trSrc = (cy_en_smartio_luttr_t)base->unLUT_SEL[lutNum].stcField.u4LUT_TR1_SEL;
        break;
    }
    case(CY_SMARTIO_TR2):
    {
        trSrc = (cy_en_smartio_luttr_t)base->unLUT_SEL[lutNum].stcField.u4LUT_TR2_SEL;
        break;
    }
    default:
    {
        trSrc = CY_SMARTIO_LUTTR_INVALID;
        break;
    }
    }

    return(trSrc);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_SetLutTr
****************************************************************************//**
*
* \brief Sets the specified LUT input trigger source.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param lutNum
* LUT index number
*
* \param trNum
* Input trigger number
*
* \param trSrc
* Input trigger source
*
* \return
* Status of the operation
*
* \note The Smart I/O block must be disabled before calling this function.
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_SetLutTr(volatile stc_SMARTIO_PRT_t* base, cy_en_smartio_lutnum_t lutNum, cy_en_smartio_trnum_t trNum, cy_en_smartio_luttr_t trSrc)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;

    if(base->unCTL.stcField.u1ENABLED == CY_SMARTIO_ENABLE)
    {
        status = CY_SMARTIO_LOCKED;
    }
    else
    {
        switch(trNum)
        {
        case(CY_SMARTIO_TR0):
        {
            base->unLUT_SEL[lutNum].stcField.u4LUT_TR0_SEL = (uint32_t)trSrc;
            break;
        }
        case(CY_SMARTIO_TR1):
        {
            base->unLUT_SEL[lutNum].stcField.u4LUT_TR1_SEL = (uint32_t)trSrc;
            break;
        }
        case(CY_SMARTIO_TR2):
        {
            base->unLUT_SEL[lutNum].stcField.u4LUT_TR2_SEL = (uint32_t)trSrc;
            break;
        }
        default:
        {
            status = CY_SMARTIO_BAD_PARAM;
            break;
        }
        }
    }

    return(status);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_SetLutTrAll
****************************************************************************//**
*
* \brief Sets all LUT input triggers to the same source.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param lutNum
* LUT index number
*
* \param trSrc
* Input trigger source
*
* \return
* Status of the operation
*
* \note The Smart I/O block must be disabled before calling this function.
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_SetLutTrAll(volatile stc_SMARTIO_PRT_t* base, cy_en_smartio_lutnum_t lutNum, cy_en_smartio_luttr_t trSrc)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;

    if(base->unCTL.stcField.u1ENABLED == CY_SMARTIO_ENABLE)
    {
        status = CY_SMARTIO_LOCKED;
    }
    else
    {
        un_SMARTIO_PRT_LUT_SEL_t workLUT_SEL = base->unLUT_SEL[lutNum];
        workLUT_SEL.stcField.u4LUT_TR0_SEL  = (uint32_t)trSrc;
        workLUT_SEL.stcField.u4LUT_TR1_SEL  = (uint32_t)trSrc;
        workLUT_SEL.stcField.u4LUT_TR2_SEL  = (uint32_t)trSrc;
        base->unLUT_SEL[lutNum].u32Register = workLUT_SEL.u32Register;
    }
    
    return(status);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_GetLutOpcode
****************************************************************************//**
*
* \brief Gets the opcode of the specified LUT.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param lutNum
* LUT index number
*
* \return
* LUT opcode
*
*******************************************************************************/
cy_en_smartio_lutopc_t Cy_SmartIO_GetLutOpcode(volatile stc_SMARTIO_PRT_t* base, cy_en_smartio_lutnum_t lutNum)
{
    return (cy_en_smartio_lutopc_t)base->unLUT_CTL[lutNum].stcField.u2LUT_OPC;
}


/*******************************************************************************
* Function Name: Cy_SmartIO_SetLutOpcode
****************************************************************************//**
*
* \brief Sets the opcode of the specified LUT.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param lutNum
* LUT index number
*
* \param opcode
* LUT opcode
*
* \return
* Status of the operation
*
* \note The Smart I/O block must be disabled before calling this function.
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_SetLutOpcode(volatile stc_SMARTIO_PRT_t* base, cy_en_smartio_lutnum_t lutNum, cy_en_smartio_lutopc_t opcode)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;

    if(base->unCTL.stcField.u1ENABLED == CY_SMARTIO_ENABLE)
    {
        status = CY_SMARTIO_LOCKED;
    }
    else
    {
        base->unLUT_CTL[lutNum].stcField.u2LUT_OPC = (uint32_t)opcode;
    }
    
    return(status);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_GetLutMap
****************************************************************************//**
*
* \brief Gets the 3:1 truth table of the specified LUT.
*
* <table class="doxtable">
*   <tr><th>tr2</th><th>tr1</th><th>tr0</th><th>lutNum</th></tr>
*   <tr><td> 0 </td><td> 0 </td><td> 0 </td><td> bit 0 </td></tr>
*   <tr><td> 0 </td><td> 0 </td><td> 1 </td><td> bit 1 </td></tr>
*   <tr><td> 0 </td><td> 1 </td><td> 0 </td><td> bit 2 </td></tr>
*   <tr><td> 0 </td><td> 1 </td><td> 1 </td><td> bit 3 </td></tr>
*   <tr><td> 1 </td><td> 0 </td><td> 0 </td><td> bit 4 </td></tr>
*   <tr><td> 1 </td><td> 0 </td><td> 1 </td><td> bit 5 </td></tr>
*   <tr><td> 1 </td><td> 1 </td><td> 0 </td><td> bit 6 </td></tr>
*   <tr><td> 1 </td><td> 1 </td><td> 1 </td><td> bit 7 </td></tr>
* </table>
*
* \param base
* Pointer to the Smart I/O base address
*
* \param lutNum
* LUT index number
*
* \return
* Bitfield [7:0] mapping of the 3:1 LUT
*
*******************************************************************************/
uint8_t Cy_SmartIO_GetLutMap(volatile stc_SMARTIO_PRT_t* base, cy_en_smartio_lutnum_t lutNum)
{
    return (uint8_t)base->unLUT_CTL[lutNum].stcField.u8LUT;
}


/*******************************************************************************
* Function Name: Cy_SmartIO_SetLutMap
****************************************************************************//**
*
* \brief Sets the 3:1 truth table of the specified LUT.
*
* <table class="doxtable">
*   <tr><th>tr2</th><th>tr1</th><th>tr0</th><th>lutNum</th></tr>
*   <tr><td> 0 </td><td> 0 </td><td> 0 </td><td> bit 0 </td></tr>
*   <tr><td> 0 </td><td> 0 </td><td> 1 </td><td> bit 1 </td></tr>
*   <tr><td> 0 </td><td> 1 </td><td> 0 </td><td> bit 2 </td></tr>
*   <tr><td> 0 </td><td> 1 </td><td> 1 </td><td> bit 3 </td></tr>
*   <tr><td> 1 </td><td> 0 </td><td> 0 </td><td> bit 4 </td></tr>
*   <tr><td> 1 </td><td> 0 </td><td> 1 </td><td> bit 5 </td></tr>
*   <tr><td> 1 </td><td> 1 </td><td> 0 </td><td> bit 6 </td></tr>
*   <tr><td> 1 </td><td> 1 </td><td> 1 </td><td> bit 7 </td></tr>
* </table>
*
* \param base
* Pointer to the Smart I/O base address
*
* \param lutNum
* LUT index number
*
* \param lutMap
* Bitfield [7:0] mapping of the 3:1 LUT
*
* \return
* Status of the operation
*
* \note The Smart I/O block must be disabled before calling this function.
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_SetLutMap(volatile stc_SMARTIO_PRT_t* base, cy_en_smartio_lutnum_t lutNum, uint8_t lutMap)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;

    if(base->unCTL.stcField.u1ENABLED == CY_SMARTIO_ENABLE)
    {
        status = CY_SMARTIO_LOCKED;
    }
    else
    {
        base->unLUT_CTL[lutNum].stcField.u8LUT = lutMap;
    }

    return(status);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_GetDuTr
****************************************************************************//**
*
* \brief Gets the data unit input trigger source.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param trNum
* Input trigger number
*
* \return
* Data unit input trigger source
*
*******************************************************************************/
cy_en_smartio_dutr_t Cy_SmartIO_GetDuTr(volatile stc_SMARTIO_PRT_t* base, cy_en_smartio_trnum_t trNum)
{
    cy_en_smartio_dutr_t trSrc;

    switch(trNum)
    {
    case(CY_SMARTIO_TR0):
    {
        trSrc = (cy_en_smartio_dutr_t)base->unDU_SEL.stcField.u4DU_TR0_SEL;
        break;
    }
    case(CY_SMARTIO_TR1):
    {
        trSrc = (cy_en_smartio_dutr_t)base->unDU_SEL.stcField.u4DU_TR1_SEL;
        break;
    }
    case(CY_SMARTIO_TR2):
    {
        trSrc = (cy_en_smartio_dutr_t)base->unDU_SEL.stcField.u4DU_TR2_SEL;
        break;
    }
    default:
    {
        trSrc = CY_SMARTIO_DUTR_INVALID;
        break;
    }
    }

    return(trSrc);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_SetDuTr
****************************************************************************//**
*
* \brief Sets the data unit input trigger source.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param trNum
* Input trigger number
*
* \param trSrc
* Input trigger source
*
* \return
* Status of the operation
*
* \note The Smart I/O block must be disabled before calling this function.
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_SetDuTr(volatile stc_SMARTIO_PRT_t* base, cy_en_smartio_trnum_t trNum, cy_en_smartio_dutr_t trSrc)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;

    if(base->unCTL.stcField.u1ENABLED == CY_SMARTIO_ENABLE)
    {
        status = CY_SMARTIO_LOCKED;
    }
    else
    {
        switch(trNum)
        {
        case(CY_SMARTIO_TR0):
        {
            base->unDU_SEL.stcField.u4DU_TR0_SEL = trSrc;
            break;
        }
        case(CY_SMARTIO_TR1):
        {
            base->unDU_SEL.stcField.u4DU_TR1_SEL = trSrc;
            break;
        }
        case(CY_SMARTIO_TR2):
        {
            base->unDU_SEL.stcField.u4DU_TR2_SEL = trSrc;
            break;
        }
        default:
        {
            status = CY_SMARTIO_BAD_PARAM;
            break;
        }
        }
    }

    return status;
}


/*******************************************************************************
* Function Name: Cy_SmartIO_SetDuTrAll
****************************************************************************//**
*
* \brief Sets all the data unit input trigger sources.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param trSrc
* Input trigger source
*
* \return
* Status of the operation
*
* \note The Smart I/O block must be disabled before calling this function.
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_SetDuTrAll(volatile stc_SMARTIO_PRT_t* base, cy_en_smartio_dutr_t trSrc)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;

    if(base->unCTL.stcField.u1ENABLED == CY_SMARTIO_ENABLE)
    {
        status = CY_SMARTIO_LOCKED;
    }
    else
    {
        un_SMARTIO_PRT_DU_SEL_t workDU_SEL;
        workDU_SEL.u32Register = base->unDU_SEL.u32Register;
        workDU_SEL.stcField.u4DU_TR0_SEL = (uint32_t)trSrc;
        workDU_SEL.stcField.u4DU_TR1_SEL = (uint32_t)trSrc;
        workDU_SEL.stcField.u4DU_TR2_SEL = (uint32_t)trSrc;
        base->unDU_SEL.u32Register       = workDU_SEL.u32Register;
    }

    return status;
}


/*******************************************************************************
* Function Name: Cy_SmartIO_GetDuData
****************************************************************************//**
*
* \brief Gets the data unit's input "data" source.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param dataNum
* Input data number
*
* \return
* Data unit input trigger source
*
*******************************************************************************/
cy_en_smartio_dudata_t Cy_SmartIO_GetDuData(volatile stc_SMARTIO_PRT_t* base, cy_en_smartio_datanum_t dataNum)
{
    return (dataNum == CY_SMARTIO_DATA0) ? (cy_en_smartio_dudata_t)base->unDU_SEL.stcField.u2DU_DATA0_SEL:
                                           (cy_en_smartio_dudata_t)base->unDU_SEL.stcField.u2DU_DATA1_SEL;
}


/*******************************************************************************
* Function Name: Cy_SmartIO_SetDuData
****************************************************************************//**
*
* \brief Sets the data unit's input "data" source.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param dataNum
* Input data number
*
* \param dataSrc
* Data unit input trigger source
*
* \return
* Status of the operation
*
* \note The Smart I/O block must be disabled before calling this function.
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_SetDuData(volatile stc_SMARTIO_PRT_t* base, cy_en_smartio_datanum_t dataNum, cy_en_smartio_dudata_t dataSrc)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;

    if(base->unCTL.stcField.u1ENABLED == CY_SMARTIO_ENABLE)
    {
        status = CY_SMARTIO_LOCKED;
    }
    else
    {
        if(dataNum == CY_SMARTIO_DATA0)
        {
            base->unDU_SEL.stcField.u2DU_DATA0_SEL = dataSrc;
        }
        else
        {
            base->unDU_SEL.stcField.u2DU_DATA1_SEL = dataSrc;
        }
    }
    
    return status;
}


/*******************************************************************************
* Function Name: Cy_SmartIO_GetDuOpc
****************************************************************************//**
*
* \brief Gets the data unit's opcode.
*
* \param base
* Pointer to the Smart I/O base address
*
* \return
* Data unit opcode
*
*******************************************************************************/
cy_en_smartio_duopc_t Cy_SmartIO_GetDuOpc(volatile stc_SMARTIO_PRT_t* base)
{
    return ((cy_en_smartio_duopc_t)base->unDU_CTL.stcField.u4DU_OPC);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_GetDuSize
****************************************************************************//**
*
* \brief Gets the data unit's opcode operand bit-width.
*
* \param base
* Pointer to the Smart I/O base address
*
* \return
* Data unit operand bit-width
*
*******************************************************************************/
cy_en_smartio_dusize_t Cy_SmartIO_GetDuSize(volatile stc_SMARTIO_PRT_t* base)
{
    return ((cy_en_smartio_dusize_t)base->unDU_CTL.stcField.u3DU_SIZE);
}


/*******************************************************************************
* Function Name: Cy_SmartIO_SetDuOperation
****************************************************************************//**
*
* \brief Sets the data unit's opcode and operand bit-width.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param opcode
* Data Unit opcode
*
* \param size
* Data unit operand bit-width
*
* \return
* Status of the operation
*
* \note The Smart I/O block must be disabled before calling this function.
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_SetDuOperation(volatile stc_SMARTIO_PRT_t* base, cy_en_smartio_duopc_t opcode, cy_en_smartio_dusize_t size)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;

    if(base->unCTL.stcField.u1ENABLED == CY_SMARTIO_ENABLE)
    {
        status = CY_SMARTIO_LOCKED;
    }
    else
    {
        un_SMARTIO_PRT_DU_CTL_t workDU_CTL;
        workDU_CTL.u32Register        = base->unDU_CTL.u32Register;
        workDU_CTL.stcField.u3DU_SIZE = size;
        workDU_CTL.stcField.u4DU_OPC  = opcode;
        base->unDU_CTL.u32Register    = workDU_CTL.u32Register;
    }

    return status;
}


/*******************************************************************************
* Function Name: Cy_SmartIO_GetDataReg
****************************************************************************//**
*
* \brief Gets the data unit's DATA register value.
*
* \param base
* Pointer to the Smart I/O base address
*
* \return
* DATA register value
*
*******************************************************************************/
uint8_t Cy_SmartIO_GetDataReg(volatile stc_SMARTIO_PRT_t* base)
{
    return ((uint8_t)(base->unDATA.stcField.u8DATA));
}


/*******************************************************************************
* Function Name: Cy_SmartIO_SetDataReg
****************************************************************************//**
*
* \brief Sets the data unit's DATA register value.
*
* \param base
* Pointer to the Smart I/O base address
*
* \param dataReg
* DATA register value
*
* \return
* Status of the operation
*
* \note The Smart I/O block must be disabled before calling this function.
*
*******************************************************************************/
cy_en_smartio_status_t Cy_SmartIO_SetDataReg(volatile stc_SMARTIO_PRT_t* base, uint8_t dataReg)
{
    cy_en_smartio_status_t status = CY_SMARTIO_SUCCESS;

    if(base->unCTL.stcField.u1ENABLED == CY_SMARTIO_ENABLE)
    {
        status = CY_SMARTIO_LOCKED;
    }
    else
    {
        base->unDATA.stcField.u8DATA = dataReg;
    }
    
    return status;
}



#if defined(__cplusplus)
}
#endif

/* [] END OF FILE */
