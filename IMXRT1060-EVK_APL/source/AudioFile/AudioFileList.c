/**
 * @file		AudioFileList.c
 * @brief		TOOD
 * @date		2019/06/13
 * @version     0.1
 * @details 
 * --
 * License Type <MIT License>
 * --
 * Copyright 2019 Takashi Kashiwagi
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
 * - 2019/06/13: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */
#include "AudioFile/AudioFileList.h"
#include "mimiclib/mimiclib.h"
#include "ff.h"
#include "common/common.h"

static TCHAR s_szAudioFileListAll[kAudioFileMax][kFilePathLenMax];
static TCHAR s_szAudioFileListDir[kAudioFileMax][kFilePathLenMax];

static uint32_t s_u32AudioFileNumAll = 0;
static uint32_t s_u32AudioFileNumDir = 0;

static _Bool IsAudioFile(const TCHAR szFileName[])
{
	TCHAR sztemp[5] = {0};
	GetFileExt(sztemp, szFileName, 5);

	mimic_tcsupper(sztemp);
	//mimic_printf("%s (%s)\r\n", szFileName, sztemp);
	if (memcmp(sztemp, _T("WAV"), 3 * sizeof(TCHAR)) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

static void rMakeAudioFileListALL(const TCHAR szDirPath[])
{
	DIR stDir;
	TCHAR szTmp[kFilePathLenMax];

	if (FR_OK == f_opendir(&stDir, szDirPath))
	{
		FILINFO stFInfo;
		while (FR_OK == f_readdir(&stDir, &stFInfo))
		{
			if (stFInfo.fname[0] == '\0')
			{
				break;
			}
			if (s_u32AudioFileNumAll >= kFilePathLenMax)
			{
				break;
			}
			if (stFInfo.fattrib & (AM_HID | AM_SYS))
			{
				continue;
			}

			if (strcmp(stFInfo.fname, ".") && strcmp(stFInfo.fname, ".."))
			{
				if (stFInfo.fattrib & AM_DIR)
				{
					/** !!! Recursive Call */
					mimic_sprintf(szTmp, kFilePathLenMax, "%s/%s", szDirPath, stFInfo.fname);
					rMakeAudioFileListALL(szTmp);
				}
				else
				{
					if (IsAudioFile(stFInfo.fname) != false)
					{
						mimic_sprintf(szTmp, kFilePathLenMax, "%s/%s", szDirPath, stFInfo.fname);
						strncpy(s_szAudioFileListAll[s_u32AudioFileNumAll], szTmp, kFilePathLenMax * sizeof(TCHAR));
						s_u32AudioFileNumAll++;
					}
				}
			}
		}
		f_closedir(&stDir);
	}
}

/** 後でJISに合わせよう */
static int tcharcmp(const void *a, const void *b)
{
	uint32_t i = 0;
	TCHAR *p1 = (TCHAR *)a;
	TCHAR *p2 = (TCHAR *)b;

	while (p1[i] == p2[i])
	{
		if (p1[i] == (TCHAR)'\0')
		{
			break;
		}
		i++;
	}

	if (p1[i] < p2[i])
	{
		return -1;
	}
	else if (p1[i] == p2[i])
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

uint32_t MakeAudioFileListALL(void)
{
	s_u32AudioFileNumAll = 0;
	memset(s_szAudioFileListAll, 0, sizeof(s_szAudioFileListAll));
	rMakeAudioFileListALL("A:");

	qsort(s_szAudioFileListAll, s_u32AudioFileNumAll, sizeof(TCHAR) * kFilePathLenMax, tcharcmp);
	return s_u32AudioFileNumAll;
}
uint32_t MakeAudioFileListCurrentDir(void)
{
	DIR stDir;

	s_u32AudioFileNumDir = 0;
	memset(s_szAudioFileListDir, 0, sizeof(s_szAudioFileListDir));

	if (FR_OK == f_opendir(&stDir, "."))
	{
		FILINFO stFInfo;
		while (FR_OK == f_readdir(&stDir, &stFInfo))
		{
			if (stFInfo.fname[0] == '\0')
			{
				break;
			}
			if (s_u32AudioFileNumDir >= kFilePathLenMax)
			{
				break;
			}
			if (stFInfo.fattrib & (AM_HID | AM_SYS))
			{
				continue;
			}
			if (strcmp(stFInfo.fname, ".") && strcmp(stFInfo.fname, ".."))
			{
				if (IsAudioFile(stFInfo.fname) != false)
				{
					strncpy(s_szAudioFileListDir[s_u32AudioFileNumDir], stFInfo.fname, kFilePathLenMax * sizeof(TCHAR));
					s_u32AudioFileNumDir++;
				}
			}
		}
		f_closedir(&stDir);
	}

	qsort(s_szAudioFileListDir, s_u32AudioFileNumDir, sizeof(TCHAR) * kFilePathLenMax, tcharcmp);
	return s_u32AudioFileNumDir;
}
uint32_t GetAudioFileNumALL(void)
{
	return s_u32AudioFileNumAll;
}
uint32_t GetAudioFileNumCurrentDir(void)
{
	return s_u32AudioFileNumDir;
}

_Bool GetAudioFilePathALL(uint32_t u32TrackNo, TCHAR szFilePath[])
{
	if (u32TrackNo < s_u32AudioFileNumAll)
	{
		mimic_tcscpy(szFilePath, s_szAudioFileListAll[u32TrackNo], kFilePathLenMax);
		return true;
	}
	else
	{
		return false;
	}
}
_Bool GetAudioFilePathCurrentDir(uint32_t u32TrackNo, TCHAR szFilePath[])
{
	if (u32TrackNo < s_u32AudioFileNumDir)
	{
		mimic_tcscpy(szFilePath, s_szAudioFileListDir[u32TrackNo], kFilePathLenMax);
		return true;
	}
	else
	{
		return false;
	}
}

void DumpAudioFileListALL(void)
{
	mimic_printf("s_u32AudioFileNumAll = %d\r\n", s_u32AudioFileNumAll);
	for (uint32_t i = 0; i < s_u32AudioFileNumAll; i++)
	{
		mimic_printf("%03d : <%s>\r\n", i, s_szAudioFileListAll[i]);
	}
	mimic_printf("End\r\n\r\n");
}
void DumpAudioFileListCurrentDir(void)
{
	mimic_printf("s_u32AudioFileNumDir = %d\r\n", s_u32AudioFileNumDir);
	for (uint32_t i = 0; i < s_u32AudioFileNumDir; i++)
	{
		mimic_printf("%03d : <%s>\r\n", i, s_szAudioFileListDir[i]);
	}
	mimic_printf("End\r\n\r\n");
}

void CmdMakeAudioFileList(uint32_t argc, const char *argv[])
{
	MakeAudioFileListALL();
	DumpAudioFileListALL();
	MakeAudioFileListCurrentDir();
	DumpAudioFileListCurrentDir();
}

enAudioFileType_t GetAudioFileType(const TCHAR szFileName[])
{
	TCHAR sztemp[5] = {0};
	GetFileExt(sztemp, szFileName, 5);

	mimic_tcsupper(sztemp);
	//mimic_printf("%s (%s)\r\n", szFileName, sztemp);
	if (memcmp(sztemp, _T("WAV"), 3 * sizeof(TCHAR)) == 0)
	{
		return enAudioFileWAV;
	}
	else
	{
		return enAudioFileMAX;
	}
}
