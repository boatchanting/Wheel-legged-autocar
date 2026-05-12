/***************************************************************************//**
* \file cy_dp83867.c
*
* \brief Driver for Ethernet PHY DP83867
* 
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/*****************************************************************************
* Include files
*****************************************************************************/
#include "cy_project.h"
#include "cy_device_headers.h"
#include "cy_dp83867.h"

/*****************************************************************************
* Local pre-processor symbols/macros ('define')
*****************************************************************************/

/*****************************************************************************
* Global variable definitions (declared in header file with 'extern')
*****************************************************************************/


/*****************************************************************************
* Local type definitions ('typedef')
*****************************************************************************/


/*****************************************************************************
* Local function prototypes ('static')                                                                            
*****************************************************************************/


/*****************************************************************************
* Local variable definitions ('static')
*****************************************************************************/


/*****************************************************************************
* Function implementation - global ('extern') and local ('static')
*****************************************************************************/

/*******************************************************************************
* Function Name: Cy_Dp83867_Init
****************************************************************************//**
*
* \brief Initialize an object which accesses DP83867 ethernet PHY.
*
* \param pEth
* Pointer to top of ethernet IP register area.
*
* \param phyAddr
* Address of the ether PHY
*
* \param handle
* The accessing object to be initialized.
*
*******************************************************************************/
void Cy_Dp83867_Init(volatile stc_ETH_t* pEth, uint8_t phyAddr, cy_stc_dp83867_t* handle)
{
    CY_ASSERT(pEth != NULL);

    handle->pEth    = pEth;
    handle->phyAddr = phyAddr;
}

/*******************************************************************************
* Function Name: Cy_Dp83867_ReadReg
****************************************************************************//**
*
* \brief Read normal register of the DP83867 ether PHY.
*
* \param reg
* The register information \ref cy_en_dp83867_reg_info_t
*
* \param handle
* The accessing object
*
* \return
* read value
*
*******************************************************************************/
uint16_t Cy_Dp83867_ReadReg(cy_en_dp83867_reg_info_t reg, cy_stc_dp83867_t* handle)
{
    CY_ASSERT(handle->pEth != NULL);

    uint16_t addr = reg;
    return Cy_EthIf_PhyRegRead(handle->pEth, addr, handle->phyAddr);
}

/*******************************************************************************
* Function Name: Cy_Dp83867_WriteReg
****************************************************************************//**
*
* \brief Write normal register of the DP83867 ether PHY.
*
* \param reg
* The register information \ref cy_en_dp83867_reg_info_t
*
* \param data
* The value to be written to the register.
*
* \param handle
* The accessing object.
*
*******************************************************************************/
void Cy_Dp83867_WriteReg(cy_en_dp83867_reg_info_t reg, uint16_t data, cy_stc_dp83867_t* handle)
{
    CY_ASSERT(handle->pEth != NULL);

    uint16_t addr = reg;
    Cy_EthIf_PhyRegWrite(handle->pEth, addr, data, handle->phyAddr);
}

/*******************************************************************************
* Function Name: Cy_Dp83867_ReadExtendedReg
****************************************************************************//**
*
* \brief Read extended register of the DP83867 ether PHY.
*
* \param reg
* The register information \ref cy_en_dp83867_ex_reg_info_t
*
* \param handle
* The accessing object
*
* \return
* read value
*
*******************************************************************************/
uint16_t Cy_Dp83867_ReadExtendedReg(cy_en_dp83867_ex_reg_info_t reg, cy_stc_dp83867_t* handle)
{
    uint16_t addr = reg;
    Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_REGCR, 0x001F, handle);
    Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_ADDAR, addr,   handle);
    Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_REGCR, 0x401F, handle);
    return (Cy_Dp83867_ReadReg(CY_DP83867_REG_NORMAL_ADDAR,  handle));
}

/*******************************************************************************
* Function Name: Cy_Dp83867_WriteExtendedReg
****************************************************************************//**
*
* \brief Write extended register of the DP83867 ether PHY.
*
* \param reg
* The register information \ref cy_en_dp83867_ex_reg_info_t
*
* \param data
* The value to be written to the register.
*
* \param handle
* The accessing object.
*
*******************************************************************************/
void Cy_Dp83867_WriteExtendedReg(cy_en_dp83867_ex_reg_info_t reg, uint16_t data, cy_stc_dp83867_t* handle)
{
    uint16_t addr = reg;
    Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_REGCR, 0x001F, handle);
    Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_ADDAR, addr,   handle);
    Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_REGCR, 0x401F, handle);
    Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_ADDAR, data,   handle);
}


/*******************************************************************************
* Function Name: Cy_Dp83867_ConfigurePhy
****************************************************************************//**
*
* \brief Resets and configures the PHY with the provided settings.
* 
* Does not wait for link up, use Cy_Dp83867_IsLinkUp to check or poll for it 
* after calling this function
*
* \param phyCfg
* PHY configuration parameters
*
* \param handle
* The accessing object.
*
*******************************************************************************/
void Cy_Dp83867_ConfigurePhy(cy_stc_dp83867_phy_cfg_t * phyCfg,  cy_stc_dp83867_t* handle)
{
    cy_un_dp83867_bmcr_t   bmcr;
    cy_un_dp83867_bmsr_t   bmsr;
    cy_un_dp83867_physts_t physts;
    cy_un_dp83867_anar_t   anar;
    cy_un_dp83867_cfg1_t   cfg1;
    cy_un_dp83867_ctrl_t   ctrl;

    // Reset the PHY via its CTRL reg and wait for completion
    ctrl.u16 = 0;
    ctrl.f.u1SW_RESET = 1;
    Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_CTRL, ctrl.u16, handle);
    do {
        ctrl.u16 = Cy_Dp83867_ReadReg(CY_DP83867_REG_NORMAL_CTRL, handle);
    } while(ctrl.f.u1SW_RESET == 1);

    // Reset the PHY via its BMCR reg and wait for completion
    // Note: At the moment the difference between reset via CTRL and BMCR is not fully clear (e.g. BMCR does not reset IO_MUX_CFG!)
    //       So maybe BMCR is not necessary, but it is triggered anyway to be on the safe side
    bmcr.u16 = 0;
    bmcr.f.u1RESET = 1;
    Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_BMCR, bmcr.u16, handle);
    do {
        bmcr.u16 = Cy_Dp83867_ReadReg(CY_DP83867_REG_NORMAL_BMCR, handle);
    } while(bmcr.f.u1RESET == 1);

    if(phyCfg->speedMode == CY_DP83867_SPEED_1000)
    {
        // Change clock source that is output in case of 1000 MBit/s
        cy_un_dp83867_io_mux_cfg_t io_mux_cfg;
        io_mux_cfg.u16 = Cy_Dp83867_ReadExtendedReg(CY_DP83867_REG_EXTENDED_IO_MUX_CFG, handle);
        io_mux_cfg.f.u5CLK_O_SEL = 0; // Channel A receive clock (reset default is 25MHz PHY crystal / ext clock)
        Cy_Dp83867_WriteExtendedReg(CY_DP83867_REG_EXTENDED_IO_MUX_CFG, io_mux_cfg.u16, handle);
    }

    if(phyCfg->ifMode == CY_DP83867_INTERFACE_RGMII)
    {
        // Enable TX and RX clock delay
        cy_un_dp83867_rgmiictl_t rgmiictl = {0};
        rgmiictl.f.u1RGMII_RX_CLK_DELAY     = 1;
        rgmiictl.f.u1RGMII_TX_CLK_DELAY     = 1;
        rgmiictl.f.u2RGMII_TX_HALF_FULL_THR = 2;
        rgmiictl.f.u2RGMII_RX_HALF_FULL_THR = 2;
        rgmiictl.f.u1RGMII_EN               = 1;
        Cy_Dp83867_WriteExtendedReg(CY_DP83867_REG_EXTENDED_RGMIICTL, rgmiictl.u16, handle);

        // Configure actual TX and RX clock delay
        cy_un_dp83867_rgmiidctl_t rgmiidctl = {0};
        rgmiidctl.f.u4RGMII_RX_DELAY_CTRL = 6; // '6'-> 1.75ns
        rgmiidctl.f.u4RGMII_TX_DELAY_CTRL = 6; // '6'-> 1.75ns
        Cy_Dp83867_WriteExtendedReg(CY_DP83867_REG_EXTENDED_RGMIIDCTL, rgmiidctl.u16, handle);
    }
    else
    {
        // Disable RGMII
        cy_un_dp83867_rgmiictl_t rgmiictl = {0};
        rgmiictl.f.u1RGMII_EN             = 0;
        Cy_Dp83867_WriteExtendedReg(CY_DP83867_REG_EXTENDED_RGMIICTL, rgmiictl.u16, handle);        
    }


    // Disable "speed optimization" (automatic fallback from 1000 MBit/s to 100 MBit/s) because no handling is implemented that would re-initialize ETH IP
    cy_un_dp83867_cfg2_t cfg2;
    cfg2.u16 = 0x2000;
    Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_CFG2, cfg2.u16, handle);

    if(phyCfg->enableAutoNeg)
    {
        // Configure advertisement of capabilities based on user configuration 
        // (Only speed! Full duplex is mandatory because MAC does not support half-duplex)
        cfg1.u16 = 0; // Clear all capabilities
        anar.u16 = 0; // Clear all capabilities    
        anar.f.u5SELECTOR = 1; // default, required

        // Advertise symmetic pause mode capabilities (MAC accepts and can send pause frames)
        anar.f.u1PAUSE = 1;
        //anar.f.u1ASM_DIR = 1;

        if(phyCfg->speedMode == CY_DP83867_SPEED_10)
        {
            anar.f.u1_10_FD = 1;
        }
        else if(phyCfg->speedMode == CY_DP83867_SPEED_100)
        {
            anar.f.u1TX_FD = 1;
        }
        else if(phyCfg->speedMode == CY_DP83867_SPEED_1000)
        {
            cfg1.f.u1_1000BASE_T_FULL_DUPLEX = 1;
        }

        Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_ANAR, anar.u16, handle);
        Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_CFG1, cfg1.u16, handle);

        // Restart auto-negotiation and wait until it has been initiated
        bmcr.u16 = 0;
        bmcr.f.u1AUTO_NEGOTIATION_ENABLE  = 1;
        bmcr.f.u1RESTART_AUTO_NEGOTIATION = 1;
        Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_BMCR, bmcr.u16, handle);
        do {
            bmcr.u16 = Cy_Dp83867_ReadReg(CY_DP83867_REG_NORMAL_BMCR, handle);
        } while(bmcr.f.u1RESTART_AUTO_NEGOTIATION == 1);

        // Wait until auto-negotiation is complete
        while(1)
        {
            bmsr.u16   = Cy_Dp83867_ReadReg(CY_DP83867_REG_NORMAL_BMSR, handle);        
            physts.u16 = Cy_Dp83867_ReadReg(CY_DP83867_REG_NORMAL_PHYSTS, handle);        
            if((physts.f.u1SPEED_DUPLEX_RESOLVED == 1) && (bmsr.f.u1AUTO_NEGOTIATION_COMPLETE == 1))
            {
                break;
            }
        }
    }
    else
    {
        bmcr.u16 = 0;
        bmcr.f.u1DUPLEX_MODE = 1ul;
        if(phyCfg->speedMode == CY_DP83867_SPEED_10)
        {
            bmcr.f.u1SPEED_SELECTION_LSB = 0;
            bmcr.f.u1SPEED_SELECTION_MSB = 0;
        }
        else if(phyCfg->speedMode == CY_DP83867_SPEED_100)
        {
            bmcr.f.u1SPEED_SELECTION_LSB = 1;
            bmcr.f.u1SPEED_SELECTION_MSB = 0;
        }
        else if(phyCfg->speedMode == CY_DP83867_SPEED_1000)
        {
            bmcr.f.u1SPEED_SELECTION_LSB = 0;
            bmcr.f.u1SPEED_SELECTION_MSB = 1;
        }

        Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_BMCR, bmcr.u16, handle);

        // Restart the PHY via its CTRL reg and wait for completion
        ctrl.u16 = 0u;
        ctrl.f.u1SW_RESTART = 1;
        Cy_Dp83867_WriteReg(CY_DP83867_REG_NORMAL_CTRL, ctrl.u16, handle);
        do {
            ctrl.u16 = Cy_Dp83867_ReadReg(CY_DP83867_REG_NORMAL_CTRL, handle);
        } while(ctrl.f.u1SW_RESTART == 1);
    }
}


/*******************************************************************************
* Function Name: Cy_Dp83867_IsLinkUp
****************************************************************************//**
*
* \brief Function checks and returns the link status from a DP83867IR register
*
* \param handle
* The accessing object.
* 
* \return true Link up
* \return false Link Down 
*
*******************************************************************************/
bool Cy_Dp83867_IsLinkUp(cy_stc_dp83867_t* handle)
{
    cy_un_dp83867_physts_t physts;
    physts.u16 = Cy_Dp83867_ReadReg(CY_DP83867_REG_NORMAL_PHYSTS, handle);
    if ((physts.f.u1LINK_STATUS == 1) && (physts.f.u1SPEED_DUPLEX_RESOLVED == 1))
    {
        /** Link up */
        return true;
    }
    else
    {
        /** Link down   */
        return false;
    }
}


/* [] END OF FILE */
