/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef I2C_PMIC_H_
#define I2C_PMIC_H_

/* Generic headers */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common_utils.h"

#define LVL_ERR        (1u)            /* Error conditions */

#define RESET_VALUE    (0x00)

#ifndef APP_PRINT
    #define APP_PRINT(fn_, ...)
#endif

#ifndef APP_ERR_PRINT
    #define APP_ERR_PRINT(fn_, ...)
#endif

#define ONE_BYTE    0x01
#define TWO_BYTE    0x02

/* Function declarations */
fsp_err_t pd_all_on_postproc(void);
fsp_err_t pd_all_on_postproc_axi(void);

#endif                                 /* I2C_PMIC_H_ */
