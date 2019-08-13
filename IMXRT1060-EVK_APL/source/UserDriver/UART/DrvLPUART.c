/**
 * @file DrvLPUART.c
 * @brief TODO
 * @author Takashi Kashiwagi
 * @date 2018/10/28
 * @version     0.1
 * @details 
 * --
 * License Type <MIT License>
 * --
 * Copyright 2018 Takashi Kashiwagi
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
 * - 2018/10/28: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */
/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "UART/DrvLPUART.h"
#include "board.h"


#if defined(FSL_FEATURE_LPUART_HAS_FIFO) && FSL_FEATURE_LPUART_HAS_FIFO
#else
#error
#endif

static LPUART_Type *s_LPUARTBaseTable[] = LPUART_BASE_PTRS;
static void LPUARTXHandleIRQ(enLPUART_t enLPUARTNo);


/**
 * @brief LPUART1 IRQ Handler
 * @return void
 */
void LPUART1_IRQHandler(void){
	LPUARTXHandleIRQ(enLPUART1);
}
/**
 * @brief LPUART2 IRQ Handler
 * @return void
 */
void LPUART2_IRQHandler(void){
	LPUARTXHandleIRQ(enLPUART2);
}
/**
 * @brief LPUART3 IRQ Handler
 * @return void
 */
void LPUART3_IRQHandler(void){
	LPUARTXHandleIRQ(enLPUART3);
}
/**
 * @brief LPUART4 IRQ Handler
 * @return void
 */
void LPUART4_IRQHandler(void){
	LPUARTXHandleIRQ(enLPUART4);
}
/**
 * @brief LPUART5 IRQ Handler
 * @return void
 */
void LPUART5_IRQHandler(void){
	LPUARTXHandleIRQ(enLPUART5);
}
/**
 * @brief LPUART6 IRQ Handler
 * @return void
 */
void LPUART6_IRQHandler(void){
	LPUARTXHandleIRQ(enLPUART6);
}
/**
 * @brief LPUART7 IRQ Handler
 * @return void
 */
void LPUART7_IRQHandler(void){
	LPUARTXHandleIRQ(enLPUART7);
}
/**
 * @brief LPUART8 IRQ Handler
 * @return void
 */
void LPUART8_IRQHandler(void){
	LPUARTXHandleIRQ(enLPUART8);
}


static _Bool GetTxData(enLPUART_t enLPUARTNo, uint8_t *pu8val){

	stTaskMsgBlock_t stTaskMsg;
	if((enLPUARTNo < enLPUART_MIN) || (enLPUARTNo > enLPUART_MIN)){
		return false;
	}

	memset(&stTaskMsg, 0, sizeof(stTaskMsgBlock_t));
	
	if(pdFALSE != xPortIsInsideInterrupt()){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		if(1u != xStreamBufferReceiveFromISR(g_sbhLPUARTTx[enLPUARTNo], pu8val, 1, &xHigherPriorityTaskWoken)){
			return false;
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}else{
		if(1u != xStreamBufferReceive(g_sbhLPUARTTx[enLPUARTNo], pu8val, 1, 10)){
			return false;
		}
	}
	return true;
}
static void PutRxData(enLPUART_t enLPUARTNo, uint8_t u8val){

	stTaskMsgBlock_t stTaskMsg;
	if((enLPUARTNo < enLPUART_MIN) || (enLPUARTNo > enLPUART_MIN)){
		return;
	}

	memset(&stTaskMsg, 0, sizeof(stTaskMsgBlock_t));
	
	if(pdFALSE != xPortIsInsideInterrupt()){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		xStreamBufferSendFromISR(g_sbhLPUARTRx[enLPUARTNo], &u8val, 1, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}else{
		xStreamBufferSend(g_sbhLPUARTRx[enLPUARTNo], &u8val, 1, 10);
	}
}


/**
 * @brief Actual Interrupt Handler
 * @param [in]  enLPUARTNo PortNumber
 * @return void
 */
static void LPUARTXHandleIRQ(enLPUART_t enLPUARTNo)
{
    uint8_t count;

	LPUART_Type *base;

	if((enLPUARTNo < enLPUART_MIN) || (enLPUARTNo > enLPUART_MIN)){
		return;
	}
	
	base = s_LPUARTBaseTable[enLPUARTNo];

    /* If RX overrun. */
    if (LPUART_STAT_OR_MASK & base->STAT)
    {
        /* Clear overrun flag, otherwise the RX does not work. */
        base->STAT = ((base->STAT & 0x3FE00000U) | LPUART_STAT_OR_MASK);

		/** TODO : オーバー欄チェックは必要なら入れる */
    }

    /* If IDLE flag is set and the IDLE interrupt is enabled. */
    if ((LPUART_STAT_IDLE_MASK & base->STAT) && (LPUART_CTRL_ILIE_MASK & base->CTRL))
    {
        count = ((uint8_t)((base->WATER & LPUART_WATER_RXCOUNT_MASK) >> LPUART_WATER_RXCOUNT_SHIFT));

        
		/* Using non block API to read the data from the registers. */
		for(uint32_t i=0u;i<(uint32_t)count;i++){
			/** TODO : オーバー欄チェックは必要なら入れる */
			PutRxData(enLPUARTNo, base->DATA);
		}

		


        /* Clear IDLE flag.*/
        base->STAT |= LPUART_STAT_IDLE_MASK;

        /* Disable interrupt.*/
		//LPUART_DisableInterrupts(base, kLPUART_IdleLineInterruptEnable);
    }
    /* Receive data register full */
    if ((LPUART_STAT_RDRF_MASK & base->STAT) && (LPUART_CTRL_RIE_MASK & base->CTRL))
    {
		/* Get the size that can be stored into buffer for this interrupt. */
        count = ((uint8_t)((base->WATER & LPUART_WATER_RXCOUNT_MASK) >> LPUART_WATER_RXCOUNT_SHIFT));

        /* If handle->rxDataSize is not 0, first save data to handle->rxData. */
        for(uint32_t i=0u;i<(uint32_t)count;i++){
			/** TODO : オーバー欄チェックは必要なら入れる */
			PutRxData(enLPUARTNo, base->DATA);
		}

		//LPUART_DisableInterrupts(base, kLPUART_IdleLineInterruptEnable);
    }

    /* Send data register empty and the interrupt is enabled. */
    if ((base->STAT & LPUART_STAT_TDRE_MASK) && (base->CTRL & LPUART_CTRL_TIE_MASK))
    {
		/* Get the bytes that available at this moment. */
        count = FSL_FEATURE_LPUART_FIFO_SIZEn(base) -
                ((base->WATER & LPUART_WATER_TXCOUNT_MASK) >> LPUART_WATER_TXCOUNT_SHIFT);

        while(count > 0u)
        {
			uint8_t u8Data;
			if(GetTxData(enLPUARTNo, &u8Data) != false){
				base->DATA = u8Data;
			}else{
				/* Disable TX register empty interrupt. */
                base->CTRL = (base->CTRL & ~LPUART_CTRL_TIE_MASK);
				break;
			}
			count--;
        }
    }

	return;
}

/**
 * @brief Initialize LPUART
 * Ex:
 * lpuart_config_t lpuart_config;
 * LPUART_GetDefaultConfig(&lpuart_config);
 * lpuart_config.baudRate_Bps = baudRate;
 * DrvLPUART_Init(enLPUART1, &lpuart_config);
 * @param [in]  enLPUARTNo PortNumber
 * @param [in]  config UART parameters
 * @return true Success
 * @return false NG
 */
_Bool DrvLPUARTInit(enLPUART_t enLPUARTNo, const lpuart_config_t *config)
{
	_Bool bret = false;
	uint32_t uartClkSrcFreq = BOARD_DebugConsoleSrcFreq();


	if((enLPUARTNo < enLPUART_MIN) || (enLPUARTNo > enLPUART_MIN)){
		bret = false;
	}else{
		LPUART_Type *base = s_LPUARTBaseTable[enLPUARTNo];

		if (LPUART_Init(base, config, uartClkSrcFreq) == kStatus_Success){
			IRQn_Type enIRQn= LPUART1_IRQn;
			enIRQn += (IRQn_Type)(enLPUARTNo - enLPUART_MIN);

			LPUART_EnableTx(base, true);
			LPUART_EnableRx(base, true);
			/* Enable Interrupts (LPUART) */
			LPUART_EnableInterrupts(base, kLPUART_RxDataRegFullInterruptEnable | 
										  kLPUART_IdleLineInterruptEnable |
										  kLPUART_RxOverrunInterruptEnable | 
										  kLPUART_NoiseErrorInterruptEnable |
										  kLPUART_FramingErrorInterruptEnable |
										  kLPUART_ParityErrorInterruptEnable
										  );

			/** Enable IRQ (NVIC) */
			NVIC_SetPriority(enIRQn, kIRQ_PRIORITY_LPUART);
			EnableIRQ(LPUART1_IRQn + enLPUARTNo - enLPUART1);
			bret = true;
		}
	}
	return bret;
}




static const uint32_t s_u32BaseAddr[] = LPUART_BASE_ADDRS;

_Bool DrvLPUARTSend(enLPUART_t enUARTNo, const uint8_t pu8data[], const uint32_t ByteCnt)
{
    _Bool bret = false;
    if ((enUARTNo >= enLPUART_MIN) && (enUARTNo <= enLPUART_MAX) && (pu8data != NULL) && (ByteCnt > 0))
	{
		if (osSemaphoreAcquire(g_bsIdLPUARTTxSemaphore[enUARTNo], portMAX_DELAY) == osOK)
		{
			if(pdFALSE != xPortIsInsideInterrupt()){
				BaseType_t xHigherPriorityTaskWoken = pdFALSE;

				if(xStreamBufferSendFromISR(g_sbhLPUARTTx[enUARTNo], pu8data, ByteCnt, &xHigherPriorityTaskWoken) >= ByteCnt){
					/** MCU Specification !! */
					
					LPUART_EnableInterrupts((LPUART_Type *)s_u32BaseAddr[enUARTNo], kLPUART_TxDataRegEmptyInterruptEnable);
					/** End */
					bret = true;
				}
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}else{
				if(xStreamBufferSend(g_sbhLPUARTTx[enUARTNo], pu8data, ByteCnt, 10) >= ByteCnt){
					/** MCU Specification !! */
					LPUART_EnableInterrupts((LPUART_Type *)s_u32BaseAddr[enUARTNo], kLPUART_TxDataRegEmptyInterruptEnable);
					/** End */
					bret = true;
				}
			}
			/** MCU Specification !! */
			if ((LPUART_GetEnabledInterrupts((LPUART_Type *)s_u32BaseAddr[enUARTNo]) & kLPUART_TxDataRegEmptyInterruptEnable) == 0)
			{
				LPUART_EnableInterrupts((LPUART_Type *)s_u32BaseAddr[enUARTNo], kLPUART_TxDataRegEmptyInterruptEnable);
				
			}
			/** End */
			osSemaphoreRelease(g_bsIdLPUARTTxSemaphore[enUARTNo]);
		}
	}
	return bret;
}

_Bool DrvLPUARTRecv(enLPUART_t enUARTNo, uint8_t pu8data[], const uint32_t ByteCnt, uint32_t u32Timeout)
{
	_Bool bret = false;
    if ((enUARTNo >= enLPUART_MIN) && (enUARTNo <= enLPUART_MAX))
	{
		if (osSemaphoreAcquire(g_bsIdLPUARTRxSemaphore[enUARTNo], portMAX_DELAY) == osOK)
		{
			if(xStreamBufferReceive(g_sbhLPUARTRx[enUARTNo], pu8data, ByteCnt, u32Timeout) >= ByteCnt)
			{
				bret = true;
			}
			osSemaphoreRelease(g_bsIdLPUARTRxSemaphore[enUARTNo]);
		}
	}
	return bret;
}

#include "mimiclib/source/mimiclib.h"

/**
 * @brief getc (Blocking)
 * @param [out] ch Received character
 * @return void
 */
void MIMICLIB_GetChar(TCHAR *ch)
{
	if (ch != NULL)
	{
		DrvLPUARTRecv(kStdioPort, (uint8_t *)ch, sizeof(TCHAR), portMAX_DELAY);
	}
}
/**
 * @brief putc (Blocking)
 * @param [in] ch character
 * @return void
 */
void MIMICLIB_PutChar(TCHAR ch)
{
	DrvLPUARTSend(kStdioPort, (const uint8_t *)&ch, sizeof(TCHAR));
}
/**
 * @brief puts (with Semapore)
 * @param [in] pszStr NULL Terminate String
 * @return void
 */
void MIMICLIB_PutString(const TCHAR pszStr[], uint32_t SizeofStr)
{
	uint32_t ByteCnt = mimic_tcslen(pszStr, SizeofStr)*sizeof(TCHAR);
	DrvLPUARTSend(kStdioPort, (const uint8_t *)pszStr, ByteCnt);
}

/**
 * @brief kbhits
 * @return true There are some characters in Buffer
 * @return false There are no characters in Buffer
 */
_Bool MIMICLIB_kbhit(void){
	return (_Bool)!DrvLPUARTIsRxBufferEmpty(kStdioPort);
}