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
#include <stdint.h>
/* ignore some GCC warnings */
#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


/**
 * @brief Disable CPU IRQ bit
 * @return primask register value
 */
static inline uint32_t CM7_DisableIRQ(void)
{
	uint32_t result;

	__asm volatile ("MRS %0, primask" : "=r" (result) );
	__asm volatile ("cpsid i" : : : "memory");
    return(result);
}

/**
 * @brief Set CPU IRQ bit
 * @param [in] priMask Last register value
 * @return void
 */
static inline void CM7_SetIRQ(uint32_t priMask)
{
	__asm volatile ("MSR primask, %0" : : "r" (priMask) : "memory");
}

#ifdef __cplusplus
}
#endif
