/**
 * @file		mimiclib.h
 * @brief		mimiclib is insteadof stdio.h, stdlib.h and string.h
 * @author		Takashi Kashiwagi
 * @date		2019/7/14
 * @version     0.5.0
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
 * - 2019/06/24: Takashi Kashiwagi: v0.4.3
 * - 2019/07/14: Takashi Kashiwagi: v0.5.0
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
#include <stdio.h>
#ifdef __cplusplus
#include <cstddef>
#include <cstdio>

#endif
#endif

/** NULL Pointer */
#ifndef __cplusplus
#ifndef NULL
#define NULL (void *)0
#endif
#endif

/** TCHAR */
#ifndef TCHAR_TYPE
#define TCHAR_TYPE
/** 
 * @brief UTF8 Mode
 */
typedef char TCHAR;
#define _T(x) u8 ## x
#define _TEXT(x) u8 ## x
#endif

/** \defgroup MIMICLIB mimic library
  @{
*/

/**
 * @brief getc (Blocking)
 * @param [out] ch Received character
 * @return void
 */
void MIMICLIB_GetChar(TCHAR *ch);

/**
 * @brief putc (Blocking)
 * @param [in] ch character
 * @return void
 */
void MIMICLIB_PutChar(TCHAR ch);

/**
 * @brief puts (with Semapore)
 * @param [in] pszStr NULL Terminate String
 * @return void
 */
void MIMICLIB_PutString(const TCHAR pszStr[], uint32_t SizeofStr);

/**
 * @brief kbhits
 * @return true There are some characters in Buffer
 * @return false There are no characters in Buffer
 */
_Bool MIMICLIB_kbhit(void);

/**
 * @brief strlen
 * @param [in] pszStr NULL Terminate String (!= NULL)
 * @return uint32_t Length
 */
static inline uint32_t mimic_strlen(const char pszStr[], const uint32_t SizeoOfStr)
{
	/*-- var --*/
	uint32_t u32Cnt = 0u;

	/*-- begin --*/
	if (pszStr != (const char *)NULL)
	{
		while (pszStr[u32Cnt] != '\0')
		{
			u32Cnt++;
			if(u32Cnt >= SizeoOfStr)
			{
				break;
			}
		}
	}
	return u32Cnt;
}
/**
 * @brief tcslen
 * @param [in] pszStr NULL Terminate String (!= NULL)
 * @return uint32_t Length
 */
static inline uint32_t mimic_tcslen(const TCHAR pszStr[], const uint32_t SizeoOfStr)
{
	/*-- var --*/
	uint32_t u32Cnt = 0u;

	/*-- begin --*/
	if (pszStr != (const char *)NULL)
	{
		while (pszStr[u32Cnt] != '\0')
		{
			u32Cnt++;
			if(u32Cnt >= SizeoOfStr)
			{
				break;
			}
		}
	}
	return u32Cnt;
}

/**
 * @brief gets
 * @param [out] pszStr NULL Terminate String buffer (!= NULL)
 * @param [out] u32Size buffer elements size
 * @return elements count
 */
extern uint32_t mimic_gets(TCHAR pszStr[], uint32_t u32Size);

/**
 * @brief printf
 * @param [in] fmt format string
 * @param [in] ... variable list
 * @detail
 * %s option : limit to 256 characters.
 */
extern void mimic_printf(const char *fmt, ...);

/**
 * @brief sprintf
 * @param [out] pszStr NULL Terminate String buffer (!= NULL)
 * @param [in] fmt format string
 * @param [in] ... variable list
 */
extern void mimic_sprintf(TCHAR pszStr[], uint32_t u32MaxElementOfszDst, const char *fmt, ...);

/**
 * @brief kbhit
 * @return true There are some characters in Buffer
 * @return false There are no characters in Buffer
 */
extern _Bool mimic_kbhit(void);

/**
 * @brief tcsvprintf
 * @param [in,out] szDst address of dest buffer
 * @param [in] u32MaxElementOfszDst size of szDst
 * @param [in] szFormat format string
 * @param [in] arg variable list
 */
extern void mimic_tcsvprintf(TCHAR szDst[], uint32_t u32MaxElementOfszDst, const TCHAR szFormat[], va_list arg);

/**
 * @brief toupper
 * @param [in] szStr NULL Terminate String
 * @return void
 */
static inline char *mimic_strupper(char szStr[])
{
	if (szStr != NULL)
	{
		uint32_t i = 0;
		while (szStr[i] != '\0')
		{
			if ((szStr[i] >= 'a') && (szStr[i] <= 'z'))
			{
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
static inline char *mimic_tcsupper(TCHAR szStr[])
{
	if (szStr != NULL)
	{
		uint32_t i = 0;
		while (szStr[i] != '\0')
		{
			if ((szStr[i] >= 'a') && (szStr[i] <= 'z'))
			{
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
static inline char *mimic_tcslowwer(TCHAR szStr[])
{
	if (szStr != NULL)
	{
		uint32_t i = 0;
		while (szStr[i] != '\0')
		{
			if ((szStr[i] >= 'A') && (szStr[i] <= 'Z'))
			{
				szStr[i] += 'a';
				szStr[i] -= 'A';
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
static inline _Bool mimic_memcmp(uintptr_t p1, uintptr_t p2, uint32_t u32ByteCnt)
{
	/*-- var --*/
	uint8_t *pu81 = (uint8_t *)p1;
	uint8_t *pu82 = (uint8_t *)p2;
	_Bool bret = true;

	/*-- begin --*/
	if ((p1 != (uintptr_t)NULL) && (p2 != (uintptr_t)NULL))
	{
		for (uint32_t i = 0u; i < u32ByteCnt; i++)
		{
			if (pu81[i] != pu82[i])
			{
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
static inline _Bool mimic_memcpy(uintptr_t p1, uintptr_t p2, uint32_t u32ByteCnt)
{
	/*-- var --*/
	uint8_t *pu81 = (uint8_t *)p1;
	uint8_t *pu82 = (uint8_t *)p2;
	_Bool bret = false;

	/*-- begin --*/
	if ((p1 != (uintptr_t)NULL) && (p2 != (uintptr_t)NULL))
	{
		for (uint32_t i = 0u; i < u32ByteCnt; i++)
		{
			pu81[i] = pu82[i];
		}
		bret = true;
	}
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
static inline _Bool mimic_memset(uintptr_t p1, uint8_t val, uint32_t u32ByteCnt)
{
	/*-- var --*/
	uint8_t *pu81 = (uint8_t *)p1;
	_Bool bret = false;

	/*-- begin --*/
	if (p1 != (uintptr_t)NULL)
	{
		for (uint32_t i = 0u; i < u32ByteCnt; i++)
		{
			pu81[i] = val;
		}
		bret = true;
	}
	return bret;
}

/**
 * @brief strtok_r
 * @param [in] szStr Target String
 * @param [in] szDelm Delemiter
 * @param [in] ctx 
 * @return char* 
 */
extern char *mimic_strtok(char szStr[], const uint32_t SizeOfStr, const char szDelm[], const uint32_t SizeOfDelm, char **ctx);
/**
 * @brief tcstok_r
 * @param [in] szStr Target String
 * @param [in] szDelm Delemiter
 * @param [in] ctx 
 * @return char* 
 */
extern TCHAR *mimic_tcstok(TCHAR szStr[], const TCHAR szDelm[], const uint32_t SizeOfDelm, TCHAR **ctx);

/**
 * @brief isprint
 * @param [in] c 
 * @return true printable
 * @return false NG not printable
 */
static inline _Bool mimic_isprint(char c)
{
	if (((uint8_t)c >= 0x20u) && ((uint8_t)c <= 0x7eu))
	{
		return true;
	}
	else
	{
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
	if ((szDst != (char *)NULL) && (szSrc != (const char *)NULL))
	{
		for (i = 0; i < u32DstSize; i++)
		{
			szDst[i] = (TCHAR)'\0';
		}
		i = 0u;
		while (szSrc[i] != (TCHAR)'\0')
		{
			szDst[i] = szSrc[i];
			i++;
			if (i >= u32DstSize)
			{
				break;
			}
		}
	}

	return szDst;
}

/**
 * @brief tcsncpy
 * @param [in,out] szDst (!= NULL)
 * @param [in] szSrc (!= NULL)
 * @param [in] u32DstSize 
 * @return char* 
 */
static inline TCHAR *mimic_tcscpy(TCHAR szDst[], const TCHAR szSrc[], uint32_t u32DstSize)
{
	/*-- var --*/
	uint32_t i = 0u;

	/*-- begin --*/
	if ((szDst != (char *)NULL) && (szSrc != (const char *)NULL))
	{
		for (i = 0; i < u32DstSize; i++)
		{
			szDst[i] = (TCHAR)'\0';
		}
		i = 0u;
		while (szSrc[i] != (TCHAR)'\0')
		{
			szDst[i] = szSrc[i];
			i++;
			if (i >= u32DstSize)
			{
				break;
			}
		}
	}

	return szDst;
}

/**
 * @brief Return Code Of strcpy functions
 */
typedef enum
{
	enStr1ltStr2 = -1,
	enStr1eqStr2 = 0,
	enStr1gtStr2 = 1,
	enRangeMax = INT32_MAX,
	enArgmentError = INT32_MIN,
} enRetrunCodeStrCmp_t;

/**
 * @brief strcmp
 * @param [in] szStr1 (!= NULL)
 * @param [in] szStr2 (!= NULL)
 * @param [in] u32NumberOfElements (!= 0)
 * @return enRetrunCodeStrCmp_t
 */
static inline enRetrunCodeStrCmp_t mimic_strcmp(const char szStr1[], const char szStr2[], uint32_t u32NumberOfElements)
{
	if ((szStr1 != (const char *)NULL) && (szStr2 != (const char *)NULL) && (u32NumberOfElements != 0))
	{
		for (uint32_t u32Cnt = 0u; u32Cnt < u32NumberOfElements; u32Cnt++)
		{
			if (szStr1[u32Cnt] < szStr2[u32Cnt])
			{
				return enStr1ltStr2;
			}
			if (szStr1[u32Cnt] > szStr2[u32Cnt])
			{
				return enStr1gtStr2;
			}
			if ((szStr1[u32Cnt] == '\0') && (szStr2[u32Cnt] == '\0'))
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
	if ((szStr1 != (const TCHAR *)NULL) && (szStr2 != (const TCHAR *)NULL) && (u32NumberOfElements != 0))
	{
		for (uint32_t u32Cnt = 0u; u32Cnt < u32NumberOfElements; u32Cnt++)
		{
			if (szStr1[u32Cnt] < szStr2[u32Cnt])
			{
				return enStr1ltStr2;
			}
			if (szStr1[u32Cnt] > szStr2[u32Cnt])
			{
				return enStr1gtStr2;
			}
			if ((szStr1[u32Cnt] == '\0') && (szStr2[u32Cnt] == '\0'))
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
 * @param [in] u32MaxElementOfszDst (!= 0)
 * @return TCHAR * (szDst)
 */
extern TCHAR *mimic_ltoa(const int32_t i32Val, TCHAR szDst[], uint32_t u32MaxElementOfszDst);

/**
 * @brief ultoa
 * @param [in] u32Val
 * @param [inout] szDst != NULL
 * @param [in] u32MaxElementOfszDst (!= 0)
 * @param [in] u32Radix (!= 0)
 * @return TCHAR * (szDst)
 */
extern TCHAR *mimic_ultoa(const uint32_t u32Val, TCHAR szDst[], const uint32_t u32MaxElementOfszDst, const uint32_t u32Radix);

/**
 * @brief lltoa
 * @param [in] i64Val
 * @param [inout] szDst != NULL
 * @param [in] u32MaxElementOfszDst (!= 0)
 * @return TCHAR * (szDst)
 */
extern TCHAR *mimic_lltoa(const int64_t i64Val, TCHAR szDst[], const uint32_t u32MaxElementOfszDst);

/**
 * @brief ulltoa
 * @param [in] u64Val
 * @param [inout] szDst != NULL
 * @param [in] u32MaxElementOfszDst (!= 0)
 * @param [in] u32Radix (!= 0)
 * @return TCHAR * (szDst)
 */
extern TCHAR *mimic_ulltoa(const uint64_t u64Val, TCHAR szDst[], const uint32_t u32MaxElementOfszDst, const uint32_t u32Radix);

/**
 * @brief tcscat
 * @param [inout] pszStr1 (!= NULL)
 * @param [in] u32MaxElementOfszStr1 (!= 0)
 * @param [in] pszStr2 (!= NULL)
 * @return TCHAR * (pszStr1)
 * @return NULL argment NG
 */
extern TCHAR *mimic_tcscat(TCHAR pszStr1[], uint32_t u32MaxElementOfszStr1, const TCHAR pszStr2[]);

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
extern TCHAR *mimic_ftoa(const double dfpVal, TCHAR szDst[], const uint32_t u32MaxElementOfszDst, const uint32_t precision_width);


/**
 * @brief atof
 * @param [in] szStr (!= NULL)
 * @param [in] u32BufSize sizeof(szStr) (!= 0)
 * @return double
 */
extern double mimic_atof(const TCHAR szStr[], const uint32_t u32BufSize);

/**
 * @brief strtol
 * @param [in] szStr (!= NULL)
 * @param [in] u32BufSize sizeof(szStr) (!= 0)
 * @return int32_t
 */
extern int32_t mimic_strtol(const TCHAR szStr[], const uint32_t u32BufSize);

/**
 * @brief strtol
 * @param [in] szStr (!= NULL)
 * @param [in] u32BufSize sizeof(szStr) (!= 0)
 * @return int64_t
 */
extern int64_t mimic_strtoll(const TCHAR szStr[], const uint32_t u32BufSize);

/**
 * @brief strtoul
 * @param [in] szStr (!= NULL)
 * @param [in] u32Base (10 or 16)
 * @param [in] u32BufSize sizeof(szStr) (!= 0)
 * @return uint32_t
 */
extern uint32_t mimic_strtoul(const TCHAR szStr[], const uint32_t u32BufSize, const uint32_t u32Base);

/**
 * @brief strtoull
 * @param [in] szStr (!= NULL)
 * @param [in] u32Base (10 or 16)
 * @param [in] u32BufSize sizeof(szStr) (!= 0)
 * @return uint64_t
 */
extern uint64_t mimic_strtoull(const TCHAR szStr[], const uint32_t u32BufSize, const uint32_t u32Base);
/*@} end of group MIMICLIB */

#ifdef __cplusplus
}
#endif
