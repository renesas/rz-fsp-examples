/***********************************************************************************************************************
 * File Name    : i2c_sensor.c
 * Description  : Contains data structures and functions used in i2c_sensor.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "i2c_sensor.h"

/***********************************************************************************************************************
 * @addtogroup r_iic_master_ep
 * @{
 **********************************************************************************************************************/

/*
 * Global Variables
 */
/* Reading I2C call back event through i2c_master callback */
static volatile i2c_master_event_t s_i2c_event = I2C_MASTER_EVENT_ABORTED;

/*
 * private function declarations
 */
static fsp_err_t get_device_id (uint8_t * dev_id);
static fsp_err_t validate_i2c_event (void);

/***********************************************************************************************************************
 *  @brief       initialize IIC Master module and set up PMOD ACL sensor
 *  @param[IN]   None
 *  @retval      FSP_SUCCESS                  Upon successful open and start of timer
 *  @retval      Any Other Error code apart from FSP_SUCCESS is  Unsuccessful open or start
 **********************************************************************************************************************/
fsp_err_t init_sensor(void)
{
    uint8_t device_id = RESET_VALUE;

    fsp_err_t err = FSP_SUCCESS;

    uint8_t measure_enable_payload[MEASURE_PAYLOAD_SIZE] = { POWER_CTL_REG, ENABLE_BIT };

    /* Opening IIC Master module */
    err = R_IIC_MASTER_Open(&g_i2c_master_ctrl, &g_i2c_master_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_IIC_MASTER_Open API failed ** \r\n");
        return err;
    }

    /* Get device ID as a start of communication */
    err =  get_device_id(&device_id);

    /* Verify I2C transaction is successful with PMOD ACL and correct device ID is received */
    if ((FSP_SUCCESS == err) && (DEVICE_SIGNATURE == device_id))
    {
        /* Access to POWER_CTL Register (0x2D) and enable measurement  */
        err = R_IIC_MASTER_Write(&g_i2c_master_ctrl, measure_enable_payload, TWO_BYTE, false);
        if (FSP_SUCCESS == err)
        {
            err = validate_i2c_event();

            /* Handle error */
            if (FSP_ERR_TRANSFER_ABORTED == err)
            {
                APP_ERR_PRINT("** POWER_CTL reg I2C write failed!!! ** \r\n");
            }
        }

        /* Handle error */
        else
        {
            /* Write API returns itself is not successful */
            APP_ERR_PRINT("** R_IIC_MASTER_Write API failed ** \r\n");
        }
    }

    /* Handle error */
    else
    {
        /* Failed to get Device ID */
        APP_ERR_PRINT("Error in reading Device ID ** \r\n");
    }
    return err;
}

/***********************************************************************************************************************
 *  @brief       DeInitialize IIC Master module
 *  @param[IN]   None
 *  @retval      None
 **********************************************************************************************************************/
void deinit_sensor(void)
{
    fsp_err_t err     = FSP_SUCCESS;

    /* close open modules */
    err =  R_IIC_MASTER_Close (&g_i2c_master_ctrl);

    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_IIC_MASTER_Close API failed ** \r\n");
    }
}

/***********************************************************************************************************************
 *  @brief       Read The ACL Sensor data for X,Y and Z axis. And prints the raw data on the RTT console
 *  @param[IN]   Array pointer to get the sensor data
 *  @retval      FSP_SUCCESS                  Upon successful open and start of timer
 *  @retval      Any Other Error code apart from FSP_SUCCES  Unsuccessful open or start
 **********************************************************************************************************************/
fsp_err_t read_sensor_data(uint8_t * p_xyz_data)
{
    fsp_err_t err           = FSP_SUCCESS;

    uint8_t adxl_data[7]    = { RESET_VALUE };
    uint8_t internal_reg    = AXIS_DATA;

    if (NULL == p_xyz_data)
    {
        err = FSP_ERR_INVALID_POINTER;
        APP_ERR_PRINT("** NULL Pointer check fail ** \r\n");
        return err;
    }

    /* Write Accelerometer Data register   */
    err = R_IIC_MASTER_Write(&g_i2c_master_ctrl, &internal_reg, ONE_BYTE, true);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_IIC_MASTER_Write API failed ** \r\n");
        return err;
    }
    else
    {
        err = validate_i2c_event();

        /* Performs read operation only when write event is successful, failure part is handled at next stage*/
        if (FSP_SUCCESS == err)
        {
            err = R_IIC_MASTER_Read(&g_i2c_master_ctrl, &adxl_data[0], DATA_REGISTERS, false);

            /* Handle error */
            if (FSP_SUCCESS != err)
            {
                APP_ERR_PRINT("** R_IIC_MASTER_Read API failed ** \r\n");
                return err;
            }
            else
            {
                err = validate_i2c_event();
                if (FSP_SUCCESS != err)
                {
                    APP_ERR_PRINT("** Data register, I2C read failed ** \r\n");
                    return err;
                }
            }
        }
        else
        {
            APP_ERR_PRINT("** Accel Data register, I2C write failed ** \r\n");
            return err;
        }

        /*  Read is successful update the x,y,z values and return success */
        for (uint8_t itr = RESET_VALUE; itr < SENSOR_DATA_SIZE; itr++)
        {
            *(p_xyz_data + itr) = adxl_data[itr];
        }
    }
    return err;
}

/***********************************************************************************************************************
 *  @brief     Read Device signature/ID from sensor
 *  @param[in] dev_id                       retrieve PMOD ACL device ID
 *  @retval    FSP_SUCCESS                  on successful I2C transaction
 *             FSP_ERR_INVALID_POINTER      if invalid parameters passed
 *             err                          whichever occurs on either API or on I2C event failure
 **********************************************************************************************************************/
static fsp_err_t get_device_id(uint8_t *dev_id)
{
    fsp_err_t err         = FSP_SUCCESS;

    uint8_t reg_address   = DEVICE_ID_REG;

    /* Parameter checking */
    if (NULL == dev_id)
    {
        err = FSP_ERR_INVALID_POINTER;
        APP_ERR_PRINT("** NULL Pointer check fail ** \r\n");
        return err;
    }

    /* Send register address to sensor */
    err = R_IIC_MASTER_Write(&g_i2c_master_ctrl, &reg_address, ONE_BYTE, true);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_IIC_MASTER_Write API failed ** \r\n");
    }
    else
    {
        err = validate_i2c_event();

        /* Handle error */
        if (FSP_ERR_TRANSFER_ABORTED == err)
        {
            APP_ERR_PRINT("** Device ID reg, I2C write failed ** \r\n");
        }
        else
        {
            /* Read only when I2C write and its callback event is successful */
            err  = R_IIC_MASTER_Read(&g_i2c_master_ctrl, dev_id, ONE_BYTE, false);

            /* Handle error */
            if (FSP_SUCCESS != err)
            {
                APP_ERR_PRINT("** R_IIC_MASTER_Read API failed ** \r\n");
            }
            else
            {
                err = validate_i2c_event();

                /* Handle error */
                if (FSP_ERR_TRANSFER_ABORTED == err)
                {
                    APP_ERR_PRINT("** Device ID read,  I2C read failed ** \r\n");
                }
            }
        }
    }

    /* On successful I2C transaction with PMOD-ACL return from here */
    return err;
}

/***********************************************************************************************************************
 *  @brief      User callback function
 *  @param[in]  p_args
 *  @retval     None
 **********************************************************************************************************************/
void i2c_master_callback(i2c_master_callback_args_t *p_args)
{
    if (NULL != p_args)
    {
        /* capture callback event for validating the i2c transfer event*/
        s_i2c_event = p_args->event;
    }
}

/***********************************************************************************************************************
 *  @brief       Validate i2c receive/transmit  based on required write read operation
 *  @param[in]   None
 *  @retval      FSP_SUCCESS                       successful event receiving returns FSP_SUCCESS
 *               FSP_ERR_TRANSFER_ABORTED          Either on timeout elapsed or received callback event is
 *                                                 I2C_MASTER_EVENT_ABORTED
 **********************************************************************************************************************/
static fsp_err_t validate_i2c_event(void)
{
    uint32_t local_time_out = UINT32_MAX;

    /* resetting call back event capture variable */
    s_i2c_event = (i2c_master_event_t)RESET_VALUE;

    do
    {
        /* This is to avoid infinite loop */
        --local_time_out;

        if (RESET_VALUE == local_time_out)
        {
            return FSP_ERR_TRANSFER_ABORTED;
        }

    }
    while (RESET_VALUE == s_i2c_event);

    if (I2C_MASTER_EVENT_ABORTED != s_i2c_event)
    {
        /* Make sure this is always Reset before return */
        s_i2c_event = (i2c_master_event_t)RESET_VALUE;
        return FSP_SUCCESS;
    }

    /* Make sure this is always Reset before return */
    s_i2c_event = (i2c_master_event_t)RESET_VALUE;
    return FSP_ERR_TRANSFER_ABORTED;
}

/***********************************************************************************************************************
 * @} (end addtogroup r_iic_master_ep)
 **********************************************************************************************************************/
