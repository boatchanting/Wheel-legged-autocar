/***************************************************************************//**
* \file cy_syslib.h
* \version 1.0
*
* Provides an API declaration of the syslib driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_syslib System Library (SysLib)
* \{
* The system libraries provide APIs that can be called in the user application
* to handle the timing, logical checking or register.
*
* SysLib driver contains a set of different system functions. These functions
* can be called in the application routine. Some features of the system library:
* * Delay functions
* * Register read/write macro
* * Assert and Halt
* * Software reset
* * Reading a reset cause
* * API to invalidate the flash cache and buffer
* * Data manipulation
* * A variable type definition from MISRA-C specifying signedness
* * Cross compiler compatible attributes
* * Get silicon unique ID API
*
* \section group_syslib_configuration Configuration Considerations
* There are no general SysLib configuration concerns.
*
* \section group_syslib_more_information More Information
* Refer to the technical reference manual (TRM).
*
* \section group_syslib_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_syslib_macro Macro
* \defgroup group_syslib_functions Functions
* \defgroup group_syslib_data_structures Data Structures
*
*/

#if !defined(_CY_SYSLIB_H_)
#define _CY_SYSLIB_H_


#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "cy_device_headers.h"

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#if defined( __ICCARM__ )
    /* Suppress the warning for multiple volatile variables in an expression. */
    /* This is common for drivers code and the usage is not order-dependent. */
    #pragma diag_suppress=Pa082
#endif  /* defined( __ICCARM__ ) */


/** \cond INTERNAL */

typedef uint32_t cy_status;

/* MISRA rule 6.3 recommends use specific-length typedef for the basic
 * numerical types of signed and unsigned variants of char, float, and double.
 */
typedef char     char_t;    /**< Specific-length typedef for the basic numerical types of char */
typedef float    float32_t; /**< Specific-length typedef for the basic numerical types of float */
typedef double   float64_t; /**< Specific-length typedef for the basic numerical types of double */


/** \endcond */


/**
* \addtogroup group_syslib_macro
* \{
*/

/******************************************************************************
* Macros
*****************************************************************************/

#define CY_CPU_CORTEX_M0P       (__CORTEX_M == 0)    /**< CM0+ core CPU Code */
#define CY_CPU_CORTEX_M4        (__CORTEX_M == 4)    /**< CM4  core CPU Code */
#define CY_CPU_CORTEX_M7        (__CORTEX_M == 7)    /**< CM7  core CPU Code */

/** Macro to disable the Fault Handler */
#define CY_ARM_FAULT_DEBUG_DISABLED    (0u)
/** Macro to enable the Fault Handler */
#define CY_ARM_FAULT_DEBUG_ENABLED     (1u)

#if !defined(CY_ARM_FAULT_DEBUG)
    /** Macro defines if the Fault Handler is enabled. Enabled by default. */
    #define CY_ARM_FAULT_DEBUG         (CY_ARM_FAULT_DEBUG_ENABLED)
#endif /* CY_ARM_FAULT_DEBUG */

/**
* \defgroup group_syslib_macro_status_codes Status codes
* \{
* Function status type codes
*/
#define CY_PDL_STATUS_CODE_Pos  (0u)        /**< The module status code position in the status code */
#define CY_PDL_STATUS_TYPE_Pos  (16u)       /**< The status type position in the status code */
#define CY_PDL_MODULE_ID_Pos    (18u)       /**< The software module ID position in the status code */
#define CY_PDL_STATUS_INFO      (0UL << CY_PDL_STATUS_TYPE_Pos)    /**< Information status type */
#define CY_PDL_STATUS_WARNING   (1UL << CY_PDL_STATUS_TYPE_Pos)    /**< Warning status type */
#define CY_PDL_STATUS_ERROR     (2UL << CY_PDL_STATUS_TYPE_Pos)    /**< Error status type */
#define CY_PDL_MODULE_ID_Msk    (0x3FFFu)   /**< The software module ID mask */
/** Get the software PDL module ID */
#define CY_PDL_DRV_ID(id)       ((uint32_t)((uint32_t)((id) & CY_PDL_MODULE_ID_Msk) << CY_PDL_MODULE_ID_Pos))
#define CY_SYSLIB_ID            CY_PDL_DRV_ID(0x11u)     /**< SYSLIB PDL ID */
/** \} group_syslib_macro_status_codes */


/**
* \defgroup group_syslib_macro_function_config
* \{
* Function status type codes
*/
#define CY_SYSLIB_CORE_D_CACHE_MAINTENANCE_OFF (0) /**< Disable functions which maintains core cache  */
#define CY_SYSLIB_CORE_D_CACHE_MAINTENANCE_ON  (1) /**< Enable functions which maintains core cache  */
#define CY_SYSLIB_CORE_D_CACHE_MAINTENANCE     CY_SYSLIB_CORE_D_CACHE_MAINTENANCE_ON /**< Set CY_SYSLIB_CORE_D_CACHE_MAINTENANCE_OFF or CY_SYSLIB_CORE_D_CACHE_MAINTENANCE_ON */
/** \} group_syslib_macro_function_config */

/** \} group_syslib_macro */

/**
* \addtogroup group_syslib_data_structures
* \{
*/
/** The SysLib status code structure. */
typedef enum
{
    CY_SYSLIB_SUCCESS       = 0x00UL,    /**< Success status code */
    CY_SYSLIB_BAD_PARAM     = CY_SYSLIB_ID | CY_PDL_STATUS_ERROR | 0x01UL,    /**< Bad parameter status code */
    CY_SYSLIB_TIMEOUT       = CY_SYSLIB_ID | CY_PDL_STATUS_ERROR | 0x02UL,    /**< Time out status code */
    CY_SYSLIB_INVALID_STATE = CY_SYSLIB_ID | CY_PDL_STATUS_ERROR | 0x03UL,    /**< Invalid state status code */
    CY_SYSLIB_UNKNOWN       = CY_SYSLIB_ID | CY_PDL_STATUS_ERROR | 0xFFUL     /**< Unknown status code */
} cy_en_syslib_status_t;

#if (CY_ARM_FAULT_DEBUG == CY_ARM_FAULT_DEBUG_ENABLED)
/** The Fault configuration structure. */
typedef struct
{
    uint32_t fault;    /**< Indicates if the fault occurred */
    uint32_t r0;       /**< R0 register content */
    uint32_t r1;       /**< R1 register content */
    uint32_t r2;       /**< R2 register content */
    uint32_t r3;       /**< R3 register content */
    uint32_t r12;      /**< R12 register content */
    uint32_t lr;       /**< LR register content */
    uint32_t pc;       /**< PC register content */
    uint32_t psr;      /**< PSR register content */

    #if (CY_CPU_CORTEX_M4) || (CY_CPU_CORTEX_M7)
        uint32_t cfsr;         /**< CFSR register content */
        uint32_t faultType;    /**< FPU or regular fault type */

        #if ((defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
             (defined (__FPU_USED   ) && (__FPU_USED    == 1U)))
            uint32_t s0;       /**< FPU S0 register content */
            uint32_t s1;       /**< FPU S1 register content */
            uint32_t s2;       /**< FPU S2 register content */
            uint32_t s3;       /**< FPU S3 register content */
            uint32_t s4;       /**< FPU S4 register content */
            uint32_t s5;       /**< FPU S5 register content */
            uint32_t s6;       /**< FPU S6 register content */
            uint32_t s7;       /**< FPU S7 register content */
            uint32_t s8;       /**< FPU S8 register content */
            uint32_t s9;       /**< FPU S9 register content */
            uint32_t s10;      /**< FPU S10 register content */
            uint32_t s11;      /**< FPU S11 register content */
            uint32_t s12;      /**< FPU S12 register content */
            uint32_t s13;      /**< FPU S13 register content */
            uint32_t s14;      /**< FPU S14 register content */
            uint32_t s15;      /**< FPU S15 register content */
            uint32_t fpscr;    /**< FPU FPSCR register content */
        #endif /* __FPU_PRESENT */
    #endif /* CY_CPU_CORTEX_M4 */
} cy_stc_fault_frame_t;
#endif /* (CY_ARM_FAULT_DEBUG == CY_ARM_FAULT_DEBUG_ENABLED) */

/** \} group_syslib_data_structures */

/**
* \addtogroup group_syslib_macro
* \{
*/

/** Driver major version */
#define CY_SYSLIB_DRV_VERSION_MAJOR    1

/** Driver minor version */
#define CY_SYSLIB_DRV_VERSION_MINOR    0

/** \cond INTERNAL */
/** ARM 32-bit Return error / status codes for backward compatibility.
*  Do not use them in your code.
*/
#define CY_RET_SUCCESS           (0x00u)    /* Successful */
#define CY_RET_BAD_PARAM         (0x01u)    /* One or more invalid parameters */

/** \endcond */


/*******************************************************************************
*  Data manipulation defines
*******************************************************************************/

/** Get the lower 8 bits of a 16-bit value. */
#define CY_LO8(x)               ((uint8_t) ((x) & 0xFFu))
/** Get the upper 8 bits of a 16-bit value. */
#define CY_HI8(x)               ((uint8_t) ((uint16_t)(x) >> 8u))

/** Get the lower 16 bits of a 32-bit value. */
#define CY_LO16(x)              ((uint16_t) ((x) & 0xFFFFu))
/** Get the upper 16 bits of a 32-bit value. */
#define CY_HI16(x)              ((uint16_t) ((uint32_t)(x) >> 16u))

/** Swap the byte ordering of a 16-bit value */
#define CY_SWAP_ENDIAN16(x)     ((uint16_t)(((x) << 8u) | (((x) >> 8u) & 0x00FFu)))

/** Swap the byte ordering of a 32-bit value */
#define CY_SWAP_ENDIAN32(x)     ((uint32_t)((((x) >> 24u) & 0x000000FFu) | (((x) & 0x00FF0000u) >> 8u) | \
                                (((x) & 0x0000FF00u) << 8u) | ((x) << 24u)))

/** Swap the byte ordering of a 64-bit value */
#define CY_SWAP_ENDIAN64(x)     ((uint64_t) (((uint64_t) CY_SWAP_ENDIAN32((uint32_t)(x)) << 32u) | \
                                CY_SWAP_ENDIAN32((uint32_t)((x) >> 32u))))


/*******************************************************************************
*   Memory model definitions
*******************************************************************************/
#if defined(__ARMCC_VERSION)

    /** To create cross compiler compatible code, use the CY_SECTION, CY_UNUSED, CY_ALIGN
     * attributes at the first place of declaration/definition.
     * For example: CY_NOINIT uint32_t noinitVar;
     */
    #define CY_SECTION(name)    __attribute__ ((section(name)))
    #define CY_UNUSED           __attribute__ ((unused))

    /* Specifies the minimum alignment (in bytes) for variables of the specified type. */
    #define CY_ALIGN(align)     __ALIGNED(align)
    #define CY_WEAK             __weak

#elif defined (__GNUC__)

    #define CY_SECTION(name)    __attribute__ ((section(name)))
    #define CY_UNUSED           __attribute__ ((unused))
    #define CY_ALIGN(align)     __ALIGNED(align)
    #define CY_WEAK             __attribute__ ((weak))

#elif defined (__ICCARM__)

    #define CY_PRAGMA(x)        _Pragma(#x)
    #define CY_SECTION(name)    CY_PRAGMA(location = name)
    #define CY_UNUSED
    #define CY_ALIGN(align)     CY_PRAGMA(data_alignment = align)
    #define CY_WEAK             __weak

#elif defined (__ghs__)

    #define CY_SECTION(name)    __attribute__ ((section(name)))
    #define CY_UNUSED           __attribute__ ((unused))
    #define CY_ALIGN(align)     __ALIGNED(align)
    #define CY_WEAK             __WEAK

#elif defined (__DCC__)

    #define CY_SECTION(name)    __attribute__ ((section(name)))
    #define CY_UNUSED           __attribute__ ((unused))
    #define CY_ALIGN(align)     __ALIGNED(align)
    #define CY_WEAK             __WEAK

#else
    #error "Unsupported toolchain"
#endif  /* (__ARMCC_VERSION) */

typedef void (* cy_israddress)(void);   /**< Type of ISR callbacks */

/** \cond INTERNAL */
/** Type of ISR callbacks for backward compatibility with the UDB components. Do not use it in your code. */
typedef void (* cyisraddress)(void);
/** \endcond */

#if defined (__ICCARM__)
    typedef union { cy_israddress __fun; void * __ptr; } cy_intvec_elem;
    /** \cond INTERNAL */
    /** Type of ISR callbacks for backward compatibility with the UDB components. Do not use it in your code. */
    typedef union { cyisraddress __fun; void * __ptr; } intvec_elem;
    /** \endcond */
#endif  /* defined (__ICCARM__) */

#if !defined(NDEBUG)
    /** Max size of file name which stores the ASSERT location */
    #define CY_MAX_FILE_NAME_SIZE  (24u)
    extern char_t cy_assertFileName[CY_MAX_FILE_NAME_SIZE];  /**< Assert buffer */
    extern uint32_t cy_assertLine;                           /**< Assert line value */
#endif /* NDEBUG */

#if (CY_ARM_FAULT_DEBUG == CY_ARM_FAULT_DEBUG_ENABLED)
    #define CY_R0_Pos              (0u)    /**< Position of R0  content in fault structure */
    #define CY_R1_Pos              (1u)    /**< Position of R1  content in fault structure */
    #define CY_R2_Pos              (2u)    /**< Position of R2  content in fault structure */
    #define CY_R3_Pos              (3u)    /**< Position of R3  content in fault structure */
    #define CY_R12_Pos             (4u)    /**< Position of R12 content in fault structure */
    #define CY_LR_Pos              (5u)    /**< Position of LR  content in fault structure */
    #define CY_PC_Pos              (6u)    /**< Position of PC  content in fault structure */
    #define CY_PSR_Pos             (7u)    /**< Position of PSR content in fault structure */
    #if (CY_CPU_CORTEX_M0P)
        #define CY_CM0P_FAULT_TYPE (0xF0F1F2F3u)       /**< Flag for indication CM0P core fault */
    #else
        #if (CY_CPU_CORTEX_M4)
            #define CY_CM4_FAULT_TYPE     (0xF4F5F6F7u)    /**< Flag for indication CM4 core fault */
        #elif (CY_CPU_CORTEX_M7)
            #define CY_CM7_FAULT_TYPE     (0xF7F8F9FAu)    /**< Flag for indication CM7 core fault */
        #endif
        #define CY_NON_FPU_FAULT_TYPE (0xA1A2A3A4u)    /**< Flag for indication CM4/7 non floating point fault type */
            #if ((defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
                 (defined (__FPU_USED   ) && (__FPU_USED    == 1U)))
                #define CY_FPU_FAULT_TYPE (0xA8A9AAABu)    /**< Flag for indication CM4/7 core floating point fault type */
                #define CY_FPSCR_IXC_Msk  (0x00000010u)    /**< Cumulative exception bit for floating-point exceptions */
                #define CY_FPSCR_IDC_Msk  (0x00000080u)    /**< Cumulative exception bit for floating-point exceptions */
                #define CY_S0_Pos         (8u)     /**< Position of FPU S0 content in fault structure */
                #define CY_S1_Pos         (9u)     /**< Position of FPU S1 content in fault structure */
                #define CY_S2_Pos         (10u)    /**< Position of FPU S2 content in fault structure */
                #define CY_S3_Pos         (11u)    /**< Position of FPU S3 content in fault structure */
                #define CY_S4_Pos         (12u)    /**< Position of FPU S4 content in fault structure */
                #define CY_S5_Pos         (13u)    /**< Position of FPU S5 content in fault structure */
                #define CY_S6_Pos         (14u)    /**< Position of FPU S6 content in fault structure */
                #define CY_S7_Pos         (15u)    /**< Position of FPU S7 content in fault structure */
                #define CY_S8_Pos         (16u)    /**< Position of FPU S8 content in fault structure */
                #define CY_S9_Pos         (17u)    /**< Position of FPU S9 content in fault structure */
                #define CY_S10_Pos        (18u)    /**< Position of FPU S10 content in fault structure */
                #define CY_S11_Pos        (19u)    /**< Position of FPU S11 content in fault structure */
                #define CY_S12_Pos        (20u)    /**< Position of FPU S12 content in fault structure */
                #define CY_S13_Pos        (21u)    /**< Position of FPU S13 content in fault structure */
                #define CY_S14_Pos        (22u)    /**< Position of FPU S14 content in fault structure */
                #define CY_S15_Pos        (23u)    /**< Position of FPU S15 content in fault structure */
                #define CY_FPSCR_Pos      (24u)    /**< Position of FPU FPSCR content in fault structure */
            #endif /* __FPU_PRESENT */
    #endif /* CY_CPU_CORTEX_M0P */

    extern cy_stc_fault_frame_t cy_faultFrame;    /**< Fault frame structure */
#endif /* (CY_ARM_FAULT_DEBUG == CY_ARM_FAULT_DEBUG_ENABLED) */


/*******************************************************************************
* Macro Name: CY_GET_REG8(addr)
****************************************************************************//**
*
* Reads the 8-bit value from the specified address. This function can't be
* used to access the Core register, otherwise a fault occurs.
*
* \param addr  The register address.
*
* \return The read value.
*
*******************************************************************************/
#define CY_GET_REG8(addr)           (*((const volatile uint8_t *)(addr)))


/*******************************************************************************
* Macro Name: CY_SET_REG8(addr, value)
****************************************************************************//**
*
* Writes an 8-bit value to the specified address. This function can't be
* used to access the Core register, otherwise a fault occurs.
*
* \param addr  The register address.
*
* \param value The value to write.
*
*******************************************************************************/
#define CY_SET_REG8(addr, value)    (*((volatile uint8_t *)(addr)) = (uint8_t)(value))


/*******************************************************************************
* Macro Name: CY_GET_REG16(addr)
****************************************************************************//**
*
* Reads the 16-bit value from the specified address.
*
* \param addr  The register address.
*
* \return The read value.
*
*******************************************************************************/
#define CY_GET_REG16(addr)          (*((const volatile uint16_t *)(addr)))


/*******************************************************************************
* Macro Name: CY_SET_REG16(addr, value)
****************************************************************************//**
*
* Writes the 16-bit value to the specified address.
*
* \param addr  The register address.
*
* \param value The value to write.
*
*******************************************************************************/
#define CY_SET_REG16(addr, value)   (*((volatile uint16_t *)(addr)) = (uint16_t)(value))


/*******************************************************************************
* Macro Name: CY_GET_REG24(addr)
****************************************************************************//**
*
* Reads the 24-bit value from the specified address.
*
* \param addr  The register address.
*
* \return The read value.
*
*******************************************************************************/
#define CY_GET_REG24(addr)          (((uint32_t) (*((const volatile uint8_t *)(addr)))) | \
                                    (((uint32_t) (*((const volatile uint8_t *)(addr) + 1))) << 8U) | \
                                    (((uint32_t) (*((const volatile uint8_t *)(addr) + 2))) << 16U))                                    


/*******************************************************************************
* Macro Name: CY_SET_REG24(addr, value)
****************************************************************************//**
*
* Writes the 24-bit value to the specified address.
*
* \param addr  The register address.
*
* \param value The value to write.
*
*******************************************************************************/
#define CY_SET_REG24(addr, value)   do  \
                                    {   \
                                        (*((volatile uint8_t *) (addr))) = (uint8_t)(value);                \
                                        (*((volatile uint8_t *) (addr) + 1)) = (uint8_t)((value) >> 8U);    \
                                        (*((volatile uint8_t *) (addr) + 2)) = (uint8_t)((value) >> 16U);   \
                                    }   \
                                    while(0)


/*******************************************************************************
* Macro Name: CY_GET_REG32(addr)
****************************************************************************//**
*
* Reads the 32-bit value from the specified register. The address is the little
* endian order (LSB in lowest address).
*
* \param addr  The register address.
*
* \return The read value.
*
*******************************************************************************/
#define CY_GET_REG32(addr)          (*((const volatile uint32_t *)(addr)))


/*******************************************************************************
* Macro Name: CY_SET_REG32(addr, value)
****************************************************************************//**
*
* Writes the 32-bit value to the specified register. The address is the little
* endian order (LSB in lowest address).
*
* \param addr  The register address.
*
* \param value The value to write.
*
*******************************************************************************/
#define CY_SET_REG32(addr, value)   (*((volatile uint32_t *)(addr)) = (uint32_t)(value))


/**
* \defgroup group_syslib_macros_assert Assert Classes and Levels
* \{
* Defines for the Assert Classes and Levels
*/

/** 
* Class 1 - The highest class, safety-critical functions which rely on parameters that could be
* changed between different PSoC devices
*/
#define CY_ASSERT_CLASS_1           (1U)

/** Class 2 - Functions that have fixed limits such as counter periods (16-bits/32-bits etc.) */
#define CY_ASSERT_CLASS_2           (2U)

/** Class 3 - Functions that accept enums as constant parameters */
#define CY_ASSERT_CLASS_3           (3U)

#ifndef CY_ASSERT_LEVEL
    /** The user-definable assert level from compiler command-line argument (similarly to DEBUG / NDEBUG) */
    #define CY_ASSERT_LEVEL         CY_ASSERT_CLASS_3
#endif /* CY_ASSERT_LEVEL */

#if (CY_ASSERT_LEVEL == CY_ASSERT_CLASS_1)
    #define CY_ASSERT_L1(x)         CY_ASSERT(x)     /**< Assert Level 1 */
    #define CY_ASSERT_L2(x)         do{} while(0)    /**< Assert Level 2 */
    #define CY_ASSERT_L3(x)         do{} while(0)    /**< Assert Level 3 */
#elif (CY_ASSERT_LEVEL == CY_ASSERT_CLASS_2)
    #define CY_ASSERT_L1(x)         CY_ASSERT(x)     /**< Assert Level 1 */
    #define CY_ASSERT_L2(x)         CY_ASSERT(x)     /**< Assert Level 2 */
    #define CY_ASSERT_L3(x)         do{} while(0)    /**< Assert Level 3 */
#else /* Default is Level 3 */
    #define CY_ASSERT_L1(x)         CY_ASSERT(x)     /**< Assert Level 1 */
    #define CY_ASSERT_L2(x)         CY_ASSERT(x)     /**< Assert Level 2 */
    #define CY_ASSERT_L3(x)         CY_ASSERT(x)     /**< Assert Level 3 */
#endif /* CY_ASSERT_LEVEL == CY_ASSERT_CLASS_1 */

/** \} group_syslib_macros_assert */

/*******************************************************************************
* Macro Name: CY_ASSERT
****************************************************************************//**
*
*  The macro that evaluates the expression and, if it is false (evaluates to 0),
*  the processor is halted. Cy_Halt() is called when the logical expression is
*  false to halt the processor.
*  The ASSERT location of the file name (including path to file) and line number
*  will be stored in a non-zero init area for debugging. They can be accessed
*  by cy_assertFileName and assertLine global variables.
*
* \note This macro is evaluated unless NDEBUG is not defined.
*  If NDEBUG is defined, just empty do while cycle is generated for this macro
*  for the sake of consistency.
*  NDEBUG is defined by default for a Release build setting and not defined for
*  a Debug build setting.
*
*  \param x  The logical expression. Asserts if false.
*
*******************************************************************************/
#if !defined(NDEBUG)
    #define CY_ASSERT(x)    do  \
                            {   \
                                if(0u == (uint32_t)(x))                                                 \
                                {                                                                       \
                                    (void) strncpy(cy_assertFileName, __FILE__, CY_MAX_FILE_NAME_SIZE); \
                                    cy_assertFileName[CY_MAX_FILE_NAME_SIZE - 1] = '\0';                \
                                    cy_assertLine = (uint32_t)(__LINE__);                               \
                                    Cy_SysLib_Halt((uint32_t)0u);                                       \
                                }                                                                       \
                            }   \
                            while(0u)
#else
    #define CY_ASSERT(x)    do  \
                            {   \
                            }   \
                            while(0u)
#endif  /* !defined(NDEBUG) */


/*******************************************************************************
* Macro Name: _CLR_SET_FLD32U
****************************************************************************//**
*
*  The macro for setting a register with a name field and value for providing
*  get-clear-modify-write operations.
*  Returns a resulting value to be assigned to the register.
*
*******************************************************************************/
#define _CLR_SET_FLD32U(reg, field, value) (((reg) & ((uint32_t)(~(field ## _Msk)))) | (_VAL2FLD(field, value)))


/*******************************************************************************
* Macro Name: CY_REG32_CLR_SET
****************************************************************************//**
*
*  The macro for _CLR_SET_FLD32U usage simplification.
*
*******************************************************************************/
#define CY_REG32_CLR_SET(reg, field, value) (reg) = _CLR_SET_FLD32U((reg), field, (value))


/*******************************************************************************
* Macro Name: _CLR_SET_FLD16U
****************************************************************************//**
*
*  The macro for setting a 16-bit register with a name field and value for providing
*  get-clear-modify-write operations.
*  Returns a resulting value to be assigned to the 16-bit register.
*
*******************************************************************************/
#define _CLR_SET_FLD16U(reg, field, value) ((uint16_t)(((reg) & ((uint16_t)(~(field ## _Msk)))) |   \
                                                       ((uint16_t)_VAL2FLD(field, value))))
                                                       
                                                       
/*******************************************************************************
* Macro Name: CY_REG16_CLR_SET
****************************************************************************//**
*
*  The macro for _CLR_SET_FLD16U usage simplification.
*
*******************************************************************************/
#define CY_REG16_CLR_SET(reg, field, value) (reg) = _CLR_SET_FLD16U((reg), field, (value))


/*******************************************************************************
* Macro Name: _CLR_SET_FLD8U
****************************************************************************//**
*
*  The macro for setting a 8-bit register with a name field and value for providing
*  get-clear-modify-write operations.
*  Returns a resulting value to be assigned to the 8-bit register.
*
*******************************************************************************/
#define _CLR_SET_FLD8U(reg, field, value) ((uint8_t)(((reg) & ((uint8_t)(~(field ## _Msk)))) |  \
                                                     ((uint8_t)_VAL2FLD(field, value))))
                                                     
                                                     
/*******************************************************************************
* Macro Name: CY_REG8_CLR_SET
****************************************************************************//**
*
*  The macro for _CLR_SET_FLD8U usage simplification.
*
*******************************************************************************/
#define CY_REG8_CLR_SET(reg, field, value) (reg) = _CLR_SET_FLD8U((reg), field, (value))


/*******************************************************************************
* Macro Name: _BOOL2FLD
****************************************************************************//**
*
*  Returns a field mask if the value is not false.
*  Returns 0, if the value is false.
*
*******************************************************************************/
#define _BOOL2FLD(field, value) (((value) != false) ? (field ## _Msk) : 0UL)


/*******************************************************************************
* Macro Name: _FLD2BOOL
****************************************************************************//**
*
*  Returns true, if the value includes the field mask.
*  Returns false, if the value doesn't include the field mask.
*
*******************************************************************************/
#define _FLD2BOOL(field, value) (((value) & (field ## _Msk)) != 0UL)


/*******************************************************************************
* Macro Name: CY_MAX
****************************************************************************//**
*
*  Returns the larger of both parameters
*
*******************************************************************************/
#define CY_MAX(x,y)                       ((x) > (y) ? (x) : (y))


/*******************************************************************************
* Macro Name: CY_MIN
****************************************************************************//**
*
*  Returns the smaller of both parameters
*
*******************************************************************************/
#define CY_MIN(x,y)                       ((x) < (y) ? (x) : (y))


/*******************************************************************************
* Macro Name: CY_GET_ARRAY_LEN
****************************************************************************//**
*
*  Returns the length of an array
*
*******************************************************************************/
#define CY_GET_ARRAY_LEN(array)           (sizeof(array) / sizeof(array[0]))


/*******************************************************************************
* Macro Name: CY_IS_IN_ITCM
****************************************************************************//**
*
*  Returns true, if the address is within the SELF_ITCM address space
*  Returns false, if the address is outside the SELF_ITCM address space or the
*                 device does not have an M7 CPUSS
*
*******************************************************************************/
#ifdef CY_IP_M7CPUSS
  // Assumes that ITCM size is identical for all CM7
  #define CY_IS_IN_ITCM(addr)             (((addr) < (0x00000000 + CY_CM7_0_ITCM_SIZE)) ? true : false)
#else
  #define CY_IS_IN_ITCM(addr)             (false)
#endif


/*******************************************************************************
* Macro Name: CY_IS_IN_DTCM
****************************************************************************//**
*
*  Returns true, if the address is within the SELF_DTCM address space
*  Returns false, if the address is outside the SELF_DTCM address space or the
*                 device does not have an M7 CPUSS
*
*******************************************************************************/
#ifdef CY_IP_M7CPUSS
  // Assumes that DTCM size is identical for all CM7
  #define CY_IS_IN_DTCM(addr)             ((((addr) >= 0x20000000)                       && \
                                            ((addr) < (0x20000000 + CY_CM7_0_DTCM_SIZE))    ) ? true : false)
#else
  #define CY_IS_IN_DTCM(addr)             (false)
#endif


/*******************************************************************************
* Macro Name: CY_IS_IN_TCM
****************************************************************************//**
*
*  Returns true, if the address is within the SELF_ITCM or SELF_DTCM address space
*  Returns false, if the address is outside the SELF_ITCM and SELF_DTCM address space
*                  or the device does not have an M7 CPUSS
*
*******************************************************************************/
#define CY_IS_IN_TCM(addr)             (CY_IS_IN_DTCM(addr) || CY_IS_IN_ITCM(addr))


/*******************************************************************************
* Macro Name: CY_GET_PUBLIC_ADDR_FROM_TCM
****************************************************************************//**
*
*  Returns unchanged address for a device with M4 CPUSS, or if the address is not within the
*  SELF_ITCM or SELF_DTCM address space.
*  Returns translated address that can be used for access by external masters like DMA, if the address
*  is within the SELF_ITCM and SELF_DTCM address space. The translated target address can be controlled
*  with the second macro parameter (Master ID)
*  Macro should be used when variables/buffers that might potentially be located in a TCM memory
*  shall be accessed by other masters, e.g. descriptors for DMA, shared memory for IPC, ...
*
*  addr  : Address to be translated
*  ms_id : Master ID, address will be translated to this master's public DTCM/ITCM address space mirror
*          - CPUSS_MS_ID_CM0:   Address will be returned unchanged, because CM0+ does not have DTCM/ITCM
*          - CPUSS_MS_ID_CM4:   Address will be returned unchanged, because CM4 does not have DTCM/ITCM
*          - CPUSS_MS_ID_CM7_0: Address will be translated to CM7_0 public ITCM/DTCM if it is
*                               within SELF_ITCM or SELF_DTCM address space
*          - CPUSS_MS_ID_CM7_1: Address will be translated to CM7_1 public ITCM/DTCM if it is
*                               within SELF_ITCM or SELF_DTCM address space
*          Other master IDs are not supported, CPUSS_MS_ID_CM7_1 is only supported if device has an CM7_1!
*
*  CPUSS->unIDENTITY.stcField.u4MS can be used to pass the current core's master ID at runtime:
*  
* Example 1):
* CY_GET_PUBLIC_ADDR_FROM_TCM(0x20001234, CPUSS->unIDENTITY.stcField.u4MS) would return
*    - 0xa0011234 if executed by CM7_0
*    - 0xa0111234 if executed by CM7_1
*    - 0x20001234 if executed by CM0+ or CM4
*
* Example 2):
* CY_GET_PUBLIC_ADDR_FROM_TCM(0x20001234, CPUSS_MS_ID_CM7_0) would always return
*    - 0xa0011234, on a device that has an M7 CPUSS, independent on whether executed by CM0+, CM7_0 or CM7_1
*    - 0x20001234 if executed by CM0+ of a device with M4 CPUSS, or by CM4
*    
*
*******************************************************************************/
#ifdef CY_IP_M7CPUSS
  #if (CPUSS_CM7_1_PRESENT == 1)
    #define CY_GET_PUBLIC_ADDR_FROM_TCM(addr,ms_id)     ( ((ms_id) == CPUSS_MS_ID_CM0)   ? (addr)                                                                                               :    \
                                                          (CY_IS_IN_DTCM(addr) != false) ? ((addr) - 0x20000000 + (((ms_id) == CPUSS_MS_ID_CM7_0) ? CY_CM7_0_DTCM_BASE : CY_CM7_1_DTCM_BASE))   :    \
                                                          (CY_IS_IN_ITCM(addr) != false) ? ((addr) - 0x00000000 + (((ms_id) == CPUSS_MS_ID_CM7_0) ? CY_CM7_0_ITCM_BASE : CY_CM7_1_ITCM_BASE))   :    \
                                                          (addr) )
  #else
    #define CY_GET_PUBLIC_ADDR_FROM_TCM(addr,ms_id)     ( ((ms_id) == CPUSS_MS_ID_CM0)   ? (addr)                                       :    \
                                                          (CY_IS_IN_DTCM(addr) != false) ? ((addr) - 0x20000000 + CY_CM7_0_DTCM_BASE)   :    \
                                                          (CY_IS_IN_ITCM(addr) != false) ? ((addr) - 0x00000000 + CY_CM7_0_ITCM_BASE)   :    \
                                                          (addr) )
  #endif      
#else
  #define CY_GET_PUBLIC_ADDR_FROM_TCM(addr,ms_id)       (addr)
#endif


/******************************************************************************
* Constants
*****************************************************************************/
/** Defines a 32-kHz clock delay */
#define CY_DELAY_MS_OVERFLOW            (0x8000U)

/*******************************************************************************
* Macro Name: CY_SYSLIB_DIV_ROUND
****************************************************************************//**
*
*  Calculates a / b with rounding to the nearest integer,
*  a and b must have the same sign.
*
*******************************************************************************/
#define CY_SYSLIB_DIV_ROUND(a, b) (((a) + ((b) / 2U)) / (b))


/*******************************************************************************
* Macro Name: CY_SYSLIB_DIV_ROUNDUP
****************************************************************************//**
*
*  Calculates a / b with rounding up if remainder != 0,
*  both a and b must be positive.
*
*******************************************************************************/
#define CY_SYSLIB_DIV_ROUNDUP(a, b) ((((a) - 1U) / (b)) + 1U)

/** \} group_syslib_macro */

/******************************************************************************
* Function prototypes
******************************************************************************/

/**
* \addtogroup group_syslib_functions
* \{
*/

void Cy_SysLib_Delay(uint32_t milliseconds);
void Cy_SysLib_DelayUs(uint16_t microseconds);
/** Delays for the specified number of cycles.
 *  The function is implemented in assembler for each supported compiler.
 *  \param cycles  The number of cycles to delay.
 */
void Cy_SysLib_DelayCycles(uint32_t cycles);
__NO_RETURN void Cy_SysLib_Halt(uint32_t reason);
void Cy_SysLib_ClearFlashCacheAndBuffer(void);

uint64_t Cy_SysLib_GetUniqueId(void);

#if (CY_ARM_FAULT_DEBUG == CY_ARM_FAULT_DEBUG_ENABLED) || defined(CY_DOXYGEN)
    void Cy_SysLib_FaultHandler(uint32_t const *faultStackAddr);
    CY_WEAK void Cy_SysLib_ProcessingFault(void);
#endif /* (CY_ARM_FAULT_DEBUG == CY_ARM_FAULT_DEBUG_ENABLED) */
void Cy_SysLib_SetWaitStates(uint32_t clkHfMHz);


/*******************************************************************************
* Function Name: Cy_SysLib_EnterCriticalSection
****************************************************************************//**
*
*  Cy_SysLib_EnterCriticalSection disables interrupts and returns a value
*  indicating whether interrupts were previously enabled.
*
*  \note Implementation of Cy_SysLib_EnterCriticalSection manipulates the IRQ
*  enable bit with interrupts still enabled.
*
*  \return
*    Returns the current interrupt status. Returns 0 if interrupts were
*    previously enabled or 1 if interrupts were previously disabled.
*
*******************************************************************************/
uint32_t Cy_SysLib_EnterCriticalSection(void);


/*******************************************************************************
* Function Name: Cy_SysLib_ExitCriticalSection
****************************************************************************//**
*
*  Re-enables interrupts if they were enabled before
*  Cy_SysLib_EnterCriticalSection() was called. The argument should be the value
*  returned from \ref Cy_SysLib_EnterCriticalSection().
*
*  \param savedIntrStatus
*   Puts the saved interrupts status returned by
*   the \ref Cy_SysLib_EnterCriticalSection().
*
*******************************************************************************/
void Cy_SysLib_ExitCriticalSection(uint32_t savedIntrStatus);


/*******************************************************************************
* Function Name: Cy_SysLib_InvalidateCoreDCacheByAddr
****************************************************************************//**
*
* Invalidates D-cache in the core for the given address.
* The D-cache memory blocks which are part of given address + given size are invalidated.
*
* \param addr address
* \param dsize size of memory block (in number of bytes)
*
* \note This function works only when called by the core with D-cache (e.g. CM7)
*       and CY_SYSLIB_CORE_D_CACHE_MAINTENANCE == CY_SYSLIB_CORE_D_CACHE_MAINTENANCE_ON.
*
*******************************************************************************/
void Cy_SysLib_InvalidateCoreDCacheByAddr(void *addr, int32_t dsize);

/*******************************************************************************
* Function Name: Cy_SysLib_CleanCoreDCacheByAddr
****************************************************************************//**
*
* Cleans D-cache in the core for the given address
* The D-cache memory blocks which are part of given address + given size are cleaned.
*
* \param addr address
* \param dsize size of memory block (in number of bytes)
*
* \note This function works only when called by the core with D-cache (e.g. CM7)
*       and CY_SYSLIB_CORE_D_CACHE_MAINTENANCE == CY_SYSLIB_CORE_D_CACHE_MAINTENANCE_ON.
*
*******************************************************************************/
void Cy_SysLib_CleanCoreDCacheByAddr(void *addr, int32_t dsize);

/*******************************************************************************
* Function Name: Cy_SysLib_CleanInvalidateCoreDCacheByAddr
****************************************************************************//**
*
* Cleans and invalidates D-cache in the core for the given address
* The D-cache memory blocks which are part of given address + given size are cleaned
* and invalidated.
*
* \param addr address
* \param dsize size of memory block (in number of bytes)
*
* \note This function works only when called by the core with D-cache (e.g. CM7)
*       and CY_SYSLIB_CORE_D_CACHE_MAINTENANCE == CY_SYSLIB_CORE_D_CACHE_MAINTENANCE_ON.
*
*******************************************************************************/
void Cy_SysLib_CleanInvalidateCoreDCacheByAddr(void *addr, int32_t dsize);

/** \} group_syslib_functions */

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* _CY_SYSLIB_H_ */

/** \} group_syslib */

/* [] END OF FILE */
