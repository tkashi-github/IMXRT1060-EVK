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

/** OS */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "timers.h"
#include "semphr.h"
#include "OSResource.h"

/** Board */
#include "MIMXRT1062.h"
#include "UART/DrvLPUART.h"

#ifndef NULL
#define NULL (void*)NULL
#endif

#define kStdioPort enLPUART1


uint32_t mimic_gets(TCHAR pszStr[], uint32_t u32Size);
void mimic_printf(const TCHAR *format, ...);
_Bool mimic_kbhit(void);


/**
 * @brief toupper
 * @param [in] szStr NULL Terminate String
 * @return void
 */
static inline char *mimic_strupper(char szStr[]){
	if(szStr != NULL){
		uint32_t i=0;
		while(szStr[i] != '\0'){
			if((szStr[i] >= 'a') && (szStr[i] <= 'z')){
				szStr[i] += 'A';
				szStr[i] -= 'a';
			}
			i++;
		}
	}
	return szStr;
}
static inline char *mimic_tcsupper(TCHAR szStr[]){
	if(szStr != NULL){
		uint32_t i=0;
		while(szStr[i] != '\0'){
			if((szStr[i] >= 'a') && (szStr[i] <= 'z')){
				szStr[i] += 'A';
				szStr[i] -= 'a';
			}
			i++;
		}
	}
	return szStr;
}
/**
 * @brief strlen
 * @param [in] szStr NULL Terminate String
 * @return uint32_t Length
 */
static inline uint32_t mimic_strlen(const char pszStr[]){
	/*-- var --*/
	uint32_t u32Cnt = 0u;

	/*-- begin --*/
	if(pszStr != NULL){
		while(pszStr[u32Cnt] != '\0'){
			u32Cnt++;
		}

	}
	return u32Cnt;
}
static inline uint32_t mimic_tcslen(const TCHAR pszStr[]){
	/*-- var --*/
	uint32_t u32Cnt = 0u;

	/*-- begin --*/
	if(pszStr != NULL){
		while(pszStr[u32Cnt] != '\0'){
			u32Cnt++;
		}

	}
	return u32Cnt;
}

/**
 * @brief getc (Blocking)
 * @param [out] ch Received character
 * @return void
 */
static inline void RTOS_GetChar(TCHAR *ch)
{
	if (ch != NULL)
	{
		xStreamBufferReceive(g_sbhLPUARTRx[kStdioPort], ch, sizeof(TCHAR), portMAX_DELAY);
	}
}
/**
 * @brief putc (NonBlocking)
 * @param [in] ch character
 * @return void
 */
static inline void RTOS_PutChar(TCHAR ch)
{

	if(pdFALSE != xPortIsInsideInterrupt()){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		if(xStreamBufferSendFromISR(g_sbhLPUARTTx[kStdioPort], &ch, sizeof(TCHAR), &xHigherPriorityTaskWoken) >= 1){
			LPUART1->CTRL |=LPUART_CTRL_TIE_MASK;
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}else{
		if(xStreamBufferSend(g_sbhLPUARTTx[kStdioPort], &ch, sizeof(TCHAR), 10) >= 1){
			LPUART1->CTRL |=LPUART_CTRL_TIE_MASK;
		}
	}
}
/**
 * @brief puts (with Semapore)
 * @param [in] pszStr NULL Terminate String
 * @return void
 */
static inline void RTOS_PutString(const TCHAR pszStr[])
{
	if (xSemaphoreTake(g_xLPUARTTxSemaphore[kStdioPort], portMAX_DELAY) == pdTRUE)
	{
		if (pszStr != NULL)
		{
			uint32_t ByteCnt = mimic_tcslen(pszStr)*sizeof(TCHAR);

			if(pdFALSE != xPortIsInsideInterrupt()){
				BaseType_t xHigherPriorityTaskWoken = pdFALSE;

				if(xStreamBufferSendFromISR(g_sbhLPUARTTx[kStdioPort], pszStr, ByteCnt, &xHigherPriorityTaskWoken) >= ByteCnt){
					LPUART1->CTRL |=LPUART_CTRL_TIE_MASK;
				}
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}else{
				if(xStreamBufferSend(g_sbhLPUARTTx[kStdioPort], pszStr, ByteCnt, 10) >= ByteCnt){
					LPUART1->CTRL |=LPUART_CTRL_TIE_MASK;
				}
			}
			uint32_t ctrl = LPUART1->CTRL;
			if ((ctrl & LPUART_CTRL_TIE_MASK) != LPUART_CTRL_TIE_MASK)
			{
				LPUART1->CTRL |= LPUART_CTRL_TIE_MASK;
			}
		}
		xSemaphoreGive(g_xLPUARTTxSemaphore[kStdioPort]);
	}
}

/**
 * @brief kbhits
 * @return true There are some characters in Buffer
 * @return false There are no characters in Buffer
 */
static inline _Bool RTOS_kbhit(void){
	return (_Bool)xStreamBufferIsEmpty(g_sbhLPUARTTx[kStdioPort]);
}

/**
 * @brief memcmp
 * @param [in] p1 Target Pointer1
 * @param [in] p2 Target Pointer2
 * @param [in] u32ByteCnt Compare Size
 * @return true Match
 * @return false Unmatch
 */
static inline _Bool mimic_memcmp(uintptr_t p1, uintptr_t p2, uint32_t u32ByteCnt){
	/*-- var --*/
	uint8_t *pu81 = (uint8_t*)p1;
	uint8_t *pu82 = (uint8_t*)p2;
	_Bool bret = true;

	/*-- begin --*/
	for(uint32_t i=0u;i<u32ByteCnt;i++){
		if(pu81[i] != pu82[i]){
			bret = false;
			break;
		}
	}
	return bret;
}
static inline _Bool mimic_memcpy(uintptr_t p1, uintptr_t p2, uint32_t u32ByteCnt){
	/*-- var --*/
	uint8_t *pu81 = (uint8_t*)p1;
	uint8_t *pu82 = (uint8_t*)p2;
	_Bool bret = true;

	/*-- begin --*/
	for(uint32_t i=0u;i<u32ByteCnt;i++){
		pu81[i] = pu82[i];
	}
	__DMB();
	return bret;
}
/**
 * @brief strtok_r
 * @param [in] szStr Target String
 * @param [in] szDelm Delemiter
 * @param [in] ctx 
 * @return char* 
 */
static inline char *mimic_strtok(char szStr[], const char szDelm[], char **ctx){
	/*-- var --*/
	char *pret = NULL;

	/*-- begin --*/
	if(szDelm != NULL){
		char *pszTmp;
		uint32_t u32=0u;
		uint32_t delmLen = mimic_strlen(szDelm);

		if(szStr == NULL){
			pszTmp = *ctx;
		}else{
			pszTmp = szStr;
		}
		if(pszTmp[0] != '\0'){
			pret = pszTmp;
			while(pszTmp[u32] != '\0'){
				*ctx = &pszTmp[u32 + delmLen];
				if(mimic_memcmp((uintptr_t)&pszTmp[u32], (uintptr_t)szDelm, delmLen) != false){
					pszTmp[u32] = '\0';
					pret = pszTmp;
					break;
				}
				u32++;
			}
		}
	}
	return pret;
}
static inline char *mimic_tcstok(TCHAR szStr[], const TCHAR szDelm[], TCHAR **ctx){
	/*-- var --*/
	TCHAR *pret = NULL;

	/*-- begin --*/
	if(szDelm != NULL){
		TCHAR *pszTmp;
		uint32_t u32=0u;
		uint32_t delmLen = mimic_tcslen(szDelm);

		if(szStr == NULL){
			pszTmp = *ctx;
		}else{
			pszTmp = szStr;
		}
		if(pszTmp[0] != (TCHAR)'\0'){
			pret = pszTmp;
			while(pszTmp[u32] != (TCHAR)'\0'){
				*ctx = &pszTmp[u32 + delmLen];
				if(mimic_memcmp((uintptr_t)&pszTmp[u32], (uintptr_t)szDelm, sizeof(TCHAR)*delmLen) != false){
					pszTmp[u32] = (TCHAR)'\0';
					pret = pszTmp;
					break;
				}
				u32++;
			}
		}
	}
	return pret;
}

static inline _Bool mimic_isprint(char c){
	if((c >= 0x20) && (c <= 0x7e)){
		return true;
	}else{
		return false;
	}
}


static inline TCHAR * mimic_tcscpy(TCHAR szDst[], const TCHAR pszSrc[], size_t DstSize){
	size_t i = 0u;

	for(i=0;i<DstSize;i++){
		szDst[i] = (TCHAR)'\0';
	}
	i = 0u;
	while(pszSrc[i] != (TCHAR)'\0'){
		szDst[i] = pszSrc[i];
		i++;
		if(i >= DstSize){
			break;
		}
	}

	return szDst;
}
#ifdef __cplusplus
}
#endif

