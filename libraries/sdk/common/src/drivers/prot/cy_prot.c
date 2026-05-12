/***************************************************************************//**
* \file cy_prot.c
* \version 1.0
*
* \brief
* Provides an API implementation of the Protection Unit driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_prot.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*******************************************************************************
* Function Name: Cy_Prot_ConfigBusMaster
****************************************************************************//**
*
* \brief Configures the allowed protection contexts, security (secure/non-secure)
* and privilege level of the bus transaction created by the specified master. 
*
* \param busMaster
* Indicates which master needs to be configured. Refer to the CY_PROT_MASTER_X
* defines.
* 
* \param privileged
* Boolean to define the privilege level of all subsequent bus transfers.
* True - privileged, False - not privileged.
* Note that this is an inherited value. If not inherited, then this bit will 
* be used.
*
* \param secure
* Security setting for the master. True - Secure, False - Not secure.
* 
* \param pcMask
* This is a 16 bit value of the allowed contexts, it is an OR'ed (|) field of the
* provided defines in cy_prot.h. For example: (PROT_PC1 | PROT_PC3 | PROT_PC4)
* 
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The function completed successfully
*   CY_PROT_FAILURE      | The resource is locked
*   CY_PROT_BAD_PARAM    | An incorrect/invalid parameter was passed
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_ConfigBusMaster(en_prot_master_t busMaster, bool privileged, bool secure, uint32_t pcMask)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;
    un_PROT_SMPU_MS0_CTL_t tProtSmpuMs0Ctl = {0};
    uint32_t * addrMsCtl = (uint32_t *)(PROT_BASE + (uint32_t)((uint32_t)busMaster << CY_PROT_MSX_CTL_SHIFT));

    if((uint32_t)(pcMask & CY_PROT_MPU_PC_LIMIT_MASK) != 0UL) 
    {
        /* PC mask out of range - not supported in device */
        status = CY_PROT_BAD_PARAM;
    }
    else
    {

        tProtSmpuMs0Ctl.stcField.u1NS = !secure;
        tProtSmpuMs0Ctl.stcField.u1P = privileged;
        tProtSmpuMs0Ctl.stcField.u15PC_MASK_15_TO_1 = pcMask;
        
        *addrMsCtl = tProtSmpuMs0Ctl.u32Register; // regVal;
        status = ((*addrMsCtl != tProtSmpuMs0Ctl.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS);
    }
    
    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_SetActivePC
****************************************************************************//**
*
* \brief Sets the current/active protection context of the specified bus master.
* 
* Allowed PC values are 1-15. If this value is not inherited from another bus 
* master, the value set through this function is used.
*
* \param busMaster
* The bus master to configure. Refer to the CY_PROT_MASTER_X defines.
* 
* \param pc
* Active protection context of the specified master. Note that only those 
* protection contexts allowed by the pcMask will take effect. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The function completed successfully
*   CY_PROT_FAILURE      | The resource is locked
*   CY_PROT_BAD_PARAM    | An incorrect/invalid parameter was passed
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_SetActivePC(en_prot_master_t busMaster, uint32_t pc)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;
    un_PROT_MPU_MS_CTL_t tProtMpuMsCtl = {0};
    volatile stc_PROT_MPU_t* addrMpu = (stc_PROT_MPU_t*)(&PROT->CYMPU[busMaster]);

    if(pc > (uint32_t)CY_PROT_MS_PC_NR_MAX)
    {
        /* Invalid PC value - not supported in device */
        status = CY_PROT_BAD_PARAM;
    }
    else
    {
        tProtMpuMsCtl.stcField.u4PC = pc;
        addrMpu->unMS_CTL.u32Register = tProtMpuMsCtl.u32Register;
        status = ((addrMpu->unMS_CTL.stcField.u4PC != pc) ? CY_PROT_FAILURE : CY_PROT_SUCCESS);
    }

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_GetActivePC
****************************************************************************//**
*
* \brief Returns the active protection context of a master. 
*
* \param busMaster
* The bus master, whose protection context is being read. Refer to the 
* CY_PROT_MASTER_X defines.
* 
* \return
* Active protection context of the master
*  
*******************************************************************************/
uint32_t Cy_Prot_GetActivePC(en_prot_master_t busMaster)
{
    volatile stc_PROT_MPU_t* addrMpu = (stc_PROT_MPU_t*)(&PROT->CYMPU[busMaster]);

    return ((uint32_t)addrMpu->unMS_CTL.stcField.u4PC);
}


/*******************************************************************************
* Function Name: Cy_Prot_ConfigMpuStruct
****************************************************************************//**
*
* \brief This function configures a memory protection unit (MPU) struct with its 
* protection attributes. 
*
* The protection structs act like the gatekeepers for a master's accesses to
* memory, allowing only the permitted transactions to go through.
*
* \param base
* The base address for the MPU struct being configured. 
* 
* \param config
* Initialization structure containing all the protection attributes.
* 
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The MPU struct was configured
*   CY_PROT_FAILURE      | Configuration failed due to a protection violation
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_ConfigMpuStruct(volatile stc_PROT_MPU_MPU_STRUCT_t* base, const cy_stc_mpu_cfg_t* config)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    stc_PROT_MPU_MPU_STRUCT_t tProtMpuMpuStruct = {0};

    tProtMpuMpuStruct.unADDR.stcField.u8SUBREGION_DISABLE = config->subregions;
    tProtMpuMpuStruct.unADDR.stcField.u24ADDR24 = (uint32_t)((uint32_t)config->address >> CY_PROT_ADDR_SHIFT);

    tProtMpuMpuStruct.unATT.stcField.u5REGION_SIZE = config->regionSize;
    tProtMpuMpuStruct.unATT.stcField.u1NS = !(config->secure);
    tProtMpuMpuStruct.unATT.stcField.u1UR = (config->userPermission & CY_PROT_PERM_R);
    tProtMpuMpuStruct.unATT.stcField.u1UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
    tProtMpuMpuStruct.unATT.stcField.u1UX = (config->userPermission & CY_PROT_PERM_X) >> 2;
    tProtMpuMpuStruct.unATT.stcField.u1PR = (config->privPermission & CY_PROT_PERM_R);
    tProtMpuMpuStruct.unATT.stcField.u1PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
    tProtMpuMpuStruct.unATT.stcField.u1PX = (config->privPermission & CY_PROT_PERM_X) >> 2;
      
    base->unATT.u32Register = tProtMpuMpuStruct.unATT.u32Register;
    base->unADDR.u32Register = tProtMpuMpuStruct.unADDR.u32Register;

    status = ((base->unADDR.u32Register != tProtMpuMpuStruct.unADDR.u32Register) || 
              (base->unATT.u32Register != tProtMpuMpuStruct.unATT.u32Register)) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_EnableMpuStruct
****************************************************************************//**
*
* \brief Enables the MPU struct, which allows the MPU protection attributes to 
* take effect. 
*
* \param base
* The base address of the MPU struct being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The MPU struct was enabled
*   CY_PROT_FAILURE      | The MPU struct is disabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_EnableMpuStruct(volatile stc_PROT_MPU_MPU_STRUCT_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unATT.stcField.u1ENABLED = CY_PROT_STRUCT_ENABLE;

    status = (base->unATT.stcField.u1ENABLED != CY_PROT_STRUCT_ENABLE) ?
                CY_PROT_FAILURE : CY_PROT_SUCCESS;

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_DisableMpuStruct
****************************************************************************//**
*
* \brief Disbles the MPU struct, which prevents the MPU protection attributes
* from taking effect.
*
* \param base
* The base address of the MPU struct being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The MPU struct was disabled
*   CY_PROT_FAILURE      | The MPU struct is enabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_DisableMpuStruct(volatile stc_PROT_MPU_MPU_STRUCT_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unATT.stcField.u1ENABLED &= ~CY_PROT_STRUCT_ENABLE;

    status = (base->unATT.stcField.u1ENABLED == CY_PROT_STRUCT_ENABLE) ?
            CY_PROT_FAILURE : CY_PROT_SUCCESS;

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();

    return status;
}

/*******************************************************************************
* Function Name: Cy_Prot_ConfigSmpuMasterStruct
****************************************************************************//**
*
* \brief Configures a Shared Memory Protection Unit (SMPU) master protection 
* struct with its protection attributes. 
*
* This function configures the master struct governing the corresponding slave
* struct pair. It is a mechanism to protect the slave SMPU struct. Since the
* memory location of the slave struct is known, the address, regionSize and
* subregions of the configuration struct are not applicable.
*
* Note that only the user/privileged write permissions are configurable. The 
* read and execute permissions are read-only and cannot be configured.
*
* \param base
* The register base address of the protection struct being configured.
* 
* \param config
* Initialization structure with all the protection attributes.
* 
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | SMPU master struct was successfully configured
*   CY_PROT_FAILURE      | The resource is locked
*   CY_PROT_BAD_PARAM    | An incorrect/invalid parameter was passed
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_ConfigSmpuMasterStruct(volatile stc_PROT_SMPU_SMPU_STRUCT_t* base, const cy_stc_smpu_cfg_t* config)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;
    //uint32_t attReg;
    un_PROT_SMPU_SMPU_STRUCT_ATT1_t tProtSmpuSmpuStructAttr1 = {0};
    
    if(((uint32_t)config->pcMask & CY_PROT_SMPU_PC_LIMIT_MASK) != 0UL) 
    {
        /* PC mask out of range - not supported in device */
        status = CY_PROT_BAD_PARAM;
    }
    else
    {
        /* ADDR1 is read only. Only configure ATT1 */
        tProtSmpuSmpuStructAttr1.stcField.u15PC_MASK_15_TO_1 = config->pcMask;
        tProtSmpuSmpuStructAttr1.stcField.u1PC_MATCH = config->pcMatch;
        tProtSmpuSmpuStructAttr1.stcField.u1NS = !(config->secure);
        
        tProtSmpuSmpuStructAttr1.stcField.u1UR = (config->userPermission & CY_PROT_PERM_R);
        tProtSmpuSmpuStructAttr1.stcField.u1UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tProtSmpuSmpuStructAttr1.stcField.u1UX = (config->userPermission & CY_PROT_PERM_X) >> 2;
        tProtSmpuSmpuStructAttr1.stcField.u1PR = (config->privPermission & CY_PROT_PERM_R);
        tProtSmpuSmpuStructAttr1.stcField.u1PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tProtSmpuSmpuStructAttr1.stcField.u1PX = (config->privPermission & CY_PROT_PERM_X) >> 2;

        base->unATT1.u32Register = tProtSmpuSmpuStructAttr1.u32Register; //attReg;
        status = (base->unATT1.u32Register != tProtSmpuSmpuStructAttr1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
    }
    
    return status;
}

/*******************************************************************************
* Function Name: Cy_Prot_ConfigSmpuSlaveStruct
****************************************************************************//**
*
* \brief Configures a Shared Memory Protection Unit (SMPU) slave protection 
* struct with its protection attributes. 
* 
* This function configures the slave struct of an SMPU pair, which can protect
* any memory region in a device from invalid bus master accesses.
*
* \param base
* The register base address of the protection structure being configured.
* 
* \param config
* Initialization structure with all the protection attributes.
* 
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | SMPU slave struct was successfully configured
*   CY_PROT_FAILURE      | The resource is locked
*   CY_PROT_BAD_PARAM    | An incorrect/invalid parameter was passed
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_ConfigSmpuSlaveStruct(volatile stc_PROT_SMPU_SMPU_STRUCT_t* base, const cy_stc_smpu_cfg_t* config)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;
    un_PROT_SMPU_SMPU_STRUCT_ADDR0_t tprotSmpuSmpuStruct_ADDR0 = { 0 };
    un_PROT_SMPU_SMPU_STRUCT_ATT0_t  tprotSmpuSmpuStruct_ATT0 = { 0 };  

    if(((uint32_t)config->pcMask & CY_PROT_SMPU_PC_LIMIT_MASK) != 0UL) 
    {
        /* PC mask out of range - not supported in device */
        status = CY_PROT_BAD_PARAM;
    }
    else
    {

        tprotSmpuSmpuStruct_ADDR0.stcField.u8SUBREGION_DISABLE = config->subregions;
        tprotSmpuSmpuStruct_ADDR0.stcField.u24ADDR24 = (uint32_t)((uint32_t)config->address >> CY_PROT_ADDR_SHIFT);

        tprotSmpuSmpuStruct_ATT0.stcField.u1PC_MASK_0 = 1; // This value is read only. The default value is "1".
        tprotSmpuSmpuStruct_ATT0.stcField.u1NS = !(config->secure);
        tprotSmpuSmpuStruct_ATT0.stcField.u15PC_MASK_15_TO_1 = config->pcMask;
        tprotSmpuSmpuStruct_ATT0.stcField.u5REGION_SIZE = config->regionSize;
        tprotSmpuSmpuStruct_ATT0.stcField.u1PC_MATCH = config->pcMatch;
        tprotSmpuSmpuStruct_ATT0.stcField.u1UR = (config->userPermission & CY_PROT_PERM_R);
        tprotSmpuSmpuStruct_ATT0.stcField.u1UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tprotSmpuSmpuStruct_ATT0.stcField.u1UX = (config->userPermission & CY_PROT_PERM_X) >> 2;
        tprotSmpuSmpuStruct_ATT0.stcField.u1PR = (config->privPermission & CY_PROT_PERM_R);
        tprotSmpuSmpuStruct_ATT0.stcField.u1PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tprotSmpuSmpuStruct_ATT0.stcField.u1PX = (config->privPermission & CY_PROT_PERM_X) >> 2;

        base->unATT0.u32Register = tprotSmpuSmpuStruct_ATT0.u32Register; // attReg;
        base->unADDR0.u32Register = tprotSmpuSmpuStruct_ADDR0.u32Register; // addrReg;

        status = ((base->unADDR0.u32Register != tprotSmpuSmpuStruct_ADDR0.u32Register) || 
                  (base->unATT0.u32Register != tprotSmpuSmpuStruct_ATT0.u32Register))
                    ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
    }

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_EnableSmpuMasterStruct
****************************************************************************//**
*
* \brief Enables the Master SMPU structure.
*
* This is an SMPU master struct enable function. The SMPU protection settings
* will take effect after successful completion of this function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Master PU struct was enabled
*   CY_PROT_FAILURE      | The Master PU struct is disabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_EnableSmpuMasterStruct(volatile stc_PROT_SMPU_SMPU_STRUCT_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unATT1.stcField.u1ENABLED = CY_PROT_STRUCT_ENABLE;

    // Check if the SMPU structure really was enabled.
    // Note this also ensures previous write access to complete before execution of below ISB.
    status = (base->unATT1.stcField.u1ENABLED != CY_PROT_STRUCT_ENABLE) ?
                CY_PROT_FAILURE : CY_PROT_SUCCESS;

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_DisableSmpuMasterStruct
****************************************************************************//**
*
* \brief Disables the Master SMPU structure.
*
* This is an SMPU master struct disable function. The SMPU protection settings
* will seize to take effect after successful completion of this function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Master PU struct was disabled
*   CY_PROT_FAILURE      | The Master PU struct is enabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_DisableSmpuMasterStruct(volatile stc_PROT_SMPU_SMPU_STRUCT_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unATT1.stcField.u1ENABLED &= ~CY_PROT_STRUCT_ENABLE;

    // Check if the SMPU structure really was enabled.
    // Note this also ensures previous write access to complete before execution of below ISB.
    status = (base->unATT1.stcField.u1ENABLED == CY_PROT_STRUCT_ENABLE) ?
                CY_PROT_FAILURE : CY_PROT_SUCCESS;

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_EnableSmpuSlaveStruct
****************************************************************************//**
*
* \brief Enables the Slave SMPU structure.
*
* This is an SMPU slave struct enable function. The SMPU protection settings
* will take effect after successful completion of this function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Slave PU struct was enabled
*   CY_PROT_FAILURE      | The Slave PU struct is disabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_EnableSmpuSlaveStruct(volatile stc_PROT_SMPU_SMPU_STRUCT_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unATT0.stcField.u1ENABLED = CY_PROT_STRUCT_ENABLE;

    // Check if the SMPU structure really was enabled.
    // Note this also ensures previous write access to complete before execution of below ISB.
    status = (base->unATT0.stcField.u1ENABLED != CY_PROT_STRUCT_ENABLE) ?
                CY_PROT_FAILURE : CY_PROT_SUCCESS;

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_DisableSmpuSlaveStruct
****************************************************************************//**
*
* \brief Disables the Slave SMPU structure.
*
* This is an SMPU slave struct disable function. The SMPU protection settings
* will seize to take effect after successful completion of this function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Slave PU struct was disabled
*   CY_PROT_FAILURE      | The Slave PU struct is enabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_DisableSmpuSlaveStruct(volatile stc_PROT_SMPU_SMPU_STRUCT_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unATT0.stcField.u1ENABLED &= ~CY_PROT_STRUCT_ENABLE;

    // Check if the SMPU structure really was enabled.
    // Note this also ensures previous write access to complete before execution of below ISB.
    status = (base->unATT0.stcField.u1ENABLED == CY_PROT_STRUCT_ENABLE) ?
                CY_PROT_FAILURE : CY_PROT_SUCCESS;

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_ConfigPpuProgMasterStruct
****************************************************************************//**
*
* \brief Configures a Programmable Peripheral Protection Unit (PPU PROG) master 
* protection struct with its protection attributes. 
*
* This function configures the master struct governing the corresponding slave
* struct pair. It is a mechanism to protect the slave PPU PROG struct. Since
* the memory location of the slave struct is known, the address, regionSize and
* subregions of the configuration struct are not applicable.
*
* Note that only the user/privileged write permissions are configurable. The 
* read and execute permissions are read-only and cannot be configured.
*
* \param base
* The register base address of the protection struct being configured.
* 
* \param cy_en_prot_pc_t
* Protection context value for which this function configures.
* Although this function accepts CY_PROT_PC0, the accessing to setting register
* for CY_PROT_PC0 will be ignored.
*
* \param config
* Initialization structure with all the protection attributes.
* 
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | PPU PROG master struct was successfully configured
*   CY_PROT_FAILURE      | The resource is locked
*   CY_PROT_BAD_PARAM    | An incorrect/invalid parameter was passed
*  
*******************************************************************************/
//rmkn: stc_PERI_MS_PPU_GR changed to stc_PERI_MS_PPU_PR
cy_en_prot_status_t Cy_Prot_ConfigPpuProgMasterStructAttr(volatile stc_PERI_MS_PPU_PR_t* base, cy_en_prot_pc_t setPC, const cy_stc_ppu_prog_attr_cfg_t* config)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;
    un_PERI_MS_PPU_PR_MS_ATT0_t tempMS_ATT0 = { 0 };
    un_PERI_MS_PPU_PR_MS_ATT1_t tempMS_ATT1 = { 0 };
    un_PERI_MS_PPU_PR_MS_ATT2_t tempMS_ATT2 = { 0 };
    un_PERI_MS_PPU_PR_MS_ATT3_t tempMS_ATT3 = { 0 };

    if(setPC >= CY_PROT_PC_NUM) //rmkn: CY_PROT_PERM_X)
    {
        return CY_PROT_BAD_PARAM;
    }

    switch(setPC)
    {
    case CY_PROT_PC0:
        tempMS_ATT0.u32Register = base->unMS_ATT0.u32Register;
        tempMS_ATT0.stcField.u1PC0_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC0_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC0_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC0_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC0_NS = !(config->secure);
        base->unMS_ATT0.u32Register = tempMS_ATT0.u32Register;
        status = (base->unMS_ATT0.u32Register != tempMS_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC1:
        tempMS_ATT0.u32Register = base->unMS_ATT0.u32Register;
        tempMS_ATT0.stcField.u1PC1_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC1_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC1_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC1_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC1_NS = !(config->secure);
        base->unMS_ATT0.u32Register = tempMS_ATT0.u32Register;
        status = (base->unMS_ATT0.u32Register != tempMS_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC2:
        tempMS_ATT0.u32Register = base->unMS_ATT0.u32Register;
        tempMS_ATT0.stcField.u1PC2_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC2_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC2_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC2_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC2_NS = !(config->secure);
        base->unMS_ATT0.u32Register = tempMS_ATT0.u32Register;
        status = (base->unMS_ATT0.u32Register != tempMS_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC3:
        tempMS_ATT0.u32Register = base->unMS_ATT0.u32Register;
        tempMS_ATT0.stcField.u1PC3_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC3_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC3_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC3_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC3_NS = !(config->secure);
        base->unMS_ATT0.u32Register = tempMS_ATT0.u32Register;
        status = (base->unMS_ATT0.u32Register != tempMS_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC4:
        tempMS_ATT1.u32Register = base->unMS_ATT1.u32Register;
        tempMS_ATT1.stcField.u1PC4_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC4_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC4_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC4_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC4_NS = !(config->secure);
        base->unMS_ATT1.u32Register = tempMS_ATT1.u32Register;
        status = (base->unMS_ATT1.u32Register != tempMS_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC5:
        tempMS_ATT1.u32Register = base->unMS_ATT1.u32Register;
        tempMS_ATT1.stcField.u1PC5_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC5_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC5_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC5_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC5_NS = !(config->secure);
        base->unMS_ATT1.u32Register = tempMS_ATT1.u32Register;
        status = (base->unMS_ATT1.u32Register != tempMS_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC6:
        tempMS_ATT1.u32Register = base->unMS_ATT1.u32Register;
        tempMS_ATT1.stcField.u1PC6_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC6_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC6_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC6_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC6_NS = !(config->secure);
        base->unMS_ATT1.u32Register = tempMS_ATT1.u32Register;
        status = (base->unMS_ATT1.u32Register != tempMS_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC7:
        tempMS_ATT1.u32Register = base->unMS_ATT1.u32Register;
        tempMS_ATT1.stcField.u1PC7_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC7_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC7_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC7_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC7_NS = !(config->secure);
        base->unMS_ATT1.u32Register = tempMS_ATT1.u32Register;
        status = (base->unMS_ATT1.u32Register != tempMS_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC8:
        tempMS_ATT2.u32Register = base->unMS_ATT2.u32Register;
        tempMS_ATT2.stcField.u1PC8_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC8_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC8_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC8_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC8_NS = !(config->secure);
        base->unMS_ATT2.u32Register = tempMS_ATT2.u32Register;
        status = (base->unMS_ATT2.u32Register != tempMS_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC9:
        tempMS_ATT2.u32Register = base->unMS_ATT2.u32Register;
        tempMS_ATT2.stcField.u1PC9_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC9_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC9_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC9_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC9_NS = !(config->secure);
        base->unMS_ATT2.u32Register = tempMS_ATT2.u32Register;
        status = (base->unMS_ATT2.u32Register != tempMS_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC10:
        tempMS_ATT2.u32Register = base->unMS_ATT2.u32Register;
        tempMS_ATT2.stcField.u1PC10_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC10_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC10_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC10_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC10_NS = !(config->secure);
        base->unMS_ATT2.u32Register = tempMS_ATT2.u32Register;
        status = (base->unMS_ATT2.u32Register != tempMS_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC11:
        tempMS_ATT2.u32Register = base->unMS_ATT2.u32Register;
        tempMS_ATT2.stcField.u1PC11_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC11_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC11_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC11_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC11_NS = !(config->secure);
        base->unMS_ATT2.u32Register = tempMS_ATT2.u32Register;
        status = (base->unMS_ATT2.u32Register != tempMS_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC12:
        tempMS_ATT3.u32Register = base->unMS_ATT3.u32Register;
        tempMS_ATT3.stcField.u1PC12_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC12_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC12_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC12_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC12_NS = !(config->secure);
        base->unMS_ATT3.u32Register = tempMS_ATT3.u32Register;
        status = (base->unMS_ATT3.u32Register != tempMS_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC13:
        tempMS_ATT3.u32Register = base->unMS_ATT3.u32Register;
        tempMS_ATT3.stcField.u1PC13_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC13_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC13_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC13_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC13_NS = !(config->secure);
        base->unMS_ATT3.u32Register = tempMS_ATT3.u32Register;
        status = (base->unMS_ATT3.u32Register != tempMS_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC14:
        tempMS_ATT3.u32Register = base->unMS_ATT3.u32Register;
        tempMS_ATT3.stcField.u1PC14_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC14_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC14_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC14_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC14_NS = !(config->secure);
        base->unMS_ATT3.u32Register = tempMS_ATT3.u32Register;
        status = (base->unMS_ATT3.u32Register != tempMS_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC15:
        tempMS_ATT3.u32Register = base->unMS_ATT3.u32Register;
        tempMS_ATT3.stcField.u1PC15_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC15_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC15_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC15_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC15_NS = !(config->secure);
        base->unMS_ATT3.u32Register = tempMS_ATT3.u32Register;
        status = (base->unMS_ATT3.u32Register != tempMS_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    default:
        return CY_PROT_BAD_PARAM;
    }

    return status;
}

/*******************************************************************************
* Function Name: Cy_Prot_ConfigPpuProgSlaveStruct
****************************************************************************//**
*
* \brief Configures a Programmable Peripheral Protection Unit (PPU PROG) slave
* protection struct with its protection attributes. 
* 
* This function configures the slave struct of a PPU PROG pair, which can 
* protect any peripheral memory region in a device from invalid bus master 
* accesses.
*
* Note that the user/privileged execute accesses are read-only and are always 
* enabled. 
*
* \param base
* The register base address of the protection structure being configured.
* 
* \param cy_en_prot_pc_t
* Protection context value for which this function configures.
* Although this function accepts CY_PROT_PC0, the accessing to setting register
* for CY_PROT_PC0 will be ignored.
*
* \param config
* Initialization structure with all the protection attributes.
* 
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | PPU PROG slave struct was successfully configured
*   CY_PROT_FAILURE      | The resource is locked
*   CY_PROT_BAD_PARAM    | An incorrect/invalid parameter was passed
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_ConfigPpuProgSlaveStructAttr(volatile stc_PERI_MS_PPU_PR_t* base, cy_en_prot_pc_t setPC, const cy_stc_ppu_prog_attr_cfg_t* config)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;
    un_PERI_MS_PPU_PR_SL_ATT0_t tempSL_ATT0 = { 0 };
    un_PERI_MS_PPU_PR_SL_ATT1_t tempSL_ATT1 = { 0 };
    un_PERI_MS_PPU_PR_SL_ATT2_t tempSL_ATT2 = { 0 };
    un_PERI_MS_PPU_PR_SL_ATT3_t tempSL_ATT3 = { 0 };

    if(setPC >= CY_PROT_PC_NUM)
    {
        return CY_PROT_BAD_PARAM;
    }

    switch(setPC)
    {
    case CY_PROT_PC0:
        tempSL_ATT0.u32Register = base->unSL_ATT0.u32Register;
        tempSL_ATT0.stcField.u1PC0_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC0_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC0_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC0_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC0_NS = !(config->secure);
        base->unSL_ATT0.u32Register = tempSL_ATT0.u32Register;
        status = (base->unSL_ATT0.u32Register != tempSL_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC1:
        tempSL_ATT0.u32Register = base->unSL_ATT0.u32Register;
        tempSL_ATT0.stcField.u1PC1_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC1_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC1_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC1_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC1_NS = !(config->secure);
        base->unSL_ATT0.u32Register = tempSL_ATT0.u32Register;
        status = (base->unSL_ATT0.u32Register != tempSL_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC2:
        tempSL_ATT0.u32Register = base->unSL_ATT0.u32Register;
        tempSL_ATT0.stcField.u1PC2_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC2_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC2_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC2_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC2_NS = !(config->secure);
        base->unSL_ATT0.u32Register = tempSL_ATT0.u32Register;
        status = (base->unSL_ATT0.u32Register != tempSL_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC3:
        tempSL_ATT0.u32Register = base->unSL_ATT0.u32Register;
        tempSL_ATT0.stcField.u1PC3_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC3_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC3_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC3_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC3_NS = !(config->secure);
        base->unSL_ATT0.u32Register = tempSL_ATT0.u32Register;
        status = (base->unSL_ATT0.u32Register != tempSL_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC4:
        tempSL_ATT1.u32Register = base->unSL_ATT1.u32Register;
        tempSL_ATT1.stcField.u1PC4_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC4_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC4_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC4_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC4_NS = !(config->secure);
        base->unSL_ATT1.u32Register = tempSL_ATT1.u32Register;
        status = (base->unSL_ATT1.u32Register != tempSL_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC5:
        tempSL_ATT1.u32Register = base->unSL_ATT1.u32Register;
        tempSL_ATT1.stcField.u1PC5_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC5_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC5_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC5_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC5_NS = !(config->secure);
        base->unSL_ATT1.u32Register = tempSL_ATT1.u32Register;
        status = (base->unSL_ATT1.u32Register != tempSL_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC6:
        tempSL_ATT1.u32Register = base->unSL_ATT1.u32Register;
        tempSL_ATT1.stcField.u1PC6_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC6_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC6_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC6_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC6_NS = !(config->secure);
        base->unSL_ATT1.u32Register = tempSL_ATT1.u32Register;
        status = (base->unSL_ATT1.u32Register != tempSL_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC7:
        tempSL_ATT1.u32Register = base->unSL_ATT1.u32Register;
        tempSL_ATT1.stcField.u1PC7_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC7_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC7_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC7_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC7_NS = !(config->secure);
        base->unSL_ATT1.u32Register = tempSL_ATT1.u32Register;
        status = (base->unSL_ATT1.u32Register != tempSL_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC8:
        tempSL_ATT2.u32Register = base->unSL_ATT2.u32Register;
        tempSL_ATT2.stcField.u1PC8_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC8_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC8_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC8_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC8_NS = !(config->secure);
        base->unSL_ATT2.u32Register = tempSL_ATT2.u32Register;
        status = (base->unSL_ATT2.u32Register != tempSL_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC9:
        tempSL_ATT2.u32Register = base->unSL_ATT2.u32Register;
        tempSL_ATT2.stcField.u1PC9_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC9_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC9_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC9_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC9_NS = !(config->secure);
        base->unSL_ATT2.u32Register = tempSL_ATT2.u32Register;
        status = (base->unSL_ATT2.u32Register != tempSL_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC10:
        tempSL_ATT2.u32Register = base->unSL_ATT2.u32Register;
        tempSL_ATT2.stcField.u1PC10_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC10_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC10_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC10_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC10_NS = !(config->secure);
        base->unSL_ATT2.u32Register = tempSL_ATT2.u32Register;
        status = (base->unSL_ATT2.u32Register != tempSL_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC11:
        tempSL_ATT2.u32Register = base->unSL_ATT2.u32Register;
        tempSL_ATT2.stcField.u1PC11_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC11_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC11_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC11_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC11_NS = !(config->secure);
        base->unSL_ATT2.u32Register = tempSL_ATT2.u32Register;
        status = (base->unSL_ATT2.u32Register != tempSL_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC12:
        tempSL_ATT3.u32Register = base->unSL_ATT3.u32Register;
        tempSL_ATT3.stcField.u1PC12_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC12_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC12_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC12_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC12_NS = !(config->secure);
        base->unSL_ATT3.u32Register = tempSL_ATT3.u32Register;
        status = (base->unSL_ATT3.u32Register != tempSL_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC13:
        tempSL_ATT3.u32Register = base->unSL_ATT3.u32Register;
        tempSL_ATT3.stcField.u1PC13_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC13_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC13_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC13_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC13_NS = !(config->secure);
        base->unSL_ATT3.u32Register = tempSL_ATT3.u32Register;
        status = (base->unSL_ATT3.u32Register != tempSL_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC14:
        tempSL_ATT3.u32Register = base->unSL_ATT3.u32Register;
        tempSL_ATT3.stcField.u1PC14_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC14_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC14_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC14_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC14_NS = !(config->secure);
        base->unSL_ATT3.u32Register = tempSL_ATT3.u32Register;
        status = (base->unSL_ATT3.u32Register != tempSL_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC15:
        tempSL_ATT3.u32Register = base->unSL_ATT3.u32Register;
        tempSL_ATT3.stcField.u1PC15_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC15_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC15_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC15_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC15_NS = !(config->secure);
        base->unSL_ATT3.u32Register = tempSL_ATT3.u32Register;
        status = (base->unSL_ATT3.u32Register != tempSL_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    default:
        return CY_PROT_BAD_PARAM;
    }

    return status;
}

cy_en_prot_status_t Cy_Prot_ConfigPpuProgSlaveStructReg(volatile stc_PERI_MS_PPU_PR_t* base, const cy_stc_ppu_prog_region_cfg_t* config)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;
    un_PERI_MS_PPU_PR_SL_ADDR_t tempSL_ADDR = { 0 };
    un_PERI_MS_PPU_PR_SL_SIZE_t tempSL_SIZE = { 0 };

    tempSL_ADDR.u32Register = base->unSL_ADDR.u32Register;
    tempSL_ADDR.stcField.u30ADDR30 = (uint32_t)((uint32_t)config->address >> CY_PROT_ADDR_SHIFT_PPU);
    base->unSL_ADDR.u32Register = tempSL_ADDR.u32Register;

    tempSL_SIZE.u32Register = base->unSL_SIZE.u32Register;
    tempSL_SIZE.stcField.u5REGION_SIZE = config->regionSize;
    base->unSL_SIZE.u32Register = tempSL_SIZE.u32Register;

    status = ((base->unSL_SIZE.stcField.u5REGION_SIZE != tempSL_SIZE.stcField.u5REGION_SIZE) ||
             (base->unSL_ADDR.u32Register != tempSL_ADDR.u32Register))
             ? CY_PROT_FAILURE : CY_PROT_SUCCESS;

    return status;
}
/*******************************************************************************
* Function Name: Cy_Prot_EnablePpuProgMasterStruct
****************************************************************************//**
*
* \brief Enables the Master PPU PROG structure.
*
* This is a PPU PROG master struct enable function. The PPU PROG protection 
* settings will take effect after successful completion of this function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Master PU struct was enabled
*   CY_PROT_FAILURE      | The Master PU struct is disabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_EnablePpuProgMasterStruct(volatile stc_PERI_MS_PPU_PR_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unMS_SIZE.stcField.u1VALID = 1u; // 1: enable
    status = (base->unMS_SIZE.stcField.u1VALID != 1u) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_DisablePpuProgMasterStruct
****************************************************************************//**
*
* \brief Disables the Master PPU PROG structure.
*
* This is a PPU PROG master struct disable function. The PPU PROG protection 
* settings will seize to take effect after successful completion of this 
* function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Master PU struct was disabled
*   CY_PROT_FAILURE      | The Master PU struct is enabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_DisablePpuProgMasterStruct(volatile stc_PERI_MS_PPU_PR_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unMS_SIZE.stcField.u1VALID = 0u; // 0: disable
    status = (base->unMS_SIZE.stcField.u1VALID != 0u) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_EnablePpuProgSlaveStruct
****************************************************************************//**
*
* \brief Enables the Slave PPU PROG structure.
*
* This is a PPU PROG slave struct enable function. The PPU PROG protection 
* settings will take effect after successful completion of this function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Slave PU struct was enabled
*   CY_PROT_FAILURE      | The Slave PU struct is disabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_EnablePpuProgSlaveStruct(volatile stc_PERI_MS_PPU_PR_t* base)
{   
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unSL_SIZE.stcField.u1VALID = 1u; // 1: enable
    status = (base->unSL_SIZE.stcField.u1VALID != 1u) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_DisablePpuProgSlaveStruct
****************************************************************************//**
*
* \brief Disables the Slave PPU PROG structure.
*
* This is a PPU PROG slave struct disable function. The PPU PROG protection 
* settings will seize to take effect after successful completion of this 
* function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Slave PU struct was disabled
*   CY_PROT_FAILURE      | The Slave PU struct is enabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_DisablePpuProgSlaveStruct(volatile stc_PERI_MS_PPU_PR_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unSL_SIZE.stcField.u1VALID = 0u; // 0: disabl
    status = (base->unSL_SIZE.stcField.u1VALID != 0u) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_ConfigPpuFixedGrMasterStruct
****************************************************************************//**
*
* \brief Configures a Fixed Peripheral Group Protection Unit (PPU GR) master 
* protection struct with its protection attributes. 
*
* This function configures the master struct governing the corresponding slave
* struct pair. It is a mechanism to protect the slave PPU GR struct. Since
* the memory location of the slave struct is known, the address, regionSize and
* subregions of the configuration struct are not applicable.
*
* Note that only the user/privileged write permissions are configurable. The 
* read and execute permissions are read-only and cannot be configured.
*
* \param base
* The register base address of the protection struct being configured.
* 
* \param cy_en_prot_pc_t
* Protection context value for which this function configures.
* Although this function accepts CY_PROT_PC0, the accessing to setting register
* for CY_PROT_PC0 will be ignored.
*
* \param config
* Initialization structure with all the protection attributes.
* 
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | PPU GR master struct was successfully configured
*   CY_PROT_FAILURE      | The resource is locked
*   CY_PROT_BAD_PARAM    | An incorrect/invalid parameter was passed
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_ConfigPpuFixedMasterStruct(volatile stc_PERI_MS_PPU_FX_t* base, cy_en_prot_pc_t setPC, const cy_stc_ppu_gr_cfg_t* config)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;
    un_PERI_MS_PPU_PR_MS_ATT0_t tempMS_ATT0 = { 0 };
    un_PERI_MS_PPU_PR_MS_ATT1_t tempMS_ATT1 = { 0 };
    un_PERI_MS_PPU_PR_MS_ATT2_t tempMS_ATT2 = { 0 };
    un_PERI_MS_PPU_PR_MS_ATT3_t tempMS_ATT3 = { 0 };

    if(setPC >= CY_PROT_PC_NUM)
    {
        return CY_PROT_BAD_PARAM;
    }

    switch(setPC)
    {
    case CY_PROT_PC0:
        tempMS_ATT0.u32Register = base->unMS_ATT0.u32Register;
        tempMS_ATT0.stcField.u1PC0_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC0_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC0_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC0_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC0_NS = !(config->secure);
        base->unMS_ATT0.u32Register = tempMS_ATT0.u32Register;
        status = (base->unMS_ATT0.u32Register != tempMS_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC1:
        tempMS_ATT0.u32Register = base->unMS_ATT0.u32Register;
        tempMS_ATT0.stcField.u1PC1_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC1_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC1_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC1_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC1_NS = !(config->secure);
        base->unMS_ATT0.u32Register = tempMS_ATT0.u32Register;
        status = (base->unMS_ATT0.u32Register != tempMS_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC2:
        tempMS_ATT0.u32Register = base->unMS_ATT0.u32Register;
        tempMS_ATT0.stcField.u1PC2_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC2_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC2_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC2_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC2_NS = !(config->secure);
        base->unMS_ATT0.u32Register = tempMS_ATT0.u32Register;
        status = (base->unMS_ATT0.u32Register != tempMS_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC3:
        tempMS_ATT0.u32Register = base->unMS_ATT0.u32Register;
        tempMS_ATT0.stcField.u1PC3_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC3_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC3_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT0.stcField.u1PC3_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT0.stcField.u1PC3_NS = !(config->secure);
        base->unMS_ATT0.u32Register = tempMS_ATT0.u32Register;
        status = (base->unMS_ATT0.u32Register != tempMS_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC4:
        tempMS_ATT1.u32Register = base->unMS_ATT1.u32Register;
        tempMS_ATT1.stcField.u1PC4_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC4_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC4_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC4_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC4_NS = !(config->secure);
        base->unMS_ATT1.u32Register = tempMS_ATT1.u32Register;
        status = (base->unMS_ATT1.u32Register != tempMS_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC5:
        tempMS_ATT1.u32Register = base->unMS_ATT1.u32Register;
        tempMS_ATT1.stcField.u1PC5_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC5_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC5_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC5_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC5_NS = !(config->secure);
        base->unMS_ATT1.u32Register = tempMS_ATT1.u32Register;
        status = (base->unMS_ATT1.u32Register != tempMS_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC6:
        tempMS_ATT1.u32Register = base->unMS_ATT1.u32Register;
        tempMS_ATT1.stcField.u1PC6_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC6_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC6_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC6_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC6_NS = !(config->secure);
        base->unMS_ATT1.u32Register = tempMS_ATT1.u32Register;
        status = (base->unMS_ATT1.u32Register != tempMS_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC7:
        tempMS_ATT1.u32Register = base->unMS_ATT1.u32Register;
        tempMS_ATT1.stcField.u1PC7_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC7_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC7_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT1.stcField.u1PC7_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT1.stcField.u1PC7_NS = !(config->secure);
        base->unMS_ATT1.u32Register = tempMS_ATT1.u32Register;
        status = (base->unMS_ATT1.u32Register != tempMS_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC8:
        tempMS_ATT2.u32Register = base->unMS_ATT2.u32Register;
        tempMS_ATT2.stcField.u1PC8_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC8_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC8_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC8_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC8_NS = !(config->secure);
        base->unMS_ATT2.u32Register = tempMS_ATT2.u32Register;
        status = (base->unMS_ATT2.u32Register != tempMS_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC9:
        tempMS_ATT2.u32Register = base->unMS_ATT2.u32Register;
        tempMS_ATT2.stcField.u1PC9_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC9_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC9_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC9_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC9_NS = !(config->secure);
        base->unMS_ATT2.u32Register = tempMS_ATT2.u32Register;
        status = (base->unMS_ATT2.u32Register != tempMS_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC10:
        tempMS_ATT2.u32Register = base->unMS_ATT2.u32Register;
        tempMS_ATT2.stcField.u1PC10_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC10_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC10_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC10_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC10_NS = !(config->secure);
        base->unMS_ATT2.u32Register = tempMS_ATT2.u32Register;
        status = (base->unMS_ATT2.u32Register != tempMS_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC11:
        tempMS_ATT2.u32Register = base->unMS_ATT2.u32Register;
        tempMS_ATT2.stcField.u1PC11_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC11_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC11_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT2.stcField.u1PC11_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT2.stcField.u1PC11_NS = !(config->secure);
        base->unMS_ATT2.u32Register = tempMS_ATT2.u32Register;
        status = (base->unMS_ATT2.u32Register != tempMS_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC12:
        tempMS_ATT3.u32Register = base->unMS_ATT3.u32Register;
        tempMS_ATT3.stcField.u1PC12_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC12_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC12_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC12_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC12_NS = !(config->secure);
        base->unMS_ATT3.u32Register = tempMS_ATT3.u32Register;
        status = (base->unMS_ATT3.u32Register != tempMS_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC13:
        tempMS_ATT3.u32Register = base->unMS_ATT3.u32Register;
        tempMS_ATT3.stcField.u1PC13_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC13_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC13_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC13_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC13_NS = !(config->secure);
        base->unMS_ATT3.u32Register = tempMS_ATT3.u32Register;
        status = (base->unMS_ATT3.u32Register != tempMS_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC14:
        tempMS_ATT3.u32Register = base->unMS_ATT3.u32Register;
        tempMS_ATT3.stcField.u1PC14_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC14_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC14_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC14_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC14_NS = !(config->secure);
        base->unMS_ATT3.u32Register = tempMS_ATT3.u32Register;
        status = (base->unMS_ATT3.u32Register != tempMS_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC15:
        tempMS_ATT3.u32Register = base->unMS_ATT3.u32Register;
        tempMS_ATT3.stcField.u1PC15_UR = (config->userPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC15_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC15_PR = (config->privPermission & CY_PROT_PERM_R);
        tempMS_ATT3.stcField.u1PC15_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempMS_ATT3.stcField.u1PC15_NS = !(config->secure);
        base->unMS_ATT3.u32Register = tempMS_ATT3.u32Register;
        status = (base->unMS_ATT3.u32Register != tempMS_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    default:
        return CY_PROT_BAD_PARAM;
    }

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_ConfigPpuFixedGrSlaveStruct
****************************************************************************//**
*
* \brief Configures a Fixed Peripheral Group Protection Unit (PPU GR) slave
* protection struct with its protection attributes. 
* 
* This function configures the slave struct of a PPU GR pair, which can 
* protect an entire peripheral MMIO group from invalid bus master accesses.
* Refer to the device Technical Reference manual for details on peripheral
* MMIO grouping and which peripherals belong to which groups.
*
* Each fixed PPU GR is devoted to a defined MMIO group. Hence the address, 
* regionSize and subregions of the configuration struct are not applicable.
*
* Note that the user/privileged execute accesses are read-only and are always 
* enabled. 
*
* \param base
* The register base address of the protection structure being configured.
* 
* \param cy_en_prot_pc_t
* Protection context value for which this function configures.
* Although this function accepts CY_PROT_PC0, the accessing to setting register
* for CY_PROT_PC0 will be ignored.
*
* \param config
* Initialization structure with all the protection attributes.
* 
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | PPU GR slave struct was successfully configured
*   CY_PROT_FAILURE      | The resource is locked
*   CY_PROT_BAD_PARAM    | An incorrect/invalid parameter was passed
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_ConfigPpuFixedSlaveStruct(volatile stc_PERI_MS_PPU_FX_t* base, cy_en_prot_pc_t setPC, const cy_stc_ppu_gr_cfg_t* config)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;
    un_PERI_MS_PPU_PR_SL_ATT0_t tempSL_ATT0 = { 0 };
    un_PERI_MS_PPU_PR_SL_ATT1_t tempSL_ATT1 = { 0 };
    un_PERI_MS_PPU_PR_SL_ATT2_t tempSL_ATT2 = { 0 };
    un_PERI_MS_PPU_PR_SL_ATT3_t tempSL_ATT3 = { 0 };

    if(setPC >= CY_PROT_PC_NUM)
    {
        return CY_PROT_BAD_PARAM;
    }

    switch(setPC)
    {
    case CY_PROT_PC0:
        tempSL_ATT0.u32Register = base->unSL_ATT0.u32Register;
        tempSL_ATT0.stcField.u1PC0_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC0_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC0_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC0_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC0_NS = !(config->secure);
        base->unSL_ATT0.u32Register = tempSL_ATT0.u32Register;
        status = (base->unSL_ATT0.u32Register != tempSL_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC1:
        tempSL_ATT0.u32Register = base->unSL_ATT0.u32Register;
        tempSL_ATT0.stcField.u1PC1_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC1_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC1_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC1_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC1_NS = !(config->secure);
        base->unSL_ATT0.u32Register = tempSL_ATT0.u32Register;
        status = (base->unSL_ATT0.u32Register != tempSL_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC2:
        tempSL_ATT0.u32Register = base->unSL_ATT0.u32Register;
        tempSL_ATT0.stcField.u1PC2_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC2_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC2_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC2_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC2_NS = !(config->secure);
        base->unSL_ATT0.u32Register = tempSL_ATT0.u32Register;
        status = (base->unSL_ATT0.u32Register != tempSL_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC3:
        tempSL_ATT0.u32Register = base->unSL_ATT0.u32Register;
        tempSL_ATT0.stcField.u1PC3_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC3_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC3_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT0.stcField.u1PC3_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT0.stcField.u1PC3_NS = !(config->secure);
        base->unSL_ATT0.u32Register = tempSL_ATT0.u32Register;
        status = (base->unSL_ATT0.u32Register != tempSL_ATT0.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC4:
        tempSL_ATT1.u32Register = base->unSL_ATT1.u32Register;
        tempSL_ATT1.stcField.u1PC4_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC4_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC4_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC4_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC4_NS = !(config->secure);
        base->unSL_ATT1.u32Register = tempSL_ATT1.u32Register;
        status = (base->unSL_ATT1.u32Register != tempSL_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC5:
        tempSL_ATT1.u32Register = base->unSL_ATT1.u32Register;
        tempSL_ATT1.stcField.u1PC5_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC5_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC5_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC5_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC5_NS = !(config->secure);
        base->unSL_ATT1.u32Register = tempSL_ATT1.u32Register;
        status = (base->unSL_ATT1.u32Register != tempSL_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC6:
        tempSL_ATT1.u32Register = base->unSL_ATT1.u32Register;
        tempSL_ATT1.stcField.u1PC6_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC6_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC6_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC6_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC6_NS = !(config->secure);
        base->unSL_ATT1.u32Register = tempSL_ATT1.u32Register;
        status = (base->unSL_ATT1.u32Register != tempSL_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC7:
        tempSL_ATT1.u32Register = base->unSL_ATT1.u32Register;
        tempSL_ATT1.stcField.u1PC7_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC7_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC7_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT1.stcField.u1PC7_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT1.stcField.u1PC7_NS = !(config->secure);
        base->unSL_ATT1.u32Register = tempSL_ATT1.u32Register;
        status = (base->unSL_ATT1.u32Register != tempSL_ATT1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC8:
        tempSL_ATT2.u32Register = base->unSL_ATT2.u32Register;
        tempSL_ATT2.stcField.u1PC8_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC8_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC8_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC8_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC8_NS = !(config->secure);
        base->unSL_ATT2.u32Register = tempSL_ATT2.u32Register;
        status = (base->unSL_ATT2.u32Register != tempSL_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC9:
        tempSL_ATT2.u32Register = base->unSL_ATT2.u32Register;
        tempSL_ATT2.stcField.u1PC9_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC9_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC9_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC9_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC9_NS = !(config->secure);
        base->unSL_ATT2.u32Register = tempSL_ATT2.u32Register;
        status = (base->unSL_ATT2.u32Register != tempSL_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC10:
        tempSL_ATT2.u32Register = base->unSL_ATT2.u32Register;
        tempSL_ATT2.stcField.u1PC10_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC10_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC10_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC10_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC10_NS = !(config->secure);
        base->unSL_ATT2.u32Register = tempSL_ATT2.u32Register;
        status = (base->unSL_ATT2.u32Register != tempSL_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC11:
        tempSL_ATT2.u32Register = base->unSL_ATT2.u32Register;
        tempSL_ATT2.stcField.u1PC11_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC11_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC11_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT2.stcField.u1PC11_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT2.stcField.u1PC11_NS = !(config->secure);
        base->unSL_ATT2.u32Register = tempSL_ATT2.u32Register;
        status = (base->unSL_ATT2.u32Register != tempSL_ATT2.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC12:
        tempSL_ATT3.u32Register = base->unSL_ATT3.u32Register;
        tempSL_ATT3.stcField.u1PC12_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC12_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC12_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC12_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC12_NS = !(config->secure);
        base->unSL_ATT3.u32Register = tempSL_ATT3.u32Register;
        status = (base->unSL_ATT3.u32Register != tempSL_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC13:
        tempSL_ATT3.u32Register = base->unSL_ATT3.u32Register;
        tempSL_ATT3.stcField.u1PC13_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC13_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC13_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC13_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC13_NS = !(config->secure);
        base->unSL_ATT3.u32Register = tempSL_ATT3.u32Register;
        status = (base->unSL_ATT3.u32Register != tempSL_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC14:
        tempSL_ATT3.u32Register = base->unSL_ATT3.u32Register;
        tempSL_ATT3.stcField.u1PC14_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC14_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC14_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC14_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC14_NS = !(config->secure);
        base->unSL_ATT3.u32Register = tempSL_ATT3.u32Register;
        status = (base->unSL_ATT3.u32Register != tempSL_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    case CY_PROT_PC15:
        tempSL_ATT3.u32Register = base->unSL_ATT3.u32Register;
        tempSL_ATT3.stcField.u1PC15_UR = (config->userPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC15_UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC15_PR = (config->privPermission & CY_PROT_PERM_R);
        tempSL_ATT3.stcField.u1PC15_PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tempSL_ATT3.stcField.u1PC15_NS = !(config->secure);
        base->unSL_ATT3.u32Register = tempSL_ATT3.u32Register;
        status = (base->unSL_ATT3.u32Register != tempSL_ATT3.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
        break;
    default:
        return CY_PROT_BAD_PARAM;
    }

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_EnablePpuFixedGrMasterStruct
****************************************************************************//**
*
* \brief Enables the Master PPU GR structure.
*
* This is a PPU GR master struct enable function. The PPU GR protection 
* settings will take effect after successful completion of this function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Master PU struct was enabled
*   CY_PROT_FAILURE      | The Master PU struct is disabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_EnablePpuFixedMasterStruct(volatile stc_PERI_MS_PPU_FX_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unMS_SIZE.stcField.u1VALID = 1u; // 1: enable
    status = (base->unMS_SIZE.stcField.u1VALID != 1u) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_DisablePpuFixedGrMasterStruct
****************************************************************************//**
*
* \brief Disables the Master PPU GR structure.
*
* This is a PPU GR master struct disable function. The PPU GR protection 
* settings will seize to take effect after successful completion of this 
* function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Master PU struct was disabled
*   CY_PROT_FAILURE      | The Master PU struct is enabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_DisablePpuFixedMasterStruct(volatile stc_PERI_MS_PPU_FX_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unMS_SIZE.stcField.u1VALID = 0u; // 0: disable
    status = (base->unMS_SIZE.stcField.u1VALID != 0u) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_EnablePpuFixedGrSlaveStruct
****************************************************************************//**
*
* \brief Enables the Slave PPU GR structure.
*
* This is a PPU GR slave struct enable function. The PPU GR protection 
* settings will take effect after successful completion of this function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Slave PU struct was enabled
*   CY_PROT_FAILURE      | The Slave PU struct is disabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_EnablePpuFixedSlaveStruct(volatile stc_PERI_MS_PPU_FX_t* base)
{   
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unSL_SIZE.stcField.u1VALID = 1u; // 1: enable
    status = (base->unSL_SIZE.stcField.u1VALID != 1u) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;

    return status;
}


/*******************************************************************************
* Function Name: Cy_Prot_DisablePpuFixedGrSlaveStruct
****************************************************************************//**
*
* \brief Disables the Slave PPU GR structure.
*
* This is a PPU GR slave struct disable function. The PPU GR protection 
* settings will seize to take effect after successful completion of this 
* function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Slave PU struct was disabled
*   CY_PROT_FAILURE      | The Slave PU struct is enabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_DisablePpuFixedSlaveStruct(volatile stc_PERI_MS_PPU_FX_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unSL_SIZE.stcField.u1VALID = 0u; // 0: disabl
    status = (base->unSL_SIZE.stcField.u1VALID != 0u) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;

    return status;
}

#if defined(LPDDR4_EMPU_EMPU_STRUCT_NR) && (LPDDR4_EMPU_EMPU_STRUCT_NR >= 0)

/*******************************************************************************
* Function Name: Cy_Prot_ConfigEmpuMasterStruct
****************************************************************************//**
*
* \brief Configures a External Memory Protection Unit (EMPU) master protection 
* struct with its protection attributes. 
*
* This function configures the master struct governing the corresponding slave
* struct pair. It is a mechanism to protect the slave EMPU struct. Since the
* memory location of the slave struct is known, the address, regionSize and
* subregions of the configuration struct are not applicable.
*
* Note that only the user/privileged write permissions are configurable. The 
* read and execute permissions are read-only and cannot be configured.
*
* \param base
* The register base address of the protection struct being configured.
* 
* \param config
* Initialization structure with all the protection attributes.
* 
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | EMPU master struct was successfully configured
*   CY_PROT_FAILURE      | The resource is locked
*   CY_PROT_BAD_PARAM    | An incorrect/invalid parameter was passed
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_ConfigEmpuMasterStruct(volatile stc_LPDDR4_EMPU_EMPU_STRUCT_t* base, const cy_stc_empu_cfg_t* config)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;
    un_LPDDR4_EMPU_EMPU_STRUCT_ATT1_t tProtEmpuEmpuStructAttr1 = {0};
    
    if(((uint32_t)config->pcMask & CY_PROT_EMPU_PC_LIMIT_MASK) != 0UL) 
    {
        /* PC mask out of range - not supported in device */
        status = CY_PROT_BAD_PARAM;
    }
    else
    {
        /* ADDR1 is read only. Only configure ATT1 */
        tProtEmpuEmpuStructAttr1.stcField.u15PC_MASK_15_TO_1 = config->pcMask;
        tProtEmpuEmpuStructAttr1.stcField.u1PC_MATCH = config->pcMatch;
        tProtEmpuEmpuStructAttr1.stcField.u1NS = !(config->secure);
        
        tProtEmpuEmpuStructAttr1.stcField.u1UR = (config->userPermission & CY_PROT_PERM_R);
        tProtEmpuEmpuStructAttr1.stcField.u1UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tProtEmpuEmpuStructAttr1.stcField.u1UX = (config->userPermission & CY_PROT_PERM_X) >> 2;
        tProtEmpuEmpuStructAttr1.stcField.u1PR = (config->privPermission & CY_PROT_PERM_R);
        tProtEmpuEmpuStructAttr1.stcField.u1PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tProtEmpuEmpuStructAttr1.stcField.u1PX = (config->privPermission & CY_PROT_PERM_X) >> 2;

        base->unATT1.u32Register = tProtEmpuEmpuStructAttr1.u32Register;
        status = (base->unATT1.u32Register != tProtEmpuEmpuStructAttr1.u32Register) ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
    }
    
    return status;
}

/*******************************************************************************
* Function Name: Cy_Prot_ConfigEmpuSlaveStruct
****************************************************************************//**
*
* \brief Configures a External Memory Protection Unit (EMPU) slave protection 
* struct with its protection attributes. 
* 
* This function configures the slave struct of an EMPU pair, which can protect
* any memory region in a device from invalid bus master accesses.
*
* \param base
* The register base address of the protection structure being configured.
* 
* \param config
* Initialization structure with all the protection attributes.
* 
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | EMPU slave struct was successfully configured
*   CY_PROT_FAILURE      | The resource is locked
*   CY_PROT_BAD_PARAM    | An incorrect/invalid parameter was passed
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_ConfigEmpuSlaveStruct(volatile stc_LPDDR4_EMPU_EMPU_STRUCT_t* base, const cy_stc_empu_cfg_t* config)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;
    un_LPDDR4_EMPU_EMPU_STRUCT_ADDR0_t tprotEmpuEmpuStruct_ADDR0 = { 0 };
    un_LPDDR4_EMPU_EMPU_STRUCT_ATT0_t  tprotEmpuEmpuStruct_ATT0 = { 0 };  

    if(((uint32_t)config->pcMask & CY_PROT_EMPU_PC_LIMIT_MASK) != 0UL) 
    {
        /* PC mask out of range - not supported in device */
        status = CY_PROT_BAD_PARAM;
    }
    else
    {
        tprotEmpuEmpuStruct_ADDR0.stcField.u8SUBREGION_DISABLE = config->subregions;
        tprotEmpuEmpuStruct_ADDR0.stcField.u24ADDR24 = (uint32_t)((uint32_t)config->address >> CY_PROT_ADDR_SHIFT);

        tprotEmpuEmpuStruct_ATT0.stcField.u1PC_MASK_0 = 1; // This value is read only. The default value is "1".
        tprotEmpuEmpuStruct_ATT0.stcField.u1NS = !(config->secure);
        tprotEmpuEmpuStruct_ATT0.stcField.u15PC_MASK_15_TO_1 = config->pcMask;
        tprotEmpuEmpuStruct_ATT0.stcField.u5REGION_SIZE = config->regionSize;
        tprotEmpuEmpuStruct_ATT0.stcField.u1PC_MATCH = config->pcMatch;
        tprotEmpuEmpuStruct_ATT0.stcField.u1UR = (config->userPermission & CY_PROT_PERM_R);
        tprotEmpuEmpuStruct_ATT0.stcField.u1UW = (config->userPermission & CY_PROT_PERM_W) >> 1;
        tprotEmpuEmpuStruct_ATT0.stcField.u1UX = (config->userPermission & CY_PROT_PERM_X) >> 2;
        tprotEmpuEmpuStruct_ATT0.stcField.u1PR = (config->privPermission & CY_PROT_PERM_R);
        tprotEmpuEmpuStruct_ATT0.stcField.u1PW = (config->privPermission & CY_PROT_PERM_W) >> 1;
        tprotEmpuEmpuStruct_ATT0.stcField.u1PX = (config->privPermission & CY_PROT_PERM_X) >> 2;

        base->unATT0.u32Register = tprotEmpuEmpuStruct_ATT0.u32Register; // attReg;
        base->unADDR0.u32Register = tprotEmpuEmpuStruct_ADDR0.u32Register; // addrReg;

        status = ((base->unADDR0.u32Register != tprotEmpuEmpuStruct_ADDR0.u32Register) || 
                  (base->unATT0.u32Register != tprotEmpuEmpuStruct_ATT0.u32Register))
                    ? CY_PROT_FAILURE : CY_PROT_SUCCESS;
    }

    return status;
}

/*******************************************************************************
* Function Name: Cy_Prot_EnableEmpuMasterStruct
****************************************************************************//**
*
* \brief Enables the Master EMPU structure.
*
* This is an EMPU master struct enable function. The EMPU protection settings
* will take effect after successful completion of this function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Master PU struct was enabled
*   CY_PROT_FAILURE      | The Master PU struct is disabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_EnableEmpuMasterStruct(volatile stc_LPDDR4_EMPU_EMPU_STRUCT_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unATT1.stcField.u1ENABLED = CY_PROT_STRUCT_ENABLE;

    // Check if the EMPU structure really was enabled.
    // Note this also ensures previous write access to complete before execution of below ISB.
    status = (base->unATT1.stcField.u1ENABLED != CY_PROT_STRUCT_ENABLE) ?
                CY_PROT_FAILURE : CY_PROT_SUCCESS;

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();

    return status;
}

/*******************************************************************************
* Function Name: Cy_Prot_DisableEmpuMasterStruct
****************************************************************************//**
*
* \brief Disables the Master EMPU structure.
*
* This is an EMPU master struct disable function. The EMPU protection settings
* will seize to take effect after successful completion of this function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Master PU struct was disabled
*   CY_PROT_FAILURE      | The Master PU struct is enabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_DisableEmpuMasterStruct(volatile stc_LPDDR4_EMPU_EMPU_STRUCT_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unATT1.stcField.u1ENABLED &= ~CY_PROT_STRUCT_ENABLE;

    // Check if the EMPU structure really was enabled.
    // Note this also ensures previous write access to complete before execution of below ISB.
    status = (base->unATT1.stcField.u1ENABLED == CY_PROT_STRUCT_ENABLE) ?
                CY_PROT_FAILURE : CY_PROT_SUCCESS;

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();

    return status;
}

/*******************************************************************************
* Function Name: Cy_Prot_EnableEmpuSlaveStruct
****************************************************************************//**
*
* \brief Enables the Slave EMPU structure.
*
* This is an EMPU slave struct enable function. The EMPU protection settings
* will take effect after successful completion of this function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Slave PU struct was enabled
*   CY_PROT_FAILURE      | The Slave PU struct is disabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_EnableEmpuSlaveStruct(volatile stc_LPDDR4_EMPU_EMPU_STRUCT_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unATT0.stcField.u1ENABLED = CY_PROT_STRUCT_ENABLE;

    // Check if the EMPU structure really was enabled.
    // Note this also ensures previous write access to complete before execution of below ISB.
    status = (base->unATT0.stcField.u1ENABLED != CY_PROT_STRUCT_ENABLE) ?
                CY_PROT_FAILURE : CY_PROT_SUCCESS;

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();

    return status;
}

/*******************************************************************************
* Function Name: Cy_Prot_DisableEmpuSlaveStruct
****************************************************************************//**
*
* \brief Disables the Slave EMPU structure.
*
* This is an EMPU slave struct disable function. The EMPU protection settings
* will seize to take effect after successful completion of this function call.
*
* \param base
* The base address for the protection unit structure being configured. 
*
* \return
* Status of the function call.
*
*   Status               | Description
*   ------------         | -----------
*   CY_PROT_SUCCESS      | The Slave PU struct was disabled
*   CY_PROT_FAILURE      | The Slave PU struct is enabled and possibly locked
*  
*******************************************************************************/
cy_en_prot_status_t Cy_Prot_DisableEmpuSlaveStruct(volatile stc_LPDDR4_EMPU_EMPU_STRUCT_t* base)
{
    cy_en_prot_status_t status = CY_PROT_SUCCESS;

    base->unATT0.stcField.u1ENABLED &= ~CY_PROT_STRUCT_ENABLE;

    // Check if the EMPU structure really was enabled.
    // Note this also ensures previous write access to complete before execution of below ISB.
    status = (base->unATT0.stcField.u1ENABLED == CY_PROT_STRUCT_ENABLE) ?
                CY_PROT_FAILURE : CY_PROT_SUCCESS;

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();

    return status;
}

#endif

#if defined(__cplusplus)
}
#endif

/* [] END OF FILE */
