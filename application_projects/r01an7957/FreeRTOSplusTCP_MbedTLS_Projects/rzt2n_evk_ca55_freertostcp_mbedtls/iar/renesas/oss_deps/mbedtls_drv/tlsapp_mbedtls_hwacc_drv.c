/*
 *  HW accelerator driver for RSIP.
 */
/*  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

/*
 *  Partially modified for Renesas RZ/T2 and RZ/N2 products.
 *  This file applies Apache-2.0.
 */
#include "tlsapp_mbedtls_hwacc_drv.h"

#if defined(PSA_CRYPTO_ACCELERATOR_DRIVER_PRESENT) && defined(PSA_CRYPTO_DRIVER_TEST)

#include "psa/crypto.h"
#include "psa_crypto_cipher.h"
#include "psa_crypto_core.h"
#include "psa_crypto_ecp.h"
#include "psa_crypto_rsa.h"
#include "psa_crypto_ffdh.h"
#include "psa_crypto_hash.h"
#include "psa_crypto_aead.h"
#include "psa_crypto_mac.h"

#include "mbedtls/platform.h"
#include "mbedtls/cipher.h"
#include "mbedtls/ecp.h"
#include "mbedtls/md.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/constant_time.h"
#include "mbedtls/rsa.h"
#include "mbedtls/error.h"
#include "debug_internal.h"

#include <string.h>
#include <stdarg.h>

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
rsip_instance_ctrl_t    g_p_ctrl;
rsip_cfg_t              g_p_cfg;

/* ephemeral ECDH Public Key */
uint8_t g_eecdh_pubkey_status;
uint8_t g_eecdh_storedpubkey[TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_SECP256R1_PUB];

/* temporary AES Key */
uint8_t g_aeskey_status;
uint8_t g_aeskey_length;
uint8_t g_aes_storedkey[TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_AESKEY_256];

#if defined(MBEDTLS_THREADING_C)
mbedtls_threading_mutex_t g_mutex_rsip;
#endif /* MBEDTLS_THREADING_C */

static inline size_t mbedtls_test_get_wrapkeysize (const psa_key_type_t key_type,
                                                   const size_t key_bits);
static inline rsip_key_type_t mbedtls_test_get_rsip_keytype(const uint32_t key_type);
static inline rsip_hash_type_t mbedtls_test_get_rsip_hashtype(
                                    const tlsapp_mbedtls_hashtype_t hash_type);
static void mbedtls_test_print_msg( int level,
                             const char *file, int line,
                             const char *format, ...);
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
static inline size_t mbedtls_test_get_wrapkeysize (const psa_key_type_t key_type,
                                                    const size_t key_bits)
{
    size_t key_buffer_size = 0;
 
    /* Get wrapped key size */
    if ((PSA_KEY_TYPE_IS_ECC_KEY_PAIR(key_type)) &&
        (PSA_KEY_TYPE_ECC_GET_FAMILY(key_type) == PSA_ECC_FAMILY_SECP_R1) &&
        (256 == key_bits)) {
        /* ECC private key(secp256r1) size */
        key_buffer_size = TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_SECP256R1_PRI;
    }

    return key_buffer_size;
}

static inline rsip_key_type_t mbedtls_test_get_rsip_keytype(const uint32_t key_type)
{
    rsip_key_type_t rsip_keytype;

    switch(key_type)
    {
    case TLSAPP_MBEDTLS_RSIP_IMPCRTKEY_RSA2K:
        rsip_keytype = RSIP_KEY_TYPE_RSA_2048_PUBLIC;
        break;
    case TLSAPP_MBEDTLS_RSIP_IMPCRTKEY_ECP256:
        rsip_keytype = RSIP_KEY_TYPE_ECC_SECP256R1_PUBLIC;
        break;
    default:
        rsip_keytype = RSIP_KEY_TYPE_INVALID;
        break;
    }
    return rsip_keytype;
}

static inline rsip_hash_type_t mbedtls_test_get_rsip_hashtype(const tlsapp_mbedtls_hashtype_t hash_type)
{
    rsip_hash_type_t rsip_hashtype;

    switch(hash_type)
    {
    case TLSAPP_MBEDTLS_HASHTYPE_SHA256:
        rsip_hashtype = RSIP_HASH_TYPE_SHA256;
        break;
    default:
        rsip_hashtype = RSIP_HASH_TYPE_NUM;
        break;
    }
    return rsip_hashtype;
}

void mbedtls_test_print_msg( int level,
                             const char *file, int line,
                             const char *format, ...)
{
    /* Unused parameters. */
    ( void ) file;
    ( void ) line;

    va_list argp;
    char str[512];
    int ret = -1;

    if (level > 0 ) {
        return;
    }

    va_start(argp, format);
    ret = mbedtls_vsnprintf(str, 512, format, argp);
    va_end(argp);

    if (ret < 0) {
        ret = 0;
    } else {
        if (ret >= 512 - 1) {
            ret = 512 - 2;
        }
    }
    str[ret]     = '\n';
    str[ret + 1] = '\0';

    printf( "Mbed TLS: |%d| %s", level, str );
}

#define TLS_MBEDTLS_DEBUG_MSG(level, args)               \
    mbedtls_test_print_msg(level, __FILE__, __LINE__,    \
                            MBEDTLS_DEBUG_STRIP_PARENS args)
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */

/***********************************************************************************************************************
 * Public Functions
 **********************************************************************************************************************/
psa_status_t mbedtls_test_transparent_init(void)
{
    return PSA_SUCCESS;
}

void mbedtls_test_transparent_free(void)
{
    return;
}

psa_status_t mbedtls_test_transparent_open(void)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    g_eecdh_pubkey_status = TLSAPP_MBEDTLS_RSIP_STOREDKEY_NOTEXIST;
    g_aeskey_length = 0;
    g_aeskey_status = TLSAPP_MBEDTLS_RSIP_STOREDKEY_NOTEXIST;
#if defined(MBEDTLS_THREADING_C)
    memset(&g_mutex_rsip, 0x00, sizeof(mbedtls_threading_mutex_t));
    mbedtls_mutex_init(&g_mutex_rsip);
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    fsp_err_t rsip_ret = R_RSIP_Open((rsip_ctrl_t*)&g_p_ctrl, &g_p_cfg);

    #if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */
    if (FSP_ERR_ALREADY_OPEN == rsip_ret){
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_Open is already called."));
        return PSA_SUCCESS;
    } else {
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_Open is called: %d", rsip_ret));
    }

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif // TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE

    return PSA_SUCCESS;
}

psa_status_t mbedtls_test_transparent_close(void)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    fsp_err_t rsip_ret = R_RSIP_Close((rsip_ctrl_t*)&g_p_ctrl);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_Close is called: %d", rsip_ret));
#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
    mbedtls_mutex_free(&g_mutex_rsip);
    memset(&g_mutex_rsip, 0x00, sizeof(mbedtls_threading_mutex_t));
#endif /* MBEDTLS_THREADING_C */
    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif // TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE

    return PSA_SUCCESS;
}

psa_status_t mbedtls_test_transparent_rootcertkey_import(
    uint8_t const *const    cert,
    uint8_t const *const    mac,
    uint8_t const           key_type,
    uint8_t const *const    keyparam1_pos,
    size_t const            key_param1_len,
    uint8_t const *const    keyparam2_pos,
    size_t const            key_param2_len,
    uint8_t *p_wrappubkey)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    rsip_key_type_t rsip_keytype;
    rsip_root_cert_mac_t rsip_mac;

#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    rsip_keytype = mbedtls_test_get_rsip_keytype(key_type);
    if (RSIP_KEY_TYPE_INVALID == rsip_keytype)
        return PSA_ERROR_INVALID_ARGUMENT;

    rsip_wrapped_key_t wrapped_pubkey = {rsip_keytype, p_wrappubkey};

    memcpy(rsip_mac.value, mac, 48);

    fsp_err_t rsip_ret = R_RSIP_PKI_RootCertKeyImport(
                                    (rsip_ctrl_t*)&g_p_ctrl,
                                    cert,
                                    &rsip_mac,
                                    keyparam1_pos,
                                    key_param1_len,
                                    keyparam2_pos,
                                    key_param2_len,
                                    &wrapped_pubkey);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_PKI_RootCertKeyImport is called: %d", rsip_ret));
#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif // TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE
    (void) cert;
    (void) mac;
    (void) key_type;
    (void) keyparam1_pos;
    (void) keyparam2_pos;
    (void) p_wrappubkey;

    return PSA_SUCCESS;
}

psa_status_t mbedtls_test_transparent_import_key(
    const psa_key_attributes_t *attributes,
    const uint8_t *data,
    size_t data_length,
    uint8_t *key_buffer,
    size_t key_buffer_size,
    size_t *key_buffer_length,
    size_t *bits)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    /* set bit size only for ECC key and aes key */
    switch (psa_get_rsip_keytype(attributes)) {
        case TLSAPP_MBEDTLS_RSIPWRAPKEY_HMAC_KDF:
        case TLSAPP_MBEDTLS_RSIPWRAPKEY_HMAC:
        case TLSAPP_MBEDTLS_RSIPWRAPKEY_CLI_ECDHEKEYPAIR:
        case TLSAPP_MBEDTLS_RSIPWRAPKEY_SRV_ECDHEKEYPAIR:
        case TLSAPP_MBEDTLS_RSIPWRAPKEY_RSA_CERTVERIFY:
        case TLSAPP_MBEDTLS_RSIPWRAPKEY_RSA_MSGVERIFY:
        case TLSAPP_MBEDTLS_RSIPWRAPKEY_RSA_SIGN:
        case TLSAPP_MBEDTLS_RSIPWRAPKEY_RSA:
            break;
        case TLSAPP_MBEDTLS_RSIPWRAPKEY_AES128:
            *bits = PSA_BYTES_TO_BITS(16);
            break;
        case TLSAPP_MBEDTLS_RSIPWRAPKEY_AES256:
            *bits = PSA_BYTES_TO_BITS(32);
            break;
        case TLSAPP_MBEDTLS_RSIPWRAPKEY_ECC_CERTVERIFY:
        case TLSAPP_MBEDTLS_RSIPWRAPKEY_ECC_MSGVERIFY:
        case TLSAPP_MBEDTLS_RSIPWRAPKEY_ECC_SIGN:
            *bits = PSA_BYTES_TO_BITS(32);
            break;
        default:
            return psa_import_key_into_slot(attributes,
                                            data, data_length,
                                            key_buffer, key_buffer_size,
                                            key_buffer_length, bits);
    }

    /* copy input data to output data */
    memcpy(key_buffer, data, data_length);
    *key_buffer_length = data_length;

    (void) attributes;
    (void) key_buffer_size;

    return PSA_SUCCESS;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) attributes;
    (void) data;
    (void) data_length;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) key_buffer_length;
    (void) bits;

    return PSA_ERROR_NOT_SUPPORTED;
}

size_t mbedtls_test_transparent_size_function(
    const psa_key_type_t key_type,
    const size_t key_bits)
{
    size_t key_buffer_size = 0;
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    /* Get wrapped key size */
    key_buffer_size = mbedtls_test_get_wrapkeysize(key_type, key_bits);
    TLS_MBEDTLS_DEBUG_MSG(1, ("mbedtls_test_get_wrapkeysize key size:%d", key_buffer_size));
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) key_type;
    (void) key_bits;

    return key_buffer_size;
}

psa_status_t mbedtls_test_transparent_generate_key(
    const psa_key_attributes_t *attributes,
    uint8_t *key, size_t key_size, size_t *key_length)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    if (((psa_get_rsip_keytype(attributes) == TLSAPP_MBEDTLS_RSIPWRAPKEY_CLI_ECDHEKEYPAIR) ||
        (psa_get_rsip_keytype(attributes) == TLSAPP_MBEDTLS_RSIPWRAPKEY_SRV_ECDHEKEYPAIR)) && 
        (PSA_KEY_TYPE_IS_ECC_KEY_PAIR(psa_get_key_type(attributes))) && 
        (psa_get_key_type(attributes) == PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1)) &&
        (256 == psa_get_key_bits(attributes)))
    {
        /* ecc public key is already exist */
        if (TLSAPP_MBEDTLS_RSIP_STOREDKEY_EXIST == g_eecdh_pubkey_status) {
            TLS_MBEDTLS_DEBUG_MSG(1, ("ecc public key is already exist."));
            return PSA_ERROR_BAD_STATE;
        }

        /* ECC key with secp256r1 */
        if (TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_SECP256R1_PRI > key_size)
            return PSA_ERROR_INVALID_ARGUMENT;

#if defined(MBEDTLS_THREADING_C)
        if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
            return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

        rsip_wrapped_key_t wrapped_pubkey = {RSIP_KEY_TYPE_ECC_SECP256R1_PUBLIC,
                                             g_eecdh_storedpubkey};
        rsip_wrapped_key_t wrapped_prikey = {RSIP_KEY_TYPE_ECC_SECP256R1_PRIVATE,
                                             key};

        fsp_err_t rsip_ret = R_RSIP_KeyPairGenerate ((rsip_ctrl_t*)&g_p_ctrl,
                                                     &wrapped_pubkey,
                                                     &wrapped_prikey);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_KeyPairGenerate is called: %d", rsip_ret));
        if (FSP_SUCCESS == rsip_ret) {
            /* ecc public key status is exist */
            g_eecdh_pubkey_status = TLSAPP_MBEDTLS_RSIP_STOREDKEY_EXIST;
            *key_length = TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_SECP256R1_PRI;
        }

#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

        return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
    }
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) attributes;
    (void) key;
    (void) key_size;
    (void) key_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_export_public_key(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer, size_t key_buffer_size,
    uint8_t *data, size_t data_size, size_t *data_length)
{
    (void) key_buffer;
    (void) key_buffer_size;
    (void) data_size;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    if ((psa_get_rsip_keytype(attributes) == TLSAPP_MBEDTLS_RSIPWRAPKEY_CLI_ECDHEKEYPAIR) ||
        (psa_get_rsip_keytype(attributes) == TLSAPP_MBEDTLS_RSIPWRAPKEY_SRV_ECDHEKEYPAIR)) {
        /* check whether ecc public key status is exist */
        if (TLSAPP_MBEDTLS_RSIP_STOREDKEY_EXIST != g_eecdh_pubkey_status)
            return PSA_ERROR_BAD_STATE;

        /* clear ecc public key status */
        g_eecdh_pubkey_status = TLSAPP_MBEDTLS_RSIP_STOREDKEY_NOTEXIST;

#if defined(MBEDTLS_THREADING_C)
        if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
            return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */
        rsip_wrapped_key_t wrapped_pubkey = {RSIP_KEY_TYPE_ECC_SECP256R1_PUBLIC,
                                             g_eecdh_storedpubkey};

        fsp_err_t rsip_ret = R_RSIP_PublicKeyExport (
                                &wrapped_pubkey,
                                data + 1);
        if (FSP_SUCCESS == rsip_ret) {
            /* set uncompressed format */
            data[0] = 0x04;
            *data_length = TLSAPP_MBEDTLS_RSIP_ECP_UNCOMPRESSED_PUBKEYSIZE;
        }
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_PublicKeyExport is called: %d", rsip_ret));
        mbedtls_platform_zeroize(&g_eecdh_storedpubkey[0], sizeof(g_eecdh_storedpubkey));

#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

        return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
    }
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) attributes;
    (void) data;
    (void) data_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_signature_verify_hash(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer, size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *hash, size_t hash_length,
    const uint8_t *signature, size_t signature_length)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    fsp_err_t rsip_ret = FSP_ERR_ASSERTION;
    uint8_t   rsip_keytype = psa_get_rsip_keytype(attributes);
    psa_key_type_t key_type = psa_get_key_type(attributes);
    rsip_wrapped_key_t wrapped_pubkey = {RSIP_KEY_TYPE_INVALID, (void *)key_buffer};

    if ((TLSAPP_MBEDTLS_RSIPWRAPKEY_RSA_CERTVERIFY == rsip_keytype) ||
        (TLSAPP_MBEDTLS_RSIPWRAPKEY_ECC_CERTVERIFY == rsip_keytype)) {
        if (PSA_KEY_TYPE_RSA_PUBLIC_KEY == key_type) {
            if (TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_RSA2048_PUB < key_buffer_size) {
                return PSA_ERROR_INVALID_ARGUMENT;
            }

            wrapped_pubkey.type = RSIP_KEY_TYPE_RSA_2048_PUBLIC;

            if (PSA_ALG_IS_RSA_PKCS1V15_SIGN(alg)) {
                rsip_ret = R_RSIP_PKI_RSASSA_PKCS1_V1_5_CertVerify(
                                    (rsip_ctrl_t*)&g_p_ctrl,
                                    &wrapped_pubkey,
                                    RSIP_HASH_TYPE_SHA256,
                                    hash,
                                    signature);
                TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_PKI_RSASSA_PKCS1_V1_5_CertVerify is called: %d", rsip_ret));
            } else if (PSA_ALG_IS_RSA_PSS(alg)) {
                rsip_ret = R_RSIP_PKI_RSASSA_PSS_CertVerify(
                                    (rsip_ctrl_t*)&g_p_ctrl,
                                    &wrapped_pubkey,
                                    RSIP_HASH_TYPE_SHA256,
                                    RSIP_MGF_TYPE_MGF1_SHA256,
                                    RSIP_RSA_SALT_LENGTH_AUTO,
                                    hash,
                                    signature);
                TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_PKI_RSASSA_PSS_CertVerify is called: %d", rsip_ret));
            }
        } else if (PSA_KEY_TYPE_IS_ECC_PUBLIC_KEY(key_type)) {
            if (TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_SECP256R1_PUB < key_buffer_size) {
                return PSA_ERROR_INVALID_ARGUMENT;
            }

            wrapped_pubkey.type    = RSIP_KEY_TYPE_ECC_SECP256R1_PUBLIC;

            rsip_ret = R_RSIP_PKI_ECDSA_CertVerify(
                                (rsip_ctrl_t*)&g_p_ctrl,
                                &wrapped_pubkey,
                                hash,
                                signature);
            TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_PKI_ECDSA_CertVerify is called: %d", rsip_ret));
        } else {
            return PSA_ERROR_INVALID_ARGUMENT;
        }
    } else if ((TLSAPP_MBEDTLS_RSIPWRAPKEY_RSA_MSGVERIFY == rsip_keytype) ||
              (TLSAPP_MBEDTLS_RSIPWRAPKEY_ECC_MSGVERIFY == rsip_keytype)) {
        if (PSA_KEY_TYPE_RSA_PUBLIC_KEY == key_type) {

            wrapped_pubkey.type = RSIP_KEY_TYPE_RSA_2048_PUBLIC;

            rsip_ret = R_RSIP_RSASSA_PKCS1_V1_5_Verify(
                                (rsip_ctrl_t*)&g_p_ctrl,
                                &wrapped_pubkey,
                                RSIP_HASH_TYPE_SHA256,
                                hash,
                                signature);
            TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_RSASSA_PKCS1_V1_5_Verify is called: %d", rsip_ret));
        } else if (PSA_KEY_TYPE_IS_ECC_PUBLIC_KEY(key_type)) {

            wrapped_pubkey.type = RSIP_KEY_TYPE_ECC_SECP256R1_PUBLIC;

            rsip_ret = R_RSIP_ECDSA_Verify(
                                    (rsip_ctrl_t*)&g_p_ctrl,
                                    &wrapped_pubkey,
                                    hash,
                                    signature);
            TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_ECDSA_Verify is called: %d", rsip_ret));
        } else {
            return PSA_ERROR_INVALID_ARGUMENT;
        }
    } else {
        return PSA_ERROR_INVALID_ARGUMENT;
    }
    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;
    (void) hash;
    (void) hash_length;
    (void) signature;
    (void) signature_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_signature_sign_hash(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer, size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *hash, size_t hash_length,
    uint8_t *signature, size_t signature_size, size_t *signature_length)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    fsp_err_t rsip_ret = FSP_ERR_ASSERTION;
    psa_key_type_t key_type = psa_get_key_type(attributes);
    rsip_wrapped_key_t wrapped_pubkey = {RSIP_KEY_TYPE_INVALID, (void *)key_buffer};

    if (PSA_KEY_TYPE_IS_RSA(key_type)) {

        wrapped_pubkey.type = RSIP_KEY_TYPE_RSA_2048_PUBLIC;

        rsip_ret = R_RSIP_RSASSA_PKCS1_V1_5_Sign(
                            (rsip_ctrl_t*)&g_p_ctrl,
                            &wrapped_pubkey,
                            RSIP_HASH_TYPE_SHA256,
                            hash,
                            signature);
        *signature_length = 256;
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_RSASSA_PKCS1_V1_5_Sign is called: %d", rsip_ret));
    } else if (PSA_KEY_TYPE_IS_ECC_KEY_PAIR(key_type)) {

        wrapped_pubkey.type = RSIP_KEY_TYPE_ECC_SECP256R1_PUBLIC;

        rsip_ret = R_RSIP_ECDSA_Sign(
                            (rsip_ctrl_t*)&g_p_ctrl,
                            &wrapped_pubkey,
                            hash,
                            signature);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_ECDSA_Sign is called: %d", rsip_ret));
        *signature_length = 64;
    } else {
        return PSA_ERROR_INVALID_ARGUMENT;
    }
    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */

    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;
    (void) hash;
    (void) hash_length;
    (void) signature;
    (void) signature_size;
    (void) signature_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_get_cert_wrappubkey(
    uint8_t const *const            p_cert,
    size_t const                    cert_length,
    uint32_t const                  key_type,
    uint8_t const *const            p_key_param1,
    size_t const                    key_param1_len,
    uint8_t const *const            p_key_param2,
    size_t const                    key_param2_len,
    tlsapp_mbedtls_hashtype_t const hash_function,
    uint8_t                         *key,
    size_t                          key_size)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    rsip_key_type_t rsip_keytype = mbedtls_test_get_rsip_keytype(key_type);
    if (RSIP_KEY_TYPE_INVALID == rsip_keytype)
        return PSA_ERROR_INVALID_ARGUMENT;

    rsip_hash_type_t rsip_hashtype = mbedtls_test_get_rsip_hashtype(hash_function);
    if (RSIP_HASH_TYPE_NUM == rsip_hashtype)
        return PSA_ERROR_INVALID_ARGUMENT;

    rsip_wrapped_key_t wrapped_pubkey = {rsip_keytype, key};

    fsp_err_t rsip_ret = R_RSIP_PKI_CertKeyImport(
                                (rsip_ctrl_t*)&g_p_ctrl,
                                p_cert,
                                cert_length,
                                p_key_param1,
                                key_param1_len,
                                p_key_param2,
                                key_param2_len,
                                rsip_hashtype,
                                &wrapped_pubkey);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_PKI_CertKeyImport is called: %d", rsip_ret));

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) p_cert;
    (void) cert_length;
    (void) key_type;
    (void) p_key_param1;
    (void) p_key_param2;
    (void) hash_function;
    (void) key;
    (void) key_size;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_key_agreement(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *peer_key,
    size_t peer_key_length,
    uint8_t *shared_secret,
    size_t shared_secret_size,
    size_t *shared_secret_length)
{
    (void) key_buffer_size;
    (void) alg;
    (void) peer_key_length;
    (void) shared_secret_size;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    fsp_err_t   rsip_ret;
    uint8_t     temp_sharedsecret[TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_PMSECRET];

#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    rsip_wrapped_key_t wrapped_prikey = {RSIP_KEY_TYPE_ECC_SECP256R1_PRIVATE,
                                         (void *)key_buffer};

    if (psa_get_rsip_keytype(attributes) == TLSAPP_MBEDTLS_RSIPWRAPKEY_CLI_ECDHEKEYPAIR) {

        rsip_wrapped_key_t wrapped_pubkey = {RSIP_KEY_TYPE_ECC_SECP256R1_PUBLIC,
                                             (void *)peer_key};

        rsip_ret = R_RSIP_ECDH_KeyAgree((rsip_ctrl_t*)&g_p_ctrl,
                                        &wrapped_prikey,
                                        &wrapped_pubkey,
                                        (rsip_wrapped_secret_t*)temp_sharedsecret);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_ECDH_KeyAgree is called: %d", rsip_ret));
        if (FSP_SUCCESS != rsip_ret)
            goto func_done;
    } else if (psa_get_rsip_keytype(attributes) == TLSAPP_MBEDTLS_RSIPWRAPKEY_SRV_ECDHEKEYPAIR) {
        rsip_ret = R_RSIP_ECDH_PlainKeyAgree((rsip_ctrl_t*)&g_p_ctrl,
                                             &wrapped_prikey,
                                             &peer_key[1],
                                             (rsip_wrapped_secret_t*)temp_sharedsecret);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_ECDH_PlainKeyAgree is called: %d", rsip_ret));
        if (FSP_SUCCESS != rsip_ret)
            goto func_done;
    } else {
#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */
        return PSA_ERROR_NOT_SUPPORTED;
    }

    rsip_wrapped_key_t wrapped_key = {RSIP_KEY_TYPE_KDF_HMAC_SHA256, shared_secret};

    /* convert wrapped secret into wrapped kdf hmac key */
    rsip_ret = R_RSIP_KDF_HMAC_ECDHSecretKeyImport(
                                        (rsip_ctrl_t*)&g_p_ctrl,
                                        (rsip_wrapped_secret_t*)temp_sharedsecret,
                                        &wrapped_key);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_KDF_HMAC_ECDHSecretKeyImport is called: %d", rsip_ret));
    if (FSP_SUCCESS == rsip_ret)
        *shared_secret_length = TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_KDFHMAC_PMS;

func_done:
    mbedtls_platform_zeroize(temp_sharedsecret, TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_PMSECRET);

#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) attributes;
    (void) key_buffer;
    (void) peer_key;
    (void) shared_secret;
    (void) shared_secret_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_kdf_mackeyimport(
    const uint8_t *p_wrapped_mac,
    size_t wrapped_macsize,
    uint8_t *p_wrapped_key)

{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    rsip_wrapped_key_t wrapped_key = {RSIP_KEY_TYPE_KDF_HMAC_SHA256, p_wrapped_key};

    /* convert wrapped mac into wrapped kdf hmac key */
    fsp_err_t rsip_ret = R_RSIP_KDF_HMAC_DKMKeyImport(
                                (rsip_ctrl_t*)&g_p_ctrl,
                                (rsip_wrapped_dkm_t*)p_wrapped_mac,
                                wrapped_macsize,
                                &wrapped_key);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_KDF_MACKeyImport is called: %d", rsip_ret));

#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) p_wrapped_mac;
    (void) wrapped_macsize;
    (void) p_wrapped_key;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_mac_sign_setup(
    mbedtls_transparent_test_driver_mac_operation_t *operation,
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg)
{
    (void) alg;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    fsp_err_t rsip_ret = FSP_ERR_ASSERTION;
    rsip_wrapped_key_t wrapped_key = {RSIP_KEY_TYPE_INVALID, (void *)key_buffer};

    if (psa_get_rsip_keytype(attributes) == TLSAPP_MBEDTLS_RSIPWRAPKEY_HMAC_KDF) {
#if defined(MBEDTLS_THREADING_C)
        if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
            return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

        wrapped_key.type = RSIP_KEY_TYPE_KDF_HMAC_SHA256;

        rsip_ret = R_RSIP_KDF_HMAC_Init(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        &wrapped_key);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_KDF_HMAC_Init is called: %d", rsip_ret));
        if (FSP_SUCCESS == rsip_ret)
            operation->ctx.hmac.hmac_type = TLSAPP_MBEDTLS_RSIP_HMACTYPE_KDF;

#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

        return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
    } else {
        if (RSIP_BYTE_SIZE_WRAPPED_KEY_HMAC_SHA256 != key_buffer_size)
            return PSA_ERROR_CORRUPTION_DETECTED;

#if defined(MBEDTLS_THREADING_C)
        if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
            return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

        wrapped_key.type = RSIP_KEY_TYPE_HMAC_SHA256;

        rsip_ret = R_RSIP_HMAC_Init(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        &wrapped_key);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_HMAC_Init is called: %d", rsip_ret));
        if (FSP_SUCCESS == rsip_ret)
            operation->ctx.hmac.hmac_type = TLSAPP_MBEDTLS_RSIP_HMACTYPE_SIGN;

#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

        return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
    }
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) operation;
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_mac_sign_finish(
    mbedtls_transparent_test_driver_mac_operation_t *operation,
    uint8_t *mac,
    size_t mac_size,
    size_t *mac_length)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    fsp_err_t rsip_ret = FSP_ERR_ASSERTION;

    if (TLSAPP_MBEDTLS_RSIP_HMACTYPE_KDF == operation->ctx.hmac.hmac_type) {
#if defined(MBEDTLS_THREADING_C)
        if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
            return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

        rsip_ret = R_RSIP_KDF_HMAC_SignFinish(
                            (rsip_ctrl_t*)&g_p_ctrl,
                            (rsip_wrapped_dkm_t*)mac);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_KDF_HMAC_SignFinish is called: %d", rsip_ret));
        if (FSP_SUCCESS == rsip_ret)
            *mac_length = mac_size;

#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

        return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
    } else if (TLSAPP_MBEDTLS_RSIP_HMACTYPE_SIGN == operation->ctx.hmac.hmac_type) {
#if defined(MBEDTLS_THREADING_C)
        if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
            return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

        rsip_ret = R_RSIP_HMAC_SignFinish(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        mac);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_HMAC_SignFinish is called: %d", rsip_ret));
        if (FSP_SUCCESS == rsip_ret)
            *mac_length = mac_size;

#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

        return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
    }
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) operation;
    (void) mac;
    (void) mac_size;
    (void) mac_length;

    return PSA_ERROR_CORRUPTION_DETECTED;
}

psa_status_t mbedtls_test_transparent_mac_verify_setup(
    mbedtls_transparent_test_driver_mac_operation_t *operation,
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg)
{
    (void) attributes;
    (void) alg;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    if (RSIP_BYTE_SIZE_WRAPPED_KEY_HMAC_SHA256 != key_buffer_size)
        return PSA_ERROR_INVALID_ARGUMENT;

#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    rsip_wrapped_key_t wrapped_key = {RSIP_KEY_TYPE_HMAC_SHA256, (void *)key_buffer};

    fsp_err_t rsip_ret = R_RSIP_HMAC_Init(
                                (rsip_ctrl_t*)&g_p_ctrl,
                                &wrapped_key);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_HMAC_Init is called: %d", rsip_ret));
    if (FSP_SUCCESS == rsip_ret)
        operation->ctx.hmac.hmac_type = TLSAPP_MBEDTLS_RSIP_HMACTYPE_VERIFY;

#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) operation;
    (void) key_buffer;
    (void) key_buffer_size;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_mac_verify_finish(
    mbedtls_transparent_test_driver_mac_operation_t *operation,
    const uint8_t *mac,
    size_t mac_length)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    if (TLSAPP_MBEDTLS_RSIP_HMACTYPE_VERIFY == operation->ctx.hmac.hmac_type) {
#if defined(MBEDTLS_THREADING_C)
        if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
            return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

        fsp_err_t rsip_ret = R_RSIP_HMAC_VerifyFinish (
                                    (rsip_ctrl_t*)&g_p_ctrl,
                                    mac,
                                    mac_length);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_HMAC_VerifyFinish is called: %d", rsip_ret));

#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

        return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
    }
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) operation;
    (void) mac;
    (void) mac_length;

    return PSA_ERROR_CORRUPTION_DETECTED;
}

psa_status_t mbedtls_test_transparent_mac_update(
    mbedtls_transparent_test_driver_mac_operation_t *operation,
    const uint8_t *input,
    size_t input_length)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    fsp_err_t rsip_ret = FSP_ERR_ASSERTION;

    if (TLSAPP_MBEDTLS_RSIP_HMACTYPE_KDF == operation->ctx.hmac.hmac_type) {
#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

        rsip_ret = R_RSIP_KDF_HMAC_Update(
                            (rsip_ctrl_t*)&g_p_ctrl,
                            input,
                            input_length);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_KDF_HMAC_Update is called: %d", rsip_ret));

#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

        return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
    } else if ((TLSAPP_MBEDTLS_RSIP_HMACTYPE_SIGN == operation->ctx.hmac.hmac_type) ||
               (TLSAPP_MBEDTLS_RSIP_HMACTYPE_VERIFY == operation->ctx.hmac.hmac_type)) {
#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

        rsip_ret = R_RSIP_HMAC_Update(
                            (rsip_ctrl_t*)&g_p_ctrl,
                            input,
                            input_length);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_HMAC_Update is called: %d", rsip_ret));

#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

        return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
    }
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) operation;
    (void) input;
    (void) input_length;

    return PSA_ERROR_BAD_STATE;
}

psa_status_t mbedtls_test_transparent_mac_abort(
    mbedtls_transparent_test_driver_mac_operation_t *operation)
{
    (void) operation;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    return PSA_SUCCESS;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    return PSA_ERROR_BAD_STATE;
}

psa_status_t mbedtls_test_transparent_hmac_macupdate(
    const uint8_t * const p_mac_msg)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    fsp_err_t rsip_ret = R_RSIP_KDF_HMAC_DKMUpdate (
                                    (rsip_ctrl_t*)&g_p_ctrl,
                                    (rsip_wrapped_dkm_t*)p_mac_msg);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_KDF_HMAC_MACUpdate is called: %d", rsip_ret));

#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) p_mac_msg;

    return PSA_SUCCESS;
}

psa_status_t mbedtls_test_transparent_mac_concatenate(
    uint8_t * const p_wrapped_mac1,
    const uint8_t * const p_wrapped_mac2,
    const uint32_t wrapped_buffer_length)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    fsp_err_t rsip_ret = R_RSIP_KDF_DKMConcatenate(
                                    (rsip_wrapped_dkm_t*)p_wrapped_mac1,
                                    (rsip_wrapped_dkm_t*)p_wrapped_mac2,
                                    wrapped_buffer_length);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_KDF_DKMConcatenate is called: %d", rsip_ret));

#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) p_wrapped_mac1;
    (void) p_wrapped_mac2;
    (void) wrapped_buffer_length;

    return PSA_SUCCESS;
}

psa_status_t mbedtls_test_transparent_get_derivedwrapkey(
    const uint8_t * const p_wrapped_data,
    const uint32_t key_type,
    const uint32_t position,
    uint8_t* const p_wrapped_key)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    rsip_key_type_t rsip_keytype = RSIP_KEY_TYPE_INVALID;
    if (TLSAPP_MBEDTLS_RSIPWRAPKEY_AES128 == key_type) {
        rsip_keytype = RSIP_KEY_TYPE_AES_128;
    } else if (TLSAPP_MBEDTLS_RSIPWRAPKEY_AES256 == key_type) {
        rsip_keytype = RSIP_KEY_TYPE_AES_256;
    } else if (TLSAPP_MBEDTLS_RSIPWRAPKEY_HMAC == key_type) {
        rsip_keytype = RSIP_KEY_TYPE_HMAC_SHA256;
    } else {
        TLS_MBEDTLS_DEBUG_MSG(1, ("mbedtls_test_transparent_get_derivedwrapkey invalid key type is selected."));
        return PSA_ERROR_INVALID_ARGUMENT;
    }

#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    rsip_wrapped_key_t wrapped_key = {rsip_keytype, p_wrapped_key};

    fsp_err_t rsip_ret = R_RSIP_KDF_DerivedKeyImport(
                                    (rsip_ctrl_t*)&g_p_ctrl,
                                    (rsip_wrapped_dkm_t*)p_wrapped_data,
                                    position,
                                    &wrapped_key);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_KDF_DerivedKeyImport is called: %d", rsip_ret));

#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) p_wrapped_data;
    (void) key_type;
    (void) position;
    (void) p_wrapped_key;

    return PSA_SUCCESS;
}

psa_status_t mbedtls_test_transparent_get_derivedwrapiv(
    const uint8_t * const p_wrapped_data,
    const uint32_t data_type,
    const uint32_t position,
    const uint8_t* const p_seq_num,
    uint8_t* const p_wrapped_iv)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    fsp_err_t rsip_ret = R_RSIP_KDF_DerivedIVWrap(
                                    (rsip_ctrl_t*)&g_p_ctrl,
                                    (rsip_wrapped_dkm_t*)p_wrapped_data,
                                    (rsip_initial_vector_type_t)data_type,
                                    position,
                                    p_seq_num,
                                    p_wrapped_iv);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_KDF_DerivedIVWrap is called: %d", rsip_ret));

#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) p_wrapped_data;
    (void) data_type;
    (void) position;
    (void) p_seq_num;
    (void) p_wrapped_iv;

    return PSA_SUCCESS;
}

psa_status_t mbedtls_test_transparent_get_verifydata(
    const uint8_t * const p_wrapped_data,
    const int label,
    const uint8_t * const p_hash,
    uint8_t * const p_verify_data)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    rsip_wrapped_key_t wrapped_key = {RSIP_KEY_TYPE_KDF_HMAC_SHA256,
                                      (void *)p_wrapped_data};

    fsp_err_t rsip_ret = R_RSIP_KDF_TLS12PRFVerifyDataCompute(
                                    (rsip_ctrl_t*)&g_p_ctrl,
                                    &wrapped_key,
                                    label,
                                    p_hash,
                                    p_verify_data);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_KDF_TLS12PRFVerifyDataCompute is called: %d", rsip_ret));

#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) p_wrapped_data;
    (void) label;
    (void) p_hash;
    (void) p_verify_data;

    return PSA_SUCCESS;
}

psa_status_t mbedtls_test_transparent_aead_encrypt(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer, size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *nonce, size_t nonce_length,
    const uint8_t *additional_data, size_t additional_data_length,
    const uint8_t *plaintext, size_t plaintext_length,
    uint8_t *ciphertext, size_t ciphertext_size, size_t *ciphertext_length)
{
    (void) key_buffer_size;
    (void) ciphertext_size;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    uint8_t key_type = psa_get_rsip_keytype(attributes);
    rsip_key_type_t rsip_keytype = RSIP_KEY_TYPE_INVALID;

    if (TLSAPP_MBEDTLS_RSIPWRAPKEY_AES128 == key_type) {
        rsip_keytype = RSIP_KEY_TYPE_AES_128;
    } else if (TLSAPP_MBEDTLS_RSIPWRAPKEY_AES256 == key_type) {
        rsip_keytype = RSIP_KEY_TYPE_AES_256;
    } else if (TLSAPP_MBEDTLS_RSIPWRAPKEY_HMAC == key_type) {
        rsip_keytype = RSIP_KEY_TYPE_INVALID;
    }

    if (RSIP_KEY_TYPE_INVALID != rsip_keytype)
    {
        fsp_err_t rsip_ret = FSP_ERR_ASSERTION;
        uint32_t output_len = 0;

#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

        rsip_wrapped_key_t wrapped_key = {rsip_keytype,
                                          (void *)key_buffer};

        rsip_ret = R_RSIP_AES_AEAD_Init(
            (rsip_ctrl_t*)&g_p_ctrl,
            RSIP_AES_AEAD_MODE_GCM_ENC_WRAPPED_IV,
            &wrapped_key,
            nonce,
            nonce_length);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_AES_AEAD_Init for encryption is called: %d", rsip_ret));
        if (FSP_SUCCESS != rsip_ret)
            goto func_done;

        rsip_ret = R_RSIP_AES_AEAD_AADUpdate(
            (rsip_ctrl_t*)&g_p_ctrl,
            additional_data, 
            additional_data_length);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_AES_AEAD_AADUpdate is called: %d", rsip_ret));
        if (FSP_SUCCESS != rsip_ret)
            goto func_done;

        rsip_ret = R_RSIP_AES_AEAD_Update(
            (rsip_ctrl_t*)&g_p_ctrl,
            plaintext,
            plaintext_length,
            &ciphertext[0],
            &output_len);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_AES_AEAD_Update is called: %d", rsip_ret));
        if (FSP_SUCCESS != rsip_ret)
            goto func_done;
        *ciphertext_length = output_len;

        rsip_ret = R_RSIP_AES_AEAD_Finish(
            (rsip_ctrl_t*)&g_p_ctrl,
            &ciphertext[output_len],
            &output_len,
            &ciphertext[plaintext_length]);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_AES_AEAD_Finish is called: %d", rsip_ret));
        if (FSP_SUCCESS != rsip_ret)
            goto func_done;
        *ciphertext_length += output_len;
        *ciphertext_length += PSA_ALG_AEAD_GET_TAG_LENGTH(alg);

func_done:
#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

        return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
    }
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) attributes;
    (void) key_buffer;
    (void) alg;
    (void) nonce;
    (void) nonce_length;
    (void) additional_data;
    (void) additional_data_length;
    (void) plaintext;
    (void) plaintext_length;
    (void) ciphertext;
    (void) ciphertext_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_aead_decrypt(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer, size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *nonce, size_t nonce_length,
    const uint8_t *additional_data, size_t additional_data_length,
    const uint8_t *ciphertext, size_t ciphertext_length,
    uint8_t *plaintext, size_t plaintext_size, size_t *plaintext_length)
{
    (void) key_buffer_size;
    (void) plaintext_size;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    uint8_t key_type = psa_get_rsip_keytype(attributes);
    rsip_key_type_t rsip_keytype = RSIP_KEY_TYPE_INVALID;

    if (TLSAPP_MBEDTLS_RSIPWRAPKEY_AES128 == key_type) {
        rsip_keytype = RSIP_KEY_TYPE_AES_128;
    } else if (TLSAPP_MBEDTLS_RSIPWRAPKEY_AES256 == key_type) {
        rsip_keytype = RSIP_KEY_TYPE_AES_256;
    } else if (TLSAPP_MBEDTLS_RSIPWRAPKEY_HMAC == key_type) {
        rsip_keytype = RSIP_KEY_TYPE_INVALID;
    }

    if (RSIP_KEY_TYPE_INVALID != rsip_keytype)
    {
        fsp_err_t rsip_ret = FSP_ERR_ASSERTION;
        uint32_t output_len = 0;

#if defined(MBEDTLS_THREADING_C)
        if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
            return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

        rsip_wrapped_key_t wrapped_key = {rsip_keytype,
                                          (void *)key_buffer};

        rsip_ret = R_RSIP_AES_AEAD_Init(
            (rsip_ctrl_t*)&g_p_ctrl,
            RSIP_AES_AEAD_MODE_GCM_DEC_WRAPPED_IV,
            &wrapped_key,
            nonce,
            nonce_length);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_AES_AEAD_Init for decryption is called: %d", rsip_ret));
        if (FSP_SUCCESS != rsip_ret)
            goto func_done;

        rsip_ret = R_RSIP_AES_AEAD_AADUpdate(
            (rsip_ctrl_t*)&g_p_ctrl,
            additional_data, 
            additional_data_length);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_AES_AEAD_AADUpdate is called: %d", rsip_ret));
        if (FSP_SUCCESS != rsip_ret)
            goto func_done;

        rsip_ret = R_RSIP_AES_AEAD_Update(
            (rsip_ctrl_t*)&g_p_ctrl,
            ciphertext,
            ciphertext_length - PSA_ALG_AEAD_GET_TAG_LENGTH(alg),
            &plaintext[0],
            &output_len);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_AES_AEAD_Update is called: %d", rsip_ret));
        if (FSP_SUCCESS != rsip_ret)
            goto func_done;
        *plaintext_length = output_len;

        rsip_ret = R_RSIP_AES_AEAD_Verify(
            (rsip_ctrl_t*)&g_p_ctrl,
            &plaintext[output_len],
            &output_len,
            &ciphertext[ciphertext_length - PSA_ALG_AEAD_GET_TAG_LENGTH(alg)],
            PSA_ALG_AEAD_GET_TAG_LENGTH(alg));
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_AES_AEAD_Verify is called: %d", rsip_ret));
        if (FSP_SUCCESS != rsip_ret)
            goto func_done;
        *plaintext_length += output_len;

func_done:
#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

        return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
    }
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) attributes;
    (void) key_buffer;
    (void) alg;
    (void) nonce;
    (void) nonce_length;
    (void) additional_data;
    (void) additional_data_length;
    (void) ciphertext;
    (void) ciphertext_length;
    (void) plaintext;
    (void) plaintext_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_cipher_encrypt_setup(
    mbedtls_transparent_test_driver_cipher_operation_t *operation,
    const psa_key_attributes_t *attributes,
    const uint8_t *key, size_t key_length,
    psa_algorithm_t alg)
{
    (void) alg;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    if ((psa_get_rsip_keytype(attributes) == TLSAPP_MBEDTLS_RSIPWRAPKEY_AES128) ||
        (psa_get_rsip_keytype(attributes) == TLSAPP_MBEDTLS_RSIPWRAPKEY_AES256)) {
        TLS_MBEDTLS_DEBUG_MSG(1, ("mbedtls_test_transparent_cipher_encrypt_setup is called"));

        /* encrypt setup is already called. */
        if (TLSAPP_MBEDTLS_RSIP_STOREDKEY_EXIST == g_aeskey_status) {
            TLS_MBEDTLS_DEBUG_MSG(1, ("encrypt setup is already called."));
            return PSA_ERROR_BAD_STATE;
        }
        memcpy(g_aes_storedkey, key, key_length);
        g_aeskey_length = key_length;
        g_aeskey_status = TLSAPP_MBEDTLS_RSIP_STOREDKEY_EXIST;
        operation->cipher_type = TLSAPP_MBEDTLS_RSIP_CIPHERTYPE_CBCENC;

        return PSA_SUCCESS;
    }
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) operation;
    (void) attributes;
    (void) key;
    (void) key_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_cipher_decrypt_setup(
    mbedtls_transparent_test_driver_cipher_operation_t *operation,
    const psa_key_attributes_t *attributes,
    const uint8_t *key, size_t key_length,
    psa_algorithm_t alg)
{
    (void) alg;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    if ((psa_get_rsip_keytype(attributes) == TLSAPP_MBEDTLS_RSIPWRAPKEY_AES128) ||
        (psa_get_rsip_keytype(attributes) == TLSAPP_MBEDTLS_RSIPWRAPKEY_AES256)) {
        TLS_MBEDTLS_DEBUG_MSG(1, ("mbedtls_test_transparent_cipher_decrypt_setup is called"));

        memcpy(g_aes_storedkey, key, key_length);
        g_aeskey_length = key_length;
        g_aeskey_status = TLSAPP_MBEDTLS_RSIP_STOREDKEY_EXIST;
        operation->cipher_type = TLSAPP_MBEDTLS_RSIP_CIPHERTYPE_CBCDEC;

        return PSA_SUCCESS;
    }
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) operation;
    (void) attributes;
    (void) key;
    (void) key_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_cipher_abort(
    mbedtls_transparent_test_driver_cipher_operation_t *operation)
{
    (void) operation;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    return PSA_SUCCESS;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    return PSA_ERROR_BAD_STATE;
}

psa_status_t mbedtls_test_transparent_cipher_set_iv(
    mbedtls_transparent_test_driver_cipher_operation_t *operation,
    const uint8_t *iv,
    size_t iv_length)
{
    (void) iv_length;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    if ((TLSAPP_MBEDTLS_RSIP_CIPHERTYPE_CBCENC == operation->cipher_type) ||
        (TLSAPP_MBEDTLS_RSIP_CIPHERTYPE_CBCDEC == operation->cipher_type))
    {
        fsp_err_t rsip_ret = FSP_ERR_ASSERTION;

        if (TLSAPP_MBEDTLS_RSIP_STOREDKEY_EXIST != g_aeskey_status)
            return PSA_ERROR_BAD_STATE;

        g_aeskey_status = TLSAPP_MBEDTLS_RSIP_STOREDKEY_NOTEXIST;

#if defined(MBEDTLS_THREADING_C)
        if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
            return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */
        rsip_key_type_t rsip_keytype = RSIP_KEY_TYPE_INVALID;

        if (TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_AESKEY_128 == g_aeskey_length) {
            rsip_keytype = RSIP_KEY_TYPE_AES_128;
        } else if (TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_AESKEY_256 == g_aeskey_length) {
            rsip_keytype = RSIP_KEY_TYPE_AES_256;
        } else {
            rsip_keytype = RSIP_KEY_TYPE_INVALID;
        }

        rsip_wrapped_key_t wrapped_key = {rsip_keytype,
                                          g_aes_storedkey};

        rsip_ret = R_RSIP_AES_Cipher_Init(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        ((TLSAPP_MBEDTLS_RSIP_CIPHERTYPE_CBCENC == operation->cipher_type)
                            ? RSIP_AES_CIPHER_MODE_CBC_ENC
                            : RSIP_AES_CIPHER_MODE_CBC_DEC),
                        &wrapped_key,
                        iv);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_AES_Cipher_Init is called: %d", rsip_ret));
        mbedtls_platform_zeroize(&g_aes_storedkey[0], sizeof(g_aes_storedkey));
        g_aeskey_length = 0;

#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

        return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
    }
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) operation;
    (void) iv;

    return PSA_ERROR_CORRUPTION_DETECTED;
}

psa_status_t mbedtls_test_transparent_cipher_update(
    mbedtls_transparent_test_driver_cipher_operation_t *operation,
    const uint8_t *input,
    size_t input_length,
    uint8_t *output,
    size_t output_size,
    size_t *output_length)
{
    (void) input_length;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    if ((TLSAPP_MBEDTLS_RSIP_CIPHERTYPE_CBCENC == operation->cipher_type) ||
        (TLSAPP_MBEDTLS_RSIP_CIPHERTYPE_CBCDEC == operation->cipher_type))
    {
        fsp_err_t rsip_ret = FSP_ERR_ASSERTION;
#if defined(MBEDTLS_THREADING_C)
        if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
            return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

        rsip_ret = R_RSIP_AES_Cipher_Update(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        input,
                        output,
                        output_size);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_AES_Cipher_Update is called: %d", rsip_ret));
        if (FSP_SUCCESS == rsip_ret) {
            *output_length = output_size;
        }

#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

        return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
    }
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) operation;
    (void) input;
    (void) output;
    (void) output_size;
    (void) output_length;

    return PSA_ERROR_CORRUPTION_DETECTED;
}

psa_status_t mbedtls_test_transparent_cipher_finish(
    mbedtls_transparent_test_driver_cipher_operation_t *operation,
    uint8_t *output,
    size_t output_size,
    size_t *output_length)
{
    (void) output;
    (void) output_size;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    if ((TLSAPP_MBEDTLS_RSIP_CIPHERTYPE_CBCENC == operation->cipher_type) ||
        (TLSAPP_MBEDTLS_RSIP_CIPHERTYPE_CBCDEC == operation->cipher_type))
    {
        fsp_err_t rsip_ret = FSP_ERR_ASSERTION;
#if defined(MBEDTLS_THREADING_C)
        if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
            return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

        rsip_ret = R_RSIP_AES_Cipher_Finish(
                        (rsip_ctrl_t*)&g_p_ctrl);
        TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_AES_Cipher_Finish is called: %d", rsip_ret));
        if (FSP_SUCCESS == rsip_ret)
            *output_length = 0;

#if defined(MBEDTLS_THREADING_C)
        mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

        return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
    }
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) operation;
    (void) output_length;

    return PSA_ERROR_CORRUPTION_DETECTED;
}

psa_status_t mbedtls_test_transparent_hash_setup(
    mbedtls_transparent_test_driver_hash_operation_t *operation,
    psa_algorithm_t alg)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    fsp_err_t rsip_ret = FSP_ERR_ASSERTION;
    rsip_hash_type_t hash_type;

    if(alg == PSA_ALG_SHA_256) {
        hash_type = RSIP_HASH_TYPE_SHA256;
    } else {
        TLS_MBEDTLS_DEBUG_MSG(1, ("invalid hash type is selected."));
        return PSA_ERROR_INVALID_ARGUMENT;
    }

#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    rsip_ret = R_RSIP_SHA_Init(
                    (rsip_ctrl_t*)&g_p_ctrl,
                    hash_type);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_SHA_Init is called: %d", rsip_ret));
    if (FSP_SUCCESS != rsip_ret)
        goto func_done;

    rsip_ret = R_RSIP_SHA_Suspend(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        &operation->ctx.rsip_sha);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_SHA_Suspend is called: %d", rsip_ret));
    if (FSP_SUCCESS != rsip_ret)
        goto func_done;

    operation->alg = alg;

func_done:
#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif // TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE
    (void) operation;
    (void) alg;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_hash_update(
    mbedtls_transparent_test_driver_hash_operation_t *operation,
    const uint8_t *input,
    size_t input_length)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    fsp_err_t rsip_ret = FSP_ERR_ASSERTION;

#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    rsip_ret = R_RSIP_SHA_Resume(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        &operation->ctx.rsip_sha);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_SHA_Resume is called: %d", rsip_ret));
    if (FSP_SUCCESS != rsip_ret)
        goto func_done;

    rsip_ret = R_RSIP_SHA_Update(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        input,
                        input_length);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_SHA_Update is called: %d", rsip_ret));
    if (FSP_SUCCESS != rsip_ret)
        goto func_done;

    rsip_ret = R_RSIP_SHA_Suspend(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        &operation->ctx.rsip_sha);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_SHA_Suspend is called: %d", rsip_ret));
    if (FSP_SUCCESS != rsip_ret)
        goto func_done;

func_done:

#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif // TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE
    (void) operation;
    (void) input;
    (void) input_length;

    return PSA_ERROR_BAD_STATE;
}

psa_status_t mbedtls_test_transparent_hash_finish(
    mbedtls_transparent_test_driver_hash_operation_t *operation,
    uint8_t *hash,
    size_t hash_size,
    size_t *hash_length)
{
    (void) hash_size;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    fsp_err_t rsip_ret = FSP_ERR_ASSERTION;

#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    rsip_ret = R_RSIP_SHA_Resume(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        &operation->ctx.rsip_sha);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_SHA_Resume is called: %d", rsip_ret));
    if (FSP_SUCCESS != rsip_ret)
        goto func_done;

    rsip_ret = R_RSIP_SHA_Finish(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        hash);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_SHA_Finish is called: %d", rsip_ret));
    if (FSP_SUCCESS != rsip_ret)
        goto func_done;

    *hash_length = PSA_HASH_LENGTH(operation->alg);

func_done:
#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif // TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE
    (void) operation;
    (void) hash;
    (void) hash_length;

    return PSA_ERROR_BAD_STATE;
}

psa_status_t mbedtls_test_transparent_hash_clone(
    const mbedtls_transparent_test_driver_hash_operation_t *source_operation,
    mbedtls_transparent_test_driver_hash_operation_t *target_operation)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    target_operation->ctx.rsip_sha = source_operation->ctx.rsip_sha;
    target_operation->alg = source_operation->alg;

    return PSA_SUCCESS;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    (void) source_operation;
    (void) target_operation;

    return PSA_ERROR_BAD_STATE;
}

psa_status_t mbedtls_test_transparent_hash_abort(
    mbedtls_transparent_test_driver_hash_operation_t *operation)
{
    (void) operation;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    return PSA_SUCCESS;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    return PSA_ERROR_BAD_STATE;
}

psa_status_t mbedtls_test_transparent_hash_compute(
    psa_algorithm_t alg,
    const uint8_t *input, size_t input_length,
    uint8_t *hash, size_t hash_size, size_t *hash_length)
{
    (void) hash_size;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    fsp_err_t rsip_ret = FSP_ERR_ASSERTION;
    rsip_hash_type_t hash_type;

    if(alg == PSA_ALG_SHA_256) {
        hash_type = RSIP_HASH_TYPE_SHA256;
    } else {
        TLS_MBEDTLS_DEBUG_MSG(1, ("invalid hash type is selected."));
        return PSA_ERROR_INVALID_ARGUMENT;
    }

#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */

    rsip_ret = R_RSIP_SHA_Compute(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        hash_type,
                        input,
                        input_length,
                        hash);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_SHA_Compute is called: %d", rsip_ret));
    if (FSP_SUCCESS == rsip_ret)
        *hash_length = PSA_HASH_LENGTH(alg);

#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif // TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE
    (void) alg;
    (void) input;
    (void) input_length;
    (void) hash;
    (void) hash_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_cipher_encrypt(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *iv,
    size_t iv_length,
    const uint8_t *input,
    size_t input_length,
    uint8_t *output,
    size_t output_size,
    size_t *output_length)
{
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;
    (void) iv;
    (void) iv_length;
    (void) input;
    (void) input_length;
    (void) output;
    (void) output_size;
    (void) output_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_cipher_decrypt(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *input,
    size_t input_length,
    uint8_t *output,
    size_t output_size,
    size_t *output_length)
{
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;
    (void) input;
    (void) input_length;
    (void) output;
    (void) output_size;
    (void) output_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_signature_sign_message(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *input,
    size_t input_length,
    uint8_t *signature,
    size_t signature_size,
    size_t *signature_length)
{
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;
    (void) input;
    (void) input_length;
    (void) signature;
    (void) signature_size;
    (void) signature_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_signature_verify_message(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *input,
    size_t input_length,
    const uint8_t *signature,
    size_t signature_length)
{
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;
    (void) input;
    (void) input_length;
    (void) signature;
    (void) signature_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_aead_encrypt_setup(
    mbedtls_transparent_test_driver_aead_operation_t *operation,
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer, size_t key_buffer_size,
    psa_algorithm_t alg)
{
    (void) operation;
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_aead_decrypt_setup(
    mbedtls_transparent_test_driver_aead_operation_t *operation,
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer, size_t key_buffer_size,
    psa_algorithm_t alg)
{
    (void) operation;
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_aead_set_nonce(
    mbedtls_transparent_test_driver_aead_operation_t *operation,
    const uint8_t *nonce,
    size_t nonce_length)
{
    (void) operation;
    (void) nonce;
    (void) nonce_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_aead_set_lengths(
    mbedtls_transparent_test_driver_aead_operation_t *operation,
    size_t ad_length,
    size_t plaintext_length)
{
    (void) operation;
    (void) ad_length;
    (void) plaintext_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_aead_update_ad(
    mbedtls_transparent_test_driver_aead_operation_t *operation,
    const uint8_t *input,
    size_t input_length)
{
    (void) operation;
    (void) input;
    (void) input_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_aead_update(
    mbedtls_transparent_test_driver_aead_operation_t *operation,
    const uint8_t *input,
    size_t input_length,
    uint8_t *output,
    size_t output_size,
    size_t *output_length)
{
    (void) operation;
    (void) input;
    (void) input_length;
    (void) output;
    (void) output_size;
    (void) output_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_aead_finish(
    mbedtls_transparent_test_driver_aead_operation_t *operation,
    uint8_t *ciphertext,
    size_t ciphertext_size,
    size_t *ciphertext_length,
    uint8_t *tag,
    size_t tag_size,
    size_t *tag_length)
{
    (void) operation;
    (void) ciphertext;
    (void) ciphertext_size;
    (void) ciphertext_length;
    (void) tag;
    (void) tag_size;
    (void) tag_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_aead_verify(
    mbedtls_transparent_test_driver_aead_operation_t *operation,
    uint8_t *plaintext,
    size_t plaintext_size,
    size_t *plaintext_length,
    const uint8_t *tag,
    size_t tag_length)
{
    (void) operation;
    (void) plaintext;
    (void) plaintext_size;
    (void) plaintext_length;
    (void) tag;
    (void) tag_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_aead_abort(
    mbedtls_transparent_test_driver_aead_operation_t *operation)
{
    (void) operation;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_pms_generate(
                        const uint8_t *key_buffer,
                        size_t key_buffer_size,
                        size_t *key_length)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    fsp_err_t rsip_ret = FSP_ERR_ASSERTION;

    if (TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_KDFHMAC_PMS > key_buffer_size)
        return PSA_ERROR_INVALID_ARGUMENT;

#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */
    rsip_wrapped_key_t wrapped_premaster_secret = {RSIP_KEY_TYPE_KDF_HMAC_SHA256,
                                                   (void *)key_buffer};

    rsip_ret = R_RSIP_TLS12_RSAPremasterSecretGenerate(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        &wrapped_premaster_secret);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_TLS12_RSAPremasterSecretGenerate is called: %d", rsip_ret));
    if (FSP_SUCCESS == rsip_ret)
        *key_length = TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_KDFHMAC_PMS;

#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif // TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE
    (void) key_buffer;
    (void) key_buffer_size;
    (void) key_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_asymmetric_encrypt(
    const psa_key_attributes_t *attributes, const uint8_t *key_buffer,
    size_t key_buffer_size, psa_algorithm_t alg, const uint8_t *input,
    size_t input_length, const uint8_t *salt, size_t salt_length,
    uint8_t *output, size_t output_size, size_t *output_length)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    fsp_err_t rsip_ret = FSP_ERR_ASSERTION;

#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */
    rsip_wrapped_key_t wrapped_pubkey = {RSIP_KEY_TYPE_RSA_2048_PUBLIC,
                                         (void *)key_buffer};

    rsip_wrapped_key_t wrapped_premaster_secret = {RSIP_KEY_TYPE_KDF_HMAC_SHA256,
                                                   (void *)input};

    rsip_ret = R_RSIP_TLS12_RSAPremasterSecretEncrypt(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        &wrapped_pubkey,
                        &wrapped_premaster_secret,
                        output);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_TLS12_RSAPremasterSecretEncrypt is called: %d", rsip_ret));
    if (FSP_SUCCESS == rsip_ret)
        *output_length = 256;

#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif // TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;
    (void) input;
    (void) input_length;
    (void) salt;
    (void) salt_length;
    (void) output;
    (void) output_size;
    (void) output_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_asymmetric_decrypt(
    const psa_key_attributes_t *attributes, const uint8_t *key_buffer,
    size_t key_buffer_size, psa_algorithm_t alg, const uint8_t *input,
    size_t input_length, const uint8_t *salt, size_t salt_length,
    uint8_t *output, size_t output_size, size_t *output_length)
{
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    fsp_err_t rsip_ret = FSP_ERR_ASSERTION;

    if (TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_KDFHMAC_PMS > output_size)
        return PSA_ERROR_INVALID_ARGUMENT;

#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_lock(&g_mutex_rsip) != 0)
        return PSA_ERROR_GENERIC_ERROR;
#endif /* MBEDTLS_THREADING_C */
    rsip_wrapped_key_t wrapped_prikey = {RSIP_KEY_TYPE_RSA_2048_PRIVATE,
                                         (void *)key_buffer};

    rsip_wrapped_key_t wrapped_premaster_secret = {RSIP_KEY_TYPE_KDF_HMAC_SHA256,
                                                   output};

    rsip_ret = R_RSIP_TLS12_RSAPremasterSecretDecrypt(
                        (rsip_ctrl_t*)&g_p_ctrl,
                        &wrapped_prikey,
                        input,
                        &wrapped_premaster_secret);
    TLS_MBEDTLS_DEBUG_MSG(1, ("R_RSIP_TLS12_RSAPremasterSecretDecrypt is called: %d", rsip_ret));
    if (FSP_SUCCESS == rsip_ret)
        *output_length = TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_KDFHMAC_PMS;

#if defined(MBEDTLS_THREADING_C)
    mbedtls_mutex_unlock(&g_mutex_rsip);
#endif /* MBEDTLS_THREADING_C */

    return (FSP_SUCCESS == rsip_ret) ? PSA_SUCCESS : PSA_ERROR_GENERIC_ERROR;
#endif // TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;
    (void) input;
    (void) input_length;
    (void) salt;
    (void) salt_length;
    (void) output;
    (void) output_size;
    (void) output_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_transparent_mac_compute(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *input,
    size_t input_length,
    uint8_t *mac,
    size_t mac_size,
    size_t *mac_length)
{
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;
    (void) input;
    (void) input_length;
    (void) mac;
    (void) mac_size;
    (void) mac_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

/*
 * opaque versions, to do
 */
psa_status_t mbedtls_test_opaque_init(void)
{
    return PSA_SUCCESS;
}

void mbedtls_test_opaque_free(void)
{
    return;
}

size_t mbedtls_test_opaque_size_function(
    const psa_key_type_t key_type,
    const size_t key_bits)
{
    (void) key_type;
    (void) key_bits;

    return 0;
}

psa_status_t mbedtls_test_opaque_generate_key(
    const psa_key_attributes_t *attributes,
    uint8_t *key, size_t key_size, size_t *key_length)
{
    (void) attributes;
    (void) key;
    (void) key_size;
    (void) key_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_import_key(
    const psa_key_attributes_t *attributes,
    const uint8_t *data,
    size_t data_length,
    uint8_t *key_buffer,
    size_t key_buffer_size,
    size_t *key_buffer_length,
    size_t *bits)
{
    (void) attributes;
    (void) data;
    (void) data_length;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) key_buffer_length;
    (void) bits;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_export_key(
    const psa_key_attributes_t *attributes,
    const uint8_t *key, size_t key_length,
    uint8_t *data, size_t data_size, size_t *data_length)
{
    (void) attributes;
    (void) key;
    (void) key_length;
    (void) data;
    (void) data_size;
    (void) data_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_export_public_key(
    const psa_key_attributes_t *attributes,
    const uint8_t *key, size_t key_length,
    uint8_t *data, size_t data_size, size_t *data_length)
{
    (void) attributes;
    (void) key;
    (void) key_length;
    (void) data;
    (void) data_size;
    (void) data_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_get_builtin_key(
    psa_drv_slot_number_t slot_number,
    psa_key_attributes_t *attributes,
    uint8_t *key_buffer, size_t key_buffer_size, size_t *key_buffer_length)
{
    (void) slot_number;
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) key_buffer_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_copy_key(
    psa_key_attributes_t *attributes,
    const uint8_t *source_key, size_t source_key_length,
    uint8_t *key_buffer, size_t key_buffer_size, size_t *key_buffer_length)
{
    (void) attributes;
    (void) source_key;
    (void) source_key_length;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) key_buffer_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_cipher_encrypt(
    const psa_key_attributes_t *attributes,
    const uint8_t *key, size_t key_length,
    psa_algorithm_t alg,
    const uint8_t *iv, size_t iv_length,
    const uint8_t *input, size_t input_length,
    uint8_t *output, size_t output_size, size_t *output_length)
{
    (void) attributes;
    (void) key;
    (void) key_length;
    (void) alg;
    (void) iv;
    (void) iv_length;
    (void) input;
    (void) input_length;
    (void) output;
    (void) output_size;
    (void) output_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_cipher_decrypt(
    const psa_key_attributes_t *attributes,
    const uint8_t *key, size_t key_length,
    psa_algorithm_t alg,
    const uint8_t *input, size_t input_length,
    uint8_t *output, size_t output_size, size_t *output_length)
{
    (void) attributes;
    (void) key;
    (void) key_length;
    (void) alg;
    (void) input;
    (void) input_length;
    (void) output;
    (void) output_size;
    (void) output_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_cipher_encrypt_setup(
    mbedtls_opaque_test_driver_cipher_operation_t *operation,
    const psa_key_attributes_t *attributes,
    const uint8_t *key, size_t key_length,
    psa_algorithm_t alg)
{
    (void) operation;
    (void) attributes;
    (void) key;
    (void) key_length;
    (void) alg;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_cipher_decrypt_setup(
    mbedtls_opaque_test_driver_cipher_operation_t *operation,
    const psa_key_attributes_t *attributes,
    const uint8_t *key, size_t key_length,
    psa_algorithm_t alg)
{
    (void) operation;
    (void) attributes;
    (void) key;
    (void) key_length;
    (void) alg;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_cipher_abort(
    mbedtls_opaque_test_driver_cipher_operation_t *operation)
{
    (void) operation;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_cipher_set_iv(
    mbedtls_opaque_test_driver_cipher_operation_t *operation,
    const uint8_t *iv,
    size_t iv_length)
{
    (void) operation;
    (void) iv;
    (void) iv_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_cipher_update(
    mbedtls_opaque_test_driver_cipher_operation_t *operation,
    const uint8_t *input,
    size_t input_length,
    uint8_t *output,
    size_t output_size,
    size_t *output_length)
{
    (void) operation;
    (void) input;
    (void) input_length;
    (void) output;
    (void) output_size;
    (void) output_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_cipher_finish(
    mbedtls_opaque_test_driver_cipher_operation_t *operation,
    uint8_t *output,
    size_t output_size,
    size_t *output_length)
{
    (void) operation;
    (void) output;
    (void) output_size;
    (void) output_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_signature_sign_message(
    const psa_key_attributes_t *attributes,
    const uint8_t *key,
    size_t key_length,
    psa_algorithm_t alg,
    const uint8_t *input,
    size_t input_length,
    uint8_t *signature,
    size_t signature_size,
    size_t *signature_length)
{
    (void) attributes;
    (void) key;
    (void) key_length;
    (void) alg;
    (void) input;
    (void) input_length;
    (void) signature;
    (void) signature_size;
    (void) signature_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_signature_verify_message(
    const psa_key_attributes_t *attributes,
    const uint8_t *key,
    size_t key_length,
    psa_algorithm_t alg,
    const uint8_t *input,
    size_t input_length,
    const uint8_t *signature,
    size_t signature_length)
{
    (void) attributes;
    (void) key;
    (void) key_length;
    (void) alg;
    (void) input;
    (void) input_length;
    (void) signature;
    (void) signature_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_signature_sign_hash(
    const psa_key_attributes_t *attributes,
    const uint8_t *key, size_t key_length,
    psa_algorithm_t alg,
    const uint8_t *hash, size_t hash_length,
    uint8_t *signature, size_t signature_size, size_t *signature_length)
{
    (void) attributes;
    (void) key;
    (void) key_length;
    (void) alg;
    (void) hash;
    (void) hash_length;
    (void) signature;
    (void) signature_size;
    (void) signature_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_signature_verify_hash(
    const psa_key_attributes_t *attributes,
    const uint8_t *key, size_t key_length,
    psa_algorithm_t alg,
    const uint8_t *hash, size_t hash_length,
    const uint8_t *signature, size_t signature_length)
{
    (void) attributes;
    (void) key;
    (void) key_length;
    (void) alg;
    (void) hash;
    (void) hash_length;
    (void) signature;
    (void) signature_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_key_agreement(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *peer_key,
    size_t peer_key_length,
    uint8_t *shared_secret,
    size_t shared_secret_size,
    size_t *shared_secret_length)
{
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;
    (void) peer_key;
    (void) peer_key_length;
    (void) shared_secret;
    (void) shared_secret_size;
    (void) shared_secret_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_asymmetric_encrypt(
    const psa_key_attributes_t *attributes, const uint8_t *key,
    size_t key_length, psa_algorithm_t alg, const uint8_t *input,
    size_t input_length, const uint8_t *salt, size_t salt_length,
    uint8_t *output, size_t output_size, size_t *output_length)
{
    (void) attributes;
    (void) key;
    (void) key_length;
    (void) alg;
    (void) input;
    (void) input_length;
    (void) salt;
    (void) salt_length;
    (void) output;
    (void) output_size;
    (void) output_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_asymmetric_decrypt(
    const psa_key_attributes_t *attributes, const uint8_t *key,
    size_t key_length, psa_algorithm_t alg, const uint8_t *input,
    size_t input_length, const uint8_t *salt, size_t salt_length,
    uint8_t *output, size_t output_size, size_t *output_length)
{
    (void) attributes;
    (void) key;
    (void) key_length;
    (void) alg;
    (void) input;
    (void) input_length;
    (void) salt;
    (void) salt_length;
    (void) output;
    (void) output_size;
    (void) output_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_mac_compute(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *input,
    size_t input_length,
    uint8_t *mac,
    size_t mac_size,
    size_t *mac_length)
{
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;
    (void) input;
    (void) input_length;
    (void) mac;
    (void) mac_size;
    (void) mac_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_mac_sign_setup(
    mbedtls_opaque_test_driver_mac_operation_t *operation,
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg)
{
    (void) operation;
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_mac_verify_setup(
    mbedtls_opaque_test_driver_mac_operation_t *operation,
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg)
{
    (void) operation;
    (void) attributes;
    (void) key_buffer;
    (void) key_buffer_size;
    (void) alg;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_mac_update(
    mbedtls_opaque_test_driver_mac_operation_t *operation,
    const uint8_t *input,
    size_t input_length)
{
    (void) operation;
    (void) input;
    (void) input_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_mac_sign_finish(
    mbedtls_opaque_test_driver_mac_operation_t *operation,
    uint8_t *mac,
    size_t mac_size,
    size_t *mac_length)
{
    (void) operation;
    (void) mac;
    (void) mac_size;
    (void) mac_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_mac_verify_finish(
    mbedtls_opaque_test_driver_mac_operation_t *operation,
    const uint8_t *mac,
    size_t mac_length)
{
    (void) operation;
    (void) mac;
    (void) mac_length;

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t mbedtls_test_opaque_mac_abort(
    mbedtls_opaque_test_driver_mac_operation_t *operation)
{
    (void) operation;

    return PSA_ERROR_NOT_SUPPORTED;
}
#endif /* PSA_CRYPTO_ACCELERATOR_DRIVER_PRESENT && PSA_CRYPTO_DRIVER_TEST */
