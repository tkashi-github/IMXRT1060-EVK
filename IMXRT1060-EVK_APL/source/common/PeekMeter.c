/**
 * @file		PeekMeter.c
 * @brief		Meter Task 
 * @author		Takashi Kashiwagi
 * @date		2019/09/19
 * @version     0.1
 * @details 
 * --
 * License Type <MIT License>
 * --
 * Copyright 2019 Takashi Kashiwagi
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 * @par Update:
 * - 2019/09/19: Takashi Kashiwagi:
 */
#include "PeekMeter.h"
#include "common.h"
#include <stdlib.h>
#include "Task/MeterTask/MeterTask.h"

DefALLOCATE_DATA_DTCM static float32_t s_sfpACoef1[] = {
	1.53512485958697,
	-2.69169618940638,
	1.19839281085285,
};

DefALLOCATE_DATA_DTCM static float32_t s_sfpBCoef1[] = {
	1.00000000000000000000,
	-1.69065929318241,
	0.73248077421585,
};
DefALLOCATE_DATA_DTCM static float32_t s_sfpACoef2[] = {
	1.0,
	-2.0,
	1.0,
};

DefALLOCATE_DATA_DTCM static float32_t s_sfpBCoef2[] = {
	1.00000000000000000000,
	-1.99004745483398,
	0.99007225036621,
};

DefALLOCATE_BSS_DTCM static float32_t s_sfpLOSamples1[2][2 + 1] = {0};
DefALLOCATE_BSS_DTCM static float32_t s_sfpLISamples1[2][2 + 1] = {0};
DefALLOCATE_BSS_DTCM static float32_t s_sfpLOSamples2[2][2 + 1] = {0};
DefALLOCATE_BSS_DTCM static float32_t s_sfpLISamples2[2][2 + 1] = {0};

DefALLOCATE_ITCM __attribute__((optimize("-O3"))) void LoudnessFilterStereoInit(void)
{
	memset(s_sfpLOSamples1, 0, sizeof(s_sfpLOSamples1));
	memset(s_sfpLISamples1, 0, sizeof(s_sfpLISamples1));
	memset(s_sfpLOSamples2, 0, sizeof(s_sfpLOSamples2));
	memset(s_sfpLISamples2, 0, sizeof(s_sfpLISamples2));
}

DefALLOCATE_ITCM __attribute__((optimize("-O3"))) void LoudnessFilter16bitStereo(const q15_t pi16Src[], q15_t pi16Dst[], uint32_t SampleCount)
{
	float32_t sfpTemp[2];
	for (uint32_t i = 0; i < SampleCount; i++)
	{
		/** Shift Old Samples */
		for (uint32_t j = 2; j > 0; j--)
		{
			s_sfpLISamples1[0][j] = s_sfpLISamples1[0][j - 1];
			s_sfpLOSamples1[0][j] = s_sfpLOSamples1[0][j - 1];
			s_sfpLISamples1[1][j] = s_sfpLISamples1[1][j - 1];
			s_sfpLOSamples1[1][j] = s_sfpLOSamples1[1][j - 1];
		}

		//Calculate the new output
		s_sfpLISamples1[0][0] = ((float32_t)pi16Src[2 * i] / INT16_MAX);
		s_sfpLISamples1[1][0] = ((float32_t)pi16Src[2 * i + 1] / INT16_MAX);

		s_sfpLOSamples1[0][0] = s_sfpACoef1[0] * s_sfpLISamples1[0][0];
		s_sfpLOSamples1[1][0] = s_sfpACoef1[0] * s_sfpLISamples1[1][0];

		for (uint32_t j = 1; j <= 2; j++)
		{
			s_sfpLOSamples1[0][0] += s_sfpACoef1[j] * s_sfpLISamples1[0][j] - s_sfpBCoef1[j] * s_sfpLOSamples1[0][j];
			s_sfpLOSamples1[1][0] += s_sfpACoef1[j] * s_sfpLISamples1[1][j] - s_sfpBCoef1[j] * s_sfpLOSamples1[1][j];
		}

		sfpTemp[0] = s_sfpLOSamples1[0][0];
		sfpTemp[1] = s_sfpLOSamples1[1][0];
		//--

		/** Shift Old Samples */
		for (uint32_t j = 2; j > 0; j--)
		{
			s_sfpLISamples2[0][j] = s_sfpLISamples2[0][j - 1];
			s_sfpLOSamples2[0][j] = s_sfpLOSamples2[0][j - 1];
			s_sfpLISamples2[1][j] = s_sfpLISamples2[1][j - 1];
			s_sfpLOSamples2[1][j] = s_sfpLOSamples2[1][j - 1];
		}

		//Calculate the new output
		s_sfpLISamples2[0][0] = sfpTemp[0];
		s_sfpLISamples2[1][0] = sfpTemp[1];

		s_sfpLOSamples2[0][0] = s_sfpACoef2[0] * s_sfpLISamples2[0][0];
		s_sfpLOSamples2[1][0] = s_sfpACoef2[0] * s_sfpLISamples2[1][0];

		for (uint32_t j = 1; j <= 2; j++)
		{
			s_sfpLOSamples2[0][0] += s_sfpACoef2[j] * s_sfpLISamples2[0][j] - s_sfpBCoef2[j] * s_sfpLOSamples2[0][j];
			s_sfpLOSamples2[1][0] += s_sfpACoef2[j] * s_sfpLISamples2[1][j] - s_sfpBCoef2[j] * s_sfpLOSamples2[1][j];
		}

		pi16Dst[2 * i] = (q15_t)__SSAT((q31_t)(s_sfpLOSamples2[0][0] * INT16_MAX), 16);
		pi16Dst[2 * i + 1] = (q15_t)__SSAT((q31_t)(s_sfpLOSamples2[1][0] * INT16_MAX), 16);
	}

	return;
}

DefALLOCATE_ITCM __attribute__((optimize("-O3"))) void LoudnessFilter24bitStereo(const uint8_t pu8[], q15_t pi16Dst[], uint32_t SampleCount)
{
	float32_t sfpTemp[2];
	uint32_t u32Pos = 0;
	for (uint32_t i = 0; i < SampleCount; i++)
	{
		q15_t q15[2];

		/** Shift Old Samples */
		for (uint32_t j = 2; j > 0; j--)
		{
			s_sfpLISamples1[0][j] = s_sfpLISamples1[0][j - 1];
			s_sfpLOSamples1[0][j] = s_sfpLOSamples1[0][j - 1];
			s_sfpLISamples1[1][j] = s_sfpLISamples1[1][j - 1];
			s_sfpLOSamples1[1][j] = s_sfpLOSamples1[1][j - 1];
		}

		//Calculate the new output
		q15[0] = pu8[u32Pos + 2];
		q15[0] <<= 8;
		q15[0] |= pu8[u32Pos + 1];
		q15[1] = pu8[u32Pos + 5];
		q15[1] <<= 8;
		q15[1] |= pu8[u32Pos + 4];
		u32Pos += 6;

		s_sfpLISamples1[0][0] = ((float32_t)q15[0] / INT16_MAX);
		s_sfpLISamples1[1][0] = ((float32_t)q15[1] / INT16_MAX);

		s_sfpLOSamples1[0][0] = s_sfpACoef1[0] * s_sfpLISamples1[0][0];
		s_sfpLOSamples1[1][0] = s_sfpACoef1[0] * s_sfpLISamples1[1][0];

		for (uint32_t j = 1; j <= 2; j++)
		{
			s_sfpLOSamples1[0][0] += s_sfpACoef1[j] * s_sfpLISamples1[0][j] - s_sfpBCoef1[j] * s_sfpLOSamples1[0][j];
			s_sfpLOSamples1[1][0] += s_sfpACoef1[j] * s_sfpLISamples1[1][j] - s_sfpBCoef1[j] * s_sfpLOSamples1[1][j];
		}
		sfpTemp[0] = s_sfpLOSamples1[0][0];
		sfpTemp[1] = s_sfpLOSamples1[1][0];

		/** Shift Old Samples */
		for (uint32_t j = 2; j > 0; j--)
		{
			s_sfpLISamples2[0][j] = s_sfpLISamples2[0][j - 1];
			s_sfpLOSamples2[0][j] = s_sfpLOSamples2[0][j - 1];
			s_sfpLISamples2[1][j] = s_sfpLISamples2[1][j - 1];
			s_sfpLOSamples2[1][j] = s_sfpLOSamples2[1][j - 1];
		}

		//Calculate the new output
		s_sfpLISamples2[0][0] = sfpTemp[0];
		s_sfpLISamples2[1][0] = sfpTemp[1];

		s_sfpLOSamples2[0][0] = s_sfpACoef2[0] * s_sfpLISamples2[0][0];
		s_sfpLOSamples2[1][0] = s_sfpACoef2[0] * s_sfpLISamples2[1][0];

		for (uint32_t j = 1; j <= 2; j++)
		{
			s_sfpLOSamples2[0][0] += s_sfpACoef2[j] * s_sfpLISamples2[0][j] - s_sfpBCoef2[j] * s_sfpLOSamples2[0][j];
			s_sfpLOSamples2[1][0] += s_sfpACoef2[j] * s_sfpLISamples2[1][j] - s_sfpBCoef2[j] * s_sfpLOSamples2[1][j];
		}
		pi16Dst[2 * i] = (q15_t)__SSAT((q31_t)(s_sfpLOSamples2[0][0] * INT16_MAX), 16);
		pi16Dst[2 * i + 1] = (q15_t)__SSAT((q31_t)(s_sfpLOSamples2[1][0] * INT16_MAX), 16);
	}

	return;
}

DefALLOCATE_ITCM __attribute__((optimize("-O3"))) void LoudnessFilter32bitStereo(const q31_t pi32Src[], q15_t pi16Dst[], uint32_t SampleCount)
{
	double dfpTemp[2];
	for (uint32_t i = 0; i < SampleCount; i++)
	{
		q15_t q15[2];

		/** Shift Old Samples */
		for (uint32_t j = 2; j > 0; j--)
		{
			s_sfpLISamples1[0][j] = s_sfpLISamples1[0][j - 1];
			s_sfpLOSamples1[0][j] = s_sfpLOSamples1[0][j - 1];
			s_sfpLISamples1[1][j] = s_sfpLISamples1[1][j - 1];
			s_sfpLOSamples1[1][j] = s_sfpLOSamples1[1][j - 1];
		}

		//Calculate the new output
		q15[0] = pi32Src[2 * i] >> 16;
		q15[1] = pi32Src[2 * i + 1] >> 16;

		s_sfpLISamples1[0][0] = ((float32_t)q15[0] / INT16_MAX);
		s_sfpLISamples1[1][0] = ((float32_t)q15[1] / INT16_MAX);

		s_sfpLOSamples1[0][0] = s_sfpACoef1[0] * s_sfpLISamples1[0][0];
		s_sfpLOSamples1[1][0] = s_sfpACoef1[0] * s_sfpLISamples1[1][0];

		for (uint32_t j = 1; j <= 2; j++)
		{
			s_sfpLOSamples1[0][0] += s_sfpACoef1[j] * s_sfpLISamples1[0][j] - s_sfpBCoef1[j] * s_sfpLOSamples1[0][j];
			s_sfpLOSamples1[1][0] += s_sfpACoef1[j] * s_sfpLISamples1[1][j] - s_sfpBCoef1[j] * s_sfpLOSamples1[1][j];
		}
		dfpTemp[0] = s_sfpLOSamples1[0][0];
		dfpTemp[1] = s_sfpLOSamples1[1][0];

		/** Shift Old Samples */
		for (uint32_t j = 2; j > 0; j--)
		{
			s_sfpLISamples2[0][j] = s_sfpLISamples2[0][j - 1];
			s_sfpLOSamples2[0][j] = s_sfpLOSamples2[0][j - 1];
			s_sfpLISamples2[1][j] = s_sfpLISamples2[1][j - 1];
			s_sfpLOSamples2[1][j] = s_sfpLOSamples2[1][j - 1];
		}

		//Calculate the new output
		s_sfpLISamples2[0][0] = dfpTemp[0];
		s_sfpLISamples2[1][0] = dfpTemp[1];

		s_sfpLOSamples2[0][0] = s_sfpACoef2[0] * s_sfpLISamples2[0][0];
		s_sfpLOSamples2[1][0] = s_sfpACoef2[0] * s_sfpLISamples2[1][0];

		for (uint32_t j = 1; j <= 2; j++)
		{
			s_sfpLOSamples2[0][0] += s_sfpACoef2[j] * s_sfpLISamples2[0][j] - s_sfpBCoef2[j] * s_sfpLOSamples2[0][j];
			s_sfpLOSamples2[1][0] += s_sfpACoef2[j] * s_sfpLISamples2[1][j] - s_sfpBCoef2[j] * s_sfpLOSamples2[1][j];
		}
		pi16Dst[2 * i] = (q15_t)__SSAT((q31_t)(s_sfpLOSamples2[0][0] * INT16_MAX), 16);
		pi16Dst[2 * i + 1] = (q15_t)__SSAT((q31_t)(s_sfpLOSamples2[1][0] * INT16_MAX), 16);
	}

	return;
}

DefALLOCATE_ITCM __attribute__((optimize("-O3"))) void peekMeterAbsQ15(
	const q15_t *pSrc,
	q15_t *pDst,
	uint32_t blockSize)
{
	uint32_t blkCnt; /* Loop counter */
	q15_t in;		 /* Temporary input variable */

	/* Loop unrolling: Compute 4 outputs at a time */
	blkCnt = blockSize >> 2U;

	while (blkCnt > 0U)
	{
		/* C = |A| */

		/* Calculate absolute of input (if -1 then saturated to 0x7fff) and store result in destination buffer. */
		in = *pSrc++;
#if defined(ARM_MATH_DSP)
		*pDst++ = (in > 0) ? in : (q15_t)__QSUB16(0, in);
#else
		*pDst++ = (in > 0) ? in : ((in == (q15_t)0x8000) ? 0x7fff : -in);
#endif

		in = *pSrc++;
#if defined(ARM_MATH_DSP)
		*pDst++ = (in > 0) ? in : (q15_t)__QSUB16(0, in);
#else
		*pDst++ = (in > 0) ? in : ((in == (q15_t)0x8000) ? 0x7fff : -in);
#endif

		in = *pSrc++;
#if defined(ARM_MATH_DSP)
		*pDst++ = (in > 0) ? in : (q15_t)__QSUB16(0, in);
#else
		*pDst++ = (in > 0) ? in : ((in == (q15_t)0x8000) ? 0x7fff : -in);
#endif

		in = *pSrc++;
#if defined(ARM_MATH_DSP)
		*pDst++ = (in > 0) ? in : (q15_t)__QSUB16(0, in);
#else
		*pDst++ = (in > 0) ? in : ((in == (q15_t)0x8000) ? 0x7fff : -in);
#endif

		/* Decrement loop counter */
		blkCnt--;
	}

	/* Loop unrolling: Compute remaining outputs */
	blkCnt = blockSize % 0x4U;

	while (blkCnt > 0U)
	{
		/* C = |A| */

		/* Calculate absolute of input (if -1 then saturated to 0x7fff) and store result in destination buffer. */
		in = *pSrc++;
#if defined(ARM_MATH_DSP)
		*pDst++ = (in > 0) ? in : (q15_t)__QSUB16(0, in);
#else
		*pDst++ = (in > 0) ? in : ((in == (q15_t)0x8000) ? 0x7fff : -in);
#endif

		/* Decrement loop counter */
		blkCnt--;
	}
}

DefALLOCATE_ITCM __attribute__((optimize("-O3"))) void peekMeterCopyQ15(
	const q15_t *pSrc,
	q15_t *pDst,
	uint32_t blockSize)
{
	uint32_t blkCnt; /* Loop counter */

	/* Loop unrolling: Compute 4 outputs at a time */
	blkCnt = blockSize >> 2U;

	while (blkCnt > 0U)
	{
		/* C = A */

		/* read 2 times 2 samples at a time */
		write_q15x2_ia(&pDst, read_q15x2_ia((q15_t **)&pSrc));
		write_q15x2_ia(&pDst, read_q15x2_ia((q15_t **)&pSrc));

		/* Decrement loop counter */
		blkCnt--;
	}

	/* Loop unrolling: Compute remaining outputs */
	blkCnt = blockSize % 0x4U;

	while (blkCnt > 0U)
	{
		/* C = A */

		/* Copy and store result in destination buffer */
		*pDst++ = *pSrc++;

		/* Decrement loop counter */
		blkCnt--;
	}
}

#define DEF_MAX_SAMPLE_RATE	(96000u)
#define DEF_MAX_CHANNELS	(2u)
#include "Task/MeterTask/MeterTask.h"

DefALLOCATE_BSS_DTCM static q15_t s_aq15AbsSample[(2 * DEF_MAX_SAMPLE_RATE * DEF_MAX_CHANNELS) / DEF_PEEK_METER_REFRESH_RATE];
DefALLOCATE_BSS_DTCM static q15_t s_aq15LoudnessedSample[(DEF_MAX_SAMPLE_RATE * DEF_MAX_CHANNELS) / DEF_PEEK_METER_REFRESH_RATE];

DefALLOCATE_ITCM __attribute__((optimize("-O3"))) void DrvSAIGetPeek16bitStereo(uint32_t u32SampleRate, const int16_t pu16[], uint32_t u32SampleCnt)
{
	uint32_t u32Cnt = 0;
	uint32_t u32ElementCnt = u32SampleCnt * DEF_MAX_CHANNELS;
	uint32_t u32SampleCntPerRR = u32SampleRate / DEF_PEEK_METER_REFRESH_RATE;
	uint32_t u32ElementCntPerRR = u32SampleCntPerRR * DEF_MAX_CHANNELS;
	static uint32_t s_u32Wp = 0;

	LoudnessFilterStereoInit();

	while (u32ElementCnt > 0u)
	{
		uint32_t u32Elem;
		uint32_t u32Rp = u32ElementCntPerRR * u32Cnt;

		/** 残データ数(u32ElementCnt)の算出 */
		if (u32ElementCnt >= u32ElementCntPerRR)
		{
			u32Elem = u32ElementCntPerRR;
			u32ElementCnt -= u32ElementCntPerRR;
		}
		else
		{
			u32Elem = u32ElementCnt;
			u32ElementCnt = 0;
		}

		LoudnessFilter16bitStereo(&pu16[u32Rp], s_aq15LoudnessedSample, u32Elem / DEF_MAX_CHANNELS);
		/** 絶対値の算出 */
		peekMeterAbsQ15(s_aq15LoudnessedSample, &s_aq15AbsSample[s_u32Wp], u32Elem);
		s_u32Wp += u32Elem;

		if (s_u32Wp >= u32ElementCntPerRR)
		{ /* DEF_PEEK_METER_REFRESH_RATE分のデータがたまった */
			q15_t i16RMax = 0;
			q15_t i16LMax = 0;

			/** 絶対値から最大値を抜き出す */
			for (uint32_t i = 0; i < (u32ElementCntPerRR / 2); i++)
			{
				if (s_aq15AbsSample[2 * i] > i16RMax)
				{
					i16RMax = s_aq15AbsSample[2 * i];
				}
				if (s_aq15AbsSample[2 * i + 1] > i16LMax)
				{
					i16LMax = s_aq15AbsSample[2 * i + 1];
				}
			}
			PostMsgMeterTaskPeek16BitStereo(i16RMax, i16LMax);
			{

				if (s_u32Wp > u32ElementCntPerRR)
				{
					uint32_t u32CopyCnt = s_u32Wp - u32ElementCntPerRR;
					peekMeterCopyQ15(&s_aq15AbsSample[u32ElementCntPerRR], s_aq15AbsSample, u32CopyCnt);
					s_u32Wp = u32CopyCnt;
				}
				else
				{
					s_u32Wp = 0;
				}
			}
		}
		u32Cnt++;
	}
}

DefALLOCATE_ITCM __attribute__((optimize("-O3"))) void DrvSAIGetPeek24bitStereo(uint32_t u32SampleRate, const uint8_t pu8[], uint32_t u32SampleCnt)
{
	uint32_t u32Cnt = 0;
	uint32_t u32ElementCnt = u32SampleCnt * DEF_MAX_CHANNELS;
	uint32_t u32SampleCntPerRR = u32SampleRate / DEF_PEEK_METER_REFRESH_RATE;
	uint32_t u32ElementCntPerRR = u32SampleCntPerRR * DEF_MAX_CHANNELS;
	static uint32_t s_u32Wp = 0;

	LoudnessFilterStereoInit();

	while (u32ElementCnt > 0u)
	{
		uint32_t u32Elem;
		uint32_t u32Rp = u32ElementCntPerRR * u32Cnt;

		/** 残データ数(u32ElementCnt)の算出 */
		if (u32ElementCnt >= u32ElementCntPerRR)
		{
			u32Elem = u32ElementCntPerRR;
			u32ElementCnt -= u32ElementCntPerRR;
		}
		else
		{
			u32Elem = u32ElementCnt;
			u32ElementCnt = 0;
		}

		LoudnessFilter24bitStereo(&pu8[u32Rp * 3], s_aq15LoudnessedSample, u32Elem / DEF_MAX_CHANNELS);
		/** 絶対値の算出 */
		peekMeterAbsQ15(s_aq15LoudnessedSample, &s_aq15AbsSample[s_u32Wp], u32Elem);
		s_u32Wp += u32Elem;

		if (s_u32Wp >= u32ElementCntPerRR)
		{ /* DEF_PEEK_METER_REFRESH_RATE分のデータがたまった */
			q15_t i16RMax = 0;
			q15_t i16LMax = 0;

			/** 絶対値から最大値を抜き出す */
			for (uint32_t i = 0; i < (u32ElementCntPerRR / 2); i++)
			{
				if (s_aq15AbsSample[2 * i] > i16RMax)
				{
					i16RMax = s_aq15AbsSample[2 * i];
				}
				if (s_aq15AbsSample[2 * i + 1] > i16LMax)
				{
					i16LMax = s_aq15AbsSample[2 * i + 1];
				}
			}
			PostMsgMeterTaskPeek24BitStereo(i16RMax, i16LMax);

			if (s_u32Wp > u32ElementCntPerRR)
			{
				uint32_t u32CopyCnt = s_u32Wp - u32ElementCntPerRR;
				peekMeterCopyQ15(&s_aq15AbsSample[u32ElementCntPerRR], s_aq15AbsSample, u32CopyCnt);
				s_u32Wp = u32CopyCnt;
			}
			else
			{
				s_u32Wp = 0;
			}
		}
		u32Cnt++;
	}
}
DefALLOCATE_ITCM __attribute__((optimize("-O3"))) void DrvSAIGetPeek32bitStereo(uint32_t u32SampleRate, const int32_t pu32[], uint32_t u32SampleCnt)
{
	uint32_t u32Cnt = 0;
	uint32_t u32ElementCnt = u32SampleCnt * DEF_MAX_CHANNELS;
	uint32_t u32SampleCntPerRR = u32SampleRate / DEF_PEEK_METER_REFRESH_RATE;
	uint32_t u32ElementCntPerRR = u32SampleCntPerRR * DEF_MAX_CHANNELS;
	static uint32_t s_u32Wp = 0;

	LoudnessFilterStereoInit();

	while (u32ElementCnt > 0u)
	{
		uint32_t u32Elem;
		uint32_t u32Rp = u32ElementCntPerRR * u32Cnt;

		/** 残データ数(u32ElementCnt)の算出 */
		if (u32ElementCnt >= u32ElementCntPerRR)
		{
			u32Elem = u32ElementCntPerRR;
			u32ElementCnt -= u32ElementCntPerRR;
		}
		else
		{
			u32Elem = u32ElementCnt;
			u32ElementCnt = 0;
		}

		LoudnessFilter32bitStereo(&pu32[u32Rp], s_aq15LoudnessedSample, u32Elem / DEF_MAX_CHANNELS);
		/** 絶対値の算出 */
		peekMeterAbsQ15(s_aq15LoudnessedSample, &s_aq15AbsSample[s_u32Wp], u32Elem);
		s_u32Wp += u32Elem;

		if (s_u32Wp >= u32ElementCntPerRR)
		{ /* DEF_PEEK_METER_REFRESH_RATE分のデータがたまった */
			q15_t i16RMax = 0;
			q15_t i16LMax = 0;

			/** 絶対値から最大値を抜き出す */
			for (uint32_t i = 0; i < (u32ElementCntPerRR / 2); i++)
			{
				if (s_aq15AbsSample[2 * i] > i16RMax)
				{
					i16RMax = s_aq15AbsSample[2 * i];
				}
				if (s_aq15AbsSample[2 * i + 1] > i16LMax)
				{
					i16LMax = s_aq15AbsSample[2 * i + 1];
				}
			}
			PostMsgMeterTaskPeek32BitStereo(i16RMax, i16LMax);
			{

				if (s_u32Wp > u32ElementCntPerRR)
				{
					uint32_t u32CopyCnt = s_u32Wp - u32ElementCntPerRR;
					peekMeterCopyQ15(&s_aq15AbsSample[u32ElementCntPerRR], s_aq15AbsSample, u32CopyCnt);
					s_u32Wp = u32CopyCnt;
				}
				else
				{
					s_u32Wp = 0;
				}
			}
		}
		u32Cnt++;
	}
}
