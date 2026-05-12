/***************************************************************************//**
* \file cy_power.h
* \version 1.0
*
* \brief
* Provides an API declaration of the power supply middleware driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_mw_power Middleware Power Control (POWER)
* \{
* Offers control and switch from internal to external regulator and vice versa.
*
* \defgroup group_mw_power_functions Functions
* \defgroup group_mw_power_data_structures Data structures
* \defgroup group_mw_power_enums Enumerated types
*/

#if !defined(CY_POWER_H)
#define CY_POWER_H

#include <stddef.h>
#include <stdbool.h>
#include "cy_device_headers.h"


#if   defined(SRSS_S40E_HTREGHC_PRESENT) && (SRSS_S40E_HTREGHC_PRESENT == 1)
  #include "sysreghc/cy_sysreghc.h"
  #define CY_POWER_USES_REGHC_IP
#elif defined(SRSS_S40E_PMIC_PRESENT)    && (SRSS_S40E_PMIC_PRESENT    == 1)
  #include "syspmic/cy_syspmic.h"
  #define CY_POWER_USES_PMIC_IP
#else
  #error "SRSS external regulator IP unknown or not yet supported!"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/**
* \addtogroup group_mw_power_data_structures
* \{
*/

/** Map type of underlying logic (REGHC/PMIC) to a generic power type */
#if   defined(CY_POWER_USES_REGHC_IP)
  typedef cy_en_sysreghc_vadj_t  cy_en_power_vadj_t;
#elif defined(CY_POWER_USES_PMIC_IP)
  typedef cy_en_syspmic_vadj_t   cy_en_power_vadj_t;
#endif

/** \} group_mw_power_data_structures */

/**
* \addtogroup group_mw_power_enums
* \{
*/

/** Polarity of the PMICs "power good" / "abnormal" status output */
typedef enum
{
    CY_POWER_PMIC_STATUS_ABNORMAL_LOW  = 0,
    CY_POWER_PMIC_STATUS_ABNORMAL_HIGH = 1,
} cy_en_power_pmic_status_polarity_t;


/** Polarity of the signal to enable the PMIC */
typedef enum
{
    CY_POWER_PMIC_ENABLE_LOW  = 0,
    CY_POWER_PMIC_ENABLE_HIGH = 1,
} cy_en_power_pmic_enable_polarity_t;

/** \} group_mw_power_enums */


/**
* \addtogroup group_mw_power_functions
* \{
*/

void Cy_Power_SwitchToPmic      ( cy_en_power_vadj_t                 voltageAdjust, 
                                  cy_en_power_pmic_enable_polarity_t enablePolarity, 
                                  cy_en_power_pmic_status_polarity_t statusPolarity,
                                  uint16_t                           waitTimeUs);
                           
void Cy_Power_SwitchToTransistor( cy_en_power_vadj_t                 voltageAdjust);
                                  
void Cy_Power_SwitchToInternal();


/** \} group_mw_power_functions */

#if defined(__cplusplus)
}
#endif

#endif /* CY_POWER_H */

/** \} group_mw_power */

/* [] END OF FILE */
