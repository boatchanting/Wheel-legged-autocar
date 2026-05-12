/***************************************************************************//**
* \file cy_project.h
*
* \brief
* Project specific header
*
* \note
* It contains references to all generated header files and should 
* not be modified.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_PROJECT_H
#define CY_PROJECT_H

#ifdef __cplusplus
extern "C" {
#endif


#include "syslib/cy_syslib.h"

#include "cpu/cy_cpu.h"
#include "gpio/cy_gpio.h"
#include "ipc/cy_ipc_drv.h"
#include "ipc/cy_ipc_pipe.h"
#include "ipc/cy_ipc_sema.h"
#include "ipc/cy_ipc_config.h"
#include "mcwdt/cy_mcwdt.h"
#include "prot/cy_prot.h"
#include "scb/cy_scb_common.h"
#include "scb/cy_scb_ezi2c.h"
#include "scb/cy_scb_i2c.h"
#include "scb/cy_scb_spi.h"
#include "scb/cy_scb_uart.h"
#include "sysint/cy_sysint.h"
#include "sysclk/cy_sysclk.h"
#include "syswdt/cy_syswdt.h"
#include "sysrtc/cy_sysrtc.h"
#include "tcpwm/cy_tcpwm.h"
#include "tcpwm/cy_tcpwm_counter.h"
#include "tcpwm/cy_tcpwm_pwm.h"
#include "tcpwm/cy_tcpwm_quaddec.h"
#include "tcpwm/cy_tcpwm_sr.h"
#include "syspm/cy_syspm.h"
#include "syslib/cy_syslib.h"
#include "trigmux/cy_trigmux.h"
#include "systick/cy_systick.h"
#include "mpu/cy_mpu.h"
#include "dma/cy_pdma.h"
#include "dma/cy_mdma.h"
#include "adc/cy_adc.h"
#include "crypto/cy_crypto.h"
#include "crypto/cy_crypto_server.h"
#include "lin/cy_lin.h"
#include "canfd/cy_canfd.h"
#include "evtgen/cy_evtgen.h"
#include "flash/cy_flash.h"
#include "sysflt/cy_sysflt.h"
#include "lvd/cy_lvd.h"
#include "smartio/cy_smartio.h"
#include "sysreset/cy_sysreset.h"
#include "srom/cy_srom.h"
  
#if defined (tviibe2m) || defined (tviibe4m) || defined (tviic2d6m) || defined (tviic2d4m) || defined (tviic2d6mddr) || defined (tviice4m)
    #include "cxpi/cy_cxpi.h"
#endif

#if defined (tviibh8m) || defined (tviibh16m)
    #include "flexray/cy_fr.h"
    #include "sd_host/cy_sd_host.h"
    #include "audioss/cy_i2s.h"
    #include "smif/ver2/cy_smif_ver_specific.h"
    #include "smif/common/cy_smif_memslot.h"
    #include "ethernet/cy_ethif.h"
    #include "sysreghc/cy_sysreghc.h"
#endif

#if defined (tviibh4m)
    #include "sd_host/cy_sd_host.h"
    #include "audioss/cy_i2s.h"
    #include "smif/ver2/cy_smif_ver_specific.h"
    #include "smif/common/cy_smif_memslot.h"
    #include "ethernet/cy_ethif.h"
    #include "sysreghc/cy_sysreghc.h"
#endif

#if defined (tviic2d6m)
    #include "audioss/mixer/cy_mixer.h"
    #include "audioss/pwm/cy_pwm.h"
    #include "audioss/dac/cy_dac.h"
    #include "audioss/tdm/cy_tdm.h"
    #include "audioss/sg/cy_sg.h"
    
    #include "audioss/i2s/cy_i2s.h"
    #include "axidma/cy_axidma.h"
    #include "smif/ver4/cy_smif_ver_specific.h"
    #include "smif/common/cy_smif_memslot.h"

    #include "fpdlink/cy_fpdlink.h"
    #include "mipicsi2/cy_mipicsi2.h"    
    #include "syspmic/cy_syspmic.h"
    #include "ethernet/cy_ethif.h"
#endif
  
#if defined (tviic2d4m)
    #include "audioss/mixer/cy_mixer.h"
    #include "audioss/i2s/cy_i2s.h"
    #include "audioss/pwm/cy_pwm.h"
    #include "audioss/dac/cy_dac.h"
    #include "audioss/tdm/cy_tdm.h"
    #include "audioss/sg/cy_sg.h"
    #include "axidma/cy_axidma.h"
    #include "smif/ver3_1/cy_smif_ver_specific.h"
    #include "smif/common/cy_smif_memslot.h"
    #include "fpdlink/cy_fpdlink.h"
    #include "mipicsi2/cy_mipicsi2.h"    
    #include "syspmic/cy_syspmic.h"
    #include "ethernet/cy_ethif.h"
    #include "audio/cy_cs42448.h"
#endif

#if defined (tviic2d6mddr)
    #include "audioss/mixer/cy_mixer.h"
    #include "audioss/i2s/cy_i2s.h"
    #include "audioss/pwm/cy_pwm.h"
    #include "audioss/dac/cy_dac.h"
    #include "audioss/tdm/cy_tdm.h"
    #include "audioss/sg/cy_sg.h"
    #include "axidma/cy_axidma.h"
    #include "smif/ver4/cy_smif_ver_specific.h"
    #include "smif/common/cy_smif_memslot.h"
    #include "fpdlink/cy_fpdlink.h"
    #include "mipicsi2/cy_mipicsi2.h"    
    #include "syspmic/cy_syspmic.h"
    #include "ethernet/cy_ethif.h"
    #include "sd_host/cy_sd_host.h"
#endif

#if defined (tviice4m)
    #include "audioss/mixer/cy_mixer.h"
    #include "audioss/i2s/cy_i2s.h"
    #include "audioss/pwm/cy_pwm.h"
    #include "audioss/tdm/cy_tdm.h"
    #include "audioss/sg/cy_sg.h"
    #include "audio/cy_cs42448.h"
    #include "smif/ver3_1/cy_smif_ver_specific.h"
    #include "smif/common/cy_smif_memslot.h"
    #include "seglcd/cy_seglcd.h"
#endif
  
//=============================================

#if defined (tviibe512k)
    #include "bb_bsp_tviibe512k.h"
#elif defined (tviibe1m)
    #include "bb_bsp_tviibe1m.h"
#elif defined (tviibe2m)
    #include "bb_bsp_tviibe2m.h"
#elif defined (tviibe4m)
    #include "bb_bsp_tviibe4m.h"
#elif defined (tviibh4m)
    #include "bb_bsp_tviibh4m.h"
#elif defined (tviibh8m)
    #include "bb_bsp_tviibh8m.h"
#elif defined (tviibh16m)
    #include "bb_bsp_tviibh16m.h"
#elif defined (tviic2d4m)
    #include "bb_bsp_tviic2d4m.h"
#elif defined (tviic2d6m)
    #include "bb_bsp_tviic2d6m.h"
#elif defined (tviic2d6mddr)
    #include "bb_bsp_tviic2d6mddr.h"
#elif defined (tviice4m)
    #include "bb_bsp_tviice4m.h"
#endif

//============ Middleware =====================

#include "button/cy_button.h"
#include "semihosting/cy_semihosting.h"
#include "sw_timer/cy_sw_tmr.h" 
#include "flash/cy_mw_flash.h" 

#if defined (tviibh4m) || defined (tviibh8m) || defined (tviibh16m)
    #include "mw/eth_phy/cy_dp83867.h"
    #include "mw/power/cy_power.h"
    #include "mw/smif_mem/cy_smif_device_common.h"
    #include "mw/smif_mem/cy_smif_hb_flash.h"
    #include "mw/smif_mem/cy_smif_s25fl.h"
    #include "mw/smif_mem/cy_smif_semp.h"
    #include "mw/smif_mem/cy_smif_s25fs.h"
    #include "aic261/cy_aic261.h"
#endif
#if defined (tviic2d4m) || defined (tviic2d6m) || defined (tviic2d6mddr)
    #include "mw/smif_mem/cy_smif_device_common.h"
    #include "mw/smif_mem/cy_smif_hb_flash.h"
    #include "mw/smif_mem/cy_smif_s25fl.h"
    #include "mw/smif_mem/cy_smif_semp.h"
    #include "mw/smif_mem/cy_smif_s25fs.h"
    #include "mw/power/cy_power.h"
    #include "mw/gfx_env/cy_gfx_env.h"
    #include "mw/mipi_sensor/cy_mipi_sensor.h"
    #include "mw/audio/cy_cs42448.h"
#endif
    
#if defined (tviic2d4m)
    #include "mw/eth_phy/cy_dp83848q_q1.h"
#endif

#if defined (tviic2d6m) || defined (tviic2d6mddr)
    #include "mw/eth_phy/cy_dp83867.h"
#endif

#if defined (tviice4m)
    #include "mw/smif_mem/cy_smif_device_common.h"
    #include "mw/smif_mem/cy_smif_hb_flash.h"
    #include "mw/smif_mem/cy_smif_s25fl.h"
    #include "mw/smif_mem/cy_smif_semp.h"
    #include "mw/smif_mem/cy_smif_s25fs.h"
    #include "aic261/cy_aic261.h"
#endif

#if defined (tviic2d4m) || defined (tviice4m) || (defined (tviic2d6m) && defined (CY_MCU_rev_b)) || (defined (tviic2d6m) && defined (CY_MCU_rev_c))
    #include "mw/audio/cy_cs42448.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* CY_PROJECT_H */

/* [] END OF FILE */
