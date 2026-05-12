/***************************************************************************//**
* \file cy_crypto_common.h
* \version 1.0
*
* \brief
*  This file provides common constants and parameters
*  for the Crypto driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_CRYPTO_COMMON_H)
#define CY_CRYPTO_COMMON_H

#include <stddef.h>
#include <stdbool.h>
#include "cy_device_headers.h"
#include "sysint/cy_sysint.h"
#include "syslib/cy_syslib.h"

#ifndef CY_IP_MXCRYPTO
    #error "The CRYPTO driver is not supported on this device"
#endif

#if (CPUSS_CRYPTO_PRESENT == 1)

/** Driver major version */
#define CY_CRYPTO_DRV_VERSION_MAJOR       (1)

/** Driver minor version */
#define CY_CRYPTO_DRV_VERSION_MINOR       (0)



/**
* \addtogroup group_crypto_macros
* \{
*/

/** Defines Crypto_Sync blocking execution type parameter */
#define CY_CRYPTO_SYNC_BLOCKING           (true)

/** Defines Crypto_Sync non-blocking execution type parameter */
#define CY_CRYPTO_SYNC_NON_BLOCKING       (false)

/** Defines the Crypto DES key size (in bytes) */
#define CY_CRYPTO_DES_KEY_SIZE            (8u)

/** Defines the Crypto TDES key size (in bytes) */
#define CY_CRYPTO_TDES_KEY_SIZE           (24u)

/** Defines the Crypto AES block size (in bytes) */
#define CY_CRYPTO_AES_BLOCK_SIZE          (16u)

/** Defines the Crypto AES key maximum size (in bytes) */
#define CY_CRYPTO_AES_128_KEY_SIZE        (16u)

/** Defines the Crypto AES key maximum size (in bytes) */
#define CY_CRYPTO_AES_192_KEY_SIZE        (24u)

/** Defines the Crypto AES key maximum size (in bytes) */
#define CY_CRYPTO_AES_256_KEY_SIZE        (32u)

/** Defines size of the AES block, in four-byte words */
#define CY_CRYPTO_AES_BLOCK_SIZE_U32      (4UL)

#if ((CPUSS_CRYPTO_SHA1 == 1) ||        \
     (CPUSS_CRYPTO_SHA2 == 1) ||        \
     (CPUSS_CRYPTO_SHA3 == 1))
#define CPUSS_CRYPTO_SHA                    (1u)
#endif

#if (CPUSS_CRYPTO_SHA == 1)

/** Hash size for the SHA1 mode (in bytes)   */
#define CY_CRYPTO_SHA1_DIGEST_SIZE          (20u)
/** Hash size for the SHA224 mode (in bytes) */
#define CY_CRYPTO_SHA224_DIGEST_SIZE        (28u)
/** Hash size for the SHA256 mode (in bytes) */
#define CY_CRYPTO_SHA256_DIGEST_SIZE        (32u)
/** Hash size for the SHA384 mode (in bytes) */
#define CY_CRYPTO_SHA384_DIGEST_SIZE        (48u)
/** Hash size for the SHA512 mode (in bytes) */
#define CY_CRYPTO_SHA512_DIGEST_SIZE        (64u)
/** Hash size for the SHA512_224 mode (in bytes) */
#define CY_CRYPTO_SHA512_224_DIGEST_SIZE    (28u)
/** Hash size for the SHA512_256 mode (in bytes) */
#define CY_CRYPTO_SHA512_256_DIGEST_SIZE    (32u)
/** Hash size for the SHA3_224 mode (in bytes) */
#define CY_CRYPTO_SHA3_224_DIGEST_SIZE      (224u/8u)
/** Hash size for the SHA3_256 mode (in bytes) */
#define CY_CRYPTO_SHA3_256_DIGEST_SIZE      (256u/8u)
/** Hash size for the SHA3_384 mode (in bytes) */
#define CY_CRYPTO_SHA3_384_DIGEST_SIZE      (384u/8u)
/** Hash size for the SHA3_512 mode (in bytes) */
#define CY_CRYPTO_SHA3_512_DIGEST_SIZE      (512u/8u)
/** Hash size for the SHAKE128 mode (in bytes) 
  * SHAKE128 can generate variable size of the digest, but in the driver implementation 
  * only fixed length 256bits is supported.
  */
#define CY_CRYPTO_SHAKE128_DIGEST_SIZE      (256u/8u)
/** Hash size for the SHAKE256 mode (in bytes) 
  * SHAKE256 can generate variable size of the digest, but in the driver implementation 
  * only fixed length 512bits is supported.
  */
#define CY_CRYPTO_SHAKE256_DIGEST_SIZE      (512u/8u)

#endif /* #if (CPUSS_CRYPTO_SHA == 1) */

#if (CPUSS_CRYPTO_VU == 1)

/** Processed message size for the RSA 1024Bit mode (in bytes) */
#define CY_CRYPTO_RSA1024_MESSAGE_SIZE      (128)
/** Processed message size for the RSA 1536Bit mode (in bytes) */
#define CY_CRYPTO_RSA1536_MESSAGE_SIZE      (192)
/** Processed message size for the RSA 2048Bit mode (in bytes) */
#define CY_CRYPTO_RSA2048_MESSAGE_SIZE      (256)

#endif /* #if (CPUSS_CRYPTO_VU == 1) */

/** Crypto Driver PDL ID */
#define CY_CRYPTO_ID              CY_PDL_DRV_ID(0x0Cu)

/** \} group_crypto_macros */

/**
* \addtogroup group_crypto_config_structure
* \{
*/

/** The Crypto user callback function type.
    Callback is called at the end of Crypto calculation. */
typedef void (*cy_crypto_callback_ptr_t)(void);

/** The Crypto configuration structure. */
typedef struct
{
    /** Defines the IPC channel used for client-server data exchange */
    uint32_t ipcChannel;

    /** Specifies the IPC notifier channel (IPC interrupt structure number)
        to notify server that data for the operation is prepared */
    uint32_t acquireNotifierChannel;

    /** Specifies the IPC notifier channel (IPC interrupt structure number)
        to notify client that operation is complete and data is valid */
    uint32_t releaseNotifierChannel;

    /** Specifies the release notifier interrupt configuration. */
    cy_stc_sysint_irq_t releaseNotifierConfig;

    /** User callback function.
        If this field is NOT NULL, it called when Crypto operation
        is complete. */
    cy_crypto_callback_ptr_t userCompleteCallback;

#if (CY_CPU_CORTEX_M0P)
    /** Server-side user IRQ handler function, called when data for the
        operation is prepared to process.
        - This field cannot work because IRQ handler cannot change in operation.
          If needed user IRQ handler, please update the Cy_Crypto_Server_GetDataHandler.
        - If this field is NULL, server will use own interrupt handler
          to get data.
        - If this field is not NULL, server will call this interrupt handler.
          This interrupt handler must call the
          \ref Cy_Crypto_Server_GetDataHandler to get data to process.

          Note: In the second case user should process data separately and
          clear interrupt by calling \ref Cy_Crypto_Server_Process.
          This model is used in the
          multitasking environment (e.g. CyOS). */
    cy_israddress userGetDataHandler;

    /** Server-side user IRQ handler function, called when a Crypto hardware
        error occurs (interrupt was raised).
        - This field cannot work because IRQ handler cannot change in operation.
          If needed user IRQ handler, please update the Cy_Crypto_Server_ErrorHandler.
        - If this field is NULL - server will use own interrupt handler
          for error processing.
        - If this field is not NULL - server will call this interrupt handler.
          This interrupt handler must call the
          \ref Cy_Crypto_Server_ErrorHandler to clear the interrupt. */
    cy_israddress userErrorHandler;

    /** Specifies the prepared data notifier interrupt configuration. */
    cy_stc_sysint_irq_t acquireNotifierConfig;

    /** Specifies the hardware error processing interrupt configuration. */
    cy_stc_sysint_irq_t cryptoErrorIntrConfig;
#endif

} cy_stc_crypto_config_t;

/** \} group_crypto_config_structure */

/**
* \addtogroup group_crypto_cli_data_structures
* \{
*/

#if (CPUSS_CRYPTO_VU == 1)
/** Structure for storing the RSA public key and additional coefficients to
    accelerate RSA calculation.

    RSA key contained from two fields:
    - n - modulus part of the key
    - e - exponent part of the key.

    Other fields are accelerating coefficients and can be calculated by
    \ref Cy_Crypto_Rsa_CalcCoefs.
*/
typedef struct
{
    /** The pointer to the modulus part of public key. */
    void const *moduloPtr;
    /** The modulus length, in bits, maximum supported size is 2048Bit */
    uint32_t moduloLength;
    /** The pointer to the exponent part of public key */
    void const *pubExpPtr;
    /** The exponent length, in bits, maximum supported size is 256Bit */
    uint32_t pubExpLength;

    /** The pointer to the Barrett coefficient. Memory for it should be
        allocated by user with size moduloLength + 1. */
    void *barretCoefPtr;

    /** The pointer to the binary inverse of the modulo. Memory for it
        should be allocated by user with size moduloLength. */
    void *inverseModuloPtr;
    /** The pointer to the (2^moduloLength mod modulo). Memory for it should
        be allocated by user with size moduloLength */
    void *rBarPtr;
} cy_stc_crypto_rsa_pub_key_t;

#endif /* #if (CPUSS_CRYPTO_VU == 1) */

/** Structure for storing a description of a Crypto hardware error  */
typedef struct
{
    /**
     Captures error description information:
      for INSTR_OPC_ERROR: - Violating instruction (from instruction FIFO).
      for INSTR_CC_ERROR : - Violating instruction (from instruction FIFO).
      for BUS_ERROR      : - violating the transfer address. */
    uint32_t errorStatus0;

    /**
     [31]     - Specifies if ERROR_STATUS0 and ERROR_STATUS1 capture valid
                error-information.
     [26..24] - The error source:
                "0": INSTR_OPC_ERROR (instuction FIFO decoder error).
                "1": INSTR_CC_ERROR (instruction FIFO decoder, VU CC error).
                "2": BUS_ERROR (bus master interface AHB-Lite bus error).
                "3": TR_AP_DETECT_ERROR.
                "4": TR_RC_DETECT_ERROR.
     [23..0]  - Captures error description information.
                For BUS_ERROR:
                  - violating the transfer, read the attribute (DATA23[0]).
                  - violating the transfer, the size attribute (DATA23[5:4]).
                     "0": an 8-bit transfer;
                     "1": 16 bits transfer;
                     "2": 32-bit transfer. */
    uint32_t errorStatus1;
} cy_stc_crypto_hw_error_t;

/** \} group_crypto_cli_data_structures */


/** The Crypto library functionality level. */
typedef enum
{
    CY_CRYPTO_NO_LIBRARY    = 0x00u,
    CY_CRYPTO_BASE_LIBRARY  = 0x01u,
    CY_CRYPTO_EXTRA_LIBRARY = 0x02u,
    CY_CRYPTO_FULL_LIBRARY  = 0x03u,
} cy_en_crypto_lib_info_t;


/**
* \addtogroup group_crypto_enums
* \{
*/

#if (CPUSS_CRYPTO_AES == 1)
/** The key length options for the AES method. */
typedef enum
{
    CY_CRYPTO_KEY_AES_128   = 0x00u,   /**< The AES key size is 128 bits */
    CY_CRYPTO_KEY_AES_192   = 0x01u,   /**< The AES key size is 192 bits */
    CY_CRYPTO_KEY_AES_256   = 0x02u    /**< The AES key size is 256 bits */
} cy_en_crypto_aes_key_length_t;
#endif /* #if (CPUSS_CRYPTO_AES == 1) */

/** Defines the direction of the Crypto methods */
typedef enum
{
    /** The forward mode, plain text will be encrypted into cipher text */
    CY_CRYPTO_ENCRYPT   = 0x00u,
    /** The reverse mode, cipher text will be decrypted into plain text */
    CY_CRYPTO_DECRYPT   = 0x01u
} cy_en_crypto_dir_mode_t;

#if (CPUSS_CRYPTO_SHA == 1)
/** Defines modes of SHA method */
typedef enum
{
#if (CPUSS_CRYPTO_SHA1 == 1)
    CY_CRYPTO_MODE_SHA1        = 0x00u,   /**< Sets the SHA1 mode */
#endif /* #if (CPUSS_CRYPTO_SHA1 == 1) */

#if (CPUSS_CRYPTO_SHA2 == 1)
    CY_CRYPTO_MODE_SHA224      = 0x11u,   /**< Sets the SHA224 mode */
    CY_CRYPTO_MODE_SHA256      = 0x01u,   /**< Sets the SHA256 mode */
    CY_CRYPTO_MODE_SHA384      = 0x12u,   /**< Sets the SHA384 mode */
    CY_CRYPTO_MODE_SHA512      = 0x02u,   /**< Sets the SHA512 mode */
    CY_CRYPTO_MODE_SHA512_256  = 0x22u,   /**< Sets the SHA512/256 mode */
    CY_CRYPTO_MODE_SHA512_224  = 0x32u,   /**< Sets the SHA512/224 mode */
#endif /* #if (CPUSS_CRYPTO_SHA2 == 1) */
#if (CPUSS_CRYPTO_SHA3 == 1)
    CY_CRYPTO_MODE_SHA3_224    = 0x81u,   /**< Sets the SHA3_224 mode */
    CY_CRYPTO_MODE_SHA3_256    = 0x82u,   /**< Sets the SHA3_256 mode */
    CY_CRYPTO_MODE_SHA3_384    = 0x83u,   /**< Sets the SHA3_384 mode */
    CY_CRYPTO_MODE_SHA3_512    = 0x84u,   /**< Sets the SHA3_512 mode */
    CY_CRYPTO_MODE_SHAKE128    = 0x85u,   /**< Sets the SHAKE128 mode */
    CY_CRYPTO_MODE_SHAKE256    = 0x86u    /**< Sets the SHAKE256 mode */
#endif /* #if (CPUSS_CRYPTO_SHA3 == 1) */
} cy_en_crypto_sha_mode_t;
#endif /* #if (CPUSS_CRYPTO_SHA == 1) */

/** Signature verification status */
typedef enum
{
    CY_CRYPTO_RSA_VERIFY_SUCCESS = 0x00u,   /**< PKCS1-v1.5 verify SUCCESS */
    CY_CRYPTO_RSA_VERIFY_FAIL    = 0x01u    /**< PKCS1-v1.5 verify FAILED */
} cy_en_crypto_rsa_ver_result_t;

#if (CPUSS_CRYPTO_CHACHA == 1)
/** ChaCha round number selection */
typedef enum
{
    CY_CRYPTO_CHACHA8   =  8u / 2u, /**< CHACHA8 */
    CY_CRYPTO_CHACHA12  = 12u / 2u, /**< CHACHA12 */
    CY_CRYPTO_CHACHA20  = 20u / 2u  /**< CHACHA20 */
} cy_en_crypto_chacha_round_t;
#endif /* #if (CPUSS_CRYPTO_CHACHA == 1) */

/** Errors of the Crypto block */
typedef enum
{
    /** Operation completed successfully. */
    CY_CRYPTO_SUCCESS             = 0x00u,

    /** A hardware error occurred, detailed information is in stc_crypto_hw_error_t. */
    CY_CRYPTO_HW_ERROR            = CY_CRYPTO_ID | CY_PDL_STATUS_ERROR   | 0x01u,

    /** The size of input data is not multiple of 16. */
    CY_CRYPTO_SIZE_NOT_X16        = CY_CRYPTO_ID | CY_PDL_STATUS_ERROR   | 0x02u,

    /** The key for the DES method is weak. */
    CY_CRYPTO_DES_WEAK_KEY        = CY_CRYPTO_ID | CY_PDL_STATUS_WARNING | 0x03u,

    /** Communication between the client and server via IPC is broken. */
    CY_CRYPTO_COMM_FAIL           = CY_CRYPTO_ID | CY_PDL_STATUS_ERROR   | 0x04u,

    /** The Crypto server is not started. */
    CY_CRYPTO_SERVER_NOT_STARTED  = CY_CRYPTO_ID | CY_PDL_STATUS_ERROR   | 0x06u,

    /** The Crypto server in process state. */
    CY_CRYPTO_SERVER_BUSY         = CY_CRYPTO_ID | CY_PDL_STATUS_INFO    | 0x07u,

    /** The Crypto driver is not initialized. */
    CY_CRYPTO_NOT_INITIALIZED     = CY_CRYPTO_ID | CY_PDL_STATUS_ERROR   | 0x08u,

    /** The Crypto hardware is not enabled. */
    CY_CRYPTO_HW_NOT_ENABLED      = CY_CRYPTO_ID | CY_PDL_STATUS_ERROR   | 0x09u,

    /** The Crypto operation is not supported. */
    CY_CRYPTO_NOT_SUPPORTED       = CY_CRYPTO_ID | CY_PDL_STATUS_ERROR   | 0x0Au,

    /** Invalid parameter is passed */
    CY_CRYPTO_BAD_PARAM           = CY_CRYPTO_ID | CY_PDL_STATUS_ERROR   | 0x0Bu

} cy_en_crypto_status_t;

/** \} group_crypto_enums */

/** \cond INTERNAL */

/** Instruction to communicate between Client and Server */
typedef enum
{
    CY_CRYPTO_INSTR_UNKNOWN      = 0x00u,
    CY_CRYPTO_INSTR_ENABLE       = 0x01u,
    CY_CRYPTO_INSTR_DISABLE      = 0x02u,

#if (CPUSS_CRYPTO_PR == 1)
    CY_CRYPTO_INSTR_PRNG_INIT    = 0x03u,
    CY_CRYPTO_INSTR_PRNG         = 0x04u,
#endif /* #if (CPUSS_CRYPTO_PR == 1) */

#if (CPUSS_CRYPTO_TR == 1)
    CY_CRYPTO_INSTR_TRNG_INIT    = 0x05u,
    CY_CRYPTO_INSTR_TRNG         = 0x06u,
#endif /* #if (CPUSS_CRYPTO_PR == 1) */

#if (CPUSS_CRYPTO_AES == 1)
    CY_CRYPTO_INSTR_AES_ECB      = 0x08u,
    CY_CRYPTO_INSTR_AES_CBC      = 0x09u,
    CY_CRYPTO_INSTR_AES_CFB      = 0x0Au,
    CY_CRYPTO_INSTR_AES_CTR      = 0x0Bu,
    CY_CRYPTO_INSTR_CMAC         = 0x0Cu,
#endif /* #if (CPUSS_CRYPTO_AES == 1) */

#if (CPUSS_CRYPTO_SHA == 1)
    CY_CRYPTO_INSTR_SHA          = 0x0Du,
#endif /* #if (CPUSS_CRYPTO_SHA == 1) */

#if (CPUSS_CRYPTO_SHA == 1)
    CY_CRYPTO_INSTR_HMAC         = 0x0Eu,
#endif /* #if (CPUSS_CRYPTO_SHA == 1) */

#if (CPUSS_CRYPTO_CRC == 1)
    CY_CRYPTO_INSTR_CRC_INIT     = 0x13u,
    CY_CRYPTO_INSTR_CRC          = 0x14u,
#endif /* #if (CPUSS_CRYPTO_CRC == 1) */

#if (CPUSS_CRYPTO_DES == 1)
    CY_CRYPTO_INSTR_DES          = 0x15u,
    CY_CRYPTO_INSTR_3DES         = 0x16u,
#endif /* #if (CPUSS_CRYPTO_DES == 1) */

#if (CPUSS_CRYPTO_VU == 1)
    CY_CRYPTO_INSTR_RSA_PROC     = 0x17u,
    CY_CRYPTO_INSTR_RSA_COEF     = 0x18u,
#endif /* #if (CPUSS_CRYPTO_VU == 1)*/

#if (CPUSS_CRYPTO_SHA == 1)
    CY_CRYPTO_INSTR_RSA_VER      = 0x19u,
#endif /* #if (CPUSS_CRYPTO_SHA == 1) */

#if (CPUSS_CRYPTO_CHACHA == 1)
    CY_CRYPTO_INSTR_CHACHA       = 0x1Au,
#endif /* #if (CPUSS_CRYPTO_CHACHA == 1) */
    CY_CRYPTO_INSTR_SRV_INFO     = 0x55u
} cy_en_crypto_comm_instr_t;

/** \endcond */

/**
* \addtogroup group_crypto_cli_data_structures
* \{
*/

#if (CPUSS_CRYPTO_AES == 1)
/** Structure for storing the AES state */
typedef struct
{
    /** Pointer to AES key */
    void const *keyPtr;
    /** AES key length */
    cy_en_crypto_aes_key_length_t keyLength;
    /** AES processed block index (for CMAC operations) */
    uint32_t blockIdx;
} cy_stc_crypto_aes_state_t;
#endif /* #if (CPUSS_CRYPTO_AES == 1) */

/** \} group_crypto_cli_data_structures */

/*************************************************************
*  Structures used for communication between Client and Server
***************************************************************/

/**
* \addtogroup group_crypto_srv_data_structures
* \{
*/

/** Structure for storing the server context */
typedef struct
{
    /** IPC communication channel number */
    uint32_t ipcChannel;
    /** IPC acquire interrupt channel number */
    uint32_t acquireNotifierChannel;
    /** IPC release interrupt channel number */
    uint32_t releaseNotifierChannel;
    /** Interrupt handler to get processing data */
    cy_israddress   getDataHandlerPtr;
    /** Crypto hardware errors interrupt handler */
    cy_israddress   errorHandlerPtr;
    /** Acquire notifier interrupt configuration */
    cy_stc_sysint_irq_t acquireNotifierConfig;
    /** Crypto hardware errors interrupt configuration */
    cy_stc_sysint_irq_t cryptoErrorIntrConfig;
    /** Hardware error occurrence flag */
    bool            isHwErrorOccured;
} cy_stc_crypto_server_context_t;

/** \} group_crypto_srv_data_structures */

/**
* \addtogroup group_crypto_cli_data_structures
* \{
*/

/** Structure for storing the global context */
typedef struct
{
    /** Operation instruction code */
    cy_en_crypto_comm_instr_t instr;
    /** Response from executed crypto function */
    cy_en_crypto_status_t resp;
    /** Hardware processing errors */
    cy_stc_crypto_hw_error_t hwErrorStatus;
    /** IPC communication channel number */
    uint32_t ipcChannel;
    /** IPC acquire interrupt channel number */
    uint32_t acquireNotifierChannel;
    /** IPC release interrupt channel number */
    uint32_t releaseNotifierChannel;
    /** User callback for Crypto HW calculation complete event */
    cy_crypto_callback_ptr_t userCompleteCallback;
    /** Release notifier interrupt configuration */
    cy_stc_sysint_irq_t releaseNotifierConfig;
    /** Pointer to the crypto function specific context data */
    void *xdata;
} cy_stc_crypto_context_t;


#if (CPUSS_CRYPTO_DES == 1)
/** Structure for storing the DES context */
typedef struct
{
    /**  Operation direction (Encrypt / Decrypt) */
    cy_en_crypto_dir_mode_t dirMode;
    /**  Pointer to key data */
    void const *keyPtr;
    /**  Pointer to data destination block */
    void *dstPtr;
    /**  Pointer to data source block */
    void const *srcPtr;
} cy_stc_crypto_context_des_t;
#endif /* #if (CPUSS_CRYPTO_DES == 1) */

#if (CPUSS_CRYPTO_AES == 1)
/** Structure for storing the AES context */
typedef struct
{
    /** AES state data */
    cy_stc_crypto_aes_state_t aesState;
    /** Operation direction (Encrypt / Decrypt) */
    cy_en_crypto_dir_mode_t dirMode;
    /** Operation data size */
    uint32_t srcSize;
    /** Size of the last non-complete block (for CTR mode only)
      *  Other than CTR mode can be set to NULL.
      *  For the first sequence, *srcOffset value should be 0.
      *  This parameter is updated by CRYPTO module.
      *  For the first call for a sequence, this field shall be initialized
      *  to '0'. For the trailing call for the sequence,
      *  it shall not be initialized.
      */
    uint32_t *srcOffset;
    /** Initialization vector, in the CTR mode is used as nonceCounter
      *  In the CTR mode, counter value is incremented by the API.
      *  For the first call for a sequence, this field shall be initialized
      *  required nonceCounter value. For the trailing call for the sequence,
      *  it shall not be initialized.
      */
    void *ivPtr;
    /** Pointer to data destination block */
    void *dstPtr;
    /** Pointer to data source block */
    void const *srcPtr;
} cy_stc_crypto_context_aes_t;
#endif /* #if (CPUSS_CRYPTO_AES == 1) */

#if (CPUSS_CRYPTO_SHA == 1)
/** Structure for storing the SHA context */
typedef struct
{
    /** Pointer to data source block */
    void const *messagePtr;
    /** Operation data size */
    uint32_t  messageSize;
    /** Pointer to data destination block */
    void *dstPtr;
    /** SHA mode */
    cy_en_crypto_sha_mode_t mode;
    /** Pointer to key data (for HMAC only) */
    void const *keyPtr;
    /** Key data length (for HMAC only) */
    uint32_t  keyLength;
} cy_stc_crypto_context_sha_t;
#endif /* #if (CPUSS_CRYPTO_SHA == 1) */

#if (CPUSS_CRYPTO_PR == 1)
/** Structure for storing the PRNG context */
typedef struct
{
    uint32_t lfsr32InitState;                               /**< lfsr32 initialization data (must be none zero) */
    uint32_t lfsr31InitState;                               /**< lfsr31 initialization data (must be none zero) */
    uint32_t lfsr29InitState;                               /**< lfsr29 initialization data (must be none zero) */
    uint32_t max;                                           /**< Maximum of the generated value */
    uint32_t *prngNumPtr;                                   /**< Pointer to generated value */
} cy_stc_crypto_context_prng_t;
#endif /* #if (CPUSS_CRYPTO_PR == 1) */

#if (CPUSS_CRYPTO_TR == 1)
/** Structure for storing the TRNG context */
typedef struct
{
    /**
     The polynomial for the programmable Galois ring oscillator (TR_GARO_CTL).
     The polynomial is represented WITHOUT the high order bit (this bit is
     always assumed '1').
     The polynomial should be aligned so that more significant bits
     (bit 30 and down) contain the polynomial and less significant bits
     (bit 0 and up) contain padding '0's. */
    uint32_t GAROPol;

    /**
     The polynomial for the programmable Fibonacci ring oscillator(TR_FIRO_CTL).
     The polynomial is represented WITHOUT the high order bit (this bit is
     always assumed '1').
     The polynomial should be aligned so that more significant bits
     (bit 30 and down) contain the polynomial and less significant bits
     (bit 0 and up) contain padding '0's. */
    uint32_t FIROPol;
    /** Bit size of the generated value. Valid range is [0,32] */
    uint8_t bitSize;
    /** Pointer to generated value */
    uint32_t *trngNumPtr;
} cy_stc_crypto_context_trng_t;
#endif /* #if (CPUSS_CRYPTO_TR == 1) */

#if (CPUSS_CRYPTO_CRC == 1)
/** Structure for storing the CRC context */
typedef struct
{
    void const *srcDataPtr;         /**<  Pointer to data source block */
    uint32_t dataSize;              /**<  operation data size */
    uint32_t *crcPtr;               /**<  Pointer to CRC destination variable */
    uint32_t polynomial;            /**<  Polynomial for CRC calculate */
    uint32_t lfsrInitState;         /**<  CRC calculation initial value */
    uint32_t dataReverse;           /**<  Input data reverse flag */
    uint32_t dataXor;               /**<  Input data  XOR flag */
    uint32_t remReverse;            /**<  Output data reverse flag */
    uint32_t remXor;                /**<  Output data XOR flag */
} cy_stc_crypto_context_crc_t;
#endif /* #if (CPUSS_CRYPTO_CRC == 1) */

/** Structure for storing the RSA verify context */
typedef struct
{
    /** Pointer to verification result /ref cy_en_crypto_rsa_ver_result_t */
    cy_en_crypto_rsa_ver_result_t *verResult;
    /** SHA digest type, used with SHA calculation of the message */
    cy_en_crypto_sha_mode_t digestType;
    /** SHA digest of the message, calculated with digestType */
    void const *hashPtr;
    /** Previously decrypted RSA signature */
    void const *decryptedSignaturePtr;
    /** Length of the decrypted RSA signature */
    uint32_t decryptedSignatureLength;
} cy_stc_crypto_context_rsa_ver_t;

#if (CPUSS_CRYPTO_VU == 1)
/** Structure for storing the RSA process context */
typedef struct
{
    /** Pointer to key data */
    cy_stc_crypto_rsa_pub_key_t const *keyPtr;
    /** Pointer to data source block */
    void const *messagePtr;
    /** Operation data size */
    uint32_t messageLength;
    /** Pointer to data destination block */
    void *resultPtr;
} cy_stc_crypto_context_rsa_t;
#endif /* #if (CPUSS_CRYPTO_VU == 1)*/

#if (CPUSS_CRYPTO_CHACHA == 1)
/** Structure for storing the CHACHA context */
typedef struct
{
    /** Operation data size */
    uint32_t srcSize;
    /** Pointer to data destination block */
    void *dstPtr;
    /** Pointer to data source block */
    void const *srcPtr;
    /** Initial nonce value (only 96bits is supported in the driver.) */
    void const *noncePtr;
    /** Initial counter value */
    uint32_t counter;
    /** Pointer to CHACHA key (Only 256bits is supported in the driver). */
    void const *keyPtr;
    /** half of a round number. Refer \ref cy_en_crypto_chacha_round_t */
    cy_en_crypto_chacha_round_t  roundNum;
} cy_stc_crypto_context_chacha_t;
#endif /* #if (CPUSS_CRYPTO_CHACHA == 1) */

/** \} group_crypto_data_structures */

/** \} group_crypto_cli_data_structures */

#endif  /* (CPUSS_CRYPTO_PRESENT == 1) */

#endif /* #if !defined(CY_CRYPTO_COMMON_H) */

/* [] END OF FILE */
