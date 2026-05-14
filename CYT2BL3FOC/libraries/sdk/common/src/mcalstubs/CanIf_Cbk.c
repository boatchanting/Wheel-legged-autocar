/***************************************************************************//**
* \file CanIf_Cbk.c
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


#ifdef CY_MCAL_MODULE_ENABLED_CAN

/*==================[inclusions]=============================================*/

#include <Can.h>                /* AUTOSAR CAN definitions */
#include <CanIf_Cbk.h>
#include <stdio.h>


/*==================[global variable definitions]===========================*/

#define CANIF_MAX_DATA_PAYLOAD 64u

uint32 CanIf_NoOfTxConfirmation = 0u;
uint32 CanIf_NoOfRxIndication = 0u;
uint32 CanIf_NoOfControllerBusOff = 0u;
uint32 CanIf_NoOfControllerModeIndication = 0u;
uint32 CanIf_NoOfCancelTxConfirmation = 0u;

Can_PduType CanIf_RxPduInfo[CANIF_MAX_RXPDUINFO] = {0};
uint8       CanIf_CanDataBuffer[CANIF_MAX_RXPDUINFO * CANIF_MAX_DATA_PAYLOAD];

/*==================[external function definitions]=========================*/

#define CANIF_START_SEC_CODE
#include <CanIf_MemMap.h>

/* Callback functions for CAN driver "" */
FUNC( void, CANIF_CODE ) CanIf_TxConfirmation(
    PduIdType CanTxPduId )
{
  #ifdef CY_MCAL_STUBS_CANIF_CBK_TX_ENABLE_PRINTF
  printf("\nCanIf_TxConfirmation(CanTxPduId=%u)\n", CanTxPduId);
  #endif
  CanIf_NoOfTxConfirmation++;
}


#ifdef RX_INDICATION_ASR403
FUNC( void, CANIF_CODE ) CanIf_RxIndication(
    Can_HwHandleType Hrh,
    Can_IdType CanId,
    uint8 CanDlc,
    P2CONST( uint8, AUTOMATIC, AUTOMATIC ) CanSduPtr )
{
  int i;  
  
  #ifdef CY_MCAL_STUBS_CANIF_CBK_RX_ENABLE_PRINTF
  printf("\nCanIf_RxIndication(Hrh=%u, CanId=0x%lx, CanDlc=%lu, CanSduPtr=0x%lx)\n->Data: ", Hrh, CanId, CanDlc, CanSduPtr);
  #endif
  
  CanIf_NoOfRxIndication++;
  
  for (i=0; i<CanDlc; i++)
  {
    if ((CanIf_NoOfRxIndication * CANIF_MAX_DATA_PAYLOAD) + i < sizeof(CanIf_CanDataBuffer))
    {
      CanIf_CanDataBuffer[(CanIf_NoOfRxIndication * CANIF_MAX_DATA_PAYLOAD) + i] = CanSduPtr[i];
    }
    #ifdef CY_MCAL_STUBS_CANIF_CBK_RX_ENABLE_PRINTF
    printf("%02x ", CanSduPtr[i]);
    #endif
  }
  #ifdef CY_MCAL_STUBS_CANIF_CBK_RX_ENABLE_PRINTF
  printf("\n");  
  #endif
  
  if (CANIF_MAX_RXPDUINFO > CanIf_NoOfRxIndication )
  {
    CanIf_RxPduInfo[CanIf_NoOfRxIndication].length = CanDlc;
    CanIf_RxPduInfo[CanIf_NoOfRxIndication].id = CanId;
    CanIf_RxPduInfo[CanIf_NoOfRxIndication].sdu = &CanIf_CanDataBuffer[CanIf_NoOfRxIndication * CANIF_MAX_DATA_PAYLOAD];
  }
}
#else
#ifdef RX_INDICATION_ASR421
FUNC( void, CANIF_CODE ) CanIf_RxIndication(
    P2CONST( Can_HwType, AUTOMATIC, AUTOMATIC ) Mailbox,
    P2CONST( PduInfoType, AUTOMATIC, AUTOMATIC ) PduInfoPtr )
{
  int i;  
  #ifdef CY_MCAL_STUBS_CANIF_CBK_RX_ENABLE_PRINTF
  printf("\nCanIf_RxIndication(Hoh=%u, Ctrl=%u, CanId=0x%lx, Len=%lu, CanSduPtr=0x%lx)\n->Data: ", Mailbox->Hoh, Mailbox->ControllerId, Mailbox->CanId, PduInfoPtr->SduLength, (uint32)PduInfoPtr->SduDataPtr);
  #endif
  
  CanIf_NoOfRxIndication++;
  
  for (i=0; i<PduInfoPtr->SduLength; i++)
  {
    if ((CanIf_NoOfRxIndication * CANIF_MAX_DATA_PAYLOAD) + i < sizeof(CanIf_CanDataBuffer))
    {
      CanIf_CanDataBuffer[(CanIf_NoOfRxIndication * CANIF_MAX_DATA_PAYLOAD) + i] = PduInfoPtr->SduDataPtr[i];
    }
    #ifdef CY_MCAL_STUBS_CANIF_CBK_RX_ENABLE_PRINTF
    printf("%02x ", PduInfoPtr->SduDataPtr[i]);
    #endif
  }
  #ifdef CY_MCAL_STUBS_CANIF_CBK_RX_ENABLE_PRINTF
  printf("\n");  
  #endif
  
  if (CANIF_MAX_RXPDUINFO > CanIf_NoOfRxIndication )
  {
    CanIf_RxPduInfo[CanIf_NoOfRxIndication].length = PduInfoPtr->SduLength;
    CanIf_RxPduInfo[CanIf_NoOfRxIndication].id = Mailbox->CanId;
    CanIf_RxPduInfo[CanIf_NoOfRxIndication].sdu = &CanIf_CanDataBuffer[CanIf_NoOfRxIndication * CANIF_MAX_DATA_PAYLOAD];
  }
}
#else
    #error Unknown CanIf_RxIndication prototype/function selection.
#endif
#endif


FUNC( void, CANIF_CODE ) CanIf_ControllerBusOff(
    uint8 Controller )
{
  #ifdef CY_MCAL_STUBS_CANIF_CBK_MODE_STATE_MISC_ENABLE_PRINTF
  printf("\nCanIf_ControllerBusOff(Controller=%u)\n", Controller);
  #endif
  CanIf_NoOfControllerBusOff++;
}


FUNC( void, CANIF_CODE ) CanIf_CancelTxConfirmation(
    PduIdType CanTxPduId,
    const PduInfoType* PduInfoPtr )
{
  int i;
  
  #ifdef CY_MCAL_STUBS_CANIF_CBK_TX_ENABLE_PRINTF
  printf("\nCanIf_CancelTxConfirmation(CanTxPduId=%u, PduInfoPtr=0x%x)\n->PduInfo=", CanTxPduId, PduInfoPtr);
  #endif
  for (i=0; i<PduInfoPtr->SduLength; i++)
  {
    #ifdef CY_MCAL_STUBS_CANIF_CBK_TX_ENABLE_PRINTF
    printf("%02x ", PduInfoPtr->SduDataPtr[i]);
    #endif
  }
  #ifdef CY_MCAL_STUBS_CANIF_CBK_TX_ENABLE_PRINTF
  printf("\n");
  #endif
  CanIf_NoOfCancelTxConfirmation++;
}


FUNC( void, CANIF_CODE ) CanIf_ControllerModeIndication( 
    uint8 Controller, 
    CanIf_ControllerModeType ControllerMode )
{
  #ifdef CY_MCAL_STUBS_CANIF_CBK_MODE_STATE_MISC_ENABLE_PRINTF
  static const char mode_uninit[] = "UNINIT";
  static const char mode_sleep[] = "SLEEP";
  static const char mode_started[] = "STARTED";
  static const char mode_stopped[] = "STOPPED";
  static const char mode_undef[] = "UNDEFINED";
  const char* mode_ptr;
  
  switch (ControllerMode)
  {
    case CANIF_CS_UNINIT:
      mode_ptr = mode_uninit;
      break;
    case CANIF_CS_SLEEP:
      mode_ptr = mode_sleep;
      break;
    case CANIF_CS_STARTED:
      mode_ptr = mode_started;
      break;
    case CANIF_CS_STOPPED:
      mode_ptr = mode_stopped;
      break;
    default:
      mode_ptr = mode_undef;
      break;
  }
  printf("\nCanIf_ControllerModeIndication(Ctrl=%d, CtrlMode=%s)\n", Controller, mode_ptr);
  #endif
  CanIf_NoOfControllerModeIndication++;
}


FUNC(Std_ReturnType, AUTOMATIC) CanIf_TriggerTransmit(
  VAR(PduIdType, AUTOMATIC) TxPduId,
  P2VAR(PduInfoType, AUTOMATIC, AUTOMATIC) PduInfoPtr )
{
  #ifdef CY_MCAL_STUBS_CANIF_CBK_TX_ENABLE_PRINTF
  printf("\nCanIf_TriggerTransmit() was called, although the call was unexpected!\n");
  #endif
  return E_NOT_OK;
}


FUNC(void, CANIF_CODE) CanIf_CurrentIcomConfiguration(
  uint8 ControllerId,
  IcomConfigIdType ConfigurationId,
  IcomSwitch_ErrorType Error)
{
  #ifdef CY_MCAL_STUBS_CANIF_CBK_MODE_STATE_MISC_ENABLE_PRINTF
  printf("\nCanIf_CurrentIcomConfiguration(CtrlId=%u, ConfId=%u, Error=%u)\n", ControllerId, ConfigurationId, Error);
  #endif
}


#define CANIF_STOP_SEC_CODE
#include <CanIf_MemMap.h>

/*==================[internal function definitions]==========================*/

#else
    
typedef int CanIf_Cbk_dummy_t; // avoid empty compilation unit by defining a dummy type

#endif // CY_MCAL_MODULE_ENABLED_CAN

/*==================[end of file]============================================*/
