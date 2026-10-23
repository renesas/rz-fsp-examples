/***********************************************************************************************************************
 * File Name    : sio_char.h
 * Description  : Contains data structures and function declaration used in sio_char.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef _SIO_CHAR_HEADER_
#define _SIO_CHAR_HEADER_

typedef char            char_t;

/******************************************************************************
Macro definitions
******************************************************************************/
#define TRANSFER_LENGTH (1)

/******************************************************************************
Functions Prototypes
******************************************************************************/
int32_t SioWrite(int32_t file_no, const char_t * buffer, uint32_t writing_b);
int32_t SioRead(int32_t file_no, char_t * buffer, uint32_t reading_b);

int32_t IoGetchar(void);
void    IoPutchar(int32_t buffer);

#endif  /* _SIO_CHAR_HEADER_ */
