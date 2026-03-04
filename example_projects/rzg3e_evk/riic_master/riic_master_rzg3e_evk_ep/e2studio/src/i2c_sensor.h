/***********************************************************************************************************************
 * File Name    : i2c_sensor.h
 * Description  : Contains declarations of data structures and functions used in i2c_sensor.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
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

#define MEASURE_PAYLOAD_SIZE    (0x03)        /* Measurement enable data length */
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

#define EP_INFO     "The example project demonstrates the typical use of the RIIC master HAL module APIs.\r\n"\
                    "The project initializes RIIC master module with fast mode and interfaces with PmodACL Board\r\n"\
                    "for ADXL345. On power up after establishing the connection of sensor with the board,\r\n"\
                    "it displays accelerometer axis data on J-Link RTT Viewer. Any API/event failure will be\r\n"\
                    "displayed on J-Link RTT Viewer.\r\n\r\n"\

/* Function declarations */
fsp_err_t init_sensor(void);
fsp_err_t read_sensor_data(uint8_t * xyz_data);

void deinit_sensor(void);

#endif /* I2C_SENSOR_H_ */
