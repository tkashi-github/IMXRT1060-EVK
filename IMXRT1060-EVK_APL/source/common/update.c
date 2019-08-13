/**
 * @file update.c
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
#include "common/update.h"
#include "board.h"
#include "mimiclib/source/mimiclib.h"
#include "ff.h"

#include "SPIFlash/SPIFlash.h"

#define kROM_BIN_TOP	(0x60400000)
#define kBinBufferSize (4 * 1024 * 1024)

uint16_t SlowCrc16(uint16_t sum, uint8_t pu8[], uint32_t len)
{
	for(uint32_t j = 0;j<len;j++)
	{
		uint8_t byte = pu8[j];
		for (uint32_t i = 0; i < 8; ++i)
		{
			uint32_t oSum = sum;
			sum <<= 1;
			if (byte & 0x80){
				sum |= 1;
			}
			if (oSum & 0x8000){
				sum ^= 0x1021;
			}
			byte <<= 1;
		}
	}
	return sum;
}


__STATIC_FORCEINLINE void JumpApplication(uint32_t topOfMainStack, uint32_t AppliAddr)
{
  __ASM volatile ("mov r13, %0" : : "r" (topOfMainStack) : );
  __ASM volatile ("mov r15, %0" : : "r" (AppliAddr) : );
}

_Bool CheckROM(void)
{
	/** var */
	_Bool bret = false;
	uint32_t u32BinSize;

	/** begin */
	u32BinSize = *(volatile uint32_t *)kROM_BIN_TOP;

	mimic_printf("u32BinSize = %lu\r\n", u32BinSize);

	if((u32BinSize > kBinBufferSize) || (u32BinSize == 0))
	{
		mimic_printf("u32BinSize NG\r\n");
		return false;
	}

	{
		/** !!!!(先頭4byteを含める) */
		uint16_t RomCRC16 = SlowCrc16(0xFFFF, (uint8_t *)kROM_BIN_TOP, u32BinSize + 4);
		uint16_t CalcCRC16;

		memcpy(&CalcCRC16, (void*)(kROM_BIN_TOP + 4 + u32BinSize), 2);


		mimic_printf("RomCRC16  = 0x%04X\r\n", RomCRC16);
		mimic_printf("CalcCRC16 = 0x%04X\r\n", CalcCRC16);

		if (RomCRC16 != CalcCRC16)
		{
			mimic_printf("CRC NG\r\n");
		}else{
			mimic_printf("CRC OK\r\n");
			bret = true;
		}
	}

	if(bret){
		memcpy((void*)0x80000000, (void*)(kROM_BIN_TOP + 4), u32BinSize + 2);
		if(memcmp((void*)(kROM_BIN_TOP + 4), (void*)0x80000000, u32BinSize + 2) == 0){
			mimic_printf("memcpy OK\r\n");
			vTaskDelay(100u);
			DisableGlobalIRQ();
			ARM_MPU_Disable();
			SCB_DisableDCache();
			SCB_DisableICache();

			JumpApplication(*(uint32_t*)0x80000000, *(uint32_t*)0x80000004);
		}
	}

	return bret;
}

_Bool CheckBinFile(const TCHAR szBinFile[])
{
	/** var */
	_Bool bret = false;
	uint32_t u32BinSize;
	FIL fBinFile;
	uint8_t *pu8;

	/** begin */
	if(FR_OK != f_open(&fBinFile, szBinFile, FA_READ)){
		return false;
	}
	u32BinSize = f_size(&fBinFile);
	mimic_printf("u32BinSize = %lu\r\n", u32BinSize);

	if((u32BinSize > kBinBufferSize) || (u32BinSize == 0))
	{
		mimic_printf("u32BinSize NG\r\n");
		f_close(&fBinFile);
		return false;
	}
	pu8 = pvPortMalloc(u32BinSize);
	if(pu8 == NULL){
		f_close(&fBinFile);
		return false;
	}
	{
		UINT br;
		if(FR_OK != f_read(&fBinFile, pu8, u32BinSize, &br)){
			f_close(&fBinFile);
			vPortFree(pu8);
			return false;
		}

		if(u32BinSize != br){
			f_close(&fBinFile);
			vPortFree(pu8);
			return false;
		}
	}
	{
		/** !!!!(先頭4byteを含める) */
		uint16_t CalcCRC16 = SlowCrc16(0xFFFF, pu8, u32BinSize-2);
		uint16_t RomCRC16;

		memcpy(&RomCRC16, &pu8[u32BinSize - 2], 2);


		mimic_printf("RomCRC16  = 0x%04X\r\n", RomCRC16);
		mimic_printf("CalcCRC16 = 0x%04X\r\n", CalcCRC16);

		if (RomCRC16 != CalcCRC16)
		{
			mimic_printf("CRC NG\r\n");
		}else{
			mimic_printf("CRC OK\r\n");

			SPIFlashWriteData(DEF_APL_TOP_SECTOR, pu8, u32BinSize);
			bret = true;
			for(uint32_t i=0;i<u32BinSize;i++){

				if(pu8[i] != *(uint8_t*)(kROM_BIN_TOP + i)){
					mimic_printf("VerifyNG (i=%lu)\r\n", i);
					bret = false;
					break;
				}
			}
		}
	}

	f_close(&fBinFile);
	vPortFree(pu8);

	return bret;
}
