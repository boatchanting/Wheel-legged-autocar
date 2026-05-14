/***************************************************************************//**
* \file cy_crypto.h
* \version 1.0
*
* \brief
*  This file provides the public interface for the Crypto driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_crypto Cryptography (Crypto)
* \{
* The Crypto driver provides an API to perform
* cryptographic operations on user-designated data. Driver uses the hardware
* IP block to accelerate of the cryptographic operations.
*
* Definitions
*
* <table class="doxtable">
*   <tr>
*     <th>Term</th>
*     <th>Definition</th>
*   </tr>
*
*   <tr>
*     <td>Plaintext</td>
*     <td>An unencrypted message</td>
*   </tr>
*
*   <tr>
*     <td>Ciphertext</td>
*     <td>An encrypted message</td>
*   </tr>
*
*   <tr>
*     <td>Block Cipher</td>
*     <td>An encryption function for fixed-size blocks of data.
*     This function takes a fixed-size key and a block of plaintext data from
*     the message and encrypts it to generate ciphertext. Block ciphers are
*     reversible. The function performed on a block of encrypted data will
*     decrypt it.</td>
*   </tr>
*
*   <tr>
*     <td>Block Cipher Mode</td>
*     <td>A mode of encrypting a message using block ciphers for messages of
*     arbitrary length. The message is padded so that its length is an integer
*     multiple of the block size. ECB (Electronic Code Book), CBC (Cipher Block
*     Chaining), and CFB (Cipher Feedback) are all modes of using block ciphers
*     to create an encrypted message of arbitrary length.</td>
*   </tr>
*
*   <tr>
*     <td>Data Encryption Standard (DES)</td>
*     <td>Is a symmetric-key algorithm for the encryption of electronic data.
*     It uses a 56-bit key and a 64-bit message block size.</td>
*   </tr>
*
*   <tr>
*     <td>Triple DES (3DES or TDES)</td>
*     <td>Is a symmetric-key block cipher, which applies the Data Encryption
*     Standard (DES) cipher algorithm three times to each data block.
*     It uses three 56-bit keys. The block size is 64-bits.</td>
*   </tr>
*
*   <tr>
*     <td>Advanced Encryption Standard (AES)</td>
*     <td>The standard specifies the Rijndael algorithm, a symmetric block
*     cipher that can process data blocks of 128 bits, using cipher keys with
*     lengths of 128, 192, and 256 bits. Rijndael was designed to handle
*     additional block sizes and key lengths, however they are not adopted in
*     this standard. AES is also used for message authentication.</td>
*   </tr>
*
*   <tr>
*     <td>Cipher-based Message Authentication Code (CMAC)</td>
*     <td>This is a block cipher-based message authentication code algorithm.
*     It computes the MAC value using the AES block cipher algorithm.</td>
*   </tr>
*
*   <tr>
*     <td>Secure Hash Algorithm (SHA)</td>
*     <td>Is a cryptographic hash function.
*     This function takes a message of the arbitrary length and reduces it to a
*     fixed-length residue or message digest after performing a series of
*     mathematically defined operations that practically guarantee that any
*     change in the message will change the hash value. It is used for message
*     authentication by transmitting a message with a hash value appended to it
*     and recalculating the message hash value using the same algorithm at the
*     recipient's end. If the hashes differ, then the message is corrupted.</td>
*   </tr>
*
*   <tr>
*     <td>Message Authentication Code (MAC)</td>
*     <td>MACs are used to verify that a received message has not been altered.
*     This is done by first computing a MAC value at the sender's end and
*     appending it to the transmitted message. When the message is received,
*     the MAC is computed again and checked against the MAC value transmitted
*     with the message. If they do not match, the message has been altered.
*     Either a Hash algorithm (such as SHA) or a block cipher (such as AES) can
*     be used to produce the MAC value. Keyed MAC schemes use a Secret Key
*     along with the message, thus the Key value must be known to be able to
*     compute the MAC value.</td>
*   </tr>
*
*   <tr>
*     <td>Hash Message Authentication Code (HMAC)</td>
*     <td>Is a specific type of message authentication code (MAC) involving a
*     cryptographic hash function and a secret cryptographic key.
*     It computes the MAC value using a Hash algorithm.</td>
*   </tr>
*
*   <tr>
*     <td>Pseudo Random Number Generator (PRNG)</td>
*     <td>Is a Linear Feedback Shift Registers-based algorithm for generating a
*     sequence of numbers starting from a non-zero seed.</td>
*   </tr>
*
*   <tr>
*     <td>True Random Number Generator (TRNG)</td>
*     <td>A block that generates a number that is statistically random and based
*     on some physical random variation. The number cannot be duplicated by running the process again.</td>
*   </tr>
*
*   <tr>
*     <td>CHACHA</td>
*     <td>is a symmetric stream cipher specified in RFC 7539.</td>
*   </tr>
*
*   <tr>
*     <td>Symmetric Key Cryptography</td>
*     <td>Uses a common, known key to encrypt and decrypt messages (a shared
*     secret between sender and receiver). An efficient method used for
*     encrypting and decrypting messages after the authenticity of the other
*     party has been established. DES (now obsolete), 3DES, and AES (currently
*     used) are well-known symmetric cryptography methods.</td>
*   </tr>
*
*   <tr>
*     <td>Asymmetric Key Cryptography</td>
*     <td>Also referred to as Public Key encryption. Someone who wishes to
*     receive a message, publishes a very large public key (up to 4096 bits
*     currently), which is one of two prime factors of a very large number. The
*     other prime factor is the private key of the recipient and a secret.
*     Someone wishing to send a message to the publisher of the public key
*     encrypts the message with the public key. This message can now be
*     decrypted only with the private key (the other prime factor held secret by
*     the recipient). The message is now sent over any channel to the recipient
*     who can decrypt it with the private, secret, key. The same process is used
*     to send messages to the sender of the original message. The asymmetric
*     cryptography relies on the mathematical impracticality (usually related to
*     the processing power available at any given time) of factoring the keys.
*     Common, computationally intensive, asymmetric algorithms are RSA and ECC.
*     The public key is described by the pair (n, e) where n is a product of two
*     randomly chosen primes p and q. The exponent e is a random integer
*     1 < e < Q where Q = (p-1) (q-1). The private key d is uniquely defined
*     by the integer 1 < d < Q such that ed congruent to 1 (mod Q ).</td>
*   </tr>
* </table>
*
* Crypto supports: DES, TDES, AES (128, 192, 256 bits), CMAC-AES, SHA, CHACHA,
* HMAC, PRNG, TRNG, CRC, RSA processing (basic RSA operation, c(m) = m^e mod n),
* and PKCS1-v1_5 verification procedure (supports only SHA paddings).
*
* The design of the Crypto driver is based on a client-server architecture.
* The Crypto server runs only on the CM0+ core, and works with the Crypto
* hardware by calls to the appropriate Crypto Core functions.
* The Crypto client can be run on either core. The client and server
* communicate through the \ref group_ipc. Using IPC for communication provides a
* simple synchronization mechanism to handle concurrent requests from different
* cores.
*
* Crypto driver utilizes:
* - 1 IPC channel to data exchange between client and server,
* - 2 IPC interrupt structures for notifications,
* - 1 interrupt for hardware errors handling.
*
* \section group_crypto_configuration Configuration Considerations
*
* \subsection group_crypto_srv_initialization Server Initialization
*
* Use \ref Cy_Crypto_Server_Start to start the Crypto server with configuration
* parameters (\ref cy_stc_crypto_config_t) and server context
* (\ref cy_stc_crypto_server_context_t).
* After this, the Crypto server will be ready to process data from the Crypto
* client.\n
*
* This call must be made from the CM0+ core only.
*
* \subsection group_crypto_cli_initialization Client initialization
*
* Use \ref Cy_Crypto_Init to initialize the Crypto client with the configuration
* parameters (\ref cy_stc_crypto_config_t) and context
* (\ref cy_stc_crypto_context_t).\n
* Then use \ref Cy_Crypto_Enable to enable the Crypto hardware IP block.
* After this, the Crypto driver and hardware will be ready to execute crypto
* functions.\n
*
* These calls must be made on the client side.
*
* Some encryption methods also have an initialization function.
* Use the function you need to configure the encryption operation and specific
* context for next step(s). For example, use \ref Cy_Crypto_Aes_Init to
* configure an AES encryption operation and provide the encryption key and key
* length and store its to the operation context
* (\ref cy_stc_crypto_context_aes_t).
* See the documentation for each Cy_Crypto_XXX_Init function.
*
* \note All functions require a context. All contexts are mandatory.
*
* \subsection group_crypto_client_server Client-server Model Considerations
*
* The two cores operate at different frequencies. As a result, calls to
* \ref Cy_Crypto_Init or \ref Cy_Crypto_Enable on the CM4 core could be invoked
* before \ref Cy_Crypto_Server_Start completes on the CM0+ core.
* You should check the return status from calls to \ref Cy_Crypto_Init or \ref
* Cy_Crypto_Enable to ensure \ref CY_CRYPTO_SUCCESS.
* You can also make the Cy_SysEnableCM4 call from the CM0+ core
* after \ref Cy_Crypto_Server_Start has executed.
*
* The Crypto client sends a pointer to the data structure that contains all
* the needed information for a particular encryption. At that time the Crypto
* IPC channel is locked. The Crypto server receives the data, runs the Crypto
* Core function, waits until the IP block operation is completed, and releases
* the IPC channel.
*
* All crypto operations are asynchronous. To check (or wait) that crypto
* operation was completed and result is valid the \ref Cy_Crypto_Sync is used.
*
* Use \ref Cy_Crypto_Sync with \ref CY_CRYPTO_SYNC_NON_BLOCKING parameter to
* check completion status only. Use \ref Cy_Crypto_Sync with \ref
* CY_CRYPTO_SYNC_BLOCKING parameter to wait for operation complete.
*
* Alternatively, the IPC release interrupt can be used to inform the application
* that a Crypto operation has finished. This is accomplished by populating the
* userCompleteCallback and setting up the release interrupt in the Crypto IPC.
*
* Set the function pointer to your callback in the configuration structure field
* named userCompleteCallback. The PDL interrupt handling routine clears the
* interrupt and calls your callback function.
*
* \subsection group_crypto_conf_rsa RSA Usage Considerations
*
* \subsubsection group_crypto_conf_rsa_proc RSA Encrypt and Decrypt
*
* \note The \ref Cy_Crypto_Rsa_Proc requires an incoming data
* ("message" parameter) in the little-endian format (as big integer)
* but \ref Cy_Crypto_Rsa_Verify requires signature to verify and SHA digest of
* the message in the big-endian format (as the octet string).
*
* So, if you have a signature created as described by the RSASSA-PKCS1-v1_5,
* you must revert the encrypted signature by \ref Cy_Crypto_Rsa_InvertEndianness
* before passing it to \ref Cy_Crypto_Rsa_Proc and reverting the decrypted
* signature after the decrypt operation.
*
* \subsubsection group_crypto_conf_rsa_verify RSA Verify
*
* Use \ref Cy_Crypto_Rsa_Verify for an RSA signature verification. \n
* It currently supports only RSASSA-PKCS1-v1_5 signature verification format. \n
* Inside of this, format supported padding using only SHA.
* Cases with MD2 and MD5 are not supported because MD algorithms are not
* supported by the Crpyto anyway.
*
* Usage:
* 1. Calculate the SHA digest of the data with \ref Cy_Crypto_Sha_Run.
* 2. Ensure that RSA signature is provided the little-endian format.
*    Note: RSASSA-PKCS1-v1_5 describes the signature to be an "octet string"
*          that has to be byte reversed to be converted to
*          little-endian big integer.
*    Use either \ref Cy_Crypto_Rsa_InvertEndianness or any other function
*    to reverse the data.
* 3. Decrypt the RSA signature by calling \ref Cy_Crypto_Rsa_Proc.
* 4. Revert byte order of the output of \ref Cy_Crypto_Rsa_Proc.
*    This is required because \ref Cy_Crypto_Rsa_Verify requires the data in
*    a big-endian format, and the \ref Cy_Crypto_Rsa_Proc output is in
*    a little-endian format.
* 5. Call \ref Cy_Crypto_Rsa_Verify.
*
* \section group_crypto_more_information More Information
*
* RSASSA-PKCS1-v1_5 described here, page 31:
* http://www.emc.com/collateral/white-papers/h11300-pkcs-1v2-2-rsa-cryptography-standard-wp.pdf*
*
* See the Cryptographic Function Block chapter of the Technical Reference Manual.
*
* \section group_crypto_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
*
* \defgroup group_crypto_common Configuration
* \{
*   \defgroup group_crypto_config_macros Macros
*   \defgroup group_crypto_config_structure Structure
* \}
*
* \defgroup group_crypto_client Crypto Client
* \{
*   \defgroup group_crypto_macros Macros
* \defgroup group_crypto_enums Enumerated Types
*   \defgroup group_crypto_cli_data_structures Data Structures
*   \defgroup group_crypto_cli_functions Functions
* \}
*
* \defgroup group_crypto_server Crypto Server
* \{
*   \defgroup group_crypto_srv_data_structures Data Structures
*   \defgroup group_crypto_srv_functions Functions
* \}
*/

#if !defined(CY_CRYPTO_H)
#define CY_CRYPTO_H


#include <stddef.h>
#include <stdbool.h>
#include "cy_device_headers.h"
#include "crypto/cy_crypto_common.h"

#if (CPUSS_CRYPTO_PRESENT == 1)


#if defined(__cplusplus)
extern "C" {
#endif


/** \cond INTERNAL */

cy_en_crypto_status_t Cy_Crypto_GetLibraryInfo(cy_en_crypto_lib_info_t *cryptoInfo, cy_stc_crypto_context_t  *context);

/** \endcond */

/**
* \addtogroup group_crypto_cli_functions
* \{
*/

/*******************************************************************************
* Function Name: Cy_Crypto_Init
****************************************************************************//**
*
* This function initializes the Crypto context buffer and
* configures the Crypto driver. Must be called at first.
*
* To start working with Crypto methods after Crypto_Init(),
* call Crypto_Enable() to turn-on the Crypto Hardware.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t instance of structure
* that stores the Crypto driver common context.
*
* \param configStruct
* The pointer to the Crypto configuration structure.
* Could be used with default values from cy_crypto_config.h
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
* \note
* After calling this function, following functions have to be called.
*   NVIC_EnableIRQ(configStruct->releaseIntrNum);
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Init(cy_stc_crypto_config_t const *configStruct, cy_stc_crypto_context_t *cryptoContext);

/*******************************************************************************
* Function Name: Cy_Crypto_DeInit
****************************************************************************//**
*
* This function de-initializes the Crypto driver.
*
* \param context
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_DeInit(cy_stc_crypto_context_t *context);

/*******************************************************************************
* Function Name: Cy_Crypto_Enable
****************************************************************************//**
*
* This function enables (turns on) the Crypo hardware.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Enable(cy_stc_crypto_context_t *cryptoContext);

/*******************************************************************************
* Function Name: Cy_Crypto_Disable
****************************************************************************//**
*
* This function disables (turns off) the Crypto hardware.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Disable(cy_stc_crypto_context_t *cryptoContext);

/*******************************************************************************
* Function Name: Cy_Crypto_Sync
****************************************************************************//**
*
* This function waits or just checks (depending on the parameter)
* for the Crypto operation to complete.
*
* \param isBlocking
* Set whether Crypto_Sync is blocking:
* True - is blocking.
* False - is not blocking.
*
* \param context
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Sync(bool isBlocking, cy_stc_crypto_context_t const *context);

/*******************************************************************************
* Function Name: Cy_Crypto_GetErrorStatus
****************************************************************************//**
*
* This function returns a cause of a Crypto hardware error.
* It is independent of the Crypto previous state.
*
* \param hwErrorCause
* \ref cy_stc_crypto_hw_error_t.
*
* \param context
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_GetErrorStatus(cy_stc_crypto_hw_error_t *hwErrorCause,
                                               const cy_stc_crypto_context_t *context);

#if (CPUSS_CRYPTO_PR == 1)
/*******************************************************************************
* Function Name: Cy_Crypto_Prng_Init
****************************************************************************//**
*
* This function initializes parameters of the PRND.
* Should be called at least once before invoking Cy_Crypto_Prng_Generate().
* Invoking this function resets the pseudo random sequence.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoPrngContext
* The pointer to the \ref cy_stc_crypto_context_prng_t structure that stores
* the Crypto function context.
*
* \param lfsr32InitState
* A non-zero seed value for the first LFSR.

* \param lfsr31InitState
* A non-zero seed value for the second LFSR.

* \param lfsr29InitState
* A non-zero seed value for the third LFSR.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Prng_Init(cy_stc_crypto_context_t *cryptoContext,
                                          cy_stc_crypto_context_prng_t *cryptoPrngContext,
                                          uint32_t lfsr32InitState,
                                          uint32_t lfsr31InitState,
                                          uint32_t lfsr29InitState);

/*******************************************************************************
* Function Name: Cy_Crypto_Prng_Generate
****************************************************************************//**
*
* This function generates the Pseudo Random Number.
* It depends on Cy_Crypto_Prng_Init that should be called before.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoPrngContext
* The pointer to the \ref cy_stc_crypto_context_prng_t structure that stores
* the Crypto function context.
*
* \param max
* The maximum value of a random number.
*
* \param rndNumPtr
* The pointer to a variable to store the generated pseudo random number.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Prng_Generate(cy_stc_crypto_context_t *cryptoContext,
                                              cy_stc_crypto_context_prng_t *cryptoPrngContext,
                                              uint32_t max,
                                              uint32_t *rndNumPtr);
#endif /* #if (CPUSS_CRYPTO_PR == 1) */

#if (CPUSS_CRYPTO_AES == 1)

/*******************************************************************************
* Function Name: Cy_Crypto_Aes_Ecb_Run
****************************************************************************//**
*
* This function performs AES operation on one block.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoAesContext
* The pointer to the \ref cy_stc_crypto_context_aes_t instance of structure
* that stores all AES internal variables.
*
* \param dirMode
* Can be CRYPTO_ENCRYPT or CRYPTO_DECRYPT.
*
* \param keyPtr
* The pointer to the encryption/decryption key.
*
* \param keyLength
* One of these: CRYPTO_AES_128, CRYPTO_AES_192, or CRYPTO_AES_256.
*
* \param srcBlockPtr
* The pointer to a source block.
*
* \param dstBlockPtr
* The pointer to a destination cipher block.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Aes_Ecb_Run(cy_stc_crypto_context_t *cryptoContext,
                                            cy_stc_crypto_context_aes_t *cryptoAesContext,
                                            cy_en_crypto_dir_mode_t dirMode,
                                            void const *keyPtr,
                                            cy_en_crypto_aes_key_length_t keyLength,
                                            void *dstBlockPtr,
                                            void const *srcBlockPtr);

/*******************************************************************************
* Function Name: Cy_Crypto_Aes_Cbc_Run
****************************************************************************//**
*
* This function performs AES operation on a plain text with Cipher Block Chaining (CBC).
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoAesContext
* The pointer to the \ref cy_stc_crypto_context_aes_t structure that stores all internal variables
* the Crypto driver requires.
*
* \param dirMode
* Can be CRYPTO_ENCRYPT or CRYPTO_DECRYPT.
*
* \param keyPtr
* The pointer to the encryption/decryption key.
*
* \param keyLength
* One of these: CRYPTO_AES_128, CRYPTO_AES_192, or CRYPTO_AES_256.
*
* \param srcSize
* The size of the source plain text.
*
* \param ivPtr
* The pointer to the initial vector.
*
* \param dstPtr
* The pointer to a destination cipher text.
*
* \param srcPtr
* The pointer to a source plain text.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Aes_Cbc_Run(cy_stc_crypto_context_t *cryptoContext,
                                            cy_stc_crypto_context_aes_t *cryptoAesContext,
                                            cy_en_crypto_dir_mode_t dirMode,
                                            void const *keyPtr,
                                            cy_en_crypto_aes_key_length_t keyLength,
                                            uint32_t srcSize,
                                            void *ivPtr,
                                            void *dstPtr,
                                            void const *srcPtr);

/*******************************************************************************
* Function Name: Cy_Crypto_Aes_Cfb_Run
****************************************************************************//**
*
* This function performs AES operation on a plain text with Cipher Feedback mode (CFB).
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoAesContext
* The pointer to the \ref cy_stc_crypto_context_aes_t structure that stores all internal variables
* the Crypto driver requires.
*
* \param dirMode
* Can be CRYPTO_ENCRYPT or CRYPTO_DECRYPT.
*
* \param keyPtr
* The pointer to the encryption/decryption key.
*
* \param keyLength
* One of these: CRYPTO_AES_128, CRYPTO_AES_192, or CRYPTO_AES_256.
*
* \param srcSize
* The size of the source plain text.
*
* \param ivPtr
* The pointer to the initial vector.
*
* \param dstPtr
* The pointer to a destination cipher text.
*
* \param srcPtr
* The pointer to a source plain text.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Aes_Cfb_Run(cy_stc_crypto_context_t *cryptoContext,
                                            cy_stc_crypto_context_aes_t *cryptoAesContext,
                                            cy_en_crypto_dir_mode_t dirMode,
                                            void const *keyPtr,
                                            cy_en_crypto_aes_key_length_t keyLength,
                                            uint32_t srcSize,
                                            void *ivPtr,
                                            void *dstPtr,
                                            void const *srcPtr);

/*******************************************************************************
* Function Name: Cy_Crypto_Aes_Ctr_Run
****************************************************************************//**
*
* This function performs AES operation on a plain text with Cipher Block Counter mode (CTR).
* NOTE: preparation of the unique nonceCounter for each block is
* the user's responsibility.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoAesContext
* The pointer to the \ref cy_stc_crypto_context_aes_t structure that stores all internal variables
* the Crypto driver requires.
*
* \param dirMode
* Can be CRYPTO_ENCRYPT or CRYPTO_DECRYPT.
*
* \param keyPtr
* The pointer to the encryption/decryption key.
*
* \param keyLength
* One of these: CRYPTO_AES_128, CRYPTO_AES_192, or CRYPTO_AES_256.
*
* \param srcSize
* The size of a source plain text.
*
* \param srcOffset
* The size of an offset within the current block stream for resuming within the current cipher stream.
*
* \param nonceCounter
* The 128-bit nonce and counter.
*
* \param dstPtr
* The pointer to a destination cipher text.
*
* \param srcPtr
* The pointer to a source plain text.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Aes_Ctr_Run(cy_stc_crypto_context_t *cryptoContext,
                                            cy_stc_crypto_context_aes_t *cryptoAesContext,
                                            cy_en_crypto_dir_mode_t dirMode,
                                            void const *keyPtr,
                                            cy_en_crypto_aes_key_length_t keyLength,
                                            uint32_t srcSize,
                                            uint32_t *srcOffset,
                                            void *nonceCounter,
                                            void *dstPtr,
                                            void const *srcPtr);

/*******************************************************************************
* Function Name: Cy_Crypto_Aes_Cmac_Run
****************************************************************************//**
*
* This function performs the cipher-block chaining-message authentication-code.
* It is independent of the previous Crypto state.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoAesContext
* The pointer to the \ref cy_stc_crypto_context_aes_t structure that stores all internal variables
* the Crypto driver requires.
*
* \param srcPtr
* The pointer to a source plain text.
*
* \param srcSize
* The size of a source plain text.
*
* \param keyPtr
* The pointer to the encryption key.
*
* \param keyLength
* \ref cy_en_crypto_aes_key_length_t
*
* \param cmacPtr
* The pointer to the calculated CMAC.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Aes_Cmac_Run(cy_stc_crypto_context_t *cryptoContext,
                                             cy_stc_crypto_context_aes_t *cryptoAesContext,
                                             void const *srcPtr,
                                             uint32_t srcSize,
                                             void const *keyPtr,
                                             cy_en_crypto_aes_key_length_t keyLength,
                                             void *cmacPtr);
#endif /* #if (CPUSS_CRYPTO_AES == 1)*/

#if (CPUSS_CRYPTO_SHA == 1)
/*******************************************************************************
* Function Name: Cy_Crypto_Sha_Run
****************************************************************************//**
*
* This function performs the SHA Hash function.
* It is independent of the previous Crypto state.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoShaContext
* the pointer to the \ref cy_stc_crypto_context_sha_t structure that stores all internal variables
* for Crypto driver.
*
* \param mode 
* The mode of the SHA family to be used. Refer \ref cy_en_crypto_sha_mode_t.
*
* \param messagePtr
* The pointer to a message whose hash value is being computed.
*
* \param messageSize
* The size of a message.
*
* \param digestPtr
* The pointer to the hash digest.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Sha_Run(cy_stc_crypto_context_t *cryptoContext,
                                        cy_stc_crypto_context_sha_t *cryptoShaContext,
                                        void const *messagePtr,
                                        uint32_t  messageSize,
                                        void *digestPtr,
                                        cy_en_crypto_sha_mode_t mode);
#endif /* #if (CPUSS_CRYPTO_SHA == 1) */

#if (CPUSS_CRYPTO_SHA == 1)
/*******************************************************************************
* Function Name: Cy_Crypto_Hmac_Run
****************************************************************************//**
*
* This function performs HMAC calculation.
* It is independent of the previous Crypto state.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoShaContext
* the pointer to the \ref cy_stc_crypto_context_sha_t structure that stores all internal variables
* for the Crypto driver.
*
* \param hmacPtr
* The pointer to the calculated HMAC.
*
* \param messagePtr
* The pointer to a message whose hash value is being computed.
*
* \param messageSize
* The size of a message.
*
* \param keyPtr
* The pointer to the key.
*
* \param keyLength
* The length of the key.
*
* \param mode
* The mode of the SHA family to be used for HMAC calculation. Refer \ref cy_en_crypto_sha_mode_t.
* Note that CY_CRYPTO_MODE_SHAKE128 and CY_CRYPTO_MODE_SHAKE256 are not supported for the HAMC
* calculation.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Hmac_Run(cy_stc_crypto_context_t *cryptoContext,
                                         cy_stc_crypto_context_sha_t *cryptoShaContext,
                                         void *hmacPtr,
                                         void const *messagePtr,
                                         uint32_t  messageSize,
                                         void const *keyPtr,
                                         uint32_t keyLength,
                                         cy_en_crypto_sha_mode_t mode);
#endif /* #if (CPUSS_CRYPTO_SHA == 1) */

#if (CPUSS_CRYPTO_CRC == 1)
/*******************************************************************************
* Function Name: Cy_Crypto_Crc_Init
****************************************************************************//**
*
* This function performs CRC initialization.
* One peculiar of the CRC hardware block is that for some polynomials
* calculated, CRC is MSB aligned and others are LSB aligned.
* Below is the table with known polynomials and their
* calculated CRCs from the string "123456789".
*
*  <table>
*  <caption id="crc_modes_table">CRC modes and parameters</caption>
*   <tr><th>Name</th><th>Width</th><th>Poly</th><th>Init</th><th>Data Rev</th><th>Data XOR</th><th>Rem Rev</th><th>Rem XOR</th><th>Expected CRC</th><th>Output of the CRC block</th>
*   </tr>
*   <tr>
*     <td>CRC-3 / ROHC</td><td>3</td><td>0x3</td><td>0x7</td><td>1</td><td>0</td><td>1</td><td>0x0</td><td>0x6</td><td>0x00000006</td>
*   </tr>
*   <tr>
*     <td>CRC-4 / ITU</td><td>4</td><td>0x3</td><td>0x0</td><td>1</td><td>0</td><td>1</td><td>0x0</td><td>0x7</td><td>0x00000007</td>
*   </tr>
*   <tr>
*     <td>CRC-5 / EPC</td><td>5</td><td>0x9</td><td>0x9</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0x0</td><td>0x00000000</td>
*   </tr>
*   <tr>
*     <td>CRC-5 / ITU</td><td>5</td><td>0x15</td><td>0x0</td><td>1</td><td>0</td><td>1</td><td>0x0</td><td>0x7</td><td>0x00000007</td>
*   </tr>
*   <tr>
*     <td>CRC-5 / USB</td><td>5</td><td>0x5</td><td>0x1F</td><td>1</td><td>0</td><td>1</td><td>0x1F</td><td>0x19</td><td>0x00000019</td>
*   </tr>
*   <tr>
*     <td>CRC-6 / CDMA2000-A</td><td>6</td><td>0x27</td><td>0x3F</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0xD</td><td>0xD0000000</td>
*   </tr>
*   <tr>
*     <td>CRC-6 / CDMA2000-B</td><td>6</td><td>0x7</td><td>0x3F</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0x3B</td><td>0x3B000000</td>
*   </tr>
*   <tr>
*     <td>CRC-6 / DARC</td><td>6</td><td>0x19</td><td>0x0</td><td>1</td><td>0</td><td>1</td><td>0x0</td><td>0x26</td><td>0x00000026</td>
*   </tr>
*   <tr>
*     <td>CRC-6 / ITU</td><td>6</td><td>0x3</td><td>0x0</td><td>1</td><td>0</td><td>1</td><td>0x0</td><td>0x6</td><td>0x00000006</td>
*   </tr>
*   <tr>
*     <td>CRC-7</td><td>7</td><td>0x9</td><td>0x0</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0x75</td><td>0x75000000</td>
*   </tr>
*   <tr>
*     <td>CRC-7 / ROHC</td><td>7</td><td>0x4F</td><td>0x7F</td><td>1</td><td>0</td><td>1</td><td>0x0</td><td>0x53</td><td>0x00000053</td>
*   </tr>
*   <tr>
*     <td>CRC-8</td><td>8</td><td>0x7</td><td>0x0</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0xF4</td><td>0xF4000000</td>
*   </tr>
*   <tr>
*     <td>CRC-8 / CDMA2000</td><td>8</td><td>0x9B</td><td>0xFF</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0xDA</td><td>0xDA000000</td>
*   </tr>
*   <tr>
*     <td>CRC-8 / DARC</td><td>8</td><td>0x39</td><td>0x0</td><td>1</td><td>0</td><td>1</td><td>0x0</td><td>0x15</td><td>0x00000015</td>
*   </tr>
*   <tr>
*     <td>CRC-8 / DVB-S2</td><td>8</td><td>0xD5</td><td>0x0</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0xBC</td><td>0xBC000000</td>
*   </tr>
*   <tr>
*     <td>CRC-8 / EBU</td><td>8</td><td>0x1D</td><td>0xFF</td><td>1</td><td>0</td><td>1</td><td>0x0</td><td>0x97</td><td>0x00000097</td>
*   </tr>
*   <tr>
*     <td>CRC-8 / I-CODE</td><td>8</td><td>0x1D</td><td>0xFD</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0x7E</td><td>0x7E000000</td>
*   </tr>
*   <tr>
*     <td>CRC-8 / ITU</td><td>8</td><td>0x7</td><td>0x0</td><td>0</td><td>0</td><td>0</td><td>0x55</td><td>0xA1</td><td>0xA1000000</td>
*   </tr>
*   <tr>
*     <td>CRC-8 / MAXIM</td><td>8</td><td>0x31</td><td>0x0</td><td>1</td><td>0</td><td>1</td><td>0x0</td><td>0xA1</td><td>0x000000A1</td>
*   </tr>
*   <tr>
*     <td>CRC-8 / ROHC</td><td>8</td><td>0x7</td><td>0xFF</td><td>1</td><td>0</td><td>1</td><td>0x0</td><td>0xD0</td><td>0x000000D0</td>
*   </tr>
*   <tr>
*     <td>CRC-8 / WCDMA</td><td>8</td><td>0x9B</td><td>0x0</td><td>1</td><td>0</td><td>1</td><td>0x0</td><td>0x25</td><td>0x00000025</td>
*   </tr>
*   <tr>
*     <td>CRC-10</td><td>10</td><td>0x233</td><td>0x0</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0x199</td><td>0x19900000</td>
*   </tr>
*   <tr>
*     <td>CRC-10 / CDMA2000</td><td>10</td><td>0x3D9</td><td>0x3FF</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0x233</td><td>0x23300000</td>
*   </tr>
*   <tr>
*     <td>CRC-11</td><td>11</td><td>0x385</td><td>0x1A</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0x5A3</td><td>0x5A300000</td>
*   </tr>
*   <tr>
*     <td>CRC-12 / 3GPP</td><td>12</td><td>0x80F</td><td>0x0</td><td>0</td><td>0</td><td>1</td><td>0x0</td><td>0xDAF</td><td>0x00000DAF</td>
*   </tr>
*   <tr>
*     <td>CRC-12 / CDMA2000</td><td>12</td><td>0xF13</td><td>0xFFF</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0xD4D</td><td>0xD4D00000</td>
*   </tr>
*   <tr>
*     <td>CRC-12 / DECT</td><td>12</td><td>0x80F</td><td>0x0</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0xF5B</td><td>0xF5B00000</td>
*   </tr>
*   <tr>
*     <td>CRC-13 / BBC</td><td>13</td><td>0x1CF5</td><td>0x0</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0x4FA</td><td>0x4FA00000</td>
*   </tr>
*   <tr>
*     <td>CRC-14 / DARC</td><td>14</td><td>0x805</td><td>0x0</td><td>1</td><td>0</td><td>1</td><td>0x0</td><td>0x82D</td><td>0x0000082D</td>
*   </tr>
*   <tr>
*     <td>CRC-15</td><td>15</td><td>0x4599</td><td>0x0</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0x59E</td><td>0x59E00000</td>
*   </tr>
*   <tr>
*     <td>CRC-15 / MPT1327</td><td>15</td><td>0x6815</td><td>0x0</td><td>0</td><td>0</td><td>0</td><td>0x1</td><td>0x2566</td><td>0x25660000</td>
*   </tr>
*   <tr>
*     <td>CRC-24</td><td>24</td><td>0x0864CFB</td><td>0x00B704CE</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0x21CF02</td><td>0x21CF0200</td>
*   </tr>
*   <tr>
*     <td>CRC-24 / FLEXRAY-A</td><td>24</td><td>0x05D6DCB</td><td>0x00FEDCBA</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0x7979BD</td><td>0x7979BD00</td>
*   </tr>
*   <tr>
*     <td>CRC-24 / FLEXRAY-B</td><td>24</td><td>0x05D6DCB</td><td>0x00ABCDEF</td><td>0</td><td>0</td><td>0</td><td>0x0</td><td>0x1F23B8</td><td>0x1F23B800</td>
*   </tr>
*   <tr>
*     <td>CRC-31 / PHILIPS</td><td>31</td><td>0x4C11DB7</td><td>0x7FFFFFFF</td><td>0</td><td>0</td><td>0</td><td>0x7FFFFFFF</td><td>0xCE9E46C</td><td>0xCE9E46C0</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / ARC</td><td>16</td><td>0x8005</td><td>0x0000</td><td>1</td><td>0</td><td>1</td><td>0x0000</td><td>0xBB3D</td><td>0x0000BB3D</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / AUG-CCITT</td><td>16</td><td>0x1021</td><td>0x1D0F</td><td>0</td><td>0</td><td>0</td><td>0x0000</td><td>0xE5CC</td><td>0xE5CC0000</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / BUYPASS</td><td>16</td><td>0x8005</td><td>0x0000</td><td>0</td><td>0</td><td>0</td><td>0x0000</td><td>0xFEE8</td><td>0xFEE80000</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / CCITT-0</td><td>16</td><td>0x1021</td><td>0xFFFF</td><td>0</td><td>0</td><td>0</td><td>0x0000</td><td>0x29B1</td><td>0x29B10000</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / CDMA2000</td><td>16</td><td>0xC867</td><td>0xFFFF</td><td>0</td><td>0</td><td>0</td><td>0x0000</td><td>0x4C06</td><td>0x4C060000</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / DDS-110</td><td>16</td><td>0x8005</td><td>0x800D</td><td>0</td><td>0</td><td>0</td><td>0x0000</td><td>0x9ECF</td><td>0x9ECF0000</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / DECT-R</td><td>16</td><td>0x0589</td><td>0x0000</td><td>0</td><td>0</td><td>0</td><td>0x0001</td><td>0x007E</td><td>0x007E0000</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / DECT-X</td><td>16</td><td>0x0589</td><td>0x0000</td><td>0</td><td>0</td><td>0</td><td>0x0000</td><td>0x007F</td><td>0x007F0000</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / DNP</td><td>16</td><td>0x3D65</td><td>0x0000</td><td>1</td><td>0</td><td>1</td><td>0xFFFF</td><td>0xEA82</td><td>0x0000EA82</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / EN-13757</td><td>16</td><td>0x3D65</td><td>0x0000</td><td>0</td><td>0</td><td>0</td><td>0xFFFF</td><td>0xC2B7</td><td>0xC2B70000</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / GENIBUS</td><td>16</td><td>0x1021</td><td>0xFFFF</td><td>0</td><td>0</td><td>0</td><td>0xFFFF</td><td>0xD64E</td><td>0xD64E0000</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / MAXIM</td><td>16</td><td>0x8005</td><td>0x0000</td><td>1</td><td>0</td><td>1</td><td>0xFFFF</td><td>0x44C2</td><td>0x000044C2</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / MCRF4XX</td><td>16</td><td>0x1021</td><td>0xFFFF</td><td>1</td><td>0</td><td>1</td><td>0x0000</td><td>0x6F91</td><td>0x00006F91</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / RIELLO</td><td>16</td><td>0x1021</td><td>0xB2AA</td><td>1</td><td>0</td><td>1</td><td>0x0000</td><td>0x63D0</td><td>0x000063D0</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / T10-DIF</td><td>16</td><td>0x8BB7</td><td>0x0000</td><td>0</td><td>0</td><td>0</td><td>0x0000</td><td>0xD0DB</td><td>0xD0DB0000</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / TELEDISK</td><td>16</td><td>0xA097</td><td>0x0000</td><td>0</td><td>0</td><td>0</td><td>0x0000</td><td>0x0FB3</td><td>0x0FB30000</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / TMS37157</td><td>16</td><td>0x1021</td><td>0x89EC</td><td>1</td><td>0</td><td>1</td><td>0x0000</td><td>0x26B1</td><td>0x000026B1</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / USB</td><td>16</td><td>0x8005</td><td>0xFFFF</td><td>1</td><td>0</td><td>1</td><td>0xFFFF</td><td>0xB4C8</td><td>0x0000B4C8</td>
*   </tr>
*   <tr>
*     <td>CRC-A</td><td>16</td><td>0x1021</td><td>0xC6C6</td><td>1</td><td>0</td><td>1</td><td>0x0000</td><td>0xBF05</td><td>0x0000BF05</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / KERMIT</td><td>16</td><td>0x1021</td><td>0x0000</td><td>1</td><td>0</td><td>1</td><td>0x0000</td><td>0x2189</td><td>0x00002189</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / MODBUS</td><td>16</td><td>0x8005</td><td>0xFFFF</td><td>1</td><td>0</td><td>1</td><td>0x0000</td><td>0x4B37</td><td>0x00004B37</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / X-25</td><td>16</td><td>0x1021</td><td>0xFFFF</td><td>1</td><td>0</td><td>1</td><td>0xFFFF</td><td>0x906E</td><td>0x0000906E</td>
*   </tr>
*   <tr>
*     <td>CRC-16 / XMODEM</td><td>16</td><td>0x1021</td><td>0x0000</td><td>0</td><td>0</td><td>0</td><td>0x0000</td><td>0x31C3</td><td>0x31C30000</td>
*   </tr>
*   <tr>
*     <td>CRC-32</td><td>32</td><td>0x04C11DB7</td><td>0xFFFFFFFF</td><td>1</td><td>0</td><td>1</td><td>0xFFFFFFFF</td><td>0xCBF43926</td><td>0xCBF43926</td>
*   </tr>
*   <tr>
*     <td>CRC-32 / BZIP2</td><td>32</td><td>0x04C11DB7</td><td>0xFFFFFFFF</td><td>0</td><td>0</td><td>0</td><td>0xFFFFFFFF</td><td>0xFC891918</td><td>0xFC891918</td>
*   </tr>
*   <tr>
*     <td>CRC-32C</td><td>32</td><td>0x1EDC6F41</td><td>0xFFFFFFFF</td><td>1</td><td>0</td><td>1</td><td>0xFFFFFFFF</td><td>0xE3069283</td><td>0xE3069283</td>
*   </tr>
*   <tr>
*     <td>CRC-32D</td><td>32</td><td>0xA833982B</td><td>0xFFFFFFFF</td><td>1</td><td>0</td><td>1</td><td>0xFFFFFFFF</td><td>0x87315576</td><td>0x87315576</td>
*   </tr>
*   <tr>
*     <td>CRC-32 / MPEG-2</td><td>32</td><td>0x04C11DB7</td><td>0xFFFFFFFF</td><td>0</td><td>0</td><td>0</td><td>0x00000000</td><td>0x0376E6E7</td><td>0x0376E6E7</td>
*   </tr>
*   <tr>
*     <td>CRC-32 / POSIX</td><td>32</td><td>0x04C11DB7</td><td>0x00000000</td><td>0</td><td>0</td><td>0</td><td>0xFFFFFFFF</td><td>0x765E7680</td><td>0x765E7680</td>
*   </tr>
*   <tr>
*     <td>CRC-32Q</td><td>32</td><td>0x814141AB</td><td>0x00000000</td><td>0</td><td>0</td><td>0</td><td>0x00000000</td><td>0x3010BF7F</td><td>0x3010BF7F</td>
*   </tr>
*   <tr>
*     <td>CRC-32 / JAMCRC</td><td>32</td><td>0x04C11DB7</td><td>0xFFFFFFFF</td><td>1</td><td>0</td><td>1</td><td>0x00000000</td><td>0x340BC6D9</td><td>0x340BC6D9</td>
*   </tr>
*   <tr>
*     <td>CRC-32 / XFER</td><td>32</td><td>0x000000AF</td><td>0x00000000</td><td>0</td><td>0</td><td>0</td><td>0x00000000</td><td>0xBD0BE338</td><td>0xBD0BE338</td>
*   </tr>
* </table>
*
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoCrcContext
* The pointer to the \ref cy_stc_crypto_context_crc_t structure that stores
* the Crypto driver context.
*
* \param polynomial
* The polynomial (specified using 32 bits) used in the computing CRC.
*
* \param dataReverse
* The order in which data bytes are processed. 0 - MSB first; 1- LSB first.
*
* \param dataXor
* The byte mask for XORing data
*
* \param remReverse
* A reminder reverse: 0 means the remainder is not reversed. 1 means - reversed.
*
* \param remXor
* Specifies a mask with which the LFSR32 register is XORed to produce a remainder.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Crc_Init(cy_stc_crypto_context_t *cryptoContext,
                                         cy_stc_crypto_context_crc_t *cryptoCrcContext,
                                         uint32_t polynomial,
                                         uint8_t  dataReverse,
                                         uint8_t  dataXor,
                                         uint8_t  remReverse,
                                         uint32_t remXor);

/*******************************************************************************
* Function Name: Cy_Crypto_Crc_Run
****************************************************************************//**
*
* This function performs CRC calculation on a message.
* It depends on Cy_Crypto_Crc_Init(),
* which should be called before.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoCrcContext
* The pointer to the \ref cy_stc_crypto_context_crc_t structure that stores
* the Crypto driver context.
*
* \param dataPtr
* The pointer to the message whose CRC is being computed.
*
* \param dataSize
* The size of a message in bytes.
*
* \param crcPtr
* The pointer to a computed CRC value.
*
* \param lfsrInitState
* The initial state of the LFSR.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Crc_Run(cy_stc_crypto_context_t *cryptoContext,
                                        cy_stc_crypto_context_crc_t *cryptoCrcContext,
                                        void const *dataPtr,
                                        uint16_t  dataSize,
                                        uint32_t *crcPtr,
                                        uint32_t  lfsrInitState);
#endif /* #if (CPUSS_CRYPTO_CRC == 1) */

#if (CPUSS_CRYPTO_TR == 1)
/*******************************************************************************
* Function Name: Cy_Crypto_Trng_Generate
****************************************************************************//**
*
* This function generates a True Random Number.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoTrngContext
* The pointer to the \ref cy_stc_crypto_context_trng_t structure that stores
* the Crypto driver context.
*
* \param GAROPol;
* The polynomial for the programmable Galois ring oscillator.
*
* \param FIROPol;
* The polynomial for the programmable Fibonacci ring oscillator.
*
* \param rndNumPtr
* The pointer to a generated true random number.
*
* \param bitSize
* The bit size of the result, valid range is [0, 32].
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Trng_Generate(cy_stc_crypto_context_t *cryptoContext,
                                              cy_stc_crypto_context_trng_t *cryptoTrngContext,
                                              uint32_t  GAROPol,
                                              uint32_t  FIROPol,
                                              uint32_t *rndNumPtr,
                                              uint8_t   bitSize);
#endif /* #if (CPUSS_CRYPTO_TR == 1) */

#if (CPUSS_CRYPTO_DES == 1)
/*******************************************************************************
* Function Name: Cy_Crypto_Des_Run
****************************************************************************//**
*
* This function performs DES operation on a Single Block.
* Ciphertext (dstBlockPtr) may overlap with plaintext (srcBlockPtr)
* This function is independent from the previous Crypto state.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoDesContext
* The pointer to the cy_stc_crypto_context_des_t structure that stores
* the Crypto driver context.
*
* \param dirMode
* Can be CRYPTO_ENCRYPT or CRYPTO_DECRYPT
*
* \param keyPtr
* The pointer to the encryption/decryption key.
*
* \param srcBlockPtr
* The pointer to a source block.
*
* \param dstBlockPtr
* The pointer to a destination cipher block.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Des_Run(cy_stc_crypto_context_t *cryptoContext,
                                        cy_stc_crypto_context_des_t *cryptoDesContext,
                                        cy_en_crypto_dir_mode_t dirMode,
                                        void const *keyPtr,
                                        void *dstBlockPtr,
                                        void const *srcBlockPtr);
/*******************************************************************************
* Function Name: Cy_Crypto_Tdes_Run
****************************************************************************//**
*
* This function performs TDES operation on a Single Block.
* Ciphertext (dstBlockPtr) may overlap with plaintext (srcBlockPtr).
* This function is independent from the previous Crypto state.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoDesContext
* The pointer to the cy_stc_crypto_context_des_t structure that stores
* the Crypto driver context.
*
* \param dirMode
* Can be CRYPTO_ENCRYPT or CRYPTO_DECRYPT.
*
* \param keyPtr
* The pointer to the encryption/decryption key.
*
* \param srcBlockPtr
* The pointer to a source block.
*
* \param dstBlockPtr
* The pointer to a destination cipher block.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Tdes_Run(cy_stc_crypto_context_t *cryptoContext,
                                         cy_stc_crypto_context_des_t *cryptoDesContext,
                                         cy_en_crypto_dir_mode_t dirMode,
                                         void const *keyPtr,
                                         void *dstBlockPtr,
                                         void const *srcBlockPtr);
#endif /* #if (CPUSS_CRYPTO_DES == 1) */

#if (CPUSS_CRYPTO_VU == 1)
/*******************************************************************************
* Function Name: Cy_Crypto_Rsa_Proc
****************************************************************************//**
*
* This function calculates (m^e mod modulo) where m is Message (Signature), e - public exponent
* using a public key in the next representation, it contains:
* modulo,
* public exponent,
* coefficient for Barrett reduction,
* binary inverse of the modulo,
* result of (2^moduloLength mod modulo).
*
* Not all fields in a key must be given. Modulo and public exponent is mandatory;
* Barrett coefficient, inverse modulo, and r-bar are optional.
* If they don't exist, their according pointers should be NULL. These coefficients
* could be calculated by \ref Cy_Crypto_Rsa_CalcCoefs.
* Their presence accelerates performance by five times.
* Approximate performance for 1024-bit modulo is 41.6 ms; for 2048-bit modulo is 142 ms
* when using a 25 MHz clock for Crypto HW. These numbers just for reference.
* They depend on many factors (compiler, optimization level, etc.).
*
* Returns the processed value and a success value.
*
* \param cryptoContext
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cryptoRsaContext
* The pointer to the \ref cy_stc_crypto_context_rsa_t structure that stores
* the RSA context.
*
* \param pubKeyPtr
* The pointer to the \ref cy_stc_crypto_rsa_pub_key_t structure that stores
* public key.
*
* \param messagePtr
* The pointer to the message to be processed.
*
* \param messageLength
* The length of the message to be processed.
*
* \param processedMessagePtr
* The pointer to processed message.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Rsa_Proc(cy_stc_crypto_context_t *cryptoContext,
                                         cy_stc_crypto_context_rsa_t *cryptoRsaContext,
                                         cy_stc_crypto_rsa_pub_key_t const *pubKeyPtr,
                                         void const *messageePtr,
                                         uint32_t messageLength,
                                         void *processedMessagePtr);

/*******************************************************************************
* Function Name: Cy_Crypto_Rsa_CalcCoefs
****************************************************************************//**
*
* This function calculates constant coefficients (which is dependent only on modulo
* and independent on message). With this pre-calculated coefficients calculations
* speed-up by five times.
*
* These coefficients are:
*                         coefficient for Barrett reduction,
*                         binary inverse of the modulo,
*                         result of (2^moduloLength mod modulo)
*
* Calculated coefficients will be placed by addresses provided in the
* pubKey structure for according coefficients.
* Function overwrites previous values.
* Approximate performance for 1024-bit modulo is 33.2 ms; for 2048-bit modulo is 113 ms
* when using a 25 MHz clock for Crypto HW. These numbers are just for reference.
* They depend on many factors (compiler, optimization level, etc.).
*
* \param pubKey
* The pointer to the \ref cy_stc_crypto_rsa_pub_key_t structure that stores a
* public key.
*
* \param context
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cfContext
* The pointer to the \ref cy_stc_crypto_context_rsa_t structure that stores
* the RSA context.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Rsa_CalcCoefs(cy_stc_crypto_context_t *cryptoContext,
                                              cy_stc_crypto_context_rsa_t *cryptoRsaContext,
                                              cy_stc_crypto_rsa_pub_key_t const *pubKeyPtr);
#endif /* #if (CPUSS_CRYPTO_VU == 1) */

/*******************************************************************************
* Function Name: Cy_Crypto_Rsa_Verify
****************************************************************************//**
*
* This function does an RSA verification with checks for content, paddings, and signature format.
* The SHA digest of the message and decrypted message should be calculated first.
* Supports only PKCS1-v1_5 format. Inside of this format supported padding
* using only SHA. Cases with MD2 and MD5 are not supported.
*
* PKCS1-v1_5 described here, page 31:
* http://www.emc.com/collateral/white-papers/h11300-pkcs-1v2-2-rsa-cryptography-standard-wp.pdf
*
* Returns the verification result \ref cy_en_crypto_rsa_ver_result_t.
*
* \param verResult
* The pointer to the verification result \ref cy_en_crypto_rsa_ver_result_t.
*
* \param digestType
* SHA mode used for hash calculation \ref cy_en_crypto_sha_mode_t.
*
* \param digest
* The pointer to the hash of the message whose signature is to be verified.
*
* \param decryptedSignature
* The pointer to the decrypted signature to be verified.
*
* \param decryptedSignatureLength
* The length of the decrypted signature to be verified (in bytes)
*
* \param context
* The pointer to the \ref cy_stc_crypto_context_t structure that stores
* the Crypto driver common context.
*
* \param cfContext
* The pointer to the \ref cy_stc_crypto_context_rsa_ver_t structure that stores
* the RSA context.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Rsa_Verify(cy_stc_crypto_context_t *cryptoContext,
                                           cy_stc_crypto_context_rsa_ver_t *cryptoRsaVerContext,
                                           cy_en_crypto_rsa_ver_result_t *verResult,
                                           cy_en_crypto_sha_mode_t digestType,
                                           void const *digestPtr,
                                           void const *decryptedSignaturePtr,
                                           uint32_t decryptedSignatureLength);

/*******************************************************************************
* Function Name: Cy_Crypto_Rsa_InvertEndianness
****************************************************************************//**
*
* This function reverts byte-array memory block, like:<br>
* inArr[0] <---> inArr[n]<br>
* inArr[1] <---> inArr[n-1]<br>
* inArr[2] <---> inArr[n-2]<br>
* ........................<br>
* inArr[n/2] <---> inArr[n/2-1]<br>
*
* Odd or even byteSize are acceptable.
*
* \param inArrPtr
* The pointer to the memory whose endianness is to be inverted.
*
* \param byteSize
* The length of the memory array whose endianness is to be inverted (in bytes)
*
*******************************************************************************/
void Cy_Crypto_Rsa_InvertEndianness(void *inArrPtr, uint32_t byteSize);

#if (CPUSS_CRYPTO_CHACHA == 1)
/*******************************************************************************
* Function Name: Cy_Crypto_Chacha_Run
****************************************************************************//**
*
* This function performs the CHACHA function.
* It is independent of the previous Crypto state.
*
* \param cryptoContext
* the pointer to the \ref cy_stc_crypto_context_t structure that stores all internal variables
* for Crypto driver.
*
* \param cryptoChachaContext
* The pointer to the \ref cy_stc_crypto_context_chacha_t structure that stores
* the CHACHA context.
*
* \param srcBlockPtr
* The pointer to the input message blocks.
*
* \param messageSize
* The size of a message in Bytes.
*
* \param dstBlockPtr
* The pointer to the output message blocks.
*
* \param noncePtr
* The pointer to the nonce value.
*
* \param counter
* The value of the initial counter.
*
* \param keyPtr
* The pointer to the key (256bits length)
*
* \param roundNum
* The value of the CHACHA round. Refer \ref cy_en_crypto_chacha_round_t.
*
* \return
* A Crypto status \ref cy_en_crypto_status_t.
*
*******************************************************************************/
cy_en_crypto_status_t Cy_Crypto_Chacha_Run(cy_stc_crypto_context_t *cryptoContext,
                                           cy_stc_crypto_context_chacha_t *cryptoChachaContext,
                                           void const *srcBlockPtr,
                                           uint32_t messageSize,
                                           void *dstBlockPtr,
                                           void const *noncePtr,
                                           uint32_t counter,
                                           void const *keyPtr,
                                           cy_en_crypto_chacha_round_t  roundNum);
#endif /* #if (CPUSS_CRYPTO_CHACHA == 1) */

/** \} group_crypto_cli_functions */


#if defined(__cplusplus)
}
#endif

#endif /* #if (CPUSS_CRYPTO_PRESENT == 1) */

#endif /* (CY_CRYPTO_H) */

/** \} group_crypto */

/* [] END OF FILE */
