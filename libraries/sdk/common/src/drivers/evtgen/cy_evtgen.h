/***************************************************************************//**
* \file cy_evtgen.h
* \version 1.0
*
* Provides the API declarations of the Event generator driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_EVTGEN_H)
#define CY_EVTGEN_H

/**
* \defgroup group_evtgen Event Generator (EVTGEN)
* \{
* This driver provides global EVTGEN defines and API function definitions.
*
* \defgroup group_evtgen_macros Macro
* \defgroup group_evtgen_functions Functions
* \defgroup group_evtgen_data_structures Data structures
* \defgroup group_evtgen_enums Enumerated types
*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cy_device_headers.h"

/***************************************
* Macro definitions
***************************************/
/**
* \addtogroup group_evtgen_macros
* \{
*/

/** Driver major version */
#define CY_EVTGEN_DRV_VERSION_MAJOR       1

/** Driver minor version */
#define CY_EVTGEN_DRV_VERSION_MINOR       0

/** \} group_evtgen_macros */

/**
* \addtogroup group_evtgen_enums
* \{
*/

/**
* Specifies return values meaning
*/

/** This enum has the return values of the Event generator driver */
typedef enum 
{
    CY_EVTGEN_OK    = 0u,
    CY_EVTGEN_ERR,
} cy_en_evtgendrv_status_t;

typedef enum 
{
    CY_EVTGEN_DISABLE    = 0u,
    CY_EVTGEN_ENABLE     = 1u,
} cy_en_evtgendrv_enable_t;

typedef enum 
{
    CY_EVTGEN_RATIO_INVALID    = 0u,
    CY_EVTGEN_RATIO_VALID      = 1u,
} cy_en_evtgendrv_ratio_valid_t;

typedef enum 
{
    CY_EVTGEN_RATIO_STATIC    = 0u,
    CY_EVTGEN_RATIO_DYNAMIC   = 1u,
} cy_en_evtgendrv_ratio_dynamic_t;

typedef enum 
{
    CY_EVTGEN_COUNTER_STATUS_INVALID = 0u,
    CY_EVTGEN_COUNTER_STATUS_VALID   = 1u,
} cy_en_evtgen_counter_status_t;

typedef enum 
{
    CY_EVTGEN_RATIO_DYNAMIC_MODE0    = 0u, // new RATIO value = (RATIO + measurement + 1) / 2.
    CY_EVTGEN_RATIO_DYNAMIC_MODE1    = 1u, // new RATIO value = (3*RATIO + measurement + 2) / 4.
    CY_EVTGEN_RATIO_DYNAMIC_MODE2    = 2u, // new RATIO value = (7*RATIO + measurement + 4) / 8.
    CY_EVTGEN_RATIO_DYNAMIC_MODE3    = 3u, // new RATIO value = (15*RATIO + measurement + 8) / 16.
    CY_EVTGEN_RATIO_DYNAMIC_MODE4    = 4u, // new RATIO value = (31*RATIO + measurement + 16) / 32.
    CY_EVTGEN_RATIO_DYNAMIC_MODE5    = 5u, // new RATIO value = (63*RATIO + measurement + 32) / 64.
    CY_EVTGEN_RATIO_DYNAMIC_MODE6    = 6u, // new RATIO value = (127*RATIO + measurement + 64) / 128.
    CY_EVTGEN_RATIO_DYNAMIC_MODE7    = 7u, // new RATIO value = (255*RATIO + measurement + 128) / 256.
} cy_en_evtgendrv_ratio_dynamic_mode_t;

typedef enum 
{
    CY_EVTGEN_RATIO_CONTROL_SW    = 0u,
    CY_EVTGEN_RATIO_CONTROL_HW,
} cy_en_evtgendrv_ratio_control;

typedef enum 
{
    CY_EVTGEN_STRUCT_DISABLE    = 0u,
    CY_EVTGEN_STRUCT_ENABLE,
} cy_en_evtgendrv_struct_enable_t;

typedef enum 
{
    CY_EVTGEN_ACTIVE_COMP_DISABLE    = 0u,
    CY_EVTGEN_ACTIVE_COMP_ENABLE,
} cy_en_evtgendrv_enable_active_cmp_t;

typedef enum 
{
    CY_EVTGEN_DSLEEP_COMP_DISABLE    = 0u,
    CY_EVTGEN_DSLEEP_COMP_ENABLE,
} cy_en_evtgendrv_enable_dsleep_cmp_t;

typedef enum 
{
    CY_EVTGEN_LEVEL_SENSITIVE    = 0u,
    CY_EVTGEN_EDGE_SENSITIVE,
} cy_en_evtgendrv_struct_trg_edge_t;

typedef enum 
{
    CY_EVTGEN_ACTIVE_FUNCTIONALITY    = 0u,
    CY_EVTGEN_DEEPSLEEP_FUNCTIONALITY,
} cy_en_evtgendrv_struct_functionality;

/** \} group_evtgen_enums */


#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
* Data Structure definitions
***************************************/

/**
* \addtogroup group_evtgen_data_structures
* \{
*/

/** Event generator driver context */
typedef struct
{
    uint32_t frequencyRef;
    uint32_t frequencyLf;
    uint32_t frequencyTick;
    cy_en_evtgendrv_ratio_control ratioControlMode;
    cy_en_evtgendrv_ratio_dynamic_mode_t ratioValueDynamicMode; /**< Specifies the configuration of evtgen operation */
}cy_stc_evtgen_config_t;

typedef void (* cy_evtgen_handler)(void);   /**< Type of ISR callbacks */

typedef struct
{
    cy_en_evtgendrv_struct_functionality functionalitySelection;
    cy_en_evtgendrv_struct_trg_edge_t triggerOutEdge;              /**< Specifies the code of evtgen operation */
    uint32_t valueActiveComparator;                                /**< Specifies the configuration of evtgen operation */
    uint32_t valueDeepSleepComparator;                             /**< Specifies the configuration of evtgen operation */
    cy_evtgen_handler userIsr;
}cy_stc_evtgen_struct_config_t;

typedef struct
{
    uint32_t addValueForCOMP0;                                /**< Specifies the configuration of evtgen operation */
    cy_evtgen_handler userIsr;
}cy_stc_evtgen_struct_context_t;

/** \} group_evtgen_data_structures */


/***************************************
* Function Prototypes
***************************************/

/**
* \addtogroup group_evtgen_functions
* \{
*/

/*******************************************************************************
* Function Name: Cy_Evtgen_Enable
****************************************************************************//**
*
* \brief Enable event generator
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_Evtgen_Enable(volatile stc_EVTGEN_t *base)
{
    base->unCTL.stcField.u1ENABLED = 1u; /* Set ENABLED bit */
}

/*******************************************************************************
* Function Name: Cy_Evtgen_SetInterrupt
****************************************************************************//**
*
* \brief Set interrupt factor of corresponding structure
*
* \param [in] structNumber
* Number of structure. The interrupt factor of that is to be set
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_Evtgen_SetInterrupt(volatile stc_EVTGEN_t *base, uint8_t structNumber)
{
    base->unINTR_SET.u32Register = ((uint32_t)1 << structNumber);
}

/*******************************************************************************
* Function Name: Cy_Evtgen_ClearInterrupt
****************************************************************************//**
*
* \brief Clear the interrupt factor
*
* \param [in] mask
* Each bit of this parameter corresponds to each structure factor to be clear
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_Evtgen_ClearInterrupt(volatile stc_EVTGEN_t *base, uint32_t mask)
{
    base->unINTR.u32Register = mask;

    // Dummy read. This is to wait for reflection above write operation.
    base->unINTR.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Evtgen_ClearStructInterrupt
****************************************************************************//**
*
* \brief Clear interrupt factor of corresponding structure
*
* \param [in] structNumber
* Number of structure. The factor of that is to be clear
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_Evtgen_ClearStructInterrupt(volatile stc_EVTGEN_t *base, uint8_t structNumber)
{
    base->unINTR.u32Register = ((uint32_t)1 << structNumber);

    // Dummy read. This is to wait for reflection above write operation.
    base->unINTR.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Evtgen_SetInterruptMask
****************************************************************************//**
*
* \brief Set interrupt mask bit of a corresponding structure
*
* \param [in] structNumber
* Number of structure. The mask bit of that is to be set
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_Evtgen_SetInterruptMask(volatile stc_EVTGEN_t *base, uint8_t structNumber)
{
    base->unINTR_MASK.u32Register |= ((uint32_t)1 << structNumber);
}


/*******************************************************************************
* Function Name: Cy_Evtgen_GetInterrupt
****************************************************************************//**
*
* \brief Get interrupt bit of corresponding structure
*
* \param [in] structNumber
* Number of structure. The interrupt factor of the structure is to be returned
*
* \return
* The interrupt bit
*
*******************************************************************************/
__STATIC_INLINE bool Cy_Evtgen_GetInterrupt(volatile stc_EVTGEN_t *base, uint8_t structNumber)
{
    return ((base->unINTR.u32Register & ((uint32_t)1 << structNumber)) == 0u) ? false : true;
}

/*******************************************************************************
* Function Name: Cy_Evtgen_GetInterruptMasked
****************************************************************************//**
*
* \brief Get interrupt masked bit of corresponding structure
*
* \param [in] structNumber
* Number of structure. The masked bit of the structure is to be returned
*
* \return
* The interrupt masked bit
*
*******************************************************************************/
__STATIC_INLINE bool Cy_Evtgen_GetInterruptMasked(volatile stc_EVTGEN_t *base, uint8_t structNumber)
{
    return ((base->unINTR_MASKED.u32Register & ((uint32_t)1 << structNumber)) == 0u) ? false : true;
}


/*******************************************************************************
* Function Name: Cy_Evtgen_SetInterruptDeepSleep
****************************************************************************//**
*
* \brief Set deep sleep interrupt factor of corresponding structure
*
* \param [in] structNumber
* Number of structure. The interrupt factor of that is to be set
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_Evtgen_SetInterruptDeepSleep(volatile stc_EVTGEN_t *base, uint8_t structNumber)
{
    base->unINTR_DPSLP_SET.u32Register |= ((uint32_t)1 << structNumber);
}

/*******************************************************************************
* Function Name: Cy_Evtgen_ClearInterruptDeepSleep
****************************************************************************//**
*
* \brief Clear the deep sleep interrupt factor
*
* \param [in] mask
* Each bit of this parameter corresponds to each structure factor to be clear
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_Evtgen_ClearInterruptDeepSleep(volatile stc_EVTGEN_t *base, uint32_t mask)
{
    base->unINTR_DPSLP.u32Register = mask;

    // Dummy read. This is to wait for reflection above write operation.
    base->unINTR_DPSLP.u32Register;
}


/*******************************************************************************
* Function Name: Cy_Evtgen_ClearStructInterruptDeepSleep
****************************************************************************//**
*
* \brief Clear deep sleep interrupt factor of corresponding structure
*
* \param [in] structNumber
* Number of structure. The factor of that is to be clear
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_Evtgen_ClearStructInterruptDeepSleep(volatile stc_EVTGEN_t *base, uint8_t structNumber)
{
    base->unINTR_DPSLP.u32Register = ((uint32_t)1 << structNumber);

    // Dummy read. This is to wait for reflection above write operation.
    base->unINTR_DPSLP.u32Register;
}


/*******************************************************************************
* Function Name: Cy_Evtgen_SetInterruptDeepSleepMask
****************************************************************************//**
*
* \brief Set deep sleep interrupt mask bit of a corresponding structure
*
* \param [in] structNumber
* Number of structure. The deep sleep mask bit of that is to be set
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_Evtgen_SetInterruptDeepSleepMask(volatile stc_EVTGEN_t *base, uint8_t structNumber)
{
    base->unINTR_DPSLP_MASK.u32Register |= ((uint32_t)1 << structNumber);
}

/*******************************************************************************
* Function Name: Cy_Evtgen_GetInterruptDeepSleep
****************************************************************************//**
*
* \brief Get deep sleep interrupt bit of corresponding structure
*
* \param [in] structNumber
* Number of structure. The deep sleep interrupt factor of the structure is to be returned
*
* \return
* The interrupt bit
*
*******************************************************************************/
__STATIC_INLINE bool Cy_Evtgen_GetInterruptDeepSleep(volatile stc_EVTGEN_t *base, uint8_t structNumber)
{
    return ((base->unINTR_DPSLP.u32Register & ((uint32_t)1 << structNumber)) == 0) ? false : true;
}

/*******************************************************************************
* Function Name: Cy_Evtgen_GetInterruptDeepSleepMasked
****************************************************************************//**
*
* \brief Get deep sleep interrupt masked bit of corresponding structure
*
* \param [in] structNumber
* Number of structure. The deep sleep masked bit of the structure is to be returned
*
* \return
* The interrupt masked bit
*
*******************************************************************************/
__STATIC_INLINE bool Cy_Evtgen_GetInterruptDeepSleepMasked(volatile stc_EVTGEN_t *base, uint8_t structNumber)
{
    return ((base->unINTR_DPSLP_MASKED.u32Register & ((uint32_t)1 << structNumber)) == 0) ? false : true;
}


/*******************************************************************************
* Function Name: Cy_Evtgen_GetComp0Status
****************************************************************************//**
*
* \brief Get comparator 0 status
*
* \return
* Comparator 0 Status
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_Evtgen_GetComp0Status(volatile stc_EVTGEN_t *base)
{
    return base->unCOMP0_STATUS.u32Register;
}


/*******************************************************************************
* Function Name: Cy_Evtgen_GetComp1Status
****************************************************************************//**
*
* \brief Get comparator 1 status
*
* \return
* Comparator 1 Status
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_Evtgen_GetComp1Status(volatile stc_EVTGEN_t *base)
{
    return base->unCOMP1_STATUS.u32Register;
}


/*******************************************************************************
* Function Name: Cy_Evtgen_GetCounterStatus
****************************************************************************//**
*
* \brief Get counter status
*
* \return
* counter status 
*
* \Note
* If this status is "1", the counter completed initialization 
* and is ready to be read
*
*******************************************************************************/
__STATIC_INLINE cy_en_evtgen_counter_status_t Cy_Evtgen_GetCounterStatus(volatile stc_EVTGEN_t *base)
{
     return (cy_en_evtgen_counter_status_t)base->unCOUNTER_STATUS.stcField.u1VALID;
}


/*******************************************************************************
* Function Name: Cy_Evtgen_GetCounterValue
****************************************************************************//**
*
* \brief Get counter value
*
* \return
* counter value
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_Evtgen_GetCounterValue(volatile stc_EVTGEN_t *base)
{
    return base->unCOUNTER.u32Register;
}


void Cy_Evtgen_Enable(volatile stc_EVTGEN_t *base);
cy_en_evtgendrv_status_t Cy_Evtgen_Initialize(volatile stc_EVTGEN_t *base, const cy_stc_evtgen_config_t* config);
void Cy_Evtgen_Deinitialize(volatile stc_EVTGEN_t *base);
uint32_t Cy_Evtgen_GetComp0Status(volatile stc_EVTGEN_t *base);
uint32_t Cy_Evtgen_GetComp1Status(volatile stc_EVTGEN_t *base);
cy_en_evtgen_counter_status_t Cy_Evtgen_GetCounterStatus(volatile stc_EVTGEN_t *base);
uint32_t Cy_Evtgen_GetCounterValue(volatile stc_EVTGEN_t *base);
cy_en_evtgendrv_status_t Cy_Evtgen_InitializeCompStruct(volatile stc_EVTGEN_t *base, 
                                                        uint8_t structNum, 
                                                        const cy_stc_evtgen_struct_config_t* configStruct, 
                                                        cy_stc_evtgen_struct_context_t* context);
void Cy_Evtgen_DeinitializeCompStruct(volatile stc_EVTGEN_t *base, uint8_t structNum);

void Cy_Evtgen_Interrupt(volatile stc_EVTGEN_t *base);

/** \cond INTERNAL */

/** \endcond */

/** \} group_evtgen_functions */

#if defined(__cplusplus)
}
#endif


#endif /* #if !defined(CY_EVTGEN_H) */
/** \endcond */


/** \} group_evtgen */


/* [] END OF FILE */
