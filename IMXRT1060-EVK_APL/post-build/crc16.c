


#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>

uint16_t SlowCrc16(uint16_t sum, uint8_t pu8[], uint32_t len)
{
	for(uint32_t j = 0;j<len;j++)
	{
		uint8_t byte = pu8[j];
		for (uint32_t i = 0; i < 8; ++i)
		{
			uint32_t oSum = sum;
			sum <<= 1;
			if (byte & 0x80){
				sum |= 1;
			}
			if (oSum & 0x8000){
				sum ^= 0x1021;
			}
			byte <<= 1;
		}
	}
	return sum;
}

typedef enum{
	enInputFileName = 0,
	enOutputFileName,
	enSeedSum,
	enUnkownOption,
}enOption_t;

enOption_t AnalyzeOption(char szOption[]){
	if(strncmp(szOption, "-i", 3) == 0){
		return enInputFileName;
	}
	if(strncmp(szOption, "-o", 3) == 0){
		return enOutputFileName;
	}
	if(strncmp(szOption, "-s", 3) == 0){
		return enSeedSum;
	}
	return enUnkownOption;
}

int main(int argc, char *argv[]){
	
	char *szInputFile = NULL;
	char *szOutputFile = NULL;
	uint16_t u16SeedSum = 0;
	uint32_t i=0;
	uint8_t *pu8Input=NULL;
	uint8_t *pu8Output=NULL;
	uint32_t u32InputSize;

	/** AnalyzeOption */
	i=1;
	printf("argc  = %d\n", argc);
	while(i < argc){
		switch(AnalyzeOption(argv[i])){
		case enInputFileName:
			i++;
			szInputFile = argv[i];
			break;
		case enOutputFileName:
			i++;
			szOutputFile = argv[i];
			break;
		case enSeedSum:
			i++;
			u16SeedSum = strtoul(argv[i], NULL, 16);
			break;
		default :
			printf("[%s (%d)] Unowkn option <%s>\n", __FUNCTION__, __LINE__, argv[i]);
			goto _END;
		}
		i++;
	}

	/** Infomation */

	{
		FILE *infp = NULL;
		struct stat statBuf;
		if (stat(szInputFile, &statBuf) == 0){
			u32InputSize = statBuf.st_size;
		}else{
			goto _END;
		}
		

		infp = fopen(szInputFile, "rb");
		if (infp == NULL) {
			printf("[%s (%d)] TP\n", __FUNCTION__, __LINE__);
			goto _END;
		}

		pu8Input = (uint8_t*)malloc(u32InputSize + 4);
		if (pu8Input == NULL) {
			printf("[%s (%d)] TP\n", __FUNCTION__, __LINE__);
			goto _END;
		}
		pu8Output = (uint8_t*)malloc(u32InputSize + 4 + 2);
		if (pu8Output == NULL) {
			printf("[%s (%d)] TP\n", __FUNCTION__, __LINE__);
			goto _END;
		}
		memset(pu8Input, 0, u32InputSize + 4);
		memset(pu8Output, 0, u32InputSize + 4);

		if(0 == fread(pu8Input, u32InputSize, 1, infp)){
			printf("[%s (%d)] TP\n", __FUNCTION__, __LINE__);
			fclose(infp);
			goto _END;
		}
		fclose(infp);
	}
	{
		FILE *outfp = NULL;
		uint16_t u16crc16;

		outfp = fopen(szOutputFile, "wb");
		if (outfp == NULL) {
			printf("[%s (%d)] TP\n", __FUNCTION__, __LINE__);
			goto _END;
		}
		memcpy(pu8Output, &u32InputSize, sizeof(uint32_t));
		memcpy(&pu8Output[4], pu8Input, u32InputSize);

		u16crc16 = SlowCrc16(u16SeedSum, pu8Output, u32InputSize + 4);

		memcpy(&pu8Output[u32InputSize + 4], &u16crc16, sizeof(uint16_t));
		
		if(0 == fwrite(pu8Output, u32InputSize + 4 + 2, 1, outfp)){
			printf("[%s (%d)] TP\n", __FUNCTION__, __LINE__);
			fclose(outfp);
			goto _END;
		}
		fclose(outfp);

		printf("[%s (%4d)] szInputFile    = <%s>\n", __FUNCTION__, __LINE__, szInputFile);
		printf("[%s (%4d)] szOutputFile   = <%s>\n", __FUNCTION__, __LINE__, szOutputFile);
		printf("[%s (%4d)] u16SeedSum     = 0x%04X\n", __FUNCTION__, __LINE__, u16SeedSum);
		printf("[%s (%4d)] u32InputSize   = %lu\n", __FUNCTION__, __LINE__, u32InputSize);
		printf("[%s (%4d)] u16crc16       = 0x%04X\n", __FUNCTION__, __LINE__, u16crc16);

		printf("[%s (%4d)] OUTPUT OK        <%s>\n", __FUNCTION__, __LINE__, szOutputFile);
	}
	
_END:
	free(pu8Input);
	free(pu8Output);
	return 0;

}

