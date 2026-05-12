/***************************************************************************//**
* \file cy_mw_flash.h
* \version 1.0
*
* \brief
* Provides an simplified Flash API
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_mw_flash Middleware Flash (MW Flash)
* \{
* Middleware APIs for flash access.
* 
* \defgroup group_mw_flash_functions Functions
*/

#if !defined(CY_MW_FLASH_H)
#define CY_MW_FLASH_H

#include "cy_device_headers.h"
#include "flash/cy_flash.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
* \addtogroup group_mw_flash_functions
* \{
*/

bool Cy_Is_SROM_API_Completed(void);
void Cy_FlashInit(bool blocking);
void Cy_FlashSectorErase(uint32_t sectorAddr, cy_en_flash_driver_blocking_t blocking);
bool Cy_WorkFlashBlankCheck(uint32_t sectorAddr, cy_en_flash_driver_blocking_t blocking);
void Cy_FlashWriteWork(uint32_t writeAddr, const uint32_t* data, cy_en_flash_driver_blocking_t blocking);
void Cy_FlashWriteCode(uint32_t writeAddr, const uint32_t* data, cy_en_flash_programrow_datasize_t size, cy_en_flash_driver_blocking_t blocking);

#if defined (tviibh16m)
bool Cy_Is_SROM_API_Completed1(void);
void Cy_FlashInit1(bool blocking);
void Cy_FlashSectorErase1(uint32_t sectorAddr, cy_en_flash_driver_blocking_t blocking);
bool Cy_WorkFlashBlankCheck1(uint32_t sectorAddr, cy_en_flash_driver_blocking_t blocking);
void Cy_FlashWriteWork1(uint32_t writeAddr, const uint32_t* data, cy_en_flash_driver_blocking_t blocking);
void Cy_FlashWriteCode1(uint32_t writeAddr, const uint32_t* data, cy_en_flash_programrow_datasize_t size, cy_en_flash_driver_blocking_t blocking);
#endif

/** \} group_mw_flash_functions */

#if defined(__cplusplus)
}
#endif

#endif /* CY_MW_FLASH_H */

/* [] END OF FILE */
