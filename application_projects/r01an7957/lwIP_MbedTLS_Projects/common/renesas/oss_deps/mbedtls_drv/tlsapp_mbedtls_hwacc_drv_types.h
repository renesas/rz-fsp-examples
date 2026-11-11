/***********************************************************************************************************************
 * Copyright [2024] Renesas Electronics Corporation and/or its affiliates.  All Rights Reserved.
 *
 * This software and documentation are supplied by Renesas Electronics Corporation and/or its affiliates and may only
 * be used with products of Renesas Electronics Corp. and its affiliates ("Renesas").  No other uses are authorized.
 * Renesas products are sold pursuant to Renesas terms and conditions of sale.  Purchasers are solely responsible for
 * the selection and use of Renesas products and Renesas assumes no liability.  No license, express or implied, to any
 * intellectual property right is granted by Renesas.  This software is protected under all applicable laws, including
 * copyright laws. Renesas reserves the right to change or discontinue this software and/or this documentation.
 * THE SOFTWARE AND DOCUMENTATION IS DELIVERED TO YOU "AS IS," AND RENESAS MAKES NO REPRESENTATIONS OR WARRANTIES, AND
 * TO THE FULLEST EXTENT PERMISSIBLE UNDER APPLICABLE LAW, DISCLAIMS ALL WARRANTIES, WHETHER EXPLICITLY OR IMPLICITLY,
 * INCLUDING WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT, WITH RESPECT TO THE
 * SOFTWARE OR DOCUMENTATION.  RENESAS SHALL HAVE NO LIABILITY ARISING OUT OF ANY SECURITY VULNERABILITY OR BREACH.
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT WILL RENESAS BE LIABLE TO YOU IN CONNECTION WITH THE SOFTWARE OR
 * DOCUMENTATION (OR ANY PERSON OR ENTITY CLAIMING RIGHTS DERIVED FROM YOU) FOR ANY LOSS, DAMAGES, OR CLAIMS WHATSOEVER,
 * INCLUDING, WITHOUT LIMITATION, ANY DIRECT, CONSEQUENTIAL, SPECIAL, INDIRECT, PUNITIVE, OR INCIDENTAL DAMAGES; ANY
 * LOST PROFITS, OTHER ECONOMIC DAMAGE, PROPERTY DAMAGE, OR PERSONAL INJURY; AND EVEN IF RENESAS HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH LOSS, DAMAGES, CLAIMS OR COSTS.
 **********************************************************************************************************************/
#ifndef TLSAPP_MBEDTLS_HWACC_DRV_TYPES_H
#define TLSAPP_MBEDTLS_HWACC_DRV_TYPES_H

#include "mbedtls/build_info.h"
#if defined(PSA_CRYPTO_ACCELERATOR_DRIVER_PRESENT) && defined(PSA_CRYPTO_DRIVER_TEST)

#define PSA_CRYPTO_TEST_DRIVER_LOCATION 0x7fffff

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
#include "r_rsip.h"

typedef enum e_tlsapp_mbedtls_rsip_key_type
{
    TLSAPP_MBEDTLS_RSIPWRAPKEY_NONE = 0,
    TLSAPP_MBEDTLS_RSIPWRAPKEY_HMAC_KDF,
    TLSAPP_MBEDTLS_RSIPWRAPKEY_HMAC,
    TLSAPP_MBEDTLS_RSIPWRAPKEY_AES128,
    TLSAPP_MBEDTLS_RSIPWRAPKEY_AES256,
    TLSAPP_MBEDTLS_RSIPWRAPKEY_CLI_ECDHEKEYPAIR,
    TLSAPP_MBEDTLS_RSIPWRAPKEY_SRV_ECDHEKEYPAIR,
    TLSAPP_MBEDTLS_RSIPWRAPKEY_RSA_CERTVERIFY,
    TLSAPP_MBEDTLS_RSIPWRAPKEY_RSA_MSGVERIFY,
    TLSAPP_MBEDTLS_RSIPWRAPKEY_ECC_CERTVERIFY,
    TLSAPP_MBEDTLS_RSIPWRAPKEY_ECC_MSGVERIFY,
    TLSAPP_MBEDTLS_RSIPWRAPKEY_RSA,
    TLSAPP_MBEDTLS_RSIPWRAPKEY_RSA_SIGN,
    TLSAPP_MBEDTLS_RSIPWRAPKEY_ECC_SIGN,
} tlsapp_mbedtls_rsip_key_type_t;

typedef enum e_tlsapp_mbedtls_rsip_impcrtkey_type
{
    TLSAPP_MBEDTLS_RSIP_IMPCRTKEY_ECP256,
    TLSAPP_MBEDTLS_RSIP_IMPCRTKEY_RSA2K,
} tlsapp_mbedtls_rsip_impcrtkey_type_t;

typedef enum e_tlsapp_mbedtls_rsip_hmac_type
{
    TLSAPP_MBEDTLS_RSIP_HMACTYPE_SIGN = 0,
    TLSAPP_MBEDTLS_RSIP_HMACTYPE_VERIFY,
    TLSAPP_MBEDTLS_RSIP_HMACTYPE_KDF
} tlsapp_mbedtls_rsip_hmac_type_t;

typedef enum e_tlsapp_mbedtls_rsip_cipher_type
{
    TLSAPP_MBEDTLS_RSIP_CIPHERTYPE_NONE = 0,
    TLSAPP_MBEDTLS_RSIP_CIPHERTYPE_CBCENC,
    TLSAPP_MBEDTLS_RSIP_CIPHERTYPE_CBCDEC
} tlsapp_mbedtls_rsip_cipher_type_t;

typedef enum e_tlsapp_mbedtls_rsip_internal_data_size
{
    TLSAPP_MBEDTLS_RSIP_ECP_UNCOMPRESSED_PUBKEYSIZE = 65,
} tlsapp_mbedtls_rsip_internal_data_size_t;

typedef enum e_tlsapp_mbedtls_rsip_wrapped_key_size
{
    /* Wrapped ECC Key */
    TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_SECP256R1_PUB = RSIP_BYTE_SIZE_WRAPPED_KEY_ECC_SECP256R1_PUBLIC,
    TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_SECP256R1_PRI = RSIP_BYTE_SIZE_WRAPPED_KEY_ECC_SECP256R1_PRIVATE,
    /* Wrapped Shared Secret(R_RSIP_ECDH_KeyAgree/PlainKeyAgree output) */
    TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_PMSECRET = sizeof(rsip_wrapped_secret_t),
    /* Wrapped Master Secret(R_RSIP_KDF_MACConcatenate output) */
    TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_MASTERSECRET = (RSIP_BYTE_SIZE_WRAPPED_DKM_HEADER + (RSIP_BYTE_SIZE_WRAPPED_KEY_KDF_HMAC_SHA256 * 2)),
    /* Wrapped KeyBlock(R_RSIP_KDF_MACConcatenate output) */
    TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_KEYBLOCK = (RSIP_BYTE_SIZE_WRAPPED_DKM_HEADER + (RSIP_BYTE_SIZE_WRAPPED_KEY_HMAC_SHA256 * 8)),
    /* Wrapped HMAC Key(PMS)(R_RSIP_KDF_ECDHSecretKeyImport output(PMS)) */
    TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_KDFHMAC_PMS = 100,
    /* Wrapped HMAC Key(MS)(R_RSIP_KDF_MACKeyImport output(MS)) */
    TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_KDFHMAC_MS = 100,
    /* Wrapped KDF MAC(R_RSIP_KDF_HMAC_SignFinish output) */
    TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_KDFMAC = (RSIP_BYTE_SIZE_WRAPPED_DKM_HEADER + (RSIP_BYTE_SIZE_WRAPPED_KEY_HMAC_SHA256)),
    /* Wrapped HMAC Key(R_RSIP_KDF_DerivedKeyImport output) */
    TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_MAC = RSIP_BYTE_SIZE_WRAPPED_KEY_HMAC_SHA256,
    /* Wrapped AES128 Key(R_RSIP_KDF_DerivedKeyImport output) */
    TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_AESKEY_128 = RSIP_BYTE_SIZE_WRAPPED_KEY_AES_128,
    /* Wrapped AES256 Key(R_RSIP_KDF_DerivedKeyImport output) */
    TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_AESKEY_256 = RSIP_BYTE_SIZE_WRAPPED_KEY_AES_256,
    /* Wrapped IV(R_RSIP_KDF_DerivedIVWrap output) */
    TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_IVLEN = 48,
    TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_RSA2048_PUB = RSIP_BYTE_SIZE_WRAPPED_KEY_RSA_2048_PUBLIC,
    TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_RSA2048_PRI = RSIP_BYTE_SIZE_WRAPPED_KEY_RSA_2048_PRIVATE,
} tlsapp_mbedtls_rsip_wrapped_key_size_t;

typedef enum e_tlsapp_mbedtls_rsip_prf_label
{
    TLSAPP_MBEDTLS_RSIP_PRF_LABEL_CLIENT_FIN = RSIP_TLS12_PRF_LABEL_CLIENT_FINISHED,
    TLSAPP_MBEDTLS_RSIP_PRF_LABEL_SERVER_FIN = RSIP_TLS12_PRF_LABEL_SERVER_FINISHED,
} tlsapp_mbedtls_rsip_prf_label_t;

typedef enum e_tlsapp_mbedtls_storedkey_status
{
    TLSAPP_MBEDTLS_RSIP_STOREDKEY_NOTEXIST = 0,
    TLSAPP_MBEDTLS_RSIP_STOREDKEY_EXIST,
} tlsapp_mbedtls_rsip_storedkey_status_t;

typedef enum tlsapp_mbedtls_rsip_iv_type
{
    TLSAPP_MBEDTLS_RSIP_IVTYPE_AEAD = RSIP_INITIAL_VECTOR_TYPE_AES_TLS12_AEAD,
} tlsapp_mbedtls_rsip_iv_type_t;

typedef enum e_tlsapp_mbedtls_certkey_verifytype
{
    TLSAPP_MBEDTLS_CERTKEYVERIFYTYPE_NONE = 0x00,
    TLSAPP_MBEDTLS_CERTKEYVERIFYTYPE_CERTVERIFY = 0x01,
    TLSAPP_MBEDTLS_CERTKEYVERIFYTYPE_MSGVERIFY = 0x02,
} tlsapp_mbedtls_certkey_verifytype_t;

typedef enum e_tlsapp_mbedtls_hashtype
{
    TLSAPP_MBEDTLS_HASHTYPE_NONE = 0,    /**< None. */
    TLSAPP_MBEDTLS_HASHTYPE_SHA256,      /**< The SHA-256 message digest. */
} tlsapp_mbedtls_hashtype_t;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */

#endif /* PSA_CRYPTO_ACCELERATOR_DRIVER_PRESENT && PSA_CRYPTO_DRIVER_TEST */

#endif /* TLSAPP_MBEDTLS_HWACC_DRV_TYPES_H */
