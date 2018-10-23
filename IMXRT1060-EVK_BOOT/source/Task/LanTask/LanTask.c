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

#include "Task/LanTask/LanTask.h"
#include "board.h"
#include "mimiclib/mimiclib.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "lwip/opt.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "netif/ethernet.h"
#include "ethernetif.h"
#include "ping/ping.h"
#include "udpecho_raw/udpecho_raw.h"


/* MAC address configuration. */
#define configMAC_ADDR                     \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x25 \
    }


/**
 * @brief Netif0
 */
struct netif fsl_netif0;

/**
 * @brief Initialize protocol stack
 * @param [in]  bDHCP true:Enable DHCP, false:Disable DHCP
 * @return void
 */
DefALLOCATE_ITCM static void InitNetwork(_Bool bDHCP)
{
    ip4_addr_t fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw;
    ethernetif_config_t fsl_enet_config0 = {
        .phyAddress = BOARD_ENET0_PHY_ADDRESS, .clockName = kCLOCK_CoreSysClk, .macAddress = configMAC_ADDR,
    };

    
	if(bDHCP == false){
		ip4_addr_t ip4PingTarget;
		IP4_ADDR(&ip4PingTarget, 192, 168, 100, 50);
		IP4_ADDR(&fsl_netif0_ipaddr, 192, 168, 100, 90);
		IP4_ADDR(&fsl_netif0_netmask, 255, 255, 255, 0);
		IP4_ADDR(&fsl_netif0_gw, 0, 0, 0, 0);

		
		mimiclib_printf("Static IP Mode\r\n");

		netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, &fsl_enet_config0, ethernetif0_init,
				tcpip_input);
		netif_set_default(&fsl_netif0);
		netif_set_up(&fsl_netif0);
		mimiclib_printf("************************************************\r\n");
		mimiclib_printf(" IPv4 Address     : %u.%u.%u.%u\r\n", ((u8_t *)&fsl_netif0_ipaddr)[0], ((u8_t *)&fsl_netif0_ipaddr)[1],
			((u8_t *)&fsl_netif0_ipaddr)[2], ((u8_t *)&fsl_netif0_ipaddr)[3]);
		mimiclib_printf(" IPv4 Subnet mask : %u.%u.%u.%u\r\n", ((u8_t *)&fsl_netif0_netmask)[0], ((u8_t *)&fsl_netif0_netmask)[1],
			((u8_t *)&fsl_netif0_netmask)[2], ((u8_t *)&fsl_netif0_netmask)[3]);
		mimiclib_printf(" IPv4 Gateway     : %u.%u.%u.%u\r\n", ((u8_t *)&fsl_netif0_gw)[0], ((u8_t *)&fsl_netif0_gw)[1],
			((u8_t *)&fsl_netif0_gw)[2], ((u8_t *)&fsl_netif0_gw)[3]);
		mimiclib_printf("************************************************\r\n");

	}else{

		mimiclib_printf("Getting IP address from DHCP ...\r\n");
		dhcp_start(&fsl_netif0);

		struct dhcp *dhcp;
		dhcp = (struct dhcp *)netif_get_client_data(&fsl_netif0, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

		while (dhcp->state != DHCP_STATE_BOUND)
		{
			vTaskDelay(1000);
		}

		if (dhcp->state == DHCP_STATE_BOUND)
		{
			mimiclib_printf("\r\n IPv4 Address     : %u.%u.%u.%u\r\n", ((u8_t *)&fsl_netif0.ip_addr.addr)[0],
				((u8_t *)&fsl_netif0.ip_addr.addr)[1], ((u8_t *)&fsl_netif0.ip_addr.addr)[2],
				((u8_t *)&fsl_netif0.ip_addr.addr)[3]);
		}
		mimiclib_printf("DHCP OK\r\n");
	}
#if 1	// UDP Echo Test
	mimiclib_printf("Start UDP Echo\r\n");
	udpecho_raw_init();
#endif
}



/**
 * @brief return Link UP/Down
 * @return true Link up
 * @return false Link down
 */
DefALLOCATE_ITCM static _Bool IsLinkUP(void)
{
	_Bool bLinkUp = false;

	PHY_GetLinkStatus(ENET, BOARD_ENET0_PHY_ADDRESS, &bLinkUp);

	return bLinkUp;
}

/**
 * @brief Reset Phy
 */
DefALLOCATE_ITCM static void ResetPhy(void){
	/** Assert ENET_RST 1sec */
	GPIO_WritePinOutput(GPIO1, 9, 0);
    vTaskDelay(500);
    GPIO_WritePinOutput(GPIO1, 9, 1);
    vTaskDelay(500);
}

/**
 * @brief Init/Deinit protocol stack
 */
DefALLOCATE_ITCM static void LinkUpDownProc(void){

	if(IsLinkUP() == false){
		mimiclib_printf("Link Down detected.\r\n");
		netif_remove(&fsl_netif0);
	}else{
		mimiclib_printf("Link Up detected.\r\n");
		InitNetwork(false);
	}
}

/**
 * @brief Lan Task Main Loop
 */
DefALLOCATE_ITCM static void LanTaskActual(void){
	static _Bool s_bLastLinkStatus = false;
	static _Bool s_bInited = false;

	stTaskMsgBlock_t stTaskMsg = {0};
	if(sizeof(stTaskMsg) <= xStreamBufferReceive(g_sbhLanTask, &stTaskMsg, sizeof(stTaskMsg), 100)){
		switch(stTaskMsg.enMsgId){
		case enLanLinkChange:
			if(s_bInited != false){
				LinkUpDownProc();
			}
			break;
		case enLanRestart:
			s_bInited = false;
			mimiclib_printf("[%s (%d)] Restart!\r\n", __FUNCTION__, __LINE__);
			ResetPhy();
			tcpip_init(NULL, NULL);
			InitNetwork(false);
			s_bInited = true;
			s_bLastLinkStatus = true;
			break;
		default:
			mimiclib_printf("[%s (%d)] Unkown Msg (%d)\r\n", __FUNCTION__, __LINE__, stTaskMsg.enMsgId);
			break;
		}
		
		/** Sync */
		if(stTaskMsg.SyncEGHandle != NULL){
			osEventFlagsSet(stTaskMsg.SyncEGHandle, stTaskMsg.wakeupbits);
		}

		/** Free */
		if(stTaskMsg.ptrDataForDst != (uintptr_t)NULL){
			vPortFree((void*)stTaskMsg.ptrDataForDst);
		}
	}else{
#if 1
		if(s_bInited != false){
			_Bool bCurrentLinkStatus = IsLinkUP();
			if(s_bLastLinkStatus != bCurrentLinkStatus){
				PostMsgLanTaskLinkChange();
				s_bLastLinkStatus = bCurrentLinkStatus;
			}
		}
#endif
	}
}


/**
 * @brief Lan Task Entry
 */
DefALLOCATE_ITCM void LanTask(void const *argument){
	gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
	gpio_pin_config_t gpio_enet_config = {kGPIO_DigitalInput, 0, kGPIO_NoIntmode};
	const clock_enet_pll_config_t config = {true, false, 1};

    CLOCK_InitEnetPll(&config);
	IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true);

	/** ENET_RST Pin*/
    GPIO_PinInit(GPIO1, 9, &gpio_config);
	GPIO_PinInit(GPIO1, 10, &gpio_enet_config);

	GPIO_WritePinOutput(GPIO1, 9, 0);
	for(;;){
		LanTaskActual();
	}

	vTaskDelete(NULL);
}


/**
 * @brief Post Message (enLanRestart)
 * @return true Post OK
 * @return false Post NG
 */
DefALLOCATE_ITCM _Bool PostMsgLanTaskRestart(void){
		/** var */
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};

	stTaskMsg.enMsgId = enLanRestart;

	if(sizeof(stTaskMsg) != xStreamBufferSend(g_sbhLanTask, &stTaskMsg, sizeof(stTaskMsg), 50)){
		mimiclib_printf("[%s (%d)] xStreamBufferSend NG\r\n", __FUNCTION__, __LINE__);
		return false;
	}
	
	return true;
}

/**
 * @brief Post Message (enLanLinkChange)
 * @return true Post OK
 * @return false Post NG
 */
DefALLOCATE_ITCM _Bool PostMsgLanTaskLinkChange(void){
	/** var */
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};

	/** begin */
	stTaskMsg.enMsgId = enLanLinkChange;

	if(pdFALSE == xPortIsInsideInterrupt()){
		if(sizeof(stTaskMsg) != xStreamBufferSend(g_sbhLanTask, &stTaskMsg, sizeof(stTaskMsg), 50)){
			mimiclib_printf("[%s (%d)] xStreamBufferSend NG\r\n", __FUNCTION__, __LINE__);
			return false;
		}
	}else{
		BaseType_t xHigherPriorityTaskWoken;

		if(sizeof(stTaskMsg) != xStreamBufferSendFromISR(g_sbhLanTask, &stTaskMsg, sizeof(stTaskMsg), &xHigherPriorityTaskWoken)){
			return false;
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

	
	return true;
}

