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
#include "common/FileCmd.h"

#if 0
#include "ff.h"

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
			if(strcmp(stFInfo.fname, ".") && 
				strcmp(stFInfo.fname, "..")){
				mimiclib_printf("%-12llu[byte] %-40s\r\n",
					stFInfo.fsize, stFInfo.fname);
			}
			vTaskDelay(20);
		}
		f_closedir(&stDir);
	}
	{	/** TODO */
		f_getfree(u8"A:/", &free, &pfatfs);
	}
	mimiclib_printf("%c:  free = %llu\r\n", 'A' + pfatfs->pdrv, (uint64_t)free*(uint64_t)pfatfs->csize*512u);
}

void FileCmd_mkdir(uint32_t argc, const char *argv[]){
	if(argc == 2){
		if(FR_OK == f_mkdir(argv[1])){
			mimiclib_printf("mkdir OK : <%s>\r\n", argv[1]);
		}else{
			mimiclib_printf("mkdir NG : <%s>\r\n", argv[1]);
		}
	}else{
		mimiclib_printf("[usage] mkdir <Directory Path>\r\n");
	}
}
void FileCmd_rm(uint32_t argc, const char *argv[]){
	if(argc == 2){
		if(FR_OK == f_unlink(argv[1])){
			mimiclib_printf("rmdir OK : <%s>\r\n", argv[1]);
		}else{
			mimiclib_printf("rmdir NG : <%s>\r\n", argv[1]);
		}
	}else{
		mimiclib_printf("[usage] rmdir <Directory Path>\r\n");
	}
}

void FileCmd_cat(uint32_t argc, const char *argv[]){
	if(argc == 2){
		FIL stFile;

		if(FR_OK == f_open(&stFile, argv[1], FA_READ)){
			TCHAR szStr[512];
			
			mimiclib_printf("f_open OK : <%s>\r\n", argv[1]);
			while(f_gets(szStr, sizeof(szStr), &stFile) != NULL){
				mimiclib_printf("%s", szStr);
			}
			f_close(&stFile);
		}else{
			mimiclib_printf("f_open NG : <%s>\r\n", argv[1]);
		}
	}else{
		mimiclib_printf("[usage] cat <File Path>\r\n");
	}
}

void FileCmd_cd(uint32_t argc, const char *argv[]){
	if(argc == 2){
		if(FR_OK == f_chdir(argv[1])){
			mimiclib_printf("cd OK : <%s>\r\n", argv[1]);
		}else{
			mimiclib_printf("cd NG : <%s>\r\n", argv[1]);
		}
	}else{
		if(FR_OK == f_chdir("A:/")){
			mimiclib_printf("cd OK : <%s>\r\n", "A:/");
		}else{
			mimiclib_printf("cd NG : <%s>\r\n", "A:/");
		}
	}
}
#endif

