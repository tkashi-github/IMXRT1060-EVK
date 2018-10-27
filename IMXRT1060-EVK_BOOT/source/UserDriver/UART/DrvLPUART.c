/**
 * @file TODO
 * @brief TODO
 * @author Takashi Kashiwagi
 * @date        2018/10/23
 * @version     0.1
 * @copyright   TODO
 * 
 * @par Update:
 * - 2018/10/23: Takashi Kashiwagi: for IMXRT1060-EVK
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
void LPUART1HandleIRQ(void){
	LPUARTXHandleIRQ(enLPUART1);
}
/**
 * @brief LPUART2 IRQ Handler
 * @return void
 */
void LPUART2HandleIRQ(void){
	LPUARTXHandleIRQ(enLPUART2);
}
/**
 * @brief LPUART3 IRQ Handler
 * @return void
 */
void LPUART3HandleIRQ(void){
	LPUARTXHandleIRQ(enLPUART3);
}
/**
 * @brief LPUART4 IRQ Handler
 * @return void
 */
void LPUART4HandleIRQ(void){
	LPUARTXHandleIRQ(enLPUART4);
}
/**
 * @brief LPUART5 IRQ Handler
 * @return void
 */
void LPUART5HandleIRQ(void){
	LPUARTXHandleIRQ(enLPUART5);
}
/**
 * @brief LPUART6 IRQ Handler
 * @return void
 */
void LPUART6HandleIRQ(void){
	LPUARTXHandleIRQ(enLPUART6);
}
/**
 * @brief LPUART7 IRQ Handler
 * @return void
 */
void LPUART7HandleIRQ(void){
	LPUARTXHandleIRQ(enLPUART7);
}
/**
 * @brief LPUART8 IRQ Handler
 * @return void
 */
void LPUART8HandleIRQ(void){
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

		if(1u != xStreamBufferReceiveFromISR(g_sbhLPUARTTx[enLPUARTNo], &u8val, 1, &xHigherPriorityTaskWoken)){
			return false;
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}else{
		if(1u != xStreamBufferReceive(g_sbhLPUARTTx[enLPUARTNo], &u8val, 1, 10)){
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
		osEventFlagsSet(g_xLPUARTEventGroup[enLPUARTNo], 1);
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

		osEventFlagsSet(g_xLPUARTEventGroup[enLPUARTNo], 1);
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

			/* Initialize OS resource */
			xSemaphoreGive(g_xLPUARTTxSemaphore[enLPUARTNo]);
			xSemaphoreGive(g_xLPUARTRxSemaphore[enLPUARTNo]);

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



