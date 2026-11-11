/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/
/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "tlsapp_mbedtls.h"

/** For handling application task */
#include "FreeRTOS.h"
#include "queue.h"

/** User module instance APIs. */
#include "um_lwip_port_api.h"       /** API of lwIP porting module */
#include "um_serial_io_api.h"       /** API of serial output for debugging */
#include "um_common.h"

/** lwIP modules */
#include "lwip/sockets.h"
#include "lwip/errno.h"

/** Standard library */
#include "stdlib.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define TLSAPP_MBEDTLS_TLS_TASK_NAME            "TLS App task"
#define TLSAPP_MBEDTLS_EP_TASK_PRIORITY         (4)
#define TLSAPP_MBEDTLS_EP_TASK_STACK_SIZE       (16384)
#define TLSAPP_MBEDTLS_INVALID_SOCKET           (-1)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/**
 * Application control structure.
 */
typedef struct st_tlsapp_mbedtls_endpoint_ctrl
{
    /* Parameters for Task */
    TaskHandle_t    p_own_task_handle;                      /** TCP listner Task handler. */
    TaskHandle_t    p_parent_task_handle;                   /** Task handle of parent task. */
#if defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
    /* Parameters for lwIP Socket */
    int32_t         max_fd;                                 /** Maximum Socket FD value */
    uint32_t        num_of_socket;                          /** Number of Sockets */
    fd_set          fdset_listners;                         /** List of listener Sockets */
    fd_set          fdset_clients;                          /** List of client Sockets */
    fd_set          fdset_all;                              /** List of all Sockets */
#endif /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
    /* Parameters for lwIP module */
    lwip_port_netif_state_t         lwip_netif_state;       /** lwIP Netif module status */
    lwip_port_instance_t const      *p_lwip_port_instance;  /** The controller of lwIP port module */
    lwip_port_callback_args_t       callback_memory;        /** Callback memory */
    lwip_port_callback_link_node_t  callback_link_node;     /** Callback function info */
    uint8_t                         lwip_status;            /** lwIP module Status */
} tlsapp_mbedtls_endpoint_ctrl_t;

/**
 * lwIP module status enumeration.
 */
typedef enum e_tlsapp_mbedtls_sockstate
{
    TLSAPP_MBEDTLS_LWIPMODULE_DOWN = 0     ,   /** lwIP Module Down state */
    TLSAPP_MBEDTLS_LWIPMODULE_UP           ,   /** lwIP Module Up state */
} tlsapp_mbedtls_lwipmodule_state_t;

/**********************************************************************************************************************
 * Public function prototypes
 **********************************************************************************************************************/
void tlsapp_mbedtls_lwip_user_main( void );

/**********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
static void tlsapp_mbedtls_server_task( void* pvParameter );
static void tlsapp_mbedtls_lwipport_server_callback( lwip_port_callback_args_t * p_arg );
static void tlsapp_mbedtls_update_maxfd_by_delsock( tlsapp_mbedtls_endpoint_ctrl_t *p_ctrl, int delsockfd );
static usr_err_t tlsapp_mbedtls_add_listensoc( tlsapp_mbedtls_endpoint_ctrl_t * p_ctrl );
static usr_err_t tlsapp_mbedtls_remove_listensoc( tlsapp_mbedtls_endpoint_ctrl_t * p_ctrl );
static usr_err_t tlsapp_mbedtls_handle_listensoc( tlsapp_mbedtls_endpoint_ctrl_t * p_ctrl, int32_t listner_socket_fd );
static usr_err_t tlsapp_mbedtls_handle_connsoc( tlsapp_mbedtls_endpoint_ctrl_t * p_ctrl, int32_t connected_socket_fd );
static int32_t tlsapp_mbedtls_getsoc_byipadd( uint32_t ip_address, int max_fd );
static int32_t tlsapp_mbedtls_create_new_listensoc( uint32_t ip_address, uint16_t port );
#else /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
static void tlsapp_mbedtls_client_task( void* pvParameter );
static void tlsapp_mbedtls_lwipport_client_callback( lwip_port_callback_args_t * p_arg );
static int tlsapp_mbedtls_client_lwip_connect( tlsapp_mbedtls_ep_ctx_t *p_epctx, tlsapp_mbedtls_endpoint_ctrl_t * p_ctrl );
#endif /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/
/**
 * Platform module instances
 */
extern lwip_port_instance_t   const * gp_lwip_port0;

/**
 * TLS module instance.
 */
static tlsapp_mbedtls_endpoint_ctrl_t g_tlsapp_ep0_ctrl;
static tlsapp_mbedtls_endpoint_ctrl_t * gp_tlsapp_ep0_ctrl = &g_tlsapp_ep0_ctrl;

/**
 * Define the errno for lwIP BSD socket interface (EWARM only)
 */
#if defined(__ICCARM__)
int errno;
#endif

/***********************************************************************************************************************
 * Public Functions
 **********************************************************************************************************************/
/**
 * @brief Example codes for initializing and handling lwIP port module.
 */
void tlsapp_mbedtls_lwip_user_main( void )
{
    /** Status */
    usr_err_t usr_err = USR_ERR_ASSERTION;      /** User status */
    BaseType_t rtos_err = pdFALSE;    /** Rtos status */

    /** Enabled the printf() function via SCI UART. */
    USR_LOG_INFO( "/** Started sample application for lwIP Port with Mbed TLS. **/" );

    /******************************************************************************************************************
     * Startup TCP Server application.
     ******************************************************************************************************************/
    /** Set lwIP port module instance. */
    gp_tlsapp_ep0_ctrl->p_lwip_port_instance = gp_lwip_port0;
    /** Set current task handle for controlling the start of the task. */
    gp_tlsapp_ep0_ctrl->p_parent_task_handle = xTaskGetCurrentTaskHandle();

#if defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
    /** Clear socket descriptor sets */
    FD_ZERO(&gp_tlsapp_ep0_ctrl->fdset_listners);
    FD_ZERO(&gp_tlsapp_ep0_ctrl->fdset_clients);
    FD_ZERO(&gp_tlsapp_ep0_ctrl->fdset_all);
    gp_tlsapp_ep0_ctrl->max_fd = TLSAPP_MBEDTLS_INVALID_SOCKET;
#endif /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */

    /** Set callback utilities into TCPIP network interface. */
    gp_tlsapp_ep0_ctrl->callback_link_node.p_context = gp_tlsapp_ep0_ctrl;
    gp_tlsapp_ep0_ctrl->callback_link_node.p_memory = &gp_tlsapp_ep0_ctrl->callback_memory;
#if defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
    gp_tlsapp_ep0_ctrl->callback_link_node.p_func = tlsapp_mbedtls_lwipport_server_callback;
#else /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
    gp_tlsapp_ep0_ctrl->callback_link_node.p_func = tlsapp_mbedtls_lwipport_client_callback;
#endif /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
    gp_tlsapp_ep0_ctrl->callback_link_node.p_next = NULL;
    usr_err =  gp_lwip_port0->p_api->callbackAdd( gp_lwip_port0->p_ctrl, &gp_tlsapp_ep0_ctrl->callback_link_node );
    if ( USR_SUCCESS != usr_err )
    {
        USR_LOG_ERROR( "Failed to set the callback function into TCP/IP stack." );
        return;
    }
    USR_LOG_INFO( "Set the callback function into TCP/IP stack." );

    /* Initialization */
    mbedtls_platform_set_calloc_free( tlsapp_mbedtls_calloc, vPortFree );
    
#if defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
    /** Create application task. */
    rtos_err = xTaskCreate(tlsapp_mbedtls_server_task,
                           TLSAPP_MBEDTLS_TLS_TASK_NAME,
                           TLSAPP_MBEDTLS_EP_TASK_STACK_SIZE / sizeof(StackType_t),
                           gp_tlsapp_ep0_ctrl,
                           TLSAPP_MBEDTLS_EP_TASK_PRIORITY,
                           & gp_tlsapp_ep0_ctrl->p_own_task_handle);
#else /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
    /** Create application task. */
    rtos_err = xTaskCreate(tlsapp_mbedtls_client_task,
                           TLSAPP_MBEDTLS_TLS_TASK_NAME,
                           TLSAPP_MBEDTLS_EP_TASK_STACK_SIZE / sizeof(StackType_t),
                           gp_tlsapp_ep0_ctrl,
                           TLSAPP_MBEDTLS_EP_TASK_PRIORITY,
                           & gp_tlsapp_ep0_ctrl->p_own_task_handle);
#endif /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */

    if ( pdTRUE != rtos_err )
    {
        USR_LOG_ERROR( "Failed to create application task." );
        return;
    }
    USR_LOG_INFO( "Created sample application task.\r\n" );

    /** Wait for notification indicating the created task is initialized. */
    (void) ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

    /** Suspend the created task. */
    vTaskSuspend( gp_tlsapp_ep0_ctrl->p_own_task_handle );

    /** Resume the created task */
    vTaskResume( gp_tlsapp_ep0_ctrl->p_own_task_handle );

    while( 1 ) {
        /** Check the netif state */
        usr_err =  gp_lwip_port0->p_api->netifStateGet( gp_lwip_port0->p_ctrl, &gp_tlsapp_ep0_ctrl->lwip_netif_state, true );
        /** If the netif is up. */
        if( LWIP_PORT_NETIF_STATE_UP == gp_tlsapp_ep0_ctrl->lwip_netif_state ){
            break;
        }
        /** Wait 1s */
        vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }
}

/**
 * @brief Network (TCP packets) receive callback.
 */
int tlsapp_mbedtls_send_msg_cb( void *ctx, const unsigned char *buf, size_t len )
{
    tlsapp_mbedtls_ep_ctx_t *p_epctx = (tlsapp_mbedtls_ep_ctx_t*)ctx;

    return (int)lwip_send( p_epctx->sock_info.sock_hdl,
                           buf, len,
                           p_epctx->sock_info.send_flg );
}

/**
 * @brief Network (TCP packets) receive callback.
 */
int tlsapp_mbedtls_recv_msg_cb( void * ctx, unsigned char *buf, size_t len )
{
    tlsapp_mbedtls_ep_ctx_t *p_epctx = ( tlsapp_mbedtls_ep_ctx_t *) ctx;

    return (int)lwip_recv( p_epctx->sock_info.sock_hdl,
                           buf, len,
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
            else if( errno == ENOTCONN )
            {
                ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
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
        do
        {
            ret = mbedtls_ssl_read( &(p_epctx->ssl),
                                        recvbuf + readlen,
                                        buf_len - readlen );
            if( ret > 0 )
            {
                /* Got data, so update the tally and keep looping. */
                readlen += ( size_t ) ret;
                break;
            }
            else if( errno == ENOTCONN )
            {
                ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
                break;
            }
            else if( MBEDTLS_ERR_SSL_WANT_READ != ret )
            {
                /* Hard error. */
                break;
            }
        } while(1);
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
#if defined(TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER)
/**
 * @brief TCP server task handling the BSD sockets.
 */
static void tlsapp_mbedtls_server_task( void* pvParameter )
{
    /** Resolve parameter */
    tlsapp_mbedtls_endpoint_ctrl_t * p_ctrl = (tlsapp_mbedtls_endpoint_ctrl_t *) pvParameter;

    /** Notify the parenet task launch of this task. */
    xTaskNotifyGive( p_ctrl->p_parent_task_handle );

    /** fdsets for select */
    fd_set fdset_read;
    fd_set fdset_except;

    /** For scanning fd sets */
    int32_t num_sockets = 0;
    int32_t socket_fd = -1;     /** Iterator */

    for(;;)
    {
        if ( p_ctrl->num_of_socket == 0 )
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        /** Get fdsets for detecting the read and except event. */
        memcpy(&fdset_read, &p_ctrl->fdset_all, sizeof(fd_set));
        memcpy(&fdset_except, &p_ctrl->fdset_all, sizeof(fd_set));

        /** Try select to detect event */
        num_sockets = (int32_t) lwip_select( p_ctrl->max_fd + 1, &fdset_read, NULL, &fdset_except, NULL );

        /** Continue if any events are detected. */
        if ( num_sockets <= 0 )
        {
            continue;
        }

        /** Check events */
        for( socket_fd = 0; socket_fd <= p_ctrl->max_fd ; socket_fd++ )
        {
            /** Check if the socket has event. */
            if ( !FD_ISSET(socket_fd, &fdset_read) && !FD_ISSET(socket_fd, &fdset_except) )
            {
                continue;
            }

            /** Check if the socket is listener socket. */
            if ( FD_ISSET(socket_fd, &p_ctrl->fdset_listners) )
            {
                if( USR_SUCCESS != tlsapp_mbedtls_handle_listensoc(p_ctrl, socket_fd) )
                {
                    printf( "Failed to handle listener socket.\n\n" );
                    return;
                }
                continue;
            }

            /** Check if the socket is client socket. */
            if ( FD_ISSET(socket_fd, &p_ctrl->fdset_clients ) )
            {
                if( USR_SUCCESS != tlsapp_mbedtls_handle_connsoc(p_ctrl, socket_fd) )
                {
                    printf( "Failed to operate TLS connection.\n\n" );
                    return;
                }
                continue;
            }
        }
    }
}

/**
 * @brief Callback function called when the lwIP Network interface is up or down.
 */
static void tlsapp_mbedtls_lwipport_server_callback( lwip_port_callback_args_t * p_arg )
{
    /** Resolve context */
    tlsapp_mbedtls_endpoint_ctrl_t * p_ctrl = (tlsapp_mbedtls_endpoint_ctrl_t *) p_arg->p_context;

    switch ( p_arg->event )
    {
    case LWIP_PORT_CALLBACK_EVENT_NETIF_UP:

        tlsapp_mbedtls_add_listensoc(p_ctrl);

        break;

    case LWIP_PORT_CALLBACK_EVENT_NETIF_DOWN:

        tlsapp_mbedtls_remove_listensoc(p_ctrl);

        break;
    }
}

/**
 * @brief Update max_fd when deleting socket from list
 */
static void tlsapp_mbedtls_update_maxfd_by_delsock( tlsapp_mbedtls_endpoint_ctrl_t *p_ctrl, int delsockfd )
{
    /** Update max_fd */
    if( p_ctrl->max_fd == delsockfd )
    {
        int socket_fd = -1;

        for( socket_fd = p_ctrl->max_fd - 1; socket_fd >= 0; socket_fd-- )
        {
            /** Check if the socket has event. */
            if ( FD_ISSET(socket_fd, &p_ctrl->fdset_all) )
            {
                p_ctrl->max_fd = socket_fd;
                return;
            }
        }

        /* When deleting the last existing socket */
        p_ctrl->max_fd = TLSAPP_MBEDTLS_INVALID_SOCKET;
    }
}

/**
 * @brief Add listener socket
 */
static usr_err_t tlsapp_mbedtls_add_listensoc( tlsapp_mbedtls_endpoint_ctrl_t *p_ctrl )
{
    /** Status */
    usr_err_t usr_err = USR_ERR_ASSERTION;

    /** For getting IP information */
    lwip_port_netif_cfg_t netif_cfg;

    /** Listener socket */
    int32_t listener_socket_fd = -1;

    /** Get IP address */
    usr_err = p_ctrl->p_lwip_port_instance->p_api->netifConfigGet( p_ctrl->p_lwip_port_instance->p_ctrl, &netif_cfg );
    USR_ERROR_RETURN( USR_SUCCESS == usr_err, USR_ERR_ABORTED );

    /** Check if the IP address is enabled. */
    listener_socket_fd = tlsapp_mbedtls_getsoc_byipadd( netif_cfg.ip_address, p_ctrl->max_fd );
    USR_ERROR_RETURN( TLSAPP_MBEDTLS_INVALID_SOCKET == listener_socket_fd, USR_ERR_ABORTED );

    /** Create new listener socket */
    listener_socket_fd = tlsapp_mbedtls_create_new_listensoc( netif_cfg.ip_address , TLSAPP_MBEDTLS_HOST_PORTNUMBER );
    USR_ERROR_RETURN( TLSAPP_MBEDTLS_INVALID_SOCKET  != listener_socket_fd, USR_ERR_ABORTED );

    /** Update listener socket informations */
    FD_SET( listener_socket_fd, &p_ctrl->fdset_listners );
    FD_SET( listener_socket_fd, &p_ctrl->fdset_all );
    if ( listener_socket_fd > p_ctrl->max_fd ) { p_ctrl->max_fd = listener_socket_fd; }
    p_ctrl->num_of_socket++;

    return USR_SUCCESS;
}

/**
 * @brief Remove listener socket
 */
static usr_err_t tlsapp_mbedtls_remove_listensoc( tlsapp_mbedtls_endpoint_ctrl_t * p_ctrl )
{
    /** Status */
    usr_err_t usr_err = USR_ERR_ASSERTION;
    int32_t soc_err = -1;

    /** For getting IP information */
    lwip_port_netif_cfg_t netif_cfg;

    /** Listener socket */
    int32_t listener_socket_fd = -1;

    /** Get IP address */
    usr_err = p_ctrl->p_lwip_port_instance->p_api->netifConfigGet( p_ctrl->p_lwip_port_instance->p_ctrl, &netif_cfg );
    USR_ERROR_RETURN( USR_SUCCESS == usr_err, USR_ERR_ABORTED );

    /** Get the target socket by referencing the IP address */
    listener_socket_fd = tlsapp_mbedtls_getsoc_byipadd( netif_cfg.ip_address, p_ctrl->max_fd );
    USR_ERROR_RETURN( TLSAPP_MBEDTLS_INVALID_SOCKET != listener_socket_fd, USR_ERR_ABORTED );

    /** Try closing the socket. */
    soc_err = lwip_close( listener_socket_fd );
    USR_ERROR_RETURN( TLSAPP_MBEDTLS_INVALID_SOCKET != soc_err, USR_ERR_ABORTED );

    /** Update listener socket informations */
    FD_CLR( listener_socket_fd, &p_ctrl->fdset_listners );
    FD_CLR( listener_socket_fd, &p_ctrl->fdset_all );
    p_ctrl->num_of_socket--;

    /** Update max_fd */
    tlsapp_mbedtls_update_maxfd_by_delsock( p_ctrl, listener_socket_fd );

    return USR_SUCCESS;
}


/**
 * @brief handle listener socket
 */
static usr_err_t tlsapp_mbedtls_handle_listensoc( tlsapp_mbedtls_endpoint_ctrl_t * p_ctrl, int32_t listner_socket_fd )
{
    /** new client socket information  */
    int32_t client_socket_fd = -1;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;

    /** Accept */
    client_socket_fd = lwip_accept(listner_socket_fd, (struct sockaddr*) &client_addr, &client_addr_len);
    USR_ERROR_RETURN( -1 != client_socket_fd, USR_ERR_ABORTED );

    /** Update fd sets */
    FD_SET( client_socket_fd, &p_ctrl->fdset_clients );
    FD_SET( client_socket_fd, &p_ctrl->fdset_all );
    if ( client_socket_fd > p_ctrl->max_fd ) { p_ctrl->max_fd = client_socket_fd; }
    p_ctrl->num_of_socket++;

    /** Return success code. */
    return USR_SUCCESS;
}

/**
 * @brief handle connected socket
 */
static usr_err_t tlsapp_mbedtls_handle_connsoc( tlsapp_mbedtls_endpoint_ctrl_t * p_ctrl, int32_t connected_socket_fd )
{
    int                     ret = USR_ERR_ASSERTION;
    tlsapp_mbedtls_ep_ctx_t *p_epctx = NULL;      /** endpoints context */

    /* Initialization */
    p_epctx = (tlsapp_mbedtls_ep_ctx_t *)pvPortMalloc( sizeof(tlsapp_mbedtls_ep_ctx_t) );
    memset( p_epctx, 0, sizeof(tlsapp_mbedtls_ep_ctx_t) );

    /** set socket in context **/
    p_epctx->sock_info.sock_hdl = connected_socket_fd;

    do
    {
        /* Mbed TLS open */
        if( (ret = tlsapp_mbedtls_open( p_epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
        {
            printf( "Failed to initialize Mbed TLS ret:%d\n\n", ret );
            fflush(stdout);
            goto exit;
        }

        /* Handshake operation */
        if( (ret = tlsapp_mbedtls_handshake( p_epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
        {
            printf( ( "Mbed TLS handshake failed. %d\n" ), ret );
            fflush(stdout);
            break;
        }

        /* Read the HTTP request */
        printf("  < Receive Get Request from client:\n");
        fflush(stdout);

        if( (ret = tlsapp_mbedtls_recv_httpreq( p_epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
        {
            printf( ( "Failed to receive HTTP Get Response. %d\n" ), ret );
            fflush(stdout);
            break;
        }

        /* Write the HTTP response */
        printf( ( "  > Send Get Response to client:\n" ) );
        fflush(stdout);

        if( (ret = tlsapp_mbedtls_send_httpres( p_epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
        {
            printf( ( "Failed to send HTTP Get Request. %d\n" ), ret );
            fflush(stdout);
            break;
        }

        USR_LOG_INFO( "/** Successfully Finished sample application for lwIP Port with Mbed TLS. **/\r\n\r\n");
    } while(0);

    /* Mbed TLS close */
    tlsapp_mbedtls_close( p_epctx );

exit:
    fflush(stdout);
    vTaskDelay( 2000 );

    vPortFree( p_epctx );

    /* delete connected socket */
    tlsapp_mbedtls_update_maxfd_by_delsock( p_ctrl, connected_socket_fd );
    FD_CLR( connected_socket_fd, &p_ctrl->fdset_clients );
    FD_CLR( connected_socket_fd, &p_ctrl->fdset_all );
    p_ctrl->num_of_socket--;
    lwip_close( connected_socket_fd );

    if( ret != USR_SUCCESS )
        ret = USR_ERR_ABORTED;

    return ret;
}

/**
 * @brief Check if the IP address is already used by existing sockets
 */
static int32_t tlsapp_mbedtls_getsoc_byipadd( uint32_t ip_address, int max_fd )
{
    int socket_err = TLSAPP_MBEDTLS_INVALID_SOCKET;
    int socket_fd = -1;
    struct sockaddr_in socket_addr;
    socklen_t socket_addr_len;

    /** Check if the IP address is already used in existing sockets */
    for ( socket_fd = 0; socket_fd <= max_fd ; socket_fd++ )
    {
        /** */
        socket_err = lwip_getsockname( socket_fd, (struct sockaddr*) &socket_addr, &socket_addr_len );
        if ( TLSAPP_MBEDTLS_INVALID_SOCKET == socket_err )
        {
            /** Is the socket is already closed? */
            continue;
        }

        if ( socket_addr.sin_addr.s_addr == ip_address )
        {
            /** The IP address is already used. */
            return socket_fd;
        }
    }

    /** The IP address is not used yet. */
    return TLSAPP_MBEDTLS_INVALID_SOCKET;
}

/**
 * @brief Create new listener socket
 */
static int32_t tlsapp_mbedtls_create_new_listensoc( uint32_t ip_address, uint16_t port )
{
    int socket_fd = -1;
    int socket_err = -1;
    struct sockaddr_in socket_addr;

    /** Create listener socket */
    socket_fd = lwip_socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( TLSAPP_MBEDTLS_INVALID_SOCKET == socket_fd )
    {
        return TLSAPP_MBEDTLS_INVALID_SOCKET;
    }

    /** Bind the socket */
    socket_addr.sin_port = htons(port);
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr.s_addr = ip_address;
    socket_err = lwip_bind( socket_fd, (struct sockaddr*) &socket_addr, (socklen_t) sizeof(struct sockaddr));
    if ( TLSAPP_MBEDTLS_INVALID_SOCKET == socket_err )
    {
        socket_err = lwip_close(socket_fd);
        return TLSAPP_MBEDTLS_INVALID_SOCKET;
    }

    /** Start listening */
    lwip_listen( socket_fd, TLSAPP_MBEDTLS_ACCEPTED_SOCKETSNUM );
    if ( TLSAPP_MBEDTLS_INVALID_SOCKET == socket_err )
    {
        socket_err = lwip_close(socket_fd);
        return TLSAPP_MBEDTLS_INVALID_SOCKET;
    }

    return socket_fd;
}

#else /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */

/**
 * @brief TLS client task handling the lwIP sockets.
 */
static void tlsapp_mbedtls_client_task( void* param )
{
    int                         ret = TLSAPP_MBEDTLS_RET_INTERNAL_ERROR;
    tlsapp_mbedtls_ep_ctx_t     epctx;

    /** Resolve parameter */
    tlsapp_mbedtls_endpoint_ctrl_t * p_ctrl = (tlsapp_mbedtls_endpoint_ctrl_t *) param;

    memset(&epctx, 0, sizeof(epctx));

    /** Notify the parenet task launch of this task. */
    xTaskNotifyGive( p_ctrl->p_parent_task_handle );

    while(1)
    {
        vTaskDelay(200);
        if ( p_ctrl->lwip_status == TLSAPP_MBEDTLS_LWIPMODULE_UP )
        {
            break;
        }
    }

    /* lwIP connection */
    if( (ret = tlsapp_mbedtls_client_lwip_connect( &epctx, p_ctrl )) != TLSAPP_MBEDTLS_RET_SUCCESS )
    {
        printf("lwIP connection failed. ret:%d\n\n", ret);
        fflush(stdout);
        return;
    }

    if( (ret = tlsapp_mbedtls_open( &epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
    {
        lwip_close(epctx.sock_info.sock_hdl);
        printf("Failed to initialize Mbed TLS ret:%d\n\n", ret);
        fflush(stdout);
        return;
    }
    
    if( (ret = tlsapp_mbedtls_handshake( &epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
    {
        printf( ( "Mbed TLS handshake failed. %d\n" ), ret );
        fflush(stdout);
        goto exit;
    }

    /*
     * Write the GET request
     */
    printf( ( "  > Send Get Request to server:\n" ) );
    fflush(stdout);

    if( (ret = tlsapp_mbedtls_send_httpreq( &epctx )) != TLSAPP_MBEDTLS_RET_SUCCESS )
    {
        printf( ( "Failed to send HTTP Get Request. %d\n" ), ret );
        fflush(stdout);
        goto exit;
    }
    fflush(stdout);

    /*
     * Read the HTTP response
     */
    printf("  < Receive Get Response from server:\n");
    fflush(stdout);

    ret = tlsapp_mbedtls_recv_httpres( &epctx );
    if( (ret != TLSAPP_MBEDTLS_RET_SUCCESS) && (ret != TLSAPP_MBEDTLS_RET_RCV_CLOSENOTIFY) )
    {
        printf( ( "Failed to receive HTTP Get Response. %d\n" ), ret );
        fflush(stdout);
        goto exit;
    }
    fflush(stdout);

    USR_LOG_INFO( "/** Successfully Finished sample application for lwIP Port with Mbed TLS. **/\r\n\r\n");
exit:
    fflush(stdout);
    vTaskDelay( 2000 );

    /* Mbed TLS Close */
    tlsapp_mbedtls_close( &epctx );

    /* FreeRTOS Socket Close */
    lwip_close(epctx.sock_info.sock_hdl);
}

/**
 * @brief Callback function called when the lwIP Network interface is up or down.
 */
static void tlsapp_mbedtls_lwipport_client_callback( lwip_port_callback_args_t * p_arg )
{
    /** Resolve context */
    tlsapp_mbedtls_endpoint_ctrl_t * p_ctrl = (tlsapp_mbedtls_endpoint_ctrl_t *) p_arg->p_context;

    switch ( p_arg->event )
    {
    case LWIP_PORT_CALLBACK_EVENT_NETIF_UP:
        p_ctrl->lwip_status = TLSAPP_MBEDTLS_LWIPMODULE_UP;
        break;

    case LWIP_PORT_CALLBACK_EVENT_NETIF_DOWN:
        if( p_ctrl->lwip_status == TLSAPP_MBEDTLS_LWIPMODULE_UP )
            p_ctrl->lwip_status = TLSAPP_MBEDTLS_LWIPMODULE_DOWN;
        break;
    }
}

/**
 * @brief Connect socket
 */
static int tlsapp_mbedtls_client_lwip_connect( tlsapp_mbedtls_ep_ctx_t *p_epctx, tlsapp_mbedtls_endpoint_ctrl_t * p_ctrl )
{
    /** Status */
    int socket_fd = -1;
    int socket_err = -1;
    struct sockaddr_in socket_addr;

    (void)p_ctrl;

    /** Create new socket */
    socket_fd = lwip_socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( TLSAPP_MBEDTLS_INVALID_SOCKET == socket_fd )
    {
        return TLSAPP_MBEDTLS_INVALID_SOCKET;
    }

    /** connect the socket */
    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sin_len = sizeof(socket_addr);
    socket_addr.sin_port = PP_HTONS(TLSAPP_MBEDTLS_HOST_PORTNUMBER);
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr.s_addr = inet_addr(TLSAPP_MBEDTLS_HOST_IPADDRESS);
    socket_err = lwip_connect( socket_fd, (struct sockaddr*) &socket_addr, (socklen_t) sizeof(struct sockaddr));
    if ( TLSAPP_MBEDTLS_INVALID_SOCKET == socket_err )
    {
        socket_err = lwip_close(socket_fd);
        return TLSAPP_MBEDTLS_INVALID_SOCKET;
    }

    /** set socket in context **/
    p_epctx->sock_info.sock_hdl = socket_fd;

    return TLSAPP_MBEDTLS_RET_SUCCESS;
}
#endif /* TLSAPP_MBEDTLS_ENDPOINT_ROLE_SERVER */
