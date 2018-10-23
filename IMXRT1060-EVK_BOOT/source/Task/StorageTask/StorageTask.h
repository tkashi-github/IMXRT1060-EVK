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
#include "diskio.h"

extern void StorageTask(void const *argument);
extern _Bool PostMsgStorageTaskInsertFromISR(_Bool bInsert);
/*!
 * @brief Initializes Storage.
 *
 * @param physicalDrive Physical drive number.
 * @retval STA_NOINIT Failed.
 * @retval RES_OK Success.
 */
extern DSTATUS StorageInitialize(uint8_t physicalDrive);

/*!
 * Gets Storage status
 *
 * @param physicalDrive Physical drive number.
 * @retval STA_NOINIT Failed.
 * @retval RES_OK Success.
 */
extern DSTATUS StorageStatus(uint8_t physicalDrive);

/*!
 * @brief Reads Storage.
 *
 * @param physicalDrive Physical drive number.
 * @param buffer The data buffer pointer to store read content.
 * @param sector The start sector number to be read.
 * @param count The sector count to be read.
 * @retval RES_PARERR Failed.
 * @retval RES_OK Success.
 */
extern DRESULT StorageRead(uint8_t physicalDrive, uint8_t *buffer, uint32_t sector, uint8_t count);

/*!
 * @brief Writes Storage.
 *
 * @param physicalDrive Physical drive number.
 * @param buffer The data buffer pointer to store write content.
 * @param sector The start sector number to be written.
 * @param count The sector count to be written.
 * @retval RES_PARERR Failed.
 * @retval RES_OK Success.
 */
extern DRESULT StorageWrite(uint8_t physicalDrive, const uint8_t *buffer, uint32_t sector, uint8_t count);

/*!
 * @brief Storage IO operation.
 *
 * @param physicalDrive Physical drive number.
 * @param command The command to be set.
 * @param buffer The buffer to store command result.
 * @retval RES_PARERR Failed.
 * @retval RES_OK Success.
 */
extern DRESULT StorageIoctl(uint8_t physicalDrive, uint8_t command, void *buffer);


extern void CmdStorageDump(uint32_t argc, const char *argv[]);
#ifdef __cplusplus
}
#endif

