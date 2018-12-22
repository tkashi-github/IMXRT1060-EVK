#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>

static char s_szUserBuildCount[1000][256] = {0};

static uint32_t GetBuildCountFromDefine(const char szStr[])
{
	char szBuf[256] = {0};
	strncpy(szBuf, szStr, 255);

	strtok(szBuf, " ");
	strtok(NULL, " ");
	return atol(strtok(NULL, " "));
}
static void LoaduserBuildCount(const char szBuildLogPath[])
{
	FILE *pBuildLogFile = NULL;
	uint32_t u32Cnt = 0;
	memset(s_szUserBuildCount, 0, sizeof(s_szUserBuildCount));

	pBuildLogFile = fopen(szBuildLogPath, "r");
	if (pBuildLogFile != NULL)
	{
		char szBuf[256];

		while (fgets(szBuf, sizeof(szBuf), pBuildLogFile) != NULL)
		{
			if (strstr(szBuf, "#define DEF_USER_BUILDCOUNT_") != NULL)
			{
				if (szBuf[strlen(szBuf) - 1] == '\n')
				{
					szBuf[strlen(szBuf) - 1] = '\0';
				}
				strncpy(s_szUserBuildCount[u32Cnt], szBuf, 255);
				u32Cnt++;
				printf("[%s (%d)] <%s> %lu\n", __FUNCTION__, __LINE__, szBuf, GetBuildCountFromDefine(szBuf));
			}
		}
		fclose(pBuildLogFile);
	}
}

static void UpdateBuildLog(const char szBuildLogPath[])
{
	FILE *pBuildLogFile = NULL;

	printf("[%s (%d)] ENTER\n", __FUNCTION__, __LINE__);

	pBuildLogFile = fopen(szBuildLogPath, "w");
	if (pBuildLogFile != NULL)
	{
		printf("[%s (%d)] ENTER\n", __FUNCTION__, __LINE__);

		uint32_t i = 0;
		uint32_t u32BuildCount = 0;

		while (s_szUserBuildCount[i][0] != '\0')
		{
			fprintf(pBuildLogFile, "%s\n", s_szUserBuildCount[i]);
			fprintf(stdout, "%s\n", s_szUserBuildCount[i]);
			u32BuildCount += GetBuildCountFromDefine(s_szUserBuildCount[i]);
			i++;
		}
		{
			char szAsciiTime[128];
			time_t timer = time(NULL);
			struct tm *date = localtime(&timer);
			strcpy(szAsciiTime, asctime(date));
			szAsciiTime[strlen(szAsciiTime) - 1] = '\0';
			fprintf(pBuildLogFile, "#define DEF_PREBUILD_TIME \"%s\"\n", szAsciiTime);
			fprintf(stdout, "#define DEF_PREBUILD_TIME \"%s\"\n", szAsciiTime);
		}
		{
			char szBuf[256];
			fprintf(pBuildLogFile, "#define DEF_TOTAL_BUILD_COUNT \"Build%s\"\n\n", itoa(u32BuildCount, szBuf, 10));
			fprintf(stdout, "#define DEF_BUILD_COUNT \"%s\"\n\n", itoa(u32BuildCount, szBuf, 10));
		}

		fclose(pBuildLogFile);
	}
	printf("[%s (%d)] EXIT\n", __FUNCTION__, __LINE__);
}

int main(int argc, char *argv[])
{
	if (argc > 1)
	{
		printf("[%s (%d)] TP\n", __FUNCTION__, __LINE__);
		LoaduserBuildCount(argv[1]);
		uint32_t i;

		if (argc > 2)
		{
			char szStr[256];
			_Bool bHit = false;

			printf("[%s (%d)] TP\n", __FUNCTION__, __LINE__);
			snprintf(szStr, 255, "#define DEF_USER_BUILDCOUNT_%s ", argv[2]);
			printf("[%s (%d)] <%s>\n", __FUNCTION__, __LINE__, szStr);
			
			uint32_t i;
			for ( i = 0; i < 1000; i++)
			{
				if (strstr(s_szUserBuildCount[i], szStr) != NULL)
				{
					printf("[%s (%d)] TP\n", __FUNCTION__, __LINE__);
					uint32_t u32;
					u32 = GetBuildCountFromDefine(s_szUserBuildCount[i]);
					u32++;
					snprintf(s_szUserBuildCount[i], 255, "%s%lu", szStr, u32);
					printf("[%s (%d)] UPdate <%s>\n", __FUNCTION__, __LINE__, s_szUserBuildCount[i]);
					bHit = true;
					break;
				}
				if(s_szUserBuildCount[i][0] == '\0'){
					break;
				}
			}

			printf("[%s (%d)] TP\n", __FUNCTION__, __LINE__);

			if (bHit == false)
			{
				printf("[%s (%d)] TP\n", __FUNCTION__, __LINE__);
				snprintf(s_szUserBuildCount[i], 255, "%s1", szStr);
				printf("[%s (%d)] <%s>\n", __FUNCTION__, __LINE__, s_szUserBuildCount[i]);
			}

			printf("[%s (%d)] TP\n", __FUNCTION__, __LINE__);
			UpdateBuildLog(argv[1]);
			printf("[%s (%d)] TP\n", __FUNCTION__, __LINE__);
		}
	}
	return 0;
}