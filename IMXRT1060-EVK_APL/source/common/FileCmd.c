/**
 * @file FileCmd.h
 * @brief TODO
 * @author Takashi Kashiwagi
 * @date 2018/10/28
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
 * - 2018/10/28: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */
#include "common/FileCmd.h"

#include "ff.h"
#include "mimiclib/mimiclib.h"

void FileCmd_ls(uint32_t argc, const char *argv[]){
	DIR stDir;
    const TCHAR *pszDir;
	DWORD free;
	FATFS *pfatfs;

	if(argc < 2){
		pszDir = u8".";
	}else{
		pszDir = argv[1];
	}

	if(FR_OK == f_opendir(&stDir, pszDir)){
		FILINFO stFInfo;
		while(FR_OK == f_readdir(&stDir, &stFInfo)){
			if(stFInfo.fname[0] == '\0'){
				break;
			}
			mimic_printf("%-12llu[byte] %-40s\r\n", stFInfo.fsize, stFInfo.fname);
		}
		f_closedir(&stDir);
	}else{
		mimic_printf("f_opendir NG <%s>\r\n", pszDir);
	}
	{	/** TODO */
		f_getfree(u8"A:/", &free, &pfatfs);
	}
	mimic_printf("%c:  free = %llu\r\n", 'A' + pfatfs->pdrv, (uint64_t)free*(uint64_t)pfatfs->csize*512u);
}

void FileCmd_mkdir(uint32_t argc, const char *argv[]){
	if(argc == 2){
		if(FR_OK == f_mkdir(argv[1])){
			mimic_printf("mkdir OK : <%s>\r\n", argv[1]);
		}else{
			mimic_printf("mkdir NG : <%s>\r\n", argv[1]);
		}
	}else{
		mimic_printf("[usage] mkdir <Directory Path>\r\n");
	}
}
void FileCmd_rm(uint32_t argc, const char *argv[]){
	if(argc == 2){
		if(FR_OK == f_unlink(argv[1])){
			mimic_printf("rmdir OK : <%s>\r\n", argv[1]);
		}else{
			mimic_printf("rmdir NG : <%s>\r\n", argv[1]);
		}
	}else{
		mimic_printf("[usage] rmdir <Directory Path>\r\n");
	}
}

void FileCmd_cat(uint32_t argc, const char *argv[]){
	if(argc == 2){
		FIL stFile;

		if(FR_OK == f_open(&stFile, argv[1], FA_READ)){
			TCHAR szStr[512];
			
			mimic_printf("f_open OK : <%s>\r\n", argv[1]);
			while(f_gets(szStr, sizeof(szStr), &stFile) != NULL){
				mimic_printf("%s", szStr);
			}
			f_close(&stFile);
		}else{
			mimic_printf("f_open NG : <%s>\r\n", argv[1]);
		}
	}else{
		mimic_printf("[usage] cat <File Path>\r\n");
	}
}

void FileCmd_cd(uint32_t argc, const char *argv[]){
	if(argc == 2){
		if(FR_OK == f_chdir(argv[1])){
			mimic_printf("cd OK : <%s>\r\n", argv[1]);
		}else{
			mimic_printf("cd NG : <%s>\r\n", argv[1]);
		}
	}else{
		if(FR_OK == f_chdir("A:/")){
			mimic_printf("cd OK : <%s>\r\n", "A:/");
		}else{
			mimic_printf("cd NG : <%s>\r\n", "A:/");
		}
	}
}

