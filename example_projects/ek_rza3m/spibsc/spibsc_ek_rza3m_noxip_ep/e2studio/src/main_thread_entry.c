/***********************************************************************************************************************
 * File Name    : main_thread_entry.c
 * Description  : Contains data structures and functions used in main_thread_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2025 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "main_thread.h"
#include "spi_flash_example.h"

/***********************************************************************************************************************
 * @addtogroup r_spibsc_ep
 * @{
 **********************************************************************************************************************/

/* Screen control code */
#define CLEAR_SCREEN         "\e[2J"
#define ERASE_ROW            "\e[2K"

/* Other parameter */
#define BLINKING_DURATION    (100 / 2)
#define WRITE_BUFFER_SIZE    (4096)

/* Input class definitions */
#define INPUT_CLASS_MAP(d0, d1, d2, d3, d4, d5, d6,                                                                 \
                        d7)    ((d0) << 0 | (d1) << 1 | (d2) << 2 | (d3) << 3 | (d4) << 4 | (d5) << 5 | (d6) << 6 | \
                                (d7) << 7)
#define INPUT_CLASS_NUM      (0)
#define INPUT_CLASS_HEX      (1)

/* Flash guard area, discarding write/erase for top of this */
#define FLASH_GUARD          (0x40000)

static inline uint32_t internal_min_uint32 (uint32_t a, uint32_t b) {
    return a < b ? a : b;
};

static void     uart_callback (uart_callback_args_t * arg);
void            vApplicationTickHook (void);
static void     uart_write (const char * msg);
static void     uart_flush (void);
static void     input_clear (void);
static size_t   input_sub (int class);
static uint32_t xtoul (const char * str);
static uint32_t atoul (const char * str);
static void     print_greeting (void);
static void     print_top_menu (void);
static void     print_erase_size_prompt (void);
static void     print_erase_address_prompt (void);
static void     run_erase_and_verify (void);
static void     run_rnd_write_and_verify (void);
static bool     select_top_menu (bool show_menu);
static void     write_sub (uint32_t address, uint32_t size);

/* Callback argument for UART driver */
static uart_callback_args_t uart_arg;

/* UART completion blocker */
static volatile bool uart_blocker;

/* UART buffer */
static char * uart_buffer      = NULL;
static size_t uart_buffer_size = 0;

/* Input class definitions */
static uint8_t input_class[][12] =
{
    {                                            /* Number */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* !"#$%&' */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* ()*+,-./ */
        INPUT_CLASS_MAP(1, 1, 1, 1, 1, 1, 1, 1), /* 01234567 */
        INPUT_CLASS_MAP(1, 1, 0, 0, 0, 0, 0, 0), /* 89:;<=>? */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* @ABCDEFG */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* HIJKLMNO */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* PQRSTUVW */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* XYZ[\]^_ */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* `abcdefg */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* hijklmno*/
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* pqrstuvw */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* xyz{|}~ */
    },
    {                                            /* HexaDecimal */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* !"#$%&' */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* ()*+,-./ */
        INPUT_CLASS_MAP(1, 1, 1, 1, 1, 1, 1, 1), /* 01234567 */
        INPUT_CLASS_MAP(1, 1, 0, 0, 0, 0, 0, 0), /* 89:;<=>? */
        INPUT_CLASS_MAP(0, 1, 1, 1, 1, 1, 1, 0), /* @ABCDEFG */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* HIJKLMNO */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* PQRSTUVW */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* XYZ[\]^_ */
        INPUT_CLASS_MAP(0, 1, 1, 1, 1, 1, 1, 0), /* `abcdefg */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* hijklmno */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* pqrstuvw */
        INPUT_CLASS_MAP(0, 0, 0, 0, 0, 0, 0, 0), /* xyz{|}~ */
    },
};

/* External symbols */
extern const bsp_leds_t g_bsp_leds;

/* UART input buffer */
static char inbuf[32];

/***********************************************************************************************************************
 * @brief       This is the UART callback function.
 *              This function handles UART receive and transmit events for this example project.
 * @param[in]   p_args   Pointer to UART callback argument structure.
 * @retval      None.
 **********************************************************************************************************************/
static void uart_callback (uart_callback_args_t * arg)
{
    if (arg->event & UART_EVENT_RX_CHAR)
    {
        /* add received char to rx queue */
        xQueueSendToBackFromISR(g_uart_rx_queue, &arg->data, NULL);
    }

    if (arg->event & UART_EVENT_TX_DATA_EMPTY)
    {
        /* Completely stored the pending data */
        uart_blocker = false;
    }
}

/***********************************************************************************************************************
 * @brief       This is the FreeRTOS tick hook function.
 *              This function toggles the user LED at a fixed interval to indicate the application is running.
 * @param[in]   None.
 * @return      None.
 **********************************************************************************************************************/
void vApplicationTickHook(void)
{
    static unsigned int count_coarse   = 0;
    static unsigned int count_fraction = 0;
    count_fraction++;
    if (count_fraction >= BLINKING_DURATION)
    {
        count_fraction = 0;
        count_coarse++;
        if (g_bsp_leds.led_count)
        {
            R_BSP_PinWrite(g_bsp_leds.p_leds[0], (count_coarse & 1) ? BSP_IO_LEVEL_HIGH : BSP_IO_LEVEL_LOW);
        }
    }
}

/***********************************************************************************************************************
 * @brief       This is the UART write function.
 *              This function sends a null-terminated string to the UART console.
 * @param[in]   msg   Pointer to the string buffer to be transmitted.
 * @retval      None.
 **********************************************************************************************************************/
static void uart_write(const char * msg)
{
    uart_flush();

    /* Make copy of message */
    size_t length = strlen(msg) +1;
    size_t alloc_size = length +UART_TX_BUFFER_ALIGNMENT_MASK;
    alloc_size &= (~UART_TX_BUFFER_ALIGNMENT_MASK);

    if (uart_buffer_size < alloc_size)
    {
        void * alloc = pvPortMalloc(alloc_size);
        assert(alloc);
        if (uart_buffer)
        {
            vPortFree(uart_buffer);
        }
        uart_buffer      = (char *) alloc;
        uart_buffer_size = alloc_size;
    }

    /* Pass copied string to the UART driver */
    memcpy(uart_buffer, msg, length);
    uart_blocker = true;
    g_uart_console.p_api->write(g_uart_console.p_ctrl, (uint8_t *)uart_buffer, (uint32_t)length - 1);
}

/***********************************************************************************************************************
 * @brief       This is the UART flush function.
 *              This function blocks execution until the previous UART transmission is completed.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void uart_flush(void)
{
    /* Wait for complete */
    while (uart_blocker)
    {
        taskYIELD();
    }
}

/***********************************************************************************************************************
 * @brief       This function clears the UART transmit queue.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void input_clear(void)
{
    xQueueReset(g_uart_tx_queue);
}

/***********************************************************************************************************************
 * @brief       This is the UART input subroutine function.
 *              This function receives characters from the UART RX queue until a carriage return is detected.
 * @param[in]   class   Input character class used to filter allowed characters.
 * @retval      Length of the received input string.
 **********************************************************************************************************************/
static size_t input_sub(int class)
{
    /* Check limits of class number */
    if (class < 0)
    {
        return 0;
    }

    if (class >= (int) (sizeof(input_class) / 12))
    {
        return 0;
    }

    /* Clear buffer and input queue */
    input_clear();
    memset(inbuf, 0, sizeof(inbuf));
    unsigned int csrpos = 0;

    /* Input until CR */
    char input;
    while (true)
    {
        rand();

        /* Receive from queue */
        xQueueReceive(g_uart_rx_queue, &input, portMAX_DELAY);

        /* Check for input */
        if (0x0d == input)
        {
            break;
        }

        if ((0x08 == input) || (0x7f == input))
        {
            if (csrpos)
            {
                uart_write("\x08 \x08");
                csrpos--;
                inbuf[csrpos] = 0;
            }
            continue;
        }

        if ((input < ' ') || (input >= 0x7f))
        {
            continue;
        }

        if (csrpos >= sizeof(inbuf) - 1)
        {
            continue;
        }

        if (input_class[class][(input - 32) / 8] & (1 << input % 8))
        {
            /* Store into buffer */
            inbuf[csrpos] = input;
            uart_write(&inbuf[csrpos]);
            csrpos++;
        }
    }
    uart_write("\r");
    return (strlen(inbuf));
}

/***********************************************************************************************************************
 * @brief       This function converts a null-terminated hexadecimal string to a 32-bit unsigned integer value.
 * @param[in]   str  Pointer to the hexadecimal string to be converted.
 * @retval      Converted unsigned 32-bit value, or 0 if overflow or invalid character is detected.
 **********************************************************************************************************************/
static uint32_t xtoul(const char * str)
{
    uint32_t value = 0;
    while (*str)
    {
        /* Overflow check */
        if (value >= 0x10000000)
        {
            return 0;
        }

        /* Convert body */
        if (((*str) >= '0') && ((*str) <= '9'))
        {
            value *= 16;
            value += (uint32_t) (*str) - '0';
        }
        else if ((((*str) >= 'a') && ((*str) <= 'f')) || (((*str) >= 'A') && ((*str) <= 'F')))
        {
            /* Temporarily convert to lowercase */
            char c = (*str) | 0x20;
            value *= 16;
            value += (uint32_t) c - 'a' + 10;
        }
        else
        {
            /* Stop convert */
            break;
        }
        str++;
    }
    return value;
}

/***********************************************************************************************************************
 * @brief       This function converts a null-terminated decimal string to a 32-bit unsigned integer value.
 * @param[in]   str  Pointer to the decimal string to be converted.
 * @retval      Converted unsigned 32-bit value, or 0 if overflow or invalid character is detected.
 **********************************************************************************************************************/
static uint32_t atoul(const char * str)
{
    uint32_t value = 0;
    while (*str)
    {
        /* Overflow check */
        if (value >= 4294967281)
        {
            return 0;
        }

        /* Convert body */
        if ((*str >= '0') && (*str <= '9'))
        {
            value *= 10;
            value += (uint32_t) *str - '0';
        }
        else
        {
            /* Stop convert */
            break;
        }
        str++;
    }
    return value;
}

/***********************************************************************************************************************
 * @brief       This function clears the console screen and prints the banner, FSP version, and example information.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void print_greeting(void)
{
    /* Put the runtime-built TX buffer in non-cached RAM so the UART sees fresh data */
    static char fsp_version_str[32]  __attribute__((section("UNCACHED_BSS"), aligned(64)));
    snprintf(fsp_version_str, sizeof(fsp_version_str), "%u.%u.%u",
            (unsigned) FSP_VERSION_MAJOR,
            (unsigned) FSP_VERSION_MINOR,
            (unsigned) FSP_VERSION_PATCH);

    /* Move the console cursor to row 3, column 1 before printing the banner */
    uart_write(CLEAR_SCREEN "\e[3;1H" BANNER_1);
    uart_write(fsp_version_str);
    uart_write(BANNER_2);
    uart_write(BANNER_3);
    uart_write(EP_INFO);
}

/***********************************************************************************************************************
 * @brief       This function outputs the main example selection menu to the UART console.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void print_top_menu(void)
{
    uart_write("\r\nSelect example\r\n"
               "1 - Erase and Verify\r\n"
               "2 - Write Random Data and Verify\r\n");
}

/***********************************************************************************************************************
 * @brief       This function outputs a prompt requesting the erase size from the user.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void print_erase_size_prompt(void)
{
    uart_write("\n\nEnter erase size (0 = back to main menu)\r\n");
}

/***********************************************************************************************************************
 * @brief       This function outputs a prompt requesting the erase start address from the user.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void print_erase_address_prompt(void)
{
    uart_write("\n\nEnter erase address (flash address, 0 = back to main menu)\r\n0x");
}

/***********************************************************************************************************************
 * @brief       This function outputs a prompt requesting the write size from the user.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void print_write_size_prompt(void)
{
    uart_write("\n\nEnter write size (0 = back to main menu)\r\n");
}

/***********************************************************************************************************************
 * @brief       This is the write address prompt display function.
 *              This function outputs a prompt requesting the write start address from the user.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void print_write_address_prompt(void)
{
    uart_write("\n\nEnter write address (flash address, 0 = back to main menu)\r\n0x");
}

/***********************************************************************************************************************
 * @brief       This function handles user input, performs SPI flash erase operation, and reports the result.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void run_erase_and_verify(void)
{
    print_erase_address_prompt();

    /* Wait for input */
    while (0 == input_sub(INPUT_CLASS_HEX))
    {
        ;
    }

    uint32_t address = xtoul(inbuf);

    uart_write("\n");

    /* Check for return to main menu */
    if (0 == address)
    {
        return;
    }

    /* Check address to guard example program area */
    if (address < FLASH_GUARD)
    {
        uart_write("The input address is overlapped with the example program.\r\n");

        return;
    }

    print_erase_size_prompt();

    /* Wait for input */
    while (0 == input_sub(INPUT_CLASS_NUM))
    {
        ;
    }

    uint32_t size = atoul(inbuf);

    uart_write("\n");

    /* Check for return to main menu */
    if (0 == size)
    {
        return;
    }
    uart_write("\nErasing...");
    uart_flush();

    /* Execute erase example code with measuring the duration */
    fsp_err_t    result = example_erase(address, size);
    char const * errstr;

    switch (result)
    {
        case FSP_SUCCESS:
        {
            errstr = " success\r\n";
            break;
        }
        case FSP_ERR_INVALID_ADDRESS:
        {
            errstr = " address error\r\n";
            break;
        }
        case FSP_ERR_INVALID_SIZE:
        {
            errstr = " size error\r\n";
            break;
        }
        default:
            errstr = " failure\r\n";
    }
    uart_write(errstr);
}

/***********************************************************************************************************************
 * @brief       This function writes random data to SPI flash in chunks and reports the write result.
 * @param[in]   address         Start address of the flash write operation.
 * @param[in]   size            Total size of data to be written.
 * @retval      None.
 **********************************************************************************************************************/
static void write_sub(uint32_t address, uint32_t size)
{
    fsp_err_t result = FSP_SUCCESS;

    /* Initialize writing data */
    uint8_t * buffer = pvPortMalloc(WRITE_BUFFER_SIZE);
    if (!buffer)
    {
        return;
    }

    /* Execute erase example code with measuring the duration */
    uart_write("\nWriting...");
    uart_flush();

    while ((!!size) && (FSP_SUCCESS == result))
    {
        for (uint32_t offset = 0; offset < WRITE_BUFFER_SIZE; offset++)
        {
            buffer[offset] = (uint8_t)(rand() &255);
        }

        uint32_t part_size = internal_min_uint32(WRITE_BUFFER_SIZE, size);
        result   = example_write(buffer, address, part_size);
        size    -= part_size;
        address += part_size;
    }

    if (FSP_SUCCESS != result)
    {
        uart_write(" failure\r\n");
    }
    else
    {
        uart_write(" success\r\n");
    }
    vPortFree(buffer);
}

/***********************************************************************************************************************
 * @brief       This function handles user input, performs random data write to SPI flash, and verifies the operation.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void run_rnd_write_and_verify(void)
{
    print_write_address_prompt();

    /* Wait for input */
    while (0 == input_sub(INPUT_CLASS_HEX))
    {
        ;
    }

    uint32_t address = xtoul(inbuf);

    uart_write("\n");

    /* Check for return to main menu */
    if (0 == address)
    {
        return;
    }

    /* Check address to guard example program area */
    if (address < FLASH_GUARD)
    {
        uart_write("The input address is overlapped with the example program.\r\n");

        return;
    }

    print_write_size_prompt();

    /* Wait for input */
    while (0 == input_sub(INPUT_CLASS_NUM))
    {
        ;
    }

    uint32_t size = atoul(inbuf);

    uart_write("\n");

    /* Check for return to main menu */
    if (0 == size)
    {
        return;
    }
    write_sub(address, size);
}

/***********************************************************************************************************************
 * @brief       This is the top menu selection function.
 *              This function processes the user menu selection and executes the corresponding example operation.
 * @param[in]   show_menu       Flag indicating whether the top menu should be displayed.
 * @retval      true            A valid menu item was selected and executed.
 * @retval      false           Invalid menu selection or return to menu.
 **********************************************************************************************************************/
static bool select_top_menu(bool show_menu)
{
    if (show_menu)
    {
        print_top_menu();
    }
    input_sub(INPUT_CLASS_NUM);

    /* Check input number */
    int num = atoi(inbuf);
    switch (num)
    {
        case 1:
        {
            run_erase_and_verify();
            return true;
        }

        case 2:
        {
            run_rnd_write_and_verify();
            return true;
        }

        default:
            break;
    }
    uart_write(ERASE_ROW);
    return false;
}

/***********************************************************************************************************************
 * @brief       This function initializes UART and SPI flash,
 *              displays the greeting message, and handles the main menu loop.
 * @param[in]   pvParameters    Pointer to task parameters (unused).
 * @retval      None.
 **********************************************************************************************************************/
void main_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);
    fsp_err_t result;

    /* Open UART console and check error */
    result = g_uart_console.p_api->open(g_uart_console.p_ctrl, g_uart_console.p_cfg);
    if (FSP_SUCCESS != result)
    {
        return;
    }

    /* Enable callback */
    result = g_uart_console.p_api->callbackSet(g_uart_console.p_ctrl, uart_callback, NULL, &uart_arg);
    if (FSP_SUCCESS != result)
    {
        return;
    }
    print_greeting();

    /* Open spi_flash and check error */
    result = R_SPIBSC_Open(&g_spi_flash_ctrl, &g_spi_flash_cfg);
    if (FSP_SUCCESS != result)
    {
        uart_write(CLEAR_SCREEN "Failure to open g_spi_flash\r\n");
        g_uart_console.p_api->close(g_uart_console.p_ctrl);
        return;
    }
    example_init();
    bool show_menu = true;
    while (1)
    {
        show_menu = select_top_menu(show_menu);
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_spibsc_ep)
 **********************************************************************************************************************/
