/***************************************************************************//**
* \file cyreg_fault.h
*
* \brief
* FAULT register definition header
*
* \note
* Generator version: 1.6.0.481
* Database revision: TVIIBH4M_PR3_0
*
********************************************************************************
* \copyright
* Copyright 2016-2021, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef _CYREG_FAULT_H_
#define _CYREG_FAULT_H_

#include "cyip_fault.h"

/**
  * \brief Fault structure (FAULT_STRUCT0)
  */
#define CYREG_FAULT_STRUCT0_CTL         ((volatile un_FAULT_STRUCT_CTL_t*) 0x40210000UL)
#define CYREG_FAULT_STRUCT0_STATUS      ((volatile un_FAULT_STRUCT_STATUS_t*) 0x4021000CUL)
#define CYREG_FAULT_STRUCT0_DATA0       ((volatile un_FAULT_STRUCT_DATA_t*) 0x40210010UL)
#define CYREG_FAULT_STRUCT0_DATA1       ((volatile un_FAULT_STRUCT_DATA_t*) 0x40210014UL)
#define CYREG_FAULT_STRUCT0_DATA2       ((volatile un_FAULT_STRUCT_DATA_t*) 0x40210018UL)
#define CYREG_FAULT_STRUCT0_DATA3       ((volatile un_FAULT_STRUCT_DATA_t*) 0x4021001CUL)
#define CYREG_FAULT_STRUCT0_PENDING0    ((volatile un_FAULT_STRUCT_PENDING0_t*) 0x40210040UL)
#define CYREG_FAULT_STRUCT0_PENDING1    ((volatile un_FAULT_STRUCT_PENDING1_t*) 0x40210044UL)
#define CYREG_FAULT_STRUCT0_PENDING2    ((volatile un_FAULT_STRUCT_PENDING2_t*) 0x40210048UL)
#define CYREG_FAULT_STRUCT0_MASK0       ((volatile un_FAULT_STRUCT_MASK0_t*) 0x40210050UL)
#define CYREG_FAULT_STRUCT0_MASK1       ((volatile un_FAULT_STRUCT_MASK1_t*) 0x40210054UL)
#define CYREG_FAULT_STRUCT0_MASK2       ((volatile un_FAULT_STRUCT_MASK2_t*) 0x40210058UL)
#define CYREG_FAULT_STRUCT0_INTR        ((volatile un_FAULT_STRUCT_INTR_t*) 0x402100C0UL)
#define CYREG_FAULT_STRUCT0_INTR_SET    ((volatile un_FAULT_STRUCT_INTR_SET_t*) 0x402100C4UL)
#define CYREG_FAULT_STRUCT0_INTR_MASK   ((volatile un_FAULT_STRUCT_INTR_MASK_t*) 0x402100C8UL)
#define CYREG_FAULT_STRUCT0_INTR_MASKED ((volatile un_FAULT_STRUCT_INTR_MASKED_t*) 0x402100CCUL)

/**
  * \brief Fault structure (FAULT_STRUCT1)
  */
#define CYREG_FAULT_STRUCT1_CTL         ((volatile un_FAULT_STRUCT_CTL_t*) 0x40210100UL)
#define CYREG_FAULT_STRUCT1_STATUS      ((volatile un_FAULT_STRUCT_STATUS_t*) 0x4021010CUL)
#define CYREG_FAULT_STRUCT1_DATA0       ((volatile un_FAULT_STRUCT_DATA_t*) 0x40210110UL)
#define CYREG_FAULT_STRUCT1_DATA1       ((volatile un_FAULT_STRUCT_DATA_t*) 0x40210114UL)
#define CYREG_FAULT_STRUCT1_DATA2       ((volatile un_FAULT_STRUCT_DATA_t*) 0x40210118UL)
#define CYREG_FAULT_STRUCT1_DATA3       ((volatile un_FAULT_STRUCT_DATA_t*) 0x4021011CUL)
#define CYREG_FAULT_STRUCT1_PENDING0    ((volatile un_FAULT_STRUCT_PENDING0_t*) 0x40210140UL)
#define CYREG_FAULT_STRUCT1_PENDING1    ((volatile un_FAULT_STRUCT_PENDING1_t*) 0x40210144UL)
#define CYREG_FAULT_STRUCT1_PENDING2    ((volatile un_FAULT_STRUCT_PENDING2_t*) 0x40210148UL)
#define CYREG_FAULT_STRUCT1_MASK0       ((volatile un_FAULT_STRUCT_MASK0_t*) 0x40210150UL)
#define CYREG_FAULT_STRUCT1_MASK1       ((volatile un_FAULT_STRUCT_MASK1_t*) 0x40210154UL)
#define CYREG_FAULT_STRUCT1_MASK2       ((volatile un_FAULT_STRUCT_MASK2_t*) 0x40210158UL)
#define CYREG_FAULT_STRUCT1_INTR        ((volatile un_FAULT_STRUCT_INTR_t*) 0x402101C0UL)
#define CYREG_FAULT_STRUCT1_INTR_SET    ((volatile un_FAULT_STRUCT_INTR_SET_t*) 0x402101C4UL)
#define CYREG_FAULT_STRUCT1_INTR_MASK   ((volatile un_FAULT_STRUCT_INTR_MASK_t*) 0x402101C8UL)
#define CYREG_FAULT_STRUCT1_INTR_MASKED ((volatile un_FAULT_STRUCT_INTR_MASKED_t*) 0x402101CCUL)

/**
  * \brief Fault structure (FAULT_STRUCT2)
  */
#define CYREG_FAULT_STRUCT2_CTL         ((volatile un_FAULT_STRUCT_CTL_t*) 0x40210200UL)
#define CYREG_FAULT_STRUCT2_STATUS      ((volatile un_FAULT_STRUCT_STATUS_t*) 0x4021020CUL)
#define CYREG_FAULT_STRUCT2_DATA0       ((volatile un_FAULT_STRUCT_DATA_t*) 0x40210210UL)
#define CYREG_FAULT_STRUCT2_DATA1       ((volatile un_FAULT_STRUCT_DATA_t*) 0x40210214UL)
#define CYREG_FAULT_STRUCT2_DATA2       ((volatile un_FAULT_STRUCT_DATA_t*) 0x40210218UL)
#define CYREG_FAULT_STRUCT2_DATA3       ((volatile un_FAULT_STRUCT_DATA_t*) 0x4021021CUL)
#define CYREG_FAULT_STRUCT2_PENDING0    ((volatile un_FAULT_STRUCT_PENDING0_t*) 0x40210240UL)
#define CYREG_FAULT_STRUCT2_PENDING1    ((volatile un_FAULT_STRUCT_PENDING1_t*) 0x40210244UL)
#define CYREG_FAULT_STRUCT2_PENDING2    ((volatile un_FAULT_STRUCT_PENDING2_t*) 0x40210248UL)
#define CYREG_FAULT_STRUCT2_MASK0       ((volatile un_FAULT_STRUCT_MASK0_t*) 0x40210250UL)
#define CYREG_FAULT_STRUCT2_MASK1       ((volatile un_FAULT_STRUCT_MASK1_t*) 0x40210254UL)
#define CYREG_FAULT_STRUCT2_MASK2       ((volatile un_FAULT_STRUCT_MASK2_t*) 0x40210258UL)
#define CYREG_FAULT_STRUCT2_INTR        ((volatile un_FAULT_STRUCT_INTR_t*) 0x402102C0UL)
#define CYREG_FAULT_STRUCT2_INTR_SET    ((volatile un_FAULT_STRUCT_INTR_SET_t*) 0x402102C4UL)
#define CYREG_FAULT_STRUCT2_INTR_MASK   ((volatile un_FAULT_STRUCT_INTR_MASK_t*) 0x402102C8UL)
#define CYREG_FAULT_STRUCT2_INTR_MASKED ((volatile un_FAULT_STRUCT_INTR_MASKED_t*) 0x402102CCUL)

/**
  * \brief Fault structure (FAULT_STRUCT3)
  */
#define CYREG_FAULT_STRUCT3_CTL         ((volatile un_FAULT_STRUCT_CTL_t*) 0x40210300UL)
#define CYREG_FAULT_STRUCT3_STATUS      ((volatile un_FAULT_STRUCT_STATUS_t*) 0x4021030CUL)
#define CYREG_FAULT_STRUCT3_DATA0       ((volatile un_FAULT_STRUCT_DATA_t*) 0x40210310UL)
#define CYREG_FAULT_STRUCT3_DATA1       ((volatile un_FAULT_STRUCT_DATA_t*) 0x40210314UL)
#define CYREG_FAULT_STRUCT3_DATA2       ((volatile un_FAULT_STRUCT_DATA_t*) 0x40210318UL)
#define CYREG_FAULT_STRUCT3_DATA3       ((volatile un_FAULT_STRUCT_DATA_t*) 0x4021031CUL)
#define CYREG_FAULT_STRUCT3_PENDING0    ((volatile un_FAULT_STRUCT_PENDING0_t*) 0x40210340UL)
#define CYREG_FAULT_STRUCT3_PENDING1    ((volatile un_FAULT_STRUCT_PENDING1_t*) 0x40210344UL)
#define CYREG_FAULT_STRUCT3_PENDING2    ((volatile un_FAULT_STRUCT_PENDING2_t*) 0x40210348UL)
#define CYREG_FAULT_STRUCT3_MASK0       ((volatile un_FAULT_STRUCT_MASK0_t*) 0x40210350UL)
#define CYREG_FAULT_STRUCT3_MASK1       ((volatile un_FAULT_STRUCT_MASK1_t*) 0x40210354UL)
#define CYREG_FAULT_STRUCT3_MASK2       ((volatile un_FAULT_STRUCT_MASK2_t*) 0x40210358UL)
#define CYREG_FAULT_STRUCT3_INTR        ((volatile un_FAULT_STRUCT_INTR_t*) 0x402103C0UL)
#define CYREG_FAULT_STRUCT3_INTR_SET    ((volatile un_FAULT_STRUCT_INTR_SET_t*) 0x402103C4UL)
#define CYREG_FAULT_STRUCT3_INTR_MASK   ((volatile un_FAULT_STRUCT_INTR_MASK_t*) 0x402103C8UL)
#define CYREG_FAULT_STRUCT3_INTR_MASKED ((volatile un_FAULT_STRUCT_INTR_MASKED_t*) 0x402103CCUL)

#endif /* _CYREG_FAULT_H_ */


/* [] END OF FILE */
