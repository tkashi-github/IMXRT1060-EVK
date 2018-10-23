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
#include "mimiclib/mimiclib.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

		memset(pszStr, 0, u32Size);
		
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

