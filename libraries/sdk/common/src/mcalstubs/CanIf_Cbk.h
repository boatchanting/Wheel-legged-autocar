/***************************************************************************//**
* \file CanIf_Cbk.h
*
* \brief
* CanIf_Cbk stub implementation
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CANIF_CBK_H)
#define CANIF_CBK_H


#ifdef CY_MCAL_MODULE_ENABLED_CAN // Can_Cfg.h (which is included below) is only generated if Can module is part of current Tresos project

/*==================[inclusions]=============================================*/

#include "Can_Cfg.h"

/*==================[macros]=================================================*/

#ifdef CAN_RX_INDICATION_COMPATIBILITY
  /* CAN-FD driver is used */
  #if defined(CAN_ASR_421_COMPATIBILITY) && (CAN_RX_INDICATION_COMPATIBILITY == CAN_ASR_421_COMPATIBILITY)
    #define RX_INDICATION_ASR421
  #elif defined(CAN_ASR_422_COMPATIBILITY) && (CAN_RX_INDICATION_COMPATIBILITY == CAN_ASR_422_COMPATIBILITY)
    #define RX_INDICATION_ASR421
  #elif defined(CAN_ASR_403_COMPATIBILITY) && (CAN_RX_INDICATION_COMPATIBILITY == CAN_ASR_403_COMPATIBILITY)
    #define RX_INDICATION_ASR403
  #else
    #error "CAN_RX_INDICATION_COMPATIBILITY is not defined properly."
  #endif
#else
  /* Old CAN driver is used */
  #define RX_INDICATION_ASR403
#endif

 #define CANIF_AR_RELEASE_MAJOR_VERSION 4U
 #define CANIF_AR_RELEASE_MINOR_VERSION 0U
 
 #define CANIF_MAX_RXPDUINFO 4U

/*==================[type definitions]=======================================*/

/** \brief Operating modes of the CAN Controller and CAN Driver
 **
 ** Possible modes for a controller in the CAN driver.
 */
typedef enum canif_controllermodetype
{
  CANIF_CS_UNINIT,            /**< UNINIT mode */
  CANIF_CS_SLEEP,             /**< SLEEP mode */
  CANIF_CS_STARTED,           /**< STARTED mode */
  CANIF_CS_STOPPED            /**< STOPPED mode */
} CanIf_ControllerModeType;

/*==================[global variable declarations]===========================*/

extern uint32 CanIf_NoOfTxConfirmation;
extern uint32 CanIf_NoOfRxIndication;
extern uint32 CanIf_NoOfControllerBusOff;
extern uint32 CanIf_NoOfControllerModeIndication;
extern uint32 CanIf_NoOfCancelTxConfirmation;

extern Can_PduType CanIf_RxPduInfo[CANIF_MAX_RXPDUINFO];

/*==================[external function declarations]=========================*/

#define CANIF_START_SEC_CODE
#include <CanIf_MemMap.h>

/* Callback functions for CAN driver "" */
extern FUNC( void, CANIF_CODE ) CanIf_TxConfirmation(
    PduIdType CanTxPduId );

#ifdef RX_INDICATION_ASR403
extern FUNC( void, CANIF_CODE ) CanIf_RxIndication(
    Can_HwHandleType Hrh,
    Can_IdType CanId,
    uint8 CanDlc,
    P2CONST( uint8, AUTOMATIC, AUTOMATIC ) CanSduPtr );
#else
#ifdef RX_INDICATION_ASR421
extern FUNC( void, CANIF_CODE ) CanIf_RxIndication(
    P2CONST( Can_HwType, AUTOMATIC, AUTOMATIC ) Mailbox,
    P2CONST( PduInfoType, AUTOMATIC, AUTOMATIC ) PduInfoPtr );
#else
    #error Unknown CanIf_RxIndication prototype/function selection.
#endif
#endif

extern FUNC( void, CANIF_CODE ) CanIf_ControllerBusOff(
    uint8 Controller );

extern FUNC( void, CANIF_CODE ) CanIf_ControllerModeIndication( 
    uint8 Controller, 
    CanIf_ControllerModeType ControllerMode );

extern FUNC( void, CANIF_CODE ) CanIf_CancelTxConfirmation(
    PduIdType CanTxPduId,
    const PduInfoType* PduInfoPtr );

extern FUNC(Std_ReturnType, AUTOMATIC) CanIf_TriggerTransmit(
  VAR(PduIdType, AUTOMATIC) TxPduId,
  P2VAR(PduInfoType, AUTOMATIC, AUTOMATIC) PduInfoPtr );
  
extern FUNC(void, CANIF_CODE) CanIf_CurrentIcomConfiguration(
  uint8 ControllerId,
  IcomConfigIdType ConfigurationId,
  IcomSwitch_ErrorType Error);

#define CANIF_STOP_SEC_CODE
#include <CanIf_MemMap.h>

/*==================[internal function declarations]=========================*/

/*==================[external constants]=====================================*/

/*==================[internal constants]=====================================*/

/*==================[external data]==========================================*/

/*==================[internal data]==========================================*/

/*==================[external function definitions]==========================*/

/*==================[internal function definitions]==========================*/


#endif // CY_MCAL_MODULE_ENABLED_CAN

#endif /* CANIF_CBK_H */
/*==================[end of file]============================================*/
