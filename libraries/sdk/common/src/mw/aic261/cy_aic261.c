/***************************************************************************//**
* \file cy_aic261.c
* \version 1.0
*
* \brief
* Provides an API implementation of the Button middleware driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


/*****************************************************************************
* Include files
*****************************************************************************/
#include "cy_project.h"
#include "cy_device_headers.h"
#include "cy_aic261.h"


#if defined(__cplusplus)
extern "C" {
#endif

#define AIC261_SPI_BAUDRATE     125000ul                                /* Please set baudrate value of SPI you want */
#define AIC261_SPI_OVERSAMPLING 16ul                                    /* Please set oversampling of SPI you want */
#define AIC261_SPI_FREQ_IN_HZ   (AIC261_SPI_BAUDRATE*AIC261_SPI_OVERSAMPLING)

static const cy_stc_scb_spi_config_t SCB_SPI_cfg =
{
    .spiMode                    = CY_SCB_SPI_MASTER,        /*** Specifies the mode of operation    */     
    .subMode                    = CY_SCB_SPI_MOTOROLA,      /*** Specifies the submode of SPI operation    */
    .sclkMode                   = CY_SCB_SPI_CPHA1_CPOL0,   /*** When clock is inactive, the level is low. Data signal is sampled on first edge*/
    .oversample                 = AIC261_SPI_OVERSAMPLING, 
    .rxDataWidth                = 16ul,
    .txDataWidth                = 16ul,
    .enableMsbFirst             = true,
    .enableFreeRunSclk          = false,
    .enableInputFilter          = false,
    .enableMisoLateSample       = true,
    .enableTransferSeperation   = false,
    .ssPolarity0                = false,
    .ssPolarity1                = false,
    .ssPolarity2                = false,
    .ssPolarity3                = false,
    .enableWakeFromSleep        = false,
    .rxFifoTriggerLevel         = 0ul,
    .rxFifoIntEnableMask        = 0ul,
    .txFifoTriggerLevel         = 0ul,
    .txFifoIntEnableMask        = 0ul,
    .masterSlaveIntEnableMask   = 0ul,
    .enableSpiDoneInterrupt     = false,
    .enableSpiBusErrorInterrupt = false,
};

void Cy_AIC261_InitSPI(void)
{
    /**************************************/
    /* Port Setting for SPI communication */
    /**************************************/
    static cy_stc_gpio_pin_config_t aic261_spiPinCfg = { 0ul };

    /* SCB MISO*/
    aic261_spiPinCfg.driveMode = CY_GPIO_DM_HIGHZ;
    aic261_spiPinCfg.hsiom = CY_AUDIOSS_SPI_MISO_PIN_MUX;
    Cy_GPIO_Pin_Init(CY_AUDIOSS_SPI_MISO_PORT, CY_AUDIOSS_SPI_MISO_PIN, &aic261_spiPinCfg);
    
    /* SCB MOSI*/
    aic261_spiPinCfg.driveMode = CY_GPIO_DM_STRONG_IN_OFF;
    aic261_spiPinCfg.hsiom = CY_AUDIOSS_SPI_MOSI_PIN_MUX;
    Cy_GPIO_Pin_Init(CY_AUDIOSS_SPI_MOSI_PORT, CY_AUDIOSS_SPI_MOSI_PIN, &aic261_spiPinCfg);
    
    /* SCB CLK*/
    aic261_spiPinCfg.driveMode = CY_GPIO_DM_STRONG_IN_OFF;
    aic261_spiPinCfg.hsiom = CY_AUDIOSS_SPI_CLK_PIN_MUX;
    Cy_GPIO_Pin_Init(CY_AUDIOSS_SPI_CLK_PORT, CY_AUDIOSS_SPI_CLK_PIN, &aic261_spiPinCfg);
    
    /* SCB SEL*/
    aic261_spiPinCfg.driveMode = CY_GPIO_DM_STRONG_IN_OFF;
    aic261_spiPinCfg.hsiom = CY_AUDIOSS_SPI_SS_PIN_MUX;
    Cy_GPIO_Pin_Init(CY_AUDIOSS_SPI_SS_PORT, CY_AUDIOSS_SPI_SS_PIN, &aic261_spiPinCfg);

    /**************************************/
    /*     SPI Clock Divider Settings     */
    /**************************************/
    cy_en_divider_types_t divType = CY_SYSCLK_DIV_24_5_BIT;
    uint8_t               divNum  = PERI_PERI_PCLK_PCLK_GROUP_NR1_GR_DIV_24_5_VECT - 2u; // Use second to last 24.5 fractional divider

    Cy_SysClk_PeriphAssignDivider(CY_AUDIOSS_SPI_PCLK, divType, divNum);
    Cy_SysClk_PeriphSetFracDividerAuto(Cy_SysClk_GetClockGroup(CY_AUDIOSS_SPI_PCLK), divType, divNum, AIC261_SPI_FREQ_IN_HZ, NULL);
    Cy_SysClk_PeriphEnableDivider     (Cy_SysClk_GetClockGroup(CY_AUDIOSS_SPI_PCLK), divType, divNum);

    /*******************************/
    /* SPI Instance Initialization */
    /*******************************/
    Cy_SCB_SPI_Init(CY_AUDIOSS_SPI_TYPE, &SCB_SPI_cfg, NULL);
    Cy_SCB_SPI_SetActiveSlaveSelect(CY_AUDIOSS_SPI_TYPE,CY_AUDIOSS_SPI_SS);
    Cy_SCB_SPI_Enable(CY_AUDIOSS_SPI_TYPE);
}

static void Cy_AIC261_FlushRxFifo(void)
{
    /* Flush RX FIFO */
    uint16_t dummy;
    while(Cy_SCB_SPI_GetNumInRxFifo(CY_AUDIOSS_SPI_TYPE) != 0ul)
    {
        Cy_SCB_SPI_ReadArray(CY_AUDIOSS_SPI_TYPE, &dummy, 1ul);
    }
}

void Cy_AIC261_WriteCtlReg(cy_en_AIC26_Registers regName, uint16_t data)
{
    cy_un_AIC261_Command command = { .u16 = 0u };
    command.bitField.pg_addr  = regName;
    command.bitField.wr       = CY_AIC26_CTROL_WRITE;

    uint16_t txData[2];
    txData[0] = command.u16;
    txData[1] = data;

    /* Make sure no Rx Fifo here */
    Cy_AIC261_FlushRxFifo();

    /* Send command and register data */
    Cy_SCB_SPI_WriteArray(CY_AUDIOSS_SPI_TYPE,(void*)txData, 2ul);

    /* Wait for TX completion */
    while(Cy_SCB_SPI_IsTxComplete(CY_AUDIOSS_SPI_TYPE) == false);

    /* Wait for RX completion */
    while(Cy_SCB_SPI_GetNumInRxFifo(CY_AUDIOSS_SPI_TYPE) < 2ul);

    /* Make sure no Rx Fifo here */
    Cy_AIC261_FlushRxFifo();

    return;
}

uint16_t Cy_AIC261_ReadCtlReg(cy_en_AIC26_Registers regName)
{
    /* Make sure no Rx Fifo here */
    Cy_AIC261_FlushRxFifo();

    /* Send command */
    cy_un_AIC261_Command command = {0u};
    command.bitField.pg_addr  = regName;
    command.bitField.wr       = CY_AIC26_CTROL_READ;

    uint16_t txData[2] = {0};
    txData[0] = command.u16;
    Cy_SCB_SPI_WriteArray(CY_AUDIOSS_SPI_TYPE,(void*)txData, 2ul);

    /* Wait for TX completion */
    while(Cy_SCB_SPI_IsTxComplete(CY_AUDIOSS_SPI_TYPE) == false);

    /* Wait for RX completion */
    while(Cy_SCB_SPI_GetNumInRxFifo(CY_AUDIOSS_SPI_TYPE) < 2ul);

    /* Receive the response */
    uint16_t rxData[2] = {0};
    Cy_SCB_SPI_ReadArray(CY_AUDIOSS_SPI_TYPE, (void*)rxData, 2ul);

    return rxData[1];
}


#if defined(__cplusplus)
}
#endif

