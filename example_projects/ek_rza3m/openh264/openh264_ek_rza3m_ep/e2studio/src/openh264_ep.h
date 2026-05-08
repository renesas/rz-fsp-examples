/***********************************************************************************************************************
 * File Name    : openh264_ep.h
 * Description  : Contains declarations of data structures used in image_thread_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef OPENH264_EP_H
#define OPENH264_EP_H

#include "codec_api.h"
#include "common_utils.h"

#define MAX_NALS     (1024*10) /* Maximum number of NAL units that can be parsed from a single H.264 bitstream */
#define FRAME_MAX    (100)     /* Maximum number of frames tracked for timing and performance statistics */

/* Macros default values */
#define VIDEO_FPS_DEFAULT       (20U)
#define INITIAL_FRAME_COUNT     (0U)

#define EP_INFO     "The example project demonstrates H.264 video decoding and image rendering\r\n"\
                    "on the Renesas RZ/A3M MPU using the OpenH264 library and Renesas FSP.\r\n"\
                    "The input data, including YUV images and H.264 video streams, is embedded into the\r\n"\
                    "application as binary arrays via an assembly file (dat.asm). These arrays are parsed\r\n"\
                    "and decoded on the RZ/A3M board to produce YUV frames suitable for display on LCD.\r\n"\
                    "API Failure messages and performance statistics will be displayed on Terminal Emulator.\r\n\r\n"

#ifdef __cplusplus
extern "C" {
#endif

fsp_err_t h264_start_decode_task (void);
fsp_err_t h264_stop_decode_task (void);
fsp_err_t h264_decode_task (const uint8_t * h264_data, uint32_t h264_data_size);

#ifdef __cplusplus
}
#endif

#endif /* OPENH264_EP_H */
