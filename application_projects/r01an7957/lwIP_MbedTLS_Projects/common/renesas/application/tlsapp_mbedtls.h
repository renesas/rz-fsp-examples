/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/
/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "tlsapp_mbedtls_config.h"
#include "tlsapp_mbedtls_sockets.h"

/* mbedTLS includes */
#include "common.h"
#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#if !defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
#define TLSAPP_MBEDTLS_HOST_IPADDRESS           "192.168.1.3"
#define TLSAPP_MBEDTLS_HOST_SERVER_NAME         "renesas.com"
#endif /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
#define TLSAPP_MBEDTLS_HOST_PORTNUMBER          65000
#define TLSAPP_MBEDTLS_ACCEPTED_SOCKETSNUM      10

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/**
 * Endpoint's certificates structure for SSL/TLS communication.
 */
typedef struct st_tlsapp_mbedtls_ep_cert
{
    mbedtls_x509_crt   cert;    /** Certificate */
    mbedtls_pk_context pkey;    /** Private Key */
} tlsapp_mbedtls_ep_cert_t;

/**
 * Endpoint's context structure for SSL/TLS communication.
 */
typedef struct st_tlsapp_mbedtls_ep_ctx
{
    tlsapp_mbedtls_sock_ctx_info_t  sock_info;      /** Socket information */
    mbedtls_ssl_context             ssl;            /** Mbed TLS ssl context */
    mbedtls_ssl_config              conf;           /** Mbed TLS configuration */
    int                             ciphersuite[2]; /** Supported Cipher Suites */
    tlsapp_mbedtls_ep_cert_t        ca_cert;        /** Root CA Certificate */
#if defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
    tlsapp_mbedtls_ep_cert_t        svr_cert;       /** Server Certificate */
#else /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
    tlsapp_mbedtls_ep_cert_t        cli_cert;       /** Client Certificate */
#endif /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
    bool                            hs_success;     /** Handshake status */
} tlsapp_mbedtls_ep_ctx_t;

/**
 * Endpoint's operation return code enumeration.
 */
 typedef enum e_tlsapp_mbedtls_retcode
 {
     TLSAPP_MBEDTLS_RET_SUCCESS  = 0        ,   /** Operation Succeed */
     TLSAPP_MBEDTLS_RET_HS_FAILED           ,   /** Mbed TLS Handshake Failed */
     TLSAPP_MBEDTLS_RET_RCV_CLOSENOTIFY     ,   /** Receive Close Notify Alert Message */
     TLSAPP_MBEDTLS_RET_INTERNAL_ERROR = -1 ,   /** Internal error */
 } tlsapp_mbedtls_retcode_t;

/***********************************************************************************************************************
 * Function prototypes
 ***********************************************************************************************************************/
/* Different definition */
int tlsapp_mbedtls_send_msg_cb( void *ctx, const unsigned char *buf, size_t len );
int tlsapp_mbedtls_recv_msg_cb( void * ctx, unsigned char *buf, size_t len );
int tlsapp_mbedtls_send_appdata( tlsapp_mbedtls_ep_ctx_t *p_epctx,
                                 const unsigned char *sendbuf,
                                 size_t buf_len );
int tlsapp_mbedtls_recv_appdata( tlsapp_mbedtls_ep_ctx_t *p_epctx,
                                 unsigned char *recvbuf,
                                 size_t buf_len );

/* Common definition */
void *tlsapp_mbedtls_calloc( size_t nmemb, size_t size );
int tlsapp_mbedtls_open( tlsapp_mbedtls_ep_ctx_t* p_epctx );
void tlsapp_mbedtls_close( tlsapp_mbedtls_ep_ctx_t* p_epctx );
int tlsapp_mbedtls_handshake( tlsapp_mbedtls_ep_ctx_t* p_epctx );
#if defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
int tlsapp_mbedtls_recv_httpreq( tlsapp_mbedtls_ep_ctx_t *p_epctx );
int tlsapp_mbedtls_send_httpres( tlsapp_mbedtls_ep_ctx_t *p_epctx );
#else /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
int tlsapp_mbedtls_send_httpreq( tlsapp_mbedtls_ep_ctx_t *p_epctx );
int tlsapp_mbedtls_recv_httpres( tlsapp_mbedtls_ep_ctx_t *p_epctx );
#endif /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
