/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/
/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "tlsapp_mbedtls.h"

/* FreeRTOS includes */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/* Serial UART I/O includes */
#include "sio_char.h"
#include "hal_data.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Public function prototypes
 **********************************************************************************************************************/
void tlsapp_mbedtls_freertostcp_user_main( void );
void tlsapp_mbedtls_user_uart_cb( uart_callback_args_t * p_args );

/**********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void tlsapp_mbedtls_sciuart_open(void);
static void tlsapp_mbedtls_sciuart_close(void);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
uint8_t                     g_out_of_band_received[128];
volatile uint32_t           g_transfer_complete = 0;
volatile uint32_t           g_receive_complete  = 0;
uint32_t                    g_out_of_band_index = 0;

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Public Functions
 **********************************************************************************************************************/
/**
 * @brief Example codes for initializing and handling FreeRTOS+TCP applications.
 */
void tlsapp_mbedtls_freertostcp_user_main( void )
{
    BaseType_t              ret = pdFALSE;
    tlsapp_mbedtls_ep_ctx_t epctx;

    /* Initialization */
    memset( &epctx, 0, sizeof( epctx ) );
    mbedtls_platform_set_calloc_free( tlsapp_mbedtls_calloc, vPortFree );

    /* Open sci uart module */
    tlsapp_mbedtls_sciuart_open();

    /** Enabled the printf() function via SCI UART. */
    printf( "/** Started sample application for FreeRTOS TCP Port with Mbed TLS. **/\r\n" );

    /* Wait until TCP module is ready */
    while(FreeRTOS_IsNetworkUp() == pdFALSE){
        vTaskDelay (200);
    }

#if defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
    BaseType_t                  socret;
    Socket_t                    listen_soc;
    struct freertos_sockaddr    listen_addrinfo;
    Socket_t                    accepted_soc;
    struct freertos_sockaddr    accepted_socaddr;
    uint32_t                    accepted_soclen = sizeof( struct freertos_sockaddr );
    TickType_t                  timeout = portMAX_DELAY;

    /* Create FreeRTOS listener Socket */
    listen_soc = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );
    if( listen_soc == FREERTOS_INVALID_SOCKET )
    {
        tlsapp_mbedtls_sciuart_close();
        printf( ("FreeRTOS listener Socket is invalid.\n") );
        fflush(stdout);
        return;
    }

    /* Bind the FreeRTOS listener Socket to a host port number */
    listen_addrinfo.sin_family = FREERTOS_AF_INET;
    listen_addrinfo.sin_port = FreeRTOS_htons( TLSAPP_MBEDTLS_HOST_PORTNUMBER );
    socret = FreeRTOS_bind( listen_soc, &listen_addrinfo, sizeof(listen_addrinfo) );
    if( socret < 0 )
    {
        printf( ("Failed to bind port number to FreeRTOS listener Socket. ret:%ld\n\n"), socret );
        fflush(stdout);
        goto exit;
    }

    /* Set timeout value */
    FreeRTOS_setsockopt( listen_soc, 0, FREERTOS_SO_RCVTIMEO, &timeout, sizeof(timeout) );

    /* Change the socket state to listening */
    socret = FreeRTOS_listen( listen_soc, TLSAPP_MBEDTLS_ACCEPTED_SOCKETSNUM );
    if( socret < 0 )
    {
        printf( ("Failed to change the socket state to listening. ret:%ld\n\n"), socret );
        fflush(stdout);
        goto exit;
    }

    while (1)
    {
        /* Wait for a client request */
        accepted_soc = FreeRTOS_accept( listen_soc, &accepted_socaddr , &accepted_soclen );
        if( accepted_soc == FREERTOS_INVALID_SOCKET )
        {
            printf( ("Accepted socket is Invalid.\n\n") );
            fflush(stdout);
            goto exit;
        }else{
            epctx.sock_info.sock_hdl = accepted_soc;
            FreeRTOS_setsockopt( epctx.sock_info.sock_hdl,
                                 0,
                                 FREERTOS_SO_RCVTIMEO,
                                 &timeout,
                                 sizeof(timeout) );

            /* Mbed TLS open */
            if( (ret = tlsapp_mbedtls_open( &epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
            {
                FreeRTOS_closesocket( epctx.sock_info.sock_hdl );
                printf("Failed to initialize Mbed TLS ret:%ld\n\n", ret);
                fflush(stdout);
                goto exit;
            }

            /* Handshake operation */
            if( (ret = tlsapp_mbedtls_handshake( &epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
            {
                printf( ( "Mbed TLS handshake failed. %ld\n" ), ret );
                fflush(stdout);
                break;
            }

            /* Read the HTTP request */
            printf("  < Receive Get Request from client:\n");
            fflush(stdout);
            if( (ret = tlsapp_mbedtls_recv_httpreq( &epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
            {
                printf( ( "Failed to receive HTTP Get Response. %ld\n" ), ret );
                fflush(stdout);
                break;
            }

            /* Write the HTTP response */
            printf( ( "  > Send Get Response to client:\n" ) );
            fflush(stdout);
            if( (ret = tlsapp_mbedtls_send_httpres( &epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
            {
                printf( ( "Failed to send HTTP Get Request. %ld\n" ), ret );
                fflush(stdout);
                break;
            }
        }

        printf( "\r\n/** Successfully Finished sample application for FreeRTOS TCP Port with Mbed TLS. **/\r\n\r\n");

        /* Mbed TLS Close */
        tlsapp_mbedtls_close( &epctx );

        /* FreeRTOS Socket Close */
        FreeRTOS_closesocket( epctx.sock_info.sock_hdl );
    }

    /* Mbed TLS Close */
    tlsapp_mbedtls_close( &epctx );

    /* FreeRTOS Socket Close */
    FreeRTOS_closesocket( epctx.sock_info.sock_hdl );
exit:
    /* FreeRTOS Socket Close */
    FreeRTOS_closesocket( listen_soc );
#else /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
    struct freertos_sockaddr    addrinfo = { 0 };
    TickType_t                  timeout = pdMS_TO_TICKS( 2000ul );
    long                        retcnt = 0;

    do
    {
        /* Create a socket. */
        epctx.sock_info.sock_hdl = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );
        if( epctx.sock_info.sock_hdl == FREERTOS_INVALID_SOCKET )
        {
            tlsapp_mbedtls_sciuart_close();
            printf( ( "FreeRTOS Socket is invalid.\n" ) );
            fflush(stdout);
            return;
        }

        /* Set timeout value */
        FreeRTOS_setsockopt( epctx.sock_info.sock_hdl, 0, FREERTOS_SO_RCVTIMEO, ( void * )&timeout, sizeof( timeout ) );

        /* Connect the socket. */
        addrinfo.sin_addr = FreeRTOS_inet_addr( TLSAPP_MBEDTLS_HOST_IPADDRESS );
        addrinfo.sin_family = FREERTOS_AF_INET;
        addrinfo.sin_len = ( uint8_t ) sizeof( addrinfo );
        addrinfo.sin_port = FreeRTOS_htons( TLSAPP_MBEDTLS_HOST_PORTNUMBER );

retly:
        if( (ret = FreeRTOS_connect( epctx.sock_info.sock_hdl, &addrinfo, sizeof( struct freertos_sockaddr ) )) != 0 )
        {
            if(retcnt < 20) {
                retcnt++;
                goto retly;
            };
            FreeRTOS_closesocket( epctx.sock_info.sock_hdl );
            printf( ( "FreeRTOS_connect failed. %ld\n" ), ret );
            fflush(stdout);
            goto exit;
        }

        /* Mbed TLS open */
        if( (ret = tlsapp_mbedtls_open( &epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
        {
            FreeRTOS_closesocket( epctx.sock_info.sock_hdl );
            printf("Failed to initialize Mbed TLS ret:%ld\n\n", ret);
            fflush(stdout);
            goto exit;
        }

        /* Handshake operation */
        if( (ret = tlsapp_mbedtls_handshake( &epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
        {
            printf( ( "Mbed TLS handshake failed. %ld\n" ), ret );
            fflush(stdout);
            break;
        }

        /* Write the GET request */
        printf( ( "  > Send Get Request to server:\n" ) );
        fflush(stdout);

        if( (ret = tlsapp_mbedtls_send_httpreq( &epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
        {
            printf( ( "Failed to send HTTP Get Request. %ld\n" ), ret );
            fflush(stdout);
            break;
        }

        /* Read the HTTP response */
        printf("  < Receive Get Response from server:\n");
        fflush(stdout);

        if( (ret = tlsapp_mbedtls_recv_httpres( &epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
        {
            printf( ( "Failed to receive HTTP Get Response. %ld\n" ), ret );
            fflush(stdout);
            break;
        }

        printf( "/** Successfully Finished sample application for FreeRTOS TCP Port with Mbed TLS. **/\r\n\r\n");
    } while(0);

    /* Mbed TLS Close */
    tlsapp_mbedtls_close( &epctx );

exit:
    /* FreeRTOS Socket Close */
    FreeRTOS_closesocket( epctx.sock_info.sock_hdl );

#endif /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */

    /* Close sci uart module */
    tlsapp_mbedtls_sciuart_close();
}

/**
 * @brief sci uart callback function.
 */
void tlsapp_mbedtls_user_uart_cb( uart_callback_args_t * p_args )
{
    /* Handle the UART event */
    switch (p_args->event)
    {
        /* Received a character */
        case UART_EVENT_RX_CHAR:
        {
            /* Only put the next character in the receive buffer if there is space for it */
            if (sizeof(g_out_of_band_received) > g_out_of_band_index)
            {
                /* Write either the next one or two bytes depending on the receive data size */
                if (UART_DATA_BITS_8 >= g_uart0_cfg.data_bits)
                {
                    g_out_of_band_received[g_out_of_band_index++] = (uint8_t) p_args->data;
                }
                else
                {
                    uint16_t * p_dest = (uint16_t *) &g_out_of_band_received[g_out_of_band_index];
                    *p_dest              = (uint16_t) p_args->data;
                    g_out_of_band_index += 2;
                }
            }
            break;
        }
        /* Receive complete */
        case UART_EVENT_RX_COMPLETE:
        {
            g_receive_complete = 1;
            break;
        }
        /* Transmit complete */
        case UART_EVENT_TX_COMPLETE:
        {
            g_transfer_complete = 1;
            break;
        }
        default:
        {
        }
    }
}

/**
 * @brief Network (TCP packets) receive callback.
 */
int tlsapp_mbedtls_send_msg_cb( void *ctx, const unsigned char *buf, size_t len )
{
    tlsapp_mbedtls_ep_ctx_t *p_epctx = (tlsapp_mbedtls_ep_ctx_t*)ctx;

    return FreeRTOS_send( p_epctx->sock_info.sock_hdl,
                          buf, len,
                          p_epctx->sock_info.send_flag );
}

/**
 * @brief Network (TCP packets) receive callback.
 */
int tlsapp_mbedtls_recv_msg_cb( void * ctx, unsigned char *buf, size_t len )
{
    tlsapp_mbedtls_ep_ctx_t *p_epctx = ( tlsapp_mbedtls_ep_ctx_t *) ctx;

    return FreeRTOS_recv( p_epctx->sock_info.sock_hdl,
                          buf,
                          len,
                          p_epctx->sock_info.recv_flg );
}

/**
 * @brief Send application data.
 */
int tlsapp_mbedtls_send_appdata( tlsapp_mbedtls_ep_ctx_t *p_epctx, const unsigned char *sendbuf, size_t buf_len )
{
    int     ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    size_t  writelen = 0;

    if( ( NULL != p_epctx ) && ( true == p_epctx->hs_success ) )
    {
        while( writelen < buf_len )
        {
            ret = mbedtls_ssl_write( &(p_epctx->ssl),
                                     sendbuf + writelen,
                                     buf_len - writelen );
            if( 0 < ret )
            {
                /* Sent data, so update the tally and keep looping. */
                writelen += (size_t)ret;
            }
            else if( ( 0 == ret ) || ( -pdFREERTOS_ERRNO_ENOSPC == ret ) )
            {
                /* No data sent. The secure sockets
                 * API supports non-blocking send, so stop the loop but don't
                 * flag an error. */
                ret = 0;
                break;
            }
            else if( MBEDTLS_ERR_SSL_WANT_WRITE != ret )
            {
                /* Hard error. */
                break;
            }
        }
    }
    else
    {
        ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    }

    if( 0 <= ret )
    {
        ret = (int)writelen;
    }

    return ret;
}

/**
 * @brief Receive application data.
 */
int tlsapp_mbedtls_recv_appdata( tlsapp_mbedtls_ep_ctx_t *p_epctx, unsigned char *recvbuf, size_t buf_len )
{
    int     ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    size_t  readlen = 0;

    if( (NULL != p_epctx) && (true == p_epctx->hs_success) )
    {
        /* This routine will return however many bytes are returned from mbedtls_ssl_read
         * immediately unless MBEDTLS_ERR_SSL_WANT_READ is returned, in which case we try again. */
        do
        {
            ret = mbedtls_ssl_read( &(p_epctx->ssl),
                                        recvbuf + readlen,
                                        buf_len - readlen );
            if( ret > 0 )
            {
                /* Got data, so update the tally and keep looping. */
                readlen += ( size_t ) ret;
            }

            /* If ret == 0, then no data was received (and there is no error).
             * The secure sockets API supports non-blocking read, so stop the loop,
             * but don't flag an error. */
        } while( (ret == MBEDTLS_ERR_SSL_WANT_READ) );
    }
    else
    {
        ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    }

    if( ret >= 0 )
    {
        ret = (int)readlen;
    }

    return ret;
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
/**
 * @brief Serial I/O UART module open.
 */
static void tlsapp_mbedtls_sciuart_open()
{
    sci_uart_baud_calculation_t baud_target;
    sci_baud_setting_t baud_setting;
    fsp_err_t err = FSP_ERR_ASSERTION;

    /* Open the transfer instance with initial configuration. */
    err = R_SCI_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
    FSP_PARAMETER_NOT_USED(err);

    /* Calculate and set baudrate settings */
    baud_target.baudrate = 115200;
    baud_target.baud_rate_error_x_1000 = 5000;
#if defined(BSP_MCU_GROUP_RZN2H) || defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N)
    err = R_SCI_UART_BaudCalculate(&baud_target, SCI_UART_CLOCK_SOURCE_SCI0ASYNCCLK, &baud_setting);
#else /* BSP_MCU_GROUP_RZT2M || BSP_MCU_GROUP_RZT2L || BSP_MCU_GROUP_RZN2L */
    err = R_SCI_UART_BaudCalculate(&baud_target, SCI_UART_CLOCK_SOURCE_PCLKM, &baud_setting);
#endif
    FSP_PARAMETER_NOT_USED(err);
    err = R_SCI_UART_BaudSet(&g_uart0_ctrl, (void *) &baud_setting);
    FSP_PARAMETER_NOT_USED(err);
}

/**
 * @brief Serial I/O UART module close.
 */
static void tlsapp_mbedtls_sciuart_close()
{
    /* Close the transfer instance */
    R_SCI_UART_Close(&g_uart0_ctrl);
}
