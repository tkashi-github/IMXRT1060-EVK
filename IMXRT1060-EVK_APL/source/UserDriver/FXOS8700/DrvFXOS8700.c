/**
 * @file DrvFXOS8700.c
 * @brief TODO
 * @author Takashi Kashiwagi
 * @date 2018/11/07
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
 * - 2018/11/07: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */
#include "FXOS8700/DrvFXOS8700.h"
#include "board.h"
#include "mimiclib.h"

#define DEF_COMBO_SENSOR_DEVICE_ADDR	FXOS8700_DEVICE_ADDR_SA_11

typedef struct{
	uint8_t u8Register;
	uint8_t u8Value;
	uint8_t u8Mask;
}stFXOSInitRegisterTable_t;

static const stFXOSInitRegisterTable_t s_stInitTable[]={
	/** CTRL_REG1 (0x2A) register : Enter Stanby Mode */
	{FXOS8700_CTRL_REG1, FXOS8700_CTRL_REG1_ACTIVE_STANDBY_MODE, FXOS8700_CTRL_REG1_ACTIVE_MASK},
	/** CTRL_REG1 (0x2A) register : ODR accelerometer is 12.5Hz */
	{FXOS8700_CTRL_REG1, FXOS8700_CTRL_REG1_DR_SINGLE_12P5_HZ, FXOS8700_CTRL_REG1_DR_MASK},
	/**  M_CTRL_REG1 (0x5B) register : Auto-calibration feature is enabled.  Hybrid mode, both accelerometer and magnetometer sensors are active */
    {FXOS8700_M_CTRL_REG1, FXOS8700_M_CTRL_REG1_M_ACAL_EN | FXOS8700_M_CTRL_REG1_M_HMS_HYBRID_MODE,
                           FXOS8700_M_CTRL_REG1_M_ACAL_MASK | FXOS8700_M_CTRL_REG1_M_HMS_MASK},
 	/**  M_CTRL_REG2 (0x5C) register : hyb_autoinc_mode is Enabled. Auto-calibration feature is enabled.  Automatic magnetic reset is disabled. */
    {FXOS8700_M_CTRL_REG2, FXOS8700_M_CTRL_REG2_M_AUTOINC_HYBRID_MODE | FXOS8700_M_CTRL_REG2_M_RST_CNT_DISABLE,
                           FXOS8700_M_CTRL_REG2_M_AUTOINC_MASK | FXOS8700_M_CTRL_REG2_M_RST_CNT_MASK},
	/** Enter ACTIVE Mode */
	{FXOS8700_CTRL_REG1, FXOS8700_CTRL_REG1_ACTIVE_ACTIVE_MODE, FXOS8700_CTRL_REG1_ACTIVE_MASK},
};


/** TODO */
status_t FXOS8700Init(void)
{
	status_t sts = kStatus_Success;	
	uint8_t u8ReadBuffer[8] = {0};
	uint8_t u8TxTmp[8] = {0};

	sts = BOARD_LPI2C_Receive(LPI2C1, DEF_COMBO_SENSOR_DEVICE_ADDR, FXOS8700_WHO_AM_I, 1, u8ReadBuffer, 1);
	if (kStatus_Success != sts)
	{
		mimic_printf("[%s (%d)] BOARD_LPI2C_Receive NG (sts=%d)\r\n", __func__, __LINE__, sts);
		return kStatus_Fail;
	}
	
	mimic_printf("[%s (%d)] FXOS8700_WHO_AM_I : 0x%02X\r\n", __func__, __LINE__, u8ReadBuffer[0]);

	
    /* Put the device into standby mode so that configuration can be applied.*/
	for(uint32_t i=0;i<(sizeof(s_stInitTable)/sizeof(s_stInitTable[0]));i++){

		sts = BOARD_LPI2C_Receive(LPI2C1, DEF_COMBO_SENSOR_DEVICE_ADDR, s_stInitTable[i].u8Register, 1, u8ReadBuffer, 1);
		if (kStatus_Success != sts)
		{
			mimic_printf("[%s (%d)] BOARD_LPI2C_Receive NG (sts=%d)\r\n", __func__, __LINE__, sts);
			return kStatus_Fail;
		}
		mimic_printf("[%s (%d)] s_stInitTable[%d].u8Register = 0x%02X : 0x%02X\r\n", __func__, __LINE__, i, s_stInitTable[i].u8Register, u8ReadBuffer[0]);
		u8ReadBuffer[0] &= ~s_stInitTable[i].u8Mask;
		u8TxTmp[0] = s_stInitTable[i].u8Value;
		u8TxTmp[0] |= u8ReadBuffer[0];
		sts = BOARD_LPI2C_Send(LPI2C1, DEF_COMBO_SENSOR_DEVICE_ADDR, s_stInitTable[i].u8Register, 1, u8TxTmp, 1);
		if (kStatus_Success != sts)
		{
			mimic_printf("[%s (%d)] BOARD_LPI2C_Receive NG (sts=%d)\r\n", __func__, __LINE__, sts);
			return kStatus_Fail;
		}
 	}

	return sts;
}

/** TODO */
status_t FXOS8700ReadStatus(uint8_t *pu8sts)
{
	status_t sts = kStatus_Success;
	uint8_t u8ReadBuffer[8];

	if(pu8sts == NULL){
		return kStatus_Fail;
	}

	sts = BOARD_LPI2C_Receive(LPI2C1, DEF_COMBO_SENSOR_DEVICE_ADDR, FXOS8700_STATUS, 1, u8ReadBuffer, 1);
	if (kStatus_Success != sts)
	{
		return kStatus_Fail;
	}
	*pu8sts = u8ReadBuffer[0];
	//mimic_printf("[%s (%d)] *pu8sts = 0x%02X\r\n", __func__, __LINE__, *pu8sts);
	*pu8sts &= FXOS8700_DR_STATUS_ZYXDR_MASK;

	return sts;
}
/** TODO */
status_t FXOS8700ReadData(uint16_t pu16Accel[], uint16_t pu16Mag[])
{
	status_t sts = kStatus_Success;
	uint8_t u8ReadBuffer[32];

	if((pu16Accel == NULL) || (pu16Mag == NULL)){
		return kStatus_Fail;
	}

	sts = BOARD_LPI2C_Receive(LPI2C1, DEF_COMBO_SENSOR_DEVICE_ADDR, FXOS8700_OUT_X_MSB, 1, u8ReadBuffer, 12);
	if (kStatus_Success != sts)
	{
		return kStatus_Fail;
	}
	
	pu16Accel[0] = (uint16_t)u8ReadBuffer[0];
	pu16Accel[0] <<= 8;
	pu16Accel[0] |= (uint16_t)u8ReadBuffer[1];
	pu16Accel[1] = (uint16_t)u8ReadBuffer[2];
	pu16Accel[1] <<= 8;
	pu16Accel[1] |= (uint16_t)u8ReadBuffer[3];
	pu16Accel[2] = (uint16_t)u8ReadBuffer[4];
	pu16Accel[2] <<= 8;
	pu16Accel[2] |= (uint16_t)u8ReadBuffer[5];

	pu16Mag[0] = (uint16_t)u8ReadBuffer[6];
	pu16Mag[0] <<= 8;
	pu16Mag[0] |= (uint16_t)u8ReadBuffer[7];
	pu16Mag[1] = (uint16_t)u8ReadBuffer[8];
	pu16Mag[1] <<= 8;
	pu16Mag[1] |= (uint16_t)u8ReadBuffer[9];
	pu16Mag[2] = (uint16_t)u8ReadBuffer[10];
	pu16Mag[2] <<= 8;
	pu16Mag[2] |= (uint16_t)u8ReadBuffer[11];

	return sts;
}

