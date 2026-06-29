/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */
#include "usart.h"
#include "stdint.h"
#include "register.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

unsigned char bqtype;

//batas proteksi defult jika tidak pakai memory
volatile uint16_t batasvoltMax = 3650;
volatile uint16_t batasvoltMin = 2800;
volatile uint16_t batasSuhuMax = 47;
volatile uint16_t batasSuhuMax_Mosfet = 90;
volatile uint16_t batasCharge = 50;
volatile uint16_t batasDischarge = 130;
volatile uint16_t batasShortCircuit = 200;
unsigned long waktuOverCharge = 0;
unsigned long waktuOverDischarge = 0;

uint32_t tlc = 3000;
uint32_t tld = 3000;

volatile uint16_t releaseVoltMin = 2900;
volatile uint16_t releaseVoltMax = 3450;

volatile uint16_t releaseSuhuMax = 42;
volatile uint16_t releaseSuhuMax_Mosfet = 70;

volatile int balancingMinVoltage = 3300; //mv
volatile int balancingVolDiff = 10; //mv
int bal_led[15];
volatile float shuntResistorValue_mOhm = 0.5;

const int VC_HI[15] = {
    0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C,
    0x1E, 0x20, 0x22, 0x24, 0x26, 0x28
};

const int VC_LO[15] = {
    0x0D, 0x0F, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D,
    0x1F, 0x21, 0x23, 0x25, 0x27, 0x29
};

typedef union regSYS_STAT {
  struct
  {
    uint8_t OCD : 1;
    uint8_t SCD : 1;
    uint8_t OV : 1;
    uint8_t UV : 1;
    uint8_t OVRD_ALERT : 1;
    uint8_t DEVICE_XREADY : 1;
    uint8_t WAKE : 1;
    uint8_t CC_READY : 1;
  } bits;
  uint8_t regByte;
} regSYS_STAT_t;

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00503D58;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}
/* I2C3 init function */
void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x00503D58;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PA13     ------> I2C1_SCL
    PA14     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
  else if(i2cHandle->Instance==I2C3)
  {
  /* USER CODE BEGIN I2C3_MspInit 0 */

  /* USER CODE END I2C3_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C3;
    PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**I2C3 GPIO Configuration
    PC8     ------> I2C3_SCL
    PC9     ------> I2C3_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_I2C3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* I2C3 clock enable */
    __HAL_RCC_I2C3_CLK_ENABLE();
  /* USER CODE BEGIN I2C3_MspInit 1 */

  /* USER CODE END I2C3_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PA13     ------> I2C1_SCL
    PA14     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_13);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_14);

  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
  else if(i2cHandle->Instance==I2C3)
  {
  /* USER CODE BEGIN I2C3_MspDeInit 0 */

  /* USER CODE END I2C3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C3_CLK_DISABLE();

    /**I2C3 GPIO Configuration
    PC8     ------> I2C3_SCL
    PC9     ------> I2C3_SDA
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_9);

  /* USER CODE BEGIN I2C3_MspDeInit 1 */

  /* USER CODE END I2C3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/* CODE BEGIN BQ769 */
//inisiasi bq
void bq769(BQ769_t *BMS, I2C_HandleTypeDef *i2cHandle, unsigned char type, int i2cAddress) {
	BMS->i2cHandle = i2cHandle;
	bqtype = type;
	BMS->bqAddress = i2cAddress;
	if (type == bq76920) {
		BMS->jumlahCell = 5;
	} else if (type == bq76930) {
		BMS->jumlahCell = 10;
	} else {
		BMS->jumlahCell = 15;
	}
  //reset nilai inisiasi
	for (int i = 0; i < BMS->jumlahCell; i++) {
		BMS->teganganCell[i] = 0;
	}
}

//done
int BQ_Start(BQ769_t *BMS){
	//cek apakah perlu crc
	if(konfirmasiAlamatdanCrc(BMS)){
		//pengaturan awal untuk BQ769
		uint8_t reset_sys_stat = 0xFF;
		uint8_t reset_sys_ctrl1 = 0x18;
		uint8_t reset_sys_ctrl2 = 0xC0;
		tulisRegister(BMS->i2cHandle, BMS->bqAddress, SYS_STAT, reset_sys_stat, BMS->crc); //reset semua nilai
		tulisRegister(BMS->i2cHandle, BMS->bqAddress, SYS_CTRL1, reset_sys_ctrl1, BMS->crc); //reset semua nilai
		tulisRegister(BMS->i2cHandle, BMS->bqAddress, SYS_CTRL2, reset_sys_ctrl2, BMS->crc); //reset semua nilai

		BMS->adcOffset = (signed int)bacaRegister(BMS->i2cHandle, BMS->bqAddress, ADCOFFSET);
		BMS->adcGain = 365 + (((bacaRegister(BMS->i2cHandle, BMS->bqAddress, ADCGAIN1) & 0x0C) << 1) | ((bacaRegister(BMS->i2cHandle, BMS->bqAddress, ADCGAIN2) & 0xE0) >> 5));
		printMsg_Val(1,"adcOffset: ", "%d\r\n",BMS->adcOffset);
		printMsg_Val(1,"adcGain: ", "%d\r\n", BMS->adcGain);

		return 0;
	} else {
//		printMsg("BMS Error!\r\n");
		return 1;
	}

}
//done
int konfirmasiAlamatdanCrc(BQ769_t *BMS){
	HAL_StatusTypeDef cek;
	uint8_t konfigurasi = 0x19;
	if(BMS->bqAddress == 0x08){
		cek = tulisRegister(BMS->i2cHandle, BMS->bqAddress, CC_CFG, konfigurasi, 1);
		if(cek == HAL_OK){
			printMsg(1,"TX1 OK\r\n");
			BMS->crc = 1;
		} else if(cek == HAL_ERROR){
			printMsg(1,"TX1 ERROR\r\n");
			HAL_I2C_MspDeInit(&hi2c1);
			HAL_Delay(2);
			HAL_I2C_MspInit(&hi2c1);
		} else if(cek == HAL_BUSY){
			printMsg(1,"TX1 BUSY\r\n");
		}
		HAL_Delay(10);
		uint8_t a = bacaRegister(BMS->i2cHandle, BMS->bqAddress, CC_CFG);
		printMsg_Val(1,"nilai a: ", "%d\r\n", a);
		if(bacaRegister(BMS->i2cHandle, BMS->bqAddress, CC_CFG) == 0x19) return 1;
	}

	if(BMS->bqAddress == 0x18){
		cek = tulisRegister(BMS->i2cHandle, BMS->bqAddress, CC_CFG, konfigurasi, 0);
		if(cek == HAL_OK){
			printMsg(1,"TX2 OK\r\n");
			BMS->crc = 1;
		} else if(cek == HAL_ERROR){
			printMsg(1,"TX2 ERROR\r\n");
		} else if(cek == HAL_BUSY){
			printMsg(1,"TX2 BUSY\r\n");
		}
		HAL_Delay(10);
		if(bacaRegister(BMS->i2cHandle, BMS->bqAddress, CC_CFG) == 0x19) return 1;
	}

	return 0;
}

void BQ_Set_Limit_Voltage(int max, int min){
	batasvoltMax = max;
	batasvoltMin = min;
}

void BQ_Set_Release_Voltage(int max, int min){
	releaseVoltMax = max;
	releaseVoltMin = min;
}

void BQ_Set_Limit_Current(float ishort, float charge, float discharge){
	batasShortCircuit = ishort;
	batasCharge = charge;
	batasDischarge = discharge;
}

void BQ_Set_Limit_Time(uint32_t timeOCC, uint32_t timeOCD) {
	tlc = timeOCC;
	tld = timeOCD;
}

void BQ_Set_ResShunt(float Rohm) {
	shuntResistorValue_mOhm = Rohm;
}

void BQ_Set_Limit_Temperature(int max, int mosfet){
	batasSuhuMax = max;
	batasSuhuMax_Mosfet = mosfet;
}

void BQ_Set_Release_Temp(int max, int mosfet){
	releaseSuhuMax = max;
	releaseSuhuMax_Mosfet = mosfet;
}

void BQ_Set_Balancing(int minVolt, int diffVolt){
	balancingMinVoltage = minVolt;
	balancingVolDiff = diffVolt;
}

uint32_t BQ_Get_Waktu_Over_Charge(){
	return tlc;
}

uint32_t BQ_Get_Waktu_Over_Discharge(){
	return tld;
}

uint16_t BQ_Get_Release_Volt_Min(){
	return releaseVoltMin;
}

uint16_t BQ_Get_Release_Volt_Max(){
	return releaseVoltMax;
}

uint16_t BQ_Get_Limit_VoltMax(){
	return batasvoltMax;
}

uint16_t BQ_Get_Limit_VoltMin(){
	return batasvoltMin;
}

float BQ_Get_Limit_Charge(){
	return batasCharge;
}

float BQ_Get_Limit_Discharge(){
	return batasDischarge;
}

float BQ_Get_Limit_ShortCircuit(){
	return batasShortCircuit;
}

float BQ_Get_ResShunt(){
	return shuntResistorValue_mOhm;
}

int BQ_Get_Limit_Temperature(){
	return batasSuhuMax;
}

int BQ_Get_Limit_Temperature_Mosfet(){
	return batasSuhuMax_Mosfet;
}

uint16_t BQ_Get_Release_Temperature(){
	return releaseSuhuMax;
}

uint16_t BQ_Get_Release_Temp_Mosfet(){
	return releaseSuhuMax_Mosfet;
}

int BQ_Get_Min_Balancing(){
	return balancingMinVoltage;
}

int BQ_Get_Diff_Balancing(){
	return balancingVolDiff;
}

long BQ_Get_Pack_Voltage(BQ769_t *BMS){
	return BMS->teganganPack;
}

uint16_t *BQ_Get_Cell_Voltage(BQ769_t *BMS){
	return BMS->teganganCell;
}

int BQ_Get_Cell_Voltage_Max(BQ769_t *BMS){
	return BMS->teganganCell[BMS->idCellMaxVoltage];
}

int BQ_Get_Cell_Voltage_Min(BQ769_t *BMS){
	return BMS->teganganCell[BMS->idCellMinVoltage];
}

void BQ_Update_Voltage(BQ769_t *BMS){
	BQ_Disable_Balancing(BMS);
	HAL_Delay(10); //harusnya rest 12.5 ms sebelum baca tegangan.
	long adcVal;
	int cellTerhubung = 0;
	//update tegangan cell
	for (int i = 0; i < BMS->jumlahCell; i++){
		adcVal = 0;
		uint8_t register1 = bacaRegister(BMS->i2cHandle, BMS->bqAddress, VC_HI[i]) & 0b00111111;
		uint8_t register2 = bacaRegister(BMS->i2cHandle, BMS->bqAddress, VC_LO[i]);
		adcVal = register1 << 8;
		adcVal |= register2;

		BMS->teganganCell[i] = adcVal * BMS->adcGain / 1000 + BMS->adcOffset;

		//menghitung berapa cell yang terhubung
		if(BMS->teganganCell[i] > 500) cellTerhubung++;
		//mencari cell maksimum dan minimum
		if(BMS->teganganCell[i] > BMS->teganganCell[BMS->idCellMaxVoltage]) BMS->idCellMaxVoltage = i;
		if(BMS->teganganCell[i] < BMS->teganganCell[BMS->idCellMinVoltage] && BMS->teganganCell[i] > 1000) BMS->idCellMinVoltage = i;
	}
	// Warning Code
	if (BMS->teganganCell[BMS->idCellMaxVoltage] > batasvoltMax - 200) {
		BMS->warningCode.OVP = BMS->teganganCell[BMS->idCellMaxVoltage];
	} else BMS->warningCode.OVP = 0;

	if (BMS->teganganCell[BMS->idCellMaxVoltage] > batasvoltMax
			&& BMS->teganganCell[BMS->idCellMaxVoltage] < 5000)
		BMS->errorCode.OVP = BMS->teganganCell[BMS->idCellMaxVoltage] | (BMS->idCellMaxVoltage << 16);

	if (BMS->teganganCell[BMS->idCellMaxVoltage] < batasvoltMax - 250
			&& BMS->errorCode.OVP != 0)
		BMS->errorCode.OVP = 0;

	// Warning Code
	if (BMS->teganganCell[BMS->idCellMinVoltage] < batasvoltMin + 200) {
		BMS->warningCode.UVP = BMS->teganganCell[BMS->idCellMinVoltage];
	} else BMS->warningCode.UVP = 0;

	if (BMS->teganganCell[BMS->idCellMinVoltage] < batasvoltMin && BMS->teganganCell[BMS->idCellMinVoltage] > 500) {
		BMS->errorCode.UVP = BMS->teganganCell[BMS->idCellMinVoltage] | (BMS->idCellMinVoltage << 16);
	}

	if(BMS->teganganCell[BMS->idCellMinVoltage] > batasvoltMin + 100 && BMS->errorCode.UVP != 0){
		BMS->errorCode.UVP = 0;
	}

	//update tegangan pack
	long adcValPack = ((bacaRegister(BMS->i2cHandle, BMS->bqAddress, BAT_HI) << 8) | bacaRegister(BMS->i2cHandle, BMS->bqAddress,BAT_LO)) & 0b1111111111111111;
	BMS->teganganPack = 4 * BMS->adcGain * adcValPack / 1000 + (cellTerhubung * BMS->adcOffset);
}

float *BQ_Get_Temperature(BQ769_t *BMS){
	return BMS->suhu;
}

void BQ_Update_Temperature(BQ769_t *BMS){
	float tmp = 0;
	long adcVal[3] = { 0, 0, 0 };
	float vtsx = 0;
	unsigned long rts = 0;
	float suhuMax = 0;
//	int i_suhu;
//	uint8_t sysctrl = bacaRegister(BMS->i2cHandle, bqAddress, SYS_CTRL1);
//	printMsg_Val("sysctrl: ", "%d\r\n", sysctrl);
	HAL_Delay(5);
	adcVal[0] = ((bacaRegister(BMS->i2cHandle, BMS->bqAddress, TS1_HI) & 0b00111111) << 8) | bacaRegister(BMS->i2cHandle, BMS->bqAddress, TS1_LO);
	adcVal[1] = ((bacaRegister(BMS->i2cHandle, BMS->bqAddress, TS2_HI) & 0b00111111) << 8) | bacaRegister(BMS->i2cHandle, BMS->bqAddress, TS2_LO);
	adcVal[2] = ((bacaRegister(BMS->i2cHandle, BMS->bqAddress, TS3_HI) & 0b00111111) << 8) | bacaRegister(BMS->i2cHandle, BMS->bqAddress, TS3_LO);
	for (int i = 0; i < 3; i++) { //cuma pakai 2 suhu
		vtsx = adcVal[i] * 0.382;
		rts = 10000.0 * vtsx / (3300.0 - vtsx);
		tmp = 1.0 / (1.0 / (273.15 + 25) + 1.0 / 3435 * log(rts / 10000.0));
		BMS->suhu[i] = (tmp - 273.15);
		//untuk filter nilai temperature (jika sensor tidak tersambung)
		if(i == 0 && BMS->i2cHandle == &hi2c1)BMS->suhu[i] = BMS->suhu[i] - 18.00;
		if(i > 0 && BMS->i2cHandle == &hi2c3)BMS->suhu[i] = 0;
//		if(BMS->i2cHandle == &hi2c3){
//			printMsg_Val(1,"bat: ","%f\r\n", BMS->suhu[i]);
//		}

		if(BMS->suhu[i] < -50) BMS->suhu[i] = 0;

		//cek maksimum suhu
		if(BMS->suhu[i] > suhuMax){
			suhuMax = BMS->suhu[i];
//			i_suhu = i;
		}
	}
	//proteksi suhu baterai
	if(BMS->i2cHandle == &hi2c1){
		// Warning Code
		if (suhuMax > batasSuhuMax - 5) {
			BMS->warningCode.OTP = (int) suhuMax;
		} else BMS->warningCode.OTP = 0;

		if(suhuMax > batasSuhuMax) {
			BMS->errorCode.OTP = (int)(suhuMax); // | (i_suhu << 7);
//			printMsg_Val(1,"bat: ","%d\r\n", suhuMax);
		}
		if(suhuMax < releaseSuhuMax && BMS->errorCode.OTP != 0) BMS->errorCode.OTP = 0;
	} else if(BMS->i2cHandle == &hi2c3) {
		// Warning Code
		if (suhuMax > batasSuhuMax_Mosfet - 10) {
			BMS->warningCode.OTP = (int) suhuMax;
		} else BMS->warningCode.OTP = 0;

		if(suhuMax > batasSuhuMax_Mosfet) {
			BMS->errorCode.OTP = (int)(suhuMax); // | (i_suhu << 7);
//			printMsg_Val(1,"limit mos: ","%d\r\n", batasSuhuMax_Mosfet);
//			printMsg_Val(1,"mos: ","%f\r\n", suhuMax);
		}
		if(suhuMax < releaseSuhuMax_Mosfet && BMS->errorCode.OTP != 0) BMS->errorCode.OTP = 0;
	}

}

//arus
void BQ_Update_Current(BQ769_t *BMS){
	int16_t adcVal = 0;
	regSYS_STAT_t sys_stat;
	float a,b;
	sys_stat.regByte = bacaRegister(BMS->i2cHandle, BMS->bqAddress, SYS_STAT);
	if(sys_stat.bits.CC_READY == 1){
		adcVal = (bacaRegister(BMS->i2cHandle, BMS->bqAddress, CC_HI) << 8) | bacaRegister(BMS->i2cHandle, BMS->bqAddress, CC_LO);
		float current0 = (adcVal * 8.44 / shuntResistorValue_mOhm);
		//filter nilai arus
		if(current0 > -300 && current0 < 300){
			BMS->arus = 0;
		} else {
			if(current0 > 0) {
		        // Charge
		        a = 1.025f;
		        b = 0.05f;
			} else {
		        // Discharge
		        a = 1.017f;
		        b = 0.0f;
			}
			current0 = (current0 * a + b);
			BMS->arus = current0 / 1000.0; //nilai arus
		}

		//proteksi arus
		if(BMS->arus > batasShortCircuit){
			BMS->errorCode.SC = BMS->arus;
		} else {
//			BMS->errorCode.bits.SC = 0;
			// Warning Code
			if(BMS->arus > (batasCharge - (batasCharge / 10))) {
				BMS->warningCode.OCC = BMS->arus;
			} else BMS->warningCode.OCC = 0;

			// Error Code
			if(BMS->arus > batasCharge){
				//hitung waktu dulu
				if(HAL_GetTick() - waktuOverCharge > tlc) {
					BMS->errorCode.OCC = BMS->arus;
				}
			} else {
				waktuOverCharge = HAL_GetTick(); //refresh waktu
			}

			// Warning Code
			if (BMS-> arus < -(batasDischarge - (batasDischarge / 10))) {
				BMS->warningCode.OCD = BMS->arus;
			} else BMS->warningCode.OCD = 0;

			// Error Code
			if (BMS->arus < -(batasDischarge)){
				//hitung waktu dulu
				if(HAL_GetTick() - waktuOverDischarge > tld) {
					BMS->errorCode.OCD = fabs(BMS->arus);
				}
			} else {
				waktuOverDischarge = HAL_GetTick(); //refresh waktu
			}
		}
	}
	//clear cc
	tulisRegister(BMS->i2cHandle, BMS->bqAddress, SYS_STAT, 0b10000000, BMS->crc);
}

float BQ_Get_Current(BQ769_t *BMS){
	return BMS->arus;
}

void BQ_Control_Balancing(BQ769_t *BMS, float arusBal){
	int selisihMaxMin = BMS->teganganCell[BMS->idCellMaxVoltage] - BMS->teganganCell[BMS->idCellMinVoltage];
	int numSection = BMS->jumlahCell / 5;

	if(BMS->teganganCell[BMS->idCellMaxVoltage] > balancingMinVoltage && selisihMaxMin > balancingVolDiff && arusBal > 0){
		//mulai balancing
		uint8_t balancingFlags;
		//		uint8_t balancingFlagsTarget;
		int section;
		int cellBalFlag[3] = {-1, -1, -1};
		for (section = 0; section < numSection; section++) {
			int cellSectionMax[3] = {0,0,0};
			int i_balance[3] = {-1,-1,-1};
			balancingFlags = 0;
			// buat dapetin maksimum cell untuk masing-masing section

			// cari 3 max
			for (int j = (section * 5); j < ((section + 1) * 5); j++) {
				if (BMS->teganganCell[j] > cellSectionMax[0]) {
					cellSectionMax[2] = cellSectionMax[1];
					i_balance[2] = i_balance[1];
					cellSectionMax[1] = cellSectionMax[0];
					i_balance[1] = i_balance[0];
					cellSectionMax[0] = BMS->teganganCell[j];
					i_balance[0] = j;
				} else if (BMS->teganganCell[j] > cellSectionMax[1]) {
					cellSectionMax[2] = cellSectionMax[1];
					i_balance[2] = i_balance[1];
					cellSectionMax[1] = BMS->teganganCell[j];
					i_balance[1] = j;
				} else if (BMS->teganganCell[j] > cellSectionMax[2]) {
					cellSectionMax[2] = BMS->teganganCell[j];
					i_balance[2] = j;
				}
			}
//			printMsg_Val("section: ", "%d\r\n", section);
//			printMsg_Val("max1: ", "%d\r\n", cellSectionMax[0]);
//			printMsg_Val("max2: ", "%d\r\n", cellSectionMax[1]);
//			printMsg_Val("max3: ", "%d\r\n", cellSectionMax[2]);

			//seleksi masuk syarat balancing atau tidak
			for(int i = 0; i < 3; i++){
				if(cellSectionMax[i] - BMS->teganganCell[BMS->idCellMinVoltage] > balancingVolDiff){
					i_balance[i] = i_balance[i];
				} else i_balance[i] = -1; //kembali ke nilai awal
			}

			//filter supaya tidak bersebelahan.
			if(i_balance[0] != -1){
				cellBalFlag[0] = i_balance[0];
				if (abs(i_balance[0] - i_balance[1]) > 1 && i_balance[1] != -1) {
					cellBalFlag[1] = i_balance[1];
					if(abs(i_balance[0] - i_balance[2] > 1) && abs(i_balance[1] - i_balance[2] > 1) && i_balance[2] != -1) {
						cellBalFlag[2] = i_balance[2];
					}
				}
				else {
					if (abs(i_balance[0] - i_balance[2] > 1) && i_balance[2] != -1) cellBalFlag[1] = i_balance[2];
				}
			}


			//konversi id cell ke id section
			for (int i = 0; i < 3; i++) {
				if(cellBalFlag[i] != -1){
					cellBalFlag[i] = cellBalFlag[i] - (section * 5) + 1;
				}
				if(cellBalFlag[i] > 0) {
					balancingFlags = balancingFlags | (1 << (cellBalFlag[i]-1));
				}
			}
//			printBinary(balancingFlags);
//			printMsg("\r\n");

			for (int i = 0; i < 5; i++) {  // nyalakan indikator balance labview
				// Memeriksa apakah bit ke-i bernilai 1
				if (balancingFlags & (1 << i)) {
					BMS->bal_led[i + (section * 5)] = 1;
//					printMsg_Val("bal led: ","%d\r\n", i + (section * 5));
				} else BMS->bal_led[i + (section * 5)] = 0;
			}
			// set balancing register for this section
			tulisRegister(BMS->i2cHandle, BMS->bqAddress, CELLBAL1 + section, balancingFlags, BMS->crc);
		}
	} else {
		//kalau tidak memenuhi balancing, clear indikator.
		for (int k = 0; k < 15; k++) {
			BMS->bal_led[k] = 0;
		}
	}

}

void BQ_Disable_Balancing(BQ769_t *BMS){
	int numSection = BMS->jumlahCell / 5;
	for (int section = 0; section < numSection; section++) {
		tulisRegister(BMS->i2cHandle, BMS->bqAddress, CELLBAL1 + section, 0x00, BMS->crc);
//	      for (int k = 0; k < 5; k++) {
//	        BMS->bal_led[k] = 0;
//	      }
    }
}

int BQ_Power_Off(BQ769_t *BMS){
	int cek[3];
	cek[0] = tulisRegister(BMS->i2cHandle, BMS->bqAddress, SYS_CTRL1, 0x00, BMS->crc);
	cek[1] = tulisRegister(BMS->i2cHandle, BMS->bqAddress, SYS_CTRL1, 0x01, BMS->crc);
	cek[2] = tulisRegister(BMS->i2cHandle, BMS->bqAddress, SYS_CTRL1, 0x02, BMS->crc);
	return (cek[0] & cek[1] & cek[2]);
}

int BQ_Check_Condition(BQ769_t *BMS) {
	if(bacaRegister(BMS->i2cHandle, BMS->bqAddress, SYS_CTRL1) & 0x03) return 1;
	else return 0;
}

/* CODE END BQ769 */

/* CODE BEGIN MEMORY*/


/* CODE END MEMORY*/


//FUNGSI I2C
HAL_StatusTypeDef tulisRegister(I2C_HandleTypeDef *bms, uint16_t i2c_addr, uint8_t reg_addr, uint8_t val, uint8_t crc){
	uint16_t address = i2c_addr << 1; //address BQ
	uint8_t buf[3] = {};
	buf[0] = reg_addr; //address register
	buf[1] = val; //value
	int len = (crc == 1) ? 3 : 2; //panjang buffer
	if(crc == 1){
		crc = 0;
		crc = _crc8_ccitt_update(crc, (i2c_addr << 1) | 0);
		crc = _crc8_ccitt_update(crc, buf[0]);
		crc = _crc8_ccitt_update(crc, buf[1]);
		buf[2] = crc;
	}
	HAL_StatusTypeDef hasil;
	hasil = HAL_I2C_Master_Transmit(bms, address, buf, len, 100 * len);
	return hasil;
}

uint8_t bacaRegister(I2C_HandleTypeDef *bms, uint16_t i2c_addr, uint8_t reg_addr){
	uint16_t address = i2c_addr << 1; //address BQ
	uint8_t buf[1] = {};
	HAL_I2C_Mem_Read(bms, address, reg_addr, I2C_MEMADD_SIZE_8BIT, buf, 1, 100);
	return buf[0];
}


HAL_StatusTypeDef tulisMemory(I2C_HandleTypeDef *wire, uint16_t i2c_addr, uint16_t mem_addr, uint8_t *data){
	HAL_StatusTypeDef hasil = HAL_I2C_Mem_Write(wire, i2c_addr << 1, mem_addr, 2, &data[0], 1, 200);
	return hasil;
}

uint8_t bacaMemory(I2C_HandleTypeDef *wire, uint16_t i2c_addr, uint16_t mem_addr){
	uint8_t data[1];
	HAL_StatusTypeDef hasil = HAL_I2C_Mem_Read(wire, i2c_addr << 1, mem_addr, 2, &data[0], 1, 200);
	if(hasil != HAL_OK) printMsg(1,"HAL read error \r\n");
	return data[0];
}


// CRC calculation taken from LibreSolar mbed firmware
uint8_t _crc8_ccitt_update(uint8_t inCrc, uint8_t inData) {
  uint8_t i;
  uint8_t data;
  data = inCrc ^ inData;

  for (i = 0; i < 8; i++) {
    if ((data & 0x80) != 0) {
      data <<= 1;
      data ^= 0x07;
    } else data <<= 1;
  }

  return data;
}
/* USER CODE END 1 */
