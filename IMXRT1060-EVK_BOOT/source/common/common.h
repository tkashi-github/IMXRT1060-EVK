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

extern void MemDump(uintptr_t addr, size_t ByteCnt);
extern TCHAR *GetFileExt(TCHAR szExt[], const TCHAR szFilePath[], size_t SizeExt);


#ifdef __cplusplus
}
#endif
