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

/* macro definition */
/* Macros for commands to be received through RTT input */
#define WRITE_AND_READ  (1)
#define WRITE_READ      (2)
#define EXIT            (3)

/* Macro for miscellaneous */
#define DELAY_ONE_SEC   (1)

/* Array indexing MACRO */
#define ARRAY_INDEX      (0)

/* MACRO for checking if no byte is received */
#define BYTES_RECEIVED_ZERO  (0)

/* SPI buffer length */
#define BUFF_LEN (32)

/* Max wait count for time-out operation */
#define MAX_COUNT (0xFFFFFF)

/* Min wait count for time-out operation */
#define MIN_COUNT (0)

/* Indicates buffers are equal when memcmp returns 0 */
#define BUFF_EQUAL (0)

/* MACRO for null character */
#define NULL_CHAR  ('\0')

/* Macro to convert 32 bits into bytes */
#define BITS_TO_BYTES   (4U)

/* Macro for EP info */
#define EP_INFO         "\r\nThe project initializes the SPI driver and configures two SPI channels in Master (CH2)"\
                        "\r\nand Slave mode (CH3) on the RZ/N2L microcontroller. After initialization, the master"\
                        "\r\nand slave can transmit and receive data based on user commands. Refer to the MPU User Manual"\
                        "\r\nfor valid bit rates and corresponding clock settings.\r\n"\
                        "\r\nOn user input via RTT Viewer:"\
                        "\r\n    \"1\": Demonstrates separate Write() and Read() operations."\
                        "\r\n    \"2\": Demonstrates WriteRead() operation."\
                        "\r\n    \"3\": Closes all opened SPI modules and exits."\
                        "\r\nAny API/event failure message is also displayed.\r\n\r\n"

#define MENU            "\r\n-------------------------------------------------------------------------------\
                        \r\nSelect from the below Menu options\
                        \r\nPress 1 to Write() and Read()\
                        \r\nPress 2 to WriteRead()\
                        \r\nPress 3 to Exit\
                        \r\n-------------------------------------------------------------------------------\n"

/*
 * Global function prototype
 */
fsp_err_t spi_init (void);
fsp_err_t spi_write_and_read (void);
fsp_err_t spi_write_read (void);
fsp_err_t spi_clean_up (void);

void check_dmac_transfer_settings (void);

#endif /* SPI_EP_H_ */
