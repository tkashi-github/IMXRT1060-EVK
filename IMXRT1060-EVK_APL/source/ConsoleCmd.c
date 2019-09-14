/**
 * @file ConsoleCmd.c
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
#include "ConsoleCmd.h"
#include "board.h"

#include <stdlib.h>
#include "version.h"
#include "FreeRTOS.h"
#include "task.h"

#include "FileCmd.h"
#include "mimiclib.h"

#include "task.h"

#include "lwip/timeouts.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "ip.h"
#include "ping/ping.h"
#include "common/storageBenchMark.h"

#include "SensorTask/SensorTask.h"
#include "SPIFlash/SPIFlash.h"
#include "common/common.h"

#include "TouchScreenTask/TouchScreenTask.h"
#include "TempMoniTask/TempMoniTask.h"
#include "PlayCtrl/PlayCtrl.h"
#include "AudioFile/AudioFileList.h"

#include "ExtLedCtrlTask/ExtLedCtrlTask.h"

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
static void CmdPing(uint32_t argc, const char *argv[]);
static void CmdReboot(uint32_t argc, const char *argv[]);

stCmdTable_t g_stCmdTable[] = {
	{"HELP", CmdHelp, "Help"},			/* Help Command*/
	{"VERSION", CmdVersion, "Version"}, /* Version Command*/
	{"ARG", CmdArg, "Argment Test"},	/* Argment Command*/
	{"TICK", CmdTick, "Tick Test"},		/* Tick Command*/
	{"LOAD", CmdLoad, "CPU Load"},		/* LOAD Command*/
	{"REBOOT",CmdReboot,"Reboot"},
	{"NVIC", CmdNvic, "NVIC"},
	{"LS", FileCmd_ls, "LS"},			/** */
	{"MKDIR", FileCmd_mkdir, "mkdir"},  /** */
	{"RM", FileCmd_rm, "RM"},			/** */
	{"CAT", FileCmd_cat, "CAT"},		/** */
	{"CD", FileCmd_cd, "CD"},			/** */

	{"CLK", CmdClock, "Show Clock"},
	{"TASK", CmdTask, "vTaskList"},
	{"PING", CmdPing, "PING"},
	{"SB", CmdStorageBenchMark, "StorageBenchMark"},
	{"SENSOR", CmdSensor, "Combo Sensor"},
	{"SFROM", CmdSFROM, "SerialFlashTest"},
	{"CTP", CmdCTPTest, "CTP Test"},
	{"TEMP", CmdTemp, "Temperature Monitor"},
	{"REPEAT", CmdRepeat, "Set Repeat Mode"},
	{"MODE", CmdMode, "Set Plya Mode"},
	{"PLAY", CmdPlay, "Play Start"},
	{"STOP", CmdStop, "Play/Rec Stop"},
	{"REC", CmdRec, "Rec Start"},
	{"LIST", CmdMakeAudioFileList, "AUdio File List"},
	{"EXTLED", CmdExtLed, "Ext Led Test"},
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
	mimic_printf("\r\n[%s (%d)] HELP\r\n", __func__, __LINE__);

	while (g_stCmdTable[u32].m_pszCmd != NULL)
	{
		mimic_printf("%03lu : %-20s %-20s\r\n", u32, g_stCmdTable[u32].m_pszCmd, g_stCmdTable[u32].m_pszComments);
		u32++;
	}
	mimic_printf("[%s (%d)] ----\r\n", __func__, __LINE__);
	return;
}

/**
 * @brief VERSION
 * @param [in]  argc Number of Argments
 * @param [in]  argv Argments
 * @return void
 */
static void CmdVersion(uint32_t argc, const char *argv[])
{
	mimic_printf("\r\n\r\n%s\r\n", g_szProgramVersion);

	mimic_printf("\r\nCompiler Version\r\n");

#ifdef __GNUC__
	mimic_printf("GCC %s\r\n\r\n", __VERSION__);
#else
	mimic_printf("Unkown Compiler\r\n\r\n");
#endif
	mimic_printf("FREERTOS_KERNEL_VERSION = %s\r\n", tskKERNEL_VERSION_NUMBER);
}

/**
 * @brief ARGMENTS TEST
 * @param [in]  argc Number of Argments
 * @param [in]  argv Argments
 * @return void
 */
static void CmdArg(uint32_t argc, const char *argv[])
{
	mimic_printf("Argment Test\r\n");

	for (uint32_t i = 0u; i < argc; i++)
	{
		mimic_printf("argc = %02lu : %s\r\n", i, argv[i]);
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
	mimic_printf("Tick Test\r\n");

	tick = xTaskGetTickCount();
	while (mimic_kbhit() == false)
	{
		mimic_printf("tick = %lu msec\r\n", tick);
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
	mimic_printf("CPU Load\r\n");

	tick = xTaskGetTickCount();
	while (mimic_kbhit() == false)
	{
		GetRunCount(&LastRun, &MaxRun);
		mimic_printf("Load = %f [%lu, %lu](%lu msec)\r\n", 1.0 - (double)LastRun / (double)MaxRun, LastRun, MaxRun, tick);
		vTaskDelayUntil((TickType_t *const) & tick, 1000);
	}
}


static void CmdClock(uint32_t argc, const char *argv[])
{
	mimic_printf("kCLOCK_CpuClk                 = %12lu\r\n", CLOCK_GetFreq(kCLOCK_CpuClk));
	mimic_printf("kCLOCK_AhbClk                 = %12lu\r\n", CLOCK_GetFreq(kCLOCK_AhbClk));
	mimic_printf("kCLOCK_SemcClk                = %12lu\r\n", CLOCK_GetFreq(kCLOCK_SemcClk));
	mimic_printf("kCLOCK_IpgClk                 = %12lu\r\n", CLOCK_GetFreq(kCLOCK_IpgClk));
	mimic_printf("kCLOCK_OscClk                 = %12lu\r\n", CLOCK_GetFreq(kCLOCK_OscClk));
	mimic_printf("kCLOCK_RtcClk                 = %12lu\r\n", CLOCK_GetFreq(kCLOCK_RtcClk));
	mimic_printf("kCLOCK_ArmPllClk              = %12lu\r\n", CLOCK_GetFreq(kCLOCK_ArmPllClk));
	mimic_printf("kCLOCK_Usb1PllClk             = %12lu\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllClk));
	mimic_printf("kCLOCK_Usb1PllPfd0Clk         = %12lu\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk));
	mimic_printf("kCLOCK_Usb1PllPfd1Clk         = %12lu\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd1Clk));
	mimic_printf("kCLOCK_Usb1PllPfd2Clk         = %12lu\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd2Clk));
	mimic_printf("kCLOCK_Usb1PllPfd3Clk         = %12lu\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd3Clk));
	mimic_printf("kCLOCK_Usb2PllClk             = %12lu\r\n", CLOCK_GetFreq(kCLOCK_Usb2PllClk));
	mimic_printf("kCLOCK_SysPllClk              = %12lu\r\n", CLOCK_GetFreq(kCLOCK_SysPllClk));
	mimic_printf("kCLOCK_SysPllPfd0Clk          = %12lu\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd0Clk));
	mimic_printf("kCLOCK_SysPllPfd1Clk          = %12lu\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd1Clk));
	mimic_printf("kCLOCK_SysPllPfd2Clk          = %12lu\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk));
	mimic_printf("kCLOCK_SysPllPfd3Clk          = %12lu\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd3Clk));
	mimic_printf("kCLOCK_EnetPll0Clk            = %12lu\r\n", CLOCK_GetFreq(kCLOCK_EnetPll0Clk));
	mimic_printf("kCLOCK_EnetPll1Clk            = %12lu\r\n", CLOCK_GetFreq(kCLOCK_EnetPll1Clk));
	mimic_printf("kCLOCK_AudioPllClk            = %12lu\r\n", CLOCK_GetFreq(kCLOCK_AudioPllClk));
	mimic_printf("kCLOCK_VideoPllClk            = %12lu\r\n", CLOCK_GetFreq(kCLOCK_VideoPllClk));
	mimic_printf("SD_HOST_CLK_FREQ              = %12lu\r\n", BOARD_SD_HOST_CLK_FREQ);
	mimic_printf("BOARD_CODEC_I2C_CLOCK_FREQ    = %12lu\r\n", BOARD_CODEC_I2C_CLOCK_FREQ);
	
}

static void CmdTask(uint32_t argc, const char *argv[])
{
	char szStr[2048];

	mimic_printf("TaskName | CurrentState | Priority | StackWM | TaskNo\r\n");
	vTaskList(szStr);
	mimic_printf("%s\r\n", szStr);
}

static void CmdNvic(uint32_t argc, const char *argv[]){
	for(uint32_t i=0;i<NUMBER_OF_INT_VECTORS;i++){
		mimic_printf("IRQ%03d = %lu:%lu, pend = %lu, pri = %lu\r\n", i, 
			NVIC_GetActive((IRQn_Type)i), NVIC_GetEnableIRQ((IRQn_Type)i), NVIC_GetPendingIRQ((IRQn_Type)i), NVIC_GetPriority((IRQn_Type)i));
	}
}


static void CmdPing(uint32_t argc, const char *argv[])
{
	int s;
	ip_addr_t ping_addr;

	if(argc != 2){
		mimic_printf("ping <ip>\r\n");
	}
	ip4addr_aton(argv[1], &ping_addr);

#if LWIP_SO_SNDRCVTIMEO_NONSTANDARD
	int timeout = PING_RCV_TIMEO;
#else
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
#endif
	
#if LWIP_IPV6
	if (IP_IS_V4(ping_target) || ip6_addr_isipv6mappedipv4(ip_2_ip6(ping_target)))
	{
		s = lwip_socket(AF_INET6, SOCK_RAW, IP_PROTO_ICMP);
	}
	else
	{
		s = lwip_socket(AF_INET6, SOCK_RAW, IP6_NEXTH_ICMP6);
	}
#else
	s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP);
#endif
	if (s < 0)
	{
		return;
	}

	lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	for(uint32_t i=0;i<4;i++)
	{
		if (ping_send(s, &ping_addr) == ERR_OK)
		{
			mimic_printf("ping: send %s OK\r\n", argv[1]);
			ping_recv(s, sys_now());
		}
		else
		{
			mimic_printf("ping: send %s NG\r\n", argv[1]);
		}
		sys_msleep(1000);
	}
}

static void CmdReboot(uint32_t argc, const char *argv[]){
	reboot();
}

