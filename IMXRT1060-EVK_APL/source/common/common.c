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

#include "common/common.h"
#include "board.h"
#include "mimiclib/mimiclib.h"

void MemDump(uintptr_t addr, size_t ByteCnt)
{
	/** var */
	size_t i;
	size_t LoopTimes;
	uint8_t *pu8 = (uint8_t *)addr;
	LoopTimes = ByteCnt / 16u;

	if ((LoopTimes * 16u) != ByteCnt)
	{
		LoopTimes++;
	}
	for (i = 0; i < LoopTimes; i++)
	{
		size_t index = i * 16;
		size_t j;
		for (j = 0; j < 16; j++)
		{
			mimic_printf("%02X ", pu8[index + j]);
		}

		vTaskDelay(10);
		mimic_printf(" : ");
		for (j = 0; j < 16; j++)
		{
			mimic_printf("%c", mimic_isprint(pu8[index + j]) ? pu8[index + j] : '.');
		}
		mimic_printf("\r\n");
		vTaskDelay(10);
	}
	mimic_printf("-------------------------------------------------------------------\r\n");
}

#if 0
#include "ff.h"

TCHAR *GetFileExt(TCHAR szExt[], const TCHAR szFilePath[], size_t SizeExt)
{
	size_t slen;
	size_t i;
	TCHAR *pret = szExt;
	szExt[0] = (TCHAR)'\0';
	slen = mimic_tcslen(szFilePath);
	if (slen > 0)
	{
		i = slen - 1;
		for (;;)
		{
			if (szFilePath[i] == (TCHAR)'.')
			{
				mimic_tcscpy(szExt, &szFilePath[i + 1], SizeExt);
				break;
			}
			if (i == 0)
			{
				break;
			}
			else
			{
				i--;
			}
		}
	}

	return pret;
}

#endif
