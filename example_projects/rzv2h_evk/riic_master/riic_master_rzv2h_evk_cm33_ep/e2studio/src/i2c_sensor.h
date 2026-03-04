/***********************************************************************************************************************
 * File Name    : i2c_sensor.h
 * Description  : Contains declarations of data structures and functions used in i2c_sensor.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2024 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef I2C_SENSOR_H_
#define I2C_SENSOR_H_

#ifndef ONE_BYTE
    #define ONE_BYTE            (0x01)
#endif

#ifndef TWO_BYTE
    #define TWO_BYTE            (0x02)
#endif

#define MEASURE_PAYLOAD_SIZE    (0x03)        /* measurement enable data length */
#define ACCELERO_DELAY          (0xC8)
#define SENSOR_READ_DELAY       (0x03)
#define ENABLE_BIT              (0x08)
#define DATA_REGISTERS          (0x06)

/* Accelerometer internal register whichever consumed here */
#define DEVICE_ID_REG           (0x00)
#define DEVICE_SIGNATURE        (0xE5)
#define POWER_CTL_REG           (0x2D)
#define AXIS_DATA               (0x32)

#define SENSOR_DATA_SIZE        (0x06)

#define EP_INFO     "This EP utilizes PMOD ACL sensor as iic slave device\r\n"\
                    "Upon successful initialization, MPU displays sensor axis data\r\n\r\n"\

/*
 * function declarations
 */
fsp_err_t init_sensor (void);
fsp_err_t read_sensor_data (uint8_t * xyz_data);

void deinit_sensor (void);

#endif /* I2C_SENSOR_H_ */
