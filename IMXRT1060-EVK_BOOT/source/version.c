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
#include "version.h"
#include "buildlog.h"

#define DEF_PROGRAM "IMXRT1060-EVK_BOOT"
#define DEF_VERSION "Ver 0.1.0 "

const char g_szProgram[] = {DEF_PROGRAM};
const char g_szVersion[] = {DEF_VERSION};	/* Major.Minor.Bugfix */
const char g_szProgramVersion[] = {DEF_PROGRAM " " DEF_VERSION DEF_PREBUILD_TIME " " DEF_TOTAL_BUILD_COUNT};

