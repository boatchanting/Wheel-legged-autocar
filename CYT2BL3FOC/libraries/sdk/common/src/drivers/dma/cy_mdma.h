/***************************************************************************//**
* \file cy_mdma.h
* \version 1.0
*
* \brief
* The header file of the MDMA driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_mdma Memory DMA (M-DMA)
* \{
* Configures a MDMA channel and its descriptor(s).
*
* The MDMA Channel component can be used in any project that needs to transfer data
* without CPU intervention, based on a hardware trigger signal from another component.
*
* A device may support more than one MDMA hardware block. Each block has a set of 
* registers, and a base hardware address. Each block also supports multiple channels. 
* Many API functions for the MDMA driver require the base hardware address and the 
* channel number. Ensure that you use the correct hardware address for the MDMA block in use.
* 
* Features:
* * Devices support one MDMA hardware block
* * Each MDMA block supports up to 16 MDMA channels
* * Supports channel descriptors in SRAM
* * Four Priority Levels for each channel
* * Byte, Half-word (2 byte), and Word (4 byte) transfers
* * Configurable source and destination addresses
*
* A simplified diagram of the M-DMA block is shown below:
* \image html dma.png
*
* \section group_mdma_configuration Configuration Considerations
*
* To set up a MDMA driver you initialize a descriptor,
* initialize and enable a channel, and enable the MDMA block.
* 
* To set up the descriptor, provide the configuration parameters for the 
* descriptor in the \ref cy_stc_mdma_descr_config_t structure. Then call the 
* \ref Cy_MDMA_Descr_Init function to initialize the descriptor in SRAM. You can 
* modify the source and destination addresses dynamically by calling 
* \ref Cy_MDMA_Descr_SetSrcAddr and \ref Cy_MDMA_Descr_SetDestAddr.
* 
* To set up the MDMA channel, provide a filled \ref cy_stc_mdma_chnl_config_t 
* structure. Call the \ref Cy_MDMA_Chnl_Init function, specifying the channel
* number. Use \ref Cy_MDMA_Chnl_Enable to enable the configured MDMA channel.
*
* Call \ref Cy_MDMA_Chnl_Enable for each MDMA channel in use.
*
* <B>NOTE:</B> Even if a MDMA channel is enabled, it is not operational until 
* the MDMA block is enabled using function \ref Cy_MDMA_Enable.\n
* <B>NOTE:</B> If the MDMA descriptor is configured to generate an interrupt, 
* the interrupt must be enabled using the \ref Cy_MDMA_Chnl_SetInterruptMask 
* function for each MDMA channel.
* 
* \section group_mdma_more_information More Information.
*
* See the MDMA Component datasheet.
* See also the MDMA chapter of the device technical reference manual (TRM).
*
*
* \section group_mdma_changelog Changelog
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

* \defgroup group_mdma_macro Macro
* \{
*   \defgroup group_mdma_common_macro_MDMAIntrCauses MDMA Interrupt Causes
* \}
*
* \defgroup group_mdma_functions Functions
* \defgroup group_mdma_data_structures Data structures
* \defgroup group_mdma_enums Enumerated types
*/

#if !defined(CY_MDMA_H)
#define CY_MDMA_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "syslib/cy_syslib.h"
#include "cy_device_headers.h"

/*
#ifndef CY_IP_M4CPUSS_MDMA
    #error "The MDMA driver is not supported on this device"
#endif
*/

#if defined(__cplusplus)
extern "C" {
#endif

/******************************************************************************
 * Macro definitions                                                          *
 ******************************************************************************/

/**
* \addtogroup group_mdma_macro
* \{
*/

/** Driver major version */
#define CY_MDMA_DRV_VERSION_MAJOR       1

/** Driver minor version */
#define CY_MDMA_DRV_VERSION_MINOR       0
    
 
/** \cond INTERNAL */
#define CY_MDMA_INTR_BIT_MASK                   (0x01UL)
/** \endcond */

/**
* \addtogroup group_mdma_common_macro_MDMAIntrCauses
* \{
*/
#define CY_MDMA_INTRCAUSE_NO_INTR                (0x00u) /**< No interrupt                         */
#define CY_MDMA_INTRCAUSE_COMPLETION             (0x01u) /**< Completion                           */
#define CY_MDMA_INTRCAUSE_SRC_BUS_ERROR          (0x02u) /**< Source bus error                     */
#define CY_MDMA_INTRCAUSE_DST_BUS_ERROR          (0x04u) /**< Destination bus error                */
#define CY_MDMA_INTRCAUSE_SRC_MISAL              (0x08u) /**< Source address is not aligned        */
#define CY_MDMA_INTRCAUSE_DST_MISAL              (0x10u) /**< Destination address is not aligned   */
#define CY_MDMA_INTRCAUSE_CURR_PTR_NULL          (0x20u) /**< Current descriptor pointer is NULL   */
#define CY_MDMA_INTRCAUSE_ACTIVE_CH_DISABLED     (0x40u) /**< Active channel is disabled           */
#define CY_MDMA_INTRCAUSE_DESCR_BUS_ERROR        (0x80u) /**< Descriptor bus error                 */
/** \} group_mdma_common_macro_MDMAIntrCauses */

#define CY_MDMA_ID             (CY_PDL_DRV_ID(0x13u)) /**< MDMA PDL ID */

#define CY_MDMA_ID_INFO        (uint32_t)( CY_MDMA_ID | CY_PDL_STATUS_INFO )   /**< Return prefix for MDMA driver function status codes */
#define CY_MDMA_ID_WARNING     (uint32_t)( CY_MDMA_ID | CY_PDL_STATUS_WARNING) /**< Return prefix for MDMA driver function warning return values */
#define CY_MDMA_ID_ERROR       (uint32_t)( CY_MDMA_ID | CY_PDL_STATUS_ERROR)   /**< Return prefix for MDMA driver function error return values */

/** \} group_mdma_macro */

/**
* \addtogroup group_mdma_data_structures
* \{
*/

/* MDMA descriptor structure type. This is a redefinition of the descriptor structure in the driver.
*  The user will use this descriptor type in his descriptor configuration functions. 
*/


/**
  * \brief MDMA descriptor structure
  */
typedef struct stc_MDMA_DESCR_CTL_field {
  uint32_t                         u2WAIT_FOR_DEACT:2;
  uint32_t                         u2INTR_TYPE:2;
  uint32_t                         u2TR_OUT_TYPE:2;
  uint32_t                         u2TR_IN_TYPE:2;
  uint32_t                         u1DATA_PREFETCH:1;
  uint32_t                         :7;
  uint32_t                         u2DATA_SIZE:2;
  uint32_t                         :6;
  uint32_t                         u1CH_DISABLE:1;
  uint32_t                         :1;
  uint32_t                         u1SRC_TRANSFER_SIZE:1;
  uint32_t                         u1DST_TRANSFER_SIZE:1;
  uint32_t                         u3DESCR_TYPE:3;
  uint32_t                         :1;
} stc_MDMA_DESCR_CTL_field_t;

typedef union un_MDMA_DESCR_CTL {
  uint32_t                         u32Register;
  stc_MDMA_DESCR_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MDMA_DESCR_CTL_t;

typedef struct stc_MDMA_DESCR_X_SIZE_field {
  uint32_t                         u16X_COUNT:16;
  uint32_t                         :16;
} stc_MDMA_DESCR_X_SIZE_field_t;

typedef union un_MDMA_DESCR_X_SIZE {
  uint32_t                         u32Register;
  stc_MDMA_DESCR_X_SIZE_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MDMA_DESCR_X_SIZE_t;

typedef struct stc_MDMA_DESCR_X_INCR_field {
  uint32_t                         u16SRC_X_INCR:16;
  uint32_t                         u16DST_X_INCR:16;
} stc_MDMA_DESCR_X_INCR_field_t;

typedef union un_MDMA_DESCR_X_INCR {
  uint32_t                         u32Register;
  stc_MDMA_DESCR_X_INCR_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MDMA_DESCR_X_INCR_t;

typedef struct stc_MDMA_DESCR_Y_SIZE_field {
  uint32_t                         u16Y_COUNT:16;
  uint32_t                         :16;
} stc_MDMA_DESCR_Y_SIZE_field_t;

typedef union un_MDMA_DESCR_Y_SIZE {
  uint32_t                         u32Register;
  stc_MDMA_DESCR_Y_SIZE_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MDMA_DESCR_Y_SIZE_t;

typedef struct stc_MDMA_DESCR_Y_INCR_field {
  uint32_t                         u16SRC_Y_INCR:16;
  uint32_t                         u16DST_Y_INCR:16;
} stc_MDMA_DESCR_Y_INCR_field_t;

typedef union un_MDMA_DESCR_Y_INCR {
  uint32_t                         u32Register;
  stc_MDMA_DESCR_Y_INCR_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MDMA_DESCR_Y_INCR_t;

typedef struct {
  un_MDMA_DESCR_CTL_t              unMDMA_DESCR_CTL;
  uint32_t                         u32MDMA_DESCR_SRC;
  uint32_t                         u32MDMA_DESCR_DST;
  un_MDMA_DESCR_X_SIZE_t           unMDMA_DESCR_X_SIZE;
  un_MDMA_DESCR_X_INCR_t           unMDMA_DESCR_X_INCR;
  un_MDMA_DESCR_Y_SIZE_t           unMDMA_DESCR_Y_SIZE;
  un_MDMA_DESCR_Y_INCR_t           unMDMA_DESCR_Y_INCR;
  uint32_t                         u32MDMA_DESCR_NEXT_PTR;
} cy_stc_mdma_descr_t;

/**
* This structure is a configuration structure pre-initialized by the user and
* passed as a parameter to the Cy_MDMA_InitDescr(). This structure has all the parameters
* of the descriptor as separate parameters. Some of these parameters are represented in the
* stc_Descr_t data structure as bit fields.
*/
typedef struct {
    uint32_t                deact;          /**< Specifies whether the MDMA controller should wait for the input trigger to be deactivated */
    uint32_t                intrType;       /**< Sets the event that triggers an interrupt, see \ref cy_en_mdma_inttype_t     */
    uint32_t                trigoutType;    /**< Sets the event that triggers an output,    see \ref cy_en_mdma_trigouttype_t */
    uint32_t                chStateAtCmplt; /**< Specifies if the channel is enabled or disabled on completion of descriptor see \ref cy_en_mdma_chdesccmpl_t */
    uint32_t                triginType;     /**< Sets what type of transfer is triggered,   see \ref cy_en_mdma_trigintype_t  */
    uint32_t                dataPrefetch;   /**< Source data prefetch */
    uint32_t                dataSize;       /**< The size of the data bus for transfer,     see \ref cy_en_mdma_datasize_t    */
    uint32_t                srcTxfrSize;    /**< The source transfer size                */
    uint32_t                destTxfrSize;   /**< The destination transfer size           */
    uint32_t                descrType;      /**< The type of the descriptor                 see \ref cy_en_mdma_descrtype_t*/
    void *                  srcAddr;        /**< The source address of the transfer      */
    void *                  destAddr;       /**< The destination address of the transfer */
    int16_t                 srcXincr;       /**< The address increment of the source after each X-loop transfer      */
    int16_t                 destXincr;      /**< The address increment of the destination after each X-loop transfer */
    uint32_t                xCount;         /**< The number of transfers in an X-loop                                */
    int16_t                 srcYincr;       /**< The address increment of the source after each Y-loop transfer      */
    int16_t                 destYincr;      /**< The address increment of the destination after each Y-loop transfer */
    uint32_t                yCount;         /**< The number of transfers in the Y-loop */
    cy_stc_mdma_descr_t*    descrNext;      /**< The next descriptor to chain after completion, a NULL value will signify no chaining */
} cy_stc_mdma_descr_config_t;

/** This structure holds the initialization values for the MDMA channel */
typedef struct {
    cy_stc_mdma_descr_t*    MDMA_Descriptor; /**< The MDMA descriptor associated with the channel being initialized           */
    uint32_t                preemptable;    /**< Specifies if the channel is preemptable by another higher-priority channel */
    uint32_t                priority;       /**< This parameter specifies the channel's priority                            */
    uint32_t                enable;         /**< This parameter specifies if the channel is enabled after initializing      */
    uint32_t                priviledge;
    uint32_t                non_secure;
    uint32_t                bufferable;
    uint32_t                protection_context;
} cy_stc_mdma_chnl_config_t;

/** \} group_mdma_data_structures */

/**
* \addtogroup group_mdma_enums
* \{
*/

/** Contains the options for the trigger input deactivation parameter type */
enum cy_en_mdma_deactivate_t {
    CY_MDMA_TRIG_DEACT_NO_WAIT,             /**< Don't wait for deactivation (for pulse sensitive triggers)*/
    CY_MDMA_TRIG_DEACT_4CYC,                /**< Wait for up to 4 cycles */
    CY_MDMA_TRIG_DEACT_16CYC,               /**< Wait for up to 16 cycles */
    CY_MDMA_TRIG_DEACT_WAIT_INDEFINITELY    /**< Wait indefinitely. This option may result in controller lockup if the trigger is not de-activated */
} ;

/** Contains the options for the descriptor type */
enum cy_en_mdma_descrtype_t {
    CY_MDMA_SINGLE_TRANSFER,                /**< Single transfer      */
    CY_MDMA_1D_TRANSFER,                    /**< 1D transfer          */
    CY_MDMA_2D_TRANSFER,                    /**< 2D transfer          */
    CY_MDMA_MEMORY_COPY_TRANSFER,           /**< Memory copy transfer */
    CY_MDMA_SCATTER_TRANSFER                /**< Scatter transfer     */
} ;

/** Contains the options for the interrupt-type parameter of the descriptor */
enum cy_en_mdma_inttype_t {
    CY_MDMA_INTR_1ELEMENT_CMPLT,            /**< Triggers on every element transfer completion          */
    CY_MDMA_INTR_X_LOOP_CMPLT,              /**< Triggers on every X loop transfer completion           */
    CY_MDMA_INTR_DESCR_CMPLT,               /**< Triggers on descriptor completion                      */
    CY_MDMA_INTR_DESCRCHAIN_CMPLT           /**< Triggers on completion of the entire descriptor chain  */
} ;

/** This enum has the options for the trigger in the type parameter of the descriptor */
enum cy_en_mdma_trigouttype_t {
    CY_MDMA_TRIGOUT_1ELEMENT_CMPLT,         /**< Triggers on every element transfer completion          */
    CY_MDMA_TRIGOUT_X_LOOP_CMPLT,           /**< Triggers on every X loop transfer completion           */
    CY_MDMA_TRIGOUT_DESCR_CMPLT,            /**< Triggers on descriptor completion                      */
    CY_MDMA_TRIGOUT_DESCRCHAIN_CMPLT        /**< Triggers on completion of the entire descriptor chain  */
} ;

/** This enum has the options for the trigger-in-type parameter of the descriptor */
enum cy_en_mdma_trigintype_t {
    CY_MDMA_TRIGIN_1ELEMENT,                /**< One transfer per trigger                   */
    CY_MDMA_TRIGIN_XLOOP,                   /**< One X loop transfer per trigger            */
    CY_MDMA_TRIGIN_DESCR,                   /**< The entire descriptor transfer per trigger */
    CY_MDMA_TRIGIN_DESCRCHAIN               /**< The entire descriptor chain per trigger    */
} ;

/** This enum has the options for the data size */
enum cy_en_mdma_datasize_t {
    CY_MDMA_BYTE,                           /**< One byte               */
    CY_MDMA_HALFWORD,                       /**< Half word (two bytes)  */
    CY_MDMA_WORD                            /**< Full word (four bytes) */
} ;

/** This enum has the options for descriptor retriggering */
enum cy_en_mdma_retrigger_t {
    CY_MDMA_RETDIG_IM,                      /**< Retrigger immediately         */
    CY_MDMA_RETDIG_4CYC,                    /**< Retrigger after 4 SYSCLK       */
    CY_MDMA_RETDIG_16CYC,                   /**< Retrigger after 16 SYSCLK      */
    CY_MDMA_WAIT_FOR_REACT                  /**< Wait for trigger reactivation  */
};

/** This enum has the options for the transfer size */
enum cy_en_mdma_txfrsize_t {
    CY_MDMA_TXFR_SIZE_DATA_SIZE,            /**< As per data size register */
    CY_MDMA_TXFR_SIZE_WORD                  /**< A full word (four bytes)  */
};

/** This enum has the options for the state of the channel when the descriptor is completed   */
enum cy_en_mdma_chdesccmpl_t {
    CY_MDMA_CH_ENABLED,                    /**< Channel stays enabled  */
    CY_MDMA_CH_DISABLED                    /**< Channel is disabled */
};

/** This enum has the return values of the MDMA driver */
typedef enum 
{
    CY_MDMA_SUCCESS                  =   0x00ul,  /**< Success */
    CY_MDMA_INVALID_INPUT_PARAMETERS =   ( CY_MDMA_ID_ERROR + 1ul),  /**< Input parameters passed to MDMA API are not valid */
    CY_MDMA_ERR_UNC                  =   ( CY_MDMA_ID_ERROR + 0xFul),/**< Unknown error */
} cy_en_mdma_status_t;

/** \} group_mdma_enums */

/**
* \addtogroup group_mdma_functions
* \{
*/

//IP functions
                void                    Cy_MDMA_Enable                          (           volatile stc_DMAC_t *       pstcMDMA);
                void                    Cy_MDMA_Disable                         (           volatile stc_DMAC_t *       pstcMDMA);
                uint32_t                Cy_MDMA_GetActiveChnl                   (   const   volatile stc_DMAC_t *       pstcMDMA);
                
//Descriptor functions
                cy_en_mdma_status_t     Cy_MDMA_Descr_Init                      (           cy_stc_mdma_descr_t*        descriptor, 
                                                                                    const   cy_stc_mdma_descr_config_t* config);
                void                    Cy_MDMA_Descr_DeInit                    (           cy_stc_mdma_descr_t*        descriptor);
__STATIC_INLINE void                    Cy_MDMA_Descr_SetSrcAddr                (           cy_stc_mdma_descr_t*        descriptor, 
                                                                                    const   void *                      srcAddr);
__STATIC_INLINE void                    Cy_MDMA_Descr_SetDestAddr               (           cy_stc_mdma_descr_t*        descriptor, 
                                                                                    const   void *                      dstAddr);
                void                    Cy_MDMA_Descr_SetNxtDescr               (           cy_stc_mdma_descr_t*        descriptor, 
                                                                                    const   cy_stc_mdma_descr_t*        nxtDescriptor);
__STATIC_INLINE void                    Cy_MDMA_Descr_SetIntrType               (           cy_stc_mdma_descr_t*        descriptor, 
                                                                                            uint32_t                    interruptType);
__STATIC_INLINE void                    Cy_MDMA_Descr_SetTrigInType             (           cy_stc_mdma_descr_t*        descriptor, 
                                                                                            uint32_t                    trigInType);
__STATIC_INLINE void                    Cy_MDMA_Descr_SetTrigOutType            (           cy_stc_mdma_descr_t*        descriptor, 
                                                                                            uint32_t                    trigOutType);
                void                    Cy_MDMA_Descr_SetTxfrWidth              (           cy_stc_mdma_descr_t*        descriptor, 
                                                                                            uint32_t                    dataElementSize, 
                                                                                            uint32_t                    srcTxfrWidth, 
                                                                                            uint32_t                    dstTxfrWidth);
//Channel functions
                cy_en_mdma_status_t     Cy_MDMA_Chnl_Init                       (           volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum, 
                                                                                    const   cy_stc_mdma_chnl_config_t*  chnlConfig);
                void                    Cy_MDMA_Chnl_DeInit                     (           volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE void                    Cy_MDMA_Chnl_SetDescr                   (           volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum, 
                                                                                    const   cy_stc_mdma_descr_t*        descriptor);
__STATIC_INLINE void                    Cy_MDMA_Chnl_Enable                     (           volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE void                    Cy_MDMA_Chnl_Disable                    (           volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE void                    Cy_MDMA_Chnl_SetPriority                (           volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum, 
                                                                                            uint32_t                    priority);
__STATIC_INLINE uint32_t                Cy_MDMA_Chnl_GetPriority                (   const   volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE cy_stc_mdma_descr_t*    Cy_MDMA_Chnl_GetCurrentDescr            (   const   volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE uint32_t                Cy_MDMA_Chnl_GetInterruptStatus         (   const   volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE uint32_t                Cy_MDMA_Chnl_GetInterruptCause          (   const   volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum);
                void                    Cy_MDMA_Chnl_ClearInterrupt             (           volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE void                    Cy_MDMA_Chnl_SetInterrupt               (           volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum,
                                                                                            uint32_t                    intMask);
__STATIC_INLINE uint32_t                Cy_MDMA_Chnl_GetInterruptMask           (   const   volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum);
__STATIC_INLINE void                    Cy_MDMA_Chnl_SetInterruptMask           (           volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum,
                                                                                            uint32_t                    intMask);
__STATIC_INLINE uint32_t                Cy_MDMA_Chnl_GetInterruptStatusMasked   (   const   volatile stc_DMAC_t *       pstcMDMA, 
                                                                                            uint32_t                    chNum);

#if defined(CPUSS_SW_TR_PRESENT) && (CPUSS_SW_TR_PRESENT == 1)
__STATIC_INLINE void                    Cy_MDMA_Chnl_SetSwTrigger               (           volatile stc_DMAC_t *       pstcMDMA,
                                                                                            uint32_t                    chNum);
#endif

/***************************************
*    In-line Function Implementation
***************************************/

/*******************************************************************************
* Function Name: Cy_MDMA_Descr_SetSrcAddr
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
__STATIC_INLINE void Cy_MDMA_Descr_SetSrcAddr(cy_stc_mdma_descr_t* descriptor, const void * srcAddr)
{
    descriptor->u32MDMA_DESCR_SRC = (uint32_t)srcAddr;
}

/*******************************************************************************
* Function Name: Cy_MDMA_Descr_SetDestAddr
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
__STATIC_INLINE void Cy_MDMA_Descr_SetDestAddr(cy_stc_mdma_descr_t* descriptor, const void * dstAddr)
{
    descriptor->u32MDMA_DESCR_DST = (uint32_t)dstAddr;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_SetDescr
****************************************************************************//**
*
* \brief Sets a descriptor as current for specified MDMA channel.
*
* \param pstcMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
*
* \param descriptor
* This is the descriptor which will be associated with the channel.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MDMA_Chnl_SetDescr(volatile stc_DMAC_t *pstcMDMA, uint32_t chNum, const cy_stc_mdma_descr_t* descriptor)
{
    pstcMDMA->CH[chNum].unCURR.u32Register = (uint32_t)descriptor;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_Enable
****************************************************************************//**
*
* The function is used to enable a MDMA channel.
*
* \param pstcMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MDMA_Chnl_Enable(volatile stc_DMAC_t *pstcMDMA, uint32_t chNum)
{
    pstcMDMA->CH[chNum].unCTL.stcField.u1ENABLED = 1ul;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_Disable
****************************************************************************//**
*
* The function is used to disable a MDMA channel.
*
* \param pstcMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MDMA_Chnl_Disable(volatile stc_DMAC_t *pstcMDMA, uint32_t chNum)
{
    pstcMDMA->CH[chNum].unCTL.stcField.u1ENABLED = 0ul;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_SetPriority
****************************************************************************//**
*
* The function is used to set a priority for the MDMA channel.
*
* \param pstcMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
*
* \param priority
* The priority to be set for the MDMA channel. The allowed values are 0,1,2,3.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MDMA_Chnl_SetPriority(volatile stc_DMAC_t *pstcMDMA, uint32_t chNum, uint32_t priority)
{
    pstcMDMA->CH[chNum].unCTL.stcField.u2PRIO = priority;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_GetPriority
****************************************************************************//**
*
* Returns the priority of MDMA channel.
*
* \param pstcMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
*
* \return
* The priority of the channel.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_MDMA_Chnl_GetPriority(const volatile stc_DMAC_t *pstcMDMA, uint32_t chNum)
{
    return pstcMDMA->CH[chNum].unCTL.stcField.u2PRIO;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_GetCurrentDescr
****************************************************************************//**
*
* This function returns the descriptor that is active in the channel.
*
* \param pstcMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
*
* \return
* The pointer to the descriptor associated with the channel.
*
*******************************************************************************/
__STATIC_INLINE cy_stc_mdma_descr_t* Cy_MDMA_Chnl_GetCurrentDescr(const volatile stc_DMAC_t *pstcMDMA, uint32_t chNum)
{
    return (cy_stc_mdma_descr_t*) pstcMDMA->CH[chNum].unCURR.u32Register;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Descr_SetIntrType
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
__STATIC_INLINE void Cy_MDMA_Descr_SetIntrType(cy_stc_mdma_descr_t* descriptor, uint32_t interruptType)
{
    descriptor->unMDMA_DESCR_CTL.stcField.u2INTR_TYPE     =       interruptType;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Descr_SetTrigInType
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
__STATIC_INLINE void Cy_MDMA_Descr_SetTrigInType(cy_stc_mdma_descr_t* descriptor, uint32_t trigInType)
{
    descriptor->unMDMA_DESCR_CTL.stcField.u2TR_IN_TYPE     =       trigInType;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Descr_SetTrigOutType
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
__STATIC_INLINE void Cy_MDMA_Descr_SetTrigOutType(cy_stc_mdma_descr_t* descriptor, uint32_t trigOutType)
{
    descriptor->unMDMA_DESCR_CTL.stcField.u2TR_OUT_TYPE     =       trigOutType;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_GetInterruptStatus
****************************************************************************//**
*
* This function can be used to determine the interrupt status of the
* MDMA channel.
*
* \param pstcMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
*
* \return
* Returns a status of an interrupt for specified channel.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_MDMA_Chnl_GetInterruptStatus(const volatile stc_DMAC_t *pstcMDMA, uint32_t chNum)
{
    return (pstcMDMA->CH[chNum].unINTR.u32Register);
}


/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_GetInterruptCause
****************************************************************************//**
*
* This function can be used to determine the interrupt reason of the
* MDMA channel.
*
* \param pstcMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
*
* \return
* Returns a status of an interrupt for specified channel.
* (see \ref group_pdma_common_macro_MDMAIntrCauses for return values).
* For error related interrupt causes (INTR_CAUSE is "1", "2", "3", ..., "8"),
* the channel is disabled (HW sets CH_CTL.ENABLED to '0').
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_MDMA_Chnl_GetInterruptCause(const volatile stc_DMAC_t *pstcMDMA, uint32_t chNum)
{
    return (pstcMDMA->CH[chNum].unINTR.u32Register & 0xFFul);
}


/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_SetInterrupt
****************************************************************************//**
*
* This function sets the interrupt.
*
* \param pstcMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
* intMask Interrupt to be enabled
*
*******************************************************************************/
__STATIC_INLINE void Cy_MDMA_Chnl_SetInterrupt(volatile stc_DMAC_t *pstcMDMA, uint32_t chNum, uint32_t intMask)
{
    pstcMDMA->CH[chNum].unINTR_SET.u32Register = intMask;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_GetInterruptMask
****************************************************************************//**
*
* This function gets interrupt mask value.
*
* \param pstcMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
*
* \return
* Returns an interrupt mask value.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_MDMA_Chnl_GetInterruptMask(const volatile stc_DMAC_t *pstcMDMA, uint32_t chNum)
{
    return (pstcMDMA->CH[chNum].unINTR_MASK.u32Register);
}


/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_SetInterruptMask
****************************************************************************//**
*
* This function sets interrupt mask value.
*
* \param pstcMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
* intMask Interrupt mask to be enabled
*
*******************************************************************************/
__STATIC_INLINE void Cy_MDMA_Chnl_SetInterruptMask(volatile stc_DMAC_t *pstcMDMA, uint32_t chNum, uint32_t intMask)
{
    pstcMDMA->CH[chNum].unINTR_MASK.u32Register = intMask;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_GetInterruptStatusMasked
****************************************************************************//**
*
* This function returns logical and of corresponding INTR and INTR_MASK fields
* in a single load operation.
*
* \param pstcMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_MDMA_Chnl_GetInterruptStatusMasked(const volatile stc_DMAC_t *pstcMDMA, uint32_t chNum)
{
    return (pstcMDMA->CH[chNum].unINTR_MASKED.u32Register);
}


#if defined(CPUSS_SW_TR_PRESENT) && (CPUSS_SW_TR_PRESENT == 1)

/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_SetSwTrigger
****************************************************************************//**
*
* This function sets the SW trigger for a channel.
*
* \param pstcMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
*
* \return
* Returns an interrupt mask value.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MDMA_Chnl_SetSwTrigger(volatile stc_DMAC_t *pstcMDMA, uint32_t chNum)
{
    pstcMDMA->CH[chNum].unTR_CMD.stcField.u1ACTIVATE = 1ul;
}

#endif


/** \} group_mdma_functions */

#if defined(__cplusplus)
}
#endif

#endif  /* (CY_MDMA_H) */

/** \} group_mdma */


/* [] END OF FILE */
