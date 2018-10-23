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



extern stu8RingBuffer_t g_stLPUARTRxBuf[];
extern stu8RingBuffer_t g_stLPUARTTxBuf[];

extern void LPUART1HandleIRQ(void);
extern void LPUART2HandleIRQ(void);
extern void LPUART3HandleIRQ(void);
extern void LPUART4HandleIRQ(void);
extern void LPUART5HandleIRQ(void);
extern void LPUART6HandleIRQ(void);
extern void LPUART7HandleIRQ(void);
extern void LPUART8HandleIRQ(void);

extern _Bool DrvLPUART_InitWithOSResource(enLPUART_t enLPUARTNo, const lpuart_config_t *config);
#ifdef __cplusplus
}
#endif

