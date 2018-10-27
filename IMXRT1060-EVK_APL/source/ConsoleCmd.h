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

typedef void (*pfnCmd)(uint32_t argc, const char *argv[]);

typedef struct{
	const char      *m_pszCmd;
	const pfnCmd    m_pfnc;
	const char      *m_pszComments;	
}stCmdTable_t;


extern stCmdTable_t g_stCmdTable[];
#ifdef __cplusplus
}
#endif
