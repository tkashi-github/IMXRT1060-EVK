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
#include "ConsoleCmd.h"

#include <stdlib.h>
#include <string.h>
#include "version.h"
#include "FreeRTOS.h"
#include "task.h"
#include "CPUFunc.h"

#include "mimiclib/mimiclib.h"

#ifdef __cplusplus
extern "C"
{
#endif
	extern void GetRunCount(uint32_t *p32Last, uint32_t *pu32Max);
#ifdef __cplusplus
}
#endif
static void CmdHelp(uint32_t argc, const char *argv[]);
static void CmdVersion(uint32_t argc, const char *argv[]);
static void CmdArg(uint32_t argc, const char *argv[]);
static void CmdTick(uint32_t argc, const char *argv[]);
static void CmdLoad(uint32_t argc, const char *argv[]);
static void CmdClock(uint32_t argc, const char *argv[]);
static void CmdTask(uint32_t argc, const char *argv[]);

static void CmdNvic(uint32_t argc, const char *argv[]);


stCmdTable_t g_stCmdTable[] = {
	{"HELP", CmdHelp, "Help"},			/* Help Command*/
	{"VERSION", CmdVersion, "Version"}, /* Version Command*/
	{"ARG", CmdArg, "Argment Test"},	/* Version Command*/
	{"TICK", CmdTick, "Tick Test"},		/* Version Command*/
	{"LOAD", CmdLoad, "CPU Load"},		/* Version Command*/
	{"NVIC", CmdNvic, "NVIC"},
	{"LS", FileCmd_ls, "LS"},			/** */
	{"MKDIR", FileCmd_mkdir, "mkdir"},  /** */
	{"RM", FileCmd_rm, "RM"},			/** */
	{"CAT", FileCmd_cat, "CAT"},		/** */
	{"CD", FileCmd_cd, "CD"},			/** */

	{"CLK", CmdClock, "Show Clock"},
	{"TASK", CmdTask, "vTaskList"},
	{NULL, NULL, NULL}, /* Terminator */
};

/**
 * @brief HELP
 * @param [in]  argc Number of Argments
 * @param [in]  argv Argments
 * @return void
 */
static void CmdHelp(uint32_t argc, const char *argv[])
{
	/*-- var --*/
	uint32_t u32 = 0u;

	/*-- begin --*/
	mimiclib_printf("\r\n[%s (%d)] HELP\r\n", __FUNCTION__, __LINE__);

	while (g_stCmdTable[u32].m_pszCmd != NULL)
	{
		mimiclib_printf("%03lu : %-20s %-20s\r\n", u32, g_stCmdTable[u32].m_pszCmd, g_stCmdTable[u32].m_pszComments);
		u32++;
		vTaskDelay(20);
	}
	mimiclib_printf("[%s (%d)] ----\r\n", __FUNCTION__, __LINE__);
	return;
}

extern const uint8_t FreeRTOSDebugConfig[];

/**
 * @brief VERSION
 * @param [in]  argc Number of Argments
 * @param [in]  argv Argments
 * @return void
 */
static void CmdVersion(uint32_t argc, const char *argv[])
{
	mimiclib_printf("\r\n\r\n%s\r\n", g_szProgramVersion);

	mimiclib_printf("\r\nCompiler Version\r\n");

#ifdef __GNUC__
	mimiclib_printf("GCC %s\r\n\r\n", __VERSION__);
#else
	mimiclib_printf("Unkown Compiler\r\n\r\n");
#endif
	mimiclib_printf("FREERTOS_DEBUG_CONFIG_VERSION = %d.%d\r\n", FreeRTOSDebugConfig[0], FreeRTOSDebugConfig[1]);
	mimiclib_printf("FREERTOS_KERNEL_VERSION = %d.%d.%d\r\n", FreeRTOSDebugConfig[2], FreeRTOSDebugConfig[3], FreeRTOSDebugConfig[4]);
}

/**
 * @brief ARGMENTS TEST
 * @param [in]  argc Number of Argments
 * @param [in]  argv Argments
 * @return void
 */
static void CmdArg(uint32_t argc, const char *argv[])
{
	mimiclib_printf("Argment Test\r\n");

	for (uint32_t i = 0u; i < argc; i++)
	{
		mimiclib_printf("argc = %02lu : %s\r\n", i, argv[i]);
	}
}

/**
 * @brief TICK TEST
 * @param [in]  argc Number of Argments
 * @param [in]  argv Argments
 * @return void
 */
static void CmdTick(uint32_t argc, const char *argv[])
{
	TickType_t tick;
	mimiclib_printf("Tick Test\r\n");

	tick = xTaskGetTickCount();
	while (bsp_kbhit() == false)
	{
		mimiclib_printf("tick = %lu msec\r\n", tick);
		vTaskDelayUntil((TickType_t *const) & tick, 1000);
	}
}

/**
 * @brief CPULOAD
 * @param [in]  argc Number of Argments
 * @param [in]  argv Argments
 * @return void
 */
static void CmdLoad(uint32_t argc, const char *argv[])
{
	TickType_t tick;
	uint32_t LastRun, MaxRun;
	mimiclib_printf("CPU Load\r\n");

	tick = xTaskGetTickCount();
	while (bsp_kbhit() == false)
	{
		GetRunCount(&LastRun, &MaxRun);
		mimiclib_printf("Load = %f [%lu, %lu](%lu msec)\r\n", 1.0 - (double)LastRun / (double)MaxRun, LastRun, MaxRun, tick);
		DebugTraceX("DEBUG", dbgMINOR, "Load = %f [%lu, %lu](%lu msec)", 1.0 - (double)LastRun / (double)MaxRun, LastRun, MaxRun, tick);
		vTaskDelayUntil((TickType_t *const) & tick, 1000);
	}
}


static void CmdClock(uint32_t argc, const char *argv[])
{
	mimiclib_printf("kCLOCK_CpuClk         = %12lu\r\n", CLOCK_GetFreq(kCLOCK_CpuClk));
	mimiclib_printf("kCLOCK_AhbClk         = %12lu\r\n", CLOCK_GetFreq(kCLOCK_AhbClk));
	mimiclib_printf("kCLOCK_SemcClk        = %12lu\r\n", CLOCK_GetFreq(kCLOCK_SemcClk));
	mimiclib_printf("kCLOCK_IpgClk         = %12lu\r\n", CLOCK_GetFreq(kCLOCK_IpgClk));
	mimiclib_printf("kCLOCK_OscClk         = %12lu\r\n", CLOCK_GetFreq(kCLOCK_OscClk));
	mimiclib_printf("kCLOCK_RtcClk         = %12lu\r\n", CLOCK_GetFreq(kCLOCK_RtcClk));
	mimiclib_printf("kCLOCK_ArmPllClk      = %12lu\r\n", CLOCK_GetFreq(kCLOCK_ArmPllClk));
	mimiclib_printf("kCLOCK_Usb1PllClk     = %12lu\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllClk));
	mimiclib_printf("kCLOCK_Usb1PllPfd0Clk = %12lu\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk));
	mimiclib_printf("kCLOCK_Usb1PllPfd1Clk = %12lu\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd1Clk));
	mimiclib_printf("kCLOCK_Usb1PllPfd2Clk = %12lu\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd2Clk));
	mimiclib_printf("kCLOCK_Usb1PllPfd3Clk = %12lu\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd3Clk));
	mimiclib_printf("kCLOCK_Usb2PllClk     = %12lu\r\n", CLOCK_GetFreq(kCLOCK_Usb2PllClk));
	mimiclib_printf("kCLOCK_SysPllClk      = %12lu\r\n", CLOCK_GetFreq(kCLOCK_SysPllClk));
	mimiclib_printf("kCLOCK_SysPllPfd0Clk  = %12lu\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd0Clk));
	mimiclib_printf("kCLOCK_SysPllPfd1Clk  = %12lu\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd1Clk));
	mimiclib_printf("kCLOCK_SysPllPfd2Clk  = %12lu\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk));
	mimiclib_printf("kCLOCK_SysPllPfd3Clk  = %12lu\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd3Clk));
	mimiclib_printf("kCLOCK_EnetPll0Clk    = %12lu\r\n", CLOCK_GetFreq(kCLOCK_EnetPll0Clk));
	mimiclib_printf("kCLOCK_EnetPll1Clk    = %12lu\r\n", CLOCK_GetFreq(kCLOCK_EnetPll1Clk));
	mimiclib_printf("kCLOCK_AudioPllClk    = %12lu\r\n", CLOCK_GetFreq(kCLOCK_AudioPllClk));
	mimiclib_printf("kCLOCK_VideoPllClk    = %12lu\r\n", CLOCK_GetFreq(kCLOCK_VideoPllClk));
	mimiclib_printf("SD_HOST_CLK_FREQ      = %12lu\r\n", SD_HOST_CLK_FREQ);
}

static void CmdTask(uint32_t argc, const char *argv[])
{
	char szStr[2048];

	mimiclib_printf("TaskName | CurrentState | Priority | StackWM | TaskNo\r\n");
	vTaskList(szStr);
	mimiclib_printf("%s\r\n", szStr);
}

static void CmdNvic(uint32_t argc, const char *argv[]){
	for(uint32_t i=0;i<MAX_IRQn;i++){
		mimiclib_printf("IRQ%03d = %lu:%lu, pend = %lu, pri = %lu\r\n", i, 
			NVIC_GetActive((IRQn_Type)i), NVIC_GetEnableIRQ((IRQn_Type)i), NVIC_GetPendingIRQ((IRQn_Type)i), NVIC_GetPriority((IRQn_Type)i));
		vTaskDelay(10);
	}
}

