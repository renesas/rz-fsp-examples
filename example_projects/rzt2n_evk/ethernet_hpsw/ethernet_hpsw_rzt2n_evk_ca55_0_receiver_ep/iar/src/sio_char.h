/***********************************************************************************************************************
 * File Name    : sio_char.h
 * Description  : Contains declarations of data structures and functions used in siochar.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef _SIO_CHAR_HEADER_
#define _SIO_CHAR_HEADER_

#include <stdint.h>

/******************************************************************************
Macro definitions
******************************************************************************/
#define TRANSFER_LENGTH (1)

/******************************************************************************
Functions Prototypes
******************************************************************************/
int32_t IoGetchar (void);
void    IoPutchar (int32_t buffer);
int     app_check_data (void);
int stdio_read (uint8_t * pbyBuffer, uint32_t uiCount);

#endif  /* _SIO_CHAR_HEADER_ */
