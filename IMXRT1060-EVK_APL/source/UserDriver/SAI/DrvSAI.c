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
#error
#endif
#endif

#include "SAI/DrvSAI.h"
#include "OSResource.h"
#include "fsl_wm8960.h"
#include "pin_mux.h"
#include "board.h"
#include "clock_config.h"
#include "mimiclib.h"
#include "SoundTask/SoundTask.h"

#define kOverSampleRate (384U)

/* DMA */
#define kDMA DMA0
#define kDMAMUX DMAMUX

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define kLPI2CClockSourceSelect (0U)
/* Clock divider for master lpi2c clock source */
#define kLPI2CClockSourceDivider (5U)

static void txCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData);
static void rxCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData);

alignas(4) static uint8_t s_TxAudioBuff[enNumOfSAI][DEF_AUDIO_BUFFER_SIZE] __attribute__((section(".bss.$AUDIOBUF")));
alignas(4) static uint8_t s_RxAudioBuff[enNumOfSAI][DEF_AUDIO_BUFFER_SIZE] __attribute__((section(".bss.$AUDIOBUF")));

/**  */
static volatile uint32_t s_u32EDMATxBufSize[enNumOfSAI] = {
	DEF_BUFFER_SAMPLE_SIZE * sizeof(uint32_t), 
	DEF_BUFFER_SAMPLE_SIZE * sizeof(uint32_t),
};
static volatile uint32_t s_u32BeginCount[enNumOfSAI] = {0};
static volatile uint32_t s_u32SendCount[enNumOfSAI] = {0};
static volatile uint32_t s_u32TxEmptyBlock[enNumOfSAI] = {DEF_BUFFER_QUEUE_SIZE, DEF_BUFFER_QUEUE_SIZE};

static volatile uint32_t s_u32EDMARxBufSize[enNumOfSAI] = {
	DEF_BUFFER_SAMPLE_SIZE * sizeof(uint32_t), 
	DEF_BUFFER_SAMPLE_SIZE * sizeof(uint32_t),
};
static _Atomic uint32_t s_u32FullBlock[enNumOfSAI] = {0};
static volatile _Bool s_bIsTxFinished[enNumOfSAI] = {false};
static volatile _Bool s_bIsRxFinished[enNumOfSAI] = {false};

/** Chacheなしエリアへの配置 */
alignas(256) static __attribute__((section(".data.$AUDIOBUF"))) sai_edma_handle_t s_stSAI1Es_hndDmaTx = {0};
alignas(256) static __attribute__((section(".data.$AUDIOBUF"))) sai_edma_handle_t s_stSAI2Es_hndDmaTx = {0};
//alignas(256) static __attribute__((section("NonCacheable.init"))) sai_edma_handle_t s_stSAI3Es_hndDmaTx = {0};
alignas(256) static __attribute__((section(".data.$AUDIOBUF"))) sai_edma_handle_t s_stSAI1Es_hndDmaRx = {0};
alignas(256) static __attribute__((section(".data.$AUDIOBUF"))) sai_edma_handle_t s_stSAI2Es_hndDmaRx = {0};
//alignas(256) static __attribute__((section("NonCacheable.init"))) sai_edma_handle_t s_stSAI3Es_hndDmaRx = {0};

static uint32_t s_u32Index[enNumOfSAI] = {0};
static uint32_t s_u32ReadIndex[enNumOfSAI] = {0};

/**  */
static sai_edma_handle_t *s_phndSaiDmaTx[enNumOfSAI] = {
	&s_stSAI1Es_hndDmaTx,
	&s_stSAI2Es_hndDmaTx,
};
static sai_edma_handle_t *s_phndSaiDmaRx[enNumOfSAI] = {
	&s_stSAI1Es_hndDmaRx,
	&s_stSAI2Es_hndDmaRx,
};

static edma_handle_t s_hndDmaTx[enNumOfSAI] = {0};
static edma_handle_t s_hndDmaRx[enNumOfSAI] = {0};
static const uint32_t kDmaTxChannel[enNumOfSAI] = {12, 14};
static const uint32_t kDmaRxChannel[enNumOfSAI] = {13, 15};
static const dma_request_source_t s_DmaRequestMuxSaiTx[enNumOfSAI] = {
	kDmaRequestMuxSai1Tx,
	kDmaRequestMuxSai2Tx,
};
static const dma_request_source_t s_DmaRequestMuxSaiRx[enNumOfSAI] = {
	kDmaRequestMuxSai1Rx,
	kDmaRequestMuxSai2Rx,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
 * AUDIO PLL setting: Frequency = CrystalFref * (DIV_SELECT + NUM / DENOM)
 *                              = 24000000 * (loopDivider + numerator/denominator)
 *                              = 786432000 Hz
 */
static const clock_audio_pll_config_t audioPllConfig786432000Hz = {
	.loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
	.postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
	.numerator = 96,	/* 30 bit numerator of fractional loop divider. */
	.denominator = 125, /* 30 bit denominator of fractional loop divider */
};
/*
 * AUDIO PLL setting: Frequency = CrystalFref * (DIV_SELECT + NUM / DENOM)
 *                              = 24000000 * (loopDivider + numerator/denominator)
 *                              = 722534400 Hz
 */
static const clock_audio_pll_config_t audioPllConfig722534400Hz = {
	.loopDivider = 30,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
	.postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
	.numerator = 66,	/* 30 bit numerator of fractional loop divider. */
	.denominator = 625, /* 30 bit denominator of fractional loop divider */
};
/* Select Audio/Video PLL (786.48 MHz) as sai clock source */
#define kSAIClockSourceSelect (2U)
/* Clock pre divider for sai clock source */
#define kSAIClockSourcePreDivider (0U)
/* Clock divider for sai clock source */
#define kSAIClockSourceDivider (63U)

/* Get frequency of sai clock */
#define kSAIClockFreq                                                    \
	(CLOCK_GetFreq(kCLOCK_AudioPllClk) / (kSAIClockSourceDivider + 1U) / \
	 (kSAIClockSourcePreDivider + 1U))

static _Bool EnableSaiMclkOutput(enSAI_t enSAI, _Bool enable)
{
	uint32_t mask[] = {
		IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK,
		IOMUXC_GPR_GPR1_SAI2_MCLK_DIR_MASK,
		IOMUXC_GPR_GPR1_SAI3_MCLK_DIR_MASK,
	};

	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}

	if (enable)
	{
		IOMUXC_GPR->GPR1 |= mask[enSAI];
	}
	else
	{
		IOMUXC_GPR->GPR1 &= (~mask[enSAI]);
	}
	return true;
}

static void txCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
{
	enSAI_t enSAI;

	if (base == SAI1)
	{
		enSAI = enSAI1;
	}
	else if (base == SAI2)
	{
		enSAI = enSAI2;
	}
	else
	{
		return;
	}

	s_u32SendCount[enSAI]++;

	if (s_u32SendCount[enSAI] == s_u32BeginCount[enSAI])
	{ /** すべて転送した */
		DrvSAIImmidiateTxStop(enSAI);
		PostMsgSoundTaskStop(enSAI, __func__, __LINE__);
	}
	else
	{
		s_u32TxEmptyBlock[enSAI]++;
		osEventFlagsSet(g_eidSAITx[enSAI], 1);
	}
}

static void rxCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
{
	enSAI_t enSAI;

	if (base == SAI1)
	{
		enSAI = enSAI1;
	}
	else if (base == SAI2)
	{
		enSAI = enSAI2;
	}
	else
	{
		return;
	}

	if (s_u32FullBlock[enSAI] >= DEF_BUFFER_QUEUE_SIZE)
	{
		/** オーバーランした */
		DrvSAIImmidiateRxStop(enSAI);
		PostMsgSoundTaskStop(enSAI, __func__, __LINE__);
	}
	else
	{
		s_u32FullBlock[enSAI]++;
		osEventFlagsSet(g_eidSAIRx[enSAI], 1);
	}
}

static void SAI_UserTxIRQHandler(I2S_Type *base)
{
	/* Clear the FEF flag */
	SAI_TxClearStatusFlags(base, kSAI_FIFOErrorFlag);
	SAI_TxSoftwareReset(base, kSAI_ResetTypeFIFO);
}

static void SAI_UserRxIRQHandler(I2S_Type *base)
{
	SAI_RxClearStatusFlags(base, kSAI_FIFOErrorFlag);
	SAI_RxSoftwareReset(base, kSAI_ResetTypeFIFO);
}

void SAI1_IRQHandler(void)
{
	if (SAI1->TCSR & kSAI_FIFOErrorFlag)
	{
		SAI_UserTxIRQHandler(SAI1);
	}

	if (SAI1->RCSR & kSAI_FIFOErrorFlag)
	{
		SAI_UserRxIRQHandler(SAI1);
	}
}
void SAI2_IRQHandler(void)
{
	if (SAI2->TCSR & kSAI_FIFOErrorFlag)
	{
		SAI_UserTxIRQHandler(SAI2);
	}

	if (SAI2->RCSR & kSAI_FIFOErrorFlag)
	{
		SAI_UserRxIRQHandler(SAI2);
	}
}
void SAI3_TX_IRQHandler(void)
{
	if (SAI3->TCSR & kSAI_FIFOErrorFlag)
	{
		SAI_UserTxIRQHandler(SAI3);
	}
}
void SAI3_RX_IRQHandler(void)
{
	if (SAI3->RCSR & kSAI_FIFOErrorFlag)
	{
		SAI_UserRxIRQHandler(SAI3);
	}
}



void DrvSAIClockInit(void)
{
	CLOCK_DeinitAudioPll();
	CLOCK_InitAudioPll(&audioPllConfig722534400Hz);

	/*Clock setting for LPI2C*/
	//CLOCK_SetMux(kCLOCK_Lpi2cMux, kLPI2CClockSourceSelect);
	//CLOCK_SetDiv(kCLOCK_Lpi2cDiv, kLPI2CClockSourceDivider);
	//CLOCK_EnableClock(kCLOCK_Sai1);
	//CLOCK_EnableClock(kCLOCK_Sai2);
	//CLOCK_EnableClock(kCLOCK_Sai3);
	/*Clock setting for SAI*/
	CLOCK_SetMux(kCLOCK_Sai1Mux, kSAIClockSourceSelect);
	CLOCK_SetDiv(kCLOCK_Sai1PreDiv, kSAIClockSourcePreDivider);
	CLOCK_SetDiv(kCLOCK_Sai1Div, kSAIClockSourceDivider);
	CLOCK_SetMux(kCLOCK_Sai2Mux, kSAIClockSourceSelect);
	CLOCK_SetDiv(kCLOCK_Sai2PreDiv, kSAIClockSourcePreDivider);
	CLOCK_SetDiv(kCLOCK_Sai2Div, kSAIClockSourceDivider);
	CLOCK_SetMux(kCLOCK_Sai3Mux, kSAIClockSourceSelect);
	CLOCK_SetDiv(kCLOCK_Sai3PreDiv, kSAIClockSourcePreDivider);
	CLOCK_SetDiv(kCLOCK_Sai3Div, kSAIClockSourceDivider);

	/*Enable MCLK clock*/
	EnableSaiMclkOutput(enSAI1, true);
}

static sai_word_width_t s_sai_word_width[enNumOfSAI] = {0};
static sai_sample_rate_t s_sai_sample_rate[enNumOfSAI] = {0};

_Bool DrvSAIDMAInit(enSAI_t enSAI)
{
	{ /** Init DMA/DMAMUX */
		edma_config_t dmaConfig = {0};
		EDMA_Deinit(DMA0);
		DMAMUX_Deinit(DMAMUX);
		/* Create EDMA handle */
		/*
		* dmaConfig.enableRoundRobinArbitration = false;
		* dmaConfig.enableHaltOnError = true;
		* dmaConfig.enableContinuousLinkMode = false;
		* dmaConfig.enableDebugMode = false;
		*/
		EDMA_GetDefaultConfig(&dmaConfig);
		EDMA_Init(DMA0, &dmaConfig);
		EDMA_ResetChannel(DMA0, kDmaTxChannel[enSAI]);
		EDMA_ResetChannel(DMA0, kDmaRxChannel[enSAI]);
		DMAMUX_Init(DMAMUX);
	}
	EDMA_CreateHandle(&s_hndDmaTx[enSAI], kDMA, kDmaTxChannel[enSAI]);
	EDMA_CreateHandle(&s_hndDmaRx[enSAI], kDMA, kDmaRxChannel[enSAI]);

	DMAMUX_SetSource(kDMAMUX, kDmaTxChannel[enSAI], (uint8_t)s_DmaRequestMuxSaiTx[enSAI]);
	DMAMUX_EnableChannel(kDMAMUX, kDmaTxChannel[enSAI]);
	DMAMUX_SetSource(kDMAMUX, kDmaRxChannel[enSAI], (uint8_t)s_DmaRequestMuxSaiRx[enSAI]);
	DMAMUX_EnableChannel(kDMAMUX, kDmaRxChannel[enSAI]);

	return true;
}

static codec_handle_t s_HndCodec[enNumOfSAI] = {0};

static codec_config_t boardCodecConfig = {.I2C_SendFunc = BOARD_Codec_I2C_Send,
                                   .I2C_ReceiveFunc = BOARD_Codec_I2C_Receive,
								   .codecConfig = NULL,
                                   .op.Init = WM8960_Init,
                                   .op.Deinit = WM8960_Deinit,
                                   .op.SetFormat = WM8960_ConfigDataFormat};

#include "Task/MeterTask/MeterTask.h"
_Bool DrvSAIInit(enSAI_t enSAI, sai_sample_rate_t enSampleRate, sai_word_width_t enPcmBit, _Bool bRec)
{
	I2S_Type *base[enNumOfSAI] = {SAI1, SAI2};
	const uint32_t TxIrq[enNumOfSAI] = {SAI1_IRQn, SAI2_IRQn};
	const uint32_t RxIrq[enNumOfSAI] = {SAI1_IRQn, SAI2_IRQn};
	const clock_div_t SaiDiv[enNumOfSAI] = {kCLOCK_Sai1Div, kCLOCK_Sai2Div};
	uint32_t SaiDivVal = 0;

	sai_config_t config;

	sai_transfer_format_t format = {0};
	uint32_t u32BclkSrcHz = 0;

	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}
	if ((enSampleRate < kSAI_SampleRate8KHz) || (enSampleRate > kSAI_SampleRate96KHz))
	{
		return false;
	}
	if ((enPcmBit < kSAI_WordWidth8bits) || (enPcmBit > kSAI_WordWidth32bits))
	{
		return false;
	}

	s_sai_word_width[enSAI] = enPcmBit;

	/* buffer size is 64 sample */
	if (s_sai_word_width[enSAI] == kSAI_WordWidth24bits)
	{
		s_u32EDMATxBufSize[enSAI] = DEF_BUFFER_SAMPLE_SIZE * 4;
		s_u32EDMARxBufSize[enSAI] = DEF_BUFFER_SAMPLE_SIZE * 4;
	}
	else
	{
		s_u32EDMATxBufSize[enSAI] = DEF_BUFFER_SAMPLE_SIZE * (enPcmBit / 8);
		s_u32EDMARxBufSize[enSAI] = DEF_BUFFER_SAMPLE_SIZE * (enPcmBit / 8);
	}
	//mimic_printf("[%s (%d)] s_u32EDMATxBufSize[%d] = %lu\r\n", __func__, __LINE__, enSAI, s_u32EDMATxBufSize[enSAI]);
	//mimic_printf("[%s (%d)] s_u32EDMARxBufSize[%d] = %lu\r\n", __func__, __LINE__, enSAI, s_u32EDMARxBufSize[enSAI]);
	
	EnableSaiMclkOutput(enSAI1, false);
	CLOCK_DeinitAudioPll();
	s_sai_sample_rate[enSAI] = enSampleRate;
	switch (enSampleRate)
	{
	case kSAI_SampleRate11025Hz:
	case kSAI_SampleRate22050Hz:
	case kSAI_SampleRate44100Hz:
		CLOCK_InitAudioPll(&audioPllConfig722534400Hz);
		break;
	default:
		CLOCK_InitAudioPll(&audioPllConfig786432000Hz);
		break;
	}
	switch (enSampleRate)
	{
	case kSAI_SampleRate96KHz:
		SaiDivVal = 31;
		break;
	default:
		SaiDivVal = 63;
		break;
	}
	CLOCK_SetDiv(SaiDiv[enSAI], SaiDivVal);
	EnableSaiMclkOutput(enSAI1, true);

	u32BclkSrcHz = kSAIClockFreq;
	mimic_printf("[%s (%d)] Audio PLL = %lu Hz\r\n", __func__, __LINE__, CLOCK_GetFreq(kCLOCK_AudioPllClk));
	mimic_printf("[%s (%d)] SAI CLOCK ROOT = %lu Hz\r\n", __func__, __LINE__, kSAIClockFreq);
	mimic_printf("[%s (%d)] enSampleRate = %ld Hz, enPcmBit = %ld bits\r\n", __func__, __LINE__, enSampleRate, enPcmBit);
	{
		//		static _Bool s_bInited = false;
		//		if(s_bInited == false){
		//			s_bInited = true;
		DrvSAIDMAInit(enSAI);
		//		}
	}
	
	SAI_Deinit(base[enSAI]);
	memset(s_TxAudioBuff[enSAI], 0, sizeof(s_TxAudioBuff[enSAI]));
	memset(s_RxAudioBuff[enSAI], 0, sizeof(s_RxAudioBuff[enSAI]));
	/* Init SAI module */
	/*
     * config.masterSlave = kSAI_Master;
     * config.mclkSource = kSAI_MclkSourceSysclk;
     * config.protocol = kSAI_BusLeftJustified;
     * config.syncMode = kSAI_ModeAsync;
     * config.mclkOutputEnable = true;
     */
	SAI_TxGetDefaultConfig(&config);
	SAI_TxInit(base[enSAI], &config);

	/* Initialize SAI Rx */
	SAI_RxGetDefaultConfig(&config);
	SAI_RxInit(base[enSAI], &config);

	format.bitWidth = enPcmBit;
	format.channel = 0U;
	format.sampleRate_Hz = enSampleRate;
#if (defined FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER && FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) || \
	(defined FSL_FEATURE_PCC_HAS_SAI_DIVIDER && FSL_FEATURE_PCC_HAS_SAI_DIVIDER)
	format.masterClockHz = kOverSampleRate * format.sampleRate_Hz;
#else
	u32BclkSrcHz = (CLOCK_GetFreq(kCLOCK_AudioPllClk) / (SaiDivVal + 1U));
#endif
	//mimic_printf("[%s (%d)] format.masterClockHz = %lu\r\n", __func__, __LINE__, format.masterClockHz);

	format.protocol = config.protocol;
	format.stereo = kSAI_Stereo;

#if defined(FSL_FEATURE_SAI_FIFO_COUNT) && (FSL_FEATURE_SAI_FIFO_COUNT > 1)
	format.watermark = FSL_FEATURE_SAI_FIFO_COUNT / 2U;
#endif

	memset(s_phndSaiDmaTx[enSAI], 0, sizeof(sai_edma_handle_t));
	memset(s_phndSaiDmaRx[enSAI], 0, sizeof(sai_edma_handle_t));

	SAI_TransferTxCreateHandleEDMA(base[enSAI], s_phndSaiDmaTx[enSAI], txCallback, NULL, &s_hndDmaTx[enSAI]);
	SAI_TransferRxCreateHandleEDMA(base[enSAI], s_phndSaiDmaRx[enSAI], rxCallback, NULL, &s_hndDmaRx[enSAI]);

	SAI_TransferTxSetFormatEDMA(base[enSAI], s_phndSaiDmaTx[enSAI], &format, kSAIClockFreq, u32BclkSrcHz);
	SAI_TransferRxSetFormatEDMA(base[enSAI], s_phndSaiDmaRx[enSAI], &format, kSAIClockFreq, u32BclkSrcHz);

#if 1
	{
		edma_channel_Preemption_config_t preempCondif = {
			.enableChannelPreemption = false,
			.enablePreemptAbility = true,
			.channelPriority = kDmaTxChannel[enSAI], /** 0 - 15 */
		};
		EDMA_SetChannelPreemptionConfig(DMA0, kDmaTxChannel[enSAI], &preempCondif);
		preempCondif.channelPriority = kDmaRxChannel[enSAI];
		EDMA_SetChannelPreemptionConfig(DMA0, kDmaRxChannel[enSAI], &preempCondif);
	}
#endif
	/* Enable interrupt to handle FIFO error */
	SAI_TxEnableInterrupts(base[enSAI], kSAI_FIFOErrorInterruptEnable);
	SAI_RxEnableInterrupts(base[enSAI], kSAI_FIFOErrorInterruptEnable);
	NVIC_SetPriority(TxIrq[enSAI], kIRQ_PRIORITY_SAI);
	EnableIRQ(TxIrq[enSAI]);
	NVIC_SetPriority(RxIrq[enSAI], kIRQ_PRIORITY_SAI);
	EnableIRQ(RxIrq[enSAI]);

	SAI_TransferTerminateSendEDMA(base[enSAI], s_phndSaiDmaTx[enSAI]);
	SAI_TransferTerminateReceiveEDMA(base[enSAI], s_phndSaiDmaRx[enSAI]);
	SAI_TxSoftwareReset(base[enSAI], kSAI_ResetAll);
	SAI_RxSoftwareReset(base[enSAI], kSAI_ResetAll);
	SAI_TxClearStatusFlags(base[enSAI], kSAI_WordStartFlag | kSAI_SyncErrorFlag | kSAI_FIFOErrorFlag | kSAI_FIFORequestFlag | kSAI_FIFOWarningFlag);
	SAI_RxClearStatusFlags(base[enSAI], kSAI_WordStartFlag | kSAI_SyncErrorFlag | kSAI_FIFOErrorFlag | kSAI_FIFORequestFlag | kSAI_FIFOWarningFlag);

	DrvSAITxReset(enSAI);
	DrvSAIRxReset(enSAI);

	/* Configure the audio format */
	
	if(kStatus_Success != CODEC_Init(&s_HndCodec[enSAI], &boardCodecConfig))
	{
		mimic_printf("[%s (%d)] CODEC_Init NG \r\n", __func__, __LINE__);
		return false;
	}
	if (kStatus_Success != CODEC_SetFormat(&s_HndCodec[enSAI], u32BclkSrcHz, enSampleRate, enPcmBit))
	{
		mimic_printf("[%s (%d)] CODEC_SetFormat NG\r\n", __func__, __LINE__);
		return false;
	}

	WM8960_SetProtocol(&s_HndCodec[enSAI], kWM8960_BusI2S);
    WM8960_SetMasterSlave(&s_HndCodec[enSAI], false);
	WM8960_SetDataRoute(&s_HndCodec[enSAI], kWM8960_RoutePlaybackandRecord);
	WM8960_SetLeftInput(&s_HndCodec[enSAI], kWM8960_InputDifferentialMicInput3);
	WM8960_SetRightInput(&s_HndCodec[enSAI], kWM8960_InputDifferentialMicInput2);
	WM8960_SetModule(&s_HndCodec[enSAI], kWM8960_ModuleHP, true);
	//WM8960_RegDump(&s_HndCodec[enSAI]);
	MeterTaskRestart();
	mimic_printf("[%s (%d)] EXIT OK\r\n", __func__, __LINE__);
	return true;
}

void DrvSAITxReset(enSAI_t enSAI)
{

	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return;
	}

	/* Reset SAI Tx internal logic */
	//
	osEventFlagsClear(g_eidSAITx[enSAI], 0x03);

	s_u32BeginCount[enSAI] = 0;
	s_u32SendCount[enSAI] = 0;
	s_u32Index[enSAI] = 0;
	s_u32TxEmptyBlock[enSAI] = DEF_BUFFER_QUEUE_SIZE;
}
void DrvSAIRxReset(enSAI_t enSAI)
{
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return;
	}

	/* Reset SAI Rx internal logic */
	//
	osEventFlagsClear(g_eidSAIRx[enSAI], 0x03);

	s_u32FullBlock[enSAI] = 0;
	s_u32ReadIndex[enSAI] = 0;
}
void DrvSAIRxEDMABufferRestart(enSAI_t enSAI)
{
	I2S_Type *base[enNumOfSAI] = {SAI1, SAI2};
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return;
	}
	//mimic_printf("[%s (%d)] ENTER\r\n", __func__, __LINE__);

	SAI_TransferTerminateReceiveEDMA(base[enSAI], s_phndSaiDmaRx[enSAI]);
	//mimic_printf("[%s (%d)] s_phndSaiDmaRx[enSAI]->state = %d\r\n", __func__, __LINE__, s_phndSaiDmaRx[enSAI]->state);
	s_u32ReadIndex[enSAI] = 0;
	s_u32FullBlock[enSAI] = 0;
	for (uint32_t i = 0; i < DEF_BUFFER_QUEUE_SIZE; i++)
	{
		sai_transfer_t stxfer = {0};
		stxfer.dataSize = s_u32EDMARxBufSize[enSAI];
		stxfer.data = &s_RxAudioBuff[enSAI][i * s_u32EDMARxBufSize[enSAI]];
		if (kStatus_Success != SAI_TransferReceiveEDMA(base[enSAI], s_phndSaiDmaRx[enSAI], &stxfer))
		{
			mimic_printf("[%s (%d)] TP\r\n", __func__, __LINE__);
		}
	}
	//mimic_printf("[%s (%d)] s_phndSaiDmaRx[enSAI]->state = %d\r\n", __func__, __LINE__, s_phndSaiDmaRx[enSAI]->state);
	//mimic_printf("[%s (%d)] EXIT\r\n", __func__, __LINE__);
}
_Bool DrvSAIIsTxBufferEmpty(enSAI_t enSAI)
{
	_Bool bTxBufferEmpty = false;
	I2S_Type *base[enNumOfSAI] = {SAI1, SAI2};
	size_t u32Cnt;

	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}

	if (kStatus_Success == SAI_TransferGetSendCountEDMA(base[enSAI], s_phndSaiDmaTx[enSAI], &u32Cnt))
	{
		if (u32Cnt == 0)
		{
			bTxBufferEmpty = true;
		}
	}

	return bTxBufferEmpty;
}
_Bool DrvSAIIsStop(enSAI_t enSAI)
{
	return s_bIsTxFinished[enSAI];
}

#include "PeekMeter.h"
_Bool DrvSAITx(enSAI_t enSAI, const uint8_t pu8[], uint32_t u32ByteCnt)
{
	sai_transfer_t stxfer = {0};
	uint32_t u32TxCnt = 0;
	uint32_t u32RemainSize = u32ByteCnt;
	I2S_Type *base[enNumOfSAI] = {SAI1, SAI2};
				
	while (u32RemainSize != 0)
	{
		/* Transfer data already prepared, so while there is any empty slot, just transfer */
		if (s_u32TxEmptyBlock[enSAI] > 0)
		{
			
			uint32_t index = s_u32Index[enSAI] * s_u32EDMATxBufSize[enSAI];

			stxfer.data = &s_TxAudioBuff[enSAI][index];


			/** 24bit PCMは細工が必要 */
			if (s_sai_word_width[enSAI] == kSAI_WordWidth24bits)
			{
				uint32_t u32 = s_u32EDMATxBufSize[enSAI];
				u32 /= 4;
				u32 *= 3;
				if (u32RemainSize <= u32)
				{
					stxfer.dataSize = (u32RemainSize * 4) / 3;
					u32RemainSize = 0;
				}
				else
				{
					stxfer.dataSize = s_u32EDMATxBufSize[enSAI];
					u32RemainSize -= u32;
				}
			}
			else
			{
				if (u32RemainSize <= s_u32EDMATxBufSize[enSAI])
				{
					stxfer.dataSize = u32RemainSize;
					u32RemainSize = 0;
				}
				else
				{
					stxfer.dataSize = s_u32EDMATxBufSize[enSAI];
					u32RemainSize -= s_u32EDMATxBufSize[enSAI];
				}
			}

			uint32_t u32ByteCnt = stxfer.dataSize;
			if ((u32ByteCnt % 32) != 0)
			{
				//				memset(&pu8[u32ByteCnt], 0, 32 - u32ByteCnt % 32);
				u32ByteCnt += (32 - (u32ByteCnt % 32));
				stxfer.dataSize = u32ByteCnt;
			}
			
			/** 24bit PCMは細工が必要 */
			if (s_sai_word_width[enSAI] == kSAI_WordWidth16bits)
			{
				PeekMeterGetPeek16bitStereo(s_sai_sample_rate[enSAI], (int16_t*)&pu8[u32TxCnt], u32ByteCnt / 4);
				memcpy(&s_TxAudioBuff[enSAI][index], &pu8[u32TxCnt], u32ByteCnt);
				u32TxCnt += u32ByteCnt;
			}
			else if (s_sai_word_width[enSAI] == kSAI_WordWidth24bits)
			{
				PeekMeterGetPeek24bitStereo(s_sai_sample_rate[enSAI], &pu8[u32TxCnt], u32ByteCnt / 8);	/** u32ByteCntには要注意 */
				for (uint32_t i = 0; i < u32ByteCnt; i += 4)
				{
					s_TxAudioBuff[enSAI][index + i] = pu8[u32TxCnt];
					s_TxAudioBuff[enSAI][index + i + 1] = pu8[u32TxCnt + 1];
					s_TxAudioBuff[enSAI][index + i + 2] = pu8[u32TxCnt + 2];
					s_TxAudioBuff[enSAI][index + i + 3] = 0;
					u32TxCnt += 3;
				}
			}
			else
			{
				PeekMeterGetPeek32bitStereo(s_sai_sample_rate[enSAI], (int32_t*)&pu8[u32TxCnt], u32ByteCnt / 8);
				memcpy(&s_TxAudioBuff[enSAI][index], &pu8[u32TxCnt], u32ByteCnt);
				u32TxCnt += u32ByteCnt;
			}
			__DMB();
			s_bIsTxFinished[enSAI] = false;

			status_t sts = SAI_TransferSendEDMA(base[enSAI], s_phndSaiDmaTx[enSAI], &stxfer);
			/* Shall make sure the sai buffer queue is not full */
			if (sts == kStatus_Success)
			{
				s_u32Index[enSAI]++;
				s_u32Index[enSAI] %= DEF_BUFFER_QUEUE_SIZE;
				s_u32BeginCount[enSAI]++;

				/**  */
				s_u32TxEmptyBlock[enSAI]--;
			}
			else
			{
				mimic_printf("[%s (%d)] SAI_TransferSend NG (sts = %ld) (0x%08lX, %lu, %lu)\r\n", __func__, __LINE__, sts, stxfer.data, stxfer.dataSize, u32RemainSize);
				return false;
			}
			{
				uint32_t u32EDMAError = EDMA_GetErrorStatusFlags(DMA0);
				if ((kEDMA_ValidFlag & u32EDMAError) != 0)
				{
					mimic_printf("[%s (%d)] u32EDMAError = 0x%08lX\r\n", __func__, __LINE__, u32EDMAError);
				}
			}
		}
		else
		{
			/** Event Wait */
			uint32_t uxBits;
			uxBits = osEventFlagsWait(g_eidSAITx[enSAI], 3, osFlagsWaitAny, 500);
			if (uxBits != 1)
			{
				mimic_printf("[%s (%d)] uxBits = 0x%08lX\r\n", __func__, __LINE__, uxBits);
				if ((uxBits & 0x80000000u) != 0)
				{
					mimic_printf("[%s (%d)] osEventFlagsWait NG(u32RemainSize = %lu)\r\n", __func__, __LINE__, u32RemainSize);
				}
				else
				{
					/** EDMAが停止 */
				}
				#if 0
				mimic_printf("[%s (%d)] EDMA_GetErrorStatusFlags   = 0x%08lX\r\n", __func__, __LINE__, EDMA_GetChannelStatusFlags(DMA0, kDmaTxChannel[enSAI]));
				mimic_printf("[%s (%d)] EDMA_GetChannelStatusFlags = 0x%08lX\r\n", __func__, __LINE__, EDMA_GetErrorStatusFlags(DMA0));
				mimic_printf("[%s (%d)] TCD[%lu] CSR = 0x%08lX\r\n", __func__, __LINE__, kDmaTxChannel[enSAI], s_phndSaiDmaTx[enSAI]->dmaHandle->base->TCD[kDmaTxChannel[enSAI]].CSR);
				mimic_printf("[%s (%d)] TCD[%lu] SADDR = 0x%08lX\r\n", __func__, __LINE__, kDmaTxChannel[enSAI], s_phndSaiDmaTx[enSAI]->dmaHandle->base->TCD[kDmaTxChannel[enSAI]].SADDR);
				mimic_printf("[%s (%d)] TCD[%lu] DADDR = 0x%08lX\r\n", __func__, __LINE__, kDmaTxChannel[enSAI], s_phndSaiDmaTx[enSAI]->dmaHandle->base->TCD[kDmaTxChannel[enSAI]].DADDR);
				mimic_printf("[%s (%d)] EARS = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaTx[enSAI]->dmaHandle->base->EARS);
				mimic_printf("[%s (%d)] INT = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaTx[enSAI]->dmaHandle->base->INT);
				mimic_printf("[%s (%d)] CR  = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaTx[enSAI]->dmaHandle->base->CR);
				mimic_printf("[%s (%d)] ES = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaTx[enSAI]->dmaHandle->base->ES);
				mimic_printf("[%s (%d)] EEI = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaTx[enSAI]->dmaHandle->base->EEI);
				mimic_printf("[%s (%d)] CEEI = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaTx[enSAI]->dmaHandle->base->CEEI);
				mimic_printf("[%s (%d)] CERQ = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaTx[enSAI]->dmaHandle->base->CERQ);
				mimic_printf("[%s (%d)] ERQ = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaTx[enSAI]->dmaHandle->base->ERQ);
				mimic_printf("[%s (%d)] ERR = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaTx[enSAI]->dmaHandle->base->ERR);
				mimic_printf("[%s (%d)] HRS = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaTx[enSAI]->dmaHandle->base->HRS);
				mimic_printf("[%s (%d)] SADDR = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaTx[enSAI]->dmaHandle->base->TCD[kDmaTxChannel[enSAI]].SADDR);
				mimic_printf("[%s (%d)] DADDR = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaTx[enSAI]->dmaHandle->base->TCD[kDmaTxChannel[enSAI]].DADDR);
				#endif
				return false;
			}
			else
			{
				/* NOP */
			}
		}
	}

	//mimic_printf("[%s (%d)] TxDone : %lu bytes\r\n", __func__, __LINE__, u32ByteCnt);
	return true;
}

/** 受信バッファサイズ固定 */
_Bool DrvSAIRx(enSAI_t enSAI, uint8_t pu8[], uint32_t *pu32RxCnt)
{

	I2S_Type *base[enNumOfSAI] = {SAI1, SAI2};

	for (;;)
	{
		uint32_t u32EDMAError = EDMA_GetErrorStatusFlags(DMA0);
		if ((kEDMA_ValidFlag & u32EDMAError) != 0)
		{
			mimic_printf("[%s (%d)] EDMA_GetErrorStatusFlags = 0x%08lX\r\n", __func__, __LINE__, u32EDMAError);
		}

		if (s_u32FullBlock[enSAI] > 0)
		{
			s_u32FullBlock[enSAI]--;

			/** 受信したブロックがある */
			uint32_t index = s_u32ReadIndex[enSAI] * s_u32EDMARxBufSize[enSAI];
			//mimic_printf("index = %lu\r\n", s_u32ReadIndex[enSAI]);

			if (s_sai_word_width[enSAI] == kSAI_WordWidth16bits)
			{
				memcpy(pu8, &s_RxAudioBuff[enSAI][index], s_u32EDMARxBufSize[enSAI]);
				*pu32RxCnt = s_u32EDMARxBufSize[enSAI];
				PeekMeterGetPeek16bitStereo(s_sai_sample_rate[enSAI], (int16_t*)pu8, s_u32EDMARxBufSize[enSAI] / 4);
			}
			else if (s_sai_word_width[enSAI] == kSAI_WordWidth24bits)
			{
				/** 24bit PCMは細工が必要 */
				uint32_t u32RxCnt = 0;
				for (uint32_t i = 0; i < s_u32EDMARxBufSize[enSAI]; i += 4)
				{
					pu8[u32RxCnt] = s_RxAudioBuff[enSAI][index + i];
					pu8[u32RxCnt + 1] = s_RxAudioBuff[enSAI][index + i + 1];
					pu8[u32RxCnt + 2] = s_RxAudioBuff[enSAI][index + i + 2];
					u32RxCnt += 3;
				}
				*pu32RxCnt = u32RxCnt;
				PeekMeterGetPeek24bitStereo(s_sai_sample_rate[enSAI], pu8, s_u32EDMARxBufSize[enSAI] / 8);
			}
			else
			{
				memcpy(pu8, &s_RxAudioBuff[enSAI][index], s_u32EDMARxBufSize[enSAI]);
				*pu32RxCnt = s_u32EDMARxBufSize[enSAI];
				PeekMeterGetPeek32bitStereo(s_sai_sample_rate[enSAI], (int32_t*)pu8, s_u32EDMARxBufSize[enSAI] / 8);
			}
			__DMB();
			{
				sai_transfer_t stxfer = {0};
				status_t sts;
				stxfer.dataSize = s_u32EDMARxBufSize[enSAI];
				stxfer.data = &s_RxAudioBuff[enSAI][s_u32ReadIndex[enSAI] * s_u32EDMARxBufSize[enSAI]];
				sts = SAI_TransferReceiveEDMA(base[enSAI], s_phndSaiDmaRx[enSAI], &stxfer);
				if (kStatus_Success != sts)
				{
					mimic_printf("[%s (%d)] SAI_TransferReceiveEDMA NG(%d)\r\n", __func__, __LINE__, sts);
					return false;
				}
			}
			s_u32ReadIndex[enSAI]++;
			s_u32ReadIndex[enSAI] %= DEF_BUFFER_QUEUE_SIZE;
			break;
		}
		else
		{
			uint32_t uxBits;
			uxBits = osEventFlagsWait(g_eidSAIRx[enSAI], 3, osFlagsWaitAny, 500);
			if (uxBits != 1)
			{
				if ((uxBits & 0x80000000u) != 0)
				{
					size_t u32ReadCnt = 0;
					mimic_printf("[%s (%d)] s_phndSaiDmaRx[enSAI]->state = %d\r\n", __func__, __LINE__, s_phndSaiDmaRx[enSAI]->state);
					if (kStatus_Success == SAI_TransferGetReceiveCountEDMA(base[enSAI], s_phndSaiDmaRx[enSAI], &u32ReadCnt))
					{
						mimic_printf("[%s (%d)] u32ReadCnt = %d\r\n", __func__, __LINE__, u32ReadCnt);
					}
					*pu32RxCnt = u32ReadCnt;
					mimic_printf("[%s (%d)] osEventFlagsWait NG\r\n", __func__, __LINE__);
					mimic_printf("[%s (%d)] EDMA_GetErrorStatusFlags   = 0x%08lX\r\n", __func__, __LINE__, EDMA_GetChannelStatusFlags(DMA0, kDmaRxChannel[enSAI]));
					mimic_printf("[%s (%d)] EDMA_GetChannelStatusFlags = 0x%08lX\r\n", __func__, __LINE__, EDMA_GetErrorStatusFlags(DMA0));
					mimic_printf("[%s (%d)] TCD[%lu] CSR = 0x%08lX\r\n", __func__, __LINE__, kDmaRxChannel[enSAI], s_phndSaiDmaRx[enSAI]->dmaHandle->base->TCD[kDmaRxChannel[enSAI]].CSR);
					mimic_printf("[%s (%d)] EEI = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaRx[enSAI]->dmaHandle->base->EEI);
					mimic_printf("[%s (%d)] INT = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaRx[enSAI]->dmaHandle->base->INT);
					mimic_printf("[%s (%d)] CR  = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaRx[enSAI]->dmaHandle->base->CR);
					mimic_printf("[%s (%d)] ERQ = 0x%08lX\r\n", __func__, __LINE__, s_phndSaiDmaRx[enSAI]->dmaHandle->base->ERQ);
				}
				else
				{
					/** EDMAが停止 */
					mimic_printf("[%s (%d)] Immidiate Stop !!\r\n", __func__, __LINE__);
					uint32_t index = s_u32ReadIndex[enSAI] * s_u32EDMARxBufSize[enSAI];
					size_t u32ReadCnt;
					s_u32ReadIndex[enSAI] = 0;

					if (kStatus_Success != SAI_TransferGetReceiveCountEDMA(base[enSAI], s_phndSaiDmaRx[enSAI], &u32ReadCnt))
					{
						u32ReadCnt = 0;
					}
					if (s_sai_word_width[enSAI] == kSAI_WordWidth24bits)
					{
						/** 24bit PCMは細工が必要 */
						uint32_t u32RxCnt = 0;
						for (uint32_t i = 0; i < u32ReadCnt; i += 4)
						{
							pu8[u32RxCnt] = s_RxAudioBuff[enSAI][index + i];
							u32RxCnt++;
							pu8[u32RxCnt] = s_RxAudioBuff[enSAI][index + i + 1];
							u32RxCnt++;
							pu8[u32RxCnt] = s_RxAudioBuff[enSAI][index + i + 2];
							u32RxCnt++;
						}
						*pu32RxCnt = u32RxCnt;
					}
					else
					{
						memcpy(pu8, &s_RxAudioBuff[enSAI][index], u32ReadCnt);
						*pu32RxCnt = u32ReadCnt;
					}
				}
				return false;
			}
		}
	}

	return true;
}

_Bool DrvSAIImmidiateTxStop(enSAI_t enSAI)
{
	I2S_Type *base[enNumOfSAI] = {SAI1, SAI2};

	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}
	s_bIsTxFinished[enSAI] = true;

	SAI_TransferTerminateSendEDMA(base[enSAI], s_phndSaiDmaTx[enSAI]);
	osEventFlagsSet(g_eidSAITx[enSAI], 0x02);
	return true;
}
_Bool DrvSAIImmidiateRxStop(enSAI_t enSAI)
{
	I2S_Type *base[enNumOfSAI] = {SAI1, SAI2};

	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}
	s_bIsRxFinished[enSAI] = true;

	SAI_TransferTerminateReceiveEDMA(base[enSAI], s_phndSaiDmaRx[enSAI]);
	osEventFlagsSet(g_eidSAIRx[enSAI], 0x02);

	return true;
}

uint32_t DrvSAIReadVolume(enSAI_t enSAI)
{
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return 0;
	}
	return WM8960_GetVolume(&s_HndCodec[enSAI], kWM8960_ModuleHP);
}
_Bool DrvSAIWriteVolume(enSAI_t enSAI, uint16_t u16Vol)
{
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}
	if (WM8960_SetVolume(&s_HndCodec[enSAI], kWM8960_ModuleHP, u16Vol) == kStatus_Success)
	{
		if (u16Vol != WM8960_GetVolume(&s_HndCodec[enSAI], kWM8960_ModuleHP))
		{
			mimic_printf("[%s (%d)] Verify NG (u16Vol = %lu) \r\n", __func__, __LINE__, u16Vol);
			return false;
		}

		return true;
	}
	else
	{
		mimic_printf("[%s (%d)] WM8960_SetVolume NG\r\n", __func__, __LINE__);
		return false;
	}
}
