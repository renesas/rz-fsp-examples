/***********************************************************************************************************************
 * File Name    : i2c_sensor.h
 * Description  : Contains data structures and functions declaration used in i2c_sensor.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef I2C_SENSOR_H_
#define I2C_SENSOR_H_

/* Number of bytes used in transmission/receipt */
#define ONE_BYTE                (0x01)
#define TWO_BYTE                (0x02)

/* Measurement enable data length */
#define MEASURE_PAYLOAD_SIZE    (0x03)

/* The delay between the measurements of the accelerator sensor (unit: MS) */
#define ACCELERO_DELAY          (0xC8)

/* The waiting time between sensor data reads (unit: seconds) */
#define SENSOR_READ_DELAY       (0x03)

/* The bit used to activate the measurement mode in the Power_CTL register */
#define ENABLE_BIT              (0x08)

/* Number of data registers to read from the sensor (x, y, z - each axis of 2 bytes) */
#define DATA_REGISTERS          (0x06)

/* Accelerometer internal register whichever consumed here */
#define DEVICE_ID_REG           (0x00)  /* The register address contains the ID of the sensor device */
#define DEVICE_SIGNATURE        (0xE5)  /* The expected ID value of the sensor device */
#define POWER_CTL_REG           (0x2D)  /* The source control register of the sensor */
#define AXIS_DATA               (0x32)  /* The register address starts to contain axial, x, y, z axis data */

/* Total number of sensor data to read (2 bytes each, 3 axis) */
#define SENSOR_DATA_SIZE        (0x06)

#define EP_INFO                 "\nThis EP utilizes PMOD ACL sensor as IIC Slave device"\
                                "\r\nUpon successful initialization, MPU displays sensor axis data."\
                                "\r\nAny API or event failure message is displayed via RTT Viewer.\r\n\r\n"

/* Function declarations */
fsp_err_t init_sensor (void);
fsp_err_t read_sensor_data (uint8_t * p_xyz_data);
void deinit_sensor (void);

#endif /* I2C_SENSOR_H_ */
