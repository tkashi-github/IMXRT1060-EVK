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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "Task/ConsoleTask/ConsoleTask.h"
#include "version.h"
#include "ConsoleCmd.h"

#include "mimiclib/mimiclib.h"
#include "UART/DrvLPUART.h"

#define DEF_PROMPT "IMXRT1060-EVK BOOT > "

/**
 * @brief ConsoleTask
 * @param [in]  argument nouse
 * @return void
 */
void ConsoleTask(void const *argument){
	/*-- var --*/
	char *szBuf;
	lpuart_config_t lpuart_config;

	/*-- begin --*/
	szBuf = pvPortMalloc(2048);
	LPUART_GetDefaultConfig(&lpuart_config);
	lpuart_config.baudRate_Bps = 115200;
	DrvLPUART_InitWithOSResource(enLPUART1, &lpuart_config);
	
	/** Welcome Message */
	mimiclib_printf("\r\n\r\nVersion = %s\r\n", g_szProgramVersion);
	mimiclib_printf("\r\n\r\nCompiler Version\r\n");
#ifdef __GNUC__
	mimiclib_printf("GCC %s\r\n\r\n", __VERSION__);
#else
	mimiclib_printf("Unkown Compiler\r\n\r\n");
#endif
	mimiclib_printf("%s", DEF_PROMPT);

	/** Main Loop */
	for(;;){
		uint32_t u32;
		uint32_t argc;
		char *argv[16];
		char *ctx = NULL;

		/** Waiting Return Code */
		bsp_gets(szBuf, 512);
		
		/** Split Command and argments */
		argc = 0u;
		argv[argc] = bsp_strtok(szBuf, " ", &ctx);
    	while (argv[argc] != NULL) {
			argc++;
			argv[argc] = bsp_strtok(NULL, " ", &ctx);
			if(argc >= 15u){
				break;
			}
		}

		/** Execute Command */
		u32 = 0u;
		while(g_stCmdTable[u32].m_pszCmd != NULL){
			bsp_strupper(argv[0]);
			if(strncmp(g_stCmdTable[u32].m_pszCmd, argv[0], 32) == 0){
				g_stCmdTable[u32].m_pfnc(argc, (const char **)argv);
				break;
			}
			u32++;
		}
		mimiclib_printf("%s", DEF_PROMPT);
	}
}
