/***********************************************************************************************************************
 * File Name    : i2c_sensor.h
 * Description  : Contains declarations of data structures and functions used in i2c_sensor.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef I2C_SENSOR_H_
#define I2C_SENSOR_H_

/* Macros for I2C data length */
#define ONE_BYTE                (0x01)
#define TWO_BYTE                (0x02)

#define MEASURE_PAYLOAD_SIZE    (0x03)        /* Measurement enable data length */
#define ACCELERO_DELAY          (0xC8)        /* Accelerometer delay value */
#define SENSOR_READ_DELAY       (0x03)        /* Sensor read delay */
#define ENABLE_BIT              (0x08)        /* Enable bit for measurement mode */
#define DATA_REGISTERS          (0x06)        /* Number of data registers to read */
/* Accelerometer internal register whichever consumed here */
#define DEVICE_ID_REG           (0x00)
#define DEVICE_SIGNATURE        (0xE5)
#define POWER_CTL_REG           (0x2D)
#define AXIS_DATA               (0x32)

#define SENSOR_DATA_SIZE        (0x06)

#define EP_INFO     "\nThis project utilizes PMOD ACL sensor as RIIC slave device\n"\
                    "Upon successful initialization, MPU displays sensor axis data\r\n\r\n"

/*
 * Function declarations
 */
/* Initialize IIC master module and set up PMOD ACL sensor */
fsp_err_t init_sensor (void);

/* DeInitialize IIC master module */
void deinit_sensor (void);

/* Read The ACL Sensor data */
fsp_err_t read_sensor_data (uint8_t * xyz_data);

#endif /* I2C_SENSOR_H_ */
