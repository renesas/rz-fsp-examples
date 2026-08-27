/***********************************************************************************************************************
 * File Name    : spi_ep.h
 * Description  : Contains declarations of data structures and functions used in spi_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef SPI_EP_H_
#define SPI_EP_H_

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* Macros for commands to be received through RTT input */
#define WRITE               (1)
#define READ                (2)
#define EXIT                (3)

/* Macro for checking if no byte is received */
#define BYTES_RECEIVED_ZERO (0)

/* Macro for EP info */
#define EP_INFO             "\r\nThe example project initializes SPI_B driver and configures SPI_B channel 2"\
                            "\r\nas Master and Pmod SF3 as Slave. After initialization, master"\
                            "\r\ncan transmit and receive data based on the commands from user."\
                            "\r\nRefer to the MPU User Manual for valid bit rates and corresponding"\
                            "\r\nclock settings.\r\n"

#define MX25L12835F_SECTOR_SIZE    (4096)
#define MX25L12835F_PAGE_SIZE      (256)

/***********************************************************************************************************************
 * User-defined APIs
 **********************************************************************************************************************/
#define MX25L12835F_SSL_PIN         (BSP_IO_PORT_06_PIN_03)
#define MX25L12835F_ADDRESS         (0x1000000U)

/* Function declarations */
fsp_err_t spi_init (void);
fsp_err_t spi_write (void);
fsp_err_t spi_read (void);
fsp_err_t spi_exit_demo (void);
void spi_clean_up (void);

#endif /* SPI_EP_H_ */
