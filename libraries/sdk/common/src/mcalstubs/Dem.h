/***************************************************************************//**
* \file Dem.h
*
* \brief
* Dem stub implementation
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(DEM_H)
#define DEM_H


/*==================[inclusions]============================================*/


/*==================[macros]=========================*/

#define DEM_EVENT_STATUS_PASSED           ((Dem_EventStatusType)0)
#define DEM_EVENT_STATUS_FAILED           ((Dem_EventStatusType)1)
#define DEM_EVENT_STATUS_PREPASSED        ((Dem_EventStatusType)2)
#define DEM_EVENT_STATUS_PREFAILED        ((Dem_EventStatusType)3)
#define DEM_EVENT_STATUS_UNDEFINED        ((Dem_EventStatusType)0x55)


// Some symbolic defines using the default DemEventParameter names from Tresos stub plugin
#define DemConf_DemEventParameter_DemEventParameter_0               ((Dem_EventIdType)1)
#define DemConf_DemEventParameter_DemEventParameter_1               ((Dem_EventIdType)2)
#define DemConf_DemEventParameter_DemEventParameter_2               ((Dem_EventIdType)3)
#define DemConf_DemEventParameter_DemEventParameter_3               ((Dem_EventIdType)4)
#define DemConf_DemEventParameter_DemEventParameter_4               ((Dem_EventIdType)5)
#define DemConf_DemEventParameter_DemEventParameter_5               ((Dem_EventIdType)6)
#define DemConf_DemEventParameter_DemEventParameter_6               ((Dem_EventIdType)7)
#define DemConf_DemEventParameter_DemEventParameter_7               ((Dem_EventIdType)8)
#define DemConf_DemEventParameter_DemEventParameter_8               ((Dem_EventIdType)9)
#define DemConf_DemEventParameter_DemEventParameter_9               ((Dem_EventIdType)10)

// For easier recognition and better distinction following module specific names can also be used (make sure to rename the created 'DemEventParameter' names in Tresos stub plugin as well)
#define DemConf_DemEventParameter_ADC_E_HARDWARE_ERROR              ((Dem_EventIdType)100)

#define DemConf_DemEventParameter_CAN_E_HARDWARE_ERROR              ((Dem_EventIdType)110)

#define DemConf_DemEventParameter_CORTST_E_CORE_FAILURE             ((Dem_EventIdType)120)

#define DemConf_DemEventParameter_FLS_E_WRITE_FAILED                ((Dem_EventIdType)130)
#define DemConf_DemEventParameter_FLS_E_ERASE_FAILED                ((Dem_EventIdType)131)
#define DemConf_DemEventParameter_FLS_E_READ_FAILED                 ((Dem_EventIdType)132)
#define DemConf_DemEventParameter_FLS_E_COMPARE_FAILED              ((Dem_EventIdType)133)

#define DemConf_DemEventParameter_FLSTST_E_FLSTST_FAILURE           ((Dem_EventIdType)140)

#define DemConf_DemEventParameter_ICU_E_HARDWARE_ERROR              ((Dem_EventIdType)150)

#define DemConf_DemEventParameter_LIN_E_TIMEOUT                     ((Dem_EventIdType)160)

#define DemConf_DemEventParameter_MCU_E_CLOCK_FAILURE               ((Dem_EventIdType)170)
#define DemConf_DemEventParameter_MCU_E_RESET_FAILURE               ((Dem_EventIdType)171)

#define DemConf_DemEventParameter_RAMTST_E_RAM_FAILURE              ((Dem_EventIdType)180)
#define DemConf_DemEventParameter_RAMTST_MAIN_RAM_FAILURE           ((Dem_EventIdType)181)
#define DemConf_DemEventParameter_RAMTST_RUNFL_RAM_FAILURE          ((Dem_EventIdType)182)
#define DemConf_DemEventParameter_RAMTST_PART_RAM_FAILURE           ((Dem_EventIdType)183)
#define DemConf_DemEventParameter_RAMTST_E_DATA_BACKUP_FAILURE      ((Dem_EventIdType)184)
#define DemConf_DemEventParameter_RAMTST_E_STACK_BACKUP_FAILURE     ((Dem_EventIdType)185)
#define DemConf_DemEventParameter_RAMTST_E_STATIC_AREA_FAILURE      ((Dem_EventIdType)186)
#define DemConf_DemEventParameter_RAMTST_E_NMI_OCCURED              ((Dem_EventIdType)187)
#define DemConf_DemEventParameter_RAMTST_E_SYSTEM_FAILURE           ((Dem_EventIdType)188)
#define DemConf_DemEventParameter_RAMTST_E_SP_FAILURE               ((Dem_EventIdType)189)

#define DemConf_DemEventParameter_SPI_E_HARDWARE_ERROR              ((Dem_EventIdType)190)

#define DemConf_DemEventParameter_WDG_66_IA_E_DISABLE_REJECTED      ((Dem_EventIdType)200)
#define DemConf_DemEventParameter_WDG_66_IA_E_MODE_FAILED           ((Dem_EventIdType)201)
#define DemConf_DemEventParameter_WDG_66_IA_E_HW_TIMEOUT            ((Dem_EventIdType)202)
#define DemConf_DemEventParameter_WDG_66_IA_E_WDG_STOPPED           ((Dem_EventIdType)203)

#define DEM_EVENT_PARAMETER_UNDEFINED                               ((Dem_EventIdType)0xAAAA)
 
/*==================[type definitions]======================================*/

typedef uint16 Dem_EventIdType;
typedef uint8  Dem_EventStatusType;

/*==================[global variable declarations]===========================*/

extern uint32 Dem_Counter;
extern Dem_EventIdType Dem_EventId;
extern Dem_EventStatusType Dem_EventStatus;

/*==================[external function declarations]=========================*/

extern FUNC(Std_ReturnType, DEM_CODE) Dem_ReportErrorStatus(Dem_EventIdType EventId, Dem_EventStatusType EventStatusType);

 
#endif /* DEM_H */
/*==================[end of file]===========================================*/
