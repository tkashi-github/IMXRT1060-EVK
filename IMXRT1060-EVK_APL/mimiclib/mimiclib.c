/**
 * @file mimiclib.c
 * @brief mimiclib is insteadof stdio.h, stdlib.h and string.h
 * @author Takashi Kashiwagi
 * @date 2018/7/5
 * @version     0.2
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
 * - 2018/07/05: Takashi Kashiwagi: v0.1
 * - 2018/10/28: Takashi Kashiwagi: v0.2 for IMXRT1060-EVK
 */
#include "mimiclib/mimiclib.h"
#include <stdarg.h>


#ifndef WIN_TEST
#define DefBSP_IMXRT1060_EVK
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void mimic_printf(const char* fmt, ...){
	va_list arg;
	char szBuffer[1024];

	va_start(arg, fmt);
	vsnprintf(szBuffer, sizeof(szBuffer), fmt, arg);
	va_end(arg);

	puts(szBuffer);
}
uint32_t mimic_gets(char pszStr[], uint32_t u32Size){
	uint32_t ret = 0;
	if (fgets(a, 20, stdin) != NULL) {
		ret = strlen(pszStr);
	}
	return ret;
}
_Bool mimic_kbhit(void){
	return kbhit();
}
#endif


#ifdef DefBSP_IMXRT1060_EVK
/**
 * @brief gets
 * @param [out] pszStr input buffer
 * @param [in]  u32Size Array size of pszStr
 * @return u32Cnt recieved count
 */
uint32_t mimic_gets(char pszStr[], uint32_t u32Size){
	uint32_t u32Cnt = 0u;

	if((pszStr != NULL) && (u32Size > 0u)){
		_Bool bReturnCode = false;

		mimic_memset(pszStr, 0, u32Size);
		
		while(bReturnCode == false){
			char ch;
			RTOS_GetChar(&ch);
			
			
			switch(ch){
			case '\b':	// バックスペース
				if(u32Cnt > 0u){
					u32Cnt--;
					pszStr[u32Cnt] = '\0';
					RTOS_PutChar('\b');
					RTOS_PutChar(' ');
					RTOS_PutChar('\b');
				}
				break;
			case '\r':		// TeraTermの改行コードは "CR"設定ではCRのみ送られてくる（CRLFにならない）
				//u32Cnt--;			
				pszStr[u32Cnt] = '\0';
				bReturnCode = true;
				RTOS_PutChar((char)ch);
				RTOS_PutChar('\n');		// 相手はWindowsと仮定してLRも送信する
				break;
			default:
				pszStr[u32Cnt] = ch;
				u32Cnt++;
				if(u32Cnt >= u32Size){
					u32Cnt--;
					pszStr[u32Cnt] = '\0';
					break;
				}
				RTOS_PutChar((char)ch);
				break;
			}
		}
	}

	return u32Cnt;
}

/**
 * @brief printf
 * @param [in]  format
 * @return void
 */
void mimic_printf(const char *format, ...){
	va_list arg;
	char szStr[512];

    va_start(arg, format);
    vsnprintf(szStr, sizeof(szStr), format, arg);
    va_end(arg);

	RTOS_PutString(szStr);
}

/**
 * @brief printf
 * @return true There are some characters in Buffer
 * @return false There are no characters in Buffer
 */
_Bool mimic_kbhit(void){
	return RTOS_kbhit();
}
#endif

/**
 * Retargeting printf/scanf(https://community.nxp.com/thread/389140)
 */
#ifdef __NEW_LIB__
/** 
 * @breif To retarget printf(), you will need to provide your own implementation of the Newlib system function _write():
 * @return Function returns number of unwritten bytes if error, otherwise 0 for success
 */
int _write(int iFileHandle, char *pcBuffer, int iLength){
	RTOS_PutString(pcBuffer);
	return 0;
}
/** 
 * @breif To retarget scanf, you will need to provide your own implementation of the Newlib system function _read():
 * @return Function returns number of characters read, stored in pcBuffer
 */
int _read(int iFileHandle, char *pcBuffer, int iLength){
	return mimic_gets(pcBuffer, iLength);
}

#endif

