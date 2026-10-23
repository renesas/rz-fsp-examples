/***********************************************************************************************************************
 * File Name    : syscalls.c
 * Description  : Redirects newlib standard input and output to the SCI UART console.
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
#include <sys/stat.h>
#include <sys/types.h>

int stdio_read(uint8_t * pbyBuffer, uint32_t uiCount);
int stdio_write(const uint8_t * pbyBuffer, uint32_t uiCount);

int _write(int file, char * ptr, int len);
int _read(int file, char * ptr, int len);
int _close(int file);
int _fstat(int file, struct stat * st);
int _isatty(int file);
off_t _lseek(int file, off_t ptr, int dir);

int _write(int file, char * ptr, int len)
{
    (void) file;

    if ((NULL == ptr) || (len <= 0))
    {
        return 0;
    }

    return len - stdio_write((const uint8_t *) ptr, (uint32_t) len);
}

int _read(int file, char * ptr, int len)
{
    (void) file;

    if ((NULL == ptr) || (len <= 0))
    {
        return 0;
    }

    return stdio_read((uint8_t *) ptr, (uint32_t) len);
}

int _close(int file)
{
    (void) file;

    return -1;
}

int _fstat(int file, struct stat * st)
{
    (void) file;

    if (NULL != st)
    {
        st->st_mode = S_IFCHR;
    }

    return 0;
}

int _isatty(int file)
{
    (void) file;

    return 1;
}

off_t _lseek(int file, off_t ptr, int dir)
{
    (void) file;
    (void) ptr;
    (void) dir;

    return (off_t) 0;
}

#endif /* !defined(__ICCARM__) */
