/***************************************************************************//**
* \file cy_mw_flash.c
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


/*****************************************************************************
* Include files
*****************************************************************************/
#include "cy_project.h"
#include "cy_device_headers.h"
#include "cy_mw_flash.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*****************************************************************************
* Local pre-processor symbols/macros ('define')
*****************************************************************************/

/*****************************************************************************
* Global variable definitions (declared in header file with 'extern')
*****************************************************************************/

/*****************************************************************************
* Local type definitions ('typedef')
*****************************************************************************/
  
/*****************************************************************************
* Local variable definitions ('static')
*****************************************************************************/
static bool g_completeFlag   = false;
static bool g_NB_ModeEnabled = false;

#if defined (tviibh16m)
static bool g_completeFlag1   = false;
static bool g_NB_ModeEnabled1 = false;
#endif

/*****************************************************************************
* Local function prototypes ('static')                                       *
*****************************************************************************/

/*****************************************************************************
* Function implementation - global ('extern') and local ('static')
*****************************************************************************/

/*******************************************************************************
* Function Cy_FlashHandler
****************************************************************************//**
*
* This is interrupt handler called when SROM API was finished by CM0+ via IPC
* This function will be assigned in "Cy_FlashInit".
*
*******************************************************************************/
static void Cy_FlashHandler(void)
{
    un_srom_api_resps_t apiResp;
    cy_en_srom_api_status_t sromDrvStatus = Cy_Srom_GetApiResponse(&apiResp);
    if(sromDrvStatus != CY_SROM_STATUS_SUCCESS)
    {
        CY_ASSERT(false);
    }

    g_completeFlag = true;
    Cy_Flashc_InvalidateFlashCacheBuffer();
}

/*******************************************************************************
* Function Cy_Is_SROM_API_Completed
****************************************************************************//**
*
* A Function for user to know whether CM0+ completed erase/program Flash or not.
* In non-blocking mode, it return "true" always.
*
* \return true : CM0+ has completed requested SROM API.
*         false: CM0+ has not completed requested SROM API yet.
*
*******************************************************************************/
bool Cy_Is_SROM_API_Completed(void)
{
    return g_completeFlag;
}

/*******************************************************************************
* Function Cy_FlashInit
****************************************************************************//**
*
* This function initialize Flash IP and this MW. Please call this function before
* using any other function in this file.
*
* \param non_blocking true : use non-blocking mode.
*                     false: use blocking mode.
*
*******************************************************************************/
void Cy_FlashInit(bool non_blocking)
{
    if(non_blocking == true)
    {
        /******  Setting for IPCs    ******/
        Cy_Srom_SetResponseHandler(Cy_FlashHandler, CPUIntIdx3_IRQn);
        NVIC_SetPriority(CPUIntIdx3_IRQn, 3ul);
        NVIC_EnableIRQ(CPUIntIdx3_IRQn);
        g_NB_ModeEnabled = true;
    }
    else
    {
        g_NB_ModeEnabled = false;
    }
    (void) g_NB_ModeEnabled; // avoid "unused" compiler warning if NDEBUG is set

    /*  Flash Write Enable   */
    Cy_Flashc_MainWriteEnable();
    Cy_Flashc_WorkWriteEnable();

    g_completeFlag = true;
}

/*******************************************************************************
* Function Cy_FlashSectorErase
****************************************************************************//**
*
* This function will erase flash sector.
*
* \param sectorAddr address of flash sector to be erased
*
* \param blocking Blocking mode or not
*
*******************************************************************************/
void Cy_FlashSectorErase(uint32_t sectorAddr, cy_en_flash_driver_blocking_t blocking)
{
    uint32_t status;
    cy_stc_flash_erasesector_config_t eraseSectorConfig = {0};

    if(blocking == CY_FLASH_DRIVER_NON_BLOCKING)
    {
        CY_ASSERT(g_NB_ModeEnabled == true);

        g_completeFlag = false;
    }

    // Erase work flash sector and verify
    eraseSectorConfig.blocking = CY_FLASH_ERASESECTOR_BLOCKING;
    eraseSectorConfig.intrMask = CY_FLASH_ERASESECTOR_NOT_SET_INTR_MASK;
    eraseSectorConfig.Addr = (uint32_t*)sectorAddr;
    status = Cy_Flash_EraseSector(NULL, &eraseSectorConfig, blocking);
    CY_ASSERT(status == CY_FLASH_DRV_SUCCESS);
    (void) status; // avoid "unused" compiler warning if NDEBUG is set
}

/*******************************************************************************
* Function Cy_WorkFlashBlankCheck
****************************************************************************//**
*
* This function will check work flash blank or not.
* Note this function return true (blank) only if SROM API called without
* error in non-blocking mode.
* 
* \param sectorAddr address of flash sector to be erased
*
* \param blocking Blocking mode or not
*
* \return true: work flash was blank or non-blocking mode
*         false: work flash was not blank
*
*******************************************************************************/
bool Cy_WorkFlashBlankCheck(uint32_t sectorAddr, cy_en_flash_driver_blocking_t blocking)
{
    uint32_t status;
    CY_ASSERT(Cy_Flash_WorkBoundsCheck(sectorAddr) == CY_FLASH_IN_BOUNDS);

    if(blocking == CY_FLASH_DRIVER_NON_BLOCKING)
    {
        CY_ASSERT(g_NB_ModeEnabled == true);

        /* Only for non-blocking operation */
        g_completeFlag = false;
    }

    cy_stc_flash_blankcheck_config_t blankCheckConfig;
    blankCheckConfig.addrToBeChecked       = (uint32_t*)sectorAddr,
    blankCheckConfig.numOfWordsToBeChecked = Cy_Flash_IsWorkSmallSector(sectorAddr)
                                           ? CY_WORK_SES_SIZE_IN_WORD
                                           : CY_WORK_LES_SIZE_IN_WORD;

    status = Cy_Flash_BlankCheck(NULL, &blankCheckConfig, blocking);
    if(status == CY_FLASH_DRV_SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*******************************************************************************
* Function Cy_FlashWriteWork
****************************************************************************//**
*
* This function will program 32bit data to address in work flash
*
* \param writeAddr address of work flash sector to be written into
*
* \param data pointer to data to be written from
*
* \param blocking Blocking mode or not
*
*******************************************************************************/
void Cy_FlashWriteWork(uint32_t writeAddr, const uint32_t* data, cy_en_flash_driver_blocking_t blocking)
{
    CY_ASSERT(Cy_Flash_WorkBoundsCheck(writeAddr) == CY_FLASH_IN_BOUNDS);

    uint32_t status;
    cy_stc_flash_programrow_config_t programRowConfig = {0};

    if(blocking == CY_FLASH_DRIVER_NON_BLOCKING)
    {
        CY_ASSERT(g_NB_ModeEnabled == true);

        /* Only for non-blocking operation */
        g_completeFlag = false;
    }

    //  Program work flash
    programRowConfig.blocking = CY_FLASH_PROGRAMROW_BLOCKING;
    programRowConfig.skipBC   = CY_FLASH_PROGRAMROW_SKIP_BLANK_CHECK;
    programRowConfig.dataSize = CY_FLASH_PROGRAMROW_DATA_SIZE_32BIT;
    programRowConfig.dataLoc  = CY_FLASH_PROGRAMROW_DATA_LOCATION_SRAM;
    programRowConfig.intrMask = CY_FLASH_PROGRAMROW_NOT_SET_INTR_MASK;
    programRowConfig.destAddr = (uint32_t*)writeAddr;
    programRowConfig.dataAddr = data;
    status = Cy_Flash_ProgramRow(NULL, &programRowConfig, blocking);
    CY_ASSERT(status == CY_FLASH_DRV_SUCCESS);
    (void) status; // avoid "unused" compiler warning if NDEBUG is set
}

/*******************************************************************************
* Function Cy_FlashWriteCode
****************************************************************************//**
*
* This function will program data to address in work flash
*
* \param writeAddr address of code flash sector to be written into
*
* \param data pointer to data to be written from
*
* \param size size of written data.
*
* \param blocking Blocking mode or not
*
*******************************************************************************/
void Cy_FlashWriteCode(uint32_t writeAddr, const uint32_t* data, cy_en_flash_programrow_datasize_t size, cy_en_flash_driver_blocking_t blocking)
{
    CY_ASSERT(Cy_Flash_MainBoundsCheck(writeAddr) == CY_FLASH_IN_BOUNDS);

    uint32_t status;
    cy_stc_flash_programrow_config_t programRowConfig = {0};

    if(blocking == CY_FLASH_DRIVER_NON_BLOCKING)
    {
        CY_ASSERT(g_NB_ModeEnabled == true);

        /* Only for non-blocking operation */
        g_completeFlag = false;
    }

    // Program work flash
    programRowConfig.blocking = CY_FLASH_PROGRAMROW_BLOCKING;
    programRowConfig.skipBC   = CY_FLASH_PROGRAMROW_SKIP_BLANK_CHECK;
    programRowConfig.dataSize = size;
    programRowConfig.dataLoc  = CY_FLASH_PROGRAMROW_DATA_LOCATION_SRAM;
    programRowConfig.intrMask = CY_FLASH_PROGRAMROW_NOT_SET_INTR_MASK;
    programRowConfig.destAddr = (uint32_t*)writeAddr;
    programRowConfig.dataAddr = data;
    status = Cy_Flash_ProgramRow(NULL, &programRowConfig, blocking);
    CY_ASSERT(status == CY_FLASH_DRV_SUCCESS);
    (void) status; // avoid "unused" compiler warning if NDEBUG is set
}


#if defined (tviibh16m)
/*******************************************************************************
* Function Cy_FlashHandler1
****************************************************************************//**
*
* This is interrupt handler called when SROM API was finished by CM0+ via IPC
* This function will be assigned in "Cy_FlashInit".
*
*******************************************************************************/
static void Cy_FlashHandler1(void)
{
    un_srom_api_resps_t apiResp;
    cy_en_srom_api_status_t sromDrvStatus = Cy_Srom_GetApiResponse(&apiResp);
    if(sromDrvStatus != CY_SROM_STATUS_SUCCESS)
    {
        CY_ASSERT(false);
    }

    g_completeFlag1 = true;
    Cy_Flashc_InvalidateFlashCacheBuffer1();
}

/*******************************************************************************
* Function Cy_Is_SROM_API_Completed1
****************************************************************************//**
*
* A Function for user to know whether CM0+ completed erase/program Flash or not.
* In non-blocking mode, it return "true" always.
*
* \return true : CM0+ has completed requested SROM API.
*         false: CM0+ has not completed requested SROM API yet.
*
*******************************************************************************/
bool Cy_Is_SROM_API_Completed1(void)
{
    return g_completeFlag1;
}

/*******************************************************************************
* Function Cy_FlashInit1
****************************************************************************//**
*
* This function initialize Flash IP and this MW. Please call this function before
* using any other function in this file.
*
* \param non_blocking true : use non-blocking mode.
*                     false: use blocking mode.
*
*******************************************************************************/
void Cy_FlashInit1(bool non_blocking)
{
    if(non_blocking == true)
    {
        /******  Setting for IPCs    ******/
        Cy_Srom_SetResponseHandler(Cy_FlashHandler1, CPUIntIdx3_IRQn);
        NVIC_SetPriority(CPUIntIdx3_IRQn, 3ul);
        NVIC_EnableIRQ(CPUIntIdx3_IRQn);
        g_NB_ModeEnabled1 = true;
    }
    else
    {
        g_NB_ModeEnabled1 = false;
    }
    (void) g_NB_ModeEnabled1; // avoid "unused" compiler warning if NDEBUG is set

    /*  Flash Write Enable   */
    Cy_Flashc_MainWriteEnable1();
    Cy_Flashc_WorkWriteEnable1();

    g_completeFlag1 = true;
}

/*******************************************************************************
* Function Cy_FlashSectorErase1
****************************************************************************//**
*
* This function will erase flash sector.
*
* \param sectorAddr address of flash sector to be erased
*
* \param blocking Blocking mode or not
*
*******************************************************************************/
void Cy_FlashSectorErase1(uint32_t sectorAddr, cy_en_flash_driver_blocking_t blocking)
{
    uint32_t status;
    cy_stc_flash_erasesector_config_t eraseSectorConfig = {0};

    if(blocking == CY_FLASH_DRIVER_NON_BLOCKING)
    {
        CY_ASSERT(g_NB_ModeEnabled1 == true);

        g_completeFlag1 = false;
    }

    // Erase work flash sector and verify
    eraseSectorConfig.blocking = CY_FLASH_ERASESECTOR_BLOCKING;
    eraseSectorConfig.intrMask = CY_FLASH_ERASESECTOR_NOT_SET_INTR_MASK;
    eraseSectorConfig.Addr = (uint32_t*)sectorAddr;
    status = Cy_Flash_EraseSector1(NULL, &eraseSectorConfig, blocking);
    CY_ASSERT(status == CY_FLASH_DRV_SUCCESS);
    (void) status; // avoid "unused" compiler warning if NDEBUG is set
}

/*******************************************************************************
* Function Cy_WorkFlashBlankCheck1
****************************************************************************//**
*
* This function will check work flash blank or not.
* Note this function return true (blank) only if SROM API called without
* error in non-blocking mode.
* 
* \param sectorAddr address of flash sector to be erased
*
* \param blocking Blocking mode or not
*
* \return true: work flash was blank or non-blocking mode
*         false: work flash was not blank
*
*******************************************************************************/
bool Cy_WorkFlashBlankCheck1(uint32_t sectorAddr, cy_en_flash_driver_blocking_t blocking)
{
    uint32_t status;
    CY_ASSERT(Cy_Flash_WorkBoundsCheck1(sectorAddr) == CY_FLASH_IN_BOUNDS);

    if(blocking == CY_FLASH_DRIVER_NON_BLOCKING)
    {
        CY_ASSERT(g_NB_ModeEnabled1 == true);

        /* Only for non-blocking operation */
        g_completeFlag1 = false;
    }

    cy_stc_flash_blankcheck_config_t blankCheckConfig;
    blankCheckConfig.addrToBeChecked       = (uint32_t*)sectorAddr,
    blankCheckConfig.numOfWordsToBeChecked = Cy_Flash_IsWorkSmallSector1(sectorAddr)
                                           ? CY_WORK_SES_SIZE_IN_WORD
                                           : CY_WORK_LES_SIZE_IN_WORD;

    status = Cy_Flash_BlankCheck1(NULL, &blankCheckConfig, blocking);
    if(status == CY_FLASH_DRV_SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*******************************************************************************
* Function Cy_FlashWriteWork1
****************************************************************************//**
*
* This function will program 32bit data to address in work flash
*
* \param writeAddr address of work flash sector to be written into
*
* \param data pointer to data to be written from
*
* \param blocking Blocking mode or not
*
*******************************************************************************/
void Cy_FlashWriteWork1(uint32_t writeAddr, const uint32_t* data, cy_en_flash_driver_blocking_t blocking)
{
    CY_ASSERT(Cy_Flash_WorkBoundsCheck1(writeAddr) == CY_FLASH_IN_BOUNDS);

    uint32_t status;
    cy_stc_flash_programrow_config_t programRowConfig = {0};

    if(blocking == CY_FLASH_DRIVER_NON_BLOCKING)
    {
        CY_ASSERT(g_NB_ModeEnabled1 == true);

        /* Only for non-blocking operation */
        g_completeFlag1 = false;
    }

    //  Program work flash
    programRowConfig.blocking = CY_FLASH_PROGRAMROW_BLOCKING;
    programRowConfig.skipBC   = CY_FLASH_PROGRAMROW_SKIP_BLANK_CHECK;
    programRowConfig.dataSize = CY_FLASH_PROGRAMROW_DATA_SIZE_32BIT;
    programRowConfig.dataLoc  = CY_FLASH_PROGRAMROW_DATA_LOCATION_SRAM;
    programRowConfig.intrMask = CY_FLASH_PROGRAMROW_NOT_SET_INTR_MASK;
    programRowConfig.destAddr = (uint32_t*)writeAddr;
    programRowConfig.dataAddr = data;
    status = Cy_Flash_ProgramRow1(NULL, &programRowConfig, blocking);
    CY_ASSERT(status == CY_FLASH_DRV_SUCCESS);
    (void) status; // avoid "unused" compiler warning if NDEBUG is set
}

/*******************************************************************************
* Function Cy_FlashWriteCode1
****************************************************************************//**
*
* This function will program data to address in work flash
*
* \param writeAddr address of code flash sector to be written into
*
* \param data pointer to data to be written from
*
* \param size size of written data.
*
* \param blocking Blocking mode or not
*
*******************************************************************************/
void Cy_FlashWriteCode1(uint32_t writeAddr, const uint32_t* data, cy_en_flash_programrow_datasize_t size, cy_en_flash_driver_blocking_t blocking)
{
    CY_ASSERT(Cy_Flash_MainBoundsCheck1(writeAddr) == CY_FLASH_IN_BOUNDS);

    uint32_t status;
    cy_stc_flash_programrow_config_t programRowConfig = {0};

    if(blocking == CY_FLASH_DRIVER_NON_BLOCKING)
    {
        CY_ASSERT(g_NB_ModeEnabled1 == true);

        /* Only for non-blocking operation */
        g_completeFlag1 = false;
    }

    // Program work flash
    programRowConfig.blocking = CY_FLASH_PROGRAMROW_BLOCKING;
    programRowConfig.skipBC   = CY_FLASH_PROGRAMROW_SKIP_BLANK_CHECK;
    programRowConfig.dataSize = size;
    programRowConfig.dataLoc  = CY_FLASH_PROGRAMROW_DATA_LOCATION_SRAM;
    programRowConfig.intrMask = CY_FLASH_PROGRAMROW_NOT_SET_INTR_MASK;
    programRowConfig.destAddr = (uint32_t*)writeAddr;
    programRowConfig.dataAddr = data;
    status = Cy_Flash_ProgramRow1(NULL, &programRowConfig, blocking);
    CY_ASSERT(status == CY_FLASH_DRV_SUCCESS);
    (void) status; // avoid "unused" compiler warning if NDEBUG is set
}
#endif


#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
