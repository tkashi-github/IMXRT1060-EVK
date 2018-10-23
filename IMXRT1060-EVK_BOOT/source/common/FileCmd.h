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

extern void FileCmd_ls(uint32_t argc, const char *argv[]);
extern void FileCmd_mkdir(uint32_t argc, const char *argv[]);
extern void FileCmd_cat(uint32_t argc, const char *argv[]);
extern void FileCmd_rm(uint32_t argc, const char *argv[]);
extern void FileCmd_cd(uint32_t argc, const char *argv[]);
#ifdef __cplusplus
}
#endif
