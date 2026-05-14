/***************************************************************************//**
* \file cy_aic261.h
* \version 1.0
*
* \brief
* Provides an API for audio codec (ADC/DAC) TI aic 261 
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_AIC261_H
#define CY_AIC261_H

typedef union 
{
    uint16_t u16;
    struct
    {
        uint16_t reserved: 5;
        uint16_t pg_addr:  10;
        uint16_t wr:       1;
    } bitField;
} cy_un_AIC261_Command;

#define AIC261_MCLK_CONST_DIV_NUM     (128ul)

/***Page1: REGISTER 00H: ADC Control***/
typedef union
{
    uint16_t u16;
    struct
    {
        uint16_t AVGFS : 1; //Average Filter select
        uint16_t RESV0 : 3; //Reserved
        uint16_t ADCR  : 2; //Conversion Rate Control.
        uint16_t ADAVG : 2; //Converter Averaging Control.
        uint16_t RESOL : 2; //Resolution Control.
        uint16_t ADSCM : 4; //A/D Scan Mode.
        uint16_t ADST  : 1; //A/D status
        uint16_t RESV1 : 1; //Reserved.
    }bitField;
} cy_un_AIC261_ADCControl;

/**** Page1:REGISTER 03H: Reference Control ****/
typedef union
{
    uint16_t u16;
    struct
    {
        uint16_t IREFV  : 1;  // Internal Reference Voltage.
        uint16_t RPWDN  : 1;  // Reference Power Down.
        uint16_t RPWUDL : 2;  // Reference Power Up Delay.
        uint16_t VREFM  : 1;  // Voltage Reference Mode.
        uint16_t RESV0  : 11; // Reserved.
    }bitField;
} cy_un_AIC261_RefferenceControl;

/**** Page2:REGISTER 00H: Audio Control_1 ****/
typedef union
{
    uint16_t u16;
    struct
    {
        uint16_t ADCFS  : 3; //ADC Sampling Rate
        uint16_t DACFS  : 3; //DAC Sampling Rate
        uint16_t RESV0  : 2; //Reserved
        uint16_t DATFM  : 2; //Digital Data Format
        uint16_t WLEN   : 2; //Codec Word Length.
        uint16_t ADCIN  : 2; //ADC Input Mux
        uint16_t ADCHPF : 2; //ADC High Pass Filter
    }bitField;
} cy_un_AIC261_AudioControl_1;

/**** REGISTER 01H: CODEC ADC Gain Control ****/
typedef union
{
    uint16_t u16;
    struct
    {
        uint16_t AGCEN : 1; //AGC Enable
        uint16_t AGCTC : 4; //AGC Time Constant
        uint16_t AGCTG : 3; //AGC Target Level
        uint16_t ADPGA : 7; //ADC PGA Settings
        uint16_t ADMUT : 1; //ADC Channel Mute
    }bitField;
} cy_un_AIC261_ADCGainControl;

/**** REGISTER 02H: CODEC DAC Gain Control ****/
typedef union
{
    uint16_t u16;
    struct
    {
        uint16_t DARVL : 7; //DAC Right Channel Muted
        uint16_t DARMU : 1; //DAC Right Channel Muted
        uint16_t DALVL : 7; //DAC Left Channel Muted
        uint16_t DALMU : 1; //DAC Left Channel Muted
    }bitField;
} cy_un_AIC261_DACGainControl;

/**** REGISTER 05H: CODEC Power Control ****/
typedef union
{
    uint16_t u16;
    struct
    {
        uint16_t DEEMPF : 1; //De_Emphasis Filter Enable
        uint16_t EFFCTL : 1; //Digital Audio Effects Filter Control
        uint16_t RESV0  : 2; //Reserved. Write only 00 into this location.
        uint16_t VBIAS  : 1; //VBIAS Voltage
        uint16_t ADWSF  : 1; //ADWS Pin Function
        uint16_t DAPWDF : 1; //DAC Power-Down Flag
        uint16_t ADPWDF : 1; //ADC Power-Down Flag
        uint16_t VGPWDN : 1; //Driver Virtual Ground Power Down
        uint16_t ADPWDN : 1; //ADC Power-Down Control
        uint16_t DAPWDN : 1; //DAC Power-Down Control
        uint16_t ASTPWF : 1; //Analog Sidetone Power-Down Flag
        uint16_t DAODRC : 1; //Audio Output Driver Control
        uint16_t ASTPWD : 1; //Analog Sidetone Power-down Control
        uint16_t RESV1  : 1; //Reserved (During read the value of this bit is 0. Write only 0 into this location.)
        uint16_t PWDNC  : 1; //Codec Power-Down Control
    }bitField;
} cy_un_AIC261_DACPowerControl;

/**** REGISTER 06H: Audio Control 3 ****/
typedef union
{
    uint16_t u16;
    struct
    {
        uint16_t REVID   : 3; //Reserved
        uint16_t CLPST   : 1; //AGC Clip Stepping Enable
        uint16_t AGCNL   : 2; //AGC Noise Threshold.
        uint16_t DAROVF  : 1; //DAC Right Channel Overflow Flag ( Read Only )
        uint16_t DALOVF  : 1; //DAC Left Channel Overflow Flag ( Read Only )
        uint16_t ADCOVF  : 1; //ADC Channel Overflow Flag ( Read Only )
        uint16_t DAPK2PK : 2; //DAC Max Output Signal Swing and Common Mode Voltage
        uint16_t SLVMS   : 1; //Codec Master Slave Selection
        uint16_t DAXFM   : 1; //Master Transfer Mode
        uint16_t REFFS   : 1; //Reference Sampling Rate.
        uint16_t DMSVOL  : 2; //DAC Channel Master Volume Control
    }bitField;
} cy_un_AIC261_AudioControl3;

/**** REGISTER 1BH: PLL Programmability ****/
typedef union
{
    uint16_t u16;
    struct
    {        
        uint16_t RESV0  : 2; //Reserved (write only 00)
        uint16_t JVAL   : 6; //J value. Valid only if PLL is enabled.
        uint16_t PVAL   : 3; //P value. Valid when PLL is enabled
        uint16_t QVAL   : 4; //Q value. Valid only if PLL is disabled.
        uint16_t PLLSEL : 1; //PLL Enable
    }bitField;
} cy_un_AIC261_PLL_Programmability;

/**** Register 1CH: PLL2 ***/
typedef union
{
    uint16_t u16;
    struct
    {
        uint16_t RESV0 : 2;  //Reserved (write only 00)
        uint16_t DVAL  : 14; //D value
    }bitField;
} cy_un_AIC261_PLL_Programmability2;

/**** REGISTER 1DH: Audio Control 4 ****/
typedef union
{
    uint16_t u16;
    struct
    {
        uint16_t PGID             : 2; //Read only
        uint16_t DAC_POP_RED_SET2 : 2; //DAC POP Reduction Setting 2
        uint16_t DAC_POP_RED_SET1 : 1; //DAC POP Reduction Setting 1
        uint16_t DAC_POP_RED      : 1; //DAC POP Reduction Enable
        uint16_t SHCKT_FLAG       : 1; //Short Circuit Detected Flag
        uint16_t SHCKT_PD         : 1; //Power Down Drivers if Short Circuit Detected
        uint16_t SHCKT_DIS        : 1; //Disable Short Circuit Detection
        uint16_t AGC_HYST         : 2; //AGC Hysteresis Control
        uint16_t Reserved         : 1; //Reserved
        uint16_t DSTPD            : 1; //Digital Sidetone Zero Cross Control
        uint16_t ASSTPD           : 1; //Analog Sidetone Soft-Stepping Control
        uint16_t DASTPD           : 1; //DAC PGA Soft-Stepping Control
        uint16_t ASTPD            : 1; //ADC PGA Soft-Stepping Control
   }bitField;
} cy_un_AIC261_AudioControl4;

/**** REGISTER 1EH: Audio Control 5 ****/
typedef union
{
    uint16_t u16;
    struct
    {
        uint16_t RESV0       : 1; // Reserved. Do not write 1 to this location.
        uint16_t DRV_POP_LEN : 1; //Audio Output Driver POP Reduction Duration
        uint16_t DRV_POP_DIS : 1; //Audio Output Driver POP Reduction Enable
        uint16_t AGC_SIG_DEB : 3; //AGC Debounce Time for Silence Mode to Speech Mode Transition
        uint16_t AGC_NOI_DEB : 3; //AGC Debounce Time for Speech Mode to Silence Mode Transition
        uint16_t MAX_AGC_PGA : 7; //MAX Input Gain Applicable for AGC
    }bitField;
} cy_un_AIC261_AudioControl5;

typedef enum
{
    CY_AIC26_CTROL_WRITE = (0u),
    CY_AIC26_CTROL_READ  = (1u),
} cy_en_AIC261_WR;

typedef enum
{
    CY_PAGE0_BAT1                   = (0x05),
    CY_PAGE0_BAT2                   = (0x06),
    CY_PAGE0_AUX                    = (0x07),
    CY_PAGE0_TEMP1                  = (0x09),
    CY_PAGE0_TEMP2                  = (0x0A),
    CY_PAGE1_ADC_CTL                = (0x40),
    CY_PAGE1_STATUS                 = (0x41),
    CY_PAGE1_REFERENCE              = (0x43),
    CY_PAGE1_RESET                  = (0x44),
    CY_PAGE2_AUDIO_CONTROL_1        = (0x80),
    CY_PAGE2_CODEC_ADC_GAIN         = (0x81),
    CY_PAGE2_CODEC_DAC_GAIN         = (0x82),
    CY_PAGE2_CODEC_SIDETONE         = (0x83),
    CY_PAGE2_AUDIO_CONTROL_2        = (0x84),
    CY_PAGE2_CODEC_POWER_CONTROL    = (0x85),
    CY_PAGE2_AUDIO_CONTROL_3        = (0x86),
    CY_PAGE2_FILTER_COEFFICIENTS_0  = (0x87),
    CY_PAGE2_FILTER_COEFFICIENTS_1  = (0x88),
    CY_PAGE2_FILTER_COEFFICIENTS_2  = (0x89),
    CY_PAGE2_FILTER_COEFFICIENTS_3  = (0x8A),
    CY_PAGE2_FILTER_COEFFICIENTS_4  = (0x8B),
    CY_PAGE2_FILTER_COEFFICIENTS_5  = (0x8C),
    CY_PAGE2_FILTER_COEFFICIENTS_6  = (0x8D),
    CY_PAGE2_FILTER_COEFFICIENTS_7  = (0x8E),
    CY_PAGE2_FILTER_COEFFICIENTS_8  = (0x8F),
    CY_PAGE2_FILTER_COEFFICIENTS_9  = (0x90),
    CY_PAGE2_FILTER_COEFFICIENTS_10 = (0x91),
    CY_PAGE2_FILTER_COEFFICIENTS_11 = (0x92),
    CY_PAGE2_FILTER_COEFFICIENTS_12 = (0x93),
    CY_PAGE2_FILTER_COEFFICIENTS_13 = (0x94),
    CY_PAGE2_FILTER_COEFFICIENTS_14 = (0x95),
    CY_PAGE2_FILTER_COEFFICIENTS_15 = (0x96),
    CY_PAGE2_FILTER_COEFFICIENTS_16 = (0x97),
    CY_PAGE2_FILTER_COEFFICIENTS_17 = (0x98),
    CY_PAGE2_FILTER_COEFFICIENTS_18 = (0x99),
    CY_PAGE2_FILTER_COEFFICIENTS_19 = (0x9A),
    CY_PAGE2_PLL_PROGRAMMABILITY_0  = (0x9B),
    CY_PAGE2_PLL_PROGRAMMABILITY_1  = (0x9C),
    CY_PAGE2_AUDIO_CONTROL_4        = (0x9D),
    CY_PAGE2_AUDIO_CONTROL_5        = (0x9E),
} cy_en_AIC26_Registers;

extern void Cy_AIC261_InitSPI(void);
extern void Cy_AIC261_WriteCtlReg(cy_en_AIC26_Registers regName, uint16_t data);
extern uint16_t Cy_AIC261_ReadCtlReg(cy_en_AIC26_Registers regName);

#endif
