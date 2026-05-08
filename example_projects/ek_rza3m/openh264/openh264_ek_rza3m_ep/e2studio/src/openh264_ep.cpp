/***********************************************************************************************************************
 * File Name    : openh264_ep.cpp
 * Description  : Contains data structures and functions used in openh264_ep.cpp.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "codec_api.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "common_utils.h"
#include "openh264_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_openh264_ep
 * @{
 **********************************************************************************************************************/

extern "C" {

void lcd_draw_yuv (const uint8_t * Y, const uint8_t * U, const uint8_t * V,
                   uint16_t video_width, uint16_t video_height,
                   uint32_t Y_Stride, uint32_t UV_Stride);
}

/* Parse H.264 Annex-B byte stream and split into NAL units */
static fsp_err_t parse_annexb (const uint8_t * buffer, uint32_t size);

/* H.264 NAL unit information */
typedef struct {
    const uint8_t * data;
    size_t          size;
} nal_unit_t;

ISVCDecoder * decoder;
nal_unit_t    nal_list[MAX_NALS];
uint32_t      nal_count;

/***********************************************************************************************************************
 * @brief      This function parses H.264 Annex-B byte stream and splits into NAL units.
 * @param[IN]  buffer    Pointer to input Annex-B stream buffer.
 *             size      Size of input buffer in bytes.
 * @retval     Any other error code apart from FSP_SUCCESS Unsuccessful parse.
 **********************************************************************************************************************/
static fsp_err_t parse_annexb(const uint8_t * buffer, uint32_t size)
{
    fsp_err_t err   = FSP_SUCCESS;
    uint32_t  pos   = 0;
    uint32_t  start = 0;
    nal_count       = 0;

    while ((pos + 4) < size)
    {
        if (memcmp(&buffer[pos], "\x00\x00\x00\x01", 4) == 0)
        {
            if ((pos > start) && (nal_count < MAX_NALS))
            {
                if (start < 3)
                {
                    nal_list[nal_count].data = &buffer[0];
                    nal_list[nal_count].size = pos;
                }
                else
                {
                    nal_list[nal_count].data = &buffer[start - 3];   /* Adjust start point to adapt to Open H.264 */
                    nal_list[nal_count].size = (pos - start) + 3;    /* Adjust NAL size */
                }

                nal_count++;

                if (nal_count >= MAX_NALS)
                {
                    err = FSP_ERR_INVALID_SIZE;
                    APP_ERR_PRINT("** parse_annexb failed **\r\n");
                    APP_ERR_TRAP(err);
                    return err;
                }
            }
            pos  += 4;
            start = pos;
        } else
        {
            pos++;
        }
    }

    if ((start < size) && (nal_count < MAX_NALS))
    {
        if (start < 3)
        {
            nal_list[nal_count].data = &buffer[0];
            nal_list[nal_count].size = size;
        }
        else
        {
            nal_list[nal_count].data = &buffer[start - 3];
            nal_list[nal_count].size = (size - start) + 3;
        }
        nal_count++;
    }

    return err;
}

/***********************************************************************************************************************
 * @brief      This function creates and initializes OpenH264 decoder with default decoding parameters.
 * @param[IN]  None.
 * @retval     Any other error code apart from FSP_SUCCESS Unsuccessful parse.
 **********************************************************************************************************************/
fsp_err_t h264_start_decode_task(void)
{
    fsp_err_t      err                   = FSP_SUCCESS;
    SDecodingParam decParam              = {};
    
    decoder                              = NULL;
    decParam.uiTargetDqLayer             = 255;
    decParam.eEcActiveIdc                = ERROR_CON_FRAME_COPY_CROSS_IDR;
    decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;
    decParam.sVideoProperty.size = sizeof(decParam.sVideoProperty);

    /* Create decoder */
    err = (fsp_err_t)WelsCreateDecoder(&decoder);

    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** WelsCreateDecoder failed **\r\n");
        APP_ERR_TRAP(err);
    }

    if (NULL == decoder)
    {
        APP_ERR_PRINT("** OpenH264 decoder creation failed **\r\n");
        APP_ERR_TRAP(FSP_ERR_NOT_INITIALIZED);
    }

    /* Initialize decoder */
    err = (fsp_err_t)decoder->Initialize(&decParam);

    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** Decoder initialization failed **\r\n");
        APP_ERR_TRAP(err);
        WelsDestroyDecoder(decoder);
        return err;
    }
    return err;
}

/***********************************************************************************************************************
 * @brief      This function stops and de-initializes the H.264 decoder.
 * @param[IN]  None.
 * @retval     Any other error code apart from FSP_SUCCESS Unsuccessful parse.
 **********************************************************************************************************************/
fsp_err_t h264_stop_decode_task(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* De-initialize the decoder */
    err = (fsp_err_t)decoder->Uninitialize();

    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** Decoder de-initialization failed **\r\n");
        APP_ERR_TRAP(err);
    }

    /* Destroy decoder */
    WelsDestroyDecoder(decoder);
    return err;
}

/***********************************************************************************************************************
 * @brief      This function decodes an H.264 Annex-B bitstream and renders decoded frames.
 * @param[IN]  h264_data       Pointer to the input H.264 Annex-B bitstream.
 * @param[IN]  h264_data_size  Size of the input bitstream in bytes.
 * @retval     Any other error code apart from FSP_SUCCESS Unsuccessful parse.
 **********************************************************************************************************************/
fsp_err_t h264_decode_task(const uint8_t *h264_data, uint32_t h264_data_size)
{
    fsp_err_t err = FSP_SUCCESS;
    nal_count     = 0;

    err = parse_annexb(h264_data, h264_data_size);

    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** parse_annexb failed **\r\n");
        APP_ERR_TRAP(err);
        return err;
    }

    SBufferInfo buf_info;
    memset(&buf_info, 0, sizeof(buf_info));
    uint8_t * yuv[3] = {NULL, NULL, NULL};
    DECODING_STATE state;
    for (uint32_t i = 0; i < nal_count; i++)
    {
        state = decoder->DecodeFrameNoDelay(
            nal_list[i].data, (int)nal_list[i].size,
            yuv, &buf_info);

        switch (state)
        {
            case dsErrorFree:

                /* Decode frame counter */
                extern uint32_t g_decode_frame_cnt;
                extern uint32_t g_decode_start_ms;

                if (0 == g_decode_frame_cnt)
                {
                    g_decode_start_ms = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
                }
                g_decode_frame_cnt++;
                lcd_draw_yuv(
                    yuv[0], yuv[1], yuv[2],
                    (uint16_t)buf_info.UsrData.sSystemBuffer.iWidth,
                    (uint16_t)buf_info.UsrData.sSystemBuffer.iHeight,
                    buf_info.UsrData.sSystemBuffer.iStride[0],
                    buf_info.UsrData.sSystemBuffer.iStride[1]
                );
                break;

            case dsNoParamSets:
                APP_PRINT("Decoder error: missing SPS/PPS\n");
                break;

            case dsBitstreamError:
                if (i < (nal_count - 5))
                {
                    APP_PRINT("Decoder error: bitstream corrupted NAL at index %d\r\n", i);
                }
                break;

            case dsRefLost:
                APP_PRINT("Decoder warning: lost reference frame\n");
                break;

            case dsInvalidArgument:
                APP_PRINT("Decoder error: invalid argument\n");
                break;

            default:
                APP_PRINT("Default: does not match any of the above cases\n");
                APP_PRINT("nal_count: %d\r\n",i);
                break;
        }
    }
    return err;
}

/***********************************************************************************************************************
 * @} (end addtogroup r_openh264_ep)
 **********************************************************************************************************************/
