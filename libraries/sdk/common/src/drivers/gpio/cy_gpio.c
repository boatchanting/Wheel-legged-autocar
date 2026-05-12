/***************************************************************************//**
* \file cy_gpio.c
* \version 1.0
*
* \brief
* Provides an API implementation of the GPIO driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_gpio.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*******************************************************************************
* Function Name: Cy_GPIO_Pin_Init
****************************************************************************//**
*
* \brief Initialize all pin configuration setting for the pin.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \param config
* Pointer to the pin config structure base address
*
* \return
* void
*
* \note 
* This function modifies port registers in read-modify-write operations. It is
* not thread safe as the resource is shared among multiple pins on a port.
*
*******************************************************************************/
cy_en_gpio_status_t Cy_GPIO_Pin_Init(volatile stc_GPIO_PRT_t *base, uint32_t pinNum, const cy_stc_gpio_pin_config_t *config)
{
    cy_en_gpio_status_t status = CY_GPIO_SUCCESS;

    if((NULL != base) && (NULL != config))
    {
        Cy_GPIO_Write(base, pinNum, config->outVal);
        Cy_GPIO_SetHSIOM(base, pinNum, config->hsiom);
        Cy_GPIO_SetVtrip(base, pinNum, config->vtrip);
        Cy_GPIO_SetSlewRate(base, pinNum, config->slewRate);
        Cy_GPIO_SetDriveSel(base, pinNum, config->driveSel);
        Cy_GPIO_SetDrivemode(base, pinNum, config->driveMode);
        Cy_GPIO_SetInterruptEdge(base, pinNum, config->intEdge);
        Cy_GPIO_ClearInterrupt(base, pinNum);
        Cy_GPIO_SetInterruptMask(base, pinNum, config->intMask);
    }
    else
    {
        status = CY_GPIO_BAD_PARAM;
    }
    
    return(status);
}

/*******************************************************************************
* Function Name: Cy_GPIO_Multi_Pin_Init
****************************************************************************//**
*
* \brief Initialize pin configuration setting for multiple pin.
* This will return CY_GPIO_BAD_PARAM, only if configuration paramter does not
* contain any pin configuration.
*
* \param config
* Pointer to top of array of pin configuration structure.
*
* \param pinAmount
* Pin number (amount) to be set.
*
* \return
* cy_en_gpio_status_t
*
*******************************************************************************/
cy_en_gpio_status_t Cy_GPIO_Multi_Pin_Init(const cy_stc_gpio_pin_prt_config_t *config, uint32_t pinAmount)
{
    cy_en_gpio_status_t status = CY_GPIO_SUCCESS;

    if((NULL != config))
    {
        for(uint32_t i = 0ul; i < pinAmount; i++)
        {
            status = Cy_GPIO_Pin_Init(config[i].portReg, config[i].pinNum, &config[i].cfg);
        }
    }
    else
    {
        status = CY_GPIO_BAD_PARAM;
    }

    return(status);
}


/*******************************************************************************
* Function Name: Cy_GPIO_Port_Init
****************************************************************************//**
*
* \brief Initialize a complete port of pins from a single init structure.
*
* \param base
* Pointer to the pin's port register base address
*
* \param config
* Pointer to the pin config structure base address
*
* \return
* void
*
*******************************************************************************/
cy_en_gpio_status_t Cy_GPIO_Port_Init(volatile stc_GPIO_PRT_t* base, const cy_stc_gpio_prt_config_t *config)
{
    cy_en_gpio_status_t status = CY_GPIO_SUCCESS;
    uint32_t portNum;
    stc_HSIOM_PRT_t* baseHSIOM;

    if((NULL != base) && (NULL != config))
    {
        portNum = ((uint32_t)(base) - GPIO_BASE) / GPIO_PRT_SECTION_SIZE;
        baseHSIOM = (stc_HSIOM_PRT_t*)(HSIOM_BASE + (HSIOM_PRT_SECTION_SIZE * portNum));
        
        baseHSIOM->unPORT_SEL0.u32Register = config->sel0Active;
        baseHSIOM->unPORT_SEL1.u32Register = config->sel1Active;
        base->unOUT.u32Register            = config->out;
        base->unCFG_IN.u32Register         = config->cfgIn;
        base->unCFG_OUT.u32Register        = config->cfgOut;
        base->unCFG.u32Register            = config->cfg;
        base->unINTR_CFG.u32Register       = config->intrCfg;
        base->unINTR.u32Register           = 0x1fful;
        base->unINTR_MASK.u32Register      = config->intrMask;
    }
    else
    {
        status = CY_GPIO_BAD_PARAM;
    }
    
    return(status);
}


#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
