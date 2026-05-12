/***************************************************************************//**
* \file cy_sysclk.c
* \version 1.0
*
* Provides an API implementation of the sysclk driver.
*
********************************************************************************
* \copyright
* Copyright 2016, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_sysclk.h"
#include "syslib/cy_syslib.h"

#define M_PI (3.1415927f)

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/* # of elements in an array */
#define  CY_SYSCLK_N_ELMTS(a)  (sizeof(a) / sizeof((a)[0]))

const uint8_t divNumLocator[PERI_PERI_PCLK_PCLK_GROUP_NR][4] =
{
#if PERI_PERI_PCLK_PCLK_GROUP_NR > 0
    {PERI_PERI_PCLK_PCLK_GROUP_NR0_GR_DIV_8_VECT, PERI_PERI_PCLK_PCLK_GROUP_NR0_GR_DIV_16_VECT, PERI_PERI_PCLK_PCLK_GROUP_NR0_GR_DIV_16_5_VECT, PERI_PERI_PCLK_PCLK_GROUP_NR0_GR_DIV_24_5_VECT},
#endif
#if PERI_PERI_PCLK_PCLK_GROUP_NR > 1
    {PERI_PERI_PCLK_PCLK_GROUP_NR1_GR_DIV_8_VECT, PERI_PERI_PCLK_PCLK_GROUP_NR1_GR_DIV_16_VECT, PERI_PERI_PCLK_PCLK_GROUP_NR1_GR_DIV_16_5_VECT, PERI_PERI_PCLK_PCLK_GROUP_NR1_GR_DIV_24_5_VECT},
#endif
};

/* ========================================================================== */
/* ===========================    ECO SECTION    ============================ */
/* ========================================================================== */
/**
* \addtogroup group_sysclk_eco_funcs
* \{
*/

/*******************************************************************************
* Function Name: Cy_SysClk_EcoConfigureWithMinRneg
****************************************************************************//**
*
* This function configures the external crystal oscillator (ECO) trim bits based
* on crystal characteristics. This function should be called only when the ECO is
* disabled.
*
* \param freq Operating frequency of the crystal in Hz.
*
* \param cSum Amount of Crystal load capacitance and shunt capacitance in pF. 
*
* \param esr Effective series resistance of the crystal in ohms.
*
* \param driveLevel Crystal drive level in uW.
*
* \param minRneg Minimum required negative resistance.
*                If the requirement is Rneg > 5*esr, please input 5*esr as minRneg.
*
* \return Error / status code:<br>
* CY_SYSCLK_SUCCESS - ECO configuration completed successfully<br>
* CY_SYSCLK_BAD_PARAM - One or more invalid parameters<br>
* CY_SYSCLK_INVALID_STATE - ECO already enabled
*
* \note
* The following calculations are implemented, generally in floating point:
*   freqMHz = freq / 1,000,000
*   max amplitude Vpp = 1,000 * sqrt(drivelevel / 2 / esr) / 3.14 / freqMHz / cSum
*   gm_min mA/V = 4 * 3.14 * 3.14 * freqMhz^2 * cSum^2 * 4 * minRneg / 1,000,000,000
*   As a result of the above calculations, max amplitude must be >= 0.3, and the
*   gm_min must be <= 17.6, otherwise this function returns with
*   a parameter error.<br>
*
* \note
* This function returns immediately if the ECO is enabled.
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_EcoConfigureWithMinRneg(uint32_t freq, uint32_t cSum, uint32_t esr, uint32_t driveLevel, uint32_t minRneg)
{
    /* Check if ECO is disabled */
    if(SRSS->unCLK_ECO_CONFIG.stcField.u1ECO_EN == 1ul)
    {
        return(CY_SYSCLK_INVALID_STATE);
    }

    /* calculate intermediate values */
    float32_t freqMHz      = (float32_t)freq / 1000000.0f;
    float32_t maxAmplitude = (1000.0f * ((float32_t)sqrt((float64_t)((float32_t)driveLevel / (2.0f * (float32_t)esr))))) /
                             (M_PI * freqMHz * (float32_t)cSum);

    float32_t gm_min       = (157.91367042f /*4 * M_PI * M_PI * 4*/ * minRneg * freqMHz * freqMHz * (float32_t)cSum * (float32_t)cSum) /
                             1000000000.0f;

    /* Get trim values according to caluculated values */
    uint32_t atrim, agcen, wdtrim, gtrim, rtrim, ftrim;
    atrim  = Cy_SysClk_SelectEcoAtrim(maxAmplitude);
    if(atrim == CY_SYSCLK_INVALID_TRIM_VALUE)
    {
        return(CY_SYSCLK_BAD_PARAM);
    }

    agcen = Cy_SysClk_SelectEcoAGCEN(maxAmplitude);
    if(agcen == CY_SYSCLK_INVALID_TRIM_VALUE)
    {
        return(CY_SYSCLK_BAD_PARAM);
    }

    wdtrim = Cy_SysClk_SelectEcoWDtrim(maxAmplitude);
    if(wdtrim == CY_SYSCLK_INVALID_TRIM_VALUE)
    {
        return(CY_SYSCLK_BAD_PARAM);
    }

    gtrim  = Cy_SysClk_SelectEcoGtrim(gm_min);
    if(gtrim == CY_SYSCLK_INVALID_TRIM_VALUE)
    {
        return(CY_SYSCLK_BAD_PARAM);
    }

    rtrim  = Cy_SysClk_SelectEcoRtrim(freqMHz);
    if(rtrim == CY_SYSCLK_INVALID_TRIM_VALUE)
    {
        return(CY_SYSCLK_BAD_PARAM);
    }

    ftrim  = Cy_SysClk_SelectEcoFtrim(atrim);

    /* update all fields of trim control register with one write, without
       changing the ITRIM field:
    */
    un_CLK_ECO_CONFIG2_t tempTrimEcoCtlReg;
    tempTrimEcoCtlReg.u32Register        = SRSS->unCLK_ECO_CONFIG2.u32Register;
    tempTrimEcoCtlReg.stcField.u3WDTRIM  = wdtrim;
    tempTrimEcoCtlReg.stcField.u4ATRIM   = atrim;
    tempTrimEcoCtlReg.stcField.u2FTRIM   = ftrim;
    tempTrimEcoCtlReg.stcField.u2RTRIM   = rtrim;
    tempTrimEcoCtlReg.stcField.u3GTRIM   = gtrim;
    SRSS->unCLK_ECO_CONFIG2.u32Register = tempTrimEcoCtlReg.u32Register;

    SRSS->unCLK_ECO_CONFIG.stcField.u1AGC_EN = agcen;

    return(CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_EcoConfigure
****************************************************************************//**
*
* This function configures the external crystal oscillator (ECO) trim bits based
* on crystal characteristics. This function should be called only when the ECO is
* disabled.
* This function assumed that required minimum negative resistance is 5 * ESR.
*
* \param freq Operating frequency of the crystal in Hz.
*
* \param cSum Amount of Crystal load capacitance and shunt capacitance in pF. 
*
* \param esr Effective series resistance of the crystal in ohms.
*
* \param driveLevel Crystal drive level in uW.
*
* \return Error / status code:<br>
* CY_SYSCLK_SUCCESS - ECO configuration completed successfully<br>
* CY_SYSCLK_BAD_PARAM - One or more invalid parameters<br>
* CY_SYSCLK_INVALID_STATE - ECO already enabled
*
* \note
* The following calculations are implemented, generally in floating point:
*   freqMHz = freq / 1,000,000
*   max amplitude Vpp = 1,000 * sqrt(drivelevel / 2 / esr) / 3.14 / freqMHz / cSum
*   gm_min mA/V = 5 * 4 * 3.14 * 3.14 * freqMhz^2 * cSum^2 * 4 * esr / 1,000,000,000
*   As a result of the above calculations, max amplitude must be >= 0.3, and the
*   gm_min must be <= 17.6, otherwise this function returns with
*   a parameter error.<br>
*
* \note
* This function returns immediately if the ECO is enabled.
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_EcoConfigure(uint32_t freq, uint32_t cSum, uint32_t esr, uint32_t driveLevel)
{
    return Cy_SysClk_EcoConfigureWithMinRneg(freq, cSum, esr, driveLevel, 5*esr);
}

/*******************************************************************************
* Function Name: Cy_SysClk_EcoEnable
****************************************************************************//**
*
* Enables the external crystal oscillator (ECO). This function should be called
* after \ref Cy_SysClk_EcoConfigure.
*
* \param timeoutus Amount of time in microseconds to wait for the ECO to lock.
* If a lock does not occur, the ECO is stopped. To avoid waiting for a lock, set
* this parameter to 0.
*
* \return Error / status code:<br>
* CY_SYSCLK_SUCCESS - ECO successfully enabled<br>
* CY_SYSCLK_TIMEOUT - Timeout waiting for ECO lock
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_EcoEnable(uint32_t timeoutus)
{
    cy_en_sysclk_status_t rtnval;

    /* invalid state error if ECO is already enabled */
    if (SRSS->unCLK_ECO_CONFIG.stcField.u1ECO_EN != 0ul) /* 1 = enabled */
    {
        return CY_SYSCLK_INVALID_STATE;
    }

    /* first set ECO enable */
    SRSS->unCLK_ECO_CONFIG.stcField.u1ECO_EN = 1ul; /* 1 = enable */

    /* now do the timeout wait for ECO_STATUS, bit ECO_OK */
    for (;
         (SRSS->unCLK_ECO_STATUS.stcField.u1ECO_OK == 0ul) &&(timeoutus != 0ul);
         timeoutus--)
    {
        Cy_SysLib_DelayUs(1u);
    }

    rtnval = ((timeoutus == 0ul) ? CY_SYSCLK_TIMEOUT : CY_SYSCLK_SUCCESS);
    return rtnval;
}

/*******************************************************************************
* Function Name: Cy_SysClk_EcoPrescaleEnable
****************************************************************************//**
*
* Enable ECO prescaler
*
* \param timeoutus Amount of time in microseconds to wait for the ECO prescaler
* gets enable.
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_EcoPrescaleEnable(uint32_t timeoutus)
{
    // Send enable command
    SRSS->unCLK_ECO_CONFIG.stcField.u1ECO_DIV_ENABLE = 1ul;

    // Wait eco prescaler get enabled
    while(CY_SYSCLK_ECO_PRESCALE_ENABLE != Cy_SysClk_GetEcoPrescaleStatus())
    {
        if(0ul == timeoutus)
        {
            return CY_SYSCLK_TIMEOUT;
        }

        Cy_SysLib_DelayUs(1u);

        timeoutus--;
    }

    return CY_SYSCLK_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_SysClk_EcoPrescaleDisable
****************************************************************************//**
*
*  Disable ECO prescaler 
*
* \param timeoutus Amount of time in microseconds to wait for the ECO prescaler
* gets disable.
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_EcoPrescaleDisable(uint32_t timeoutus)
{
    // Send disable command
    SRSS->unCLK_ECO_CONFIG.stcField.u1ECO_DIV_DISABLE = 1ul;

    // Wait eco prescaler actually get disabled
    while(CY_SYSCLK_ECO_PRESCALE_DISABLE != Cy_SysClk_GetEcoPrescaleStatus())
    {
        if(0ul == timeoutus)
        {
            return CY_SYSCLK_TIMEOUT;
        }

        Cy_SysLib_DelayUs(1u);

        timeoutus--;
    }

    return CY_SYSCLK_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_SysClk_SetEcoPrescale
****************************************************************************//**
*
*  Set ECO prescaler divider value
*
* \param ecoFreq provided frequency of ECO.
*
* \param targetFreq the target frequency of output clock after division.
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_SetEcoPrescale(uint32_t ecoFreq, uint32_t targetFreq)
{
    // Frequency of ECO (4MHz ~ 33.33MHz) might exceed 32bit value if shifted 8 bit.
    // So, it uses 64 bit data for fixed point operation.
    // Lowest 8 bit are fractional value. Next 10 bit are integer value.
    uint64_t fixedPointEcoFreq = ((uint64_t)ecoFreq << 8ull);
    uint64_t fixedPointDivNum64;
    uint32_t fixedPointDivNum;

    // Culculate divider number
    fixedPointDivNum64 = fixedPointEcoFreq / (uint64_t)targetFreq;

    // Dividing num should be larger 1.0, and smaller than maximum of 10bit number.
    if((fixedPointDivNum64 < 0x100ull) && (fixedPointDivNum64 > 0x40000ull))
    {
        return CY_SYSCLK_BAD_PARAM;
    }

    fixedPointDivNum = (uint32_t)fixedPointDivNum64;

    Cy_SysClk_SetEcoPrescaleManual(
                                        (((fixedPointDivNum & 0x0003FF00ul) >> 8ul) - 1ul),
                                        (fixedPointDivNum & 0x000000FFul)
                                    );

    return CY_SYSCLK_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetEcoPrescale
****************************************************************************//**
*
*  Get ECO prescaler divider value in floating point value
*
* \return The divider value in floating point
*
*******************************************************************************/
float32_t Cy_SysClk_GetEcoPrescale(void)
{
    uint32_t dividerValueMultipliedBy256;
    dividerValueMultipliedBy256  = SRSS->unCLK_ECO_PRESCALE.stcField.u8ECO_FRAC_DIV
                                  + ((SRSS->unCLK_ECO_PRESCALE.stcField.u10ECO_INT_DIV + 1ul) << 8ul);

    return((float32_t)dividerValueMultipliedBy256 / 256.0f);
}

/** \} group_sysclk_eco_funcs */

/* ========================================================================== */
/* ====================    INPUT MULTIPLEXER SECTION    ===================== */
/* ========================================================================== */
/**
* \addtogroup group_sysclk_muxes_funcs
* \{
*/
/*******************************************************************************
* Function Name: Cy_SysClk_ClkPathSetSource
****************************************************************************//**
*
* Configures the source for the specified clock path.
*
* \param clkPath Selects which clock path to configure; 0 is the first clock
* path, which is the FLL.
*
* \param source \ref cy_en_clkpath_in_sources_t
*
* \return \ref cy_en_sysclk_status_t
*
* \note
* If calling this function changes an FLL or PLL input frequency, disable the FLL
* or PLL before calling this function. After calling this function, call the FLL
* or PLL configure function, for example \ref Cy_SysClk_FllConfigure().
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_ClkPathSetSource(uint32_t clkPath, cy_en_clkpath_in_sources_t source)
{
    if(
          (clkPath >= SRSS_NUM_CLKPATH) ||
          ((CY_SYSCLK_CLKPATH_IN_DSIMUX < source) && (source < CY_SYSCLK_CLKPATH_IN_DSI0)) ||
          (CY_SYSCLK_CLKPATH_IN_ILO1 < source)
      )
    {
        return CY_SYSCLK_BAD_PARAM;
    }

    if (source >= CY_SYSCLK_CLKPATH_IN_DSI0)
    {
        SRSS->unCLK_DSI_SELECT[clkPath].stcField.u5DSI_MUX = ((uint32_t)source & CY_SYSCLK_CLKPATH_MSK);
        SRSS->unCLK_PATH_SELECT[clkPath].stcField.u3PATH_MUX = (uint32_t)CY_SYSCLK_CLKPATH_IN_DSIMUX;
    }
    else
    {
        SRSS->unCLK_PATH_SELECT[clkPath].stcField.u3PATH_MUX = (uint32_t)source;
    }

    return CY_SYSCLK_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_SysClk_ClkPathGetSource
****************************************************************************//**
*
* Reports which source is selected for the path mux.
*
* \param clkPath Selects which clock path to report; 0 is the first clock path,
* which is the FLL.
*
* \return \ref cy_en_clkpath_in_sources_t
*
*******************************************************************************/
cy_en_clkpath_in_sources_t Cy_SysClk_ClkPathGetSource(uint32_t clkPath)
{
    CY_ASSERT(clkPath < SRSS_NUM_CLKPATH);

    cy_en_clkpath_in_sources_t rtnval =
        (cy_en_clkpath_in_sources_t )SRSS->unCLK_PATH_SELECT[clkPath].stcField.u3PATH_MUX;

    if (rtnval == CY_SYSCLK_CLKPATH_IN_DSIMUX)
    {
        rtnval = (cy_en_clkpath_in_sources_t)(CY_SYSCLK_CLKPATH_IN_DSI0 |
                    (SRSS->unCLK_DSI_SELECT[clkPath].stcField.u5DSI_MUX));
    }

    return rtnval;
}
/** \} group_sysclk_muxes_funcs */


/* ========================================================================== */
/* ===========================    FLL SECTION    ============================ */
/* ========================================================================== */

/* min and max FLL output frequencies, in Hz */
#define  CY_SYSCLK_MIN_FLL_CCO_OUTPUT_FREQ  96000000ul
#define  CY_SYSCLK_MIN_FLL_OUTPUT_FREQ     (CY_SYSCLK_MIN_FLL_CCO_OUTPUT_FREQ / 2ul)
#define  CY_SYSCLK_MAX_GUARANTEED_ACCURACY 100000000ul
#define  CY_SYSCLK_MAX_FLL_OUTPUT_FREQ     160000000ul

/**
* \addtogroup group_sysclk_fll_funcs
* \{
*/
/*******************************************************************************
* Function Name: Cy_SysClk_FllConfigure
****************************************************************************//**
*
* Configures the FLL, for best accuracy optimization.
*
* \param inputFreq frequency of input source, in Hz
*
* \param outputFreq Desired FLL output frequency, in Hz. Allowable range is
* 24 MHz to 100 MHz (200 MHz with a relatively poor duty cycle. If the output
* frequency is less than 48 MHz, FLL_OUTPUT_DIV must be set; the output divide by
* 2 option is required.
*
* \param outputMode \ref cy_en_fll_pll_output_mode_t
* If output mode is bypass, then the output frequency equals the input source
* frequency regardless of the frequency parameter values.
*
* \return  Error / status code:<br>
* CY_SYSCLK_SUCCESS - FLL successfully configured<br>
* CY_SYSCLK_INVALID_STATE - FLL not configured because it is enabled<br>
* CY_SYSCLK_BAD_PARAM - desired output frequency is out of valid range
*
* \note
* Call this function after changing the FLL input frequency, for example if
* \ref Cy_SysClk_ClkPathSetSource() is called.
* \note
* Do not call this function when the FLL is enabled. If it is, then this function
* returns immediately with an error return value and no register updates.
*******************************************************************************/
/* constants indexed by ccoRange */
static const float32_t trimSteps_RefArray[]   = { 0.00110f, 0.00110f,   0.00110f, 0.00110f, 0.00117f}; // e.g. 0.00112f means 0.112%.
static const float32_t fMargin_MHz_RefArray[] = { 43600.0f, 58100.0f, 77200.0f, 103000.0f, 132000.0f};
/* igain and pgain bitfield values correspond to: 1/256, 1/128, ..., 4, 8 */
static const float32_t fll_gains_RefArray[]   = {0.00390625f, 0.0078125f, 0.015625f, 0.03125f, 0.0625f, 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f};
#define LOCK_TOLERANCE_THRESHOLD (0u)
cy_en_sysclk_status_t Cy_SysClk_FllConfigureStandard(uint32_t inputFreq, uint32_t outputFreq, cy_en_fll_pll_output_mode_t outputMode)
{
    /* check for errors */
    if (SRSS->unCLK_FLL_CONFIG.stcField.u1FLL_ENABLE != 0ul) /* 1 = enabled */
    {
        return(CY_SYSCLK_INVALID_STATE);
    }
    else if ((outputFreq < CY_SYSCLK_MIN_FLL_OUTPUT_FREQ) || (CY_SYSCLK_MAX_FLL_OUTPUT_FREQ < outputFreq)) /* invalid output frequency */
    {
        return(CY_SYSCLK_INVALID_STATE);
    }
    else if (((float32_t)outputFreq / (float32_t)inputFreq) < 2.2f) /* check output/input frequency ratio */
    {
        return(CY_SYSCLK_INVALID_STATE);
    }

    /* no error */

    /* If output mode is bypass (input routed directly to output), then done.
       The output frequency equals the input frequency regardless of the
       frequency parameters. */
    if (outputMode == CY_SYSCLK_FLLPLL_OUTPUT_INPUT)
    {
        /* bypass mode */
        /* update CLK_FLL_CONFIG3 register with divide by 2 parameter */
        SRSS->unCLK_FLL_CONFIG3.stcField.u2BYPASS_SEL = (uint32_t)outputMode;
        return(CY_SYSCLK_SUCCESS);
    }

    cy_stc_fll_manual_config_t config = { 0ul };

    config.outputMode = outputMode;

    /* 1. Output division is not required for standard accuracy. */
    config.enableOutputDiv = false;

    /* 2. Compute the target CCO frequency from the target output frequency and output division. */
    uint32_t ccoFreq;
    ccoFreq = outputFreq * ((uint32_t)(config.enableOutputDiv) + 1ul);

    /* 3. Compute the CCO range value from the CCO frequency */
    if(ccoFreq >= CY_SYSCLK_FLL_CCO_BOUNDARY4_FREQ)
    {
        config.ccoRange = CY_SYSCLK_FLL_CCO_RANGE4;
    }
    else if(ccoFreq >= CY_SYSCLK_FLL_CCO_BOUNDARY3_FREQ)
    {
        config.ccoRange = CY_SYSCLK_FLL_CCO_RANGE3;
    }
    else if(ccoFreq >= CY_SYSCLK_FLL_CCO_BOUNDARY2_FREQ)
    {
        config.ccoRange = CY_SYSCLK_FLL_CCO_RANGE2;
    }
    else if(ccoFreq >= CY_SYSCLK_FLL_CCO_BOUNDARY1_FREQ)
    {
        config.ccoRange = CY_SYSCLK_FLL_CCO_RANGE1;
    }
    else
    {
        config.ccoRange = CY_SYSCLK_FLL_CCO_RANGE0;
    }

    /* 4. Compute the FLL reference divider value. */
    config.refDiv = CY_SYSCLK_DIV_ROUNDUP(inputFreq * 200ul, outputFreq);

    /* 5. Compute the FLL multiplier value.
          Formula is fllMult = (ccoFreq * refDiv) /  fref */
    config.fllMult = CY_SYSCLK_DIV_ROUND((uint64_t)ccoFreq * (uint64_t)config.refDiv, (uint64_t)inputFreq);

    /* 6. Compute the lock tolerance.
          Recommendation: ROUNDUP((refDiv / fref ) * ccoFreq  * 3 * CCO_Trim_Step) + 2  */
    config.updateTolerance = CY_SYSCLK_DIV_ROUND(config.fllMult, (100ul * 2u) /* Reciprocal number of Ratio */ );
    if(config.updateTolerance > 255u)
    {
        config.updateTolerance = 255u;
    }

    config.lockTolerance   = (uint16_t)((config.fllMult * 2ul) / 100ul); /* Reciprocal number of Ratio */
    if(config.lockTolerance > 255u)
    {
        config.lockTolerance = 255u;
    }
    else if(config.lockTolerance > 1u)
    {
        config.lockTolerance = config.lockTolerance - 1u + LOCK_TOLERANCE_THRESHOLD;
    }
    else
    {
        config.lockTolerance = config.lockTolerance + LOCK_TOLERANCE_THRESHOLD;
    }

    // TODO: Need to check the recommend formula to calculate the value.

    /* 7. Compute the CCO igain and pgain. */
    /* intermediate parameters */
    float32_t kcco = trimSteps_RefArray[config.ccoRange] * fMargin_MHz_RefArray[config.ccoRange];
    float32_t ki_p = (0.85f * (float32_t)inputFreq) / (kcco * (float32_t)(config.refDiv)) / 1000.0f;
     /* find the largest IGAIN value that is less than or equal to ki_p */
    for(config.igain = CY_SYSCLK_N_ELMTS(fll_gains_RefArray) - 1ul;config.igain > 0ul; config.igain--)
    {
        if(fll_gains_RefArray[config.igain] < ki_p)
        {
            break;
        }
    }

    /* then find the largest PGAIN value that is less than or equal to ki_p - gains[igain] */
    for(config.pgain = CY_SYSCLK_N_ELMTS(fll_gains_RefArray) - 1ul; config.pgain > 0ul; config.pgain--)
    {
        if(fll_gains_RefArray[config.pgain] < (ki_p - fll_gains_RefArray[config.igain]))
        {
            break;
        }
    }

    /* 8. Compute the CCO_FREQ bits will be set by HW */
    config.ccoHwUpdateDisable = 0ul;

    /* 9. Compute the settling count, using a 1-usec settling time. */
    config.settlingCount = (uint16_t)((float32_t)inputFreq / 1000000.0f);

    /* configure FLL based on calculated values */
    cy_en_sysclk_status_t returnStatus;
    returnStatus = Cy_SysClk_FllManualConfigure(&config);

    return (returnStatus);
}

/*******************************************************************************
* Function Name: Cy_SysClk_FllManualConfigure
****************************************************************************//**
*
* Manually configures the FLL based on user inputs.
*
* \param config \ref cy_stc_fll_manual_config_t
*
* \return  Error / status code:<br>
* CY_SYSCLK_SUCCESS - FLL successfully configured<br>
* CY_SYSCLK_INVALID_STATE - FLL not configured because it is enabled
*
* \note
* Call this function after changing the FLL input frequency, for example if
* \ref Cy_SysClk_ClkPathSetSource() is called.
* \note
* Do not call this function when the FLL is enabled. If it is, then this function
* returns immediately with an error return value and no register updates.
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_FllManualConfigure(const cy_stc_fll_manual_config_t *config)
{
    cy_en_sysclk_status_t returnStatus = CY_SYSCLK_SUCCESS;

    /* check for errors */
    if (SRSS->unCLK_FLL_CONFIG.stcField.u1FLL_ENABLE != 0ul) /* 1 = enabled */
    {
        returnStatus = CY_SYSCLK_INVALID_STATE;
    }
    else
    { /* return status is OK */
    }

    /* no error */
    if (returnStatus == CY_SYSCLK_SUCCESS) /* no errors */
    {
        /* update CLK_FLL_CONFIG register with 2 parameters; FLL_ENABLE is already 0 */
        un_CLK_FLL_CONFIG_t tempConfg;
        tempConfg.u32Register               = SRSS->unCLK_FLL_CONFIG.u32Register;
        tempConfg.stcField.u18FLL_MULT      = config->fllMult;
        tempConfg.stcField.u1FLL_OUTPUT_DIV = (uint32_t)(config->enableOutputDiv);
        SRSS->unCLK_FLL_CONFIG.u32Register  = tempConfg.u32Register;

        /* update CLK_FLL_CONFIG2 register with 2 parameters */
        un_CLK_FLL_CONFIG2_t tempConfg2;
        tempConfg2.u32Register              = SRSS->unCLK_FLL_CONFIG2.u32Register;
        tempConfg2.stcField.u13FLL_REF_DIV  = config->refDiv;
        tempConfg2.stcField.u8LOCK_TOL      = config->lockTolerance;
        tempConfg2.stcField.u8UPDATE_TOL    = config->updateTolerance;
        SRSS->unCLK_FLL_CONFIG2.u32Register = tempConfg2.u32Register;

        /* update CLK_FLL_CONFIG3 register with 4 parameters */
        un_CLK_FLL_CONFIG3_t tempConfg3;
        tempConfg3.u32Register                = SRSS->unCLK_FLL_CONFIG3.u32Register;
        tempConfg3.stcField.u4FLL_LF_IGAIN    = config->igain;
        tempConfg3.stcField.u4FLL_LF_PGAIN    = config->pgain;
        tempConfg3.stcField.u13SETTLING_COUNT = config->settlingCount;
        tempConfg3.stcField.u2BYPASS_SEL      = (uint32_t)(config->outputMode);
        SRSS->unCLK_FLL_CONFIG3.u32Register   = tempConfg3.u32Register;

        /* update CLK_FLL_CONFIG4 register with 1 parameter; preserve other bits */
        un_CLK_FLL_CONFIG4_t tempConfg4;
        tempConfg4.u32Register                  = SRSS->unCLK_FLL_CONFIG4.u32Register;
        tempConfg4.stcField.u3CCO_RANGE         = (uint32_t)(config->ccoRange);
        tempConfg4.stcField.u9CCO_FREQ          = (uint32_t)(config->cco_Freq);
        tempConfg4.stcField.u1CCO_HW_UPDATE_DIS = (uint32_t)(config->ccoHwUpdateDisable);
        SRSS->unCLK_FLL_CONFIG4.u32Register     = tempConfg4.u32Register;
    } /* if no error */

    return (returnStatus);
}

/*******************************************************************************
* Function Name: Cy_SysClk_FllGetConfiguration
****************************************************************************//**
*
* Reports the FLL configuration settings.
*
* \param config \ref cy_stc_fll_manual_config_t
*
*******************************************************************************/
void Cy_SysClk_FllGetConfiguration(cy_stc_fll_manual_config_t *config)
{
    /* read 2 parameters from CLK_FLL_CONFIG register */
    un_CLK_FLL_CONFIG_t tempConfg;
    tempConfg.u32Register      = SRSS->unCLK_FLL_CONFIG.u32Register;
    config->fllMult            = tempConfg.stcField.u18FLL_MULT;
    config->enableOutputDiv    = (bool)tempConfg.stcField.u1FLL_OUTPUT_DIV;

    /* read 2 parameters from CLK_FLL_CONFIG2 register */
    un_CLK_FLL_CONFIG2_t tempConfg2;
    tempConfg2.u32Register     = SRSS->unCLK_FLL_CONFIG2.u32Register;
    config->refDiv             = tempConfg2.stcField.u13FLL_REF_DIV;
    config->lockTolerance      = tempConfg2.stcField.u8LOCK_TOL;
    config->updateTolerance    = tempConfg2.stcField.u8LOCK_TOL;

    /* read 4 parameters from CLK_FLL_CONFIG3 register */
    un_CLK_FLL_CONFIG3_t tempConfg3;
    tempConfg3.u32Register     = SRSS->unCLK_FLL_CONFIG3.u32Register;
    config->igain              = tempConfg3.stcField.u4FLL_LF_IGAIN;
    config->pgain              = tempConfg3.stcField.u4FLL_LF_PGAIN;
    config->settlingCount      = tempConfg3.stcField.u13SETTLING_COUNT;
    config->outputMode         = (cy_en_fll_pll_output_mode_t)tempConfg3.stcField.u2BYPASS_SEL;

    /* read 1 parameter from CLK_FLL_CONFIG4 register */
    un_CLK_FLL_CONFIG4_t tempConfg4;
    tempConfg4.u32Register     = SRSS->unCLK_FLL_CONFIG4.u32Register;
    config->ccoRange           = (cy_en_fll_cco_ranges_t)tempConfg4.stcField.u3CCO_RANGE;
    config->cco_Freq           = tempConfg4.stcField.u9CCO_FREQ;
    config->ccoHwUpdateDisable = tempConfg4.stcField.u1CCO_HW_UPDATE_DIS;
}

/*******************************************************************************
* Function Name: Cy_SysClk_FllEnable
****************************************************************************//**
*
* Enables the FLL. The FLL should be configured before calling this function.
*
* \param timeoutus amount of time in micro seconds to wait for FLL to lock.
* If lock doesn't occur, FLL is stopped. To avoid waiting for lock set this to 0,
* and manually check for lock using \ref Cy_SysClk_FllGetLockStatus.
*
* \return Error / status code:<br>
* CY_SYSCLK_SUCCESS - FLL successfully enabled<br>
* CY_SYSCLK_TIMEOUT - Timeout waiting for FLL lock
*
* \note
* While waiting for the FLL to lock, the FLL bypass mode is set to \ref CY_SYSCLK_FLLPLL_OUTPUT_INPUT.
* After the FLL is locked, the FLL bypass mode is then set to \ref CY_SYSCLK_FLLPLL_OUTPUT_OUTPUT.
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_FllEnable(uint32_t timeoutus)
{
    /* first set the CCO enable bit */
    SRSS->unCLK_FLL_CONFIG4.stcField.u1CCO_ENABLE = 1ul;

    /* Wait until CCO is ready */
    while(SRSS->unCLK_FLL_STATUS.stcField.u1CCO_READY == 0ul)
    {
        if(timeoutus == 0ul)
        {
            /* If cco ready doesn't occur, FLL is stopped. */
            Cy_SysClk_FllDisable();
            return(CY_SYSCLK_TIMEOUT);
        }
        Cy_SysLib_DelayUs(1u);
        timeoutus--;
    }

    /* Set the FLL bypass mode to 2 */
    SRSS->unCLK_FLL_CONFIG3.stcField.u2BYPASS_SEL = (uint32_t)CY_SYSCLK_FLLPLL_OUTPUT_INPUT;

    /* Set the FLL enable bit, if CCO is ready */
    SRSS->unCLK_FLL_CONFIG.stcField.u1FLL_ENABLE = 1ul;

    /* now do the timeout wait for FLL_STATUS, bit LOCKED */
    while(SRSS->unCLK_FLL_STATUS.stcField.u1LOCKED == 0ul)
    {
        if(timeoutus == 0ul)
        {
            /* If lock doesn't occur, FLL is stopped. */
            Cy_SysClk_FllDisable();
            return(CY_SYSCLK_TIMEOUT);
        }
        Cy_SysLib_DelayUs(1u);
        timeoutus--;
    }

    /* Lock occurred; we need to clear the unlock occurred bit.
       Do so by writing a 1 to it. */
    SRSS->unCLK_FLL_STATUS.stcField.u1UNLOCK_OCCURRED = 1ul;
    /* Set the FLL bypass mode to 3 */
    SRSS->unCLK_FLL_CONFIG3.stcField.u2BYPASS_SEL = (uint32_t)CY_SYSCLK_FLLPLL_OUTPUT_OUTPUT;

    return(CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_FllDisableSequence
****************************************************************************//**
*
* This function provides sequence to disable FLL.
*
* \return Error / status code:<br>
* CY_SYSCLK_SUCCESS - FLL successfully enabled<br>
* CY_SYSCLK_TIMEOUT - Timeout waiting for FLL lock
*
* \param waitCycle this parameter controls wait time after setting BYPASS register.
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_FllDisableSequence(uint32_t waitCycle)
{
    // Write CLK_FLL_CONFIG3.BYPASS_SEL=FLL_REF.
    SRSS->unCLK_FLL_CONFIG3.stcField.u2BYPASS_SEL = (uint32_t)CY_SYSCLK_FLLPLL_OUTPUT_INPUT;

    // Read CLK_FLL_CONFIG3.BYPASS_SEL to ensure the write completes (read is not optional).
    (void)SRSS->unCLK_FLL_CONFIG3.u32Register;

    // Wait at least ten cycles of either FLL reference clock or FLL output clock, whichever is slower.
    for(volatile uint32_t i = 0ul; i < waitCycle; i++);

    // Disable FLL with FLL_ENABLE=0.
    // Disable the CCO by writing CLK_FLL_CONFIG4.CCO_ENABLE=0.
    Cy_SysClk_FllDisable();

    // Write CLK_FLL_CONFIG3.BYPASS_SEL=AUTO.
    SRSS->unCLK_FLL_CONFIG3.stcField.u2BYPASS_SEL = (uint32_t)CY_SYSCLK_FLLPLL_OUTPUT_AUTO;

    // Read CLK_FLL_CONFIG3.BYPASS_SEL to ensure the write completes (read is not optional).
    (void)SRSS->unCLK_FLL_CONFIG3.u32Register;

    return(CY_SYSCLK_SUCCESS);
}

/** \} group_sysclk_fll_funcs */


/* ========================================================================== */
/* ===========================    PLL SECTION    ============================ */
/* ========================================================================== */
/** \cond INTERNAL */
/* PLL OUTPUT_DIV bitfield allowable range */
#define MIN_OUTPUT_DIV     (2ul)
#define MAX_OUTPUT_DIV    (16ul)

/* PLL REFERENCE_DIV bitfield allowable range */
#define MIN_REF_DIV        (1ul)
#define MAX_REF_DIV       (18ul)

/* PLL FEEDBACK_DIV bitfield allowable ranges, LF and normal modes */
#define MIN_FB_DIV_LF     (19ul)
#define MAX_FB_DIV_LF     (56ul)
#define MIN_FB_DIV_NORM   (22ul)
#define MAX_FB_DIV_NORM  (112ul)

/* PLL Fvco range allowable ranges, LF and normal modes */
#define MIN_FVCO_LF   (170000000ul)
#define MAX_FVCO_LF   (200000000ul)
#define MIN_FVCO_NORM (200000000ul)
#define MAX_FVCO_NORM (400000000ul)

/* PLL input and output frequency limits */
#define MIN_FPD_FREQ   (3960000ul)
#define MAX_FPD_FREQ   (8000000ul)
#define MIN_OUT_FREQ  (10625000ul)
#define MAX_OUT_FREQ (200000000ul)

/* PLL 400M OUTPUT_DIV bitfield allowable range */
#define PLL_400M_MIN_OUTPUT_DIV     (2ul)
#define PLL_400M_MAX_OUTPUT_DIV    (16ul)

/* PLL 400M REFERENCE_DIV bitfield allowable range */
#define PLL_400M_MIN_REF_DIV        (1ul)
#define PLL_400M_MAX_REF_DIV       (16ul)

/* PLL 400M FEEDBACK_DIV bitfield allowable ranges, LF and normal modes */
#define PLL_400M_MIN_FB_DIV        (16ul)
#define PLL_400M_MAX_FB_DIV       (200ul)

/* PLL 400M Fvco range */
#define PLL_400M_MIN_FVCO   (400000000ul)
#define PLL_400M_MAX_FVCO   (800000000ul)

/* PLL 400M input and output frequency limits */
#define PLL_400M_MIN_FPD_FREQ        (4000000ul)
#define PLL_400M_FRAC_MIN_FPD_FREQ   (8000000ul)
#define PLL_400M_MAX_FPD_FREQ       (20000000ul)
#define PLL_400M_MIN_OUT_FREQ       (25000000ul)
#define PLL_400M_MAX_OUT_FREQ      (400000000ul)

static const cy_stc_pll_limitation_t g_limPllLF =
{
    .maxFpd         = MAX_FPD_FREQ,
    .minFpd         = MIN_FPD_FREQ,
    .maxFvco        = MAX_FVCO_LF,
    .minFvco        = MIN_FVCO_LF,
    .maxFoutput     = MAX_OUT_FREQ,
    .minFoutput     = MIN_OUT_FREQ,
    .maxRefDiv      = MAX_REF_DIV,
    .minRefDiv      = MIN_REF_DIV,
    .maxFeedBackDiv = MAX_FB_DIV_LF,
    .minFeedBackDiv = MIN_FB_DIV_LF,
    .maxOutputDiv   = MAX_OUTPUT_DIV,
    .minOutputDiv   = MIN_OUTPUT_DIV,
};

static const cy_stc_pll_limitation_t g_limPllNORM =
{
    .maxFpd         = MAX_FPD_FREQ,
    .minFpd         = MIN_FPD_FREQ,
    .maxFvco        = MAX_FVCO_NORM,
    .minFvco        = MIN_FVCO_NORM,
    .maxFoutput     = MAX_OUT_FREQ,
    .minFoutput     = MIN_OUT_FREQ,
    .maxRefDiv      = MAX_REF_DIV,
    .minRefDiv      = MIN_REF_DIV,
    .maxFeedBackDiv = MAX_FB_DIV_NORM,
    .minFeedBackDiv = MIN_FB_DIV_NORM,
    .maxOutputDiv   = MAX_OUTPUT_DIV,
    .minOutputDiv   = MIN_OUTPUT_DIV,
};

static const cy_stc_pll_limitation_t g_limPll400M =
{
    .maxFpd         = PLL_400M_MAX_FPD_FREQ,
    .minFpd         = PLL_400M_MIN_FPD_FREQ,
    .maxFvco        = PLL_400M_MAX_FVCO,
    .minFvco        = PLL_400M_MIN_FVCO,
    .maxFoutput     = PLL_400M_MAX_OUT_FREQ,
    .minFoutput     = PLL_400M_MIN_OUT_FREQ,
    .maxRefDiv      = PLL_400M_MAX_REF_DIV,
    .minRefDiv      = PLL_400M_MIN_REF_DIV,
    .maxFeedBackDiv = PLL_400M_MAX_FB_DIV,
    .minFeedBackDiv = PLL_400M_MIN_FB_DIV,
    .maxOutputDiv   = PLL_400M_MAX_OUTPUT_DIV,
    .minOutputDiv   = PLL_400M_MIN_OUTPUT_DIV,
};

static const cy_stc_pll_limitation_t g_limPll400MFrac =
{
    .maxFpd         = PLL_400M_MAX_FPD_FREQ,
    .minFpd         = PLL_400M_FRAC_MIN_FPD_FREQ,
    .maxFvco        = PLL_400M_MAX_FVCO,
    .minFvco        = PLL_400M_MIN_FVCO,
    .maxFoutput     = PLL_400M_MAX_OUT_FREQ,
    .minFoutput     = PLL_400M_MIN_OUT_FREQ,
    .maxRefDiv      = PLL_400M_MAX_REF_DIV,
    .minRefDiv      = PLL_400M_MIN_REF_DIV,
    .maxFeedBackDiv = PLL_400M_MAX_FB_DIV,
    .minFeedBackDiv = PLL_400M_MIN_FB_DIV,
    .maxOutputDiv   = PLL_400M_MAX_OUTPUT_DIV,
    .minOutputDiv   = PLL_400M_MIN_OUTPUT_DIV,
};


/** \endcond */

/**
* \addtogroup group_sysclk_pll_funcs
* \{
*/
/*******************************************************************************
* Function Name: Cy_SysClk_PllConfigure
****************************************************************************//**
*
* Configures a given PLL.
* The configuration formula used is:
*   Fout = pll_clk * (P / Q / div_out), where:
*     Fout is the desired output frequency
*     pll_clk is the frequency of the input source
*     P is the feedback divider. Its value is in bitfield FEEDBACK_DIV.
*     Q is the reference divider. Its value is in bitfield REFERENCE_DIV.
*     div_out is the reference divider. Its value is in bitfield OUTPUT_DIV.
*
* \param clkPath Selects which PLL to configure. Please input PATH No. (e.g. "1" => PATH1 -> PLL0)
*
* \param config \ref cy_stc_pll_config_t

* \return  Error / status code:<br>
* CY_SYSCLK_SUCCESS - PLL successfully configured<br>
* CY_SYSCLK_INVALID_STATE - PLL not configured because it is enabled<br>
* CY_SYSCLK_BAD_PARAM - invalid clock path number, or input or desired output frequency is out of valid range
*
* \note
* Call this function after changing the PLL input frequency, for example if
* \ref Cy_SysClk_ClkPathSetSource() is called.
* \note
* Do not call this function when the PLL is enabled. If it is, then this function
* returns immediately with an error return value and no register updates.
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_PllConfigure(uint32_t clkPath, const cy_stc_pll_config_t *config)
{
    /* check for error */
    uint32_t pllNo;
    cy_en_sysclk_status_t status = Cy_SysClk_GetPllNo(clkPath, &pllNo);
    if(status != CY_SYSCLK_SUCCESS)
    {
        return(status);
    }

    if (SRSS->unCLK_PLL_CONFIG[pllNo].stcField.u1ENABLE != 0ul) /* 1 = enabled */
    {
        return (CY_SYSCLK_INVALID_STATE);
    }

    /* invalid output frequency */
    cy_stc_pll_manual_config_t manualConfig = {0ul};
    const cy_stc_pll_limitation_t* pllLim = (config->lfMode) ? &g_limPllLF : &g_limPllNORM;
    status = Cy_SysClk_PllCalucDividers(config->inputFreq,
                                           config->outputFreq,
                                           pllLim,
                                           0ul, // Frac bit num
                                           &manualConfig.feedbackDiv,
                                           &manualConfig.referenceDiv,
                                           &manualConfig.outputDiv,
                                           NULL
                                           );
    if(status != CY_SYSCLK_SUCCESS)
    {
        return(status);
    }

    /* configure PLL based on calculated values */
    manualConfig.lfMode     = config->lfMode;
    manualConfig.outputMode = config->outputMode;

    status = Cy_SysClk_PllManualConfigure(clkPath, &manualConfig);
    return (status);

}

/*******************************************************************************
* Function Name: Cy_SysClk_PllManualConfigure
****************************************************************************//**
*
* Manually configures a PLL based on user inputs.
*
* \param clkPath Selects which PLL to configure. Please input PATH No. (e.g. "1" => PATH1 -> PLL0)
*
* \param config \ref cy_stc_pll_manual_config_t
*
* \return  Error / status code:<br>
* CY_SYSCLK_SUCCESS - PLL successfully configured<br>
* CY_SYSCLK_INVALID_STATE - PLL not configured because it is enabled<br>
* CY_SYSCLK_BAD_PARAM - invalid clock path number
*
* \note
* Call this function after changing the PLL input frequency, for example if
* \ref Cy_SysClk_ClkPathSetSource() is called.
* \note
* Do not call this function when the PLL is enabled. If it is, then this function
* returns immediately with an error return value and no register updates.
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_PllManualConfigure(uint32_t clkPath, const cy_stc_pll_manual_config_t *config)
{
    /* check for error */
    uint32_t pllNo;
    cy_en_sysclk_status_t status = Cy_SysClk_GetPllNo(clkPath, &pllNo);
    if(status != CY_SYSCLK_SUCCESS)
    {
        return(status);
    }

    /* valid divider bitfield values */
    if((config->outputDiv < MIN_OUTPUT_DIV) || (MAX_OUTPUT_DIV < config->outputDiv))
    {
         return(CY_SYSCLK_BAD_PARAM);
    }

    if((config->referenceDiv < MIN_REF_DIV) || (MAX_REF_DIV < config->referenceDiv))
    {
         return(CY_SYSCLK_BAD_PARAM);
    }

    if((config->feedbackDiv  < (config->lfMode ? MIN_FB_DIV_LF : MIN_FB_DIV_NORM)) || 
       ((config->lfMode ? MAX_FB_DIV_LF : MAX_FB_DIV_NORM) < config->feedbackDiv))
    {
         return(CY_SYSCLK_BAD_PARAM);
    }

    un_CLK_PLL_CONFIG_t tempClkPLLConfigReg;
    tempClkPLLConfigReg.u32Register = SRSS->unCLK_PLL_CONFIG[pllNo].u32Register;
    if (tempClkPLLConfigReg.stcField.u1ENABLE != 0ul) /* 1 = enabled */
    {
        return(CY_SYSCLK_INVALID_STATE);
    }

    /* no errors */
    /* If output mode is bypass (input routed directly to output), then done.
       The output frequency equals the input frequency regardless of the frequency parameters. */
    if (config->outputMode != CY_SYSCLK_FLLPLL_OUTPUT_INPUT)
    {
        tempClkPLLConfigReg.stcField.u7FEEDBACK_DIV  = (uint32_t)config->feedbackDiv;
        tempClkPLLConfigReg.stcField.u5REFERENCE_DIV = (uint32_t)config->referenceDiv;
        tempClkPLLConfigReg.stcField.u5OUTPUT_DIV    = (uint32_t)config->outputDiv;
        tempClkPLLConfigReg.stcField.u1PLL_LF_MODE   = (uint32_t)config->lfMode;
    }
    tempClkPLLConfigReg.stcField.u2BYPASS_SEL = (uint32_t)config->outputMode;

    SRSS->unCLK_PLL_CONFIG[pllNo].u32Register = tempClkPLLConfigReg.u32Register;

    return (CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_PllGetConfiguration
****************************************************************************//**
*
* Reports configuration settings for a PLL.
*
* \param clkPath Selects which PLL to configure. Please input PATH No. (e.g. "1" => PATH1 -> PLL0)
*
* \param config \ref cy_stc_pll_manual_config_t
*
* \return  Error / status code:<br>
* CY_SYSCLK_SUCCESS - PLL data successfully reported<br>
* CY_SYSCLK_BAD_PARAM - invalid clock path number
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_PllGetConfiguration(uint32_t clkPath, cy_stc_pll_manual_config_t *config)
{
    uint32_t pllNo;
    cy_en_sysclk_status_t status = Cy_SysClk_GetPllNo(clkPath, &pllNo);
    if(status != CY_SYSCLK_SUCCESS)
    {
        return(status);
    }

    un_CLK_PLL_CONFIG_t tempReg;
    tempReg.u32Register  = SRSS->unCLK_PLL_CONFIG[pllNo].u32Register;
    config->feedbackDiv  = tempReg.stcField.u7FEEDBACK_DIV;
    config->referenceDiv = tempReg.stcField.u5REFERENCE_DIV;
    config->outputDiv    = tempReg.stcField.u5OUTPUT_DIV;
    config->outputMode   = (cy_en_fll_pll_output_mode_t)tempReg.stcField.u2BYPASS_SEL;
    config->lfMode       = tempReg.stcField.u1PLL_LF_MODE;

    return (CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_PllEnable
****************************************************************************//**
*
* Enables the PLL. The PLL should be configured before calling this function.
*
* \param clkPath Selects which PLL to configure. Please input PATH No. (e.g. "1" => PATH1 -> PLL0)
*
* \param timeoutus amount of time in microseconds to wait for the PLL to lock.
* If lock doesn't occur, PLL is stopped. To avoid waiting for lock set this to 0,
* and manually check for lock using \ref Cy_SysClk_PllGetLockStatus.
*
* \return Error / status code:<br>
* CY_SYSCLK_SUCCESS - PLL successfully enabled<br>
* CY_SYSCLK_TIMEOUT - Timeout waiting for PLL lock<br>
* CY_SYSCLK_BAD_PARAM - invalid clock path number
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_PllEnable(uint32_t clkPath, uint32_t timeoutus)
{
    uint32_t pllNo;
    cy_en_sysclk_status_t status = Cy_SysClk_GetPllNo(clkPath, &pllNo);
    if(status != CY_SYSCLK_SUCCESS)
    {
        return(status);
    }

    /* first set the PLL enable bit */
    SRSS->unCLK_PLL_CONFIG[pllNo].stcField.u1ENABLE = 1ul;

    /* now do the timeout wait for PLL_STATUS, bit LOCKED */
    for (; (SRSS->unCLK_PLL_STATUS[pllNo].stcField.u1LOCKED == 0ul) &&
           (timeoutus != 0ul);
         timeoutus--)
    {
        Cy_SysLib_DelayUs(1u);
    }

    status = ((timeoutus == 0ul) ? CY_SYSCLK_TIMEOUT : CY_SYSCLK_SUCCESS);

    return (status);
}

/*******************************************************************************
* Function Name: Cy_SysClk_Pll400MConfigure
****************************************************************************//**
*
* Configures a given PLL 400M.
* The configuration formula used is:
*   Fout = pll_clk * (P + Pfrac/(2^24) / Q / div_out), where:
*     Fout is the desired output frequency
*     pll_clk is the frequency of the input source
*     P is the feedback divider. Its value is in bitfield FEEDBACK_DIV.
*     Pfrac is the fractional portion of feedback divider Its value is in bitfield FRAC_DIV
*     Q is the reference divider. Its value is in bitfield REFERENCE_DIV.
*     div_out is the reference divider. Its value is in bitfield OUTPUT_DIV.
*
* \param clkPath Selects which PLL to configure. Please input PATH No. (e.g. "1" => PATH1 -> PLL0)
*
* \param config \ref cy_stc_pll_400M_config_t
* if config->fracEn is 0, then FRAC_DIV is not used (would be set to "0").
*
* \return  Error / status code:<br>
* CY_SYSCLK_SUCCESS - PLL successfully configured<br>
* CY_SYSCLK_INVALID_STATE - PLL not configured because it is enabled<br>
* CY_SYSCLK_BAD_PARAM - invalid clock path number, or input or desired output frequency is out of valid range
*
* \note
* Call this function after changing the PLL input frequency, for example if
* \ref Cy_SysClk_ClkPathSetSource() is called.
* \note
* Do not call this function when the PLL is enabled. If it is, then this function
* returns immediately with an error return value and no register updates.
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_Pll400MConfigure(uint32_t clkPath, const cy_stc_pll_400M_config_t *config)
{
    /* check for error */
    uint32_t pllNo;
    cy_en_sysclk_status_t status = Cy_SysClk_GetPll400MNo(clkPath, &pllNo);
    if(status != CY_SYSCLK_SUCCESS)
    {
        return(status);
    }

    if (SRSS->CLK_PLL400M[pllNo].unCONFIG.stcField.u1ENABLE != 0ul) /* 1 = enabled */
    {
        return (CY_SYSCLK_INVALID_STATE);
    }

    cy_stc_pll_400M_manual_config_t manualConfig = {0ul};
    const cy_stc_pll_limitation_t* pllLim;
    uint32_t fracBitNum;
    if(config->fracEn == true)
    {
        pllLim = &g_limPll400MFrac;
        fracBitNum = 24ul;
    }
    else
    {
        pllLim = &g_limPll400M;
        fracBitNum = 0ul;
    }
    status = Cy_SysClk_PllCalucDividers(config->inputFreq,
                                           config->outputFreq,
                                           pllLim,
                                           fracBitNum,
                                           &manualConfig.feedbackDiv,
                                           &manualConfig.referenceDiv,
                                           &manualConfig.outputDiv,
                                           &manualConfig.fracDiv
                                           );
    if(status != CY_SYSCLK_SUCCESS)
    {
        return(status);
    }

    manualConfig.outputMode   = config->outputMode;
    manualConfig.fracEn       = config->fracEn;
    manualConfig.fracDitherEn = config->fracDitherEn;
    manualConfig.sscgEn       = config->sscgEn;
    manualConfig.sscgDitherEn = config->sscgDitherEn;
    manualConfig.sscgDepth    = config->sscgDepth;
    manualConfig.sscgRate     = config->sscgRate;

    status = Cy_SysClk_Pll400MManualConfigure(clkPath, &manualConfig);
    return (status);
}

/*******************************************************************************
* Function Name: Cy_SysClk_Pll400MManualConfigure
****************************************************************************//**
*
* Manually configures a PLL 400M based on user inputs.
*
* \param clkPath Selects which PLL to configure. Please input PATH No. (e.g. "1" => PATH1 -> PLL0)
*
* \param config \ref cy_stc_pll_400M_manual_config_t
*
* \return  Error / status code:<br>
* CY_SYSCLK_SUCCESS - PLL successfully configured<br>
* CY_SYSCLK_INVALID_STATE - PLL not configured because it is enabled<br>
* CY_SYSCLK_BAD_PARAM - invalid clock path number
*
* \note
* Call this function after changing the PLL input frequency, for example if
* \ref Cy_SysClk_ClkPathSetSource() is called.
* \note
* Do not call this function when the PLL is enabled. If it is, then this function
* returns immediately with an error return value and no register updates.
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_Pll400MManualConfigure(uint32_t clkPath, const cy_stc_pll_400M_manual_config_t *config)
{
    /* check for error */
    uint32_t pllNo;
    cy_en_sysclk_status_t status = Cy_SysClk_GetPll400MNo(clkPath, &pllNo);
    if(status != CY_SYSCLK_SUCCESS)
    {
        return(status);
    }

    /* valid divider bitfield values */
    if((config->outputDiv < PLL_400M_MIN_OUTPUT_DIV) || (PLL_400M_MAX_OUTPUT_DIV < config->outputDiv))
    {
         return(CY_SYSCLK_BAD_PARAM);
    }

    if((config->referenceDiv < PLL_400M_MIN_REF_DIV) || (PLL_400M_MAX_REF_DIV < config->referenceDiv))
    {
         return(CY_SYSCLK_BAD_PARAM);
    }

    if((config->feedbackDiv  < PLL_400M_MIN_FB_DIV) || (PLL_400M_MAX_FB_DIV < config->feedbackDiv))
    {
         return(CY_SYSCLK_BAD_PARAM);
    }

    un_CLK_PLL400M_CONFIG_t tempClkPLL400MConfigReg;
    tempClkPLL400MConfigReg.u32Register = SRSS->CLK_PLL400M[pllNo].unCONFIG.u32Register;
    if (tempClkPLL400MConfigReg.stcField.u1ENABLE != 0ul) /* 1 = enabled */
    {
        return(CY_SYSCLK_INVALID_STATE);
    }

    /* no errors */
    /* If output mode is bypass (input routed directly to output), then done.
       The output frequency equals the input frequency regardless of the frequency parameters. */
    if (config->outputMode != CY_SYSCLK_FLLPLL_OUTPUT_INPUT)
    {
        tempClkPLL400MConfigReg.stcField.u8FEEDBACK_DIV    = (uint32_t)config->feedbackDiv;
        tempClkPLL400MConfigReg.stcField.u5REFERENCE_DIV   = (uint32_t)config->referenceDiv;
        tempClkPLL400MConfigReg.stcField.u5OUTPUT_DIV      = (uint32_t)config->outputDiv;
    }
    tempClkPLL400MConfigReg.stcField.u2BYPASS_SEL          = (uint32_t)config->outputMode;
    SRSS->CLK_PLL400M[pllNo].unCONFIG.u32Register          = tempClkPLL400MConfigReg.u32Register;

    un_CLK_PLL400M_CONFIG2_t tempClkPLL400MConfig2Reg;
    tempClkPLL400MConfig2Reg.u32Register               = SRSS->CLK_PLL400M[pllNo].unCONFIG2.u32Register;
    tempClkPLL400MConfig2Reg.stcField.u24FRAC_DIV      = config->fracDiv;
    tempClkPLL400MConfig2Reg.stcField.u3FRAC_DITHER_EN = config->fracDitherEn;
    tempClkPLL400MConfig2Reg.stcField.u1FRAC_EN        = config->fracEn;
    SRSS->CLK_PLL400M[pllNo].unCONFIG2.u32Register     = tempClkPLL400MConfig2Reg.u32Register;

    un_CLK_PLL400M_CONFIG3_t tempClkPLL400MConfig3Reg;
    tempClkPLL400MConfig3Reg.u32Register               = SRSS->CLK_PLL400M[pllNo].unCONFIG3.u32Register;
    tempClkPLL400MConfig3Reg.stcField.u10SSCG_DEPTH    = (uint32_t)config->sscgDepth;
    tempClkPLL400MConfig3Reg.stcField.u3SSCG_RATE      = (uint32_t)config->sscgRate;
    tempClkPLL400MConfig3Reg.stcField.u1SSCG_DITHER_EN = (uint32_t)config->sscgDitherEn;
    tempClkPLL400MConfig3Reg.stcField.u1SSCG_EN        = (uint32_t)config->sscgEn;
    SRSS->CLK_PLL400M[pllNo].unCONFIG3.u32Register     = tempClkPLL400MConfig3Reg.u32Register;

    return (CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_Pll400MGetConfiguration
****************************************************************************//**
*
* Reports configuration settings for a PLL 400M.
*
* \param clkPath Selects which PLL to configure. Please input PATH No. (e.g. "1" => PATH1 -> PLL0)
*
* \param config \ref cy_stc_pll_400M_manual_config_t

* \return  Error / status code:<br>
* CY_SYSCLK_SUCCESS - PLL data successfully reported<br>
* CY_SYSCLK_BAD_PARAM - invalid clock path number
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_Pll400MGetConfiguration(uint32_t clkPath, cy_stc_pll_400M_manual_config_t *config)
{
    uint32_t pllNo;
    cy_en_sysclk_status_t status = Cy_SysClk_GetPll400MNo(clkPath, &pllNo);
    if(status != CY_SYSCLK_SUCCESS)
    {
        return(status);
    }

    un_CLK_PLL400M_CONFIG_t tempConfigReg;
    tempConfigReg.u32Register = SRSS->CLK_PLL400M[pllNo].unCONFIG.u32Register;
    config->feedbackDiv       = tempConfigReg.stcField.u8FEEDBACK_DIV;
    config->referenceDiv      = tempConfigReg.stcField.u5REFERENCE_DIV;
    config->outputDiv         = tempConfigReg.stcField.u5OUTPUT_DIV;
    config->outputMode        = (cy_en_fll_pll_output_mode_t)tempConfigReg.stcField.u2BYPASS_SEL;

    un_CLK_PLL400M_CONFIG2_t tempConfig2Reg;
    tempConfig2Reg.u32Register = SRSS->CLK_PLL400M[pllNo].unCONFIG2.u32Register;
    config->fracEn             = tempConfig2Reg.stcField.u1FRAC_EN;
    config->fracDitherEn       = (bool)tempConfig2Reg.stcField.u3FRAC_DITHER_EN;
    config->fracDiv            = tempConfig2Reg.stcField.u24FRAC_DIV;

    un_CLK_PLL400M_CONFIG3_t tempConfig3Reg;
    tempConfig3Reg.u32Register = SRSS->CLK_PLL400M[pllNo].unCONFIG3.u32Register;
    config->sscgEn             = tempConfig3Reg.stcField.u1SSCG_EN;
    config->sscgDitherEn       = tempConfig3Reg.stcField.u1SSCG_DITHER_EN;
    config->sscgDepth          = (cy_en_pll_400M_ssgc_depth_t)tempConfig3Reg.stcField.u10SSCG_DEPTH;
    config->sscgRate           = (cy_en_pll_400M_ssgc_rate_t)tempConfig3Reg.stcField.u3SSCG_RATE;

    return (CY_SYSCLK_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_SysClk_Pll400MEnable
****************************************************************************//**
*
* Enables the PLL 400M. The PLL 400M should be configured before calling this function.
*
* \param clkPath Selects which PLL to configure. Please input PATH No. (e.g. "1" => PATH1 -> PLL0)
*
* \param timeoutus amount of time in microseconds to wait for the PLL to lock.
* If lock doesn't occur, PLL is stopped. To avoid waiting for lock set this to 0,
* and manually check for lock using \ref Cy_SysClk_Pll400MGetLockStatus.
*
* \return Error / status code:<br>
* CY_SYSCLK_SUCCESS - PLL successfully enabled<br>
* CY_SYSCLK_TIMEOUT - Timeout waiting for PLL lock<br>
* CY_SYSCLK_BAD_PARAM - invalid clock path number
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_Pll400MEnable(uint32_t clkPath, uint32_t timeoutus)
{
    uint32_t pllNo;
    cy_en_sysclk_status_t status = Cy_SysClk_GetPll400MNo(clkPath, &pllNo);
    if(status != CY_SYSCLK_SUCCESS)
    {
        return(status);
    }

    /* first set the PLL enable bit */
    SRSS->CLK_PLL400M[pllNo].unCONFIG.stcField.u1ENABLE = 1ul;

    /* now do the timeout wait for PLL_STATUS, bit LOCKED */
    for (; (SRSS->CLK_PLL400M[pllNo].unSTATUS.stcField.u1LOCKED == 0ul) &&
           (timeoutus != 0ul);
         timeoutus--)
    {
        Cy_SysLib_DelayUs(1u);
    }

    status = ((timeoutus == 0ul) ? CY_SYSCLK_TIMEOUT : CY_SYSCLK_SUCCESS);

    return (status);
}

/** \} group_sysclk_pll_funcs */


/* ========================================================================== */
/* ====================    Clock Measurement section    ===================== */
/* ========================================================================== */

/* Cy_SysClk_StartClkMeasurementCounters() input parameter saved for use later in other functions. */
static uint32_t clk1Count1;

/* These variables act as locks to prevent collisions between clock measurement and entry into
   DeepSleep mode. See Cy_SysClk_DeepSleep(). */
static bool clkCounting     = false;
static bool preventCounting = false;

/**
* \addtogroup group_sysclk_calclk_funcs
* \{
*/
/*******************************************************************************
* Function Name: Cy_SysClk_StartClkMeasurementCounters
****************************************************************************//**
*
* Assigns clocks to the clock measurement counters, and starts counting. The counters
* let you measure a clock frequency using another clock as a reference. There are two
* counters.<br>
* One counter (counter1), which is clocked by clock1, is loaded with an initial value
* and counts down to zero.<br>
* The second counter (counter2), which is clocked by clock2, counts up until the first
* counter reaches zero.<br>
* Either clock1 or clock2 can be a reference clock; the other clock becomes the
* measured clock. The reference clock frequency is always known.<br>
* After calling this function, call \ref Cy_SysClk_ClkMeasurementCountersDone()
* to determine when counting is done, that is, counter1 has counted down to zero.
* Then call \ref Cy_SysClk_ClkMeasurementCountersGetFreq() to calculate the frequency
* of the measured clock.
*
* \param clock1 The clock for counter1
*
* \param count1 The initial value for counter1, from which counter1 counts down to zero.
*
* \param clock2 The clock for counter2
*
* \return Error / status code:<br>
* CY_SYSCLK_INVALID_STATE if already doing a measurement<br>
* CY_SYSCLK_BAD_PARAM if invalid clock input parameter<br>
* else CY_SYSCLK_SUCCESS
*
* \note The counters are both 24-bit, so the maximum value of count1 is 0xFFFFFF.
* If clock2 frequency is greater than clock1, make sure that count1 is low enough
* that counter2 does not overflow before counter1 reaches zero.
* \note The time to complete a measurement is count1 / clock1 frequency.
* \note The clocks for both counters must have a nonzero frequency, or
* \ref Cy_SysClk_ClkMeasurementCountersGetFreq() incorrectly reports the result of the
* previous  measurement.
* \note Do not enter a device low power mode (Sleep, Deep Sleep) while doing a measurement;
* the measured clock frequency may not be accurate.
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_StartClkMeasurementCounters(cy_en_meas_clks_t clock1, uint32_t count1, cy_en_meas_clks_t clock2)
{
    cy_en_sysclk_status_t rtnval = CY_SYSCLK_INVALID_STATE;

    if(count1 > 0x00FFFFFFul)
    {
        clk1Count1 = 0ul;
        return(CY_SYSCLK_BAD_PARAM);
    }
    
    if(count1 == 0ul)
    {
        clk1Count1 = 0ul;
        return(CY_SYSCLK_BAD_PARAM);
    }

    if (!preventCounting /* don't start a measurement if about to enter DeepSleep mode */  ||
        SRSS->unCLK_CAL_CNT1.stcField.u1CAL_COUNTER_DONE != 0ul/*1 = done*/)
    {
        /* Connect the indicated clocks to the respective counters.

           if clock1 is a slow clock,
             select it in SRSS_CLK_OUTPUT_SLOW.SLOW_SEL0, and SRSS_CLK_OUTPUT_FAST.FAST_SEL0 = SLOW_SEL0
           else if clock1 is a fast clock,
             select it in SRSS_CLK_OUTPUT_FAST.FAST_SEL0,
           else error, do nothing and return.

           if clock2 is a slow clock,
             select it in SRSS_CLK_OUTPUT_SLOW.SLOW_SEL1, and SRSS_CLK_OUTPUT_FAST.FAST_SEL1 = SLOW_SEL1
           else if clock2 is a fast clock,
             select it in SRSS_CLK_OUTPUT_FAST.FAST_SEL1,
           else error, do nothing and return.
        */
        rtnval = CY_SYSCLK_BAD_PARAM;
        if ((clock1 < CY_SYSCLK_MEAS_CLK_LAST_CLK) && (clock2 < CY_SYSCLK_MEAS_CLK_LAST_CLK))
        {
            /* Disallow entry into DeepSleep mode while counting. */
            clkCounting = true;

            if (clock1 < CY_SYSCLK_MEAS_CLK_FAST_CLKS)
            { /* slow clock */
                SRSS->unCLK_OUTPUT_SLOW.stcField.u4SLOW_SEL0 = (uint32_t)clock1;
                SRSS->unCLK_OUTPUT_FAST.stcField.u4FAST_SEL0 = 7ul; /*slow_sel0 output*/
            }
            else
            { /* fast clock */
                if (clock1 < CY_SYSCLK_MEAS_CLK_PATH_CLKS)
                { /* ECO, EXT */
                    SRSS->unCLK_OUTPUT_FAST.stcField.u4FAST_SEL0 = (uint32_t)clock1;
                }
                else
                { /* PATH or HFCLK */
                    SRSS->unCLK_OUTPUT_FAST.stcField.u4FAST_SEL0 = (((uint32_t)clock1 >> 8ul) & 0x0Ful); /*use enum bits [11:8]*/
                    if (clock1 < CY_SYSCLK_MEAS_CLK_HF_CLKS)
                    { /* PATH select */
                        SRSS->unCLK_OUTPUT_FAST.stcField.u4PATH_SEL0 = ((uint32_t)clock1 & 0x0Ful); /*use enum bits [3:0]*/
                    }
                    else
                    { /* HFCLK select */
                        SRSS->unCLK_OUTPUT_FAST.stcField.u4HFCLK_SEL0 = ((uint32_t)clock1 & 0x0Ful); /*use enum bits [3:0]*/
                    }
                }
            } /* clock1 fast clock */

            if (clock2 < CY_SYSCLK_MEAS_CLK_FAST_CLKS)
            { /* slow clock */
                SRSS->unCLK_OUTPUT_SLOW.stcField.u4SLOW_SEL1 = (uint32_t)clock2;
                SRSS->unCLK_OUTPUT_FAST.stcField.u4FAST_SEL1 =  7ul; /*slow_sel1 output*/
            }
            else
            { /* fast clock */
                if (clock2 < CY_SYSCLK_MEAS_CLK_PATH_CLKS)
                { /* ECO, EXT */
                    SRSS->unCLK_OUTPUT_FAST.stcField.u4FAST_SEL1 = (uint32_t)clock2;
                }
                else
                { /* PATH or HFCLK */
                    SRSS->unCLK_OUTPUT_FAST.stcField.u4FAST_SEL1 = (((uint32_t)clock2 >> 8ul) & 0x0Ful); /*use enum bits [11:8]*/
                    if (clock2 < CY_SYSCLK_MEAS_CLK_HF_CLKS)
                    { /* PATH select */
                        SRSS->unCLK_OUTPUT_FAST.stcField.u4PATH_SEL1 = ((uint32_t)clock2 & 0x0Ful); /*use enum bits [3:0]*/
                    }
                    else
                    { /* HFCLK select */
                        SRSS->unCLK_OUTPUT_FAST.stcField.u4HFCLK_SEL1 = ((uint32_t)clock2 & 0x0Ful); /*use enum bits [3:0]*/
                    }
                }
            } /* clock2 fast clock */

            rtnval = CY_SYSCLK_SUCCESS;
            
            /* Save this input parameter for use later, in other functions.
               No error checking is done on this parameter.*/
            clk1Count1 = count1;

            /* Counting starts when counter1 is written with a nonzero value. */
            SRSS->unCLK_CAL_CNT1.stcField.u24CAL_COUNTER1 = clk1Count1;
        } /* if (clock1 < CY_SYSCLK_MEAS_CLK_LAST_CLK && clock2 < CY_SYSCLK_MEAS_CLK_LAST_CLK) */
        else
        {
            clk1Count1 = 0ul;
        }
    } /* if (not done) */
    return (rtnval);
}

/*******************************************************************************
* Function Name: Cy_SysClk_ClkMeasurementCountersGetFreq
****************************************************************************//**
*
* Calculates the frequency of the indicated measured clock (clock1 or clock2).<br>
* If clock1 is the measured clock, its frequency is:<br>
* clock1 frequency = (count1 / count2) * clock2 frequency<br>
* If clock2 is the measured clock, its frequency is:<br>
* clock2 frequency = (count2 / count1) * clock1 frequency<br>
* Call this function only after counting is done; see \ref Cy_SysClk_ClkMeasurementCountersDone().
*
* \param measuredClock False (0) if the measured clock is clock1, true (nonzero)
* if the measured clock is clock2.
*
* \param refClkFreq The reference clock frequency (clock1 or clock2).
*
* \return The frequency of the measured clock, in Hz.
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_ClkMeasurementCountersGetFreq(uint32_t *measuredFreq, uint32_t refClkFreq)
{
    if(SRSS->unCLK_CAL_CNT1.stcField.u1CAL_COUNTER_DONE != 1ul)
    {
        return(CY_SYSCLK_INVALID_STATE);
    }
    
    if(clk1Count1 == 0ul)
    {
        return(CY_SYSCLK_INVALID_STATE);
    }

    volatile uint64_t counter2Value = (uint64_t)SRSS->unCLK_CAL_CNT2.stcField.u24CAL_COUNTER2;

    /* Done counting; allow entry into DeepSleep mode. */
    clkCounting = false;
    
    *measuredFreq = CY_SYSCLK_DIV_ROUND(counter2Value * (uint64_t)refClkFreq, (uint64_t)clk1Count1 );

    return(CY_SYSCLK_SUCCESS);
}
/** \} group_sysclk_calclk_funcs */


/* ========================================================================== */
/* ==========================    TRIM SECTION    ============================ */
/* ========================================================================== */
/**
* \addtogroup group_sysclk_trim_funcs
* \{
*/

/*******************************************************************************
* Function Name: Cy_SysClk_IloTrim
****************************************************************************//**
*
* Trims the ILO to be as close to 32,768 Hz as possible.
*
* \param iloFreq current ILO frequency. Call \ref Cy_SysClk_StartClkMeasurementCounters
* and other measurement functions to obtain the current frequency of the ILO. 
*
* \param iloNo ILO No to be set. If 0, this function sets ILO0, else sets ILO1
*
* \return Change in trim value; 0 if done, that is, no change in trim value.
*
* \note The watchdog timer (WDT) must be unlocked before calling this function.
*******************************************************************************/
/** \cond INTERNAL */
/* target frequency */
#define CY_SYSCLK_ILO_TARGET_FREQ  32768u
/** \endcond */

int32_t Cy_SysClk_IloTrim(uint32_t iloFreq, uint8_t iloNo)
{
    /* Nominal trim step size is 1.5% of "the frequency". Using the target frequency. */
    const uint32_t trimStep = CY_SYSCLK_DIV_ROUND((uint32_t)CY_SYSCLK_ILO_TARGET_FREQ * 15ul, 1000ul);

    uint32_t newTrim = 0ul;
    uint32_t curTrim = 0ul;

    /* Do nothing if iloFreq is already within one trim step from the target */
    uint32_t diff = (uint32_t)abs((int32_t)iloFreq - (int32_t)CY_SYSCLK_ILO_TARGET_FREQ);
    if (diff >= trimStep)
    {
        if(iloNo == 0u)
        {
            curTrim = SRSS->unCLK_TRIM_ILO0_CTL.stcField.u6ILO0_FTRIM;
        }
        else
        {
            curTrim = SRSS->unCLK_TRIM_ILO1_CTL.stcField.u6ILO1_FTRIM;
        }

        if (iloFreq > CY_SYSCLK_ILO_TARGET_FREQ)
        { /* iloFreq is too high. Reduce the trim value */
            newTrim = curTrim - CY_SYSCLK_DIV_ROUND(iloFreq - CY_SYSCLK_ILO_TARGET_FREQ, trimStep);
        }
        else
        { /* iloFreq too low. Increase the trim value. */
            newTrim = curTrim + CY_SYSCLK_DIV_ROUND(CY_SYSCLK_ILO_TARGET_FREQ - iloFreq, trimStep);
        }

        /* Update the trim value */
        if(iloNo == 0u)
        {
            if(WDT->unLOCK.stcField.u2WDT_LOCK != 0ul) /* WDT registers are disabled */
            {
                return(CY_SYSCLK_INVALID_STATE);
            }
            SRSS->unCLK_TRIM_ILO0_CTL.stcField.u6ILO0_FTRIM = newTrim;
        }
        else
        {
            SRSS->unCLK_TRIM_ILO1_CTL.stcField.u6ILO1_FTRIM = newTrim;
        }
    }
    return (int32_t)(curTrim - newTrim);
}

/** \} group_sysclk_trim_funcs */


/* ========================================================================== */
/* ======================    POWER MANAGEMENT SECTION    ==================== */
/* ========================================================================== */
/**
* \addtogroup group_sysclk_pm_funcs
* \{
*/
/** \cond INTERNAL */
/* timeout count for use in function Cy_SysClk_DeepSleepCallback() is sufficiently large for ~1 second at 100 MHz */
#define TIMEOUTK 5000000ul
/** \endcond */

/*******************************************************************************
* Function Name: Cy_SysClk_DeepSleepCallback
****************************************************************************//**
*
* This is a callback function for System Power Management (SysPm), for entering
* DeepSleep mode. This function does the following tests:<br>
* FLL and PLL:<br>
* 1. If the FLL or a PLL have the ECO as a source (see \ref Cy_SysClk_ClkPathGetSource),
*    and is enabled, changes the source to IMO.<br>
* 2. On wakeup, timeout-waits for FLL and PLL to regain their frequency locks,
*    if enabled.<br>
* Frequency measurement: Prevents entry into DeepSleep mode if currently counting;
* see \ref Cy_SysClk_StartClkMeasurementCounters. Does not allow start of counting
* if already indicated that entry into DeepSleep mode is OK.<br>
*
* This function can be called during execution of \ref Cy_SysPm_DeepSleep.
* To do so, register this function as a callback before calling
* \ref Cy_SysPm_DeepSleep - specify \ref CY_SYSPM_DEEPSLEEP as the callback
* type and call \ref Cy_SysPm_RegisterCallback.
*
* \note If the FLL or PLL source is the ECO, this function must be registered, and
* it must be the last callback function that is registered. This is needed to allow the
* ECO to stabilize before reconnecting it to the FLL or PLL.
*
* \param callbackParams
* structure with the syspm callback parameters,
* see \ref cy_stc_syspm_callback_params_t.
*
* \return Error / status code; see \ref cy_en_syspm_status_t. Pass if not doing
* a clock measurement, otherwise Fail. Timeout if timeout waiting for FLL or a PLL
* to regain its frequency lock.
*
*******************************************************************************/
cy_en_syspm_status_t Cy_SysClk_DeepSleepCallback(cy_stc_syspm_callback_params_t *callbackParams)
{
    /* bitmapped paths and roots that may be affected by FLL or PLL being sourced by ECO */
    static uint16_t changedSourcePaths;

    cy_en_syspm_status_t rtnval = CY_SYSPM_SUCCESS;

    /* Entry into DeepSleep mode tests */
    if (callbackParams->mode == CY_SYSPM_CHECK_READY)
    {
        /* Don't allow entry into DeepSleep mode if currently measuring a frequency. */
        if (clkCounting)
        {
            rtnval = CY_SYSPM_FAIL;
        }
        else 
        { /* Indicating that we can go into DeepSleep. Before doing so ... */
            uint32_t fllpll; /* 0 = FLL, all other values = a PLL */

            /* initialize record of changed paths */
            changedSourcePaths = 0u;

            /* for FLL and each PLL, */
            for (fllpll = 0ul; fllpll < (SRSS_NUM_PLL + 1ul); fllpll++)
            {
                /* If FLL or PLL is enabled, */
                if (0ul != ((fllpll == 0ul) ? (SRSS->unCLK_FLL_CONFIG.stcField.u1FLL_ENABLE): 
                                              (SRSS->unCLK_PLL_CONFIG[fllpll - 1ul].stcField.u1ENABLE)))
                {
                    /* and the FLL or PLL has ECO as a source, */
                    if (Cy_SysClk_ClkPathGetSource(fllpll) == CY_SYSCLK_CLKPATH_IN_ECO)
                    {
                        /* Change this FLL or PLL source to IMO */
                        (void)Cy_SysClk_ClkPathSetSource(fllpll, CY_SYSCLK_CLKPATH_IN_IMO);
                        /* keep a record that this FLL or PLL's source was changed from ECO */
                        changedSourcePaths |= (uint16_t)(1ul << fllpll);
                    }
                }
            }
            /* Set the FLL bypass mode to 2 */
            SRSS->unCLK_FLL_CONFIG3.stcField.u2BYPASS_SEL = (uint32_t)CY_SYSCLK_FLLPLL_OUTPUT_INPUT;

            /* Also, prevent starting a new clock measurement until after we've come back from DeepSleep. */
            preventCounting = true;
        }
    }

    /* After return from DeepSleep, for each FLL and PLL, if needed, restore the source to ECO.
       And block until the FLL or PLL has regained its frequency locks. */
    else if (callbackParams->mode == CY_SYSPM_AFTER_EXIT)
    {
        /* if any FLL/PLL was sourced by the ECO, timeout wait for the ECO to become fully stabilized again. */
        if (changedSourcePaths != 0u)
        {
            uint32_t timeout = TIMEOUTK;
            /* Cy_SysClk_EcoGetStatus()return value 2ul = fully stabilized */
            //for (timeout = TIMEOUTK; (timeout != 0ul) && (Cy_SysClk_EcoGetStatus() != 2ul); timeout--){}
            
            while ((2ul != Cy_SysClk_EcoGetStatus()) && (0UL != timeout))
            {
                timeout--;
            }
            
            //if (timeout != 0ul)
            //{
            //    rtnval = CY_SYSPM_TIMEOUT;
            //}
            
            if (0UL != timeout)
            {
                /* for FLL and each PLL, */
                uint32_t fllpll; /* 0 = FLL, all other values = a PLL */
                for (fllpll = 0ul; fllpll < (SRSS_NUM_PLL + 1ul); fllpll++)
                {
                    /* If FLL or PLL is enabled, */
                    if (0ul != ((fllpll == 0ul) ? (SRSS->unCLK_FLL_CONFIG.stcField.u1FLL_ENABLE): 
                                                  (SRSS->unCLK_PLL_CONFIG[fllpll - 1].stcField.u1ENABLE)))
                    {
                        /* check the record that this FLL or PLL's source was changed from ECO */
                        if ((changedSourcePaths & (uint16_t)(1ul << fllpll)) != 0u)
                        {
                            /* Change this FLL or PLL source back to ECO */
                            (void)Cy_SysClk_ClkPathSetSource(fllpll, CY_SYSCLK_CLKPATH_IN_ECO);
                        }
                        { /* Timeout wait for FLL or PLL to regain lock. */
                            uint32_t timout;
                            for (timout = TIMEOUTK; timout != 0ul; timout--)
                            {
                                if (true == ((fllpll == 0ul) ? Cy_SysClk_FllGetLockStatus() : Cy_SysClk_PllGetLockStatus(fllpll - 1ul)))
                                {
                                    break;
                                }
                            }
                            if (timout != 0ul)
                            {
                                rtnval = CY_SYSPM_TIMEOUT;
                            }
                        }
                    }
                }
            }
        }
        
        /* Set the FLL bypass mode to 3 */
        SRSS->unCLK_FLL_CONFIG3.stcField.u2BYPASS_SEL = (uint32_t)CY_SYSCLK_FLLPLL_OUTPUT_OUTPUT;

        /* Coming back from DeepSleep; we can allow clock measurement. */
        preventCounting = false;
    }

    /* No other modes need be checked. */
    else
    {
    }

    return rtnval;
}
/** \} group_sysclk_pm_funcs */


/* ========================================================================== */
/* ========================    clk_hf[n] SECTION    ========================= */
/* ========================================================================== */
/**
* \addtogroup group_sysclk_clk_hf_funcs
* \{
*/
/*******************************************************************************
* Function Name: Cy_SysClk_HfClockConfigureCsv
****************************************************************************//**
*
* Configures the HfClk clock supervisor (CSV).
*
* \param hfClk selects which HfClk CSV to configure.
*
* \param config \ref cy_stc_hf_csv_config_t
*
* \return Error / status code: CY_SYSCLK_INVALID_STATE if HF CSV is not present
* in the device, else CY_SYSCLK_SUCCESS
*
* \note
* If loss detection is enabled, writes to other register bits are ignored.
* Therefore loss detection is disabled before writing the config structure
* members to the CTL register. Note that one of the config structure members is
* an enable bit.
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_HfClockConfigureCsv(cy_en_hfclk_t hfClk, const cy_stc_hf_csv_config_t *config)
{
#if defined(SRSS_MASK_HFCSV) && (SRSS_MASK_HFCSV != 0)
    if(
        (hfClk >= SRSS_NUM_HFROOT) ||
        (config == NULL) ||
        (config->FrequencyAction > CY_SYSCLK_CSV_ERROR_FAULT_RESET) ||
        (config->LossWindow > CY_SYSCLK_CSV_LOSS_512_CYCLES) ||
        (config->LossAction > CY_SYSCLK_CSV_ERROR_FAULT_RESET)
       )
    {
        return CY_SYSCLK_BAD_PARAM;
    }

    un_CLK_CSV_HF_LIMIT_t tempCLK_CSV_HF_LIMITValue;
    un_CLK_CSV_HF_CTL_t tempCLK_CSV_HF_CTLValue;

    /* First update the limit bits; this can be done regardless of enable state. */
    tempCLK_CSV_HF_LIMITValue.u32Register             = SRSS->CLK_CSV[hfClk].unHF_LIMIT.u32Register;
    tempCLK_CSV_HF_LIMITValue.stcField.u16UPPER_LIMIT = config->FrequencyUpperLimit;
    tempCLK_CSV_HF_LIMITValue.stcField.u16LOWER_LIMIT = config->FrequencyLowerLimit;
    SRSS->CLK_CSV[hfClk].unHF_LIMIT.u32Register       = tempCLK_CSV_HF_LIMITValue.u32Register;

    /* Then clear all CTL register bits, including the enable bit; disable loss detection. */
    tempCLK_CSV_HF_CTLValue.u32Register                 = SRSS->CLK_CSV[hfClk].unHF_CTL.u32Register;
    /* Finally, write the structure elements (which include an enable bit) to the CTL register. */
    tempCLK_CSV_HF_CTLValue.stcField.u1CSV_LOSS_EN      = config->enableLossDetection;
    tempCLK_CSV_HF_CTLValue.stcField.u2CSV_LOSS_ACTION  = (uint32_t)(config->LossAction);
    tempCLK_CSV_HF_CTLValue.stcField.u1CSV_FREQ_EN      = config->enableFrquencyFaultDetection;
    tempCLK_CSV_HF_CTLValue.stcField.u2CSV_FREQ_ACTION  = (uint32_t)(config->FrequencyAction);
    tempCLK_CSV_HF_CTLValue.stcField.u3CSV_LOSS_WINDOW  = (uint32_t)(config->LossWindow);
    tempCLK_CSV_HF_CTLValue.stcField.u2CSV_MUX          = (uint32_t)(config->SupervisorClock);
    tempCLK_CSV_HF_CTLValue.stcField.u16CSV_FREQ_WINDOW = config->SupervisingWindow;
    SRSS->CLK_CSV[hfClk].unHF_CTL.u32Register           = tempCLK_CSV_HF_CTLValue.u32Register;

    return CY_SYSCLK_SUCCESS;
#else
    return CY_SYSCLK_INVALID_STATE;
#endif /* SRSS_MASK_HFCSV != 0 */
}

/** \} group_sysclk_clk_hf_funcs */


/* ========================================================================== */
/* =====================     clk_reports_ SECTION      ====================== */
/* ========================================================================== */
/**
* \addtogroup group_sysclk_reculc_freq_funcs
* \{
*/

cy_stc_base_clk_freq_t internalBaseFreqInfo;
bool intFreqInitialized = false;

/*******************************************************************************
* Function Name: Cy_SysClk_InitGetFreqParams
****************************************************************************//**
*
* Set base clock frequency which are necessary to calculate further clocks.
*
* \param in \ref cy_stc_base_clk_freq_t
*
* \return \cy_en_sysclk_status_t
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_InitGetFreqParams(cy_stc_base_clk_freq_t *in)
{
    internalBaseFreqInfo = *in;
    intFreqInitialized   = true;

    return(CY_SYSCLK_SUCCESS);

}

/*******************************************************************************
* Function Name: Cy_SysClk_GetImoFrequency
****************************************************************************//**
*
* Reports clock IMO frequency. This function does not measure the frequency,
* just return values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetImoFrequency(uint32_t *result)
{
    if(SRSS->unCLK_IMO_CONFIG.stcField.u1ENABLE == 0ul)
    {
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }

    if(intFreqInitialized)
    {
        *result = internalBaseFreqInfo.clk_imo_freq;
        return(CY_SYSCLK_SUCCESS);
    }
    else
    {
        /* Internal parameter has not been initialized */
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }

}

/*******************************************************************************
* Function Name: Cy_SysClk_GetEcoFrequency
****************************************************************************//**
*
* Reports clock ECO frequency. This function does not measure the frequency,
* just return values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetEcoFrequency(uint32_t *result)
{
    if(Cy_SysClk_EcoGetStatus() == 0ul)
    {
        /* ECO does not have sufficient amplitude */
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }

    if(intFreqInitialized)
    {
        *result = internalBaseFreqInfo.clk_eco_freq;
        return(CY_SYSCLK_SUCCESS);
    }
    else
    {
        /* Internal parameter has not been initialized */
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetExtFrequency
****************************************************************************//**
*
* Reports clock External frequency. This function does not measure the frequency,
* just return values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetExtFrequency(uint32_t *result)
{
    if(intFreqInitialized)
    {
        *result = internalBaseFreqInfo.clk_ext_freq;
        return(CY_SYSCLK_SUCCESS);
    }
    else
    {
        /* Internal parameter has not been initialized */
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetIlo0Frequency
****************************************************************************//**
*
* Reports clock ILO0 frequency. This function does not measure the frequency,
* just return values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetIlo0Frequency(uint32_t *result)
{
    if(SRSS->unCLK_ILO0_CONFIG.stcField.u1ENABLE == 0ul)
    {
        /* ILO0 is not enabled */
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }

    if(intFreqInitialized)
    {
        *result = internalBaseFreqInfo.clk_ilo0_freq;
        return(CY_SYSCLK_SUCCESS);
    }
    else
    {
        /* Internal parameter has not been initialized */
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetIlo1Frequency
****************************************************************************//**
*
* Reports clock ILO1 frequency. This function does not measure the frequency,
* just return values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetIlo1Frequency(uint32_t *result)
{
    if(SRSS->unCLK_ILO1_CONFIG.stcField.u1ENABLE == 0ul)
    {
        /* ILO1 is not enabled */
        *result = 0UL;
        return(CY_SYSCLK_INVALID_STATE);
    }

    if(intFreqInitialized)
    {
        *result = internalBaseFreqInfo.clk_ilo1_freq;
        return(CY_SYSCLK_SUCCESS);
    }
    else
    {
        /* Internal parameter has not been initialized */
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetWcoFrequency
****************************************************************************//**
*
* Reports clock WCO frequency. This function does not measure the frequency,
* just return values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetWcoFrequency(uint32_t *result)
{
    if(Cy_SysClk_WcoOkay() == false)
    {
        /* WCO is not enabled */
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }

    if(intFreqInitialized)
    {
        *result = internalBaseFreqInfo.clk_wco_freq;
        return(CY_SYSCLK_SUCCESS);
    }
    else
    {
        /* Internal parameter has not been initialized */
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetRefHfFrequency
****************************************************************************//**
*
* Reports clock ref_hf frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetRefHfFrequency(uint32_t *result)
{
    switch(Cy_SysClk_ClkRefGetSource())
    {
    case CY_SYSCLK_CLKREF_IN_IMO:
        return(Cy_SysClk_GetImoFrequency(result));
    case CY_SYSCLK_CLKREF_IN_EXTCLK:
        return(Cy_SysClk_GetExtFrequency(result));
    case CY_SYSCLK_CLKREF_IN_ECO:
        return(Cy_SysClk_GetEcoFrequency(result));
    default:
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);  // unknown state
    }
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetPathSourceFrequency
****************************************************************************//**
*
* Reports clock PATHx source frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \param pathNo The number of PATH clock.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetPathSourceFrequency(uint8_t pathNo, uint32_t *result)
{
    switch(Cy_SysClk_ClkPathGetSource(pathNo))
    {
    case CY_SYSCLK_CLKPATH_IN_IMO:
        return(Cy_SysClk_GetImoFrequency(result));
    case CY_SYSCLK_CLKPATH_IN_EXT:
        return(Cy_SysClk_GetExtFrequency(result));
    case CY_SYSCLK_CLKPATH_IN_ECO:
        return(Cy_SysClk_GetEcoFrequency(result));
    case CY_SYSCLK_CLKPATH_IN_ILO0:
        return(Cy_SysClk_GetIlo0Frequency(result));
    case CY_SYSCLK_CLKPATH_IN_WCO:
        return(Cy_SysClk_GetWcoFrequency(result));
    case CY_SYSCLK_CLKPATH_IN_ILO1:
        return(Cy_SysClk_GetIlo1Frequency(result));
    default:
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);  // unknown state
    }
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetFllInputFrequency
****************************************************************************//**
*
* Reports clock FLL(PATH0) source frequency. This function does not calculate
* the frequency, just return values which is the result of calculation from register
* values and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetFllInputFrequency(uint32_t *result)
{
    return(Cy_SysClk_GetPathSourceFrequency(/* fll */ 0u, result));
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetPllInputFrequency
****************************************************************************//**
*
* Reports clock PLL source frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \param pllNo The number of PLL.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetPllInputFrequency(uint32_t pllNo, uint32_t *result)
{
    uint8_t pllMaxNo = SRSS_NUM_PLL + SRSS_NUM_PLL400M - 1ul;

    if(pllNo > (uint32_t)pllMaxNo)
    {
        *result = 0ul;
        return(CY_SYSCLK_BAD_PARAM);  // unknown state
    }

    return(Cy_SysClk_GetPathSourceFrequency(pllNo + 1ul, result));
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetFllOutputFrequency
****************************************************************************//**
*
* Reports clock FLL output frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetFllOutputFrequency(uint32_t *result)
{
    uint32_t fllInputFreq;
    cy_en_sysclk_status_t status;
    status = Cy_SysClk_GetFllInputFrequency(&fllInputFreq);
    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }

    cy_stc_fll_manual_config_t fllConfig;
    Cy_SysClk_FllGetConfiguration(&fllConfig);

#if (CY_USE_PSVP == 1)
    *result = fllInputFreq;
    return(CY_SYSCLK_SUCCESS);
#else
    if(fllConfig.outputMode == CY_SYSCLK_FLLPLL_OUTPUT_INPUT)
    {
        *result = fllInputFreq;
        return(CY_SYSCLK_SUCCESS);
    }

    if(SRSS->unCLK_FLL_CONFIG.stcField.u1FLL_ENABLE == 0ul)
    {
        *result = fllInputFreq;
        return(CY_SYSCLK_SUCCESS);
    }

    bool locked = Cy_SysClk_FllGetLockStatus();
    if(locked == false)
    {
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);  // unknown state
    }

    *result = (fllInputFreq * fllConfig.fllMult) / fllConfig.refDiv / (fllConfig.enableOutputDiv + 1ul);
    return(CY_SYSCLK_SUCCESS);
#endif
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetPllOutputFrequency
****************************************************************************//**
*
* Reports clock PLL output frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \param pllNo The number of PLL.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetPllOutputFrequency(uint8_t pllNo, uint32_t *result)
{
    uint32_t pllInputFreq;
    cy_en_sysclk_status_t status;
    status = Cy_SysClk_GetPllInputFrequency(pllNo, &pllInputFreq);
    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }

#if (CY_USE_PSVP == 1)
    *result = pllInputFreq;
    return(CY_SYSCLK_SUCCESS);
#else

    if(pllNo > (SRSS_NUM_PLL400M - 1ul)) // PLL
    {
        cy_stc_pll_manual_config_t pllConfig;
        status = Cy_SysClk_PllGetConfiguration((pllNo + 1u), &pllConfig);
        CY_ASSERT(status == CY_SYSCLK_SUCCESS);

        if(SRSS->unCLK_PLL_CONFIG[pllNo - SRSS_NUM_PLL400M].stcField.u1ENABLE == 0ul)
        {
            *result = pllInputFreq;
            return(CY_SYSCLK_SUCCESS);
        }

        if(pllConfig.outputMode == CY_SYSCLK_FLLPLL_OUTPUT_INPUT)
        {
            *result = pllInputFreq;
            return(CY_SYSCLK_SUCCESS);
        }

        uint8_t locked = Cy_SysClk_PllGetLockStatus(pllNo + 1u);
        if((locked == 0x00u) || (locked == 0xFFu))
        {
            *result = 0ul;
            return(CY_SYSCLK_INVALID_STATE);  // unknown state
        }

        *result = (pllInputFreq * pllConfig.feedbackDiv) / pllConfig.referenceDiv / pllConfig.outputDiv ;
        return(CY_SYSCLK_SUCCESS);
    }
    else // PLL 400M
    {
        cy_stc_pll_400M_manual_config_t pllConfig;
        status = Cy_SysClk_Pll400MGetConfiguration((pllNo + 1u), &pllConfig);
        CY_ASSERT(status == CY_SYSCLK_SUCCESS);

        if(SRSS->CLK_PLL400M[pllNo].unCONFIG.stcField.u1ENABLE == 0ul)
        {
            *result = pllInputFreq;
            return(CY_SYSCLK_SUCCESS);
        }

        if(pllConfig.outputMode == CY_SYSCLK_FLLPLL_OUTPUT_INPUT)
        {
            *result = pllInputFreq;
            return(CY_SYSCLK_SUCCESS);
        }

        uint8_t locked = Cy_SysClk_Pll400MGetLockStatus(pllNo + 1u);
        if((locked == 0x00u) || (locked == 0xFFu))
        {
            *result = 0ul;
            return(CY_SYSCLK_INVALID_STATE);  // unknown state
        }

        if(pllConfig.fracEn == true)
        {
            *result = (uint32_t)((((uint64_t)pllInputFreq * (((uint64_t)pllConfig.feedbackDiv << 24ull) + (uint64_t)pllConfig.fracDiv)) / ((uint64_t)pllConfig.referenceDiv * (uint64_t)pllConfig.outputDiv)) >> 24ull);
        }
        else
        {
            *result = (pllInputFreq * pllConfig.feedbackDiv) / pllConfig.referenceDiv / pllConfig.outputDiv ;
        }
        return(CY_SYSCLK_SUCCESS);
    }

#endif
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetPathFrequency
****************************************************************************//**
*
* Reports clock PATHx output frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \param pathNo The number of PATH clock.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetPathFrequency(uint8_t pathNo, uint32_t *result)
{
    uint8_t pllMaxNo = SRSS_NUM_PLL + SRSS_NUM_PLL400M - 1u;
    uint8_t pllNo    = pathNo - 1u;

    if(pathNo == 0u)
    {
        return(Cy_SysClk_GetFllOutputFrequency(result));
    }
    else if(pllNo <= pllMaxNo)
    {
        return(Cy_SysClk_GetPllOutputFrequency(pllNo, result));
    }
    else
    {
        return(Cy_SysClk_GetPathSourceFrequency(pathNo, result));
    }
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetHfClkFrequency
****************************************************************************//**
*
* Reports clock HFx frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \param hfClkNo The number of the hf clock.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetHfClkFrequency(cy_en_hfclk_t hfClkNo, uint32_t *result)
{
#if (CY_USE_PSVP == 1)
    // Note: This function may return wrong results if default HF clock initialization in system_tviic2d6mddr_cm0plus.c for PSVP is changed 
    *result = CY_INITIAL_TARGET_PERI_FREQ; // Just use one of the available defines in system header that reflects the PSVP frequency.
    return(CY_SYSCLK_SUCCESS);
#else
    cy_en_sysclk_status_t   status;
    uint32_t                clkSourceFreq;

    cy_en_hf_clk_sources_t  hfClkSource   = Cy_SysClk_HfClockGetSource(hfClkNo);
    if(hfClkSource == CY_SYSCLK_HFCLK_IN_CLKIMO)
    {
        status = Cy_SysClk_GetImoFrequency(result);
        return(status);
    }

    status = Cy_SysClk_GetPathFrequency((uint8_t)hfClkSource, &clkSourceFreq);
    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }

    cy_en_hf_clk_dividers_t divNum        = Cy_SysClk_HfClockGetDivider(hfClkNo);
    switch(divNum)
    {
    case CY_SYSCLK_HFCLK_NO_DIVIDE:
        *result = clkSourceFreq;
        break;
    case CY_SYSCLK_HFCLK_DIVIDE_BY_2:
        *result = clkSourceFreq / 2ul;
        break;
    case CY_SYSCLK_HFCLK_DIVIDE_BY_4:
        *result = clkSourceFreq / 4ul;
        break;
    case CY_SYSCLK_HFCLK_DIVIDE_BY_8:
        *result = clkSourceFreq / 8ul;
        break;
    default:
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }
    return(CY_SYSCLK_SUCCESS);
#endif
}


/*******************************************************************************
* Function Name: Cy_SysClk_GetWDTClkFrequency
****************************************************************************//**
*
* Reports clock for WDT frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetWDTClkFrequency(uint32_t *result)
{
    return(Cy_SysClk_GetIlo0Frequency(result));
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetRTCClkFrequency
****************************************************************************//**
*
* Reports clock for RTC (or BAK) frequency. This function does not measure the
* frequency, just return values which is the result of calculation from register
* values and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetRTCClkFrequency(uint32_t *result)
{
    cy_en_clkbak_input_sources_t source = Cy_SysClk_ClkBakGetSource();
    switch(source)
    {
    case CY_SYSCLK_BAK_IN_WCO:
        return(Cy_SysClk_GetWcoFrequency(result));
    case CY_SYSCLK_BAK_IN_ALTBAK:
        *result = 0ul;
        return CY_SYSCLK_BAD_PARAM; // No ALTBAK
    case CY_SYSCLK_BAK_IN_ILO:
        return(Cy_SysClk_GetIlo0Frequency(result));
#if defined(SRSS_BACKUP_S40E_LPECO_PRESENT) && (SRSS_BACKUP_S40E_LPECO_PRESENT == 1)
    case CY_SYSCLK_BAK_IN_LPECO_PRESCALER:
        *result = 0ul;
        return CY_SYSCLK_BAD_PARAM; // Not supported yet
#endif
    default:
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetEcoPrescaleFrequency
****************************************************************************//**
*
* Reports clock ECO prescale frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetEcoPrescaleFrequency(uint32_t *result)
{
    if(Cy_SysClk_GetEcoPrescaleStatus() != CY_SYSCLK_ECO_PRESCALE_ENABLE)
    {
        return(CY_SYSCLK_INVALID_STATE);
    }

    cy_en_sysclk_status_t status;
    uint32_t              ecoClkFreq;
    float32_t             divVal;
    status = Cy_SysClk_GetEcoFrequency(&ecoClkFreq);
    if(status != CY_SYSCLK_SUCCESS)
    {
        return(status);
    }

    divVal = Cy_SysClk_GetEcoPrescale();
    *result = (uint32_t)((float32_t)ecoClkFreq / divVal);
    return(CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetLfFrequency
****************************************************************************//**
*
* Reports clock LF frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetLfFrequency(uint32_t *result)
{
    cy_en_lfclk_input_sources_t cldLfSource = Cy_SysClk_LfClkGetSource();
    switch(cldLfSource)
    {
    case CY_SYSCLK_LFCLK_IN_ILO0:
        return(Cy_SysClk_GetIlo0Frequency(result));
    case CY_SYSCLK_LFCLK_IN_WCO:
        return(Cy_SysClk_GetWcoFrequency(result));
    case CY_SYSCLK_LFCLK_IN_ILO1:
        return(Cy_SysClk_GetIlo1Frequency(result));
    case CY_SYSCLK_LFCLK_IN_ECO_PRESCALE:
        return(Cy_SysClk_GetEcoPrescaleFrequency(result));
    default:
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetMTWDTFrequency
****************************************************************************//**
*
* Reports clock for MCWDT frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetMTWDTFrequency(uint32_t *result)
{
    return(Cy_SysClk_GetLfFrequency(result));
}



/*******************************************************************************
* Function Name: Cy_SysClk_GetTimerFrequency
****************************************************************************//**
*
* Reports clock Timer frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetTimerFrequency(uint32_t *result)
{
    // CLK_TIMER_CTL register is defined as "Obsolete. Do not use in new designs" now.
    // This results in a clock of 1MHz based on the register default value (IMO / 8)
    *result = 1000000UL; 
    return CY_SYSCLK_SUCCESS;
    
#if 0
    if(SRSS->unCLK_TIMER_CTL.stcField.u1ENABLE == 0ul)
    {
        return(CY_SYSCLK_INVALID_STATE);
    }

    cy_en_sysclk_status_t status;

    cy_en_clktimer_input_sources_t souceInfo = Cy_SysClk_ClkTimerGetSource();
    uint32_t sourceFreq;
    switch(souceInfo)
    {
    case CY_SYSCLK_CLKTIMER_IN_IMO:
        status = Cy_SysClk_GetImoFrequency(&sourceFreq);
        break;
    case CY_SYSCLK_CLKTIMER_IN_HF0_NODIV:
        status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_0, &sourceFreq);
        break;
    case CY_SYSCLK_CLKTIMER_IN_HF0_DIV2:
        status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_0, &sourceFreq);
        sourceFreq = sourceFreq / 2ul;
        break;
    case CY_SYSCLK_CLKTIMER_IN_HF0_DIV4:
        status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_0, &sourceFreq);
        sourceFreq = sourceFreq / 4ul;
        break;
    case CY_SYSCLK_CLKTIMER_IN_HF0_DIV8:
        status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_0, &sourceFreq);
        sourceFreq = sourceFreq / 8ul;
        break;
    default:
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }

    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }

    *result = sourceFreq / (Cy_SysClk_ClkTimerGetDivider() + 1ul);
    return(CY_SYSCLK_SUCCESS);
#endif
}


/*******************************************************************************
* Function Name: Cy_SysClk_GetFast0Frequency
****************************************************************************//**
*
* Reports clock Fast 0 frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetFast0Frequency(uint32_t *result)
{
    uint32_t clkFastFreq;
    cy_en_sysclk_status_t status;

    status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_1,&clkFastFreq);
    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }

    uint32_t intDiv;
    uint32_t fracDiv;
    Cy_SysClk_Fast0ClkGetDivider(&intDiv, &fracDiv);

    uint64_t divNumShifted5bit = (((uint64_t)intDiv+1ull) << 5ull) + (uint64_t)fracDiv;
    *result = (uint32_t)(((uint64_t)clkFastFreq << 5ull) / divNumShifted5bit);

    return(CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetFast1Frequency
****************************************************************************//**
*
* Reports clock Fast 1 frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetFast1Frequency(uint32_t *result)
{
    uint32_t clkFastFreq;
    cy_en_sysclk_status_t status;

    status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_1,&clkFastFreq);
    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }

    uint32_t intDiv;
    uint32_t fracDiv;
    Cy_SysClk_Fast1ClkGetDivider(&intDiv, &fracDiv);

    uint64_t divNumShifted5bit = (((uint64_t)intDiv+1ull) << 5ull) + (uint64_t)fracDiv;
    *result = (uint32_t)(((uint64_t)clkFastFreq << 5ull) / divNumShifted5bit);
    return(CY_SYSCLK_SUCCESS);
}

#if defined (tviibh16m)
/*******************************************************************************
* Function Name: Cy_SysClk_GetFast2Frequency
****************************************************************************//**
*
* Reports clock Fast 2 frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetFast2Frequency(uint32_t *result)
{
    uint32_t clkFastFreq;
    cy_en_sysclk_status_t status;

    status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_1,&clkFastFreq);
    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }

    uint32_t intDiv;
    uint32_t fracDiv;
    Cy_SysClk_Fast2ClkGetDivider(&intDiv, &fracDiv);

    uint64_t divNumShifted5bit = (((uint64_t)intDiv+1ull) << 5ull) + (uint64_t)fracDiv;
    *result = (uint32_t)(((uint64_t)clkFastFreq << 5ull) / divNumShifted5bit);
    return(CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetFast3Frequency
****************************************************************************//**
*
* Reports clock Fast 3 frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetFast3Frequency(uint32_t *result)
{
    uint32_t clkFastFreq;
    cy_en_sysclk_status_t status;

    status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_1,&clkFastFreq);
    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }

    uint32_t intDiv;
    uint32_t fracDiv;
    Cy_SysClk_Fast3ClkGetDivider(&intDiv, &fracDiv);

    uint64_t divNumShifted5bit = (((uint64_t)intDiv+1ull) << 5ull) + (uint64_t)fracDiv;
    *result = (uint32_t)(((uint64_t)clkFastFreq << 5ull) / divNumShifted5bit);
    return(CY_SYSCLK_SUCCESS);
}
#endif

/*******************************************************************************
* Function Name: Cy_SysClk_GetClkPeriFrequency
****************************************************************************//**
*
* Reports clock Peri frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetClkPeriFrequency(uint32_t *result)
{
    uint32_t freqHfClk0;
    cy_en_sysclk_status_t status;

    status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_0, &freqHfClk0);
    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }
    
    *result = freqHfClk0 / (Cy_SysClk_PeriClkGetDivider() + 1ul);
    return (CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetClkMemFrequency
****************************************************************************//**
*
* Reports clock Mem frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetClkMemFrequency(uint32_t *result)
{
    uint32_t freqHfClk0;
    cy_en_sysclk_status_t status;

    status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_0, &freqHfClk0);
    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }

    *result = freqHfClk0 / (Cy_SysClk_MemClkGetDivider() + 1ul);
    return(CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetClkSlowFrequency
****************************************************************************//**
*
* Reports clock Slow frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetClkSlowFrequency(uint32_t *result)
{
    uint32_t clkMemFreq;
    cy_en_sysclk_status_t status;

    status = Cy_SysClk_GetClkMemFrequency(&clkMemFreq);
    if(status != CY_SYSCLK_SUCCESS)
    {
        return(status);
    }

    *result = clkMemFreq / (Cy_SysClk_SlowClkGetDivider() + 1ul);
    return(CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetCoreFrequency
****************************************************************************//**
*
* Reports Core frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetCoreFrequency(uint32_t *result)
{
#if CY_CPU_CORTEX_M0P
    return(Cy_SysClk_GetClkSlowFrequency(result));
#else
    if(CPUSS->unIDENTITY.stcField.u4MS == CPUSS_MS_ID_CM7_0)
    {
        return(Cy_SysClk_GetFast0Frequency(result));
    }
    #if defined(CPUSS_CM7_1_PRESENT) && (CPUSS_CM7_1_PRESENT == 1)
    else if(CPUSS->unIDENTITY.stcField.u4MS == CPUSS_MS_ID_CM7_1)
    {
        return(Cy_SysClk_GetFast1Frequency(result));
    }
    #endif
    #if defined(CPUSS_CM7_2_PRESENT) && (CPUSS_CM7_2_PRESENT == 1)
    else if(CPUSS->unIDENTITY.stcField.u4MS == CPUSS_MS_ID_CM7_2)
    {
        return(Cy_SysClk_GetFast2Frequency(result));
    }
    #endif
    #if defined(CPUSS_CM7_3_PRESENT) && (CPUSS_CM7_3_PRESENT == 1)
    else if(CPUSS->unIDENTITY.stcField.u4MS == CPUSS_MS_ID_CM7_3)
    {
        return(Cy_SysClk_GetFast3Frequency(result));
    }
    #endif
    else
    {
        return CY_SYSCLK_BAD_PARAM;
    }
#endif
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetSystickFrequency
****************************************************************************//**
*
* Reports frequency of clock for Systick timer. This function does not measure the
* frequency, just return values which is the result of calculation from register
* values and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetSystickFrequency(uint32_t *result)
{
    cy_en_systick_clock_source_t systicClkSource = Cy_SysTick_GetClockSource();

    switch(systicClkSource)
    {
    case CY_SYSTICK_CLOCK_SOURCE_CLK_LF:
        return(Cy_SysClk_GetLfFrequency(result));
    case CY_SYSTICK_CLOCK_SOURCE_CLK_IMO:
        return(Cy_SysClk_GetImoFrequency(result));
    case CY_SYSTICK_CLOCK_SOURCE_CLK_ECO:
        return(Cy_SysClk_GetEcoFrequency(result));
    case CY_SYSTICK_CLOCK_SOURCE_CLK_TIMER:
        return(Cy_SysClk_GetTimerFrequency(result));
    case CY_SYSTICK_CLOCK_SOURCE_CLK_CPU:
        return(Cy_SysClk_GetCoreFrequency(result));
    default:
        *result = 0ul;
        return(CY_SYSCLK_INVALID_STATE);
    }
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetGroupFrequency
****************************************************************************//**
*
* Reports frequency of clock of CLK_GRx. This function does not measure the
* frequency, just return values which is the result of calculation from register
* values and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetGroupFrequency(uint32_t clkGR, uint32_t *result)
{
    cy_en_sysclk_status_t status;

    if((clkGR == 5ul) || (clkGR == 6ul) || (clkGR == 9ul))
    {
         status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_2, result);
         if(status != CY_SYSCLK_SUCCESS)
         {
             *result = 0ul;
             return status;
         }
         *result = *result / (uint32_t)(Cy_SysClk_GetClkGrDiv(clkGR) + 1ul);
         return CY_SYSCLK_SUCCESS;
    }
    else if((clkGR == 3ul) || (clkGR == 4ul) || (clkGR == 8ul))
    {
        status = Cy_SysClk_GetClkPeriFrequency(result);
        if(status != CY_SYSCLK_SUCCESS)
        {
            *result = 0ul;
            return status;
        }
        *result = *result / (uint32_t)(Cy_SysClk_GetClkGrDiv(clkGR) + 1ul);
        return CY_SYSCLK_SUCCESS;
    }
    else
    {
        *result = 0ul;
        return CY_SYSCLK_BAD_PARAM;
    }
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetFlashFrequency
****************************************************************************//**
*
* Reports Flash interface frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetFlashInterfaceFrequency(uint32_t *result)
{
    uint32_t freqHfClk0;
    cy_en_sysclk_status_t status;

    status = Cy_SysClk_GetClkMemFrequency(&freqHfClk0);
    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }

    *result = freqHfClk0 / (FLASHC->unFLASH_CTL.stcField.u4WS + 1ul);
    return(CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetRamFastFrequency
****************************************************************************//**
*
* Reports SRAM clk_fast domain frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetRamFastFrequency(uint32_t ramNo, uint32_t *result)
{
    uint32_t freqHfClk0;
    cy_en_sysclk_status_t status;

    status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_0, &freqHfClk0);
    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }

    if(ramNo == 0ul)
    {
        *result = freqHfClk0 / (CPUSS->unRAM0_CTL0.stcField.u2FAST_WS + 1ul);
    }
#if defined (CPUSS_RAMC1_PRESENT) && (CPUSS_RAMC1_PRESENT == 1ul)
    else if(ramNo == 1ul)
    {
        *result = freqHfClk0 / (CPUSS->unRAM1_CTL0.stcField.u2FAST_WS + 1ul);
    }
#endif
#if defined (CPUSS_RAMC2_PRESENT) && (CPUSS_RAMC2_PRESENT == 1ul)
    else if(ramNo == 2ul)
    {
        *result = freqHfClk0 / (CPUSS->unRAM2_CTL0.stcField.u2FAST_WS + 1ul);
    }
#endif
    else
    {
        *result = 0ul;
        return (CY_SYSCLK_BAD_PARAM);
    }

    return(CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetRamSlowFrequency
****************************************************************************//**
*
* Reports SRAM clk_slow domain frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetRamSlowFrequency(uint32_t ramNo, uint32_t *result)
{
    uint32_t freqHfClk0;
    cy_en_sysclk_status_t status;

    status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_0, &freqHfClk0);
    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }

    if(ramNo == 0ul)
    {
        *result = freqHfClk0 / (CPUSS->unRAM0_CTL0.stcField.u2SLOW_WS + 1ul);
    }
#if defined (CPUSS_RAMC1_PRESENT) && (CPUSS_RAMC1_PRESENT == 1ul)
    else if(ramNo == 1ul)
    {
        *result = freqHfClk0 / (CPUSS->unRAM1_CTL0.stcField.u2SLOW_WS + 1ul);
    }
#endif
#if defined (CPUSS_RAMC2_PRESENT) && (CPUSS_RAMC2_PRESENT == 1ul)
    else if(ramNo == 2ul)
    {
        *result = freqHfClk0 / (CPUSS->unRAM2_CTL0.stcField.u2SLOW_WS + 1ul);
    }
#endif
    else
    {
        *result = 0ul;
        return (CY_SYSCLK_BAD_PARAM);
    }

    return(CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetRomFastFrequency
****************************************************************************//**
*
* Reports ROM clk_fast domain frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetRomFastFrequency(uint32_t *result)
{
    uint32_t freqHfClk0;
    cy_en_sysclk_status_t status;

    status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_0, &freqHfClk0);
    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }

    *result = freqHfClk0 / (CPUSS->unROM_CTL.stcField.u2FAST_WS + 1ul);
    return(CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_GetRomSlowFrequency
****************************************************************************//**
*
* Reports ROM clk_slow domain frequency. This function does not measure the frequency,
* just return values which is the result of calculation from register values 
* and values set by Cy_SysClk_InitGetFreqParams.
*
* \param result A pointer to variable in which the frequency value is stored.
*
* \return \cy_en_sysclk_status_t
*
* Note: Please call this function after setting base clock information 
*       by calling Cy_SysClk_InitGetFreqParams
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_GetRomSlowFrequency(uint32_t *result)
{
    uint32_t freqHfClk0;
    cy_en_sysclk_status_t status;

    status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_0, &freqHfClk0);
    if(status != CY_SYSCLK_SUCCESS)
    {
        *result = 0ul;
        return(status);
    }

    *result = freqHfClk0 / (CPUSS->unROM_CTL.stcField.u2SLOW_WS + 1ul);
    return(CY_SYSCLK_SUCCESS);
}

/** \} group_sysclk_reculc_freq_funcs */

/* ========================================================================== */
/* =====================    clk_peripherals SECTION    ====================== */
/* ========================================================================== */
/**
* \addtogroup group_sysclk_clk_peripheral_funcs
* \{
*/
/*******************************************************************************
* Function Name: Cy_SysClk_PeriphGetFrequency
****************************************************************************//**
*
* Reports the frequency of the output of a given peripheral divider.
*
* \param ipBlock \en_clk_dst_t
*
* \return The frequency, in Hz.
*
* \note
* The reported frequency may be zero, which indicates unknown. This happens if
* the source input is clk_ext, ECO, or dsi_out.
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_PeriphGetFrequency(en_clk_dst_t ipBlock, uint32_t *result)
{
    /* clk_peri divider to selected peripheral clock */
    union
    {
        struct
        {
            uint32_t frac    :5;
            uint32_t integer :16;
            uint32_t reserve :11;
        } divType16_5;
        struct
        {
            uint32_t frac    :5;
            uint32_t integer :24;
            uint32_t reserve :3;
        } divType24_5;
        uint32_t u32;
    } clkdiv = { 0ul };
    cy_en_divider_types_t dividerType;
    uint32_t              dividerNum;
    Cy_SysClk_PeriphGetAssignedDivider(ipBlock, &dividerType, &dividerNum);

    uint8_t group_no = Cy_SysClk_GetClockGroup(ipBlock);

    /* get the divider value for clk_peri to the selected peripheral clock */
    switch(dividerType)
    {
        case CY_SYSCLK_DIV_8_BIT:
        case CY_SYSCLK_DIV_16_BIT:
            clkdiv.divType16_5.integer = (uint32_t)Cy_SysClk_PeriphGetDivider(group_no, dividerType, dividerNum);
            clkdiv.divType16_5.frac    = 0ul;  /* frac = 0 means it's an integer divider */
            break;
        case CY_SYSCLK_DIV_16_5_BIT:
            {
                uint32_t integer, frac;
                Cy_SysClk_PeriphGetFracDivider(group_no, dividerType, dividerNum, &integer, &frac);
                clkdiv.divType16_5.integer = integer;
                clkdiv.divType16_5.frac    = frac;
            }
            break;
        case CY_SYSCLK_DIV_24_5_BIT:
            {
                uint32_t integer, frac;
                Cy_SysClk_PeriphGetFracDivider(group_no, dividerType, dividerNum, &integer, &frac);
                clkdiv.divType24_5.integer = integer;
                clkdiv.divType24_5.frac    = frac;
            }
            break;
        default:
            CY_ASSERT(false);
            break;
    }

    uint32_t tempFreq;
    cy_en_sysclk_status_t status;
    switch(group_no)
    {
    case 0ul:
        status = Cy_SysClk_GetClkPeriFrequency(&tempFreq);
        break;
    case 1ul:
        status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_2, &tempFreq);
        break;
    default:
        status = CY_SYSCLK_BAD_PARAM;
        tempFreq = 0ul;
        break;
    }
    if(status != CY_SYSCLK_SUCCESS)
    {
        return(status);
    }

    /* peripheral divider output frequency */
    *result = (uint32_t)(((uint64_t)tempFreq << 5ull) / (uint64_t)clkdiv.u32);
    return(CY_SYSCLK_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SysClk_PeriphSetFracDividerAuto
****************************************************************************//**
*
* Sets one of the programmable clock dividers. This function should only be used
* for fractional clock dividers.
*
* \param dividerType specifies which type of divider to use; \ref cy_en_divider_types_t
*
* \param dividerNum specifies which divider of the selected type to configure
*
* \param targetFreq the integer value for output frequency to be configured
*
* \param targetResutFreq A pointer to variable in which the actual frequency value is stored.
*
* \return \ref cy_en_sysclk_status_t
*
*******************************************************************************/
cy_en_sysclk_status_t Cy_SysClk_PeriphSetFracDividerAuto(uint32_t groupNum, cy_en_divider_types_t dividerType, uint32_t dividerNum, uint32_t targetFreq, uint32_t *targetResutFreq)
{
    /* For below 3 variables, the least significant 5 bit indicates fractional value. 
       The other 27 bit indicate integer value */
    uint64_t fixedPointPeriClk;
    uint64_t fixedPointDivNum64;
    uint32_t fixedPointDivNum;

    if(Cy_SysClk_CheckDividerExisting(groupNum, dividerType, dividerNum) == CY_DIVIDER_NOT_EXISTING)
    {
        return CY_SYSCLK_BAD_PARAM;
    }

    // Get peri clock frequency to integer part of "fixedPointPeriClk"
    {
        uint32_t tempFreq;
        cy_en_sysclk_status_t status;
        switch(groupNum)
        {
        case 0ul:
            status = Cy_SysClk_GetClkPeriFrequency(&tempFreq);
            break;
        case 1ul:
            status = Cy_SysClk_GetHfClkFrequency(CY_SYSCLK_HFCLK_2, &tempFreq);
            break;
        default:
            status = CY_SYSCLK_BAD_PARAM;
            tempFreq = 0ul;
            break;
        }

        if(status != CY_SYSCLK_SUCCESS)
        {
            return(status);
        }

        fixedPointPeriClk = (uint64_t)tempFreq << 5ull;
    }

    fixedPointDivNum64 = fixedPointPeriClk / (uint64_t)targetFreq;
    fixedPointDivNum = (uint32_t)fixedPointDivNum64;

    if (dividerType == CY_SYSCLK_DIV_16_5_BIT)
    {
        if(
              (fixedPointDivNum64 < 0x20ull) &&      // divided value is less than "1"
              (fixedPointDivNum64 > 0x00200000ull)   // divided value go over the range of 16bit value
          )
        {
            return CY_SYSCLK_BAD_PARAM;
        }

        PERI_PCLK->GR[groupNum].unDIV_16_5_CTL[dividerNum].stcField.u16INT16_DIV = ((fixedPointDivNum & 0x001FFFE0ul) >> 5ul) - 1ul;
        PERI_PCLK->GR[groupNum].unDIV_16_5_CTL[dividerNum].stcField.u5FRAC5_DIV = (fixedPointDivNum & 0x1Ful);

    }
    else if (dividerType == CY_SYSCLK_DIV_24_5_BIT)
    {
        if(
              (fixedPointDivNum64 < 0x20ull) &&      // divided value is less than "1"
              (fixedPointDivNum64 > 0x20000000ull)   // divided value go over the range of 24bit value
          )
        {
            return CY_SYSCLK_BAD_PARAM;
        }

        PERI_PCLK->GR[groupNum].unDIV_24_5_CTL[dividerNum].stcField.u24INT24_DIV = ((fixedPointDivNum & 0x1FFFFFE0ul) >> 5ul) - 1ul;
        PERI_PCLK->GR[groupNum].unDIV_24_5_CTL[dividerNum].stcField.u5FRAC5_DIV = (fixedPointDivNum & 0x1Ful);
    }
    else
    {
        return CY_SYSCLK_BAD_PARAM;
    }
    
    if(targetResutFreq != NULL)
    {
        *targetResutFreq = (uint32_t)(fixedPointPeriClk / fixedPointDivNum64);
    }

    return CY_SYSCLK_SUCCESS;
}


#if defined(__cplusplus)
}
#endif /* __cplusplus */


/* [] END OF FILE */
