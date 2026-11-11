/**
 * \file certs.h
 *
 * \brief Sample certificates and DHM parameters for testing
 */
/*
 *  Copyright The Mbed TLS Contributors
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
#ifndef MBEDTLS_CERTS_H
#define MBEDTLS_CERTS_H

#include "mbedtls/build_info.h"
#include "common.h"
#include "tlsapp_mbedtls_config.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
#define TLSAPP_MBEDTLS_FLASH_ROOTCA_CERT_SIZE      (450 + 14)
#define TLSAPP_MBEDTLS_FLASH_ROOTCA_CERT_MAC_SIZE  (48)
#define TLSAPP_MBEDTLS_ROOTCA_KEY_PRIKEYTYPE       TLSAPP_MBEDTLS_RSIP_IMPCRTKEY_ECP256

#if (CIPHERSUITE_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 == TLSAPP_MBEDTLS_ENABLED_CIPHERSUITE) || \
    (CIPHERSUITE_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 == TLSAPP_MBEDTLS_ENABLED_CIPHERSUITE)
/* If the encryption algorithm used for authentication is ECDSA. */
/* ECC certificate and key size and type definitions. */
#define TLSAPP_MBEDTLS_FLASH_OWN_CERT_SIZE         (359)
#define TLSAPP_MBEDTLS_FLASH_OWN_PRIKEY_SIZE       (64)
#define TLSAPP_MBEDTLS_OWN_KEY_PRIKEYTYPE          TLSAPP_MBEDTLS_RSIP_IMPCRTKEY_ECP256

#else /* TLSAPP_MBEDTLS_ENABLED_CIPHERSUITE */
/* If the encryption algorithm used for authentication is RSA. */
/* RSA certificate and key size and type definitions. */
#define TLSAPP_MBEDTLS_FLASH_OWN_CERT_SIZE         (597)
#define TLSAPP_MBEDTLS_FLASH_OWN_PRIKEY_SIZE       (544)
#define TLSAPP_MBEDTLS_OWN_KEY_PRIKEYTYPE          TLSAPP_MBEDTLS_RSIP_IMPCRTKEY_RSA2K

#endif /* TLSAPP_MBEDTLS_ENABLED_CIPHERSUITE */

#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */

/* List of all DER-encoded CA certificates, terminated by NULL */
extern const unsigned char *tlsapp_mbedtls_test_cas_der[];
extern const size_t tlsapp_mbedtls_test_cas_derlen[];

/*
 * CA test certificates
 */
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
extern unsigned char tlsapp_mbedtls_test_cacrt_ec_der[TLSAPP_MBEDTLS_FLASH_ROOTCA_CERT_SIZE];
extern unsigned char tlsapp_mbedtls_test_cacrt_ec_mac[TLSAPP_MBEDTLS_FLASH_ROOTCA_CERT_MAC_SIZE];
#else /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
extern unsigned char tlsapp_mbedtls_test_cacrt_ec_der[];
extern const unsigned char tlsapp_mbedtls_test_cakey_ec_der[];
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */

/*
 * Own test certificates
 */
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
extern unsigned char tlsapp_mbedtls_test_owncrt_der[TLSAPP_MBEDTLS_FLASH_OWN_CERT_SIZE];
extern unsigned char tlsapp_mbedtls_test_ownkey_der[TLSAPP_MBEDTLS_FLASH_OWN_PRIKEY_SIZE];
#else /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
extern const unsigned char tlsapp_mbedtls_test_owncrt_der[];
extern const unsigned char tlsapp_mbedtls_test_ownkey_der[];
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
extern const size_t tlsapp_mbedtls_test_owncrt_der_len;
extern const size_t tlsapp_mbedtls_test_ownkey_der_len;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
void tlsapp_mbedtls_setup_certinfo_from_flash(void);
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* certs.h */
