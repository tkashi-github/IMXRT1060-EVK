/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ffconf.h"     /* FatFs configuration options */
#include "diskio.h"     /* FatFs lower layer API */
#ifdef RAM_DISK_ENABLE
#include "fsl_ram_disk.h"
#endif

#ifdef USB_DISK_ENABLE
#include "fsl_usb_disk.h"
#endif

#ifdef SD_DISK_ENABLE
#include "fsl_sd_disk.h"
#endif

#ifdef MMC_DISK_ENABLE
#include "fsl_mmc_disk.h"
#endif

#ifdef SDSPI_DISK_ENABLE
#include "fsl_sdspi_disk.h"
#endif

#ifdef NAND_DISK_ENABLE
#include "fsl_nand_disk.h"
#endif

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
    switch (pdrv)
    {
		case 0:
		case 1:
			StorageStatus(pdrv);
			break;
        default:
            break;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
    switch (pdrv)
    {
		case 0:
		case 1:
			StorageInitialize(pdrv);
			break;
        default:
            break;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
    switch (pdrv)
    {
		case 0:
		case 1:
			StorageRead(pdrv, buff, sector, count);
			break;
        default:
            break;
    }

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
    switch (pdrv)
    {
		case 0:
		case 1:
			StorageWrite(pdrv, buff, sector, count);
			break;
        default:
            break;
    }
    return RES_PARERR;
}


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
    switch (pdrv)
    {
		case 0:
		case 1:
			StorageIoctl(pdrv, cmd, buff);
			break;
        default:
            break;
    }
    return RES_PARERR;
}

