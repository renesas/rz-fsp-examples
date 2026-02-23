/***********************************************************************************************************************
 * File Name    : i2c_slave.h
 * Description  : Contains declarations of data structures and functions used in i2c_slave.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef I2C_SLAVE_H_
#define I2C_SLAVE_H_

/* macro definition */
/* for on board LED */
#define LED_ON             (BSP_IO_LEVEL_HIGH)
#define LED_OFF            (BSP_IO_LEVEL_LOW)

/* MACRO for checking if two buffers are equal */
#define BUFF_EQUAL         (0U)

/* buffer size for slave and master data */
#define BUF_LEN            (0x06)

/* Human eye noticeable LED toggle delay */
#define TOGGLE_DELAY       (0x3E8)

#define EP_INFO     "\nThis EP demonstrates IIC slave operation using two I2C channels." \
                    "\r\nAfter successful initialization, slave read/write operations run continuously." \
                    "\r\nOn each successful I2C transaction (6 bytes), transceived data is compared."\
                    "\r\nLED blinks on data match; stays ON to indicate failure." \
                    "\r\nCorresponding messages for each slave operation are shown on the RTT Viewer."\
                    "\r\nAny API or event failure is also reported.\r\n\r\n"
/*
 *  Global functions
 */
fsp_err_t init_i2C_driver (void);
fsp_err_t process_slave_WriteRead (void);

void deinit_i2c_driver (void);
void set_led (bsp_io_level_t led_state);

#endif /* I2C_SLAVE_H_ */
