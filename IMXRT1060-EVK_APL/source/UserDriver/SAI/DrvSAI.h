/**
 * @brief		SAI Driver wrapper
 * @author		Takashi Kashiwagi
 * @date		2019/06/13
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
 * - 2019/06/13: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */
#ifndef __cplusplus
#if __STDC_VERSION__ < 201112L
#error /** Only C11 */
#endif
#endif
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
/** User Typedefine */
#include "UserTypedef.h"

#include "board.h"
#include "fsl_dmamux.h"
#include "fsl_sai.h"
#include "fsl_sai_edma.h"
#include "arm_math.h"


extern void DrvSAIClockInit(void);
extern _Bool DrvSAIInit(enSAI_t enSAI, sai_sample_rate_t enSampleRate, sai_word_width_t enPcmBit, _Bool bRec);
extern _Bool DrvSAITx(enSAI_t enSAI, const uint8_t pu8[], uint32_t u32ByteCnt);
extern _Bool DrvSAIRx(enSAI_t enSAI, uint8_t pu8[], uint32_t *pu32RxCnt);
extern void DrvSAITxReset(enSAI_t enSAI);
extern void DrvSAIRxReset(enSAI_t enSAI);
extern _Bool DrvSAIIsStop(enSAI_t enSAI);
extern _Bool DrvSAIImmidiateRxStop(enSAI_t enSAI);
extern _Bool DrvSAIImmidiateTxStop(enSAI_t enSAI);
extern uint32_t DrvSAIReadVolume(enSAI_t enSAI);
extern _Bool DrvSAIWriteVolume(enSAI_t enSAI, uint16_t u16Vol);
extern void DrvSAIRxEDMABufferRestart(enSAI_t enSAI);
extern _Bool DrvSAIIsTxBufferEmpty(enSAI_t enSAI);

#ifdef __cplusplus
}
#endif


