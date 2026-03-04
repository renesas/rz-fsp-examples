/***********************************************************************************************************************
 * File Name    : spi_ep.h
 * Description  : Contains declarations of data structures and functions used in spi_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef SPI_EP_H_
#define SPI_EP_H_

/***********************************************************************************************************************
 * @ingroup rspi_ep
 * @{
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/* Macros for commands to be received through RTT input */
#define WRITE                (1)
#define READ                 (2)
#define EXIT                 (3)

/* Macro array indexing */
#define ARRAY_INDEX          (0)

/* Macro for checking if no byte is received */
#define BYTES_RECEIVED_ZERO  (0)

/* Macro for null character */
#define NULL_CHAR            ('\0')

/* Macro to convert 32 bits into bytes */
#define BITS_TO_BYTES        (4U)

/* Macro for EP info */
#define EP_INFO                 "\r\nThe project initializes RSPI driver and configures RSPI channel 0 "\
                                "\r\nas Master and PMOD SF3 as Slave. After initialization, master "\
                                "\r\ncan transmit and receive data based on the commands from user."\
                                "\r\nRefer to the MPU User Manual for valid bit rates and corresponding"\
                                "\r\nclock settings.\r\n"

#define MX25L12835F_SSL_PIN     (BSP_IO_PORT_15_PIN_03)
#define MX25L12835F_ADDRESS     (0x1000000U)
/***********************************************************************************************************************
 * User-defined APIs
 **********************************************************************************************************************/
/*
 * Function declarations
 */
/* This functions initializes SPI master and slave modules */
fsp_err_t spi_init (void);

/* This function demos R_RSPI_Write() */
fsp_err_t spi_write (void);

/* This function demos R_RSPI_Read() */
fsp_err_t spi_read (void);

/* This function demos R_RSPI_Read() */
fsp_err_t spi_exit_demo (void);

/* This function closes all the opened SPI modules */
void spi_clean_up (void);

#endif /* SPI_EP_H_ */
