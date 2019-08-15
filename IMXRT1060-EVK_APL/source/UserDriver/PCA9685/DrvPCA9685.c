/**
 * @file	DrvPCA9685.h
 * @brief	
 * @author Takashi Kashiwagi
 * @date 2019/08/14
 * @version     0.1
 * @details 
 * --
 * License Type <MIT License>
 * --
 * Copyright 2019 Takashi Kashiwagi
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
 * - 2019/08/14: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */
#include "UserDriver/PCA9685/DrvPCA9685.h"
#include "board.h"
#include "fsl_lpi2c.h"
#include "fsl_lpi2c_freertos.h"
#include "mimiclib/source/mimiclib.h"

/*! @brief PCA9685 I2C address. */
#define PCA9685_I2C_ADDRESS (0x40)

#define REG_MODE1	(0x00u)
#define REG_MODE2	(0x01u)

#define REG_ALLCALLADR	(0x05u)
#define REG_LED0_ON_L	(0x06u)
#define REG_LED0_ON_H	(0x06u)
#define REG_LED0_OFF_L	(0x07u)
#define REG_LED0_OFF_H	(0x08u)

#define REG_ALL_LED_ON_L	(0xFAu)
#define REG_ALL_LED_ON_H	(0xFBu)
#define REG_ALL_LED_OFF_L	(0xFCu)
#define REG_ALL_LED_OFF_H	(0xFDu)
#define REG_PRE_SCALE		(0xFEu)

DefALLOCATE_ITCM _Bool DrvPCA9685Init(LPI2C_Type *base)
{
	_Bool bret = false;

	lpi2c_master_transfer_t xfer;

	/* Register Auto-Increment enabled. */
	uint8_t val[4] = {0x10, 0x00, 0x00, 0x00};

	if(LPI2C1 != base) && (LPI2C2 != base) && (LPI2C3 != base) && (LPI2C4 != base))
	{
		mimic_printf("[%s (%d)] EXIT!\r\n", __func__, __LINE__);
		goto _END;
	}

	if(kStatus_Success != BOARD_LPI2C_Send(base, PCA9685_I2C_ADDRESS, REG_MODE1, 1, val, 1))
	{
		mimic_printf("[%s (%d)] EXIT!\r\n", __func__, __LINE__);
		goto _END;
	}

	/* Set MODE2 reset Val */
	val[0] = 0x00;
	if(kStatus_Success != BOARD_LPI2C_Send(base, PCA9685_I2C_ADDRESS, REG_MODE2, 1, val, 1))
	{
		mimic_printf("[%s (%d)] EXIT!\r\n", __func__, __LINE__);
		goto _END;
	}

	/* 1526 Hz */
	val[0] = 0x03u;
	if(kStatus_Success != BOARD_LPI2C_Send(base, PCA9685_I2C_ADDRESS, REG_PRE_SCALE, 1, val, 1))
	{
		mimic_printf("[%s (%d)] EXIT!\r\n", __func__, __LINE__);
		goto _END;
	}

	val[0] = 0x00u;
	val[1] = 0x00u;
	val[2] = 0xFFu;
	val[3] = 0xFFu;
	if(kStatus_Success != BOARD_LPI2C_Send(base, PCA9685_I2C_ADDRESS, REG_ALL_LED_ON_L, 1, val, 4))
	{
		mimic_printf("[%s (%d)] EXIT!\r\n", __func__, __LINE__);
		goto _END;
	}
	
	mimic_printf("[%s (%d)] INIT OK\r\n", __func__, __LINE__);
	bret = true;
_END:
	return bret;
}

DefALLOCATE_ITCM _Bool DrvPCA9685SetPWMVal(LPI2C_Type *base, enPCA9685PortNo_t enPortNo, uint16_t PwmVal)
{
	if((LPI2C1 != base) && (LPI2C2 != base) && (LPI2C3 != base) && (LPI2C4 != base)) ||
		(enPortNo < enPCA9685Port0) ||
		(enPortNo < enPCA9685Port15) ||
		(PwmVal > 4095))
	{
		mimic_printf("[%s (%d)] EXIT!\r\n", __func__, __LINE__);
		goto _END;
	}
_END:
	return bret;
}
