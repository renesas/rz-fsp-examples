/***********************************************************************************************************************
 * File Name    : siorw.c
 * Description  : Low-level SIO read/write retargeting functions for the standard library.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "sio_char.h"
#ifndef __ICCARM__
#include <sys/stat.h>
#include <errno.h>
#endif

#ifdef __ICCARM__
  #include <yfuns.h>
#if __VER__ < 8000000
  _STD_BEGIN
#endif
  #pragma module_name = "?__write"
#endif

/*******************************************************************************
Typedef definitions
*******************************************************************************/


/*******************************************************************************
Macro definitions
*******************************************************************************/
/* File descriptor */
#ifndef __ICCARM__
  #define STDIN           (0)
  #define STDOUT          (1)
  #define STDERR          (2)
#endif

#define SIORW_SUCCESS   (0)
#define SIORW_ERROR     (-1)
#define SIORW_FLAG_OFF  (0)
#define SIORW_FLAG_ON   (1)

int _write(int file,char *ptr,int len);
int _read (int file, char *ptr, int len);
#ifndef __ICCARM__
/* EWARM migration: GCC/newlib syscall declarations. */
int _close(int file);
int _fstat(int file, struct stat *st);
int _getpid(void);
int _close(int file);
int _isatty(int file);
int _kill(int pid, int sig);
int _lseek(int file, int ptr, int dir);
#endif

/*******************************************************************************
Imported global variables and functions (from other files)
*******************************************************************************/


/*******************************************************************************
Exported global variables and functions (to be accessed by other files)
*******************************************************************************/


/*******************************************************************************
Private global variables and functions
*******************************************************************************/


/*******************************************************************************************************************//**
 * @brief       The character string specified by buffer is output for writing_b bytes to the serial port. The
 *              output is determined by file number file_no. The effective outputs in this version are STDOUT and
 *              STDERR, and both are output to the same serial port. The linefeed code '\n' (LF) is converted to
 *              '\r' (CR) + '\n' (LF) on output.
 * @param[in]   file_no      File number to be the target of writing
 * @param[in]   buffer       Pointer to the area in which the writing data is stored
 * @param[in]   writing_b    Writing bytes
 * @retval      >=0    Number of transmitted characters
 * @retval      -1     File number error
 **********************************************************************************************************************/
#ifndef __ICCARM__
int _write(int file,char *ptr, int len)
{
    /* Forward the standard library write request to SioWrite. */
    return (SioWrite(file,ptr,(uint32_t)len));
}

  int32_t SioWrite(int32_t file_no, const char_t * buffer, uint32_t writing_b)
#else
  size_t __write(int handle, const unsigned char * buffer, size_t size)
#endif
{
    uint32_t offset;

#ifndef __ICCARM__
    if ((STDOUT == file_no) || (STDERR == file_no))
    {
        for (offset = 0U; (offset < writing_b); offset++)
#else
    if ((_LLIO_STDOUT == handle) && (_LLIO_STDERR != handle))
    {
        for (offset = 0U; (offset < size); offset++)
#endif
        {
            /* Writing in buffer converting linefeed code */
            if (('\n') == (*(buffer + offset)))
            {
                if (0 == offset)
                {
                    IoPutchar('\r');
                }
                else
                {
                    if ((*(buffer + (offset - 1))) != ('\r'))
                    {
                        IoPutchar('\r');
                    }
                }
                IoPutchar('\n');
            }
            else
            {
                IoPutchar(*(buffer + offset));
            }
        }
        /* Return the number of characters transmitted. */
        return (int32_t)offset;
    }

#ifndef __ICCARM__
    return SIORW_ERROR;     /* File number error */
#else
    return _LLIO_ERROR;     /* File number error */
#endif
}

/*******************************************************************************************************************//**
 * @brief       The character string specified by buffer is read for reading_b bytes from the serial port. The
 *              input is determined by file number file_no. The effective input in this version is STDIN.
 * @param[in]   file_no      File number to be the target of reading
 * @param[in]   buffer       Pointer to the area in which the reading data is stored
 * @param[in]   reading_b    Reading bytes
 * @retval      >0    Number of received characters
 * @retval      -1    File number or receiving data error
 **********************************************************************************************************************/
#ifndef __ICCARM__
int _read (int file, char *ptr, int len)
{
    /* Forward the standard library read request to SioRead. */
    return (SioRead(file,ptr,(uint32_t)len));
}


  int32_t SioRead(int32_t file_no, char_t * buffer, uint32_t reading_b)
#else
  size_t __read(int handle, unsigned char * buffer, size_t size)
#endif
{
    int32_t        char_mem;
    int32_t        sp_char;
    uint32_t       offset;
    static int32_t sjis_flg = SIORW_FLAG_OFF;

#ifndef __ICCARM__
    if (STDIN == file_no)
    {
        for (offset = 0U; (offset < reading_b); )
#else
    if (_LLIO_STDIN == handle)
    {
        for (offset = 0U; (offset < size); )
#endif
        {
            /* Reading receiving data */
            char_mem = IoGetchar();

            if ((-1) == (char_mem))  /* -1 is returned when it is receiving data error */
            {
#ifndef __ICCARM__
                return SIORW_ERROR;
#else
                return _LLIO_ERROR;
#endif
            }

            if (SIORW_FLAG_ON == sjis_flg)
            {
                sjis_flg = SIORW_FLAG_OFF;
#ifndef __ICCARM__
                /* Echo the second byte of the SJIS character back to the terminal. */
                SioWrite(STDOUT, (char_t *)&char_mem, 1);
                /* Store the second byte of the SJIS character into the buffer. */
                *(buffer + offset) = (char_t)char_mem;
#else
                __write(_LLIO_STDOUT, (const unsigned char *)&char_mem, 1);
                *(buffer + offset) = (unsigned char)char_mem;
#endif
                offset++;
            }
            else if ((0x20 <= char_mem) && (char_mem <= 0x7E))
            {
                /* Data possible to display */
#ifndef __ICCARM__
                /* Echo the received character back to the terminal. */
                SioWrite(STDOUT, (char_t *)&char_mem, 1);
                /* Store the received character into the buffer. */
                *(buffer + offset) = (char_t)char_mem;
#else
                __write(_LLIO_STDOUT, (const unsigned char *)&char_mem, 1);
                *(buffer + offset) = (unsigned char)char_mem;
#endif
                offset++;
            }
            else if ('\b' == char_mem)      /* BS process */
            {
                if ((offset) > (0))
                {
                    sp_char = 0x20;
#ifndef __ICCARM__
                    /* Move the terminal cursor back over the previous character. */
                    SioWrite(STDOUT, (char_t *)&char_mem, 1);
                    /* Overwrite the previous character on the terminal with a space. */
                    SioWrite(STDOUT, (char_t *)&sp_char, 1);
                    /* Move the terminal cursor back again onto the erased position. */
                    SioWrite(STDOUT, (char_t *)&char_mem, 1);
#else
                    __write(_LLIO_STDOUT, (const unsigned char *)&char_mem, 1);
                    __write(_LLIO_STDOUT, (const unsigned char *)&sp_char, 1);
                    __write(_LLIO_STDOUT, (const unsigned char *)&char_mem, 1);
#endif
                    offset--;
                }
            }
            else if ('\r' == char_mem)      /* CR process */
            {
                *(buffer + offset) = '\n';
#ifndef __ICCARM__
                /* Echo the line feed to the terminal. */
                SioWrite(STDOUT, buffer + offset, 1);
#else
                __write(_LLIO_STDOUT, (const unsigned char*)(buffer + offset), 1);
#endif
                offset++;
                break;
            }
            /* Japanese SJIS ? */
            else if (((char_mem >= 0x80) && (char_mem < 0xA0)) || ((char_mem >= 0xE0) && (char_mem < 0xFE)))
            {
                /* Data possible to display */
#ifndef __ICCARM__
                /* Echo the first byte of the SJIS character back to the terminal. */
                SioWrite(STDOUT, (char_t *)&char_mem, 1);
                /* Store the first byte of the SJIS character into the buffer. */
                *(buffer + offset) = (char_t)char_mem;
#else
                __write(_LLIO_STDOUT, (const unsigned char*)&char_mem, 1);
                *(buffer + offset) = (unsigned char)char_mem;
#endif
                offset++;
                sjis_flg = SIORW_FLAG_ON;
            }
            /* Character cord out of the support */
            else
            {
                /* Do Nothing */
            }
        }
        /* Return the number of characters received. */
        return (int32_t)offset;
    }

#ifndef __ICCARM__
    return SIORW_ERROR;     /* File number error */
#else
    return _LLIO_ERROR;     /* File number error */
#endif
}

#ifndef __ICCARM__
/* EWARM migration: GCC/newlib syscall implementations. */
int _close(int file)
{
    (void) file;
    return -1;
}

int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    (void) file;
    return 0;
}

int _getpid(void)
{
    return 1;
}

int _isatty(int file)
{
    (void) file;
    return 1;
}

int _kill(int pid, int sig)
{
    errno = EINVAL;
    (void) pid;
    (void) sig;
    return -1;
}

int _lseek(int file, int ptr, int dir)
{
    (void) file;
    (void) ptr;
    (void) dir;
    return 0;
}
#endif

#ifdef __ICCARM__
#if __VER__ < 8000000
  _STD_END
#endif
#endif

/* End of File */
