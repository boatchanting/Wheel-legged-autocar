/***************************************************************************//**
* \file cyreg_backup.h
*
* \brief
* BACKUP register definition header
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

#ifndef _CYREG_BACKUP_H_
#define _CYREG_BACKUP_H_

#include "cyip_backup_v3.h"

/**
  * \brief SRSS Backup Domain (ver3) (BACKUP)
  */
#define CYREG_BACKUP_CTL                ((volatile un_BACKUP_CTL_t*) 0x40270000UL)
#define CYREG_BACKUP_RTC_RW             ((volatile un_BACKUP_RTC_RW_t*) 0x40270008UL)
#define CYREG_BACKUP_CAL_CTL            ((volatile un_BACKUP_CAL_CTL_t*) 0x4027000CUL)
#define CYREG_BACKUP_STATUS             ((volatile un_BACKUP_STATUS_t*) 0x40270010UL)
#define CYREG_BACKUP_RTC_TIME           ((volatile un_BACKUP_RTC_TIME_t*) 0x40270014UL)
#define CYREG_BACKUP_RTC_DATE           ((volatile un_BACKUP_RTC_DATE_t*) 0x40270018UL)
#define CYREG_BACKUP_ALM1_TIME          ((volatile un_BACKUP_ALM1_TIME_t*) 0x4027001CUL)
#define CYREG_BACKUP_ALM1_DATE          ((volatile un_BACKUP_ALM1_DATE_t*) 0x40270020UL)
#define CYREG_BACKUP_ALM2_TIME          ((volatile un_BACKUP_ALM2_TIME_t*) 0x40270024UL)
#define CYREG_BACKUP_ALM2_DATE          ((volatile un_BACKUP_ALM2_DATE_t*) 0x40270028UL)
#define CYREG_BACKUP_INTR               ((volatile un_BACKUP_INTR_t*) 0x4027002CUL)
#define CYREG_BACKUP_INTR_SET           ((volatile un_BACKUP_INTR_SET_t*) 0x40270030UL)
#define CYREG_BACKUP_INTR_MASK          ((volatile un_BACKUP_INTR_MASK_t*) 0x40270034UL)
#define CYREG_BACKUP_INTR_MASKED        ((volatile un_BACKUP_INTR_MASKED_t*) 0x40270038UL)
#define CYREG_BACKUP_RESET              ((volatile un_BACKUP_RESET_t*) 0x40270048UL)
#define CYREG_BACKUP_BREG0              ((volatile un_BACKUP_BREG_t*) 0x40271000UL)
#define CYREG_BACKUP_BREG1              ((volatile un_BACKUP_BREG_t*) 0x40271004UL)
#define CYREG_BACKUP_BREG2              ((volatile un_BACKUP_BREG_t*) 0x40271008UL)
#define CYREG_BACKUP_BREG3              ((volatile un_BACKUP_BREG_t*) 0x4027100CUL)

#endif /* _CYREG_BACKUP_H_ */


/* [] END OF FILE */
