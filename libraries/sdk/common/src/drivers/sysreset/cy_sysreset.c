/***************************************************************************//**
* \file cy_sysreset.c
* \version 1.0
*
*  Description:
*   Provides an API definition of the sysreset driver
*
********************************************************************************
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_sysreset.h"
#include "ipc/cy_ipc_drv.h"

/*******************************************************************************
* Function Name: Cy_SysReset_GetResetReason
****************************************************************************//**
*
*  The function returns the cause for the latest consolidated reset(s) that occurred in
*  the system. The reset causes include HFCLK error, system faults, and device
*  reset on a wakeup from Hibernate mode.
*
*  \param None
*
*  \return 32-bit Contents of RES_CAUSE register
*  | Name			        | Value
*  |------------------------------------|--------------------	
*  | CY_SYSRESET_WDT			| 0x00000001 (bit0)
*  | CY_SYSRESET_ACT_FAULT		| 0x00000002 (bit1)
*  | CY_SYSRESET_DPSLP_FAULT		| 0x00000004 (bit2)
*  | CY_SYSRESET_TC_DBGRESET		| 0x00000008 (bit3)
*  | CY_SYSRESET_SOFT			| 0x00000010 (bit4)
*  | CY_SYSRESET_MCWDT0			| 0x00000020 (bit5)
*  | CY_SYSRESET_MCWDT1			| 0x00000040 (bit6)
*  | CY_SYSRESET_MCWDT2			| 0x00000080 (bit7)
*  | CY_SYSRESET_MCWDT3			| 0x00000100 (bit8)
*  | CY_SYSRESET_XRES			| 0x00010000 (bit16)
*  | CY_SYSRESET_BODVDDD		| 0x00020000 (bit17)
*  | CY_SYSRESET_BODVDDA		| 0x00040000 (bit18)
*  | CY_SYSRESET_BODVCCD		| 0x00080000 (bit19)
*  | CY_SYSRESET_OVDVDDD		| 0x00100000 (bit20)
*  | CY_SYSRESET_OVDVDDA		| 0x00200000 (bit21)
*  | CY_SYSRESET_OVDVCCD		| 0x00400000 (bit22)
*  | CY_SYSRESET_OCD_ACT_LINREG		| 0x00800000 (bit23)
*  | CY_SYSRESET_OCD_DPSLP_LINREG	| 0x01000000 (bit24)
*  | CY_SYSRESET_OCD_REGHC	        | 0x02000000 (bit25)
*  | CY_SYSRESET_PMIC	                | 0x04000000 (bit26)
*  | CY_SYSRESET_PXRES	                | 0x08000000 (bit27)
*  | CY_SYSRESET_STRUCT_XRES		| 0x20000000 (bit29)
*  | CY_SYSRESET_PORVDDD		| 0x40000000 (bit30)
*
*******************************************************************************/
uint32_t Cy_SysReset_GetResetReason(void)
{
    return(SRSS->unRES_CAUSE.u32Register);
}

/*******************************************************************************
* Function Name: Cy_SysReset_GetNumHfclkResetCause
****************************************************************************//**
*
* This function returns the number of HF_CLK which is a reset cause (RES_CAUSE2)
* by a loss or an error of the high frequency clock.
*
* The Clock supervisors (CSV) can make a reset as CSV_FREQ_ACTION setting
* when a CSV frequency anomaly is detected. The function returns the index
* of HF_CLK, if a reset occurred due to an anomaly HF_CLK. There are two
* different options for monitoring on HF_CLK which are a frequency loss
* and a frequency error.
*
* \return
*   - The number HF_CLK from Clock Supervisor High Frequency Loss:  Bits[15:0]
*   - The number HF_CLK from Clock Supervisor High Frequency error: Bits[31:16]
*
*******************************************************************************/
uint32_t Cy_SysReset_GetNumHfclkResetCause(void)
{
    return (SRSS->unRES_CAUSE2.u32Register);
}

/*******************************************************************************
* Function Name: Cy_SysReset_IsResetDueToHibWakeup
****************************************************************************//**
*
* This function returns reset state if it is due to hibernate wakeup.
*
* \return
*   true - Reset cause is due to hibernate wakeup
*   false - Reset cause is not due to hibernate wakeup
*
*******************************************************************************/
bool Cy_SysReset_IsResetDueToHibWakeup(void)
{
    bool retReason = false;
	
    // Contains a 8-bit token that is retained through a HIBERNATE/WAKEUP sequence 
    // that can be used by firmware to differentiate WAKEUP from a general RESET event
	// Value 0x1B into the TOKEN is added before jumping to HIBERNATE
    if(0x1Bu == SRSS->unPWR_HIBERNATE.stcField.u8TOKEN)
    {
        retReason = true;
    }
	
    return retReason;
}

/*******************************************************************************
* Function Name: Cy_SysReset_ClearAllResetReasons
****************************************************************************//**
*
*  This function clears the values of RES_CAUSE and RES_CAUSE2.
*
*******************************************************************************/
void Cy_SysReset_ClearAllResetReasons(void)
{
    /* RES_CAUSE and RES_CAUSE2 register's bits are RW1C (every bit is cleared upon writing 1),
     * so write all ones to clear all the reset reasons.
     */
    SRSS->unRES_CAUSE.u32Register  = 0xFFFFFFFFu;
    SRSS->unRES_CAUSE2.u32Register = 0xFFFFFFFFu;
    SRSS->unPWR_HIBERNATE.u32Register = 0u;
}


#if (CY_CPU_CORTEX_M0P) || defined(CY_DOXYGEN)
/*******************************************************************************
* Function Name: Cy_SysReset_SoftResetCM4
****************************************************************************//**
*
*  This function performs a CM4 Core software reset using the CM4_PWR_CTL
*  register.
*
*  \note This function should be called only when the CM4 core is in Deep
*        Sleep mode.
*  \note This function will not reset CM0+ Core.
*
*******************************************************************************/
void Cy_SysReset_SoftResetCM4(void)
{
    volatile uint32_t msg = CY_IPC_DATA_FOR_CM4_SOFT_RESET;

    /* Tries to acquire the IPC structure and pass the arguments to SROM API.
    *  SROM API parameters:
    *   ipcPtr: IPC_STRUCT0 - IPC Structure 0 reserved for M0+ Secure Access.
    *   notifyEvent_Intr: 1u - IPC Interrupt Structure 1 is used for Releasing IPC 0 (M0+ NMI Handler).
    *   msgPtr: &msg - The address of SRAM with the API's parameters.
    */
    if(CY_IPC_DRV_SUCCESS != Cy_IPC_Drv_SendMsgPtr(IPC_STRUCT0, 1u, (void *) &msg))
    {
        CY_ASSERT(0u != 0u);
    }
}
#endif /* CY_CPU_CORTEX_M0P || defined(CY_DOXYGEN) */


/* [] END OF FILE */
