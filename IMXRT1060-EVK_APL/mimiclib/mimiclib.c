/**
 * @file		mimiclib.c
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
#include "mimiclib.h"

#ifdef UNIT_TEST
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void MIMICLIB_GetChar(TCHAR *ch)
{
	*ch = getc(stdin);
}

void MIMICLIB_PutChar(TCHAR ch)
{
	putc(ch, stdout);
}

void MIMICLIB_PutString(const TCHAR pszStr[])
{
	fputs(pszStr, stdout);
}

_Bool MIMICLIB_kbhit(void)
{
	return true;
}
#endif

uint32_t mimic_gets(char pszStr[], uint32_t u32Size)
{
	uint32_t u32Cnt = 0u;

	if ((pszStr != NULL) && (u32Size > 0u))
	{
		_Bool bReturnCode = false;

		mimic_memset((uintptr_t)pszStr, 0, u32Size);

		while (bReturnCode == false)
		{
			char ch;
			MIMICLIB_GetChar(&ch);

			switch (ch)
			{
			case '\b': // バックスペース
				if (u32Cnt > 0u)
				{
					u32Cnt--;
					pszStr[u32Cnt] = '\0';
					MIMICLIB_PutChar('\b');
					MIMICLIB_PutChar(' ');
					MIMICLIB_PutChar('\b');
				}
				break;
			case '\r': // TeraTermの改行コードは "CR"設定ではCRのみ送られてくる（CRLFにならない）
				//u32Cnt--;
				pszStr[u32Cnt] = '\0';
				bReturnCode = true;
				MIMICLIB_PutChar((char)ch);
				MIMICLIB_PutChar('\n'); // 相手はWindowsと仮定してLRも送信する
				break;
			default:
				pszStr[u32Cnt] = ch;
				u32Cnt++;
				if (u32Cnt >= u32Size)
				{
					u32Cnt--;
					pszStr[u32Cnt] = '\0';
					break;
				}
				MIMICLIB_PutChar((char)ch);
				break;
			}
		}
	}

	return u32Cnt;
}

_Bool mimic_kbhit(void)
{
	return MIMICLIB_kbhit();
}

void mimic_printf(const char *fmt, ...)
{
	va_list arg;
	char szBuffer[512];

	va_start(arg, fmt);
	mimic_tcsvprintf(szBuffer, sizeof(szBuffer), fmt, arg);
	va_end(arg);

	MIMICLIB_PutString(szBuffer, sizeof(szBuffer));
}
void mimic_sprintf(TCHAR pszStr[], uint32_t u32MaxElementOfszDst, const char* fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	mimic_tcsvprintf(pszStr, u32MaxElementOfszDst, fmt, arg);
	va_end(arg);
}

/**
 * @brief print format Flags
 */
typedef enum
{
	enPrintfFlagsMinus = 0x01U,
	enPrintfFlagsPlus = 0x02U,
	enPrintfFlagsSpace = 0x04U,
	enPrintfFlagsZero = 0x08U,
	enPrintfFlagsPound = 0x10U,
	enPrintfFlagsLengthChar = 0x20U,
	enPrintfFlagsLengthShortInt = 0x40U,
	enPrintfFlagsLengthLongInt = 0x80U,
	enPrintfFlagsLengthLongLongInt = 0x100U,
} enPrintfFlags_t;

void mimic_tcsvprintf(
	TCHAR szDst[],
	uint32_t u32MaxElementOfszDst,
	const TCHAR szFormat[],
	va_list arg)
{
	TCHAR *pszStr = NULL;
	uint32_t u32Cnt = 0;
	uint32_t u32FlagsUsed = 0;
	uint32_t u32FlagsWidth = 0;
	_Bool bValidFlagsWidth = false;
	uint32_t u32PrecisionWidth = 0;
	_Bool bValidPrecisionWidth = false;
	TCHAR vstr[33] = {0};
	uint32_t vlen = 0;

	/* begin */
	if ((szDst == NULL) ||
		(szFormat == NULL))
	{
		return;
	}
	mimic_memset((uintptr_t)szDst, 0, sizeof(TCHAR) * u32MaxElementOfszDst);

	pszStr = (TCHAR *)szFormat;
	while (*pszStr != (TCHAR)'\0')
	{
		TCHAR ch = *pszStr;

		if (ch != (TCHAR)'%')
		{
			szDst[u32Cnt] = *pszStr;
			u32Cnt++;
			pszStr++;
		}
		else
		{
			for (;;)
			{
				pszStr++;
				if (*pszStr == '-')
				{
					u32FlagsUsed |= enPrintfFlagsMinus;
				}
				else if (*pszStr == (TCHAR)'+')
				{
					u32FlagsUsed |= enPrintfFlagsPlus;
				}
				else if (*pszStr == (TCHAR)'0')
				{
					u32FlagsUsed |= enPrintfFlagsZero;
				}
				else
				{
					--pszStr;
					break;
				}
			}
			u32FlagsWidth = 0;
			bValidFlagsWidth = false;
			for (;;)
			{
				pszStr++;
				TCHAR ch = *pszStr;
				if ((ch >= (TCHAR)'0') && (ch <= (TCHAR)'9'))
				{
					bValidFlagsWidth = true;
					u32FlagsWidth = (u32FlagsWidth * 10) + (ch - (TCHAR)'0');
				}
				else if (ch == (TCHAR)'*')
				{
					bValidFlagsWidth = true;
					u32FlagsWidth = (uint32_t)va_arg(arg, uint32_t);
				}
				else
				{
					--pszStr;
					break;
				}
			}

			u32PrecisionWidth = 6;
			bValidPrecisionWidth = false;
			pszStr++;
			if (*pszStr == (TCHAR)'.')
			{
				u32PrecisionWidth = 0;
				for (;;)
				{
					pszStr++;
					TCHAR ch = *pszStr;
					if ((ch >= (TCHAR)'0') && (ch <= (TCHAR)'9'))
					{
						u32PrecisionWidth = (u32PrecisionWidth * 10) + (ch - (TCHAR)'0');
						bValidPrecisionWidth = true;
					}
					else if (ch == (TCHAR)'*')
					{
						u32PrecisionWidth = (uint32_t)va_arg(arg, uint32_t);
						bValidPrecisionWidth = true;
					}
					else
					{
						--pszStr;
						break;
					}
				}
			}
			else
			{
				--pszStr;
			}

			pszStr++;
			switch (*pszStr)
			{
			case (TCHAR)'l':
				pszStr++;
				if (*pszStr != (TCHAR)'l')
				{
					u32FlagsUsed |= enPrintfFlagsLengthLongInt;
					--pszStr;
				}
				else
				{
					u32FlagsUsed |= enPrintfFlagsLengthLongLongInt;
				}
				break;
			default:
				--pszStr;
				break;
			}

			pszStr++;
			ch = *pszStr;
			{
				if ((ch == (TCHAR)'d') ||
					(ch == (TCHAR)'i') ||
					(ch == (TCHAR)'f') ||
					(ch == (TCHAR)'F') ||
					(ch == (TCHAR)'x') ||
					(ch == (TCHAR)'X') ||
					(ch == (TCHAR)'u') ||
					(ch == (TCHAR)'U'))
				{
					if ((ch == (TCHAR)'d') || (ch == (TCHAR)'i'))
					{
						if (u32FlagsUsed & enPrintfFlagsLengthLongLongInt)
						{
							mimic_lltoa((int64_t)va_arg(arg, int64_t), vstr, sizeof(vstr));
						}
						else
						{
							mimic_ltoa((int32_t)va_arg(arg, int32_t), vstr, sizeof(vstr));
						}
						vlen = mimic_tcslen(vstr, sizeof(vstr));
					}
					else if ((ch == (TCHAR)'f') || (ch == (TCHAR)'F'))
					{
						if (bValidPrecisionWidth == false)
						{
							mimic_ftoa((double)va_arg(arg, double), vstr, sizeof(vstr), 6);
						}
						else
						{
							mimic_ftoa((double)va_arg(arg, double), vstr, sizeof(vstr), u32PrecisionWidth);
						}
						vlen = mimic_tcslen(vstr, sizeof(vstr));
					}
					else if ((ch == (TCHAR)'X') || (ch == (TCHAR)'x'))
					{
						if (u32FlagsUsed & enPrintfFlagsLengthLongLongInt)
						{
							mimic_ulltoa((uint64_t)va_arg(arg, uint64_t), vstr, sizeof(vstr), 16);
						}
						else
						{
							mimic_ultoa((uint32_t)va_arg(arg, uint32_t), vstr, sizeof(vstr), 16);
						}
						vlen = mimic_tcslen(vstr, sizeof(vstr));
					}
					else if ((ch == (TCHAR)'U') || (ch == (TCHAR)'u'))
					{
						if (u32FlagsUsed & enPrintfFlagsLengthLongLongInt)
						{
							mimic_ulltoa((uint64_t)va_arg(arg, uint64_t), vstr, sizeof(vstr), 10);
						}
						else
						{
							mimic_ultoa((uint32_t)va_arg(arg, uint32_t), vstr, sizeof(vstr), 10);
						}
						vlen = mimic_tcslen(vstr, sizeof(vstr));
					}
					else
					{
						/* NOP */
						vlen = 0;
					}

					if (u32FlagsWidth > 0)
					{
						if (vlen >= u32FlagsWidth)
						{
							for (uint32_t i = 0; i < u32FlagsWidth; i++)
							{
								vstr[i] = vstr[i + (vlen - u32FlagsWidth)];
							}
							vlen = u32FlagsWidth;
							vstr[vlen] = (TCHAR)'\0';
						}
						else
						{
							uint32_t u32 = u32FlagsWidth - vlen;
							TCHAR szTemp[64];

							TCHAR tcTemp = (TCHAR)' ';
							if ((u32FlagsUsed & enPrintfFlagsZero) == enPrintfFlagsZero)
							{
								tcTemp = (TCHAR)'0';
							}
							mimic_tcscpy(szTemp, vstr, sizeof(szTemp));

							uint32_t i;
							for (i = 0; i < u32; i++)
							{
								vstr[i] = tcTemp;
							}
							vstr[i] = (TCHAR)'\0';

							mimic_tcscat(vstr, sizeof(vstr), szTemp);
							vlen = u32FlagsWidth;
							vstr[vlen] = (TCHAR)'\0';
						}
					}

					mimic_tcscat(&szDst[u32Cnt], u32MaxElementOfszDst - u32Cnt, vstr);
					u32Cnt += vlen;
					pszStr++;
				}
				else if (ch == (TCHAR)'c')
				{
					szDst[u32Cnt] = (TCHAR)va_arg(arg, uint32_t);
					u32Cnt++;
					pszStr++;
				}
				else if (ch == (TCHAR)'s')
				{
					TCHAR *psz = (TCHAR *)va_arg(arg, TCHAR *);
					if (psz != NULL)
					{
						if (bValidFlagsWidth == false)
						{
							mimic_tcscat(&szDst[u32Cnt], u32MaxElementOfszDst - u32Cnt, psz);
							u32Cnt += mimic_tcslen(psz, 256);
						}
						else
						{
							vlen = mimic_tcslen(psz, 256);
							if (vlen > u32FlagsWidth)
							{
								for (uint32_t i = 0; i < u32FlagsWidth; i++)
								{
									szDst[u32Cnt] = psz[i];
									u32Cnt++;
								}
							}
							else
							{
								uint32_t u32 = u32FlagsWidth - vlen;
								if ((u32FlagsUsed & enPrintfFlagsMinus) != enPrintfFlagsMinus)
								{
									/* zero */
									for (uint32_t i = 0; i < vlen; i++)
									{
										szDst[u32Cnt] = psz[i];
										u32Cnt++;
									}
									for (uint32_t i = 0; i < u32; i++)
									{
										szDst[u32Cnt] = (TCHAR)' ';
										u32Cnt++;
									}
								}
								else
								{
									for (uint32_t i = 0; i < u32; i++)
									{
										szDst[u32Cnt] = (TCHAR)' ';
										u32Cnt++;
									}
									for (uint32_t i = 0; i < vlen; i++)
									{
										szDst[u32Cnt] = psz[i];
										u32Cnt++;
									}
								}
							}
							szDst[u32Cnt] = (TCHAR)'\0';
						}
					}
					pszStr++;
				}
				else
				{
					szDst[u32Cnt] = *pszStr;
					u32Cnt++;
					pszStr++;
				}
			}
		}
	}
	return;
}

char *mimic_strtok(char szStr[], const uint32_t SizeOfStr, const char szDelm[], const uint32_t SizeOfDelm, char **ctx)
{
	/*-- var --*/
	char *pret = NULL;

	/*-- begin --*/
	if (szDelm != (char *)NULL)
	{
		char *pszTmp;
		uint32_t u32 = 0u;
		uint32_t delmLen = mimic_strlen(szDelm, SizeOfDelm);

		if (szStr == NULL)
		{
			pszTmp = *ctx;
		}
		else
		{
			pszTmp = szStr;
		}
		if (pszTmp[0] != '\0')
		{
			pret = pszTmp;
			while (pszTmp[u32] != '\0')
			{
				*ctx = &pszTmp[u32 + delmLen];
				if (mimic_memcmp((uintptr_t)&pszTmp[u32], (uintptr_t)szDelm, delmLen) != false)
				{
					pszTmp[u32] = '\0';
					pret = pszTmp;
					break;
				}
				u32++;
				if(u32 >= SizeOfStr)
				{
					break;
				}
			}
		}
	}
	return pret;
}

TCHAR *mimic_tcstok(TCHAR szStr[], const TCHAR szDelm[], const uint32_t SizeOfDelm, TCHAR **ctx)
{
	/*-- var --*/
	TCHAR *pret = NULL;

	/*-- begin --*/
	if (szDelm != NULL)
	{
		TCHAR *pszTmp;
		uint32_t u32 = 0u;
		uint32_t delmLen = mimic_tcslen(szDelm, SizeOfDelm);

		if (szStr == NULL)
		{
			pszTmp = *ctx;
		}
		else
		{
			pszTmp = szStr;
		}
		if (pszTmp[0] != (TCHAR)'\0')
		{
			pret = pszTmp;
			while (pszTmp[u32] != (TCHAR)'\0')
			{
				*ctx = &pszTmp[u32 + delmLen];
				if (mimic_memcmp((uintptr_t)&pszTmp[u32], (uintptr_t)szDelm, sizeof(TCHAR) * delmLen) != false)
				{
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

TCHAR *mimic_ltoa(const int32_t i32Val, TCHAR szDst[], uint32_t u32MaxElementOfszDst)
{
	uint32_t u32Index = 0;
	int32_t i32Sign;
	uint32_t u32Val;

	if ((szDst == NULL) || (u32MaxElementOfszDst == 0))
	{
		return NULL;
	}

	i32Sign = i32Val;
	if (i32Sign < 0)
	{
		u32Val = (uint32_t)-i32Val;
	}
	else
	{
		u32Val = (uint32_t)i32Val;
	}
	mimic_memset((uintptr_t)szDst, 0, sizeof(TCHAR) * u32MaxElementOfszDst);

	while (u32Index < (u32MaxElementOfszDst - 1))
	{
		uint32_t u32 = (u32Val % 10);
		u32Val /= 10;
		szDst[u32Index] = (TCHAR)u32 + (TCHAR)'0';
		u32Index++;
		if (u32Val == 0)
		{
			break;
		}
	}

	/* Add sign */
	if (i32Sign < 0)
	{
		szDst[u32Index] = (TCHAR)'-';
		u32Index++;
	}

	/* Reverse */
	for (uint32_t j = 0; j < (u32Index / 2); j++)
	{
		TCHAR tcTemp = szDst[j];
		szDst[j] = szDst[u32Index - 1 - j];
		szDst[u32Index - 1 - j] = tcTemp;
	}
	return szDst;
}

TCHAR *mimic_ultoa(const uint32_t u32Val, TCHAR szDst[], const uint32_t u32MaxElementOfszDst, const uint32_t u32Radix)
{
	uint32_t u32Index = 0;
	uint32_t u32Temp = u32Val;

	if ((szDst == NULL) || (u32MaxElementOfszDst == 0) || (u32Radix == 0))
	{
		return NULL;
	}

	mimic_memset((uintptr_t)szDst, 0, sizeof(TCHAR) * u32MaxElementOfszDst);

	while (u32Index < (u32MaxElementOfszDst - 1))
	{
		uint32_t u32 = (u32Temp % u32Radix);
		u32Temp /= u32Radix;

		if (u32 <= 9)
		{
			szDst[u32Index] = (TCHAR)u32 + (TCHAR)'0';
		}
		else
		{
			szDst[u32Index] = (TCHAR)(u32 - 10) + (TCHAR)'A';
		}
		u32Index++;
		if (u32Temp == 0)
		{
			break;
		}
	}

	/* Reverse*/
	for (uint32_t j = 0; j < (u32Index / 2); j++)
	{
		TCHAR tcTemp = szDst[j];
		szDst[j] = szDst[u32Index - 1 - j];
		szDst[u32Index - 1 - j] = tcTemp;
	}
	return szDst;
}

TCHAR *mimic_lltoa(const int64_t i64Val, TCHAR szDst[], const uint32_t u32MaxElementOfszDst)
{
	uint32_t u32Index = 0;
	int64_t i64Sign;
	uint64_t u64Val;

	if ((szDst == NULL) || (u32MaxElementOfszDst == 0))
	{
		return NULL;
	}
	i64Sign = i64Val;
	if (i64Sign < 0)
	{
		u64Val = (uint64_t)-i64Val;
	}
	else
	{
		u64Val = (uint64_t)i64Val;
	}
	mimic_memset((uintptr_t)szDst, 0, sizeof(TCHAR) * u32MaxElementOfszDst);

	while (u32Index < (u32MaxElementOfszDst - 1))
	{
		uint32_t u32 = (uint32_t)(u64Val % 10ull);
		u64Val /= 10ull;
		szDst[u32Index] = (TCHAR)u32 + (TCHAR)'0';
		u32Index++;
		if (u64Val == 0)
		{
			break;
		}
	}

	/* Add sign */
	if (i64Sign < 0)
	{
		szDst[u32Index] = (TCHAR)'-';
		u32Index++;
	}

	/* Reverse*/
	for (uint32_t j = 0; j < (u32Index / 2); j++)
	{
		TCHAR tcTemp = szDst[j];
		szDst[j] = szDst[u32Index - 1 - j];
		szDst[u32Index - 1 - j] = tcTemp;
	}
	return szDst;
}

TCHAR *mimic_ulltoa(const uint64_t u64Val, TCHAR szDst[], const uint32_t u32MaxElementOfszDst, const uint32_t u32Radix)
{
	uint32_t u32Index = 0;
	uint64_t u64Temp = u64Val;

	if ((szDst == NULL) || (u32MaxElementOfszDst == 0) || (u32Radix == 0))
	{
		return NULL;
	}

	mimic_memset((uintptr_t)szDst, 0, sizeof(TCHAR) * u32MaxElementOfszDst);

	while (u32Index < (u32MaxElementOfszDst - 1))
	{
		uint32_t u32 = (uint32_t)(u64Temp % u32Radix);
		u64Temp /= u32Radix;
		if (u32 < 10)
		{
			szDst[u32Index] = (TCHAR)u32 + (TCHAR)'0';
		}
		else
		{
			szDst[u32Index] = (TCHAR)(u32 - 10u) + (TCHAR)'A';
		}
		u32Index++;
		if (u64Temp == 0)
		{
			break;
		}
	}

	/* Reverse*/
	for (uint32_t j = 0; j < (u32Index / 2); j++)
	{
		TCHAR tcTemp = szDst[j];
		szDst[j] = szDst[u32Index - 1 - j];
		szDst[u32Index - 1 - j] = tcTemp;
	}
	return szDst;
}

TCHAR *mimic_tcscat(TCHAR pszStr1[], uint32_t u32MaxElementOfszStr1, const TCHAR pszStr2[])
{
	uint32_t u32Pos = 0;
	uint32_t u32Index = 0;

	if ((pszStr1 == (TCHAR *)NULL) || (u32MaxElementOfszStr1 == 0) || (pszStr2 == 0))
	{
		return NULL;
	}
	while (pszStr1[u32Pos] != (TCHAR)'\0')
	{
		u32Pos++;
		if (u32Pos >= (u32MaxElementOfszStr1 - 1))
		{
			return pszStr1;
		}
	}

	while (pszStr2[u32Index] != (TCHAR)'\0')
	{
		pszStr1[u32Pos] = pszStr2[u32Index];
		u32Pos++;
		u32Index++;
		if (u32Pos >= (u32MaxElementOfszStr1 - 1))
		{
			return pszStr1;
		}
	}

	return pszStr1;
}

TCHAR *mimic_ftoa(const double dfpVal, TCHAR szDst[], const uint32_t u32MaxElementOfszDst, const uint32_t precision_width)
{
	double dfpTemp = dfpVal;
	uint64_t u64Z;
	uint32_t u32Pos = 0;
	uint32_t u32PrecCnt = 0;

	if ((szDst == (TCHAR *)NULL) || (u32MaxElementOfszDst == 0) || (precision_width == 0))
	{
		return NULL;
	}

	mimic_memset((uintptr_t)szDst, 0, sizeof(TCHAR) * u32MaxElementOfszDst);
	if (dfpVal < 0.0)
	{
		dfpTemp = -dfpVal;
	}

	u64Z = dfpTemp;
	dfpTemp -= (double)u64Z;

	if (u64Z != 0)
	{
		if (dfpVal < 0.0)
		{
			szDst[0] = (TCHAR)'-';
			mimic_ulltoa(u64Z, &szDst[1], u32MaxElementOfszDst - 1, 10);
		}
		else
		{
			mimic_ulltoa(u64Z, szDst, u32MaxElementOfszDst, 10);
		}

		u32Pos = mimic_tcslen(szDst, u32MaxElementOfszDst);
	}
	else
	{
		if (dfpVal < 0.0)
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

	for (;;)
	{
		dfpTemp *= 10.0;
		if (u32PrecCnt >= (precision_width - 1))
		{
			dfpTemp += 0.5;
		}
		uint32_t u32Z = (uint32_t)dfpTemp;
		dfpTemp -= (double)u32Z;

		szDst[u32Pos] = (TCHAR)u32Z + (TCHAR)'0';
		u32Pos++;
		u32PrecCnt++;
		if ((dfpTemp == 0.0) || (u32PrecCnt >= precision_width))
		{
			break;
		}
	}

	return szDst;
}

double mimic_atof(const TCHAR szStr[], const uint32_t u32BufSize)
{
	volatile double dfp = 0.0;
	uint32_t i = 0;
	int32_t sign = 1;

	if ((szStr == NULL) || (u32BufSize == 0u))
	{
		return 0.0;
	}

	while (szStr[i] == ' ')
	{
		i++;
		if (i >= u32BufSize)
		{
			return 0.0;
		}
	}

	if (szStr[i] == '-')
	{
		i++;
		sign = -1;
	}

	while ((szStr[i] != '\0') && (i < u32BufSize))
	{
		if ((szStr[i] >= '0') && (szStr[i] <= '9'))
		{
			uint32_t tmp = (uint32_t)(szStr[i] - '0');
			dfp *= 10;
			dfp += tmp;
		}
		else if (szStr[i] == '.')
		{
			i++;
			break;
		}
		else
		{
			/* NOP */
		}
		i++;
	}

	{
		uint32_t u32DecimalPoint = 1u;
		volatile double dfp2 = 0.0;
		while ((szStr[i] != '\0') && (i < u32BufSize))
		{
			if ((szStr[i] >= '0') && (szStr[i] <= '9'))
			{
				uint32_t tmp = (uint32_t)(szStr[i] - '0');
				u32DecimalPoint *= 10;
				dfp2 += (double)tmp / (double)u32DecimalPoint;
			}
			else
			{
				break;
			}
			i++;
		}
		dfp += dfp2;
	}
	return sign * (dfp);
}

int32_t mimic_strtol(const TCHAR szStr[], const uint32_t u32BufSize)
{
	int32_t iret = 0;
	uint32_t u32Val = 0;
	uint32_t i = 0;
	int32_t sign = 1;

	if ((szStr == NULL) || (u32BufSize == 0u))
	{
		return 0;
	}

	while (szStr[i] == ' ')
	{
		i++;
		if (i >= u32BufSize)
		{
			return 0;
		}
	}

	if (szStr[i] == '-')
	{
		i++;
		sign = -1;
	}

	while ((szStr[i] != '\0') && (i < u32BufSize))
	{
		if ((szStr[i] >= '0') && (szStr[i] <= '9'))
		{
			uint32_t tmp = (uint32_t)(szStr[i] - '0');
			u32Val *= 10;
			u32Val += tmp;
		}
		else
		{
			/* NOP */
		}
		i++;
	}

	if (u32Val >= INT32_MAX)
	{
		iret = INT32_MAX;
	}

	iret = (int32_t)u32Val;
	return sign * iret;
}

int64_t mimic_strtoll(const TCHAR szStr[], const uint32_t u32BufSize)
{
	int64_t iret = 0;
	uint64_t u64Val = 0;
	uint32_t i = 0;
	int32_t sign = 1;

	if ((szStr == NULL) || (u32BufSize == 0u))
	{
		return 0;
	}

	while (szStr[i] == ' ')
	{
		i++;
		if (i >= u32BufSize)
		{
			return 0;
		}
	}

	if (szStr[i] == '-')
	{
		i++;
		sign = -1;
	}

	while ((szStr[i] != '\0') && (i < u32BufSize))
	{
		if ((szStr[i] >= '0') && (szStr[i] <= '9'))
		{
			uint64_t tmp = (uint64_t)(szStr[i] - '0');
			u64Val *= 10;
			u64Val += tmp;
		}
		else
		{
			/* NOP */
		}
		i++;
	}

	if (u64Val >= INT64_MAX)
	{
		iret = INT64_MAX;
	}

	iret = (int64_t)u64Val;
	return sign * iret;
}

uint32_t mimic_strtoul(const TCHAR szStr[], const uint32_t u32BufSize, const uint32_t u32Base)
{
	uint32_t u32Val = 0;
	uint32_t i = 0;
	if ((szStr == NULL) || (u32BufSize == 0u) || ((u32Base != 10u) && (u32Base != 16u)))
	{
		return 0;
	}
	while (szStr[i] == ' ')
	{
		i++;
		if (i >= u32BufSize)
		{
			return 0;
		}
	}

	while ((szStr[i] != '\0') && (i < u32BufSize))
	{
		if ((szStr[i] >= '0') && (szStr[i] <= '9'))
		{
			uint32_t tmp = (uint32_t)(szStr[i] - '0');
			u32Val *= u32Base;
			u32Val += tmp;
		}
		else if ((szStr[i] >= 'a') && (szStr[i] <= 'f'))
		{
			uint32_t tmp = (uint32_t)(szStr[i] - 'a') + 10u;
			u32Val *= u32Base;
			u32Val += tmp;
		}
		else if ((szStr[i] >= 'A') && (szStr[i] <= 'F'))
		{
			uint32_t tmp = (uint32_t)(szStr[i] - 'A') + 10u;
			u32Val *= u32Base;
			u32Val += tmp;
		}
		else
		{
			/* NOP */
		}
		i++;
	}

	return u32Val;
}

uint64_t mimic_strtoull(const TCHAR szStr[], const uint32_t u32BufSize, const uint32_t u32Base)
{
	uint64_t u64Val = 0;
	uint32_t i = 0;
	if ((szStr == NULL) || (u32BufSize == 0u) || ((u32Base != 10u) && (u32Base != 16u)))
	{
		return 0;
	}
	while (szStr[i] == ' ')
	{
		i++;
		if (i >= u32BufSize)
		{
			return 0;
		}
	}

	while ((szStr[i] != '\0') && (i < u32BufSize))
	{
		if ((szStr[i] >= '0') && (szStr[i] <= '9'))
		{
			uint64_t tmp = (uint64_t)(szStr[i] - '0');
			u64Val *= u32Base;
			u64Val += tmp;
		}
		else if ((szStr[i] >= 'a') && (szStr[i] <= 'f'))
		{
			uint64_t tmp = (uint64_t)(szStr[i] - 'a') + 10u;
			u64Val *= u32Base;
			u64Val += tmp;
		}
		else if ((szStr[i] >= 'A') && (szStr[i] <= 'F'))
		{
			uint64_t tmp = (uint64_t)(szStr[i] - 'A') + 10u;
			u64Val *= u32Base;
			u64Val += tmp;
		}
		else
		{
			/* NOP */
		}
		i++;
	}

	return u64Val;
}