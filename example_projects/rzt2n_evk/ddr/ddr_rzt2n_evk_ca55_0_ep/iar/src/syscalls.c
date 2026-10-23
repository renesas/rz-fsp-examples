/***********************************************************************************************************************
* File Name    : syscalls.c
* Description  : Contains data structures and functions used in the EP.
***********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include <stdint.h>
#include "sio_char.h"

#ifdef __ICCARM__
  #include <yfuns.h>
#if __VER__ < 8000000
  _STD_BEGIN
#endif
  #pragma module_name = "?__write"
#endif

/*******************************************************************************
* Function Name: __write
* Description  : IAR DLIB low-level write hook. Writes stdout/stderr to the console UART one
*              : character at a time via IoPutchar() (sio_char.c). A bare '\n' not already
*              : preceded by '\r' in the same write is expanded to "\r\n" -- this EP's own
*              : APP_PRINT strings already embed "\r\n" literally, so this only matters for
*              : anything printed without it (e.g. a bare '\n' from a library call).
* Arguments    : handle -- IAR file handle (_LLIO_STDOUT / _LLIO_STDERR / ...)
*              : buffer -- data to write
*              : size   -- number of bytes to write
* Return Value : Number of bytes written, or _LLIO_ERROR.
*******************************************************************************/
size_t __write (int handle, const unsigned char * buffer, size_t size)
{
    size_t offset;

    if ((_LLIO_STDOUT == handle) && (_LLIO_STDERR != handle))
    {
        for (offset = 0; offset < size; offset++)
        {
            if ('\n' == (char) buffer[offset])
            {
                if ((0 == offset) || ('\r' != (char) buffer[offset - 1U]))
                {
                    IoPutchar('\r');
                }
                IoPutchar('\n');
            }
            else
            {
                IoPutchar(buffer[offset]);
            }
        }
        return offset;
    }

    return _LLIO_ERROR;
}

/*******************************************************************************
* Function Name: __read
* Description  : IAR DLIB low-level read hook. Reads stdin from the console UART one character
*              : at a time via IoGetchar() (sio_char.c), echoing each character back (the
*              : console viewer relies on this, not local echo) and stopping at CR (written to
*              : the caller's buffer as '\n' and counted). A blocking, line-oriented read this
*              : way always captures a whole burst typed quickly (e.g. "11") in one call, the
*              : same property the GCC/e2studio build's ring-buffer APP_READ relied on for its
*              : double-digit input rejection in ddr_example_read_choice().
* Arguments    : handle -- IAR file handle (_LLIO_STDIN / ...)
*              : buffer -- destination buffer
*              : size   -- number of bytes requested
* Return Value : Number of bytes read (including the trailing '\n'), or _LLIO_ERROR.
*******************************************************************************/
size_t __read (int handle, unsigned char * buffer, size_t size)
{
    size_t  offset;
    int32_t data_char;

    if (_LLIO_STDIN == handle)
    {
        for (offset = 0; offset < size; )
        {
            data_char = IoGetchar();
            if (-1 == data_char)
            {
                return _LLIO_ERROR;
            }

            if ('\r' == (char) data_char)
            {
                buffer[offset] = (unsigned char) '\n';
                IoPutchar('\r');
                IoPutchar('\n');
                offset++;
                break;
            }
            else if ('\b' == (char) data_char)
            {
                if (offset > 0U)
                {
                    IoPutchar('\b');
                    IoPutchar(' ');
                    IoPutchar('\b');
                    offset--;
                }
            }
            else
            {
                buffer[offset] = (unsigned char) data_char;
                IoPutchar(data_char);
                offset++;
            }
        }
        return offset;
    }

    return _LLIO_ERROR;
}

#ifdef __ICCARM__
#if __VER__ < 8000000
  _STD_END
#endif
#endif

/* End of File */
