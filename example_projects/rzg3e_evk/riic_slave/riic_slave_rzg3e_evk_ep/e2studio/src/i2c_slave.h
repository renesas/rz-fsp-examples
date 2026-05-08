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

/* Define the pins on board LED */
#define LED_0               (BSP_IO_PORT_06_PIN_05)
#define LED_1               (BSP_IO_PORT_05_PIN_06)
#define LED_2               (BSP_IO_PORT_06_PIN_06)
#define LED_3               (BSP_IO_PORT_06_PIN_04)

/* Macro definition */
/* For state LED */
#define LED_ON             (BSP_IO_LEVEL_HIGH)
#define LED_OFF            (BSP_IO_LEVEL_LOW)

/* Macro for checking if two buffers are equal */
#define BUFF_EQUAL         (0U)

/* Buffer size for slave and master data */
#define BUF_LEN            (0x06)

/* Human eye noticeable LED toggle delay */
#define TOGGLE_DELAY       (500U)
#define TIME_DELAY         (1000U)

#define EP_INFO     "\nThis example project demonstrates i2c slave operation using two i2c channels." \
                    "\r\nAfter successful initialization, slave read/write operations run continuously." \
                    "\r\nOn each successful I2C transaction (6 bytes), transceived data is compared."\
                    "\r\nLED blinks on data match; stays on to indicate failure." \
                    "\r\nCorresponding messages for each slave operation are shown on the RTT Viewer."\
                    "\r\nAny API or event failure is also reported.\r\n\r\n"

/*  Global functions */
fsp_err_t init_i2c_driver (void);
fsp_err_t process_slave_WriteRead (void);
void deinit_i2c_driver (void);
void set_led (bsp_io_level_t led_state);

#endif /* I2C_SLAVE_H_ */
