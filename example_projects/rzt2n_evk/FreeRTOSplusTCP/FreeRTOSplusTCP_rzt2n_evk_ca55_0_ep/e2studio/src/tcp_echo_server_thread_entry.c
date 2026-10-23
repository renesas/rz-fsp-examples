#include "tcp_echo_server_thread.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
/* TCP Echo Server entry function */
uint8_t ucTcpReceivedBuffer[1500];

/* pvParameters contains TaskHandle_t */
void tcp_echo_server_thread_entry(void *pvParameters)
{
    Socket_t xTCPListenSocket;
    Socket_t xTCPSocket;
    struct freertos_sockaddr xClientAddress;
    struct freertos_sockaddr xServerAddress;
    uint32_t xClientLength = sizeof( struct freertos_sockaddr );
    BaseType_t lBytes;
    BaseType_t lResult;
    uint32_t xReceiveTimeout;
    BaseType_t xBacklog;

    FSP_PARAMETER_NOT_USED (pvParameters);

    /* TODO: add your own code here */
    while(FreeRTOS_IsNetworkUp() == pdFALSE){
        vTaskDelay (100);
    }

    xTCPListenSocket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );
    if( xTCPListenSocket == FREERTOS_INVALID_SOCKET )
    {
        vTaskDelete(NULL);
    }
    xServerAddress.sin_family = FREERTOS_AF_INET;
    xServerAddress.sin_port = FreeRTOS_htons( 10000 );
    lResult = FreeRTOS_bind( xTCPListenSocket, &xServerAddress, sizeof(xServerAddress) );
    if( lResult < 0)
    {
        vTaskDelete(NULL);
    }
    xReceiveTimeout = portMAX_DELAY;
    FreeRTOS_setsockopt( xTCPListenSocket ,
                         0,
                         FREERTOS_SO_RCVTIMEO,
                         &xReceiveTimeout,
                         sizeof(xReceiveTimeout) );

    xBacklog = 10;
    FreeRTOS_listen( xTCPListenSocket, xBacklog);

    while (1)
    {
        xTCPSocket = FreeRTOS_accept(xTCPListenSocket,
                                     &xClientAddress , &xClientLength );
        if( xTCPSocket == FREERTOS_INVALID_SOCKET )
        {
            FreeRTOS_closesocket(xTCPListenSocket);
            vTaskDelete(NULL);
        }else{
            xReceiveTimeout = portMAX_DELAY;
            FreeRTOS_setsockopt( xTCPSocket ,
                                 0,
                                 FREERTOS_SO_RCVTIMEO,
                                 &xReceiveTimeout,
                                 sizeof(xReceiveTimeout) );
            do{
                lBytes = FreeRTOS_recv(xTCPSocket,
                                       &ucTcpReceivedBuffer, sizeof( ucTcpReceivedBuffer ),
                                       0);
                if( lBytes > 0){
                    FreeRTOS_send(xTCPSocket,
                                  &ucTcpReceivedBuffer, (size_t)lBytes,
                                  0);
                }
            }while(lBytes > 0);
            FreeRTOS_closesocket(xTCPSocket);
        }
    }
}
