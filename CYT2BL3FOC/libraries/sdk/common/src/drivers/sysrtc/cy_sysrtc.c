/***************************************************************************//**
* \file cy_sysrtc.c
* \version 1.0
*
* This file provides constants and parameter values for the APIs for the 
* Real-Time Clock (RTC).
*
********************************************************************************
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_sysrtc.h"

#if defined(__cplusplus)
extern "C" {
#endif

/** RTC days in months table */
uint8_t const cy_Rtc_daysInMonthTbl[CY_RTC_MONTHS_PER_YEAR] =
{
    CY_RTC_DAYS_IN_JANUARY,
    CY_RTC_DAYS_IN_FEBRUARY,
    CY_RTC_DAYS_IN_MARCH,
    CY_RTC_DAYS_IN_APRIL,
    CY_RTC_DAYS_IN_MAY,
    CY_RTC_DAYS_IN_JUNE,
    CY_RTC_DAYS_IN_JULY,
    CY_RTC_DAYS_IN_AUGUST,
    CY_RTC_DAYS_IN_SEPTEMBER,
    CY_RTC_DAYS_IN_OCTOBER,
    CY_RTC_DAYS_IN_NOVEMBER,
    CY_RTC_DAYS_IN_DECEMBER
};

static void Cy_Rtc_ConstructTimeDate(cy_stc_rtc_config_t const *timeDate, uint32_t *time, uint32_t *date);
static void Cy_Rtc_ConstructAlarmTimeDate(cy_stc_rtc_alarm_t const *alarmDateTime, uint32_t *alarmTime, uint32_t *alarmDate);
static uint32_t Cy_Rtc_RelativeToFixed(cy_str_rtc_dst_format_t const *convertDst);


/*******************************************************************************
* Function Name: Cy_Rtc_Init
****************************************************************************//**
* Initializes the RTC driver and returns an RTC register address.
*
* \param
* *config The pointer to the RTC configuration structure, see \ref 
* cy_stc_rtc_config_t.
*
* \return
* cy_en_rtc_status_t *config checking result. If the pointer is NULL, 
* returns an error.
*
*******************************************************************************/
cy_en_rtc_status_t Cy_Rtc_Init(cy_stc_rtc_config_t const *config)
{
    cy_en_rtc_status_t retVal;

    if(NULL != config)
    {
       retVal = Cy_Rtc_SetDateAndTime(config);
    }
    else
    {
        retVal = CY_RTC_INVALID_STATE;
    }
    return(retVal);
}

/*******************************************************************************
* Function Name: Cy_Rtc_SetHourBit
****************************************************************************//**
*
* Only set the hour bit with out any checking. Please make sure write bit has 
* been enabled before calling this function.
*
* \param hour Hour value to be set. only 0~23 will be accepted.
*
* \return \ref cy_en_rtc_status_t
*
*******************************************************************************/
cy_en_rtc_status_t Cy_Rtc_SetHourBit(uint8_t hour)
{
    BACKUP->unRTC_TIME.stcField.u5RTC_HOUR = hour;
    return CY_RTC_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_Rtc_SetReadBit
****************************************************************************//**
*
* Sets READ bit with checking busy status.
*
* \return \ref cy_en_rtc_status_t 
*
*******************************************************************************/
cy_en_rtc_status_t Cy_Rtc_SetReadBit(void)
{
    if(BACKUP->unSTATUS.stcField.u1RTC_BUSY == 1)
    {
        return CY_RTC_INVALID_STATE;
    }

    BACKUP->unRTC_RW.stcField.u1READ = 1ul;

    return(CY_RTC_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_Rtc_ClearReadBit
****************************************************************************//**
*
* Clears READ bit.
*
* \return \ref cy_en_rtc_status_t
*
*******************************************************************************/
cy_en_rtc_status_t Cy_Rtc_ClearReadBit(void)
{
    BACKUP->unRTC_RW.stcField.u1READ = 0ul;

    return(CY_RTC_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_Rtc_IsReadBitSet
****************************************************************************//**
*
* Returns READ bit status.
*
* \return true : If the READ bit is set.
*         false: If the READ bit is cleared.
*
*******************************************************************************/
bool Cy_Rtc_IsReadBitSet(void)
{
    if(BACKUP->unRTC_RW.stcField.u1READ == 0ul)
    {
        return false;
    }
    else
    {
        return true;
    }
}
/*******************************************************************************
* Function Name: Cy_Rtc_CalibrationControlEnable
****************************************************************************//**
*
* Set calibration control register with checking write bit status.
*
* \param calib_val
*
* \param calib_sign \ref cy_en_rtc_calib_sign_t
*
* \param calib_val \ref cy_en_rtc_cal_sel_t
*
* \return \ref cy_en_rtc_status_t
*
*******************************************************************************/
cy_en_rtc_status_t Cy_Rtc_CalibrationControlEnable(uint8_t calib_val, cy_en_rtc_calib_sign_t calib_sign, cy_en_rtc_cal_sel_t cal_sel)
{
    if(BACKUP->unRTC_RW.stcField.u1WRITE == 0)
    {
        // Writes are ignored unless Write bit is set
        return CY_RTC_INVALID_STATE;
    }

    un_BACKUP_CAL_CTL_t tmpBackupCalCtl = {0ul};
    tmpBackupCalCtl.stcField.u6CALIB_VAL  = calib_val;
    tmpBackupCalCtl.stcField.u1CALIB_SIGN = (uint32_t)calib_sign;
    tmpBackupCalCtl.stcField.u2CAL_SEL    = (uint32_t)cal_sel;
    tmpBackupCalCtl.stcField.u1CAL_OUT    = 1ul;                  // Output enable for wave signal for calibration and allow CALIB_VAL to be written.
    BACKUP->unCAL_CTL.u32Register = tmpBackupCalCtl.u32Register;

    return CY_RTC_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_Rtc_CalibrationControlDisable
****************************************************************************//**
*
* Disable calibration control.
*
* \return \ref cy_en_rtc_status_t
*
*******************************************************************************/
cy_en_rtc_status_t Cy_Rtc_CalibrationControlDisable(void)
{
    if(BACKUP->unRTC_RW.stcField.u1WRITE == 0)
    {
        // Writes are ignored unless Write bit is set
        return CY_RTC_INVALID_STATE;
    }

    BACKUP->unCAL_CTL.stcField.u1CAL_OUT    = 0ul; // Output disable for wave signal for calibration

    return CY_RTC_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_Rtc_clock_source
****************************************************************************//**
* Sets the clock source for the RTC block.
*
* \param
* clock_source The clock source to be set, see \ref cy_en_rtc_clock_src_t.
*
* \return
* none
*
*******************************************************************************/
void Cy_Rtc_clock_source(cy_en_rtc_clock_src_t clock_source)
{
    BACKUP->unCTL.stcField.u2CLK_SEL = clock_source;
}

/*******************************************************************************
* Function Name: Cy_Rtc_SetDateAndTime
****************************************************************************//**
*
* Sets the time and date values into the RTC_TIME and RTC_DATE registers.
*
* \param
* dateTime  The pointer to the RTC configuration structure,
* see \ref cy_stc_rtc_config_t.
*
* \return
* cy_en_rtc_status_t A validation check result of date and month. Returns an error, if 
* the date range is invalid.
*
*******************************************************************************/
cy_en_rtc_status_t Cy_Rtc_SetDateAndTime(cy_stc_rtc_config_t const *dateTime)
{
    uint32_t tmpTime;
    uint32_t tmpDate;
    uint32_t tmpDaysInMonth;
    uint32_t interruptState;

    cy_en_rtc_status_t retVal = CY_RTC_BAD_PARAM;

    /* Check the input parameters valid ranges */
    if((dateTime->month > 0ul) && (dateTime->month <= CY_RTC_MONTHS_PER_YEAR) && (dateTime->year <= CY_RTC_MAX_YEAR))
    {
        tmpDaysInMonth = Cy_Rtc_DaysInMonth(dateTime->month, (dateTime->year + CY_RTC_TWO_THOUSAND_YEARS));
        
        /* Check if the date is in the valid range */
        if((dateTime->date > 0ul) && (dateTime->date <= tmpDaysInMonth))
        {
            Cy_Rtc_ConstructTimeDate(dateTime, &tmpTime, &tmpDate);

            /* The RTC AHB register can be updated only under condition that the
            *  Write bit is set and the RTC busy bit is cleared (CY_RTC_BUSY = 0).
            */
            interruptState = Cy_SysLib_EnterCriticalSection();
            retVal = Cy_Rtc_WriteEnable(CY_RTC_WRITE_ENABLED);
            if(retVal == CY_RTC_SUCCESS)
            {
                BACKUP->unRTC_TIME.u32Register = tmpTime;
                BACKUP->unRTC_DATE.u32Register = tmpDate;
                
                /* Clear the RTC Write bit to finish RTC register update */
                retVal = Cy_Rtc_WriteEnable(CY_RTC_WRITE_DISABLED);
            }
            Cy_SysLib_ExitCriticalSection(interruptState);
        }
    }
    return(retVal);
}


/*******************************************************************************
* Function Name: Cy_Rtc_GetDateAndTime
****************************************************************************//**
*
* Gets the current RTC time and date. The AHB RTC Time and Date register values 
* are stored into the *dateTime structure.
*
* \param
* dateTime The RTC time and date structure, see 
* \ref group_rtc_data_structures.
*
*******************************************************************************/
void   Cy_Rtc_GetDateAndTime(cy_stc_rtc_config_t* dateTime)
{
    uint32_t tmpTime;
    uint32_t tmpDate;

    /* Read the current RTC time and date to validate the input parameters */
    Cy_Rtc_SyncRegisters();

    /* Write the AHB RTC registers date and time into the local variables and 
    * updating the dateTime structure elements
    */
    tmpTime = BACKUP->unRTC_TIME.u32Register;
    tmpDate = BACKUP->unRTC_DATE.u32Register;

    dateTime->sec    = (_FLD2VAL(BACKUP_RTC_TIME_RTC_SEC, tmpTime));
    dateTime->min    = (_FLD2VAL(BACKUP_RTC_TIME_RTC_MIN, tmpTime));
    dateTime->hrMode = (_FLD2VAL(BACKUP_RTC_TIME_CTRL_12HR, tmpTime));

    /* Read the current hour mode to know how many hour bits should be converted
    * In the 24-hour mode, the hour value is presented in [21:16] bits in the 
    * Integer format.
    * In the 12-hour mode the hour value is presented in [20:16] bits in the Integer
    * format and bit [21] is present: 0 - AM; 1 - PM. 
    */
    if(dateTime->hrMode != CY_RTC_24_HOURS)
    {
        dateTime->hour = 
        ((tmpTime & CY_RTC_BACKUP_RTC_TIME_RTC_12HOUR) >> BACKUP_RTC_TIME_RTC_HOUR_Pos);

        dateTime->amPm = ((0ul != (tmpTime & CY_RTC_BACKUP_RTC_TIME_RTC_PM)) ? CY_RTC_PM : CY_RTC_AM);
    }
    else
    {
        dateTime->hour = (_FLD2VAL(BACKUP_RTC_TIME_RTC_HOUR, tmpTime));
    }
    dateTime->dayOfWeek =(_FLD2VAL(BACKUP_RTC_TIME_RTC_DAY, tmpTime));
    dateTime->date  = (_FLD2VAL(BACKUP_RTC_DATE_RTC_DATE, tmpDate));
    dateTime->month = (_FLD2VAL(BACKUP_RTC_DATE_RTC_MON, tmpDate));
    dateTime->year  = (_FLD2VAL(BACKUP_RTC_DATE_RTC_YEAR, tmpDate));
}


/*******************************************************************************
* Function Name: Cy_Rtc_SetAlarmDateAndTime
****************************************************************************//**
*
* Sets alarm time and date values into the ALMx_TIME and ALMx_DATE registers.
*
* \param
* alarmDateTime The alarm configuration structure, see \ref cy_stc_rtc_alarm_t.
*
* \param
* alarmIndex The alarm index to be configured, see \ref cy_en_rtc_alarm_t.
*
* \return
* cy_en_rtc_status_t A validation check result of date and month. Returns an error, if
* the date range is invalid.
*
*******************************************************************************/
cy_en_rtc_status_t Cy_Rtc_SetAlarmDateAndTime(cy_stc_rtc_alarm_t const *alarmDateTime, cy_en_rtc_alarm_t alarmIndex)
{
    uint32_t tmpAlarmTime;
    uint32_t tmpAlarmDate;
    uint32_t tmpYear;
    uint32_t tmpDaysInMonth;
    uint32_t interruptState;
    cy_en_rtc_status_t retVal = CY_RTC_BAD_PARAM;

    /* Read the current RTC time and date to validate the input parameters */
    Cy_Rtc_SyncRegisters();

    tmpYear = CY_RTC_TWO_THOUSAND_YEARS + (_FLD2VAL(BACKUP_RTC_DATE_RTC_YEAR, BACKUP->unRTC_DATE.u32Register));

    /* Parameters validation */
    if((alarmDateTime->month > 0ul) && (alarmDateTime->month <= CY_RTC_MONTHS_PER_YEAR))
    {
        tmpDaysInMonth = Cy_Rtc_DaysInMonth(alarmDateTime->month, tmpYear);
        
        if((alarmDateTime->date > 0ul) && (alarmDateTime->date <= tmpDaysInMonth))
        {
            Cy_Rtc_ConstructAlarmTimeDate(alarmDateTime, &tmpAlarmTime, &tmpAlarmDate);
            
            /* The RTC AHB register can be updated only under condition that the
            *  Write bit is set and the RTC busy bit is cleared (RTC_BUSY = 0).
            */
            interruptState = Cy_SysLib_EnterCriticalSection();
            retVal = Cy_Rtc_WriteEnable(CY_RTC_WRITE_ENABLED);
            if(CY_RTC_SUCCESS == retVal)
            {
                /* Update the AHB RTC registers with formed values */
                if(alarmIndex != CY_RTC_ALARM_2)
                {
                    BACKUP->unALM1_TIME.u32Register = tmpAlarmTime;
                    BACKUP->unALM1_DATE.u32Register = tmpAlarmDate;
                }
                else
                {
                    BACKUP->unALM2_TIME.u32Register = tmpAlarmTime;
                    BACKUP->unALM2_DATE.u32Register = tmpAlarmDate;
                }

                /* Clear the RTC Write bit to finish RTC update */
                retVal = Cy_Rtc_WriteEnable(CY_RTC_WRITE_DISABLED);
            }
            Cy_SysLib_ExitCriticalSection(interruptState);
        }
    }
    return(retVal);
}


/*******************************************************************************
* Function Name: Cy_Rtc_GetAlarmDateAndTime
****************************************************************************//**
*
* Returns the current alarm time and date values from the ALMx_TIME and 
* ALMx_DATE registers.
*
* \param
* alarmDateTime The alarm configuration structure, see \ref cy_stc_rtc_alarm_t.
*
* \param alarmIndex The alarm index to be configured, see \ref cy_en_rtc_alarm_t.
*
*******************************************************************************/
void Cy_Rtc_GetAlarmDateAndTime(cy_stc_rtc_alarm_t *alarmDateTime, cy_en_rtc_alarm_t alarmIndex)
{
    uint32_t tmpAlarmTime;
    uint32_t tmpAlarmDate;
    uint32_t curHourMode;

    /* Read the current RTC time and date to validate the input parameters */
    Cy_Rtc_SyncRegisters();

    /* Write the AHB RTC registers into the local variables and update the
    * alarmDateTime structure elements
    */
    if(alarmIndex != CY_RTC_ALARM_2)
    {
        tmpAlarmTime = BACKUP->unALM1_TIME.u32Register;
        tmpAlarmDate = BACKUP->unALM1_DATE.u32Register;
        curHourMode = (_FLD2VAL(BACKUP_RTC_TIME_CTRL_12HR, BACKUP->unRTC_TIME.u32Register));

        alarmDateTime->sec    = (_FLD2VAL(BACKUP_ALM1_TIME_ALM_SEC, tmpAlarmTime));
        alarmDateTime->sec_en = (_FLD2VAL(BACKUP_ALM1_TIME_ALM_SEC_EN, tmpAlarmTime));
        
        alarmDateTime->min    = (_FLD2VAL(BACKUP_ALM1_TIME_ALM_MIN, tmpAlarmTime));
        alarmDateTime->min_en = (_FLD2VAL(BACKUP_ALM1_TIME_ALM_MIN_EN, tmpAlarmTime)); 

        /* Read the current hour mode to know how many hour bits to convert.
        *  In the 24-hour mode, the hour value is presented in [21:16] bits in 
        *  the Integer format.
        *  In the 12-hour mode, the hour value is presented in [20:16] bits in 
        *  the Integer format and bit [21] is present: 0 - AM; 1 - PM.
        */
        if(curHourMode != CY_RTC_24_HOURS)
        {
            alarmDateTime->hour = 
            ((tmpAlarmTime & CY_RTC_BACKUP_RTC_TIME_RTC_12HOUR) >> BACKUP_ALM1_TIME_ALM_HOUR_Pos);

            /* In the structure, the hour value should be presented in the 24-hour mode. In 
            *  that condition the firmware checks the AM/PM status and adds 12 hours to 
            *  the converted hour value if the PM bit is set.
            */
            if((alarmDateTime->hour < CY_RTC_HOURS_PER_HALF_DAY) && 
            (0ul != (BACKUP->unALM1_TIME.u32Register & CY_RTC_BACKUP_RTC_TIME_RTC_PM)))
            {
                alarmDateTime->hour += CY_RTC_HOURS_PER_HALF_DAY;
            }

            /* Set zero hour, as the 12 A hour is zero hour in 24-hour format */
            if((alarmDateTime->hour == CY_RTC_HOURS_PER_HALF_DAY) && 
              (0ul == (BACKUP->unALM1_TIME.u32Register & CY_RTC_BACKUP_RTC_TIME_RTC_PM)))
            {
                alarmDateTime->hour = 0ul;
            }

        }
        else
        {
            alarmDateTime->hour = (_FLD2VAL(BACKUP_ALM1_TIME_ALM_HOUR, tmpAlarmTime));
        }
        alarmDateTime->hour_en = (_FLD2VAL(BACKUP_ALM1_TIME_ALM_HOUR_EN, tmpAlarmTime));

        alarmDateTime->dayOfWeek = (_FLD2VAL(BACKUP_ALM1_TIME_ALM_DAY, tmpAlarmTime));
        alarmDateTime->dayOfWeek_en = (_FLD2VAL(BACKUP_ALM1_TIME_ALM_DAY_EN, tmpAlarmTime));

        alarmDateTime->date = (_FLD2VAL(BACKUP_ALM1_DATE_ALM_DATE, tmpAlarmDate));
        alarmDateTime->date_en  = (_FLD2VAL(BACKUP_ALM1_DATE_ALM_DATE_EN, tmpAlarmDate));

        alarmDateTime->month = (_FLD2VAL(BACKUP_ALM1_DATE_ALM_MON, tmpAlarmDate)); 
        alarmDateTime->month_en = (_FLD2VAL(BACKUP_ALM1_DATE_ALM_MON_EN, tmpAlarmDate));

        alarmDateTime->alm_en   = (_FLD2VAL(BACKUP_ALM1_DATE_ALM_EN, tmpAlarmDate));
    }
    else
    {
        tmpAlarmTime = BACKUP->unALM2_TIME.u32Register;
        tmpAlarmDate = BACKUP->unALM2_DATE.u32Register;
        curHourMode = (_FLD2VAL(BACKUP_RTC_TIME_CTRL_12HR, BACKUP->unRTC_TIME.u32Register));

        alarmDateTime->sec    = (_FLD2VAL(BACKUP_ALM2_TIME_ALM_SEC, tmpAlarmTime));
        alarmDateTime->sec_en = (_FLD2VAL(BACKUP_ALM2_TIME_ALM_SEC_EN, tmpAlarmTime));
        
        alarmDateTime->min    = (_FLD2VAL(BACKUP_ALM2_TIME_ALM_MIN, tmpAlarmTime));
        alarmDateTime->min_en = (_FLD2VAL(BACKUP_ALM2_TIME_ALM_MIN_EN, tmpAlarmTime)); 

        /* Read the current hour mode to know how many hour bits to convert.
        *  In the 24-hour mode, the hour value is presented in [21:16] bits in 
        *  the Integer format.
        *  In the 12-hour mode the hour value is presented in [20:16] bits in 
        *  the Integer format and bit [21] is present: 0 - AM; 1 - PM.
        */
        if(curHourMode != CY_RTC_24_HOURS)
        {
            alarmDateTime->hour = 
            ((tmpAlarmTime & CY_RTC_BACKUP_RTC_TIME_RTC_12HOUR) >> BACKUP_ALM2_TIME_ALM_HOUR_Pos);

            /* In the structure, the hour value should be presented in the 24-hour mode. In 
            *  that condition the firmware checks the AM/PM status and adds 12 hours to 
            *  the converted hour value if the PM bit is set.
            */
            if((alarmDateTime->hour < CY_RTC_HOURS_PER_HALF_DAY) && 
            (0ul != (BACKUP->unALM2_TIME.u32Register & CY_RTC_BACKUP_RTC_TIME_RTC_PM)))
            {
                alarmDateTime->hour += CY_RTC_HOURS_PER_HALF_DAY;
            }
            /* Set zero hour, as the 12 am hour is zero hour in 24-hour format */
            else if((alarmDateTime->hour == CY_RTC_HOURS_PER_HALF_DAY) && 
                    (0ul == (BACKUP->unALM2_TIME.u32Register & CY_RTC_BACKUP_RTC_TIME_RTC_PM)))
            {
                alarmDateTime->hour = 0ul;
            }
            else
            {
                /* No corrections are required */
            }
        }
        else
        {
            alarmDateTime->hour = (_FLD2VAL(BACKUP_ALM2_TIME_ALM_HOUR, tmpAlarmTime));
        }
        alarmDateTime->hour_en = (_FLD2VAL(BACKUP_ALM2_TIME_ALM_HOUR_EN, tmpAlarmTime));

        alarmDateTime->dayOfWeek = (_FLD2VAL(BACKUP_ALM2_TIME_ALM_DAY, tmpAlarmTime));
        alarmDateTime->dayOfWeek_en = (_FLD2VAL(BACKUP_ALM2_TIME_ALM_DAY_EN, tmpAlarmTime));

        alarmDateTime->date = (_FLD2VAL(BACKUP_ALM2_DATE_ALM_DATE, tmpAlarmDate));
        alarmDateTime->date_en  = (_FLD2VAL(BACKUP_ALM2_DATE_ALM_DATE_EN, tmpAlarmDate));

        alarmDateTime->month = (_FLD2VAL(BACKUP_ALM2_DATE_ALM_MON, tmpAlarmDate));
        alarmDateTime->month_en = (_FLD2VAL(BACKUP_ALM2_DATE_ALM_MON_EN, tmpAlarmDate));

        alarmDateTime->alm_en = (_FLD2VAL(BACKUP_ALM2_DATE_ALM_EN, tmpAlarmDate));
    }
}


/*******************************************************************************
* Function Name: Cy_Rtc_SetDateAndTimeDirect
****************************************************************************//**
*
* Sets the time and date values into the RTC_TIME and RTC_DATE registers using 
* direct time parameters.
*
* \param sec The second valid range is [0-59].
*
* \param min The minute valid range is [0-59].
*
* \param
* hour The hour valid range is [0-23]. This parameter type should be presented 
* in the 24-hour type.
*
* The function reads the current 12/24-hour mode, then converts the hour value
* properly as the mode.
*
* \param
* date The date valid range is [1-31], if the month of February is 
* selected as the Month parameter, then the valid range is [0-29].
*
* \param month The month valid range is [1-12].
*
* \param year The year valid range is [0-99].
*
* \return
* cy_en_rtc_status_t A validation check result of date and month. Returns an error, if 
* the date range is invalid or the RTC time and date set was cancelled: the RTC 
* Write bit was not set, the RTC was synchronizing.
*
*******************************************************************************/
cy_en_rtc_status_t Cy_Rtc_SetDateAndTimeDirect(uint32_t sec, uint32_t min, uint32_t hour, 
                                               uint32_t date, uint32_t month, uint32_t year)
{
    cy_stc_rtc_config_t curTimeAndDate;
    uint32_t tmpDaysInMonth;
    uint32_t tmpTime;
    uint32_t tmpDate;
    uint32_t interruptState;

    cy_en_rtc_status_t retVal = CY_RTC_BAD_PARAM;

    /* Check if the parameters are in their valid ranges */
    if((month > 0ul) && (month <= CY_RTC_MONTHS_PER_YEAR) && (year <= CY_RTC_MAX_YEAR))
    {
        /* Calculate days in a month for the current month and year */
        tmpDaysInMonth = Cy_Rtc_DaysInMonth(month, (year + CY_RTC_TWO_THOUSAND_YEARS));
        
        /* Check if the parameters are in their valid ranges */
        if((sec <= CY_RTC_MAX_SEC_OR_MIN) && (min <= CY_RTC_MAX_SEC_OR_MIN) && (hour <= CY_RTC_MAX_HOURS_24H) &&
           (date > 0ul) && (date <= tmpDaysInMonth))
        {
            /* Fill the date and time structure */
            curTimeAndDate.sec = sec;
            curTimeAndDate.min = min;
            
            /* Read the current hour mode */
            Cy_Rtc_SyncRegisters();
            
            if(CY_RTC_12_HOURS != Cy_Rtc_GetHoursFormat())
            {
                curTimeAndDate.hrMode = CY_RTC_24_HOURS;
                curTimeAndDate.hour = hour;
            }
            else
            {
                curTimeAndDate.hrMode = CY_RTC_12_HOURS;

                /* Convert the 24-hour format input value into the 12-hour format */
                if(hour >= CY_RTC_HOURS_PER_HALF_DAY)
                {
                    /* The current hour is more than 12 in the 24-hour mode. Set
                    *  the PM bit and convert the hour: hour = hour - 12, except
                    *  that the hour is 12.
                    */
                    curTimeAndDate.hour = (uint32_t) hour - CY_RTC_HOURS_PER_HALF_DAY;
                    curTimeAndDate.hour = 
                    ((0ul != curTimeAndDate.hour) ? curTimeAndDate.hour : CY_RTC_HOURS_PER_HALF_DAY);

                    curTimeAndDate.amPm = CY_RTC_PM;
                }
                else if(hour < 1ul)
                {
                    /* The current hour in the 24-hour mode is 0 which is equal 
                    *  to 12:00 AM
                    */
                    curTimeAndDate.hour = CY_RTC_HOURS_PER_HALF_DAY;
                    curTimeAndDate.amPm = CY_RTC_AM; 
                }
                else
                {
                    /* The current hour is less than 12. Set the AM bit */
                    curTimeAndDate.hour = hour;
                    curTimeAndDate.amPm = CY_RTC_AM;
                }
            }
            curTimeAndDate.dayOfWeek = Cy_Rtc_ConvertDayOfWeek(date, month, year);
            curTimeAndDate.date = date;
            curTimeAndDate.month = month;
            curTimeAndDate.year = year;

            Cy_Rtc_ConstructTimeDate(&curTimeAndDate, &tmpTime, &tmpDate);

            /* The RTC AHB register can be updated only under condition that the
            *  Write bit is set and the RTC busy bit is cleared (CY_RTC_BUSY = 0).
            */
            interruptState = Cy_SysLib_EnterCriticalSection();
            retVal = Cy_Rtc_WriteEnable(CY_RTC_WRITE_ENABLED);
            if(retVal == CY_RTC_SUCCESS)
            {
                BACKUP->unRTC_TIME.u32Register = tmpTime;
                BACKUP->unRTC_DATE.u32Register  = tmpDate;

                /* Clear the RTC Write bit to finish RTC register update */
                retVal = Cy_Rtc_WriteEnable(CY_RTC_WRITE_DISABLED);
            }
            Cy_SysLib_ExitCriticalSection(interruptState);
        }
    }
    return(retVal);
}


/*******************************************************************************
* Function Name: Cy_Rtc_SetAlarmDateAndTimeDirect
****************************************************************************//**
*
* Sets alarm time and date values into the ALMx_TIME and ALMx_DATE 
* registers using direct time parameters. ALM_DAY_EN is default 0 (=ignore) for 
* this function.
*
* \param sec The alarm second valid range is [0-59].
*
* \param min The alarm minute valid range is [0-59].
*
* \param hour
* The valid range is [0-23].
* This parameter type is always in the 24-hour type. This function reads the 
* current 12/24-hour mode, then converts the hour value properly as the mode.
*
* \param
* date The valid range is [1-31], if the month of February is selected as
* the Month parameter, then the valid range is [0-29].
*
* \param month The alarm month valid range is [1-12].
*
* \param
* alarmIndex The alarm index to be configured, see \ref cy_en_rtc_alarm_t.
*
* \return
* cy_en_rtc_status_t A validation check result of date and month. Returns an error, if 
* the date range is invalid.
*
*******************************************************************************/
cy_en_rtc_status_t Cy_Rtc_SetAlarmDateAndTimeDirect(uint32_t sec, uint32_t min, uint32_t hour, 
                                                    uint32_t date, uint32_t month, uint32_t alarmIndex)
{
    cy_stc_rtc_alarm_t alarmDateTime;
    uint32_t tmpDaysInMonth;
    uint32_t tmpCurrentYear;
    uint32_t tmpAlarmTime;
    uint32_t tmpAlarmDate;
    uint32_t interruptState;

    cy_en_rtc_status_t retVal = CY_RTC_BAD_PARAM;

    /* Read the current time to validate the input parameters */
    Cy_Rtc_SyncRegisters();

    /* Get the current year value to calculate */
    tmpCurrentYear =(_FLD2VAL(BACKUP_RTC_DATE_RTC_YEAR, BACKUP->unRTC_DATE.u32Register));
    
    /* Check if the parameters are in their valid ranges */
    if((month > 0ul) && (month < CY_RTC_MONTHS_PER_YEAR))
    {
        tmpDaysInMonth = Cy_Rtc_DaysInMonth(month, (tmpCurrentYear + CY_RTC_TWO_THOUSAND_YEARS));

        if((sec <= CY_RTC_MAX_SEC_OR_MIN) && (min <= CY_RTC_MAX_SEC_OR_MIN) && (hour <= CY_RTC_MAX_HOURS_24H) &&
           (date > 0ul) &&(date <= tmpDaysInMonth))
        {
            /* Fill the alarm structure */
            alarmDateTime.sec          = sec;
            alarmDateTime.sec_en       = CY_RTC_ALARM_ENABLE;
            alarmDateTime.min          = min;
            alarmDateTime.min_en       = CY_RTC_ALARM_ENABLE;
            alarmDateTime.hour         = hour;
            alarmDateTime.hour_en      = CY_RTC_ALARM_ENABLE;
            alarmDateTime.dayOfWeek    = CY_RTC_SUNDAY;
            alarmDateTime.dayOfWeek_en = CY_RTC_ALARM_DISABLE;
            alarmDateTime.date         = date;
            alarmDateTime.date_en      = CY_RTC_ALARM_ENABLE;
            alarmDateTime.month        = month;
            alarmDateTime.month_en     = CY_RTC_ALARM_ENABLE;
            alarmDateTime.alm_en       = CY_RTC_ALARM_ENABLE;

            Cy_Rtc_ConstructAlarmTimeDate(&alarmDateTime, &tmpAlarmTime, &tmpAlarmDate);

            /* The RTC AHB register can be updated only under condition that the
            *  Write bit is set and the RTC busy bit is cleared (CY_RTC_BUSY = 0).
            */
            interruptState = Cy_SysLib_EnterCriticalSection();
            retVal = Cy_Rtc_WriteEnable(CY_RTC_WRITE_ENABLED);
            if(retVal == CY_RTC_SUCCESS)
            {
                if(alarmIndex != ((uint32_t) CY_RTC_ALARM_2))
                {
                    BACKUP->unALM1_TIME.u32Register = tmpAlarmTime;
                    BACKUP->unALM1_DATE.u32Register = tmpAlarmDate;
                }
                else
                {
                    BACKUP->unALM2_TIME.u32Register = tmpAlarmTime;
                    BACKUP->unALM2_DATE.u32Register = tmpAlarmDate;
                }

                /* Clear the RTC Write bit to finish RTC register update */
                retVal = Cy_Rtc_WriteEnable(CY_RTC_WRITE_DISABLED);
            }
            Cy_SysLib_ExitCriticalSection(interruptState);
        }
    }
    return(retVal);
}


/*******************************************************************************
* Function Name: Cy_Rtc_SetHoursFormat
****************************************************************************//**
*
* Sets the 12/24-hour mode.
*
* \param hourMode The current hour format, see \ref group_rtc_hour_format.
*
* \return cy_en_rtc_status_t A validation check result of RTC register update.
*
*******************************************************************************/
cy_en_rtc_status_t Cy_Rtc_SetHoursFormat(uint32_t hourMode)
{
    uint32_t interruptState;
    uint32_t curTime;
    uint32_t hourValue;
    cy_en_rtc_status_t retVal = CY_RTC_INVALID_STATE;

    /* Read the current time to validate the input parameters */
    Cy_Rtc_SyncRegisters();
    curTime = BACKUP->unRTC_TIME.u32Register;

    /* Hour format can be changed in condition that current hour format is not 
    * the same as requested in function argument
    */
    if(hourMode != Cy_Rtc_GetHoursFormat())
    {
        /* Convert the current hour value from 24H into the 12H format */
        if(hourMode == CY_RTC_12_HOURS)
        {
            hourValue = (_FLD2VAL(BACKUP_RTC_TIME_RTC_HOUR, curTime));
            if(hourValue >= CY_RTC_HOURS_PER_HALF_DAY)
            {
                /* The current hour is more than 12 or equal 12 in the 24-hour 
                *  mode. Set the PM bit and convert the hour: hour = hour - 12.
                */
                hourValue = (uint32_t) (hourValue - CY_RTC_HOURS_PER_HALF_DAY);
                hourValue = ((0ul != hourValue) ? hourValue : CY_RTC_HOURS_PER_HALF_DAY);

                curTime = (_CLR_SET_FLD32U(curTime, BACKUP_RTC_TIME_RTC_HOUR, (hourValue)));
                curTime |= CY_RTC_BACKUP_RTC_TIME_RTC_PM;
            }
            else if(hourValue < 1ul)
            {
                /* The current hour in the 24-hour mode is 0 which is equal 
                *  to 12:00 AM
                */
                curTime = (_CLR_SET_FLD32U(curTime, BACKUP_RTC_TIME_RTC_HOUR, (CY_RTC_HOURS_PER_HALF_DAY)));

                /* Set the AM bit */
                curTime &= ((uint32_t) ~CY_RTC_BACKUP_RTC_TIME_RTC_PM);
            }
            else
            {
                /* The current hour is less than 12 */
                curTime = (_CLR_SET_FLD32U(curTime, BACKUP_RTC_TIME_RTC_HOUR, (hourValue)));
                curTime &= ((uint32_t) ~CY_RTC_BACKUP_RTC_TIME_RTC_PM);
            }

            curTime |= BACKUP_RTC_TIME_CTRL_12HR_Msk;
        }
        /* Convert the hour value into 24H format value */
        else
        {
            /* Mask the AM/PM bit as the hour value is in [20:16] bits */
            hourValue = (_FLD2VAL(BACKUP_RTC_TIME_RTC_HOUR, (curTime & (uint32_t) ~CY_RTC_BACKUP_RTC_TIME_RTC_PM)));

            /* Add 12 hours in condition that current time is in PM period */
            if((hourValue < CY_RTC_HOURS_PER_HALF_DAY) && (0ul != (curTime & CY_RTC_BACKUP_RTC_TIME_RTC_PM)))
            {
                hourValue += CY_RTC_HOURS_PER_HALF_DAY;
            }

            /* Current hour is 12 AM which is equal to zero hour in 24-hour format */
            if((hourValue == CY_RTC_HOURS_PER_HALF_DAY) && (0ul == (curTime & CY_RTC_BACKUP_RTC_TIME_RTC_PM)))
            {
                hourValue = 0ul;
            }

            curTime = (_CLR_SET_FLD32U(curTime, BACKUP_RTC_TIME_RTC_HOUR, (hourValue)));
            curTime &= (uint32_t) ~BACKUP_RTC_TIME_CTRL_12HR_Msk;
        }

        /* Writing corrected hour value and hour format bit into the RTC AHB 
        *  register. The RTC AHB register can be updated only under condition 
        *  that the Write bit is set and the RTC busy bit is cleared
        *  (CY_RTC_BUSY = 0).
        */
        retVal = Cy_Rtc_WriteEnable(CY_RTC_WRITE_ENABLED);
        if(retVal == CY_RTC_SUCCESS)
        {
            interruptState = Cy_SysLib_EnterCriticalSection();
            BACKUP->unRTC_TIME.u32Register = curTime;
            Cy_SysLib_ExitCriticalSection(interruptState);

            /* Clear the RTC Write bit to finish RTC register update */
            retVal = Cy_Rtc_WriteEnable(CY_RTC_WRITE_DISABLED);
        }
    }
    return(retVal);
}


/*******************************************************************************
* Function Name: Cy_Rtc_SelectFrequencyPrescaler()
****************************************************************************//**
*
* Selects the RTC pre-scaler value and changes its clock frequency. Under 
* condition that the external 32.768 kHz WCO is absent on the board, the RTC can
* be driven by an external 50-Hz or 60-Hz sine-wave clock source, for example 
* the wall AC frequency.
*
* \param clkSel clock frequency, see \ref cy_en_rtc_clock_freq_t.
*
* In addition to generating the 32.768 kHz clock from external crystals, the WCO
* can be sourced by an external clock source (50 Hz or 60Hz), even the wall AC 
* frequency as a timebase, to drive the RTC using wco_out and wco_in pins. 
* The API helps select between the RTC sources:
* * The external crystal WCO <br>
* * An external 50-Hz or 60-Hz sine-wave clock source
*
* If you want to use an external 50-Hz or 60-Hz sine-wave clock source to 
* drive the WCO, the next procedure is required: <br>
* 1) Disable the WCO <br>
* 2) Drive the wco_out and wco_in pins to an external signal source* <br>
* 3) Call Cy_RTC_SelectFrequencyPrescaler(CY_RTC_FREQ_60_HZ) if you want to 
* drive the WCO, for example, with 60 Hz source <br>
* 4) Enable the WCO <br>
*
* If you want to use the WCO after using an external 50-Hz or 60-Hz sine-wave 
* clock source:  <br>
* 1) Disable the WCO <br>
* 2) Drive off wco_out and wco_in pins with external signal source <br>
* 3) Call Cy_RTC_SelectFrequencyPrescaler(CY_RTC_FREQ_WCO_32768_HZ) <br>
* 4) Enable the WCO <br>
* 
* \warning 
* There is a limitation to the external clock source frequencies. Only two 
* frequencies are allowed - 50 Hz or 60 Hz. Note that this limitation is related
* to the RTC pre-scaling feature presented in this function. This 
* limitation is not related to WCO external clock sources which can drive the 
* WCO in the bypass mode. 
*
* \note 
* 50-Hz and 60-Hz external pin sources must be a sine wave, connected to the 
* wco_input pin with the wco_out pin floating.
* An external 60-Hz sine wave derived from the 60Hz/120V mains through a 100:1 
* capacitive divider. For the capacitive divider, connect a 220pF/6V capacitor 
* between wco_input and ground, a 2,2pF/200V cap between wco_input pad and 
* the 60Hz/120V mains input.
* The same setup should be used for the European 50Hz/220V mains standard, 
* except that a 1pF/250V cap should be used between wco_input pad and the mains 
* input, in the capacitive divider.
*
* \note 
* The 32.768 kHz WCO input must be a square wave, connected to the wco_out pin,
* with the wco_input pin grounded. 
*
*******************************************************************************/
void Cy_Rtc_SelectFrequencyPrescaler(cy_en_rtc_clock_freq_t clkSel)
{
    BACKUP->unCTL.u32Register = (_CLR_SET_FLD32U(BACKUP->unCTL.u32Register, BACKUP_CTL_PRESCALER, clkSel));
}

/*******************************************************************************
* Function Name: Rtc_ConvertDayOfWeek
****************************************************************************//**
*
* \brief  Returns a day of the week for a year, month, and day of month that are passed
*     through parameters. Zeller's congruence is used to calculate the day of
*     the week. 
*     RTC HW block does not provide the converting function for day of week. This 
*     function should be called before Cy_RTC_SetDateAndTime() to get the day of 
*     week.
*
*     For the Georgian calendar, Zeller's congruence is:
*     h = (q + [13 * (m + 1)] + K + [K/4] + [J/4] - 2J) mod 7
*
*     h - The day of the week (0 = Saturday, 1 = Sunday, 2 = Monday, ., 6 = Friday).
*     q - The day of the month.
*     m - The month (3 = March, 4 = April, 5 = May, ..., 14 = February)
*     K - The year of the century (year mod 100).
*     J - The zero-based century (actually [year/100]) For example, the zero-based
*     centuries for 1995 and 2000 are 19 and 20 respectively (not to be
*     confused with the common ordinal century enumeration which indicates
*     20th for both cases).

*
* \param  day   : The day of the month (1..31).
*         month : The month of the year, (1..12) see \ref group_rtc_month.
*         year  : The year value. Valid range - 2000...2100.
*
* \return Returns a day of the week, see \ref group_rtc_day_of_the_week.
*
* \note   In this algorithm January and February are counted as months 13 and 14
*         of the previous year.
*
*******************************************************************************/
uint32_t Cy_Rtc_ConvertDayOfWeek(uint32_t day, uint32_t month, uint32_t year)
{
    uint32_t retVal;

    /* Converts month number from regular convention
    * (1=January,..., 12=December) to convention required for this
    * algorithm (January and February are counted as months 13 and 14 of
    * previous year).
    */
    if(month < CY_RTC_MARCH)
    {
        month = CY_RTC_MONTHS_PER_YEAR + month;
        year--;
    }

    /* Calculates Day of Week using Zeller's congruence algorithms */
    retVal = 
    (day + (((month + 1ul) * 26ul) / 10ul) + year + (year / 4ul) + (6ul * (year / 100ul)) + (year / 400ul)) % 7ul;

    /* Makes correction for Saturday. Saturday number should be 7 instead of 0*/
    if(0u == retVal)
    {
        retVal = CY_RTC_SATURDAY;
    }

    return(retVal);
}

/*******************************************************************************
* Function Name: Rtc_IsLeapYear
****************************************************************************//**
*
* \breif  Checks whether the year passed through the parameter is leap or not.
*
*     This API is for checking an invalid value input for leap year.
*     RTC HW block does not provide a validation checker against time/date values, 
*     the valid range of days in Month should be checked before SetDateAndTime()
*     function call. Leap year is identified as a year that is a multiple of 4 
*     or 400 but not 100.
*
* \param  year : The year to be checked.
*
* \return false : The year is not leap;
*         true  : The year is leap.
*
*******************************************************************************/
bool Cy_Rtc_IsLeapYear(uint32_t year)
{
    return(0u != (((0u == (year % 4uL)) && (0u != (year % 100uL))) || (0u == (year % 400uL))));
}

/*******************************************************************************
* Function Name: Rtc_DaysInMonth
****************************************************************************//**
*
* \brief  Returns a number of days in a month passed through the parameters. This API
*         is for checking an invalid value input for days.
*         RTC HW block does not provide a validation checker against time/date values, 
*         the valid range of days in Month should be checked before SetDateAndTime()
*         function call.
*
* \param  month : The month of the year, see \ref group_rtc_month.
*         year  : A year value.
*
* \return A number of days in a month in the year passed through the parameters.
*
*******************************************************************************/
uint32_t Cy_Rtc_DaysInMonth(uint32_t month, uint32_t year)
{
    uint32_t retVal;
    retVal = cy_Rtc_daysInMonthTbl[month - 1ul];

    if(CY_RTC_FEBRUARY == month)
    {
        if(Cy_Rtc_IsLeapYear(year))
        {
            retVal++;
        }
    }
    return(retVal);
}

/*******************************************************************************
* Function Name: Rtc_SyncRegisters
****************************************************************************//**
*
* The Synchronizer updates RTC values into AHB RTC user registers from the 
* actual RTC. By calling this function, the actual RTC register values will be 
* copied to AHB user registers.
*
* \note Only after calling Rtc_SyncRegisters() the RTC time values can be 
* read.
* After Rtc_SyncRegisters() calling the snapshot of the actual RTC registers
* are copied to the user registers. Meanwhile the RTC is continuing to clock.
*
*******************************************************************************/
void Cy_Rtc_SyncRegisters(void)
{
    uint32_t interruptState;

    interruptState = Cy_SysLib_EnterCriticalSection(); 

    /* RTC Write is possible only in the condition that CY_RTC_BUSY bit = 0 
    *  or RTC Write bit is not set.
    */
    if((CY_RTC_BUSY != Cy_Rtc_GetSyncStatus()) && (!_FLD2BOOL(BACKUP_RTC_RW_WRITE, BACKUP->unRTC_RW.u32Register)))
    {
        /* Setting RTC Read bit */
        BACKUP->unRTC_RW.u32Register = BACKUP_RTC_RW_READ_Msk;

        /* Poll till the read bit is set */
        while(BACKUP->unRTC_RW.u32Register !=  BACKUP_RTC_RW_READ_Msk);

        /* Clearing RTC Read bit */
        BACKUP->unRTC_RW.u32Register = 0ul;
    }
    Cy_SysLib_ExitCriticalSection(interruptState);
}

/*******************************************************************************
* Function Name: Rtc_WriteEnable
****************************************************************************//**
*
* Set/Clear writeable option for RTC user registers. When the Write bit is set, 
* data can be written into the RTC user registers. After all the RTC writes are 
* done, the firmware must clear (call Rtc_WriteEnable(RTC_WRITE_DISABLED))
* the Write bit for the RTC update to take effect. 
*
* Set/Clear cannot be done if the RTC is still busy with a previous update 
* (RTC_BUSY = 1) or RTC Reading is executing.
*
* \param
* writeEnable see \ref group_rtc_write_status.
*
* \return
* en_result_t RET_SUCCESS - Set/Clear Write bit was successful;
* RET_CANCELED - RTC is busy with a previous update.
*
*******************************************************************************/
cy_en_rtc_status_t Cy_Rtc_WriteEnable(uint32_t writeEnable)
{
    cy_en_rtc_status_t retVal = CY_RTC_INVALID_STATE;

    if(writeEnable == CY_RTC_WRITE_ENABLED)
    {
        /* RTC Write bit set is possible only in condition that CY_RTC_BUSY bit = 0 
        * or RTC Read bit is not set
        */
        if((CY_RTC_BUSY != Cy_Rtc_GetSyncStatus()) && (!_FLD2BOOL(BACKUP_RTC_RW_READ, BACKUP->unRTC_RW.u32Register)))
        {
            BACKUP->unRTC_RW.u32Register |= BACKUP_RTC_RW_WRITE_Msk;
            retVal = CY_RTC_SUCCESS;
        }
    }
    else
    {
        /* Clearing Write Bit to complete write procedure */
        BACKUP->unRTC_RW.u32Register &= ((uint32_t) ~BACKUP_RTC_RW_WRITE_Msk);

        /* wait until CY_RTC_BUSY bit is cleared */
        while(CY_RTC_BUSY == Cy_Rtc_GetSyncStatus());
        
        retVal = CY_RTC_SUCCESS;
    }

    return(retVal);
}

/*******************************************************************************
* Function Name: Rtc_GetSyncStatus
****************************************************************************//**
*
* Return current status of CY_RTC_BUSY. The status indicates  
* synchronization between RTC user register and the actual RTC register. 
* CY_RTC_BUSY bit will be set if it is synchronizing. It is not possible to set 
* the Read or Write bit until CY_RTC_BUSY clears.
*
* \return
* The status of RTC user register synchronization, see
* \ref group_rtc_busy_status
*
*******************************************************************************/
uint32_t Cy_Rtc_GetSyncStatus(void)
{
    return((_FLD2BOOL(BACKUP_STATUS_RTC_BUSY, BACKUP->unSTATUS.u32Register)) ? CY_RTC_BUSY : CY_RTC_AVAILABLE);
}

/*******************************************************************************
* Function Name: Rtc_GetHoursFormat
****************************************************************************//**
*
* \brief  Returns current 12/24 hours mode.
*
* \param  None
*
* \return The current RTC hour format, see \ref group_rtc_hour_format.
*
* \note   Before getting the RTC current hour mode the Rtc_SyncRegisters() function
* 	  should be called.
*
*******************************************************************************/
uint32_t Cy_Rtc_GetHoursFormat(void)
{
    return((_FLD2BOOL(BACKUP_RTC_TIME_CTRL_12HR, BACKUP->unRTC_TIME.u32Register)) ? CY_RTC_12_HOURS : CY_RTC_24_HOURS);
}

/*******************************************************************************
* Function Name: Cy_RTC_IsExternalResetOccurred
****************************************************************************//**
*
* The function checks the reset cause and returns the Boolean result.
*
* \return
* True if the reset reason is the power cycle and the XRES (external reset) <br>
* False if the reset reason is other than power cycle and the XRES.
*
* \note Based on a return value the RTC time and date can be updated or skipped 
* after the device reset. For example, you should skip the 
* Cy_RTC_SetAlarmDateAndTime() call function if internal WDT reset occurs.
*
*******************************************************************************/
bool Cy_Rtc_IsExternalResetOccurred(void)
{
    uint32_t retReson = Cy_SysReset_GetResetReason();

    if((retReson & CY_SYSRESET_XRES) != 0ul)
    {
        return(true);
    }

    return(false);
}

/*******************************************************************************
* Function Name: Cy_Rtc_EnableDstTime
****************************************************************************//**
* 
* The function sets the DST time and configures the ALARM2 interrupt register 
* with the appropriate DST time. This function sets the DST stop time if the 
* current time is already in the DST period. The DST period is a period of time 
* between the DST start time and DST stop time. The DST start time and DST stop 
* time is presented in the DST configuration structure, 
* see \ref cy_str_rtc_dst_t.
*
* \param dstTime The DST configuration structure, see \ref cy_str_rtc_dst_t.
*
* \return cy_en_rtc_status_t A validation check result of RTC register update.
*
*******************************************************************************/
cy_en_rtc_status_t Cy_Rtc_EnableDstTime(cy_str_rtc_dst_t const *dstTime)
{
    cy_en_rtc_status_t retVal;
    
    if(Cy_Rtc_GetDstStatus(dstTime))
    {
        retVal = Cy_Rtc_SetNextDstTime(&dstTime->stopDst);
    }
    else
    {
        retVal = Cy_Rtc_SetNextDstTime(&dstTime->startDst);
    }

    Cy_Rtc_SetInterruptMask(CY_RTC_INTR_ALARM2);
    return retVal;
}


/*******************************************************************************
* Function Name: Cy_Rtc_SetNextDstTime
****************************************************************************//**
*
* Set the next time of the DST. This function sets the time to ALARM2 for a next
* DST event. If Cy_RTC_GetDSTStatus() is true(=1), the next DST event should be
* the DST stop, then this function should be called with the DST stop time. 
*
* If the time format(.format) is relative option(=0), the 
* Cy_RTC_RelativeToFixed() is called to convert to a fixed date. 
*
* \param
* nextDst - The structure with time at which a next DST event should occur 
* (ALARM2 interrupt should occur). See \ref cy_stc_rtc_config_t.
*
* \return cy_en_rtc_status_t A validation check result of RTC register update.
*
*******************************************************************************/
cy_en_rtc_status_t Cy_Rtc_SetNextDstTime(cy_str_rtc_dst_format_t const *nextDst)
{
    cy_en_rtc_status_t retVal = CY_RTC_BAD_PARAM;
    uint32_t tryesToSetup = CY_RTC_TRYES_TO_SETUP_DST;
    cy_stc_rtc_alarm_t dstAlarmTimeAndDate;

    /* Configure an alarm structure based on the DST structure */
    dstAlarmTimeAndDate.sec          = 0ul;
    dstAlarmTimeAndDate.sec_en       = CY_RTC_ALARM_ENABLE;
    dstAlarmTimeAndDate.min          = 0ul;
    dstAlarmTimeAndDate.min_en       = CY_RTC_ALARM_ENABLE;
    dstAlarmTimeAndDate.hour         = nextDst->hour;
    dstAlarmTimeAndDate.hour_en      = CY_RTC_ALARM_ENABLE;
    dstAlarmTimeAndDate.dayOfWeek_en = CY_RTC_ALARM_DISABLE;

    /* Calculate a day-of-month value for the relative DST start structure */
    if(CY_RTC_DST_FIXED != nextDst->format)
    {
        dstAlarmTimeAndDate.date = Cy_Rtc_RelativeToFixed(nextDst);
    }
    else
    {
        dstAlarmTimeAndDate.date = nextDst->dayOfMonth;
    }
    dstAlarmTimeAndDate.date_en  = CY_RTC_ALARM_ENABLE;
    dstAlarmTimeAndDate.month    = nextDst->month;
    dstAlarmTimeAndDate.month_en = CY_RTC_ALARM_ENABLE;
    dstAlarmTimeAndDate.alm_en   = CY_RTC_ALARM_ENABLE;

    while((retVal != CY_RTC_SUCCESS) && (0ul != tryesToSetup))
    {
        retVal = Cy_Rtc_SetAlarmDateAndTime(&dstAlarmTimeAndDate, CY_RTC_ALARM_2);
        --tryesToSetup;
    }

    if(tryesToSetup == 0ul)
    {
        retVal = CY_RTC_TIMEOUT;
    }
    return retVal;
}


/*******************************************************************************
* Function Name: Cy_Rtc_GetDstStatus
****************************************************************************//**
*
* Returns the current DST status using given time information. This function 
* is used in the initial state of a system. If the DST is enabled, the system 
* will set the DST start or stop as a result of this function.
*
* \param
*  dstTime The DST configuration structure, see \ref cy_str_rtc_dst_t.
*
* \return
* false - The current date and time is out of the DST period.
* true - The current date and time is in the DST period.
*
*******************************************************************************/
bool Cy_Rtc_GetDstStatus(cy_str_rtc_dst_t const *DstTime)
{
    cy_stc_rtc_config_t currentDateTime;
    uint32_t dstStartTime;
    uint32_t currentTime;
    uint32_t dstStopTime; 
    uint32_t dstStartDayOfMonth;
    uint32_t dstStopDayOfMonth;

    Cy_Rtc_GetDateAndTime(&currentDateTime);

    /* Calculate a day-of-month value for the relative DST start structure */
    if(CY_RTC_DST_RELATIVE != DstTime->startDst.format)
    {
        dstStartDayOfMonth = DstTime->startDst.dayOfMonth;
    }
    else
    {
        dstStartDayOfMonth = Cy_Rtc_RelativeToFixed(&DstTime->startDst);
    }

    /* Calculate the day of a month value for the relative DST stop structure */
    if(CY_RTC_DST_RELATIVE != DstTime->stopDst.format)
    {
        dstStopDayOfMonth = DstTime->stopDst.dayOfMonth;
    }
    else
    {
        dstStopDayOfMonth = Cy_Rtc_RelativeToFixed(&DstTime->stopDst);
    }

    /* The function forms the date and time values for the DST start time, 
    *  the DST Stop Time and for the Current Time. The function that compares 
    *  the three formed values returns "true" under condition that: 
    *  dstStartTime < currentTime < dstStopTime.
    *  The date and time value are formed this way:
    *  [13-10] - Month
    *  [9-5]   - Day of Month
    *  [0-4]   - Hour
    */
    dstStartTime = ((uint32_t) (DstTime->startDst.month << CY_RTC_DST_MONTH_POSITION) |
    (dstStartDayOfMonth << CY_RTC_DST_DAY_OF_MONTH_POSITION) | (DstTime->startDst.hour));

    currentTime = ((uint32_t) (currentDateTime.month << CY_RTC_DST_MONTH_POSITION) |
    (currentDateTime.date << CY_RTC_DST_DAY_OF_MONTH_POSITION) | (currentDateTime.hour));
    
    dstStopTime = ((uint32_t) (DstTime->stopDst.month << CY_RTC_DST_MONTH_POSITION) |
    (dstStopDayOfMonth << CY_RTC_DST_DAY_OF_MONTH_POSITION) | (DstTime->stopDst.hour));

    return((dstStartTime <= currentTime) && (dstStopTime > currentTime));
}


/*******************************************************************************
* Function Name: Cy_Rtc_Alarm1Interrupt
****************************************************************************//**
*
* This is a weak function and it should be redefined in user source code
* in condition that such event handler is required.
*
*******************************************************************************/
__WEAK void Cy_Rtc_Alarm1Interrupt(void)
{
    /* weak blank function */
}


/*******************************************************************************
* Function Name: Cy_Rtc_Alarm2Interrupt
****************************************************************************//**
*
* This is a weak function and it should be redefined in user source code
* in condition that such event handler is required.
*
*******************************************************************************/
__WEAK void Cy_Rtc_Alarm2Interrupt(void)
{
    /* weak blank function */
}


/*******************************************************************************
* Function Name: Cy_Rtc_DstInterrupt
****************************************************************************//**
* 
* This is a processing handler against the DST event. It adjusts the current 
* time using the DST start/stop parameters and registers the next DST event time
* into the ALARM2 interrupt.
* 
* \param
*  dstTime The DST configuration structure, see \ref cy_str_rtc_dst_t.
*
*******************************************************************************/
void Cy_Rtc_DstInterrupt(cy_str_rtc_dst_t const *dstTime)
{
    cy_stc_rtc_config_t curDateTime;

    Cy_Rtc_GetDateAndTime(&curDateTime);

    if(Cy_Rtc_GetDstStatus(dstTime))
    {
        /* Under condition that the DST start time was selected as 23:00, and 
        *  the time adjusting occurs, the other time and date values should be 
        *  corrected (day of the week, date, month and year).
        */
        if(curDateTime.hour > CY_RTC_MAX_HOURS_24H)
        {
            /* Incrementing day of the week value as hour adjusted next day of 
            *  the week and date. Correcting hour value as its incrementation 
            *  adjusted it out of valid range [0-23].
            */
            curDateTime.dayOfWeek++;
            curDateTime.hour = 0ul;

            /* Correct a day of the week if its incrementation adjusted it out 
            *  of valid range [1-7].
            */
            if(curDateTime.dayOfWeek > CY_RTC_SATURDAY)
            {
                curDateTime.dayOfWeek = CY_RTC_SUNDAY;
            }

            curDateTime.date++;

            /* Correct a day of a month if its incrementation adjusted it out of
            *  the valid range [1-31]. Increment month value.
            */
            if(curDateTime.date > Cy_Rtc_DaysInMonth(curDateTime.month, 
                                                    (curDateTime.year + CY_RTC_TWO_THOUSAND_YEARS)))
            {
               curDateTime.date = CY_RTC_FIRST_DAY_OF_MONTH;
               curDateTime.month++;
            }

            /* Correct a month if its incrementation adjusted it out of the 
            *  valid range [1-12]. Increment year value.
            */
            if(curDateTime.month > CY_RTC_MONTHS_PER_YEAR)
            {
                curDateTime.month = CY_RTC_JANUARY;
                curDateTime.year++;
            }
        }
        else
        {
            curDateTime.hour++;
        }
        
        (void) Cy_Rtc_SetDateAndTime(&curDateTime);
        (void) Cy_Rtc_SetNextDstTime(&dstTime->stopDst);
    }
    else
    {
        if(curDateTime.hour < 1ul)
        {
            /* Decrementing day of the week time and date values as hour 
            *  adjusted next day of the week and date. Correct hour value as 
            *  its incrementation adjusted it out of valid range [0-23]. 
            */
            curDateTime.hour = CY_RTC_MAX_HOURS_24H;
            curDateTime.dayOfWeek--;

            /* Correct a day of the week if its incrementation adjusted it out 
            *  of the valid range [1-7].
            */
            if(curDateTime.dayOfWeek < CY_RTC_SUNDAY)
            {
                curDateTime.dayOfWeek = CY_RTC_SUNDAY;
            }

            curDateTime.date--;

            /* Correct a day of a month value if its incrementation adjusted it
            *  out of the valid range [1-31]. Decrement month value.
            */
            if(curDateTime.date < CY_RTC_FIRST_DAY_OF_MONTH)
            {
               curDateTime.date = 
               Cy_Rtc_DaysInMonth(curDateTime.month, (curDateTime.year + CY_RTC_TWO_THOUSAND_YEARS));
               curDateTime.month--;
            }

            /* Correct a month if its increment pushed it out of the valid 
            *  range [1-12]. Decrement year value.
            */
            if(curDateTime.month < CY_RTC_JANUARY)
            {
                curDateTime.month = CY_RTC_DECEMBER;
                curDateTime.year--;
            }
        }
        else
        {
            curDateTime.hour--;
        }
        
        (void) Cy_Rtc_SetDateAndTime(&curDateTime);
        (void) Cy_Rtc_SetNextDstTime(&dstTime->startDst);
    }
}


/*******************************************************************************
* Function Name: Cy_Rtc_CenturyInterrupt
****************************************************************************//**
*
* This is a weak function and it should be redefined in user source code
* in condition that such event handler is required.
* By calling this function, it indicates the year reached 2100. It 
* should add an adjustment to avoid the Y2K problem.
*
*******************************************************************************/
__WEAK void Cy_Rtc_CenturyInterrupt(void)
{
    /* weak blank function */
}


/*******************************************************************************
* Function Name: Cy_Rtc_GetInterruptStatus
****************************************************************************//**
*
* Returns a status of RTC interrupt requests.
*
* \return Bit mapping information, see \ref group_rtc_macro_interrupts.
*
*******************************************************************************/
uint32_t Cy_Rtc_GetInterruptStatus(void)
{
    /* The RTC interrupt registers must be read twice to guarantee accuracy and
    *  only the second read contains valid data. The first read data cannot be 
    *  guaranteed as valid.
    *  Cypress ID #253805, #256202, #258057
    */
    (void) (BACKUP->unINTR.u32Register);
    return(BACKUP->unINTR.u32Register);
}


/*******************************************************************************
* Function Name: Cy_Rtc_GetInterruptStatusMasked
****************************************************************************//**
*
* Returns an interrupt request register masked by the interrupt mask. Returns a 
* result of the bitwise AND operation between the corresponding interrupt 
* request and mask bits.
*
* \return Bit mapping information, see \ref group_rtc_macro_interrupts.
*
*******************************************************************************/
uint32_t Cy_Rtc_GetInterruptStatusMasked(void)
{
    /* The RTC interrupt registers must be read twice to guarantee accuracy and
    *  only the second read contains valid data. The first read data cannot be 
    *  guaranteed as valid.
    *  Cypress ID #253805, #256202, #258057
    */
    (void) (BACKUP->unINTR_MASKED.u32Register);
    return(BACKUP->unINTR_MASKED.u32Register);
}


/*******************************************************************************
* Function Name: Cy_Rtc_GetInterruptMask
****************************************************************************//**
*
* Returns an interrupt mask.
*
* \return Bit mapping information, see \ref group_rtc_macro_interrupts.
*
*******************************************************************************/
uint32_t Cy_Rtc_GetInterruptMask(void)
{
    return (BACKUP->unINTR_MASK.u32Register);
}


/*******************************************************************************
* Function Name: Cy_Rtc_SetInterrupt
****************************************************************************//**
*
* Sets a software interrupt request
*
* \param
* interruptMask  Bit mask, see \ref group_rtc_macro_interrupts.
*
*******************************************************************************/
void Cy_Rtc_SetInterrupt(uint32_t interruptMask)
{
    BACKUP->unINTR_SET.u32Register = interruptMask;
}


/*******************************************************************************
* Function Name: Cy_Rtc_ClearInterrupt
****************************************************************************//**
*
* Clears RTC interrupts by setting each bit. 
*
* \param
* interruptMask The bit mask of interrupts to set,
* see \ref group_rtc_macro_interrupts.
*
*******************************************************************************/
void Cy_Rtc_ClearInterrupt(uint32_t interruptMask)
{
    BACKUP->unINTR.u32Register = interruptMask;

    (void) BACKUP->unINTR.u32Register;
}


/*******************************************************************************
* Function Name: Cy_Rtc_SetInterruptMask
****************************************************************************//**
*
* Configures which bits of the interrupt request register will trigger an 
* interrupt event.
*
* \param
* interruptMask The bit mask of interrupts to set,
* see \ref group_rtc_macro_interrupts.
*
*******************************************************************************/
void Cy_Rtc_SetInterruptMask(uint32_t interruptMask)
{
    BACKUP->unINTR_MASK.u32Register = interruptMask;
}


/*******************************************************************************
* Function Name: Cy_Rtc_Interrupt
****************************************************************************//**
*
* The interrupt handler function which should be called in user provided
* RTC interrupt function.
*
* This is the handler of the RTC interrupt in CPU NVIC. The handler checks 
* which RTC interrupt was asserted and calls the respective RTC interrupt 
* handler functions: Cy_RTC_Alarm1Interrupt(), Cy_RTC_Alarm2Interrupt() or 
* Cy_RTC_DstInterrupt(), and Cy_RTC_CenturyInterrupt().
* 
* The order of the RTC handler functions execution is incremental. 
* Cy_RTC_Alarm1Interrupt() is run as the first one and Cy_RTC_CenturyInterrupt()
* is called as the last one.
*
* This function clears the RTC interrupt every time when it is called.
*
* Cy_RTC_DstInterrupt() function is called instead of Cy_RTC_Alarm2Interrupt() 
* in condition that the mode parameter is true.
*
* \param
* dstTime The daylight saving time configuration structure, see 
* \ref cy_str_rtc_dst_t.
*
* \param mode false - if the DST is disabled, true - if DST is enabled.
*
* \note This function is required to be called in user interrupt handler.
*
*******************************************************************************/
void Cy_Rtc_Interrupt(cy_str_rtc_dst_t const *dstTime, bool mode)
{
    uint32_t interruptStatus;
    interruptStatus = Cy_Rtc_GetInterruptStatusMasked();

    Cy_Rtc_ClearInterrupt(interruptStatus);

    if(0ul != (CY_RTC_INTR_ALARM1 & interruptStatus))
    {
        Cy_Rtc_Alarm1Interrupt();
    }

    if(0ul != (CY_RTC_INTR_ALARM2 & interruptStatus))
    {
        if(mode)
        {
            Cy_Rtc_DstInterrupt(dstTime);
        }
        else
        {
            Cy_Rtc_Alarm2Interrupt();
        }
    }

    if(0ul != (CY_RTC_INTR_CENTURY & interruptStatus))
    {
        Cy_Rtc_CenturyInterrupt();
    }
}


/*******************************************************************************
* Function Name: Cy_Rtc_ConstructTimeDate
****************************************************************************//**
*
* Returns Integer time and Integer date in the format used in APIs from individual 
* elements passed.
* Converted Integer time(*Time) and Integer date(*date) are matched with RTC_TIME 
* and RTC_DATE bit fields format.
*
* \param
* timeDate The structure of time and date, see \ref cy_stc_rtc_config_t.
*
* \param time The Integer formatted time variable which is the same bit masks of 
* the RTC_TIME register:
* [0:6]   - Calendar seconds in Integer, range 0-59.
* [14:8]  - Calendar minutes in Integer, range 0-59.
* [21:16] - Calendar hours in Integer, value depending on the 12/24-hour mode.
* 12HR: [21]:0 = AM, 1 = PM, [20:16] = 1 - 12; 
* 24HR: [21:16] = 0-23;
* [22]    - Selects the 12/24-hour mode: 1 - 12-hour, 0 - 24-hour.
* [26:24] - A calendar day of the week, range 1 - 7, where 1 - Sunday.
*
* \param date The Integer formatted date variable which is the same bit masks of 
* the RTC_DATE register.
* [5:0]   - A calendar day of a month in Integer, range 1-31.
* [12:8]  - A calendar month in Integer, range 1-12.
* [23:16] - A calendar year in Integer, range 0-99.
*
*******************************************************************************/
static void Cy_Rtc_ConstructTimeDate(cy_stc_rtc_config_t const *timeDate, uint32_t *time, uint32_t *date)
{
    uint32_t tmpTime;
    uint32_t tmpDate;

    /* Prepare the RTC TIME value based on the structure obtained */
    tmpTime = (_VAL2FLD(BACKUP_RTC_TIME_RTC_SEC, (timeDate->sec)));
    tmpTime |= (_VAL2FLD(BACKUP_RTC_TIME_RTC_MIN, (timeDate->min)));

    /* Read the current hour mode to know how many hour bits to convert.
    *  In the 24-hour mode, the hour value is presented in [21:16] bits in the 
    *  Integer format.
    *  In the 12-hour mode, the hour value is presented in [20:16] bits in the 
    *  Integer format and
    *  bit [21] is present: 0 - AM; 1 - PM.
    */
    if(timeDate->hrMode != CY_RTC_24_HOURS)
    {
        if(CY_RTC_AM != timeDate->amPm)
        {
            /* Set the PM bit */
            tmpTime |= CY_RTC_BACKUP_RTC_TIME_RTC_PM;
        }
        else
        {
            /* Set the AM bit */
            tmpTime &= ((uint32_t) ~CY_RTC_BACKUP_RTC_TIME_RTC_PM); 
        }
        tmpTime |= BACKUP_RTC_TIME_CTRL_12HR_Msk;
        tmpTime |= (_VAL2FLD(BACKUP_RTC_TIME_RTC_HOUR, ((timeDate->hour) & ((uint32_t) ~CY_RTC_12HRS_PM_BIT))));
    }
    else
    {
        tmpTime &= ((uint32_t) ~BACKUP_RTC_TIME_CTRL_12HR_Msk);
        tmpTime |= (_VAL2FLD(BACKUP_RTC_TIME_RTC_HOUR, (timeDate->hour)));
    }
    tmpTime |= (_VAL2FLD(BACKUP_RTC_TIME_RTC_DAY, (timeDate->dayOfWeek)));

    /* Prepare the RTC Date value based on the structure obtained */
    tmpDate  = (_VAL2FLD(BACKUP_RTC_DATE_RTC_DATE,(timeDate->date)));
    tmpDate |= (_VAL2FLD(BACKUP_RTC_DATE_RTC_MON, (timeDate->month)));
    tmpDate |= (_VAL2FLD(BACKUP_RTC_DATE_RTC_YEAR, (timeDate->year)));

    /* Update the parameter values with prepared values */
    *time = tmpTime;
    *date = tmpDate;
}


/*******************************************************************************
* Function Name: Cy_Rtc_ConstructAlarmTimeDate
****************************************************************************//**
*
* Returns the Integer time and Integer date in the format used in APIs from individual 
* elements passed for alarm.
* Converted Integer time(*alarmTime) and Integer date(*alarmDate) should be 
* matched with the ALMx_TIME and ALMx_DATE bit fields format.
*
* \param
* timeDate The structure of time and date, see \ref cy_stc_rtc_alarm_t.
*
* \param
* alarmTime Integer formatted time variable which is same bit masks of 
* ALMx_TIME register time fields:
* [0:6]   - Alarm seconds in Integer, range 0-59.
* [7]     - Alarm seconds Enable: 0 - ignore, 1 - match.
* [14:8]  - Alarm minutes in Integer, range 0-59.
* [15]    - Alarm minutes Enable: 0 - ignore, 1 - match.
* [21:16] - Alarm hours in Integer, value depending on the 12/24-hour mode 
* (RTC_CTRL_12HR)
* 12HR: [21]:0 = AM, 1 = PM, [20:16] = 1 - 12; 
* 24HR: [21:16] = 0-23;
* [23]    - Alarm hours Enable: 0 - ignore, 1 - match;
* [26:24] - An alarm day of the week, range 1 - 7, where 1 - Monday.
* [31]    - An alarm day of the week Enable: 0 - ignore, 1 - match.
*
* \param alarmDate The Integer formatted date variable which is the same bit 
* masks of the ALMx_DATE register date fields:
* [5:0]  - An alarm day of a month in Integer, range 1-31.
* [7]    - An alarm day of a month Enable: 0 - ignore, 1 - match.
* [12:8] - An alarm month in Integer, range 1-12.
* [15]   - An alarm month Enable: 0 - ignore, 1 - match.
* [31]   - The Enable alarm: 0 - Alarm is disabled, 1 - Alarm is enabled.
*
* This function reads current AHB register RTC_TIME value to know hour mode.
* It is recommended to call Cy_RTC_SyncRegisters() function before calling the 
* Cy_RTC_ConstructAlarmTimeDate() functions.
* 
* Construction is based on RTC_ALARM1 register bit fields.
*
*******************************************************************************/
static void Cy_Rtc_ConstructAlarmTimeDate(cy_stc_rtc_alarm_t const *alarmDateTime, uint32_t *alarmTime,
                                                                             uint32_t *alarmDate)
{
    uint32_t tmpAlarmTime;
    uint32_t tmpAlarmDate;
    uint32_t hourValue;

    /* Prepare the RTC ALARM value based on the structure obtained */
    tmpAlarmTime  = (_VAL2FLD(BACKUP_ALM1_TIME_ALM_SEC, (alarmDateTime->sec)));
    tmpAlarmTime |= (_VAL2FLD(BACKUP_ALM1_TIME_ALM_SEC_EN, alarmDateTime->sec_en));
    tmpAlarmTime |= (_VAL2FLD(BACKUP_ALM1_TIME_ALM_MIN, (alarmDateTime->min)));
    tmpAlarmTime |= (_VAL2FLD(BACKUP_ALM1_TIME_ALM_MIN_EN, alarmDateTime->min_en));

    /* Read the current hour mode to know how many hour bits to convert.
    *  In the 24-hour mode, the hour value is presented in [21:16] bits in the 
    *  Integer format.
    *  In the 12-hour mode, the hour value is presented in [20:16] bits in the 
    *  Integer format and bit [21] is present: 0 - AM; 1 - PM
    */
    Cy_Rtc_SyncRegisters();
    if(CY_RTC_24_HOURS != Cy_Rtc_GetHoursFormat())
    {
        /* Convert the hour from the 24-hour mode into the 12-hour mode */
        if(alarmDateTime->hour >= CY_RTC_HOURS_PER_HALF_DAY)
        {
            /* The current hour is more than 12 in the 24-hour mode. Set the PM 
            *  bit and converting hour: hour = hour - 12
            */
            hourValue = (uint32_t) alarmDateTime->hour - CY_RTC_HOURS_PER_HALF_DAY;
            hourValue = ((0ul != hourValue) ? hourValue : CY_RTC_HOURS_PER_HALF_DAY);
            tmpAlarmTime |= 
            CY_RTC_BACKUP_RTC_TIME_RTC_PM | (_VAL2FLD(BACKUP_ALM1_TIME_ALM_HOUR, (hourValue)));
        }
        else if(alarmDateTime->hour < 1ul)
        {
            /* The current hour in the 24-hour mode is 0 which is equal to 12:00 AM */
            tmpAlarmTime = (tmpAlarmTime & ((uint32_t) ~CY_RTC_BACKUP_RTC_TIME_RTC_PM)) | 
            (_VAL2FLD(BACKUP_ALM1_TIME_ALM_HOUR, CY_RTC_HOURS_PER_HALF_DAY));
        }
        else
        {
            /* The current hour is less than 12. Set the AM bit */
            tmpAlarmTime = (tmpAlarmTime & ((uint32_t) ~CY_RTC_BACKUP_RTC_TIME_RTC_PM)) |
            (_VAL2FLD(BACKUP_ALM1_TIME_ALM_HOUR, (alarmDateTime->hour)));
        }
        tmpAlarmTime |= BACKUP_RTC_TIME_CTRL_12HR_Msk;
    }
    else
    {
        tmpAlarmTime |= (_VAL2FLD(BACKUP_ALM1_TIME_ALM_HOUR, (alarmDateTime->hour)));
        tmpAlarmTime &= ((uint32_t) ~BACKUP_RTC_TIME_CTRL_12HR_Msk);
    }
    tmpAlarmTime |= (_VAL2FLD(BACKUP_ALM1_TIME_ALM_HOUR_EN, alarmDateTime->hour_en));
    tmpAlarmTime |= (_VAL2FLD(BACKUP_ALM1_TIME_ALM_DAY, (alarmDateTime->dayOfWeek)));
    tmpAlarmTime |= (_VAL2FLD(BACKUP_ALM1_TIME_ALM_DAY_EN, alarmDateTime->dayOfWeek_en));

    /* Prepare the RTC ALARM DATE value based on the obtained structure */
    tmpAlarmDate  = (_VAL2FLD(BACKUP_ALM1_DATE_ALM_DATE, (alarmDateTime->date)));
    tmpAlarmDate |= (_VAL2FLD(BACKUP_ALM1_DATE_ALM_DATE_EN, alarmDateTime->date_en));
    tmpAlarmDate |= (_VAL2FLD(BACKUP_ALM1_DATE_ALM_MON, (alarmDateTime->month)));
    tmpAlarmDate |= (_VAL2FLD(BACKUP_ALM1_DATE_ALM_MON_EN, alarmDateTime->month_en));
    tmpAlarmDate |= (_VAL2FLD(BACKUP_ALM1_DATE_ALM_EN, alarmDateTime->alm_en));

    /* Update the parameter values with prepared values */
    *alarmTime = tmpAlarmTime;
    *alarmDate = tmpAlarmDate;
}


/*******************************************************************************
* Function Name: Cy_Rtc_RelativeToFixed
****************************************************************************//**
*
* Converts time from a relative format to a fixed format to set the ALARM2.
*
* \param
* convertDST - The DST structure, its appropriate elements should be converted.
*
* \return
* The current date of a month.
*
*******************************************************************************/
static uint32_t Cy_Rtc_RelativeToFixed(cy_str_rtc_dst_format_t const *convertDst)
{
    uint32_t currentYear;
    uint32_t currentDay;
    uint32_t currentWeek;
    uint32_t daysInMonth;
    uint32_t tmpDayOfMonth;

    /* Read the current year */
    Cy_Rtc_SyncRegisters();

    currentYear = 
    CY_RTC_TWO_THOUSAND_YEARS + (_FLD2VAL(BACKUP_RTC_DATE_RTC_YEAR, BACKUP->unRTC_DATE.u32Register));

    currentDay  = CY_RTC_FIRST_DAY_OF_MONTH;
    currentWeek = CY_RTC_FIRST_WEEK_OF_MONTH;
    daysInMonth = Cy_Rtc_DaysInMonth(convertDst->month, currentYear);
    tmpDayOfMonth = currentDay;

    while((currentWeek <= convertDst->weekOfMonth) && (currentDay <= daysInMonth))
    {
        if(convertDst->dayOfWeek == Cy_Rtc_ConvertDayOfWeek(currentDay, convertDst->month, currentYear))
        {
            tmpDayOfMonth = currentDay;
            currentWeek++;
        }
        currentDay++;
    }
    return(tmpDayOfMonth);
}

#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
