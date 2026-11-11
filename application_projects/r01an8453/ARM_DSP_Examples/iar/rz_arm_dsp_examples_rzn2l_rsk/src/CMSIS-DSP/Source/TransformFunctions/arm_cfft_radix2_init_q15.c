/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cfft_radix2_init_q15.c
 * Description:  Radix-2 Decimation in Frequency Q15 FFT & IFFT initialization function
 *
 * $Date:        23 April 2021
 * $Revision:    V1.9.0
 *
 * Target Processor: Cortex-M and Cortex-A cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2021 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dsp/transform_functions.h"
#include "arm_common_tables.h"


/**
  @addtogroup ComplexFFTDeprecated
  @{
 */

/**
  @brief                        Initialization function for the Q15 CFFT/CIFFT.
  @deprecated                   Do not use this function. It has been superseded by \ref arm_cfft_q15 and will be removed
  @param[in,out] S              points to an instance of the Q15 CFFT/CIFFT structure.
  @param[in]     fftLen         length of the FFT.
  @param[in]     ifftFlag       flag that selects transform direction
                   - value = 0: forward transform
                   - value = 1: inverse transform
  @param[in]     bitReverseFlag flag that enables / disables bit reversal of output
                   - value = 0: disables bit reversal of output
                   - value = 1: enables bit reversal of output
  @return        execution status
                   - \ref ARM_MATH_SUCCESS        : Operation successful
                   - \ref ARM_MATH_ARGUMENT_ERROR : <code>fftLen</code> is not a supported length

  @par           Details
                   The parameter <code>ifftFlag</code> controls whether a forward or inverse transform is computed.
                   Set(=1) ifftFlag for calculation of CIFFT otherwise  CFFT is calculated
  @par
                   The parameter <code>bitReverseFlag</code> controls whether output is in normal order or bit reversed order.
                   Set(=1) bitReverseFlag for output to be in normal order otherwise output is in bit reversed order.
  @par
                   The parameter <code>fftLen</code> Specifies length of CFFT/CIFFT process. Supported FFT Lengths are 16, 64, 256, 1024.
  @par
                   This Function also initializes Twiddle factor table pointer and Bit reversal table pointer.
*/

#if defined(ARM_CFFT_RADIX2_Q15_SAMPALL)
#define ARM_CFFT_RADIX2_Q15_SAMP16
#define ARM_CFFT_RADIX2_Q15_SAMP32
#define ARM_CFFT_RADIX2_Q15_SAMP64
#define ARM_CFFT_RADIX2_Q15_SAMP128
#define ARM_CFFT_RADIX2_Q15_SAMP256
#define ARM_CFFT_RADIX2_Q15_SAMP512
#define ARM_CFFT_RADIX2_Q15_SAMP1024
#define ARM_CFFT_RADIX2_Q15_SAMP2048
#define ARM_CFFT_RADIX2_Q15_SAMP4096
#endif

#if defined(ARM_CFFT_RADIX2_Q15_SAMP16)   || defined(ARM_CFFT_RADIX2_Q15_SAMP32)  || defined(ARM_CFFT_RADIX2_Q15_SAMP64)   || defined(ARM_CFFT_RADIX2_Q15_SAMP128)  \
 || defined(ARM_CFFT_RADIX2_Q15_SAMP256)  || defined(ARM_CFFT_RADIX2_Q15_SAMP512) || defined(ARM_CFFT_RADIX2_Q15_SAMP1024) || defined(ARM_CFFT_RADIX2_Q15_SAMP2048) \
 || defined(ARM_CFFT_RADIX2_Q15_SAMP4096) || defined(ARM_CFFT_RADIX2_Q15_SAMPDEF)
 ARM_DSP_ATTRIBUTE arm_status arm_cfft_radix2_init_q15(
  arm_cfft_radix2_instance_q15 * S,
  uint16_t fftLen,
  uint8_t ifftFlag,
  uint8_t bitReverseFlag)
{
   /*  Initialise the default arm status */
  arm_status status = ARM_MATH_ARGUMENT_ERROR;

  /*  Initialise the default arm status */
  status = ARM_MATH_SUCCESS;

  /*  Initialise the FFT length */
  S->fftLen = fftLen;

  /*  Initialise the Flag for selection of CFFT or CIFFT */
  S->ifftFlag = ifftFlag;
  /*  Initialise the Flag for calculation Bit reversal or not */
  S->bitReverseFlag = bitReverseFlag;

  /*  Initializations of structure parameters depending on the FFT length */
  switch (S->fftLen)
  {
#if defined(ARM_CFFT_RADIX2_Q15_SAMP4096)
  case 4096U:
    /*  Initializations of structure parameters for 4096 point FFT */

    /*  Initialise the Twiddle coefficient pointer */
    S->pTwiddle = (q15_t *) twiddleCoef_4096_q15;
    /*  Initialise the twiddle coef modifier value */
    S->twidCoefModifier = 1U;
    /*  Initialise the bit reversal table modifier */
    S->bitRevFactor = 1U;
    /*  Initialise the bit reversal table pointer */
    S->pBitRevTable = (uint16_t *) armBitRevTable;

    break;
#endif

#if defined(ARM_CFFT_RADIX2_Q15_SAMP2048)
  case 2048U:
    /*  Initializations of structure parameters for 2048 point FFT */

    /*  Initialise the Twiddle coefficient pointer */
    S->pTwiddle = (q15_t *) twiddleCoef_2048_q15;
    /*  Initialise the twiddle coef modifier value */
    S->twidCoefModifier = 1U;
    /*  Initialise the bit reversal table modifier */
    S->bitRevFactor = 1U;
    /*  Initialise the bit reversal table pointer */
    S->pBitRevTable = (uint16_t *) & armBitRevTable_2048;

    break;
#endif

#if defined(ARM_CFFT_RADIX2_Q15_SAMP1024)
  case 1024U:
    /*  Initializations of structure parameters for 1024 point FFT */

    /*  Initialise the Twiddle coefficient pointer */
    S->pTwiddle = (q15_t *) twiddleCoef_1024_q15;
    /*  Initialise the twiddle coef modifier value */
    S->twidCoefModifier = 1U;
    /*  Initialise the bit reversal table modifier */
    S->bitRevFactor = 1U;
    /*  Initialise the bit reversal table pointer */
    S->pBitRevTable = (uint16_t *) & armBitRevTable_1024;

    break;
#endif

#if defined(ARM_CFFT_RADIX2_Q15_SAMP512)
  case 512U:
    /*  Initializations of structure parameters for 512 point FFT */

    /*  Initialise the Twiddle coefficient pointer */
    S->pTwiddle = (q15_t *) twiddleCoef_512_q15;
    /*  Initialise the twiddle coef modifier value */
    S->twidCoefModifier = 1U;
    /*  Initialise the bit reversal table modifier */
    S->bitRevFactor = 1U;
    /*  Initialise the bit reversal table pointer */
    S->pBitRevTable = (uint16_t *) & armBitRevTable_512;

    break;
#endif

#if defined(ARM_CFFT_RADIX2_Q15_SAMP256)
  case 256U:
    /*  Initializations of structure parameters for 256 point FFT */

    /*  Initialise the Twiddle coefficient pointer */
    S->pTwiddle = (q15_t *) twiddleCoef_256_q15;
    /*  Initialise the twiddle coef modifier value */
    S->twidCoefModifier = 1U;
    /*  Initialise the bit reversal table modifier */
    S->bitRevFactor = 1U;
    /*  Initialise the bit reversal table pointer */
    S->pBitRevTable = (uint16_t *) & armBitRevTable_256;

    break;
#endif

#if defined(ARM_CFFT_RADIX2_Q15_SAMP128)
  case 128U:
    /*  Initializations of structure parameters for 128 point FFT */

    /*  Initialise the Twiddle coefficient pointer */
    S->pTwiddle = (q15_t *) twiddleCoef_128_q15;
    /*  Initialise the twiddle coef modifier value */
    S->twidCoefModifier = 1U;
    /*  Initialise the bit reversal table modifier */
    S->bitRevFactor = 1U;
    /*  Initialise the bit reversal table pointer */
    S->pBitRevTable = (uint16_t *) & armBitRevTable_128;

    break;
#endif

#if defined(ARM_CFFT_RADIX2_Q15_SAMP64)
  case 64U:
    /*  Initializations of structure parameters for 64 point FFT */

    /*  Initialise the Twiddle coefficient pointer */
    S->pTwiddle = (q15_t *) twiddleCoef_64_q15;
    /*  Initialise the twiddle coef modifier value */
    S->twidCoefModifier = 1U;
    /*  Initialise the bit reversal table modifier */
    S->bitRevFactor = 1U;
    /*  Initialise the bit reversal table pointer */
    S->pBitRevTable = (uint16_t *) & armBitRevTable_64;

    break;
#endif

#if defined(ARM_CFFT_RADIX2_Q15_SAMP32)
  case 32U:
    /*  Initializations of structure parameters for 32 point FFT */

    /*  Initialise the Twiddle coefficient pointer */
    S->pTwiddle = (q15_t *) twiddleCoef_32_q15;
    /*  Initialise the twiddle coef modifier value */
    S->twidCoefModifier = 1U;
    /*  Initialise the bit reversal table modifier */
    S->bitRevFactor = 1U;
    /*  Initialise the bit reversal table pointer */
    S->pBitRevTable = (uint16_t *) & armBitRevTable_32;

    break;
#endif

#if defined(ARM_CFFT_RADIX2_Q15_SAMP16)
  case 16U:
    /*  Initializations of structure parameters for 16 point FFT */

    /*  Initialise the Twiddle coefficient pointer */
    S->pTwiddle = (q15_t *) twiddleCoef_16_q15;
    /*  Initialise the twiddle coef modifier value */
    S->twidCoefModifier = 1U;
    /*  Initialise the bit reversal table modifier */
    S->bitRevFactor = 1U;
    /*  Initialise the bit reversal table pointer */
    S->pBitRevTable = (uint16_t *) & armBitRevTable_16;

    break;
#endif

  default:
    /*  Reporting argument error if fftSize is not valid value */
    status = ARM_MATH_ARGUMENT_ERROR;
    break;
  }

  return (status);
}
#endif

/**
  @} end of ComplexFFTDeprecated group
 */
