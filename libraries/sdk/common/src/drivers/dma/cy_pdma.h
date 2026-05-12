/***************************************************************************//**
* \file cy_pdma.h
* \version 1.0
*
* \brief
* The header file of the PDMA driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_pdma Peripheral DMA (P-DMA)
* \{
* Configures a PDMA channel and its descriptor(s).
*
* The PDMA Channel component can be used in any project that needs to transfer data
* without CPU intervention, based on a hardware trigger signal from another component.
*
* A device may support more than one PDMA hardware block. Each block has a set of 
* registers, and a base hardware address. Each block also supports multiple channels. 
* Many API functions for the PDMA driver require the base hardware address and the 
* channel number. Ensure that you use the correct hardware address for the PDMA block in use.
* 
* Features:
* * Devices support up to two PDMA hardware blocks
* * Each PDMA block supports up to 16 PDMA channels
* * Supports channel descriptors in SRAM
* * Four Priority Levels for each channel
* * Byte, Half-word (2 byte), and Word (4 byte) transfers
* * Configurable source and destination addresses
*
* \section group_pdma_configuration Configuration Considerations
*
* To set up a PDMA driver you initialize a descriptor,
* intialize and enable a channel, and enable the PDMA block.
* 
* To set up the descriptor, provide the configuration parameters for the 
* descriptor in the \ref cy_stc_pdma_descr_config_t structure. Then call the 
* \ref Cy_PDMA_Descr_Init function to initialize the descriptor in SRAM. You can 
* modify the source and destination addresses dynamically by calling 
* \ref Cy_PDMA_Descr_SetSrcAddr and \ref Cy_PDMA_Descr_SetDestAddr.
* 
* To set up the PDMA channel, provide a filled \ref cy_stc_pdma_chnl_config_t 
* structure. Call the \ref Cy_PDMA_Chnl_Init function, specifying the channel
* number. Use \ref Cy_PDMA_Chnl_Enable to enable the configured PDMA channel.
*
* Call \ref Cy_PDMA_Chnl_Enable for each PDMA channel in use.
*
* <B>NOTE:</B> Even if a PDMA channel is enabled, it is not operational until 
* the PDMA block is enabled using function \ref Cy_PDMA_Enable.\n
* <B>NOTE:</B> If the PDMA descriptor is configured to generate an interrupt, 
* the interrupt must be enabled using the \ref Cy_PDMA_Chnl_SetInterruptMask 
* function for each PDMA channel.
* 
* \section group_pdma_more_information More Information.
*
* See the PDMA Component datasheet.
* See also the PDMA chapter of the device technical reference manual (TRM).
*
*
* \section group_pdma_changelog Changelog
*
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>7.3.0</td>
*     <td>group_pdma_common_macro_PDMAIntrCauses corrections </td>
*     <td>Status alignment with design</td>
*   </tr>
* </table>
*
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*

* \defgroup group_pdma_macro Macro
* \{
*   \defgroup group_pdma_common_macro_PDMAIntrCauses PDMA Interrupt Causes
* \}
*
* \defgroup group_pdma_functions Functions
* \defgroup group_pdma_data_structures Data structures
* \defgroup group_pdma_enums Enumerated types
*/

#if !defined(CY_PDMA_H)
#define CY_PDMA_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "syslib/cy_syslib.h"
#include "cy_device_headers.h"

/*
#ifndef CY_IP_M4CPUSS_PDMA
    #error "The PDMA driver is not supported on this device"
#endif
*/

#if defined(__cplusplus)
extern "C" {
#endif

/******************************************************************************
 * Macro definitions                                                          *
 ******************************************************************************/

/**
* \addtogroup group_pdma_macro
* \{
*/

/** Driver major version */
#define CY_PDMA_DRV_VERSION_MAJOR       1

/** Driver minor version */
#define CY_PDMA_DRV_VERSION_MINOR       0
    
 
/** \cond INTERNAL */
#define CY_PDMA_INTR_BIT_MASK                   (0x01UL)
/** \endcond */

/**
* \addtogroup group_pdma_common_macro_PDMAIntrCauses
* \{
*/
#define CY_PDMA_INTRCAUSE_NO_INTR                (0x00u) /**< No interrupt generated               */
#define CY_PDMA_INTRCAUSE_COMPLETION             (0x01u) /**< Interrupt based on transfer completion configuration based on INTR_TYPE  */
#define CY_PDMA_INTRCAUSE_SRC_BUS_ERROR          (0x02u) /**< Source transfer bus error            */
#define CY_PDMA_INTRCAUSE_DST_BUS_ERROR          (0x03u) /**< Destination transfer bus error       */
#define CY_PDMA_INTRCAUSE_SRC_MISAL              (0x04u) /**< Source address misalignment          */
#define CY_PDMA_INTRCAUSE_DST_MISAL              (0x05u) /**< Destination address misalignment     */
#define CY_PDMA_INTRCAUSE_CURR_PTR_NULL          (0x06u) /**< Current descriptor pointer is null   */
#define CY_PDMA_INTRCAUSE_ACTIVE_CH_DISABLED     (0x07u) /**< Active channel is disabled           */
#define CY_PDMA_INTRCAUSE_DESCR_BUS_ERROR        (0x08u) /**< Descriptor bus error                 */
/** \} group_pdma_common_macro_PDMAIntrCauses */

#define CY_PDMA_ID             (CY_PDL_DRV_ID(0x13u)) /**< PDMA PDL ID */

#define CY_PDMA_ID_INFO        (uint32_t)( CY_PDMA_ID | CY_PDL_STATUS_INFO )   /**< Return prefix for PDMA driver function status codes */
#define CY_PDMA_ID_WARNING     (uint32_t)( CY_PDMA_ID | CY_PDL_STATUS_WARNING) /**< Return prefix for PDMA driver function warning return values */
#define CY_PDMA_ID_ERROR       (uint32_t)( CY_PDMA_ID | CY_PDL_STATUS_ERROR)   /**< Return prefix for PDMA driver function error return values */

/** \} group_pdma_macro */

/**
* \addtogroup group_pdma_data_structures
* \{
*/

/* PDMA descriptor structure type. This is a redefinition of the descriptor structure in the driver.
*  The user will use this descriptor type in his descriptor configuration functions. 
*/


/**
  * \brief PDMA descriptor structure
  */
typedef struct stc_PDMA_DESCR_CTL_field {
  uint32_t                         u2WAIT_FOR_DEACT:2;
  uint32_t                         u2INTR_TYPE:2;
  uint32_t                         u2TR_OUT_TYPE:2;
  uint32_t                         u2TR_IN_TYPE:2;
  uint32_t                         :16;
  uint32_t                         u1CH_DISABLE:1;
  uint32_t                         :1;
  uint32_t                         u1SRC_TRANSFER_SIZE:1;
  uint32_t                         u1DST_TRANSFER_SIZE:1;
  uint32_t                         u2DATA_SIZE:2;
  uint32_t                         u2DESCR_TYPE:2;
} stc_PDMA_DESCR_CTL_field_t;

typedef union un_PDMA_DESCR_CTL {
  uint32_t                         u32Register;
  stc_PDMA_DESCR_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PDMA_DESCR_CTL_t;

typedef struct stc_PDMA_DESCR_X_CTL_field {
  uint32_t                         u12SRC_X_INCR:12;
  uint32_t                         u12DST_X_INCR:12;
  uint32_t                         u8X_COUNT:8;
} stc_PDMA_DESCR_X_CTL_field_t;

typedef union un_PDMA_DESCR_X_CTL {
  uint32_t                         u32Register;
  stc_PDMA_DESCR_X_CTL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PDMA_DESCR_X_CTL_t;

typedef struct stc_PDMA_DESCR_Y_CTL_field {
  uint32_t                         u12SRC_Y_INCR:12;
  uint32_t                         u12DST_Y_INCR:12;
  uint32_t                         u8Y_COUNT:8;
} stc_PDMA_DESCR_Y_CTL_field_t;

typedef union un_PDMA_DESCR_Y_CTL {
  uint32_t                         u32Register;
  stc_PDMA_DESCR_Y_CTL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PDMA_DESCR_Y_CTL_t;

typedef struct {            
    un_PDMA_DESCR_CTL_t         unPDMA_DESCR_CTL;
    uint32_t                    u32PDMA_DESCR_SRC;
    uint32_t                    u32PDMA_DESCR_DST;
    un_PDMA_DESCR_X_CTL_t       unPDMA_DESCR_X_CTL;
    un_PDMA_DESCR_Y_CTL_t       unPDMA_DESCR_Y_CTL;
    uint32_t                    u32PDMA_DESCR_NEXT_PTR;
}   cy_stc_pdma_descr_t;

/**
* This structure is a configuration structure pre-initialized by the user and
* passed as a parameter to the CY_PDMA_InitDescr(). This structure has all the parameters
* of the descriptor as separate parameters. Some of these parameters are represented in the
* stc_Descr_t data structure as bit fields.
*/
typedef struct {
    uint32_t                deact;          /**< Specifies whether the PDMA controller should wait for the input trigger to be deactivated */
    uint32_t                intrType;       /**< Sets the event that triggers an interrupt, see \ref cy_en_pdma_inttype_t     */
    uint32_t                trigoutType;    /**< Sets the event that triggers an output,    see \ref cy_en_pdma_trigouttype_t */
    uint32_t                chStateAtCmplt; /**< Specifies if the channel is enabled or disabled on completion of descriptor see \ref cy_en_pdma_chdesccmpl_t */
    uint32_t                triginType;     /**< Sets what type of transfer is triggered,   see \ref cy_en_pdma_trigintype_t  */
    uint32_t                dataSize;       /**< The size of the data bus for transfer,     see \ref cy_en_pdma_datasize_t    */
    uint32_t                srcTxfrSize;    /**< The source transfer size                */
    uint32_t                destTxfrSize;   /**< The destination transfer size           */
    uint32_t                descrType;      /**< The type of the descriptor                 see \ref cy_en_pdma_descrtype_t*/
    void *                  srcAddr;        /**< The source address of the transfer      */
    void *                  destAddr;       /**< The destination address of the transfer */
    int32_t                 srcXincr;       /**< The address increment of the source after each X-loop transfer      */
    int32_t                 destXincr;      /**< The address increment of the destination after each X-loop transfer */
    uint32_t                xCount;         /**< The number of transfers in an X-loop                                */
    int32_t                 srcYincr;       /**< The address increment of the source after each Y-loop transfer      */
    int32_t                 destYincr;      /**< The address increment of the destination after each Y-loop transfer */
    uint32_t                yCount;         /**< The number of transfers in the Y-loop */
    cy_stc_pdma_descr_t*    descrNext;      /**< The next descriptor to chain after completion, a NULL value will signify no chaining */
} cy_stc_pdma_descr_config_t;

/** This structure holds the initialization values for the PDMA channel */
typedef struct {
    cy_stc_pdma_descr_t*    PDMA_Descriptor;/**< The PDMA descriptor associated with the channel being initialized           */
    uint32_t                preemptable;    /**< Specifies if the channel is preemptable by another higher-priority channel */
    uint32_t                priority;       /**< This parameter specifies the channel's priority                            */
    uint32_t                enable;         /**< This parameter specifies if the channel is enabled after initializing      */
    uint32_t                priviledge;
    uint32_t                non_secure;
    uint32_t                bufferable;
    uint32_t                protection_context;
} cy_stc_pdma_chnl_config_t;

/** This structure holds the initialization values for the CRC feature on PDMA */
typedef struct {
    uint32_t                data_reverse;
    uint32_t                rem_reverse;
    uint8_t                 data_xor;
    uint32_t                polynomial;
    uint32_t                lfsr32;
    uint32_t                rem_xor;
} cy_stc_pdma_crc_config_t;

/** \} group_pdma_data_structures */

/**
* \addtogroup group_pdma_enums
* \{
*/

/** Contains the options for the trigger input deactivation parameter type */
enum cy_en_pdma_deactivate_t {
    CY_PDMA_TRIG_DEACT_NO_WAIT,             /**< Don't wait for deactivation (for pulse sensitive triggers)*/
    CY_PDMA_TRIG_DEACT_4CYC,                /**< Wait for up to 4 cycles */
    CY_PDMA_TRIG_DEACT_16CYC,               /**< Wait for up to 16 cycles */
    CY_PDMA_TRIG_DEACT_WAIT_INDEFINITELY,   /**< Wait indefinitely. This option may result in controller lockup if the trigger is not de-activated */
} ;

/** Contains the options for the descriptor type */
enum cy_en_pdma_descrtype_t {
    CY_PDMA_SINGLE_TRANSFER,                /**< Single transfer */
    CY_PDMA_1D_TRANSFER,                    /**< 1D transfer     */
    CY_PDMA_2D_TRANSFER,                    /**< 2D transfer     */
    CY_PDMA_CRC_TRANSFER                    /**< CRC transfer     */
} ;

/** Contains the options for the interrupt-type parameter of the descriptor */
enum cy_en_pdma_inttype_t {
    CY_PDMA_INTR_1ELEMENT_CMPLT,            /**< Triggers on every element transfer completion          */
    CY_PDMA_INTR_X_LOOP_CMPLT,              /**< Triggers on every X loop transfer completion           */
    CY_PDMA_INTR_DESCR_CMPLT,               /**< Triggers on descriptor completion                      */
    CY_PDMA_INTR_DESCRCHAIN_CMPLT           /**< Triggers on completion of the entire descriptor chain  */
} ;

/** This enum has the options for the trigger in the type parameter of the descriptor */
enum cy_en_pdma_trigouttype_t {
    CY_PDMA_TRIGOUT_1ELEMENT_CMPLT,         /**< Triggers on every element transfer completion          */
    CY_PDMA_TRIGOUT_X_LOOP_CMPLT,           /**< Triggers on every X loop transfer completion           */
    CY_PDMA_TRIGOUT_DESCR_CMPLT,            /**< Triggers on descriptor completion                      */
    CY_PDMA_TRIGOUT_DESCRCHAIN_CMPLT        /**< Triggers on completion of the entire descriptor chain  */
} ;

/** This enum has the options for the trigger-in-type parameter of the descriptor */
enum cy_en_pdma_trigintype_t {
    CY_PDMA_TRIGIN_1ELEMENT,                /**< One transfer per trigger                   */
    CY_PDMA_TRIGIN_XLOOP,                   /**< One X loop transfer per trigger            */
    CY_PDMA_TRIGIN_DESCR,                   /**< The entire descriptor transfer per trigger */
    CY_PDMA_TRIGIN_DESCRCHAIN               /**< The entire descriptor chain per trigger    */
} ;

/** This enum has the options for the data size */
enum cy_en_pdma_datasize_t {
    CY_PDMA_BYTE,                           /**< One byte               */
    CY_PDMA_HALFWORD,                       /**< Half word (two bytes)  */
    CY_PDMA_WORD                            /**< Full word (four bytes) */
} ;

/** This enum has the options for descriptor retriggering */
enum cy_en_pdma_retrigger_t {
    CY_PDMA_RETDIG_IM,                      /**< Retrigger immediately         */
    CY_PDMA_RETDIG_4CYC,                    /**< Retrigger after 4 SYSCLK       */
    CY_PDMA_RETDIG_16CYC,                   /**< Retrigger after 16 SYSCLK      */
    CY_PDMA_WAIT_FOR_REACT                  /**< Wait for trigger reactivation  */
};

/** This enum has the options for the transfer size */
enum cy_en_pdma_txfrsize_t {
    CY_PDMA_TXFR_SIZE_DATA_SIZE,            /**< As per data size register */
    CY_PDMA_TXFR_SIZE_WORD                  /**< A full word (four bytes)  */
};

/** This enum has the options for the state of the channel when the descriptor is completed   */
enum cy_en_pdma_chdesccmpl_t {
    CY_PDMA_CH_ENABLED,                    /**< Channel stays enabled  */
    CY_PDMA_CH_DISABLED                    /**< Channel is disabled */
};

/** This enum has the return values of the PDMA driver */
typedef enum 
{
    CY_PDMA_SUCCESS                  =   0x00ul,  /**< Success */
    CY_PDMA_INVALID_INPUT_PARAMETERS =   ( CY_PDMA_ID_ERROR + 1ul),  /**< Input parameters passed to PDMA API are not valid */
    CY_PDMA_ERR_UNC                  =   ( CY_PDMA_ID_ERROR + 0xFul),/**< Unknown error */
} cy_en_pdma_status_t;

/** \} group_pdma_enums */

/**
* \addtogroup group_pdma_functions
* \{
*/

//IP functions
                void                    Cy_PDMA_Enable                          (           volatile stc_DW_t *         pstcPDMA);
                void                    Cy_PDMA_Disable                         (           volatile stc_DW_t *         pstcPDMA);
                uint32_t                Cy_PDMA_GetActiveChnl                   (   const   volatile stc_DW_t *         pstcPDMA);
                void*                   Cy_PDMA_GetActiveSrcAddr                (   const   volatile stc_DW_t *         pstcPDMA);
                void*                   Cy_PDMA_GetActiveDstAddr                (   const   volatile stc_DW_t *         pstcPDMA);
                void                    Cy_PDMA_CRC_Config                      (           volatile stc_DW_t *         pstcPDMA,
                                                                                    const   cy_stc_pdma_crc_config_t*   pstcCrcConfig);
                uint32_t                Cy_PDMA_GetCrcRemainderResult           (           volatile stc_DW_t *         pstcPDMA);

//Descriptor functions
                cy_en_pdma_status_t     Cy_PDMA_Descr_Init                      (           cy_stc_pdma_descr_t*        descriptor, 
                                                                                    const   cy_stc_pdma_descr_config_t* config);
                void                    Cy_PDMA_Descr_DeInit                    (           cy_stc_pdma_descr_t*        descriptor);
__STATIC_INLINE void                    Cy_PDMA_Descr_SetSrcAddr                (           cy_stc_pdma_descr_t*        descriptor, 
                                                                                    const   void *                      srcAddr);
__STATIC_INLINE void                    Cy_PDMA_Descr_SetDestAddr               (           cy_stc_pdma_descr_t*        descriptor, 
                                                                                    const   void *                      dstAddr);
                void                    Cy_PDMA_Descr_SetNxtDescr               (           cy_stc_pdma_descr_t*        descriptor, 
                                                                                    const   cy_stc_pdma_descr_t*        nxtDescriptor);
__STATIC_INLINE void                    Cy_PDMA_Descr_SetIntrType               (           cy_stc_pdma_descr_t*        descriptor, 
                                                                                            uint32_t                    interruptType);
__STATIC_INLINE void                    Cy_PDMA_Descr_SetTrigInType             (           cy_stc_pdma_descr_t*        descriptor, 
                                                                                            uint32_t                    trigInType);
__STATIC_INLINE void                    Cy_PDMA_Descr_SetTrigOutType            (           cy_stc_pdma_descr_t*        descriptor, 
                                                                                            uint32_t                    trigOutType);
                void                    Cy_PDMA_Descr_SetTxfrWidth              (           cy_stc_pdma_descr_t*        descriptor, 
                                                                                            uint32_t                    dataElementSize, 
                                                                                            uint32_t                    srcTxfrWidth, 
                                                                                            uint32_t                    dstTxfrWidth);
//Channel functions
                cy_en_pdma_status_t     Cy_PDMA_Chnl_Init                       (           volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum, 
                                                                                    const   cy_stc_pdma_chnl_config_t*  chnlConfig);
                void                    Cy_PDMA_Chnl_DeInit                     (           volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE void                    Cy_PDMA_Chnl_SetDescr                   (           volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum, 
                                                                                    const   cy_stc_pdma_descr_t*        descriptor);
__STATIC_INLINE void                    Cy_PDMA_Chnl_Enable                     (           volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE void                    Cy_PDMA_Chnl_Disable                    (           volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE void                    Cy_PDMA_Chnl_SetPriority                (           volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum, 
                                                                                            uint32_t                    priority);
__STATIC_INLINE uint32_t                Cy_PDMA_Chnl_GetPriority                (   const   volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE cy_stc_pdma_descr_t*    Cy_PDMA_Chnl_GetCurrentDescr            (   const   volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE uint32_t                Cy_PDMA_Chnl_GetInterruptStatus         (   const   volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE uint32_t                Cy_PDMA_Chnl_GetInterruptCause          (   const   volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum);
                void                    Cy_PDMA_Chnl_ClearInterrupt             (           volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE void                    Cy_PDMA_Chnl_SetInterrupt               (           volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE uint32_t                Cy_PDMA_Chnl_GetInterruptMask           (   const   volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE void                    Cy_PDMA_Chnl_SetInterruptMask           (           volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE uint32_t                Cy_PDMA_Chnl_GetInterruptStatusMasked   (   const   volatile stc_DW_t *         pstcPDMA, 
                                                                                            uint32_t                    chNum);
#if defined(CPUSS_SW_TR_PRESENT) && (CPUSS_SW_TR_PRESENT == 1)
__STATIC_INLINE void                    Cy_PDMA_Chnl_SetSwTrigger               (           volatile stc_DW_t *         pstcPDMA,
                                                                                            uint32_t                    chNum);
#endif

/***************************************
*    In-line Function Implementation
***************************************/

/*******************************************************************************
* Function Name: Cy_PDMA_Descr_SetSrcAddr
****************************************************************************//**
*
* \brief Sets the source address parameter for the descriptor structure.
*
* This function will operate on the descriptor only if it was defined as a RAM
* variable.
* If the descriptor is being defined in Flash, then the user has to use the
* configuration defines to initialize it.
*
* \param descriptor
* This is a descriptor-structure instance declared by the user. The function
* modifies the source-address parameter in this descriptor.
*
* \param srcAddr
* The source address value for the descriptor.
*
*******************************************************************************/
__STATIC_INLINE void Cy_PDMA_Descr_SetSrcAddr(cy_stc_pdma_descr_t* descriptor, const void * srcAddr)
{
    descriptor->u32PDMA_DESCR_SRC = (uint32_t)srcAddr;
}

/*******************************************************************************
* Function Name: Cy_PDMA_Descr_SetDestAddr
****************************************************************************//**
*
* \brief Sets the destination address parameter for the descriptor structure.
*
* This function will operate in the descriptor only if it was defined as a RAM
* variable.
* If the descriptor is being defined in Flash, then the user has to use the
* configuration defines to initialize it.
*
* \param descriptor
* This is a descriptor-structure instance declared by the user. The function
* modifies the destination-address parameter in this descriptor.
*
* \param dstAddr
* The destination address value for the descriptor.
*
*******************************************************************************/
__STATIC_INLINE void Cy_PDMA_Descr_SetDestAddr(cy_stc_pdma_descr_t* descriptor, const void * dstAddr)
{
    descriptor->u32PDMA_DESCR_DST = (uint32_t)dstAddr;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_SetDescr
****************************************************************************//**
*
* \brief Sets a descriptor as current for specified PDMA channel.
*
* \param pstcPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
* \param descriptor
* This is the descriptor which will be associated with the channel.
*
*******************************************************************************/
__STATIC_INLINE void Cy_PDMA_Chnl_SetDescr(volatile stc_DW_t *pstcPDMA, uint32_t chNum, const cy_stc_pdma_descr_t* descriptor)
{
    pstcPDMA->CH_STRUCT[chNum].unCH_CURR_PTR.u32Register = (uint32_t)descriptor;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_Enable
****************************************************************************//**
*
* The function is used to enable a PDMA channel.
*
* \param pstcPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
*******************************************************************************/
__STATIC_INLINE void Cy_PDMA_Chnl_Enable(volatile stc_DW_t *pstcPDMA, uint32_t chNum)
{
    pstcPDMA->CH_STRUCT[chNum].unCH_CTL.stcField.u1ENABLED = 1ul;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_Disable
****************************************************************************//**
*
* The function is used to disable a PDMA channel.
*
* \param pstcPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
*******************************************************************************/
__STATIC_INLINE void Cy_PDMA_Chnl_Disable(volatile stc_DW_t *pstcPDMA, uint32_t chNum)
{
    pstcPDMA->CH_STRUCT[chNum].unCH_CTL.stcField.u1ENABLED = 0ul;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_SetPriority
****************************************************************************//**
*
* The function is used to set a priority for the PDMA channel.
*
* \param pstcPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
* \param priority
* The priority to be set for the PDMA channel. The allowed values are 0,1,2,3.
*
*******************************************************************************/
__STATIC_INLINE void Cy_PDMA_Chnl_SetPriority(volatile stc_DW_t *pstcPDMA, uint32_t chNum, uint32_t priority)
{
    pstcPDMA->CH_STRUCT[chNum].unCH_CTL.stcField.u2PRIO = priority;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_GetPriority
****************************************************************************//**
*
* Returns the priority of PDMA channel.
*
* \param pstcPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
* \return
* The priority of the channel.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_PDMA_Chnl_GetPriority(const volatile stc_DW_t *pstcPDMA, uint32_t chNum)
{
    return pstcPDMA->CH_STRUCT[chNum].unCH_CTL.stcField.u2PRIO;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_GetCurrentDescr
****************************************************************************//**
*
* This function returns the descriptor that is active in the channel.
*
* \param pstcPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
* \return
* The pointer to the descriptor associated with the channel.
*
*******************************************************************************/
__STATIC_INLINE cy_stc_pdma_descr_t* Cy_PDMA_Chnl_GetCurrentDescr(const volatile stc_DW_t *pstcPDMA, uint32_t chNum)
{
    return (cy_stc_pdma_descr_t*) pstcPDMA->CH_STRUCT[chNum].unCH_CURR_PTR.u32Register;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Descr_SetIntrType
****************************************************************************//**
*
* Sets the interrupt type parameter for the descriptor structure.
* This function will operate in the descriptor only if it was defined as a RAM
* variable.
* If the descriptor is being defined in Flash, then the user has to use the
* configuration defines to initialize it.
*
* \param descriptor
* This is the descriptor-structure instance declared by the user. This is the
* descriptor being modified.
*
* \param interruptType
* The interrupt type set for the descriptor.
*
*******************************************************************************/
__STATIC_INLINE void Cy_PDMA_Descr_SetIntrType(cy_stc_pdma_descr_t* descriptor, uint32_t interruptType)
{
    descriptor->unPDMA_DESCR_CTL.stcField.u2INTR_TYPE     =       interruptType;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Descr_SetTrigInType
****************************************************************************//**
*
* Sets the trigger-in-type parameter for the descriptor structure.
* This function will operate in the descriptor only if it was defined as a RAM
* variable.
* If the descriptor is being defined in Flash, then the user has to use the
* configuration defines to initialize it.
*
* \param descriptor
* This is a descriptor structure instance declared by the user. This is the
* descriptor being modified.
*
* \param trigInType
* The trigger-in-type set for the descriptor. \ref cy_en_pdma_trigintype_t
*
*******************************************************************************/
__STATIC_INLINE void Cy_PDMA_Descr_SetTrigInType(cy_stc_pdma_descr_t* descriptor, uint32_t trigInType)
{
    descriptor->unPDMA_DESCR_CTL.stcField.u2TR_IN_TYPE     =       trigInType;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Descr_SetTrigOutType
****************************************************************************//**
*
* This function sets the trigger-out-type parameter for for the descriptor
* structure.
* This function will operate in the descriptor only if it was defined as a RAM
* variable.
* If the descriptor is being defined in Flash, then the user has to use the
* configuration defines to initialize it.
*
* \param descriptor
* This is a descriptor-structure instance declared by the user. This is the
* descriptor being modified.
*
* \param trigOutType
* The trigger-out-type set for the descriptor. \ref cy_en_pdma_trigouttype_t
*
*******************************************************************************/
__STATIC_INLINE void Cy_PDMA_Descr_SetTrigOutType(cy_stc_pdma_descr_t* descriptor, uint32_t trigOutType)
{
    descriptor->unPDMA_DESCR_CTL.stcField.u2TR_OUT_TYPE     =       trigOutType;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_GetInterruptStatus
****************************************************************************//**
*
* This function can be used to determine the interrupt status of the
* PDMA channel.
*
* \param pstcPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
* \return
* Returns a status of an interrupt for specified channel.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_PDMA_Chnl_GetInterruptStatus(const volatile stc_DW_t *pstcPDMA, uint32_t chNum)
{
    return (pstcPDMA->CH_STRUCT[chNum].unINTR.u32Register);
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_GetInterruptCause
****************************************************************************//**
*
* This function can be used to determine the interrupt reason of the
* PDMA channel.
*
* \param pstcPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
* \return
* Returns a status of an interrupt for specified channel.
* (see \ref group_pdma_common_macro_PDMAIntrCauses for return values).
* For error related interrupt causes (INTR_CAUSE is "1", "2", "3", ..., "8"),
* the channel is disabled (HW sets CH_CTL.ENABLED to '0').
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_PDMA_Chnl_GetInterruptCause(const volatile stc_DW_t *pstcPDMA, uint32_t chNum)
{
    return (pstcPDMA->CH_STRUCT[chNum].unCH_STATUS.stcField.u4INTR_CAUSE);
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_SetInterrupt
****************************************************************************//**
*
* This function sets the interrupt.
*
* \param pstcPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
*******************************************************************************/
__STATIC_INLINE void Cy_PDMA_Chnl_SetInterrupt(volatile stc_DW_t *pstcPDMA, uint32_t chNum)
{
    pstcPDMA->CH_STRUCT[chNum].unINTR_SET.u32Register = CY_PDMA_INTR_BIT_MASK;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_GetInterruptMask
****************************************************************************//**
*
* This function gets interrupt mask value.
*
* \param pstcPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
* \return
* Returns an interrupt mask value.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_PDMA_Chnl_GetInterruptMask(const volatile stc_DW_t *pstcPDMA, uint32_t chNum)
{
    return (pstcPDMA->CH_STRUCT[chNum].unINTR_MASK.u32Register);
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_SetInterruptMask
****************************************************************************//**
*
* This function sets interrupt mask value.
*
* \param pstcPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
*******************************************************************************/
__STATIC_INLINE void Cy_PDMA_Chnl_SetInterruptMask(volatile stc_DW_t *pstcPDMA, uint32_t chNum)
{
    pstcPDMA->CH_STRUCT[chNum].unINTR_MASK.u32Register = CY_PDMA_INTR_BIT_MASK;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_GetInterruptStatusMasked
****************************************************************************//**
*
* This function returns logical and of corresponding INTR and INTR_MASK fields
* in a single load operation.
*
* \param pstcPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_PDMA_Chnl_GetInterruptStatusMasked(const volatile stc_DW_t *pstcPDMA, uint32_t chNum)
{
    return (pstcPDMA->CH_STRUCT[chNum].unINTR_MASKED.u32Register);
}


#if defined(CPUSS_SW_TR_PRESENT) && (CPUSS_SW_TR_PRESENT == 1)

/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_SetSwTrigger
****************************************************************************//**
*
* This function sets the SW trigger for a channel.
*
* \param pstcPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
* \return
* Returns an interrupt mask value.
*
*******************************************************************************/
__STATIC_INLINE void Cy_PDMA_Chnl_SetSwTrigger(volatile stc_DW_t *pstcPDMA, uint32_t chNum)
{
    pstcPDMA->CH_STRUCT[chNum].unTR_CMD.stcField.u1ACTIVATE = 1ul;
}

#endif


/** \} group_pdma_functions */

#if defined(__cplusplus)
}
#endif

#endif  /* (CY_PDMA_H) */

/** \} group_pdma */


/* [] END OF FILE */
