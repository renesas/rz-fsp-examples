/***********************************************************************************************************************
 * File Name    : syscalls.c
 * Description  : Redirects newlib standard output to the SCI UART console.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#if !defined(__ICCARM__)

/* EWARM migration: GCC/newlib implementation. IAR uses DLIB hooks. */

#include <stddef.h>
#include <stdint.h>

int stdio_write(const uint8_t * p_buffer, uint32_t count);
int _write(int file, char * ptr, int len);

int _write(int file, char * ptr, int len)
{
    (void) file;

    if ((NULL == ptr) || (len <= 0))
    {
        return 0;
    }

    return len - stdio_write((const uint8_t *) ptr, (uint32_t) len);
}

#endif /* !defined(__ICCARM__) */
