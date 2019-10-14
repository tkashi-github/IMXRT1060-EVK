/**
 * @file compat.c
 * @brief portintg layer
 * @author Takashi Kashiwagi
 * @date 2019/10/14
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
 * - 2019/10/14: Takashi Kashiwagi
 */
#include "flac/inc/share/compat.h"

#ifdef FLAC_HEAP_DEBUG
#include "fsl_common.h"
typedef struct{
	uintptr_t addr;
	uint32_t u32size;
	char *psz;
	uint32_t u32Line;
}stMallocDebug_t;

static stMallocDebug_t s_stMallocDbgTable[1000] = {0};


void AddMallocInfo(uintptr_t addr, uint32_t u32size, char *psz, uint32_t u32Line){
	uint32_t primask = DisableGlobalIRQ();
	for(uint32_t i=0;i<1000;i++){
		if(s_stMallocDbgTable[i].addr == 0){
			s_stMallocDbgTable[i].addr = addr;
			s_stMallocDbgTable[i].u32size = u32size;
			s_stMallocDbgTable[i].psz = psz;
			s_stMallocDbgTable[i].u32Line = u32Line;
			break;
		}
	}
	EnableGlobalIRQ(primask);
}
void DelMallocInfo(uintptr_t addr){
	uint32_t primask = DisableGlobalIRQ();
	for(uint32_t i=0;i<1000;i++){
		if(s_stMallocDbgTable[i].addr == addr){
			s_stMallocDbgTable[i].addr = 0;
			break;
		}
	}
	EnableGlobalIRQ(primask);
}
void DumpMallocInfo(void){
	flac_printf("[%s (%d)] ENTER\r\n", __FUNCTION__, __LINE__);
	for(uint32_t i=0;i<1000;i++){
		if(s_stMallocDbgTable[i].addr != 0){
			flac_printf("DBG :: [%s (%d)] ptr = 0x%08lX, size = %lu\r\n", s_stMallocDbgTable[i].psz, s_stMallocDbgTable[i].u32Line, s_stMallocDbgTable[i].addr, s_stMallocDbgTable[i].u32size);
		}
	}
	flac_printf("[%s (%d)] EXIT\r\n", __FUNCTION__, __LINE__);
}
#endif

int flac_chmod(const char szFilePath[], int mode){
	if(szFilePath == NULL){
		return -1;
	}
	/** 多分 flac_fstatとつじつまが合ってればよい? */
	f_chmod(szFilePath, (BYTE)mode, (AM_RDO | AM_ARC | AM_SYS | AM_HID));
	return 0;
}

UINT flac_fwrite(const void *buf, size_t size, size_t n, FLAC_FILE *fp){
	UINT bw;

	if(FR_OK != f_write(fp, buf, size*n, &bw)){
		return 0;
	}

	return bw;
}

UINT flac_fread(void *buf, size_t size, size_t n, FLAC_FILE *fp){
	UINT br;
	FRESULT res = f_read(fp, buf, size*n, &br);
	if(FR_OK != res){
		return 0;
	}

	return br;
}

int flac_fseeko(FLAC_FILE *fp, int32_t offset, int32_t whence){
	int64_t i64pos;

	if(fp == NULL){
		return -1;
	}
	if((whence != SEEK_SET) && (whence != SEEK_CUR)){
		return -1;
	}
	if(whence == SEEK_SET){
		i64pos = 0;
	}else{
		i64pos = f_tell(fp);
	}
	if(i64pos >= offset){
		i64pos -= offset;
	}else{
		i64pos = 0;
	}

	if(FR_OK != f_lseek(fp, i64pos)){
		return -1;
	}

	return 0;
}
