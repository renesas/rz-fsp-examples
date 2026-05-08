/***********************************************************************************************************************
 * File Name    : image_thread_entry.c
 * Description  : Contains data structures and functions used in image_thread_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "ili9881.h"
#include "openh264_ep.h"

/*******************************************************************************************************************//**
 * @addtogroup r_openh264_ep
 * @{
 **********************************************************************************************************************/

/* Flag indicating completion of a MIPI DSI command transmission */
volatile bool g_message_sent = false;

/* File data block information */
typedef struct
{
    const uint8_t * data;             /* Pointer to data */
    uint32_t        len;              /* Length of the data */
    bool            isEndOfFile;      /* Has reached file end */
    bool            isStartOfFile;    /* Is start of file */
} file_data_block_t;

/* Embedded H.264 video stream and image data buffers for display */
extern const uint8_t video_h264_data[];
extern const uint8_t video_h264_finish[];
extern const uint8_t page1_data[];
extern const uint8_t page1_data_finish[];
extern const uint8_t page2_data[];
extern const uint8_t page2_data_finish[];
extern const uint8_t page3_data[];
extern const uint8_t page3_data_finish[];
extern const uint8_t page4_data[];
extern const uint8_t page4_data_finish[];
extern const uint8_t page5_data[];
extern const uint8_t page5_data_finish[];

/* Private global variables */
const uint8_t * input_buffer;
static uint32_t current_frame;
static uint32_t start_time_ms;
static int      first_show;
static uint32_t video_fps;

/* FPS statistics (decode / display) */
uint32_t g_decode_frame_cnt = 0;
uint32_t g_display_frame_cnt = 0;
uint32_t g_decode_start_ms = 0;
uint32_t g_display_start_ms = 0;

/* CPU busy time (approximate) */
static uint32_t g_cpu_busy_start_ms = 0;
static uint32_t g_cpu_busy_end_ms   = 0;

uint8_t image_index = 0;

int32_t frame_time[FRAME_MAX];
int32_t frame_wait[FRAME_MAX];

volatile uint8_t g_flag_lcdc_frameend;

static void test_display_image_yuv (void);
void        display_opening_image (void);

void lcd_draw_yuv (const uint8_t * Y, const uint8_t * U, const uint8_t * V,
                   uint16_t video_width, uint16_t video_height,
                   uint32_t Y_Stride, uint32_t UV);

void print_frame_summary(void);

/***********************************************************************************************************************
 * @brief      This is FreeRTOS task for H.264 decoding and LCD display.
 * @param[in]  pvParameters    Pointer to data.
 * @retval     None.
 **********************************************************************************************************************/
void image_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);

    fsp_err_t          err        = FSP_SUCCESS;
    fsp_pack_version_t version    = {RESET_VALUE};
    file_data_block_t  blk;

    g_flag_lcdc_frameend          = 0;
    video_fps                     = VIDEO_FPS_DEFAULT;
    current_frame                 = INITIAL_FRAME_COUNT;

    /* Get the FSP version */
    R_FSP_VersionGet(&version);

    /* Example project information printed on console */
    APP_PRINT(BANNER_INFO, EP_VERSION, version.version_id_b.major, version.version_id_b.minor,
                version.version_id_b.patch);
    APP_PRINT(EP_INFO);

    /* Open LCDC driver */
    err = R_LCDC_Open(g_display.p_ctrl, g_display.p_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* LCDC open failed */
        APP_ERR_PRINT("** R_LCDC_Open API failed **\r\n\r\n");
        APP_ERR_TRAP(err);
    }
    else
    {
        APP_PRINT("** LCDC driver initialization successful **\r\n\r\n");
    }

    /* Start LCDC display output */
    err = R_LCDC_Start(g_display.p_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* LCDC initialization failed */
        APP_ERR_PRINT("** R_LCDC_Start API failed **\r\n\r\n");
        APP_ERR_TRAP(err);
    }
    else
    {
        APP_PRINT("** LCDC driver start successful **\r\n\r\n");
    }

    /* Initialize MIPI DSI driver */
    err = R_MIPI_DSI_B_Open(&g_mipi_dsi0_ctrl, &g_mipi_dsi0_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* MIPI DSI initialization failed */
        APP_ERR_PRINT("** R_MIPI_DSI_B_Open API failed **\r\n\r\n");
        APP_ERR_TRAP(err);
    }
    else
    {
        APP_PRINT("** MIPI DSI driver initialization successful **\r\n\r\n");
    }

    /* Initialize ili9881 device */
    ili9881_init(&g_mipi_dsi0_ctrl);

    /* Start MIPI DSI display output */
    err = R_MIPI_DSI_B_Start(g_mipi_dsi0.p_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* MIPI DSI start failed */
        APP_ERR_PRINT("** R_MIPI_DSI_B_Start API failed **\r\n");
        APP_ERR_TRAP(err);
    }
    else
    {
        APP_PRINT("** MIPI DSI driver start successful **\r\n\r\n");
    }

    test_display_image_yuv();
    err = h264_start_decode_task();

    if (FSP_SUCCESS != err)
    {
        /* H264 decode failed */
        APP_ERR_PRINT("** h264_start_decode_task failed **\r\n");
        APP_ERR_TRAP(err);
    }
    else
    {
        APP_PRINT("** H264 start decoding successful **\r\n\r\n");
    }

    while (1)
    {
        start_time_ms     = 0;
        first_show        = 1;

        blk.data = &video_h264_data[0];
        blk.len = (uint32_t)(video_h264_finish - video_h264_data);
        blk.isStartOfFile = 1;
        blk.isEndOfFile   = 1;

        if (blk.len == 0)
        {
            APP_ERR_PRINT("Invalid video data size: 0 bytes\r\n");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        if (0 == video_fps)
        {
            video_fps = VIDEO_FPS_DEFAULT;
        }

        /* Reset FPS counters */
        g_decode_frame_cnt  = 0;
        g_display_frame_cnt = 0;
        g_decode_start_ms   = 0;
        g_display_start_ms  = 0;
        APP_PRINT("\r\nBegin H264 decoder task (size: %u bytes)\r\n", blk.len);

        /* Mark CPU busy period start */
        g_cpu_busy_start_ms =
            (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

        err = h264_decode_task(blk.data, blk.len);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("** h264_decode_task failed **\r\n");
            APP_ERR_TRAP(err);
        }

        /* Mark CPU busy period end */
        g_cpu_busy_end_ms =
            (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

        APP_PRINT("Process done!!!\r\n");
        print_frame_summary();

        /* Interval between next video play is 1000msec */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/***********************************************************************************************************************
 * @brief      This function handles display buffer change event at the end of a frame.
 * @param[in]  p_args    Callback arguments.
 * @retval     None.
 **********************************************************************************************************************/
void disp_buf_change(display_callback_args_t *p_args)
{
    if (NULL == p_args)
    {
        return;
    }

    if (DISPLAY_EVENT_FRAME_END == p_args->event)
    {
        g_flag_lcdc_frameend = 1;
    }
}

/***********************************************************************************************************************
 * @brief      This is callback functions for MIPI DSI interrupts.
 * @param[in]  p_args    Callback arguments.
 * @retval     None.
 **********************************************************************************************************************/
void mipi_dsi_callback(mipi_dsi_callback_args_t *p_args)
{
    if (NULL == p_args)
    {
        return;
    }
    if (MIPI_DSI_EVENT_SEQUENCE_0 == p_args->event)
    {
        g_message_sent = ((p_args->tx_status & MIPI_DSI_SEQUENCE_STATUS_DESCRIPTORS_FINISHED)
                           == MIPI_DSI_SEQUENCE_STATUS_DESCRIPTORS_FINISHED);
    }
    else
    {
        ; /* Do nothing */
    }
}

/***********************************************************************************************************************
 * @brief      This function copies YUV420 frame to LCD buffers and update display with frame timing control.
 * @param[IN]  Y               Pointer to the Y plane.
 * @param[IN]  U               Pointer to the U plane.
 * @param[IN]  V               Pointer to the V plane.
 * @param[IN]  video_width     Width of the video frame in pixels.
 * @param[IN]  video_height    Height of the video frame in pixels.
 * @param[IN]  Y_Stride        Stride (bytes per line) of the Y plane.
 * @param[IN]  UV_Stride       Stride (bytes per line) of the U/V planes.
 * @retval     None.
 **********************************************************************************************************************/
void lcd_draw_yuv(const uint8_t * Y, const uint8_t * U, const uint8_t * V,
                  uint16_t video_width, uint16_t video_height,
                  uint32_t Y_Stride, uint32_t UV_Stride)
{
    uint8_t       * p_buff_0;
    uint8_t       * p_buff_1;
    uint8_t       * p_buff_2;
    const uint8_t * p_data_0;
    const uint8_t * p_data_1;
    const uint8_t * p_data_2;
    uint32_t        height;

    if (1 == first_show)
    {
        display_opening_image();
        vTaskDelay(2000);  /* Show image for 2sec */

        current_frame = 0;
        start_time_ms = (uint32_t)(xTaskGetTickCount() *portTICK_PERIOD_MS);
        first_show    = 0;

        /* Start display FPS time base */
        g_display_start_ms = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

        /* Reset timing arrays */
        memset(frame_time, 0, sizeof(frame_time));
        memset(frame_wait, 0, sizeof(frame_wait));
    } else
    {
        current_frame++;
    }

    /* Initialize LCDC buffer */
    int32_t pass_time_ms      = ((int32_t)(xTaskGetTickCount() *portTICK_PERIOD_MS)) -(int32_t)start_time_ms;
    int32_t next_time_ms      = (((int32_t)current_frame *1000) / (int32_t)video_fps);
    int32_t remaining_time_ms = next_time_ms -pass_time_ms;

    if (remaining_time_ms > 0)
    {
        /* Wait until nest frame timing */
        vTaskDelay((TickType_t)remaining_time_ms);
    }

    if (current_frame < FRAME_MAX)
    {
        frame_time[current_frame] = ((int32_t)(xTaskGetTickCount() *portTICK_PERIOD_MS)) - (int32_t)start_time_ms;
        frame_wait[current_frame] = remaining_time_ms;
    }
    p_buff_0 = (uint8_t *)fb_foreground[0];
    p_buff_1 = fb_foreground_CB[0];
    p_buff_2 = fb_foreground_CR[0];

    p_data_0 = Y;
    p_data_1 = U;
    p_data_2 = V;

    for (height = 0; height < video_height; height++)
    {
        memcpy(p_buff_0, p_data_0, video_width);
        p_buff_0 += DISPLAY_BUFFER_STRIDE_BYTES_INPUT0;
        p_data_0 += Y_Stride;
    }
    for (height = 0; height < (video_height / 2); height++)
    {
        memcpy(p_buff_1, p_data_1, video_width/2);
        memcpy(p_buff_2, p_data_2, video_width/2);
        p_buff_1 += DISPLAY_BUFFER_STRIDE_UV_BYTES_INPUT0;
        p_buff_2 += DISPLAY_BUFFER_STRIDE_UV_BYTES_INPUT0;
        p_data_1 += UV_Stride;
        p_data_2 += UV_Stride;
    }
    R_LCDC_LayerChange(g_display.p_ctrl, &(g_display_runtime_cfg_bg), DISPLAY_FRAME_LAYER_1);

    /* Count displayed frame */
    g_display_frame_cnt++;
}

/***********************************************************************************************************************
 * @brief      This function calculates and displays a summary of the video decoding session including:
 *             - Total number of frames decoded
 *             - Total decode time in milliseconds
 *             - Actual frames per second (FPS) achieved
 *             The summary helps verify decoder performance and timing accuracy.
 * @param[in]  None.
 * @retval     None.
 **********************************************************************************************************************/
void print_frame_summary(void)
{
    uint32_t now_ms = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

    /* No frame decoded at all */
    if (g_decode_frame_cnt == 0U)
    {
        APP_PRINT("No frames decoded\r\n");
        return;
    }

    /* Decode FPS */
    if (g_decode_start_ms > 0U)
    {
        uint32_t decode_time = now_ms - g_decode_start_ms;
        if (decode_time > 0U)
        {
            float decode_fps =
                ((float)g_decode_frame_cnt * 1000.0f) / (float)decode_time;

            APP_PRINT("Decode  FPS : %u frames / %u ms (%.1f fps)\r\n",
                      g_decode_frame_cnt,
                      decode_time,
                      decode_fps);

            /* Approximate CPU load based on busy time */
            if ((g_cpu_busy_end_ms > g_cpu_busy_start_ms) &&
                (g_decode_start_ms > 0U))
            {
                uint32_t busy_time =
                    g_cpu_busy_end_ms - g_cpu_busy_start_ms;

                uint32_t wall_time =
                    (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS) -
                    g_decode_start_ms;

                if (wall_time > 0U)
                {
                    float cpu_load =
                        ((float)busy_time / (float)wall_time) * 100.0f;

                    if (cpu_load > 100.0f) cpu_load = 100.0f;

                    APP_PRINT("Estimated CPU load (decode+display): %.1f %%\r\n",
                              cpu_load);
                }
            }
        }
    }

    /* Display FPS */
    if ((g_display_frame_cnt > 0U) && (g_display_start_ms > 0U))
    {
        uint32_t display_time = now_ms - g_display_start_ms;
        if (display_time > 0U)
        {
            float display_fps =
                ((float)g_display_frame_cnt * 1000.0f) / (float)display_time;

            APP_PRINT("Display FPS : %u frames / %u ms (%.1f fps)\r\n",
                      g_display_frame_cnt,
                      display_time,
                      display_fps);
        }
    }

    /* Decode / display mismatch */
    if (g_decode_frame_cnt != g_display_frame_cnt)
    {
        APP_PRINT("Frame mismatch: decode=%u, display=%u (drop=%d)\r\n",
                  g_decode_frame_cnt,
                  g_display_frame_cnt,
                  (int32_t)g_decode_frame_cnt -
                  (int32_t)g_display_frame_cnt);
    }
}


/***********************************************************************************************************************
 * @brief      This function initializes the display buffer with a static test image
 *             in YUV420 format. Used for verifying display functionality before video playback.
 * @param[in]  None.
 * @retval     None.
 **********************************************************************************************************************/
static void test_display_image_yuv(void)
{
    uint8_t * p_buff_0;
    uint8_t * p_buff_1;
    uint8_t * p_buff_2;

    const uint8_t * p_data_0;
    const uint8_t * p_data_1;
    const uint8_t * p_data_2;
    uint32_t        height;

    uint32_t output_width = g_display_cfg.input[DISPLAY_FRAME_LAYER_1].hsize;
    uint32_t fb_y_stride  = DISPLAY_BUFFER_STRIDE_BYTES_INPUT0;
    uint32_t fb_uv_stride = DISPLAY_BUFFER_STRIDE_UV_BYTES_INPUT0;

    /* Initialize display buffer */
    APP_PRINT("** Initialize display buffer **\r\n\r\n");

    /* Get LCDC configuration */
    p_buff_0 = (uint8_t *)fb_foreground[0];
    p_buff_1 = fb_foreground_CB[0];
    p_buff_2 = fb_foreground_CR[0];

    input_buffer = page3_data;

    p_data_0 = input_buffer;
    p_data_1 = &input_buffer[480*720];
    p_data_2 = &input_buffer[((480*720)*5)/4];

    for (height = 0; height < 480; height++)
    {
        memcpy(p_buff_0, p_data_0, 720);
        p_buff_0 += fb_y_stride;
        p_data_0 += output_width;
    }

    for (height = 0; height < 240; height++)
    {
        memcpy(p_buff_1, p_data_1, 360);
        memcpy(p_buff_2, p_data_2, 360);
        p_buff_1 += fb_uv_stride;
        p_buff_2 += fb_uv_stride;
        p_data_1 += (output_width/2);
        p_data_2 += (output_width/2);
    }

    /* Display LCD screen */
    R_LCDC_LayerChange(g_display.p_ctrl, &(g_display_runtime_cfg_bg), DISPLAY_FRAME_LAYER_1);
}

/***********************************************************************************************************************
 * @brief      This function displays a splash screen selected from 5 predefined images.
 *             The image index cycles automatically on each call. Image format is YUV420 with
 *             resolution 720x1280 pixels. The displayed image is copied to LCD framebuffer
 *             and rendered via LCDC layer change.
 * @param[in]  None.
 * @retval     None.
 **********************************************************************************************************************/
void display_opening_image(void)
{
    uint8_t * p_buff_0;
    uint8_t * p_buff_1;
    uint8_t * p_buff_2;

    const uint8_t * p_data_0;
    const uint8_t * p_data_1;
    const uint8_t * p_data_2;
    uint32_t        height;

    uint32_t output_width  = g_display_cfg.input[DISPLAY_FRAME_LAYER_1].hsize;
    uint32_t output_height = g_display_cfg.input[DISPLAY_FRAME_LAYER_1].vsize;

    uint32_t fb_y_stride  = DISPLAY_BUFFER_STRIDE_BYTES_INPUT0;
    uint32_t fb_uv_stride = DISPLAY_BUFFER_STRIDE_UV_BYTES_INPUT0;

    /* Get LCDC configuration */
    p_buff_0 = (uint8_t *)fb_foreground[0];
    p_buff_1 = fb_foreground_CB[0];
    p_buff_2 = fb_foreground_CR[0];

    switch (image_index)
    {
        case 0:
            input_buffer = page1_data;
            break;
        case 1:
            input_buffer = page2_data;
            break;
        case 2:
            input_buffer = page3_data;
            break;
        case 3:
            input_buffer = page4_data;
            break;
        case 4:
        default:
            input_buffer = page5_data;
            break;
    }

    /* Page data in YUV format */
    p_data_0 = input_buffer;
    p_data_1 = &input_buffer[output_height *720];
    p_data_2 = &input_buffer[((output_height *720) *5)/4];


    for (height = 0; height < output_height; height++)
    {
        memcpy(p_buff_0, p_data_0, 720);
        p_buff_0 += fb_y_stride;
        p_data_0 += output_width;
    }

    for (height = 0; height < (output_height/2); height++)
    {
        memcpy(p_buff_1, p_data_1, 360);
        memcpy(p_buff_2, p_data_2, 360);
        p_buff_1 += fb_uv_stride;
        p_buff_2 += fb_uv_stride;
        p_data_1 += output_width/2;
        p_data_2 += output_width/2;
    }

    /* Display LCD screen */
    R_LCDC_LayerChange(g_display.p_ctrl, &(g_display_runtime_cfg_bg), DISPLAY_FRAME_LAYER_1);

    image_index++;
    if (5 == image_index)
    {
        image_index = 0;
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_openh264_ep)
 **********************************************************************************************************************/
