/**
 * @file StorageBenchMark.c
 * @brief TODO
 * @author Takashi Kashiwagi
 * @date 2018/11/02
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
 * - 2018/11/02: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */
#include "common/StorageBenchMark.h"
#include "board.h"
#include "mimiclib/mimiclib.h"
#include "ff.h"

#include <stdio.h>
#include <stdlib.h>

#define DefTEST_FILE_SIZE	(10*1024u*1024U)
#define DefTEST_FILE_NUM	(10u)


void CmdStorageBenchMark(uint32_t argc, const char *argv[]){
	uint32_t au32BlockSize[] = {8192, 16384, 32768, 65536};
	uint32_t au32Ptn[] = {0x00000000, 0x55555555, 0xAAAAAAAA, 0xFFFFFFFF};
	FIL stFIL;
	uint32_t *pu32WriteBuffer = NULL;
	uint32_t *pu32ReadBuffer = NULL;
	
	mimic_printf("\r\nStorageBenchMark\r\n");

	pu32WriteBuffer = (uint32_t*)pvPortMalloc(65536);
	if(pu32WriteBuffer == NULL){
		mimic_printf("[%s (%d)] pu32WriteBuffer NG\r\n", __FUNCTION__, __LINE__);		
		goto _END;
	}

	pu32ReadBuffer = (uint32_t*)pvPortMalloc(65536);
	if(pu32ReadBuffer == NULL){
		mimic_printf("[%s (%d)] pu32WriteBuffer NG\r\n", __FUNCTION__, __LINE__);		
		goto _END;
	}

	for(uint32_t i=0;i<(sizeof(au32BlockSize)/sizeof(au32BlockSize[0]));i++){
		uint32_t u32BlockSize = au32BlockSize[i];
		uint32_t u32LoopCnt = DefTEST_FILE_SIZE / u32BlockSize;
		uint32_t u32PtnCnt = sizeof(au32Ptn)/sizeof(au32Ptn[0]);
		double dfpWriteTime = 0;
		double dfpReadTime = 0;
		double dfpWriteMBperSec = (double)DefTEST_FILE_SIZE;
		double dfpReadMBperSec = (double)DefTEST_FILE_SIZE;
		uint32_t ProgressCnt = 0;
		char Progress[] = {'\\', '|', '/', '-'};

		//mimic_printf("[%s (%d)] u32BlockSize = %lu\r\n", __FUNCTION__, __LINE__, u32BlockSize);	
		for(uint32_t j=0;j<u32PtnCnt;j++){
			uint32_t u32Ptn = au32Ptn[j];
				
			//mimic_printf("[%s (%d)] u32Ptn = 0x%08lX\r\n", __FUNCTION__, __LINE__, u32Ptn);	

			/** Make Test Ptn */
			for(uint32_t k=0;k<(u32BlockSize/sizeof(uint32_t));k++){
				pu32WriteBuffer[k] = u32Ptn;
			}


			for(uint32_t k=0;k<DefTEST_FILE_NUM;k++){
				TCHAR szFilePath[256];
				FRESULT fres;
				uint32_t StartTick;
				uint32_t WriteTick;
				uint32_t ReadTick;

				mimic_printf("\r%c", Progress[ProgressCnt]);
				ProgressCnt++;
				ProgressCnt %= 4;

				sprintf(szFilePath, "A:/%03lu.dat", k);
				//mimic_printf("[%s (%d)] fopen = <%s>\r\n", __FUNCTION__, __LINE__, szFilePath);
				fres = f_open(&stFIL, szFilePath, FA_WRITE | FA_CREATE_ALWAYS);
				if(FR_OK != fres){
					mimic_printf("[%s (%d)] f_open NG (%d)\r\n", __FUNCTION__, __LINE__, fres);
					f_close(&stFIL);
					goto _END;
				}


				//mimic_printf("[%s (%d)] f_write\r\n", __FUNCTION__, __LINE__);
				StartTick = osKernelGetTickCount();

				for(uint32_t l=0;l<u32LoopCnt;l++){
					uint32_t bw;

					fres = f_write(&stFIL, pu32WriteBuffer, u32BlockSize, (UINT*)&bw);
					if(FR_OK != fres){
						mimic_printf("[%s (%d)] f_write NG (%d)\r\n", __FUNCTION__, __LINE__, fres);
						f_close(&stFIL);
						goto _END;
					}
				}
				WriteTick = osKernelGetTickCount() - StartTick;
				//mimic_printf("[%s (%d)] f_write time = <%lu msec>\r\n", __FUNCTION__, __LINE__, WriteTick);
				f_close(&stFIL);
				dfpWriteTime += (double)WriteTick;

				fres = f_open(&stFIL, szFilePath, FA_READ | FA_OPEN_EXISTING);
				if(FR_OK != fres){
					mimic_printf("[%s (%d)] f_open NG (%d)\r\n", __FUNCTION__, __LINE__, fres);
					f_close(&stFIL);
					goto _END;
				}

				//mimic_printf("[%s (%d)] f_read\r\n", __FUNCTION__, __LINE__);
				
				

				for(uint32_t l=0;l<u32LoopCnt;l++){
					uint32_t br;

					StartTick = osKernelGetTickCount();
					fres = f_read(&stFIL, pu32ReadBuffer, u32BlockSize, (UINT*)&br);
					if(FR_OK != fres){
						mimic_printf("[%s (%d)] f_write NG (%d)\r\n", __FUNCTION__, __LINE__, fres);
						f_close(&stFIL);
						goto _END;
					}
					ReadTick = osKernelGetTickCount() - StartTick;
					dfpReadTime += (double)ReadTick;
					if(0!= memcmp(pu32WriteBuffer, pu32ReadBuffer, u32BlockSize))
					{
						mimic_printf("memcmp NG\r\n" );
						goto _END;
					}
				}

				f_close(&stFIL);

				f_unlink(szFilePath);
			}
		}

		dfpWriteTime /= (u32PtnCnt * DefTEST_FILE_NUM);
		dfpReadTime /= (u32PtnCnt * DefTEST_FILE_NUM);

		dfpWriteMBperSec /= dfpWriteTime;
		dfpWriteMBperSec *= 1000.0;
		dfpWriteMBperSec /= 1024.0;
		dfpWriteMBperSec /= 1024.0;

		dfpReadMBperSec /= dfpReadTime;
		dfpReadMBperSec *= 1000.0;
		dfpReadMBperSec /= 1024.0;
		dfpReadMBperSec /= 1024.0;
		mimic_printf("\rBS = %lu [BYTE], WP=%f [MB/S], RP=%f [MB/S]\r\n", u32BlockSize, dfpWriteMBperSec, dfpReadMBperSec);	
	}

_END:
	if(pu32WriteBuffer != NULL){
		vPortFree(pu32WriteBuffer);
	}
	if(pu32ReadBuffer != NULL){
		vPortFree(pu32ReadBuffer);
	}
}

