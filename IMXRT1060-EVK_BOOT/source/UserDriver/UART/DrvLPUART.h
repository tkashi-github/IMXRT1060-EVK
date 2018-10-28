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
#include "OSResource.h"

#include "fsl_lpuart.h"

extern void LPUART1_IRQHandler(void);
extern void LPUART2_IRQHandler(void);
extern void LPUART3_IRQHandler(void);
extern void LPUART4_IRQHandler(void);
extern void LPUART5_IRQHandler(void);
extern void LPUART6_IRQHandler(void);
extern void LPUART7_IRQHandler(void);
extern void LPUART8_IRQHandler(void);

extern _Bool DrvLPUARTInit(enLPUART_t enLPUARTNo, const lpuart_config_t *config);

#ifdef __cplusplus
}
#endif

