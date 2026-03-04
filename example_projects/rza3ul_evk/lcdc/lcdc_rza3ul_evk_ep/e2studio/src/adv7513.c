/***********************************************************************************************************************
 * File Name    : adv7513.c
 * Description  : Contains data structures and functions used in adv7513.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "hal_data.h"
#include <unistd.h>
#include "adv7513.h"
#include "adv7513_data.h"
#include <stdbool.h>
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
typedef struct
{
    __IOM uint32_t IEN0E;
} R_IIC_IEN0E_Type;

#define R_IIC_IEN0E  ((R_IIC_IEN0E_Type *) 0x11031870)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void iic_master_init_1 (void);
static void adv7513_set_reg_array_byte (const struct reg_value * settings, uint16_t num_settings,
                                        uint8_t slave_addr_wr);
static void adv7513_set_reg_byte (uint8_t reg, uint8_t val, uint8_t slave_addr_wr);
static void adv7513_write_reg2 (uint8_t reg, uint8_t val);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @brief       This function initializes the ADV7513 HDMI transmitter.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
void adv7513_init(void)
{
    uint8_t slave_addr_wr = 0x72;
    uint16_t num_settings;

    iic_master_init_1();

    num_settings = sizeof(adv7513_init_setting) / sizeof(uint16_t);

    adv7513_set_reg_array_byte(adv7513_init_setting, num_settings, slave_addr_wr);
}

/***********************************************************************************************************************
 * @brief      This function initializes IIC master channel 1.
 * @param[IN]  None.
 * @retval     None.
 **********************************************************************************************************************/
static void iic_master_init_1(void)
{
    R_IIC_IEN0E->IEN0E = 0x01010000;

    R_BSP_MODULE_START(FSP_IP_RIIC, 1);

    R_BSP_MODULE_CLKON(FSP_IP_RIIC, 1);
    R_BSP_MODULE_RSTOFF(FSP_IP_RIIC, 1);

    R_RIIC1->ICCR1_b.ICE    = 0b0;
    R_RIIC1->ICCR1_b.IICRST = 0b1;
    R_RIIC1->ICCR1_b.ICE    = 0b1;

    R_RIIC1->ICSAR0_hword.L = 0x0055;
    R_RIIC1->ICBRH_byte.LL  = 0x30; //0xca;
    R_RIIC1->ICBRL_byte.LL  = 0x30; //0xca;
    R_RIIC1->ICSER_byte.LL  = 0x01;
    R_RIIC1->ICMR1_byte.LL  = 0x38; //0x08; 20
    R_RIIC1->ICMR2_byte.LL  = 0x26;
    R_RIIC1->ICMR3_byte.LL  = 0x00; //0x00;
    R_RIIC1->ICFER_byte.LL  = 0x40;
    R_RIIC1->ICIER_byte.LL  = 0x00;

    R_RIIC1->ICCR1_b.IICRST = 0b0;
}

/***********************************************************************************************************************
 * @brief      This function writes a byte register array to the ADV7513 device.
 * @param[IN]  settings        pointer to the register-value setting array.
 *             num_settings    number of register settings.
 *             slave_addr_wr   IIC slave write address.
 * @retval     None.
 **********************************************************************************************************************/
void adv7513_set_reg_array_byte(const struct reg_value *settings, uint16_t num_settings, uint8_t slave_addr_wr)
{
    uint16_t i;

    for (i = 0; i < num_settings; ++i, ++settings)
    {
        iic_master_init_1();
        adv7513_set_reg_byte(settings->reg, settings->val, slave_addr_wr);
    }
}

/***********************************************************************************************************************
 * @brief      This function writes a byte value to the specified ADV7513 register.
 * @param[IN]  reg             register address.
 *             val             value to be written to the register.
 *             slave_addr_wr   IIC slave write address.
 * @retval     None.
 **********************************************************************************************************************/
void adv7513_set_reg_byte(uint8_t reg, uint8_t val, uint8_t slave_addr_wr)
{
    R_RIIC1->ICCR2_b.ST    = 0b1;

    while (R_RIIC1->ICSR2_b.START == 0)
    {
        ; /* Do nothing */
    }

    R_RIIC1->ICDRT_byte.LL = slave_addr_wr | 0x00;  /* Slave address */

    while (R_RIIC1->ICSR2_b.TEND == 0)
    {
        ; /* Do nothing */
    }

    while (R_RIIC1->ICSR2_b.NACKF == 1)
    {
        ; /* Do nothing */
    }

    adv7513_write_reg2(reg, val);

    R_RIIC1->ICSR2_b.STOP = 0x0;
    R_RIIC1->ICCR2_b.SP   = 0x1;

    while (R_RIIC1->ICSR2_b.STOP != 1)
    {
        ; /* Do nothing */
    }

    R_RIIC1->ICSR2_b.STOP = 0b0;
}

/***********************************************************************************************************************
 * @brief      This function writes a byte value to the ADV7513 register.
 * @param[IN]  reg    Register address.
 *             val    value to be written to the register.
 * @retval     None.
 **********************************************************************************************************************/
void adv7513_write_reg2(uint8_t reg, uint8_t val)
{
    uint8_t regbuf[2];
    uint32_t  i;

    regbuf[0] = reg;
    regbuf[1] = val;

    R_RIIC1->ICDRT_byte.LL = regbuf[0];

    while (R_RIIC1->ICSR2_b.TEND == 0)
    {
        ; /* Do nothing */
    }

    R_RIIC1->ICDRT_byte.LL = regbuf[1];

    while (R_RIIC1->ICSR2_b.TEND == 0)
    {
        ; /* Do nothing */
    }

    for (i=0; i<200000; i++)
    {
        ; /* Do nothing */
    }
}
