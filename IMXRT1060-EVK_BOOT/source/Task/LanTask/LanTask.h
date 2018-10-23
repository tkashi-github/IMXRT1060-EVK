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
#ifndef __cplusplus
#if __STDC_VERSION__ < 201112L
#error /** Only C11 */
#endif
#endif
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
/** User Typedefine */
#include "UserTypedef.h"
#include "fsl_gpio.h"
#include "fsl_phy.h"

extern struct netif fsl_netif0;

extern void LanTask(void const *argument);
extern _Bool PostMsgLanTaskRestart(void);
extern _Bool PostMsgLanTaskLinkChange(void);

#ifdef __cplusplus
}
#endif

