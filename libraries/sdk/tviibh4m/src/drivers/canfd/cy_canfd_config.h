/***************************************************************************//**
* \file cy_canfd_config.h
* \version 1.0
*
* \brief
* Provides the configuration for the  CANFD driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_CANFD_CONFIG_H)
#define CY_CANFD_CONFIG_H

#if defined(__cplusplus)
extern "C" {
#endif

/******************************************************************************/
/*                      Message RAM Definition                                */
/******************************************************************************/
  
/** Offset of CAN FD Message RAM (MRAM). Total 32k MRAM per CAN FD instance is shared by all CAN FD channels  
 *  within an instance and allocation for each channel is done by user. Below shown is example allocation */

/** Defining MRAM size (in bytes) per channel for CAN0 */
#define CY_CANFD0_0_MSGRAM_SIZE         ((CANFD0_MRAM_SIZE*1024)/CANFD0_CAN_NR)
#define CY_CANFD0_1_MSGRAM_SIZE         ((CANFD0_MRAM_SIZE*1024)/CANFD0_CAN_NR)
#define CY_CANFD0_2_MSGRAM_SIZE         ((CANFD0_MRAM_SIZE*1024)/CANFD0_CAN_NR)
#define CY_CANFD0_3_MSGRAM_SIZE         ((CANFD0_MRAM_SIZE*1024)/CANFD0_CAN_NR)

/** Calculation of the MRAM start address for each channel of CAN0*/
#define CY_CANFD0_0_MSGRAM_OFFSET       0u                                                       
#define CY_CANFD0_1_MSGRAM_OFFSET       (CY_CANFD0_0_MSGRAM_OFFSET + CY_CANFD0_0_MSGRAM_SIZE)    
#define CY_CANFD0_2_MSGRAM_OFFSET       (CY_CANFD0_1_MSGRAM_OFFSET + CY_CANFD0_1_MSGRAM_SIZE)    
#define CY_CANFD0_3_MSGRAM_OFFSET       (CY_CANFD0_2_MSGRAM_OFFSET + CY_CANFD0_2_MSGRAM_SIZE)    

/** Defining MRAM size (in bytes) per channel for CAN1 */
#define CY_CANFD1_0_MSGRAM_SIZE         ((CANFD1_MRAM_SIZE*1024)/CANFD1_CAN_NR)
#define CY_CANFD1_1_MSGRAM_SIZE         ((CANFD1_MRAM_SIZE*1024)/CANFD1_CAN_NR)
#define CY_CANFD1_2_MSGRAM_SIZE         ((CANFD1_MRAM_SIZE*1024)/CANFD1_CAN_NR)
#define CY_CANFD1_3_MSGRAM_SIZE         ((CANFD1_MRAM_SIZE*1024)/CANFD1_CAN_NR)

/** Calculation of the MRAM start address for each channel of CAN1*/
#define CY_CANFD1_0_MSGRAM_OFFSET       0u                                                       
#define CY_CANFD1_1_MSGRAM_OFFSET       (CY_CANFD1_0_MSGRAM_OFFSET + CY_CANFD1_0_MSGRAM_SIZE)    
#define CY_CANFD1_2_MSGRAM_OFFSET       (CY_CANFD1_1_MSGRAM_OFFSET + CY_CANFD1_1_MSGRAM_SIZE)    
#define CY_CANFD1_3_MSGRAM_OFFSET       (CY_CANFD1_2_MSGRAM_OFFSET + CY_CANFD1_2_MSGRAM_SIZE)    

/******************************************************************************/
/*              CAN FD Channel Definition (Fixed for this device)             */
/******************************************************************************/
  
/** CANFD0 Channel 0 */
#define CY_CANFD0_0_TYPE                ((cy_pstc_canfd_type_t)CANFD0_CH0)  
/** CANFD0 Channel 1 */             
#define CY_CANFD0_1_TYPE                ((cy_pstc_canfd_type_t)CANFD0_CH1)  
/** CANFD0 Channel 2 */             
#define CY_CANFD0_2_TYPE                ((cy_pstc_canfd_type_t)CANFD0_CH2)  
/** CANFD0 Channel 3 */             
#define CY_CANFD0_3_TYPE                ((cy_pstc_canfd_type_t)CANFD0_CH3)
  
/** CANFD1 Channel 0 */             
#define CY_CANFD1_0_TYPE                ((cy_pstc_canfd_type_t)CANFD1_CH0)  
/** CANFD1 Channel 1 */             
#define CY_CANFD1_1_TYPE                ((cy_pstc_canfd_type_t)CANFD1_CH1)  
/** CANFD1 Channel 2 */             
#define CY_CANFD1_2_TYPE                ((cy_pstc_canfd_type_t)CANFD1_CH2) 
/** CANFD1 Channel 3*/              
#define CY_CANFD1_3_TYPE                ((cy_pstc_canfd_type_t)CANFD1_CH3) 
    
#if defined(__cplusplus)
}
#endif

#endif /* CY_CANFD_CONFIG_H */

/** \} group_canfd */

/* [] END OF FILE */
