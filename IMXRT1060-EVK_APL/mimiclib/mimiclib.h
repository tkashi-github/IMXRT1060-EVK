/**
 * @file		mimiclib.h
 * @brief		mimiclib is insteadof stdio.h, stdlib.h and string.h
 * @author		Takashi Kashiwagi
 * @date		2019/6/17
 * @version     0.4.0
 * @details 
 * --
 * License Type (MIT License)
 * --
 * Copyright 2018 - 2019 Takashi Kashiwagi
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
 * - 2018/07/05: Takashi Kashiwagi: v0.1
 * - 2018/10/28: Takashi Kashiwagi: v0.2 for IMXRT1060-EVK
 * - 2019/05/19: Takashi Kashiwagi: v0.3.1
 * - 2019/06/17: Takashi Kashiwagi: v0.4.0
 */
#ifndef __cplusplus
#if __STDC_VERSION__ < 201112L
#error
#endif
#endif
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#ifdef UNIT_TEST
	#ifdef __cplusplus
		#include <cstddef>
		#include <cstdio>
	#endif
#endif

#ifndef __cplusplus
#ifndef NULL
#define NULL (void*)0
#endif
#endif

#ifndef TCHAR_TYPE
typedef char TCHAR;
#endif

/** \defgroup MIMICLIB
  @{
*/

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
/**
 * @brief tcslen
 * @param [in] szStr NULL Terminate String
 * @return uint32_t Length
 */
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

#ifndef UNIT_TEST
/** OS */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "timers.h"
#include "semphr.h"
#include "OSResource.h"

/** Board */
#include "UART/DrvLPUART.h"

#ifndef kStdioPort
#warning "Please set kStdioPort!!"
#define kStdioPort enLPUART1
#endif

#define DEF_NEED_DMB

/**
 * @brief getc (Blocking)
 * @param [out] ch Received character
 * @return void
 */
static inline void RTOS_GetChar(TCHAR *ch)
{
	if (ch != NULL)
	{
		DrvLPUARTRecv(kStdioPort, (uint8_t *)ch, sizeof(TCHAR), portMAX_DELAY);
	}
}
/**
 * @brief putc (NonBlocking)
 * @param [in] ch character
 * @return void
 */
static inline void RTOS_PutChar(TCHAR ch)
{
	DrvLPUARTSend(kStdioPort, (const uint8_t *)&ch, sizeof(TCHAR));
}
/**
 * @brief puts (with Semapore)
 * @param [in] pszStr NULL Terminate String
 * @return void
 */
static inline void RTOS_PutString(const TCHAR pszStr[])
{
	uint32_t ByteCnt = mimic_tcslen(pszStr)*sizeof(TCHAR);
	DrvLPUARTSend(kStdioPort, (const uint8_t *)pszStr, ByteCnt);
}

/**
 * @brief kbhits
 * @return true There are some characters in Buffer
 * @return false There are no characters in Buffer
 */
static inline _Bool RTOS_kbhit(void){
	return (_Bool)!DrvLPUARTIsRxBufferEmpty(kStdioPort);
}
#endif


/**
 * @brief gets
 * @param [out] szStr NULL Terminate String buffer
 * @param [out] u32Size buffer elements size
 * @return elements count
 */
extern uint32_t mimic_gets(TCHAR pszStr[], uint32_t u32Size);

/**
 * @brief printf
 */
extern void mimic_printf(const char* fmt, ...);

/**
 * @brief sprintf
 */
extern void mimic_sprintf(TCHAR szDst[], uint32_t u32MaxElementOfszDst, const char* fmt, ...);


/**
 * @brief kbhit
 * @return true There are some characters in Buffer
 * @return false There are no characters in Buffer
 */
extern _Bool mimic_kbhit(void);

/**
 * @brief tcsvprintf
 */
extern void mimic_tcsvprintf(TCHAR szDst[], uint32_t u32MaxElementOfszDst, const TCHAR szFormat[], va_list arg);

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
/**
 * @brief tcsupper
 * @param [in] szStr NULL Terminate String
 * @return void
 */
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
	if((p1 != (uintptr_t)NULL) && (p2 != (uintptr_t)NULL))
	{
		for(uint32_t i=0u;i<u32ByteCnt;i++)
		{
			if(pu81[i] != pu82[i]){
				bret = false;
				break;
			}
		}
	}
	return bret;
}

/**
 * @brief memcpy
 * @param [in] p1 Target Pointer1 (!= NULL)
 * @param [in] p2 src Pointer2 (!= NULL)
 * @param [in] u32ByteCnt copy Size
 * @return true OK
 * @return false NG (argment error)
 */
static inline _Bool mimic_memcpy(uintptr_t p1, uintptr_t p2, uint32_t u32ByteCnt){
	/*-- var --*/
	uint8_t *pu81 = (uint8_t*)p1;
	uint8_t *pu82 = (uint8_t*)p2;
	_Bool bret = false;

	/*-- begin --*/
	if((p1 != (uintptr_t)NULL) && (p2 != (uintptr_t)NULL))
	{
		for(uint32_t i=0u;i<u32ByteCnt;i++)
		{
			pu81[i] = pu82[i];
		}
		bret = true;
	}
#ifdef DEF_NEED_DMB
	__DMB();
#endif
	return bret;
}

/**
 * @brief memset
 * @param [in] p1 Target Pointer1 (!= NULL)
 * @param [in] val
 * @param [in] u32ByteCnt copy Size
 * @return true OK
 * @return false NG (argment error)
 */
static inline _Bool mimic_memset(uintptr_t p1, uint8_t val, uint32_t u32ByteCnt){
	/*-- var --*/
	uint8_t *pu81 = (uint8_t*)p1;
	_Bool bret = false;

	/*-- begin --*/
	if(p1 != (uintptr_t)NULL)
	{
		for(uint32_t i=0u;i<u32ByteCnt;i++){
			pu81[i] = val;
		}
		bret = true;
	}
#ifdef DEF_NEED_DMB
	__DMB();
#endif
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
	if(szDelm != (char *)NULL){
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

/**
 * @brief tcstok_r
 * @param [in] szStr Target String
 * @param [in] szDelm Delemiter
 * @param [in] ctx 
 * @return char* 
 */
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

/**
 * @brief isprint
 * @param [in] c 
 * @return true printable
 * @return false NG not printable
 */
static inline _Bool mimic_isprint(char c){
	if(((uint8_t)c >= 0x20u) && ((uint8_t)c <= 0x7eu)){
		return true;
	}else{
		return false;
	}
}

/**
 * @brief strncpy
 * @param [inout] szDst (!=NULL)
 * @param [in] szSrc (!=NULL)
 * @param [in] u32DstSize 
 * @return char* 
 */
static inline char *mimic_strcpy(char szDst[], const char szSrc[], const uint32_t u32DstSize)
{
	/*-- var --*/
	uint32_t i = 0u;

	/*-- begin --*/
	if((szDst != (char*)NULL) && (szSrc != (const char*)NULL))
	{
		for(i=0;i<u32DstSize;i++)
		{
			szDst[i] = (TCHAR)'\0';
		}
		i = 0u;
		while(szSrc[i] != (TCHAR)'\0')
		{
			szDst[i] = szSrc[i];
			i++;
			if(i >= u32DstSize)
			{
				break;
			}
		}
	}

	return szDst;
}

/**
 * @brief tcsncpy
 * @param [inout] szDst (!=NULL)
 * @param [in] szSrc (!=NULL)
 * @param [in] u32DstSize 
 * @return char* 
 */
static inline TCHAR * mimic_tcscpy(TCHAR szDst[], const TCHAR szSrc[], uint32_t DstSize)
{
	/*-- var --*/
	uint32_t i = 0u;

	/*-- begin --*/
	if((szDst != (char*)NULL) && (szSrc != (const char*)NULL))
	{
		for(i=0;i<DstSize;i++)
		{
			szDst[i] = (TCHAR)'\0';
		}
		i = 0u;
		while(szSrc[i] != (TCHAR)'\0')
		{
			szDst[i] = szSrc[i];
			i++;
			if(i >= DstSize)
			{
				break;
			}
		}
	}

	return szDst;
}

typedef enum{
	enStr1ltStr2 = -1,
	enStr1eqStr2 = 0,
	enStr1gtStr2 = 1,
	enRangeMax = INT32_MAX,
	enArgmentError = INT32_MIN,
}enRetrunCodeStrCmp_t;

/**
 * @brief strcmp
 * @param [in] szStr1 (!= NULL)
 * @param [in] szStr2 (!= NULL)
 * @param [in] u32NumberOfElements (!= 0)
 * @return enRetrunCodeStrCmp_t
 */
static inline enRetrunCodeStrCmp_t mimic_strcmp(const char szStr1[], const char szStr2[], uint32_t u32NumberOfElements)
{
	if((szStr1 != (const char*)NULL) && (szStr2 != (const char*)NULL) && (u32NumberOfElements != 0))
	{
		for(uint32_t u32Cnt = 0u; u32Cnt < u32NumberOfElements; u32Cnt++)
		{
			if(szStr1[u32Cnt] < szStr2[u32Cnt])
			{
				return enStr1ltStr2;
			}
			if(szStr1[u32Cnt] > szStr2[u32Cnt])
			{
				return enStr1gtStr2;
			}
			if((szStr1[u32Cnt] == '\0') && (szStr2[u32Cnt] == '\0'))
			{
				return enStr1eqStr2;
			}
		}
		return enRangeMax;
	}

	return enArgmentError;
}

/**
 * @brief tcsncmp
 * @param [in] szStr1 (!= NULL)
 * @param [in] szStr2 (!= NULL)
 * @param [in] u32NumberOfElements (!= 0)
 * @return enRetrunCodeStrCmp_t
 */
static inline enRetrunCodeStrCmp_t mimic_tcsncmp(const TCHAR szStr1[], const TCHAR szStr2[], uint32_t u32NumberOfElements)
{
	if((szStr1 != (const TCHAR*)NULL) && (szStr2 != (const TCHAR*)NULL) && (u32NumberOfElements != 0))
	{
		for(uint32_t u32Cnt = 0u; u32Cnt < u32NumberOfElements; u32Cnt++)
		{
			if(szStr1[u32Cnt] < szStr2[u32Cnt])
			{
				return enStr1ltStr2;
			}
			if(szStr1[u32Cnt] > szStr2[u32Cnt])
			{
				return enStr1gtStr2;
			}
			if((szStr1[u32Cnt] == '\0') && (szStr2[u32Cnt] == '\0'))
			{
				return enStr1eqStr2;
			}
		}
		return enRangeMax;
	}

	return enArgmentError;
}

/**
 * @brief ltoa
 * @param [in] i32Val
 * @param [inout] szDst != NULL
 * @param [in] u32NumberOfElements (!= 0)
 * @return TCHAR * (szDst)
 */
static inline TCHAR *mimic_ltoa(const int32_t i32Val, TCHAR szDst[], uint32_t u32MaxElementOfszDst)
{
	uint32_t u32Index = 0;
	int32_t i32Sign;
	uint32_t u32Val;

	if((szDst == NULL) || (u32MaxElementOfszDst == 0))
	{
		return NULL;
	}

	i32Sign = i32Val;
	if(i32Sign < 0)
	{
		u32Val = (uint32_t)-i32Val;
	}
	else
	{
		u32Val = (uint32_t)i32Val;
	}
	mimic_memset((uintptr_t)szDst, 0, sizeof(TCHAR)*u32MaxElementOfszDst);

	while(u32Index < (u32MaxElementOfszDst - 1))
	{
		uint32_t u32 = (u32Val % 10);
		u32Val /= 10;
		szDst[u32Index] = (TCHAR)u32 + (TCHAR)'0'; 
		u32Index++;
		if(u32Val == 0)
		{
			break;
		}
	}

	/* Add sign */
	if(i32Sign < 0)
	{
		szDst[u32Index] = (TCHAR)'-';
		u32Index++;
	}

	/* Reverse */
	for(uint32_t j=0;j<(u32Index/2);j++)
	{
		TCHAR tcTemp = szDst[j];
		szDst[j] = szDst[u32Index - 1 - j];
		szDst[u32Index - 1 - j] = tcTemp;
	}
	return szDst;
}

/**
 * @brief ultoa
 * @param [in] u32Val
 * @param [inout] szDst != NULL
 * @param [in] u32NumberOfElements (!= 0)
 * @param [in] u32Radix (!= 0)
 * @return TCHAR * (szDst)
 */
static inline TCHAR *mimic_ultoa(const uint32_t u32Val, TCHAR szDst[], const uint32_t u32MaxElementOfszDst, const uint32_t u32Radix)
{
	uint32_t u32Index = 0;
	uint32_t u32Temp = u32Val;

	if((szDst == NULL) || (u32MaxElementOfszDst == 0) || (u32Radix == 0))
	{
		return NULL;
	}

	mimic_memset((uintptr_t)szDst, 0, sizeof(TCHAR)*u32MaxElementOfszDst);

	while(u32Index < (u32MaxElementOfszDst - 1))
	{
		uint32_t u32 = (u32Temp % u32Radix);
		u32Temp /= u32Radix;

		if(u32 <= 9)
		{
			szDst[u32Index] = (TCHAR)u32 + (TCHAR)'0';
		}
		else
		{
			szDst[u32Index] = (TCHAR)(u32 - 10) + (TCHAR)'A';
		}
		u32Index++;
		if(u32Temp == 0)
		{
			break;
		}
	}

	/* Reverse*/
	for(uint32_t j=0;j<(u32Index/2);j++)
	{
		TCHAR tcTemp = szDst[j];
		szDst[j] = szDst[u32Index - 1 - j];
		szDst[u32Index - 1 - j] = tcTemp;
	}
	return szDst;
}

/**
 * @brief lltoa
 * @param [in] i64Val
 * @param [inout] szDst != NULL
 * @param [in] u32NumberOfElements (!= 0)
 * @return TCHAR * (szDst)
 */
static inline TCHAR *mimic_lltoa(const int64_t i64Val, TCHAR szDst[], const uint32_t u32MaxElementOfszDst)
{
	uint32_t u32Index = 0;
	int64_t i64Sign;
	uint64_t u64Val;

	if((szDst == NULL) || (u32MaxElementOfszDst == 0))
	{
		return NULL;
	}
	i64Sign = i64Val;
	if(i64Sign < 0)
	{
		u64Val = (uint64_t)-i64Val;
	}
	else
	{
		u64Val = (uint64_t)i64Val;
	}
	mimic_memset((uintptr_t)szDst, 0, sizeof(TCHAR)*u32MaxElementOfszDst);

	while(u32Index < (u32MaxElementOfszDst - 1))
	{
		uint32_t u32 = (uint32_t)(u64Val % 10ull);
		u64Val /= 10ull;
		szDst[u32Index] = (TCHAR)u32 + (TCHAR)'0'; 
		u32Index++;
		if(u64Val == 0)
		{
			break;
		}
	}

	/* Add sign */
	if(i64Sign < 0)
	{
		szDst[u32Index] = (TCHAR)'-';
		u32Index++;
	}

	/* Reverse*/
	for(uint32_t j=0;j<(u32Index/2);j++)
	{
		TCHAR tcTemp = szDst[j];
		szDst[j] = szDst[u32Index - 1 - j];
		szDst[u32Index - 1 - j] = tcTemp;
	}
	return szDst;
}

/**
 * @brief ulltoa
 * @param [in] u64Val
 * @param [inout] szDst != NULL
 * @param [in] u32NumberOfElements (!= 0)
 * @param [in] u32Radix (!= 0)
 * @return TCHAR * (szDst)
 */
static inline TCHAR *mimic_ulltoa(const uint64_t u64Val, TCHAR szDst[], const uint32_t u32MaxElementOfszDst, const uint32_t u32Radix)
{
	uint32_t u32Index = 0;
	uint64_t u64Temp = u64Val;

	if((szDst == NULL) || (u32MaxElementOfszDst == 0) || (u32Radix == 0))
	{
		return NULL;
	}

	mimic_memset((uintptr_t)szDst, 0, sizeof(TCHAR)*u32MaxElementOfszDst);

	while(u32Index < (u32MaxElementOfszDst - 1))
	{
		uint32_t u32 = (uint32_t)(u64Temp % u32Radix);
		u64Temp /= u32Radix;
		if(u32 < 10)
		{
			szDst[u32Index] = (TCHAR)u32 + (TCHAR)'0';
		}
		else
		{
			szDst[u32Index] = (TCHAR)(u32 - 10u) + (TCHAR)'A';
		}
		u32Index++;
		if(u64Temp == 0)
		{
			break;
		}
	}

	/* Reverse*/
	for(uint32_t j=0;j<(u32Index/2);j++)
	{
		TCHAR tcTemp = szDst[j];
		szDst[j] = szDst[u32Index - 1 - j];
		szDst[u32Index - 1 - j] = tcTemp;
	}
	return szDst;
}

/**
 * @brief tcscat
 * @param [inout] pszStr1 (!= NULL)
 * @param [in] u32MaxElementOfszStr1 (!= 0)
 * @param [in] pszStr2 (!= NULL)
 * @return TCHAR * (pszStr1)
 * @return NULL argment NG
 */
static inline TCHAR *mimic_tcscat(TCHAR pszStr1[], uint32_t u32MaxElementOfszStr1, const TCHAR pszStr2[])
{
	uint32_t u32Pos = 0;
	uint32_t u32Index = 0;

	if((pszStr1 == (TCHAR *)NULL) || (u32MaxElementOfszStr1 == 0) || (pszStr2 == 0))
	{
		return NULL;
	}
	while(pszStr1[u32Pos] != (TCHAR)'\0')
	{
		u32Pos++;
		if(u32Pos >= (u32MaxElementOfszStr1 -1))
		{
			return pszStr1;
		}
	}

	while(pszStr2[u32Index] != (TCHAR)'\0')
	{
		pszStr1[u32Pos] = pszStr2[u32Index];
		u32Pos++;
		u32Index++;
		if(u32Pos >= (u32MaxElementOfszStr1 -1))
		{
			return pszStr1;
		}
	}

	return pszStr1;
}

/**
 * @brief ftoa
 * TODO : これは良くない実装
 * @param [in] dfpVal (!= NULL)
 * @param [inout] szDst (!= NULL)
 * @param [in] u32MaxElementOfszDst (!= 0)
 * @param [in] precision_width (!= 0)
 * @return TCHAR * (pszStr1)
 * @return NULL argment NG
 */
static inline TCHAR *mimic_ftoa(const double dfpVal, TCHAR szDst[], const uint32_t u32MaxElementOfszDst, const uint32_t precision_width)
{
	double dfpTemp = dfpVal;
	uint64_t u64Z;
	uint32_t u32Pos = 0;
	uint32_t u32PrecCnt = 0;

	if((szDst == (TCHAR *)NULL) || (u32MaxElementOfszDst == 0) || (precision_width == 0))
	{
		return NULL;
	}
	mimic_memset((uintptr_t)szDst, 0, sizeof(TCHAR)*u32MaxElementOfszDst);
	if(dfpVal < 0.0)
	{
		dfpTemp = -dfpVal;
	}

	u64Z = dfpTemp;
	dfpTemp -= (double)u64Z;

	if(u64Z != 0)
	{
		if(dfpVal < 0.0)
		{
			szDst[0] = (TCHAR)'-';
			mimic_ulltoa(u64Z, &szDst[1], u32MaxElementOfszDst-1, 10);
		}
		else
		{
			mimic_ulltoa(u64Z, szDst, u32MaxElementOfszDst, 10);
		}
		
		u32Pos = mimic_tcslen(szDst);
	}
	else
	{
		if(dfpVal < 0.0)
		{
			szDst[u32Pos] = (TCHAR)'-';
			u32Pos++;
		}
		szDst[u32Pos] = (TCHAR)'0';
		u32Pos++;
	}
	szDst[u32Pos] = (TCHAR)'.';
	u32Pos++;
	u32PrecCnt = 0;

	for(;;)
	{
		dfpTemp *= 10.0;
		if(u32PrecCnt >= (precision_width - 1))
		{
			dfpTemp += 0.5;
		}
		uint32_t u32Z = (uint32_t)dfpTemp;
		dfpTemp -= (double)u32Z;
		
		szDst[u32Pos] = (TCHAR)u32Z + (TCHAR)'0';
		u32Pos++;
		u32PrecCnt++;
		if((dfpTemp == 0.0) || (u32PrecCnt >= precision_width))
		{
			break;
		}

	}

	return szDst;
}


/*@} end of group MIMICLIB */

#ifdef __cplusplus
}
#endif

