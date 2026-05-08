/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

	 .section ".rodata"
	 .align 8
	 .global page1_data
	 .global page1_data_finish
	 .global page2_data
	 .global page2_data_finish
	 .global page3_data
	 .global page3_data_finish
	 .global page4_data
	 .global page4_data_finish
	 .global page5_data
	 .global page5_data_finish
	 .global video_h264_data
	 .global video_h264_finish

page1_data:
	 .incbin "../src/data/Page1_720_1280.yuv"
page1_data_finish:
page2_data:
	 .incbin "../src/data/Page2_720_1280.yuv"
page2_data_finish:
page3_data:
	 .incbin "../src/data/Page3_720_1280.yuv"
page3_data_finish:
page4_data:
	 .incbin "../src/data/Page4_720_1280.yuv"
page4_data_finish:
page5_data:
	 .incbin "../src/data/Page5_720_1280.yuv"
page5_data_finish:
video_h264_data:
	 .incbin "../src/data/output_rotate_keep_res720x1280.264"
video_h264_finish:

.end
