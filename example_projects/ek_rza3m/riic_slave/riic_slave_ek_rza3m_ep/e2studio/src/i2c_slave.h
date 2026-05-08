/***********************************************************************************************************************
 * File Name    : i2c_slave.h
 * Description  : Contains declarations of data structures and functions used in i2c_slave.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2025 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef I2C_SLAVE_H_
#define I2C_SLAVE_H_

/* Macro definition */
/* For state LED */
#define LED_ON             (BSP_IO_LEVEL_HIGH)
#define LED_OFF            (BSP_IO_LEVEL_LOW)

/* Macro for checking if two buffers are equal */
#define BUFF_EQUAL         (0U)

/* Buffer size for slave and master data */
#define BUF_LEN            (0x06)

/* Human eye noticeable LED toggle delay */
#define TOGGLE_DELAY       (0x3E8)

#define EP_INFO     "This example project demonstrates RIIC slave operation using two I2C channels.\r\n"\
                    "It performs Slave read and write operation continuously once successful initialization.\r\n"\
                    "On successful I2C transaction(6 bytes), transceived data is compared.\r\n"\
                    "LED blinks when data matches; otherwise, it is turned ON to indicate a failure.\r\n"\
                    "In both cases, corresponding slave operation message is displayed on Terminal Emulator.\r\n"\
                    "Any API/event failure message is also displayed.\r\n\r\n"

/* Global functions */
/* I2C initialization function */
fsp_err_t init_i2c_driver (void);

/* I2C Slave write and read operation function */
fsp_err_t process_slave_writeread (void);

/* I2C de-initialization function */
void deinit_i2c_driver (void);

/* LED setting function */
void set_led (bsp_io_level_t led_state);

#endif /* I2C_SLAVE_H_ */
