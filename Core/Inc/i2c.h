/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern I2C_HandleTypeDef hi2c1;

extern I2C_HandleTypeDef hi2c3;

/* USER CODE BEGIN Private defines */
#define bq76920 1
#define bq76930 2
#define bq76940 3

typedef struct  {
	int32_t UVP;
	int32_t OVP;
	int8_t OTP;
	int8_t OCC;
	int8_t OCD;
	int8_t SC;
	int8_t undeff1;
	int8_t undeff2;
} regERROR_t;

typedef struct  {
	int32_t UVP;
	int32_t OVP;
	int8_t OTP;
	int8_t OCC;
	int8_t OCD;
	int8_t SC;
	int8_t undeff1;
	int8_t undeff2;
} regWARNING_t;

typedef struct {
	/* I2C Handle for pass by reference*/
	I2C_HandleTypeDef *i2cHandle;
	uint16_t bqAddress;
	uint8_t crc;
	int adcGain; //uV
	int adcOffset;	//mV
	uint16_t teganganCell[15]; //variable menyimpan tegangan per cell
	int teganganPack; //variable menyimpan tegangan Pack
	float suhu[3];
	float arus;
	int jumlahCell;     //jumlah maksimal cell dalam satu IC
	uint8_t idCellMaxVoltage;
	uint8_t idCellMinVoltage;
	int bal_led[15];
	regWARNING_t warningCode;
	regERROR_t errorCode;
} BQ769_t;
/* USER CODE END Private defines */

void MX_I2C1_Init(void);
void MX_I2C3_Init(void);

/* USER CODE BEGIN Prototypes */
void bq769(BQ769_t *BMS, I2C_HandleTypeDef *i2cHandle, unsigned char type, int i2cAddress);  //inisiasi tipe bq dan address
int BQ_Start(BQ769_t *BMS);
int konfirmasiAlamatdanCrc(BQ769_t *BMS);

void BQ_Set_Limit_Voltage(int max, int min);
void BQ_Set_Release_Voltage(int max, int min);

void BQ_Set_Limit_Current(float ishort, float charge, float discharge);
void BQ_Set_Limit_Time(uint32_t timeOCC, uint32_t timeOCD);

void BQ_Set_ResShunt(float Rohm);
void BQ_Set_Limit_Temperature(int max, int mosfet);
void BQ_Set_Release_Temp(int max, int mosfet);

void BQ_Set_Balancing(int minVolt, int diffVolt);

uint16_t BQ_Get_Limit_VoltMax();
uint16_t BQ_Get_Limit_VoltMin();
uint16_t BQ_Get_Release_Volt_Max();
uint16_t BQ_Get_Release_Volt_Min();

float BQ_Get_Limit_Charge();
uint32_t BQ_Get_Waktu_Over_Charge();
float BQ_Get_Limit_Discharge();
uint32_t BQ_Get_Waktu_Over_Discharge();

float BQ_Get_Limit_ShortCircuit();
float BQ_Get_ResShunt();
int BQ_Get_Limit_Temperature();
int BQ_Get_Limit_Temperature_Mosfet();
uint16_t BQ_Get_Release_Temperature();
uint16_t BQ_Get_Release_Temp_Mosfet();

int BQ_Get_Min_Balancing();
int BQ_Get_Diff_Balancing();

void BQ_Update_Voltage(BQ769_t *BMS);
long BQ_Get_Pack_Voltage(BQ769_t *BMS);
uint16_t *BQ_Get_Cell_Voltage(BQ769_t *BMS);
int BQ_Get_Cell_Voltage_Max(BQ769_t *BMS);
int BQ_Get_Cell_Voltage_Min(BQ769_t *BMS);
void BQ_Update_Temperature(BQ769_t *BMS);
float *BQ_Get_Temperature(BQ769_t *BMS);
void BQ_Update_Current(BQ769_t *BMS);
float BQ_Get_Current(BQ769_t *BMS);

void BQ_Control_Balancing(BQ769_t *BMS, float arusBal);
void BQ_Disable_Balancing(BQ769_t *BMS);
int BQ_Power_Off(BQ769_t *BMS);
int BQ_Check_Condition(BQ769_t *BMS);

HAL_StatusTypeDef tulisRegister(I2C_HandleTypeDef *bms, uint16_t i2c_addr, uint8_t reg_addr, uint8_t val, uint8_t crc);
uint8_t bacaRegister(I2C_HandleTypeDef *bms, uint16_t i2c_addr, uint8_t reg_addr);
uint8_t _crc8_ccitt_update(uint8_t inCrc, uint8_t inData);
HAL_StatusTypeDef tulisMemory(I2C_HandleTypeDef *wire, uint16_t i2c_addr, uint16_t mem_addr, uint8_t *data);
uint8_t bacaMemory(I2C_HandleTypeDef *wire, uint16_t i2c_addr, uint16_t mem_addr);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */

