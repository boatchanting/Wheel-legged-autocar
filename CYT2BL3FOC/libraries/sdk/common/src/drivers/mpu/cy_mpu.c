/***************************************************************************//**
* \file cy_mpu.c
* \version 1.0
*
* Provides the API definitions of the Cortex-M MPU driver.
*
********************************************************************************
* \copyright
* Copyright 2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "mpu/cy_mpu.h"
#include "cy_device_headers.h"

/*******************************************************************************
* Function Name: Cy_MPU_Setup
****************************************************************************//**
*
* This function setups Core MPU region and enables the MPU.
*
* \param  cfg : Array of settings for each MPU region.
*               \ref cy_stc_mpu_region_cfg_t
* \param  cfgSize: Length of 'cfg' array. If this value is larger than 
*                  maximum value of implemented MPU region. this function
*                  will return immediately.
* \param  privDefMapEn : \ref cy_en_mpu_privdefena_t
* \param  faultNmiEn : \ref cy_en_mpu_hfnmiena_t
*
* \return \cy_en_mpu_status_t
* 
*******************************************************************************/
cy_en_mpu_status_t Cy_MPU_Setup(const cy_stc_mpu_region_cfg_t cfg[], uint8_t cfgSize, cy_en_mpu_privdefena_t privDefMapEn, cy_en_mpu_hfnmiena_t faultNmiEn)
{
    // Check if MPU is implemented or not
    if (MPU->TYPE == 0ul)
    {
        // MPU is not implemented
        return CY_MPU_FAILURE;
    }
    
    // Check for NULL ptr
    if (cfg == NULL)
    {
        return CY_MPU_FAILURE;
    }

    // Check if input configuration array number is not larger than MPU region implemented
    if(cfgSize > CY_MPU_MAX_NUM)
    {
        // input parameter invalid
        return CY_MPU_FAILURE;
    }

    // Ensure all memory accesses are completed before new memory access is committed
    __DMB();

    // Disable the MPU
    MPU->CTRL = 0ul;

    uint32_t i_mpuRegionNo;
    for(i_mpuRegionNo = 0ul; i_mpuRegionNo < CY_MPU_MAX_NUM; i_mpuRegionNo++)
    {
        // Select which MPU region to configure
        MPU->RNR = i_mpuRegionNo;

        if(i_mpuRegionNo < cfgSize)
        {
            // Configure  region  base  address register
            // VALID and REGION field of RBAR register will be 0 since this function sets RNR register manually.
            MPU->RBAR = (cfg[i_mpuRegionNo].addr & MPU_RBAR_ADDR_Msk);

            uint32_t srd;
            if(cfg[i_mpuRegionNo].size < CY_MPU_SIZE_256B)
            {
                srd = 0ul;
            }
            else
            {
                srd = (cfg[i_mpuRegionNo].srd << MPU_RASR_SRD_Pos);
            }

            // Configure region attribute and size register
            MPU->RASR = ((uint32_t)cfg[i_mpuRegionNo].size       |
                         (uint32_t)cfg[i_mpuRegionNo].permission |
                         (uint32_t)cfg[i_mpuRegionNo].attribute  |
                         (uint32_t)cfg[i_mpuRegionNo].execute    |
                         srd                                     |
                         (uint32_t)cfg[i_mpuRegionNo].enable);
        }
        else // Disables unused regions
        {
            // Configure region base address register
            MPU->RBAR = 0ul;

            // Configure region attribute and size register
            MPU->RASR = 0ul;
        }
    }

    // Enable the MPU
    MPU->CTRL = ((uint32_t)privDefMapEn | (uint32_t)faultNmiEn | MPU_CTRL_ENABLE_Msk);

    // Ensure all memory accesses are completed before next instruction is executed
    __DSB();

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();

    return CY_MPU_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_MPU_SetRegion
****************************************************************************//**
*
* This function sets a single region.
* Enabling/Disabling of MPU would need to be done by user
*
* \param  cfg : Settings for MPU region.
*               \ref cy_stc_mpu_region_cfg_t
* \param  regionNr: Region number to be configured
*
* \return \cy_en_mpu_status_t
* 
*******************************************************************************/
cy_en_mpu_status_t Cy_MPU_SetRegion(const cy_stc_mpu_region_cfg_t * cfg, uint8_t regionNr)
{
    // Check if MPU is implemented or not
    if (MPU->TYPE == 0ul)
    {
        // MPU is not implemented
        return CY_MPU_FAILURE;
    }
    
    // Check for NULL ptr
    if (cfg == NULL)
    {
        return CY_MPU_FAILURE;
    }

    // Check if region number is not larger than MPU region implemented
    if(regionNr > CY_MPU_MAX_NUM)
    {
        // input parameter invalid
        return CY_MPU_FAILURE;
    }

    // Ensure all memory accesses are completed before new memory access is committed
    __DMB();


    // Select which MPU region to configure
    MPU->RNR = regionNr;

    // Configure  region  base  address register
    // VALID and REGION field of RBAR register will be 0 since this function sets RNR register manually.
    MPU->RBAR = (cfg->addr & MPU_RBAR_ADDR_Msk);

    uint32_t srd;
    if(cfg->size < CY_MPU_SIZE_256B)
    {
        srd = 0ul;
    }
    else
    {
        srd = (cfg->srd << MPU_RASR_SRD_Pos);
    }

    // Configure region attribute and size register
    MPU->RASR = ((uint32_t)cfg->size       |
                 (uint32_t)cfg->permission |
                 (uint32_t)cfg->attribute  |
                 (uint32_t)cfg->execute    |
                 srd                       |
                 (uint32_t)cfg->enable);

    // Ensure all memory accesses are completed before next instruction is executed
    __DSB();

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();

    return CY_MPU_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_MPU_GetRegion
****************************************************************************//**
*
* This function gets the settings of a single region.
*
* \param  cfg : Settings of the selected MPU region.
*               \ref cy_stc_mpu_region_cfg_t
* \param  regionNr: Region number for which the settings shall be retrieved
*
* \return \cy_en_mpu_status_t
* 
*******************************************************************************/
cy_en_mpu_status_t Cy_MPU_GetRegion(cy_stc_mpu_region_cfg_t * cfg, uint8_t regionNr)
{
    // Check if MPU is implemented or not
    if (MPU->TYPE == 0ul)
    {
        // MPU is not implemented
        return CY_MPU_FAILURE;
    }
    
    // Check for NULL ptr
    if (cfg == NULL)
    {
        return CY_MPU_FAILURE;
    }

    // Check if region number is not larger than MPU region implemented
    if(regionNr > CY_MPU_MAX_NUM)
    {
        // input parameter invalid
        return CY_MPU_FAILURE;
    }


    // Select which MPU region to configure
    MPU->RNR = regionNr;

    // Configure  region  base  address register
    // VALID and REGION field of RBAR register will be 0 since this function sets RNR register manually.
    cfg->addr = MPU->RBAR & MPU_RBAR_ADDR_Msk;
    
    uint32_t rasr = MPU->RASR;
    cfg->srd        = (uint8_t)                ((rasr & MPU_RASR_SRD_Msk) >> MPU_RASR_SRD_Pos);
    cfg->size       = (cy_en_mpu_region_size_t) (rasr & MPU_RASR_SIZE_Msk);
    cfg->permission = (cy_en_mpu_access_p_t)    (rasr & MPU_RASR_AP_Msk);
    cfg->attribute  = (cy_en_mpu_attr_t)        (rasr & (MPU_RASR_TEX_Msk | MPU_RASR_B_Msk | MPU_RASR_C_Msk | MPU_RASR_S_Msk));
    cfg->execute    = (cy_en_mpu_execute_n_t)   (rasr & MPU_RASR_XN_Msk);
    cfg->enable     = (cy_en_mpu_region_en_t)   (rasr & MPU_RASR_ENABLE_Msk);

    return CY_MPU_SUCCESS;
}


void Cy_MPU_Enable(void)
{
    // Ensure all memory accesses are completed before new memory access is committed
    __DSB();

    // Enable the MPU
    MPU->CTRL |= MPU_CTRL_ENABLE_Msk;

    // Ensure all memory accesses are completed before next instruction is executed
    __DSB();

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();
}


void Cy_MPU_Disable(void)
{
    // Ensure all memory accesses are completed before new memory access is committed
    __DSB();

    // Disable the MPU
    MPU->CTRL &= (~MPU_CTRL_ENABLE_Msk);

    // Ensure all memory accesses are completed before next instruction is executed
    __DSB();

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();
}

void Cy_MPU_GetGlobalControlBits(cy_stc_mpu_global_ctrl_bits_t * mpuGlobalCtrl)
{
    uint32_t mpuCtrl = MPU->CTRL;

    mpuGlobalCtrl->privDefMapEn    = (cy_en_mpu_privdefena_t)(mpuCtrl & MPU_CTRL_PRIVDEFENA_Msk);
    mpuGlobalCtrl->faultNmiEn      = (cy_en_mpu_hfnmiena_t)  (mpuCtrl & MPU_CTRL_HFNMIENA_Msk);
    mpuGlobalCtrl->mpuGlobalEnable = (cy_en_mpu_global_en_t) (mpuCtrl & MPU_CTRL_ENABLE_Msk);
}


void Cy_MPU_SetGlobalControlBits(const cy_stc_mpu_global_ctrl_bits_t * mpuGlobalCtrl)
{
    // Ensure all memory accesses are completed before new memory access is committed
    __DSB();
    
    // Set the MPU global control bits
    MPU->CTRL = (uint32_t)mpuGlobalCtrl->mpuGlobalEnable | (uint32_t)mpuGlobalCtrl->privDefMapEn | (uint32_t)mpuGlobalCtrl->faultNmiEn;
    
    // Ensure all memory accesses are completed before next instruction is executed
    __DSB();

    // Flush the pipeline and ensure all previous instructions are completed before executing new instructions
    __ISB();
}

/* [] END OF FILE */
