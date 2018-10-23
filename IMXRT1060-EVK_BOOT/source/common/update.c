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
#include "common/update.h"
#include "board.h"
#include "mimiclib/mimiclib.h"
#include "ff.h"
#include "CPUFunc.h"

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

	mimiclib_printf("u32BinSize = %lu\r\n", u32BinSize);

	if((u32BinSize > kBinBufferSize) || (u32BinSize == 0))
	{
		mimiclib_printf("u32BinSize NG\r\n");
		return false;
	}

	{
		/** !!!!(先頭4byteを含める) */
		uint16_t RomCRC16 = SlowCrc16(0x1234, (uint8_t *)kROM_BIN_TOP, u32BinSize + 4);	
		uint16_t CalcCRC16;

		memcpy(&CalcCRC16, (void*)(kROM_BIN_TOP + 4 + u32BinSize), 2);


		mimiclib_printf("RomCRC16  = 0x%04X\r\n", RomCRC16);
		mimiclib_printf("CalcCRC16 = 0x%04X\r\n", CalcCRC16);

		if (RomCRC16 != CalcCRC16)
		{
			mimiclib_printf("CRC NG\r\n");
		}else{
			mimiclib_printf("CRC OK\r\n");
			bret = true;
		}
	}

	if(bret){
		memcpy((void*)0x80000000, (void*)(kROM_BIN_TOP + 4), u32BinSize + 2);
		if(memcmp((void*)(kROM_BIN_TOP + 4), (void*)0x80000000, u32BinSize + 2) == 0){
			mimiclib_printf("memcpy OK\r\n");
			vTaskDelay(100u);
			CM7_DisableIRQ();
			ARM_MPU_Disable();
			SCB_DisableDCache();
			SCB_DisableICache();

			JumpApplication(*(uint32_t*)0x80000000, *(uint32_t*)0x80000004);
		}
	}

	return bret;
}
