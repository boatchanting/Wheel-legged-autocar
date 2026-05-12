/***************************************************************************//**
* \file startup.c
* \version 1.0
*
* \brief
* C startup code (ANSI C context not yet established!)
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
#include "startup_customize.h"


#if defined(__cplusplus)
extern "C" {
#endif

/*****************************************************************************
* Local pre-processor symbols/macros ('define')
*****************************************************************************/

#ifdef STARTUP_CUSTOMIZE__ENABLE_ASSERTIONS
    /** If assertions are enabled and an assertion fails, execution will stop in this endless loop */
    #define STARTUP_ASSERT(x)                            if((x) == 0) { while(1){} }
#else
    /** Remove assertion code with this empty macro if assertions are disabled */
    #define STARTUP_ASSERT(x)                            
#endif

/** Returns 'logical true' if the given address is within the given area, otherwise 'logical false' */
#define IS_ADDRESS_IN_AREA(addr,start,end)               ((start) != 0 ? ((start) <= (addr)) && ((end) >= (addr)) : ((end) >= (addr)))

/** Returns the dimension of an array */
#define GET_NR_OF_ARRAY_ELEMENTS(array)                  (sizeof(array)/sizeof(array[0]))


// Most restrictive native ECC width of all "normal" memories (SRAM, DTCM, ITCM) in any Traveo II derivate is used to keep the code generic
// Note: In M7 based TVII derivates the native ECC width is different across memories, e.g. SRAM/ITCM = 64-bit <-> DTCM = 32-bit.
//       In M4 based TVII derivates the native ECC width of SRAM is 32-bit and there are no TCMs, but 64-bit is used anyway to keep the code generic
/** The ECC initialization is based on this granularity */
#define ECC_INIT_WIDTH_BYTES   8
/** Define an abstract type for the chosen ECC initialization granularity */
typedef uint64_t  ecc_init_width_t;


/** Check whether necessary project defines are set if enabling of caches is requested by the user */
#if (CY_CPU_CORTEX_M7)
    #ifdef STARTUP_CUSTOMIZE__ENABLE_CM7_INSTRUCTION_CACHE
        #if !defined(__ICACHE_PRESENT) || (__ICACHE_PRESENT == 0)
            #error "__ICACHE_PRESENT=1 must be set as preprocessor define for the project, otherwise CMSIS code is not included!"
        #endif
    #endif
    #ifdef STARTUP_CUSTOMIZE__ENABLE_CM7_DATA_CACHE
        #if !defined(__DCACHE_PRESENT) || (__DCACHE_PRESENT == 0)
            #error "__DCACHE_PRESENT=1 must be set as preprocessor define for the project, otherwise CMSIS code is not included!"
        #endif
    #endif
#endif


#define SROM_VECTOR_TABLE_BASE_ADDRESS     0x00000000
#define VECTOR_TABLE_OFFSET_IRQ0                 0x40
#define VECTOR_TABLE_OFFSET_IRQ1                 0x44


/*****************************************************************************
* Global variable definitions (declared in header file with 'extern')
*****************************************************************************/
// CAUTION: Static or global initialized and non-const variables will not have their init value yet!


/*****************************************************************************
* Local type definitions ('typedef')
*****************************************************************************/
/** Describes a memory area which is defined by its start and end address */
typedef struct
{
    uint32_t  u32StartAddr;  //!< First address within a region
    uint32_t  u32EndAddr;    //!< Last address within a region
} stc_region_t;


/*****************************************************************************
* Local variable definitions ('static')
*****************************************************************************/
// CAUTION: Static or global initialized and non-const variables will not have their init value yet!



/*****************************************************************************
* Local function prototypes ('static')
*****************************************************************************/
#ifdef STARTUP_CUSTOMIZE__ENABLE_ECC_INITIALIZATION
static void InitRamEcc(void);
static void InitRamEccClearArea(uint32_t u32StartAddr, uint32_t u32EndAddr);
#endif

static void CopyVectorTable(void);
#if (CY_CPU_CORTEX_M0P)
static void PrepareSystemCallInfrastructure(void);
#endif

/*****************************************************************************
* Function implementation - global ('extern') and local ('static')
*****************************************************************************/


/**
 *****************************************************************************
 ** Startup initializations written in C, called from startup assembly code
 ** and executed in a non-ANSI C context (i.e. .bss sections not yet cleared,
 ** init values not yet copied to .data sections), so static and global variables
 ** must be used carefully, if at all!
 **
 ** \param none
 **
 ** \return none
 *****************************************************************************/
void Startup_Init(void)
{
  #if !(CY_CPU_CORTEX_M0P)
    // Enable the FPU if used
    // CAUTION: Implementation of this C function must be aware that it is executed in non-ANSI C context!
    Cy_SystemInitFpuEnable();
  #endif
    
    #ifdef STARTUP_CUSTOMIZE__ENABLE_ECC_INITIALIZATION
    InitRamEcc();
    #endif
    
  #if (defined (tviibh4m) || defined (tviibh8m) || defined (tviic2d6m) || defined (tviic2d4m) || defined (tviic2d6mddr)  || defined (tviibh16m)) && (CY_CPU_CORTEX_M0P)
    // Enable ECC checking in SRAM controllers again (had been switched off by assembly startup code)
    CPUSS->unRAM0_CTL0.stcField.u1ECC_CHECK_DIS = 0;
    #ifdef CPUSS_RAMC1_PRESENT
    CPUSS->unRAM1_CTL0.stcField.u1ECC_CHECK_DIS = 0;
    #endif
    #ifdef CPUSS_RAMC2_PRESENT
    CPUSS->unRAM2_CTL0.stcField.u1ECC_CHECK_DIS = 0;
    #endif
  #endif
  
    CopyVectorTable();
  #if (CY_CPU_CORTEX_M0P)
    PrepareSystemCallInfrastructure();
  #endif

    
  #if (CY_CPU_CORTEX_M7)
    #ifdef STARTUP_CUSTOMIZE__ENABLE_CM7_INSTRUCTION_CACHE
    SCB_EnableICache();
    #endif
    #ifdef STARTUP_CUSTOMIZE__ENABLE_CM7_DATA_CACHE
    SCB_EnableDCache();
    #else
    // Ensure that the undefined valid bits in the cache RAM are set to invalid if cache is disabled, because the application
    // may call further cache maintenance functions (e.g. SCB_CleanInvalidateDCache) independent of the "cache enabled" state.
    SCB_InvalidateDCache();
    #endif
  #endif

}


#ifdef STARTUP_CUSTOMIZE__ENABLE_ECC_INITIALIZATION

/**
 *****************************************************************************
 ** Takes care of initializing the ECC in all memory areas as per the user
 ** configuration given in startup_customize.h
 ** The startup stack has already been initialized by startup_cm*.arm/s, so it
 ** will be skipped here (also to preserve the state of caller functions!)
 **
 ** \param none
 **
 ** \return none
 *****************************************************************************/
static void InitRamEcc(void)
{
    stc_region_t astcClearRegions[] =
    {
        {
            .u32StartAddr = STARTUP_CUSTOMIZE__ECC_INIT_SRAM_START_ADDRESS,
            .u32EndAddr   = STARTUP_CUSTOMIZE__ECC_INIT_SRAM_END_ADDRESS,
        },
      #if (CY_CPU_CORTEX_M7)
        {
            .u32StartAddr = STARTUP_CUSTOMIZE__ECC_INIT_DTCM_START_ADDRESS,
            .u32EndAddr   = STARTUP_CUSTOMIZE__ECC_INIT_DTCM_END_ADDRESS,
        },
        {
            .u32StartAddr = STARTUP_CUSTOMIZE__ECC_INIT_ITCM_START_ADDRESS,
            .u32EndAddr   = STARTUP_CUSTOMIZE__ECC_INIT_ITCM_END_ADDRESS,
        },
      #endif
    };
    
  #ifdef STARTUP_CUSTOMIZE__ENABLE_ASSERTIONS
    // Check alignment of region start/end addresses
    for(uint8_t u8Index = 0; u8Index < GET_NR_OF_ARRAY_ELEMENTS(astcClearRegions); u8Index++)
    {
        STARTUP_ASSERT(((astcClearRegions[u8Index].u32StartAddr)   % ECC_INIT_WIDTH_BYTES) == 0);
        STARTUP_ASSERT(((astcClearRegions[u8Index].u32EndAddr + 1) % ECC_INIT_WIDTH_BYTES) == 0);
    }
    STARTUP_ASSERT(((Cy_u32StartupStackStartAddress)   % ECC_INIT_WIDTH_BYTES) == 0);
    STARTUP_ASSERT(((Cy_u32StartupStackEndAddress + 1) % ECC_INIT_WIDTH_BYTES) == 0);
  #endif
    
    for(uint8_t u8Index = 0; u8Index < GET_NR_OF_ARRAY_ELEMENTS(astcClearRegions); u8Index++)
    {
        // If startup stack is located in current region to be cleared, the clear operation is split up, effectively excluding the startup stack
        if(IS_ADDRESS_IN_AREA(Cy_u32StartupStackStartAddress, astcClearRegions[u8Index].u32StartAddr, astcClearRegions[u8Index].u32EndAddr))
        {
            InitRamEccClearArea(astcClearRegions[u8Index].u32StartAddr, Cy_u32StartupStackStartAddress - 1);
            InitRamEccClearArea(Cy_u32StartupStackEndAddress + 1,       astcClearRegions[u8Index].u32EndAddr);
        }
        else
        {
            InitRamEccClearArea(astcClearRegions[u8Index].u32StartAddr, astcClearRegions[u8Index].u32EndAddr);
        }
    }
}


/**
 *****************************************************************************
 ** Clears an area by writing '0' using a pointer of type #ecc_init_width_t
 **
 ** \param u32StartAddr    Start address of area to be cleared,
 **                        must be aligned to #ECC_INIT_WIDTH_BYTES
 ** \param u32EndAddr      Last address within area to be cleared, (u32EndAddr+1)
 **                        must be aligned to #ECC_INIT_WIDTH_BYTES
 **
 ** \return none
 *****************************************************************************/
static void InitRamEccClearArea(uint32_t u32StartAddr, uint32_t u32EndAddr)
{
    volatile ecc_init_width_t * pRam = (volatile ecc_init_width_t *) u32StartAddr;
    ecc_init_width_t Zero = 0;
    
    for(; (uint32_t)pRam < u32EndAddr; pRam++)
    {
        // Note: Even if ecc_init_width_t is uint64_t, this will be compiled as two 32-bit accesses 
        //       in case of CM0+, because there is no STRD instruction specified in ARMv6-M Thumb
        *pRam = Zero;
    }
}

#endif // STARTUP_CUSTOMIZE__ENABLE_ECC_INITIALIZATION



/**
 *****************************************************************************
 ** Copies the vector table from ROM to RAM and updates the VTOR (CMx vector
 ** table base register) accordingly
 **
 ** \return none
 *****************************************************************************/
static void CopyVectorTable(void)
{
    const uint8_t    u8NrOfVectors = (uint8_t) ((uint32_t) &__Vectors_Size / 4);
    uint32_t * const pu32RamTable  = (uint32_t *) __ramVectors;
    uint32_t * const pu32RomTable  = (uint32_t *) (&__Vectors);
    
    
    for(uint8_t u8Index = 0; u8Index < u8NrOfVectors; u8Index++)
    {
        pu32RamTable[u8Index] = pu32RomTable[u8Index];
    }
    
    SCB->VTOR = (uint32_t) pu32RamTable;
}


#if (CY_CPU_CORTEX_M0P)

/**
 *****************************************************************************
 ** Prepares necessary settings to get SROM system calls working
 **
 ** \return none
 *****************************************************************************/
static void PrepareSystemCallInfrastructure(void)
{
    const uint8_t u8Irq0Index = (uint8_t) (VECTOR_TABLE_OFFSET_IRQ0 / 4);
    const uint8_t u8Irq1Index = (uint8_t) (VECTOR_TABLE_OFFSET_IRQ1 / 4);
    uint32_t * const pu32RamTable   = (uint32_t *) __ramVectors;
    uint32_t * const pu32SromTable  = (uint32_t *) SROM_VECTOR_TABLE_BASE_ADDRESS;
    
    // Use IRQ0 and IRQ1 handlers from SROM vector table
    pu32RamTable[u8Irq0Index] = pu32SromTable[u8Irq0Index];
    pu32RamTable[u8Irq1Index] = pu32SromTable[u8Irq1Index];
    
    NVIC_SetPriority(CPUIntIdx0_IRQn, 1);
    NVIC_SetPriority(CPUIntIdx1_IRQn, 0);
    NVIC_EnableIRQ(CPUIntIdx0_IRQn);
    NVIC_EnableIRQ(CPUIntIdx1_IRQn);
    
    // Only item left is clearing of PRIMASK:
    // This should be done by the application at a later point in time (e.g. in main())    
}

#endif


#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
