/**
 * @file StorageTask.c
 * @brief TODO
 * @author Takashi Kashiwagi
 * @date 2018/10/28
 * @version     0.1
 * @details 
 * --
 * License Type <MIT License>
 * --
 * Copyright 2018 Takashi Kashiwagi
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 * @par Update:
 * - 2018/10/28: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */
#include "StorageTask/StorageTask.h"
#include "board.h"
#include "mimiclib/mimiclib.h"
#include "common/common.h"
#include "fsl_sd.h"
#include "ff.h"


static void StorageInserted(bool isInserted, void *userData);
static void StorageRemoved(bool isInserted, void *userData);

static sd_card_t s_stSD[enNumOfSD];
static const sdmmchost_detect_card_t s_sdCardDetect[enNumOfSD] = {
	{
		.cdType = kSDMMCHOST_DetectCardByGpioCD,
		.cdTimeOut_ms = (~0U),
		.cardInserted = StorageInserted, /*!< card inserted callback which is meaningful for interrupt case */
		.cardRemoved = StorageRemoved,   /*!< card removed callback which is meaningful for interrupt case */
		.userData = (void *)enUSDHC1,	/** Slot No */
	},
	{
		.cdType = kSDMMCHOST_DetectCardByGpioCD,
		.cdTimeOut_ms = (~0U),
		.cardInserted = StorageInserted, /*!< card inserted callback which is meaningful for interrupt case */
		.cardRemoved = StorageRemoved,   /*!< card removed callback which is meaningful for interrupt case */
		.userData = (void *)enUSDHC2,	/** Slot No */
	},
};

/** Dummy */
DefALLOCATE_ITCM static void StorageInserted(bool isInserted, void *userData)
{
	enSD_t enSlotNo = (enSD_t)userData;

	if ((enSlotNo >= enUSDHC1) && (enSlotNo <= enUSDHC2))
	{
		TickType_t tTimeout = portMAX_DELAY;
		if(pdFALSE != xPortIsInsideInterrupt()){
			tTimeout = 0;
		}

		if (osSemaphoreAcquire(g_bsIdStorageTaskMsg, tTimeout) == osOK)
		{
			stTaskMsgBlock_t stTaskMsg;
			memset(&stTaskMsg, 0, sizeof(stTaskMsgBlock_t));
			stTaskMsg.enMsgId = enSDInsterted;

			if(pdFALSE != xPortIsInsideInterrupt()){
				BaseType_t xHigherPriorityTaskWoken = pdFALSE;
				xStreamBufferSendFromISR(g_sbhStorageTask[enSlotNo], &stTaskMsg, sizeof(stTaskMsg), &xHigherPriorityTaskWoken);
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}else{
				xStreamBufferSend(g_sbhStorageTask[enSlotNo], &stTaskMsg, sizeof(stTaskMsg), 10);
			}
			osSemaphoreRelease(g_bsIdStorageTaskMsg);
		}
	}
}
/** Dummy */
DefALLOCATE_ITCM static void StorageRemoved(bool isInserted, void *userData)
{

	enSD_t enSlotNo = (enSD_t)userData;
	if ((enSlotNo >= enUSDHC1) && (enSlotNo <= enUSDHC2))
	{
		TickType_t tTimeout = portMAX_DELAY;
		if(pdFALSE != xPortIsInsideInterrupt()){
			tTimeout = 0;
		}

		if (osSemaphoreAcquire(g_bsIdStorageTaskMsg, tTimeout) == osOK)
		{
			stTaskMsgBlock_t stTaskMsg;
			memset(&stTaskMsg, 0, sizeof(stTaskMsgBlock_t));
			stTaskMsg.enMsgId = enSDInsterted;

			if(pdFALSE != xPortIsInsideInterrupt()){
				BaseType_t xHigherPriorityTaskWoken = pdFALSE;
				xStreamBufferSendFromISR(g_sbhStorageTask[enSlotNo], &stTaskMsg, sizeof(stTaskMsg), &xHigherPriorityTaskWoken);
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}else{
				xStreamBufferSend(g_sbhStorageTask[enSlotNo], &stTaskMsg, sizeof(stTaskMsg), 10);
			}
			osSemaphoreRelease(g_bsIdStorageTaskMsg);
		}
	}
}

/**
 * @brief Init Storage
 * @param [in]  enSlotNo Slot No
 * @param [in]  card Pointer of Card Handle
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM static _Bool StorageInit(enSD_t enSlotNo, sd_card_t *card)
{
	/** var */
	status_t sts;
	_Bool bret = false;
	uint32_t StartTick = osKernelGetTickCount();

	/** begin */
	SD_HostInit(card);
	SD_HostReset(&(card->host));
	SD_PowerOffCard(card->host.base, NULL);
	SD_PowerOnCard(card->host.base, NULL);

	sts = SD_CardInit(card);

	if (sts == kStatus_Success)
	{
		mimic_printf("\r\nStorage Info [%lu msec]\r\n", StartTick);
		mimic_printf("SD_HOST_CLK_FREQ       = %lu\r\n", BOARD_SD_HOST_CLK_FREQ);
		mimic_printf("card->busClock_Hz      = %lu\r\n", card->busClock_Hz);
		mimic_printf("card->version          = %lu\r\n", card->version);
		mimic_printf("card->currentTiming    = %lu\r\n", card->currentTiming);
		mimic_printf("card->operationVoltage = %lu\r\n", card->operationVoltage);
		mimic_printf("card->blockCount       = %lu\r\n", card->blockCount);
		mimic_printf("card->blockSize        = %lu\r\n", card->blockSize);
		mimic_printf("<Initial Time = %lu msec>\r\n", osKernelGetTickCount() - StartTick);
		bret = true;
		
	}
	else
	{
		mimic_printf("[%s (%d)] SD_CardInit NG <%d> (Slot = %d)!\r\n", __FUNCTION__, __LINE__, sts, enSlotNo + 1);
	}
	return bret;
}

/**
 * @brief Deinit Storage
 * @param [in]  enSlotNo Slot No
 * @param [in]  card Pointer of Card Handle
 */
DefALLOCATE_ITCM static void StorageDeinit(enSD_t enSlotNo, sd_card_t *card)
{
	/** begin */
	SD_PowerOffCard(card->host.base, card->usrParam.pwr);
	SD_HostDeinit(card);
	mimic_printf("[%s (%d)] StorageDeinit OK (Slot = %d)!\r\n", __FUNCTION__, __LINE__, enSlotNo + 1);
}

static FATFS s_stFS[enNumOfSD]; /* File system object */

static const TCHAR *s_pszDriverLetter[enNumOfSD] = {
	"A:/",
	"B:/",
};

/**
 * @brief Mount File System
 * @param [in]  enSlotNo Slot No
 */
DefALLOCATE_ITCM static void StorageMount(enSD_t enSlotNo)
{
	FRESULT sts;

	memset(&s_stFS[enSlotNo], 0, sizeof(FATFS));
	sts = f_mount(&s_stFS[enSlotNo], s_pszDriverLetter[enSlotNo], 1U);
	if (sts == FR_OK)
	{
		mimic_printf("f_mount OK (Slot = %d)\r\n", enSlotNo + 1);
		osEventFlagsSet(g_efFSReady, 1);
	}
	else
	{
		mimic_printf("f_mount NG <%d> (Slot = %d)\r\n", sts, enSlotNo + 1);
	}
}

/**
 * @brief Unmount File System
 * @param [in]  enSlotNo Slot No
 */
DefALLOCATE_ITCM static void StorageUnmount(enSD_t enSlotNo)
{
	FRESULT sts;

	sts = f_unmount(s_pszDriverLetter[enSlotNo]);
	if (sts == FR_OK)
	{
		mimic_printf("f_unmount OK (Slot = %d)!\r\n", enSlotNo + 1);
	}
	else
	{
		mimic_printf("f_unmount NG <%d> (Slot = %d)!\r\n", sts, enSlotNo + 1);
	}
}

/**
 * @brief Task Main Loop
 */
DefALLOCATE_ITCM static void StorageTaskActual(enSD_t enSlotNo)
{
	stTaskMsgBlock_t stTaskMsg = {0};
	if (sizeof(stTaskMsg) == xStreamBufferReceive(g_sbhStorageTask[enSlotNo], &stTaskMsg, sizeof(stTaskMsg), portMAX_DELAY))
	{
		switch (stTaskMsg.enMsgId)
		{
		case enSDInsterted:
			if (StorageInit(enSlotNo, &s_stSD[enSlotNo]) != false)
			{
				StorageMount(enSlotNo);
			}
			break;
		case enSDRemoved:
			StorageDeinit(enSlotNo, &s_stSD[enSlotNo]);
			StorageUnmount(enSlotNo);
			break;
		default:
			mimic_printf("[%s (%d)] Unkown Msg (Slot = %d)!\r\n", __FUNCTION__, __LINE__, enSlotNo + 1);
			break;
		}

		/** Sync */
		if (stTaskMsg.SyncEGHandle != NULL)
		{
			osEventFlagsSet(stTaskMsg.SyncEGHandle, stTaskMsg.wakeupbits);
		}
	}
}

/**
 * @brief Task Entry
 * @param [in]  argument nouse
 * @return void
 */
DefALLOCATE_ITCM void StorageTask(void const *argument)
{
	enSD_t enSlotNo = (enSD_t)argument;
	IRQn_Type enIRQn;

	if ((enSlotNo < enUSDHC1) || (enSlotNo > enUSDHC2))
	{
		vTaskSuspend(NULL);
	}

	enIRQn = USDHC1_IRQn + enSlotNo;
	mimic_printf("[%s (%d)] Start  (Slot = %d)!", __FUNCTION__, __LINE__, enSlotNo + 1);

	s_stSD[enSlotNo].host.base = BOARD_SD_HOST_BASEADDR;
	s_stSD[enSlotNo].host.sourceClock_Hz = BOARD_SD_HOST_CLK_FREQ;
	/* card detect type */
	s_stSD[enSlotNo].usrParam.cd = &s_sdCardDetect[enSlotNo];

	NVIC_SetPriority(enIRQn, kIRQ_PRIORITY_USDHC);

	for (;;)
	{
		StorageTaskActual(enSlotNo);
	}

	vTaskSuspend(NULL);
}

/** for FATFS */

DefALLOCATE_ITCM DSTATUS StorageInitialize(uint8_t physicalDrive)
{
	enSD_t enSlotNo = (enSD_t)physicalDrive;
	if ((enSlotNo < enUSDHC1) || (enSlotNo > enUSDHC2))
	{
		return RES_PARERR;
	}

	if (s_stSD[enSlotNo].isHostReady)
	{
		/** StorageTaskが初期化済み */
		return 0;
	}
	else
	{
		return STA_NOINIT;
	}
}

DefALLOCATE_ITCM DSTATUS StorageStatus(uint8_t physicalDrive)
{
	enSD_t enSlotNo = (enSD_t)physicalDrive;
	if ((enSlotNo < enUSDHC1) || (enSlotNo > enUSDHC2))
	{
		return RES_PARERR;
	}

	if (s_stSD[enSlotNo].isHostReady)
	{
		/** StorageTaskが初期化済み */
		return 0;
	}
	else
	{
		return STA_NOINIT;
	}
}

DefALLOCATE_ITCM DRESULT StorageRead(uint8_t physicalDrive, uint8_t *buffer, uint32_t sector, uint8_t count)
{
	enSD_t enSlotNo = (enSD_t)physicalDrive;
	if ((enSlotNo < enUSDHC1) || (enSlotNo > enUSDHC2))
	{
		return RES_PARERR;
	}

	if (kStatus_Success != SD_ReadBlocks(&s_stSD[enSlotNo], buffer, sector, count))
	{
		return RES_ERROR;
	}
	return RES_OK;
}

DefALLOCATE_ITCM DRESULT StorageWrite(uint8_t physicalDrive, const uint8_t *buffer, uint32_t sector, uint8_t count)
{
	enSD_t enSlotNo = (enSD_t)physicalDrive;
	if ((enSlotNo < enUSDHC1) || (enSlotNo > enUSDHC2))
	{
		return RES_PARERR;
	}

	if (kStatus_Success != SD_WriteBlocks(&s_stSD[enSlotNo], buffer, sector, count))
	{
		return RES_ERROR;
	}

	return RES_OK;
}

DefALLOCATE_ITCM DRESULT StorageIoctl(uint8_t physicalDrive, uint8_t command, void *buffer)
{
	DRESULT result = RES_OK;
	enSD_t enSlotNo = (enSD_t)physicalDrive;

	if ((enSlotNo < enUSDHC1) || (enSlotNo > enUSDHC2))
	{
		return RES_PARERR;
	}

	switch (command)
	{
	case GET_SECTOR_COUNT:
		if (buffer)
		{
			*(uint32_t *)buffer = s_stSD[enSlotNo].blockCount;
		}
		else
		{
			result = RES_PARERR;
		}
		break;
	case GET_SECTOR_SIZE:
		if (buffer)
		{
			*(uint32_t *)buffer = s_stSD[enSlotNo].blockSize;
		}
		else
		{
			result = RES_PARERR;
		}
		break;
	case GET_BLOCK_SIZE:
		if (buffer)
		{
			*(uint32_t *)buffer = s_stSD[enSlotNo].csd.eraseSectorSize;
		}
		else
		{
			result = RES_PARERR;
		}
		break;
	case CTRL_SYNC:
		result = RES_OK;
		break;
	default:
		result = RES_PARERR;
		break;
	}

	return result;
}

/**
 * @brief Post Message (enSDInsterted / enSDRemoved)
 * @param [in]  bInsert
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool PostMsgStorageTaskInsertFromISR(_Bool bInsert)
{
	/** var */
	_Bool bret = true;
	TickType_t tTimeout = portMAX_DELAY;
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};

	/** begin */
	memset(&stTaskMsg, 0, sizeof(stTaskMsg));

	
	if(pdFALSE != xPortIsInsideInterrupt()){
		tTimeout = 0;
	}

	if (osSemaphoreAcquire(g_bsIdStorageTaskMsg, tTimeout) == osOK)
	{
		stTaskMsgBlock_t stTaskMsg;
		memset(&stTaskMsg, 0, sizeof(stTaskMsgBlock_t));
		if (bInsert != false)
		{
			stTaskMsg.enMsgId = enSDInsterted;
		}
		else
		{
			stTaskMsg.enMsgId = enSDRemoved;
		}

		if(pdFALSE != xPortIsInsideInterrupt()){
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			if(sizeof(stTaskMsg) != xStreamBufferSendFromISR(g_sbhStorageTask[enUSDHC1], &stTaskMsg, sizeof(stTaskMsg), &xHigherPriorityTaskWoken)){
				bret = false;
			}
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}else{
			if(sizeof(stTaskMsg) != xStreamBufferSend(g_sbhStorageTask[enUSDHC1], &stTaskMsg, sizeof(stTaskMsg), 10)){
				bret = false;
			}
		}
		osSemaphoreRelease(g_bsIdStorageTaskMsg);
	}

	return true;
}

static void StorageDumpActual(enSD_t enSlotNo, uint32_t SectorNo)
{
	uint32_t u32buf[512u / sizeof(uint32_t)];
	if ((enSlotNo < enUSDHC1) || (enSlotNo > enUSDHC2))
	{
		return;
	}
	memset(u32buf, 0, 512u);
	if (kStatus_Success == SD_ReadBlocks(&s_stSD[enSlotNo], (uint8_t *)u32buf, SectorNo, 1))
	{
		MemDump((uintptr_t)u32buf, 512u);
	}
}

void CmdStorageDump(uint32_t argc, const char *argv[])
{

	if (argc == 2u)
	{
		uint32_t SectorNo;
		SectorNo = atoi(argv[1]);
		StorageDumpActual(enUSDHC1, SectorNo);
	}
	else
	{
		mimic_printf("StorageDump <SectorNo>\r\n");
	}
}
