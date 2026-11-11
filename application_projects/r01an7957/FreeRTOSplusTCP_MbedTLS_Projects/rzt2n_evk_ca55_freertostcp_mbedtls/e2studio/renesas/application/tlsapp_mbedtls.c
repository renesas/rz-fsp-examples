/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/
/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "tlsapp_mbedtls.h"

/* mbedTLS includes */
#if !defined( MBEDTLS_CONFIG_FILE )
    #include "mbedtls/mbedtls_config.h"
#else
    #include MBEDTLS_CONFIG_FILE
#endif
#include "mbedtls/debug.h"
#include "entropy_poll.h"
#include "pk_internal.h"
#include "certs.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define TLSAPP_MBEDTLS_HTTP_GET_REQUEST         "GET / HTTP/1.0\r\n\r\n"
#define TLSAPP_MBEDTLS_HTTP_GET_RESPONSE        "HTTP/1.0 200 OK\r\n" \
                                                "Content-Type: text/html\r\n\r\n" \
                                                "<h2>Mbed TLS Test</h2>\r\n" \
                                                "<p>Successful connection using: %s</p>\r\n"
#ifdef MBEDTLS_DEBUG_C
#define TLSAPP_MBEDTLS_DEBUGPRINT_LOGLVL        0
#endif

#if CIPHERSUITE_TLS_RSA_WITH_AES_128_CBC_SHA256 == TLSAPP_MBEDTLS_ENABLED_CIPHERSUITE
#define TLSAPP_MBEDTLS_CIPHERSUITE_NAME    "TLS-RSA-WITH-AES-128-CBC-SHA256"
#elif CIPHERSUITE_TLS_RSA_WITH_AES_256_CBC_SHA256 == TLSAPP_MBEDTLS_ENABLED_CIPHERSUITE
#define TLSAPP_MBEDTLS_CIPHERSUITE_NAME    "TLS-RSA-WITH-AES-256-CBC-SHA256"
#elif CIPHERSUITE_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 == TLSAPP_MBEDTLS_ENABLED_CIPHERSUITE
#define TLSAPP_MBEDTLS_CIPHERSUITE_NAME    "TLS-ECDHE-ECDSA-WITH-AES-128-CBC-SHA256"
#elif CIPHERSUITE_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256 == TLSAPP_MBEDTLS_ENABLED_CIPHERSUITE
#define TLSAPP_MBEDTLS_CIPHERSUITE_NAME    "TLS-ECDHE-RSA-WITH-AES-128-CBC-SHA256"
#elif CIPHERSUITE_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 == TLSAPP_MBEDTLS_ENABLED_CIPHERSUITE
#define TLSAPP_MBEDTLS_CIPHERSUITE_NAME    "TLS-ECDHE-ECDSA-WITH-AES-128-GCM-SHA256"
#elif CIPHERSUITE_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 == TLSAPP_MBEDTLS_ENABLED_CIPHERSUITE
#define TLSAPP_MBEDTLS_CIPHERSUITE_NAME    "TLS-ECDHE-RSA-WITH-AES-128-GCM-SHA256"
#endif

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void tlsapp_mbedtls_freertos_mutex_init( mbedtls_threading_mutex_t * mutex );
static void tlsapp_mbedtls_freertos_mutex_free( mbedtls_threading_mutex_t * mutex );
static int tlsapp_mbedtls_freertos_mutex_lock( mbedtls_threading_mutex_t * mutex );
static int tlsapp_mbedtls_freertos_mutex_unlock( mbedtls_threading_mutex_t * mutex );
static int tlsapp_mbedtls_rng_get( void *p_rng, unsigned char *output, size_t output_len );
static int tlsapp_mbedtls_setcert( tlsapp_mbedtls_ep_ctx_t* p_epctx );
static void tlsapp_mbedtls_freecert( tlsapp_mbedtls_ep_ctx_t* p_epctx );
#ifdef MBEDTLS_DEBUG_C
static void tlsapp_mbedtls_debugprint( void * ctx, int lvl,
                                       const char * file, int linestr, const char * str );
#endif /* ifdef MBEDTLS_DEBUG_C */

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/
/**
 * @brief Generate random value
 */
int mbedtls_hardware_poll( void *data, unsigned char *output, size_t len, size_t *olen )
{
    ( void ) data;

    tlsapp_mbedtls_rng_get(data, output, len);

    *olen = len;

    return 0;
}

/**
 * @brief FreeRTOS platform calloc function.
 */
void * tlsapp_mbedtls_calloc( size_t nmemb, size_t size )
{
    void * pvnew = pvPortMalloc( nmemb * size );

    if( NULL != pvnew )
    {
        memset( pvnew, 0, nmemb * size );
    }

    return pvnew;
}

/**
 * @brief Mbed TLS open.
 */
int tlsapp_mbedtls_open( tlsapp_mbedtls_ep_ctx_t* p_epctx )
{
    int ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;

    mbedtls_ssl_init( &(p_epctx->ssl) );
    mbedtls_ssl_config_init( &(p_epctx->conf) );

    /* Configure mbedtls to use FreeRTOS mutexes. */
    mbedtls_threading_set_alt( tlsapp_mbedtls_freertos_mutex_init,
                               tlsapp_mbedtls_freertos_mutex_free,
                               tlsapp_mbedtls_freertos_mutex_lock,
                               tlsapp_mbedtls_freertos_mutex_unlock );

#if defined(PSA_CRYPTO_ACCELERATOR_DRIVER_PRESENT) && defined(PSA_CRYPTO_DRIVER_TEST)
    /* Open HWACC driver */
    if (PSA_SUCCESS != mbedtls_test_transparent_open())
        goto exit;
#endif /* PSA_CRYPTO_ACCELERATOR_DRIVER_PRESENT && PSA_CRYPTO_DRIVER_TEST */

    /* PSA Crypto module initialization */
    psa_crypto_init();

    /* Register RNG function into Mbed TLS */
    mbedtls_ssl_conf_rng( &(p_epctx->conf), tlsapp_mbedtls_rng_get, NULL );

    /* Set the socket callbacks. */
    mbedtls_ssl_set_bio( &(p_epctx->ssl), p_epctx,
                         tlsapp_mbedtls_send_msg_cb,
                         tlsapp_mbedtls_recv_msg_cb,
                         NULL );

    /* Set default config */
#if defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
    ret = mbedtls_ssl_config_defaults( &(p_epctx->conf), MBEDTLS_SSL_IS_SERVER,
                                       MBEDTLS_SSL_TRANSPORT_STREAM,
                                       MBEDTLS_SSL_PRESET_DEFAULT );
#else /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
    ret = mbedtls_ssl_config_defaults( &(p_epctx->conf), MBEDTLS_SSL_IS_CLIENT,
                                       MBEDTLS_SSL_TRANSPORT_STREAM,
                                       MBEDTLS_SSL_PRESET_DEFAULT );
#endif /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
    if( ret != 0 )
        goto exit;

    /* Certificate validation is required. */
    mbedtls_ssl_conf_authmode( &(p_epctx->conf), MBEDTLS_SSL_VERIFY_REQUIRED );

    /* Set a ciphersuite to agree */
    p_epctx->ciphersuite[0] = mbedtls_ssl_get_ciphersuite_id( TLSAPP_MBEDTLS_CIPHERSUITE_NAME );
    p_epctx->ciphersuite[1] = 0;
    mbedtls_ssl_conf_ciphersuites( &(p_epctx->conf), p_epctx->ciphersuite );

    /* Set Mbed TLS configuration into ssl context*/
    ret = mbedtls_ssl_setup( &(p_epctx->ssl), &(p_epctx->conf) );
    if( ret != 0 )
        goto exit;

    /* Set certification */
    ret = tlsapp_mbedtls_setcert( p_epctx );
    if( ret != 0 )
        goto exit;

exit:
    return ret;
}

/**
 * @brief Mbed TLS close.
 */
void tlsapp_mbedtls_close( tlsapp_mbedtls_ep_ctx_t* p_epctx )
{
    /* Send Close Notify Alert message */
    mbedtls_ssl_close_notify( &(p_epctx->ssl) );
    vTaskDelay (200);

    /* Free Certificate info */
    tlsapp_mbedtls_freecert( p_epctx );

    /* Free SSL Context */
    mbedtls_ssl_free( &(p_epctx->ssl) );

    /* Free SSL Config */
    mbedtls_ssl_config_free( &(p_epctx->conf) );

    /* Free PSA Crypto module */
    mbedtls_psa_crypto_free();

#if defined(PSA_CRYPTO_ACCELERATOR_DRIVER_PRESENT) && defined(PSA_CRYPTO_DRIVER_TEST)
    /* Close HWACC driver */
    mbedtls_test_transparent_close();
#endif /* PSA_CRYPTO_ACCELERATOR_DRIVER_PRESENT && PSA_CRYPTO_DRIVER_TEST */

    p_epctx->hs_success = false;
}

/**
 * @brief Mbed TLS certificate setup.
 */
int tlsapp_mbedtls_setcert( tlsapp_mbedtls_ep_ctx_t* p_epctx )
{
    int                         ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    tlsapp_mbedtls_ep_cert_t    *ca_cert = NULL;
    tlsapp_mbedtls_ep_cert_t    *own_cert = NULL;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    /* setup cert and key information from flash */
    tlsapp_mbedtls_setup_certinfo_from_flash();
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */

    /* Root CA Certification */
    ca_cert = &(p_epctx->ca_cert);
    mbedtls_x509_crt_init( &(ca_cert->cert) );

    /* Own Certification */
#if defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
    own_cert = &(p_epctx->svr_cert);
#else /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
    own_cert = &(p_epctx->cli_cert);
#endif /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
    mbedtls_x509_crt_init( &(own_cert->cert) );
    mbedtls_pk_init( &(own_cert->pkey) );

    /* Load the trusted CA(Root CA and Intermediate CA, if any) */
    for( int i = 0; tlsapp_mbedtls_test_cas_der[i] != NULL; i++ )
    {
        ret = mbedtls_x509_crt_parse_der( &(ca_cert->cert),
                    (const unsigned char *) tlsapp_mbedtls_test_cas_der[i],
                    tlsapp_mbedtls_test_cas_derlen[i] );
        if( ret != 0 )
            goto exit;
    }

    /* Load the own endpoint certificate and secret key */
    ret = mbedtls_x509_crt_parse( &(own_cert->cert),
                                  (const unsigned char *) tlsapp_mbedtls_test_owncrt_der,
                                  tlsapp_mbedtls_test_owncrt_der_len );
    if( ret != 0 )
        goto exit;

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    /* verify root ca cert */
    if (mbedtls_pk_can_do(&ca_cert->cert.pk, MBEDTLS_PK_ECKEY)) {
        /* MBEDTLS_PK_RSA */
        mbedtls_ecp_keypair *eck = mbedtls_pk_ec_rw(ca_cert->cert.pk);
        eck->publen = TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_SECP256R1_PUB;
        eck->pubbuf = mbedtls_calloc(1, eck->publen);

        /* set wrap publik key */
        ret = mbedtls_test_transparent_rootcertkey_import(
                                ca_cert->cert.raw.p,
                                tlsapp_mbedtls_test_cacrt_ec_mac,
                                TLSAPP_MBEDTLS_ROOTCA_KEY_PRIKEYTYPE,
                                eck->pub_qxpos,
                                eck->pub_qxlen,
                                eck->pub_qypos,
                                eck->pub_qylen,
                                eck->pubbuf);
    } else {
        /* MBEDTLS_PK_RSA */
        mbedtls_rsa_context *rsactx = mbedtls_pk_rsa(ca_cert->cert.pk);
        rsactx->publen = TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_RSA2048_PUB;
        rsactx->pubbuf = mbedtls_calloc(1, rsactx->publen);

        /* set wrap publik key */
        ret = mbedtls_test_transparent_rootcertkey_import(
                                ca_cert->cert.raw.p,
                                tlsapp_mbedtls_test_cacrt_ec_mac,
                                TLSAPP_MBEDTLS_ROOTCA_KEY_PRIKEYTYPE,
                                rsactx->pub_npos,
                                rsactx->pub_nlen,
                                rsactx->pub_epos,
                                rsactx->pub_elen,
                                rsactx->pubbuf);
    }
    if( ret != 0 )
        goto exit;
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */

#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    /* load own cert private key from flash */
    const mbedtls_pk_info_t     *pk_info;

    if (mbedtls_pk_can_do(&own_cert->cert.pk, MBEDTLS_PK_ECKEY)) {
        pk_info = mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY);
        ret = mbedtls_pk_setup(&(own_cert->pkey), pk_info);
        if (0 != ret)
            goto exit;

        mbedtls_ecp_keypair *eck = mbedtls_pk_ec_rw(own_cert->pkey);
        if (NULL == eck)
            goto exit;

        mbedtls_ecp_group_load(&(eck->grp), MBEDTLS_ECP_DP_SECP256R1);

        eck->prilen = TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_SECP256R1_PRI;
        eck->pribuf = mbedtls_calloc(1, eck->prilen);
        if (NULL == eck->pribuf)
            goto exit;

        memcpy(eck->pribuf, tlsapp_mbedtls_test_ownkey_der, eck->prilen);
    } else {
        pk_info = mbedtls_pk_info_from_type(MBEDTLS_PK_RSA);
        ret = mbedtls_pk_setup(&(own_cert->pkey), pk_info);
        if (0 != ret)
            goto exit;

        mbedtls_rsa_context *rsactx = mbedtls_pk_rsa(own_cert->pkey);
        if (NULL == rsactx)
            goto exit;
        
        rsactx->prilen = rsactx->len = TLSAPP_MBEDTLS_RSIP_WRAPKEYSIZE_RSA2048_PRI;
        rsactx->pribuf = mbedtls_calloc(1, rsactx->prilen);
        if (NULL == rsactx->pribuf)
            goto exit;

        memcpy(rsactx->pribuf, tlsapp_mbedtls_test_ownkey_der, rsactx->prilen);
    }
#else /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    ret = mbedtls_pk_parse_key( &(own_cert->pkey),
                                (const unsigned char *) tlsapp_mbedtls_test_ownkey_der,
                                tlsapp_mbedtls_test_ownkey_der_len, NULL, 0,
                                tlsapp_mbedtls_rng_get, NULL );
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */

    if( ret != 0 )
        goto exit;

    /* Register CA certificates chain into Mbed TLS configuration */
    mbedtls_ssl_conf_ca_chain( &(p_epctx->conf), &(ca_cert->cert), NULL );

    /* Register Own certificate and secret key into Mbed TLS configuration */
    ret = mbedtls_ssl_conf_own_cert( &(p_epctx->conf), &(own_cert->cert),
                                     &(own_cert->pkey) );
    if( ret != 0 )
        goto exit;
    if( p_epctx->conf.key_cert == NULL )
        goto exit;

exit:
    if( ret != 0 )
    {
        tlsapp_mbedtls_freecert(p_epctx);
    }

    return ret;
}

/**
 * @brief Mbed TLS certificate free.
 */
void tlsapp_mbedtls_freecert( tlsapp_mbedtls_ep_ctx_t* p_epctx )
{
    tlsapp_mbedtls_ep_cert_t    *ca_cert = NULL;
    tlsapp_mbedtls_ep_cert_t    *own_cert = NULL;

    ca_cert = &(p_epctx->ca_cert);
#if defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
    own_cert = &(p_epctx->svr_cert);
#else /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
    own_cert = &(p_epctx->cli_cert);
#endif /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */

    mbedtls_x509_crt_free( &(ca_cert->cert) );
    mbedtls_x509_crt_free( &(own_cert->cert) );
#if defined(TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE)
    int                         ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    const mbedtls_pk_info_t     *pk_info;

    if (mbedtls_pk_can_do(&ca_cert->cert.pk, MBEDTLS_PK_ECKEY)) {
        pk_info = mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY);
        ret = mbedtls_pk_setup(&(own_cert->pkey), pk_info);
        if (0 != ret)
            goto exit;

        mbedtls_ecp_keypair *eck = mbedtls_pk_ec_rw(own_cert->pkey);
        if (NULL == eck)
            goto exit;

        mbedtls_ecp_group_load(&(eck->grp), MBEDTLS_ECP_DP_SECP256R1);

        if (NULL != eck->pribuf)
            mbedtls_free(eck->pribuf);
    } else {
        pk_info = mbedtls_pk_info_from_type(MBEDTLS_PK_RSA);
        ret = mbedtls_pk_setup(&(own_cert->pkey), pk_info);
        if (0 != ret)
            goto exit;

        mbedtls_rsa_context *rsactx = mbedtls_pk_rsa(own_cert->pkey);
        if (NULL == rsactx)
            goto exit;
        
        if (NULL != rsactx->pribuf)
            mbedtls_free(rsactx->pribuf);
    }
exit:
#endif /* TLSAPP_MBEDTLS_RSIPDRIVER_ENABLE */
    mbedtls_pk_free( &(own_cert->pkey) );
}

/**
 * @brief Mbed TLS handshake operation.
 */
int tlsapp_mbedtls_handshake( tlsapp_mbedtls_ep_ctx_t* p_epctx )
{
    int ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;

#ifdef MBEDTLS_DEBUG_C
    /* Set debug print function */
    mbedtls_ssl_conf_dbg( &(p_epctx->conf), tlsapp_mbedtls_debugprint, NULL );
    /* Set debug output level */
    mbedtls_debug_set_threshold( TLSAPP_MBEDTLS_DEBUGPRINT_LOGLVL );
#endif /* MBEDTLS_DEBUG_C */

#if !defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
    if (0 != mbedtls_ssl_set_hostname(&(p_epctx->ssl), TLSAPP_MBEDTLS_HOST_SERVER_NAME))
    {
        return TLSAPP_MBEDTLS_RET_HS_FAILED;
    }
#endif

    /* Perform TLS handshake. */
    while( 0 != (ret = mbedtls_ssl_handshake( &(p_epctx->ssl) )) )
    {
        if( (MBEDTLS_ERR_SSL_WANT_READ != ret) && (MBEDTLS_ERR_SSL_WANT_WRITE != ret) )
        {
            ret = TLSAPP_MBEDTLS_RET_HS_FAILED;
            break;
        }
    }

    /* Set handshake status */
    if ( TLSAPP_MBEDTLS_RET_SUCCESS == ret )
        p_epctx->hs_success = true;

    return ret;
}

#if defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
/**
 * @brief Receive HTTP Request.
 */
int tlsapp_mbedtls_recv_httpreq( tlsapp_mbedtls_ep_ctx_t *p_epctx )
{
    int             ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    unsigned char   buf[1024];
    int             len = sizeof(buf) - 1;

    memset(buf, 0, sizeof(buf));

    if( (ret = tlsapp_mbedtls_recv_appdata( p_epctx, buf, len )) < 0 )
    {
        printf(" failed\n  ! tlsapp_mbedtls_recv_httpreq returned %d\n\n", ret);
        if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
        {
            printf("Receive Closed Notify Alert Message.\n\n");
            return TLSAPP_MBEDTLS_RET_RCV_CLOSENOTIFY;
        }

        return TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    }

    printf(" %d bytes read\n\n%s", ret, (char *) buf);

    return TLSAPP_MBEDTLS_RET_SUCCESS;
}

/**
 * @brief Send HTTP Response.
 */
int tlsapp_mbedtls_send_httpres( tlsapp_mbedtls_ep_ctx_t *p_epctx )
{
    int             ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    unsigned char   buf[1024];
    int             len;

    /* Create and send HTTP Response message */
    len = sprintf((char *)buf, TLSAPP_MBEDTLS_HTTP_GET_RESPONSE, mbedtls_ssl_get_ciphersuite( &(p_epctx->ssl) ));
    if( (ret = tlsapp_mbedtls_send_appdata(p_epctx, buf, len)) < 0 )
    {
        printf(" failed\n  ! tlsapp_mbedtls_send_appdata returned %d\n\n", ret);
        return TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    }

    printf(" %d bytes written\n\n%s", len, (char *) buf);
    return TLSAPP_MBEDTLS_RET_SUCCESS;
}

#else /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
/**
 * @brief Send HTTP Request.
 */
int tlsapp_mbedtls_send_httpreq( tlsapp_mbedtls_ep_ctx_t *p_epctx )
{
    int             ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    unsigned char   buf[1024];
    int             len = sizeof(buf) - 1;

    /* Create and send HTTP Request message */
    len = sprintf( (char *)buf, TLSAPP_MBEDTLS_HTTP_GET_REQUEST );
    if( (ret = tlsapp_mbedtls_send_appdata(p_epctx, buf, len)) < 0 )
    {
        printf(" failed\n  ! tlsapp_mbedtls_send_appdata returned %d\n\n", ret);
        return TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    }

    printf(" %d bytes written\n\n%s", len, (char *) buf);
    return TLSAPP_MBEDTLS_RET_SUCCESS;
}

/**
 * @brief Receive HTTP Response.
 */
int tlsapp_mbedtls_recv_httpres( tlsapp_mbedtls_ep_ctx_t *p_epctx )
{
    int             ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    unsigned char   buf[1024];
    int             len = sizeof(buf) - 1;

    memset(buf, 0, sizeof(buf));

    if( (ret = tlsapp_mbedtls_recv_appdata( p_epctx, buf, len )) < 0 )
    {
        printf(" failed\n  ! tlsapp_mbedtls_recv_httpres returned %d\n\n", ret);
        if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
        {
            printf("Receive Closed Notify Alert Message.\n\n");
            return TLSAPP_MBEDTLS_RET_RCV_CLOSENOTIFY;
        }
        return TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    }

    printf(" %d bytes read\n\n%s", ret, (char *) buf);

    return TLSAPP_MBEDTLS_RET_SUCCESS;
}
#endif /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
/**
 * @brief Implementation of mbedtls_mutex_init for thread-safety.
 *
 */
static void tlsapp_mbedtls_freertos_mutex_init( mbedtls_threading_mutex_t * mutex )
{
    mutex->mutex = xSemaphoreCreateMutex();

    if( mutex->mutex != NULL )
    {
        mutex->is_valid = 1;
    }
    else
    {
        mutex->is_valid = 0;
        printf( "Failed to create mutex.\r\n" );
    }
}

/**
 * @brief Implementation of mbedtls_mutex_free for thread-safety.
 *
 */
static void tlsapp_mbedtls_freertos_mutex_free( mbedtls_threading_mutex_t * mutex )
{
    if( mutex->is_valid == 1 )
    {
        vSemaphoreDelete( mutex->mutex );
        mutex->is_valid = 0;
    }
}
 
/**
 * @brief Implementation of mbedtls_mutex_lock for thread-safety.
 *
 * @return 0 if successful, MBEDTLS_ERR_THREADING_MUTEX_ERROR if timeout,
 * MBEDTLS_ERR_THREADING_BAD_INPUT_DATA if the mutex is not valid.
 */
static int tlsapp_mbedtls_freertos_mutex_lock( mbedtls_threading_mutex_t * mutex )
{
    int ret = MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;

    if( mutex->is_valid == 1 )
    {
        if( xSemaphoreTake( mutex->mutex, portMAX_DELAY ) )
        {
            ret = 0;
        }
        else
        {
            ret = MBEDTLS_ERR_THREADING_MUTEX_ERROR;
            printf( "Failed to lock mutex.\r\n" );
        }
    }

    return ret;
}

/**
 * @brief Implementation of mbedtls_mutex_unlock for thread-safety.
 *
 * @return 0 if successful, MBEDTLS_ERR_THREADING_MUTEX_ERROR if timeout,
 * MBEDTLS_ERR_THREADING_BAD_INPUT_DATA if the mutex is not valid.
 */
static int tlsapp_mbedtls_freertos_mutex_unlock( mbedtls_threading_mutex_t * mutex )
{
    int ret = MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;

    if( mutex->is_valid == 1 )
    {
        if( xSemaphoreGive( mutex->mutex ) )
        {
            ret = 0;
        }
        else
        {
            ret = MBEDTLS_ERR_THREADING_MUTEX_ERROR;
            printf( "Failed to unlock mutex.\r\n" );
        }
    }

    return ret;
}

/**
 * @brief Implementation of RNG function.
 */
static int tlsapp_mbedtls_rng_get( void *p_rng, unsigned char *output, size_t output_len )
{
    (void)p_rng;

    int copylen = output_len;
    int use_len;
    int random;
    int size = sizeof(random);

    while( copylen > 0 )
    {
        random = rand();

        if( random == 0 )
        {
            printf("Error RAND is returning 0 at copylen 0x%x \n", copylen);
        }

        use_len = (copylen > size) ? size : copylen;

        memcpy( output, &random, use_len );
        output += use_len;
        copylen -= use_len;
    }


    return 0;
}

#ifdef MBEDTLS_DEBUG_C
/**
 * @brief Implementation of debugprint function.
 */
static void tlsapp_mbedtls_debugprint( void * ctx,
                                       int lvl,
                                       const char * file,
                                       int linestr,
                                       const char * str )
{
    /* Unused parameters. */
    ( void ) ctx;
    ( void ) file;
    ( void ) linestr;

    /* Send the debug string to the portable logger. */
    printf( "Mbed TLS: |%d| %s", lvl, str );
}
#endif /* ifdef MBEDTLS_DEBUG_C */
