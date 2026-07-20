/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "fdcan.h"
#include "i2c.h"
#include "iwdg.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include "string.h"
#include <stdio.h>
#include "stdarg.h"
#include <stdlib.h>
#include "battery_state.h"
#include "ble.h"
#include "flash.h"
#include "pcf8563.h"
#include "stdlib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ON GPIO_PIN_SET
#define OFF GPIO_PIN_RESET
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
BQ769_t BMS;
BQ769_t BMS2;
allERROR_t errorCode;
allWARNING_t warningCode;
bluetooth_data_t bleData;
flash_data_t savedData;
flash_data_t inputData;
flash_data_t savedData2;
flash_data_t inputData2;

can_t CAN;
int16_t errorValue[ERROR_COUNT];

uint8_t changeMenu = 0;
uint8_t debug_select = 1;
uint8_t debug_uart_select = 1;
uint8_t fdcan_select = 1;

//variabel sensor
long voltage_pack;
uint16_t *voltage_cell_1, *voltage_cell_2;
float arus;
float *suhu, *suhu2;

unsigned long delayProteksi = 0;

unsigned long tick_zeroCurrent = 0;
unsigned long tick_UVP = 0;
unsigned long tick_Standby = 0;
unsigned long tick_Standby_Precharge = 0;
unsigned long tick_Buzzer = 0;
unsigned long tick_Precharge = 0;
unsigned long tick_SC = 0;
unsigned long tick_Precharge_Valid = 0;


int CD,CD_limit, LD, LD_limit;

uint8_t cek_log;
uint8_t cek_save;

uint32_t serialnumber;
const char *firmware_ver = "ELBI_EV_V0.6";
uint32_t socFlashMem;
uint32_t count_Error_System;

/*
 * [0] = proteksi
 * [1] = balancing
 * [2] =
 * [3] =
 * [4] =
*/
//uint8_t forceControl[5] = {0,0,0,0,0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void modbus_send(uint8_t frame);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	BMS.i2cHandle = &hi2c1;
	BMS2.i2cHandle = &hi2c3;
	bq769(&BMS, BMS.i2cHandle, bq76940, 0x08);
	bq769(&BMS2, BMS2.i2cHandle, bq76930, 0x08);
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_FDCAN1_Init();
  MX_I2C1_Init();
  MX_I2C3_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM1_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */

  //  pcf8563_SetTime(hi2c, hours, minute, day, month, year);

  errorCode.bits.precharge = 1; // kondisi default off (dari tombol)
  CD_limit = 28;
  LD_limit = 7;
  /* BQ76940 CODE BEGIN*/
  printMsg(1,"<INIT>,PRORGRAM START\r\n");

    // tone ketika BMS hidup.
	for (int i = 0; i < 5; i++) {
	    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
	    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 1333);  // 50% duty
		HAL_Delay(100);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
		HAL_Delay(100);
	}

	pin_Dcdc(ON);
	pin_Boot(ON);

  while (BQ_Start(&BMS) == 1) {
	  printMsg(1,"BQ_Error,No Battery Connected!\r\n");
	  printMsg(2,"BQ_Error,No Battery Connected!\r\n");
	  pin_Led(ON);
	  HAL_Delay(50);
	  pin_Led(OFF);
	  HAL_Delay(200);
  }
  printMsg(1,"<INIT>,BQ76940 OKE \r\n");
  pin_Boot(OFF);
  HAL_Delay(500);
  /* BQ76940 CODE END*/

  /* BQ76930 CODE BEGIN*/
  pin_Boot(ON);
  while (BQ_Start(&BMS2) == 1) {
	  printMsg(1,"BQ_Error,No Battery Connected!\r\n");
	  printMsg(2,"BQ_Error,No Battery Connected!\r\n");
	  pin_Led(ON);
	  HAL_Delay(50);
	  pin_Led(OFF);
	  HAL_Delay(200);
  }
  printMsg(1,"<INIT>,BQ76930 OKE \r\n");
  pin_Boot(OFF);
  HAL_Delay(1000);
  /* BQ76930 CODE END*/

  bluetooth_begin(&huart3);
  printMsg(1, "<INIT>,Bluetooth OKE \r\n");

  // inisiasi interupt serial
	serial_Interrupt_Init();
	memset(operCommand, 0, 100);
	memset(UART_BUFFER, 0, 200);
	printMsg(1,"<INIT>, Serial OKE \r\n");

	lastCommTick = HAL_GetTick();


	  // Flash Memory
	  HAL_SPI_Init(&hspi1);
	  flash_begin(&hspi1);
	  flash_setCsPin(SPI_CS_GPIO_Port, SPI_CS_Pin);

	  uint32_t flashId = flash_readId();
	  if(flashId == 0x000000){
		  printMsg(1,"<INIT>,FAIL TO READ FLASH MEMORY\r\n"); // Failed to connect to flash memory
	  } else {
		  printMsg_Val(1,"<INIT>,FLASH MEMORY OKE: ","0x%lx\r\n", flashId); // Print flash memory id
	  }

		// Get BMS Serial Number
		serialnumber = *(uint32_t*)0x0801F810;
		printMsg_Val(1,"Serial Number: ", "0x%08lX\r\n", serialnumber);

		socFlashMem = *(uint32_t*)0x0801F800;
		printMsg_Val(1,"memory: ", "%d\r\n", socFlashMem);

		count_Error_System = *(uint32_t*)0x0801F820;
		printMsg_Val(1, "<INIT>,Error System: ", "%d\r\n", count_Error_System);

		// Write Counting to Flash Memory
		printMsg(1, "<INIT>,Error Count System++ ==> Save\r\n");
		count_Error_System++;
		BMS_Save_Mem_Flash();
		count_Error_System = *(uint32_t*)0x0801F820;
		printMsg_Val(1, "<INIT>,count_Error_System:", "%d\r\n", count_Error_System);

		// select mode soc
		BMS_SoC_Mem_Init(socFlashMem);

	  socFlashMem = *(uint32_t*)0x0801F800;
	  printMsg_Val(1,"memory: ", "%d\r\n", socFlashMem);

	//start FDCAN
	if(HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
		printMsg(1,"<INIT>,FDCAN START FAIL \r\n");
	}

	if(HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_TX_COMPLETE | FDCAN_IT_TX_ABORT_COMPLETE, 0) != HAL_OK) {
		printMsg(1,"<INIT>,FDCAN INTERRUPT RX FAIL \r\n");
	}

	printMsg(1, "<INIT>,FDCAN OKE \r\n");

	printMsg(1, "<INIT>,BMS Init OKE \r\n");

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void BMS_SoC_Mem_Init(uint32_t memFlag){
	if (memFlag != -1) {
		flash_get_data(&savedData, 0);
		flash_get_data(&savedData2, 4096);

		print_Memory_Parameter();
		inputData = savedData;
		inputData2 = savedData2;

		bat.Ah = (float) (savedData.Ah) / 100.0;
		printMsg_Val(1, "bat.Ah: ", "%.2f\r\n", bat.Ah);
		bat.Ah_max = (float) (savedData.AhMax) / 100.0;
		bat.Ah_capacity = (float) (savedData.AhCapacity) / 100.0;
		bat.sohFlag = savedData.SohFlag;
		bat.cycle = savedData.Cycle;

		soc_init(2);

		printMsg_Val(1, "bat.cc: ", "%.2f\r\n", bat.cc);
		printMsg_Val(1, "bat.ccMax: ", "%.2f\r\n", bat.cc_max);

		BQ_Set_Limit_Current(savedData.ShortCircuit / 100.0,
				savedData.ChargeMax / 100.0, savedData.DischargeMax / 100.0);
		BQ_Set_Limit_Voltage(savedData.VMax, savedData.VMin);
		BQ_Set_Limit_Temperature(savedData.TMax / 100, savedData.TMosfet / 100);
		BQ_Set_Balancing(savedData.BalancingMin, savedData.BalancingDiff);
		BQ_Set_ResShunt(savedData.RShunt / 1000.0);

		BQ_Set_Release_Voltage(savedData2.recovery_ovp, savedData2.recovery_uvp);
		BQ_Set_Release_Temp(savedData2.recovery_OT / 100.0, savedData2.recovery_OTM / 100.0);
		BQ_Set_Limit_Time(savedData2.delayTime_occ, savedData2.delayTime_ocd);

	} else { // memory kosong
		printMsg(1, "SOC PERTAMA KALI \r\n");
		BQ_Update_Voltage(&BMS);
		BQ_Update_Voltage(&BMS2);

		soc_init(1);

		inputData.AhCapacity = (int) (bat.Ah_capacity) * 100;
		inputData.AhMax = (int) (bat.Ah_max) * 100;
		inputData.Ah = (int) (bat.Ah) * 100;

		parameter_SetDefault();

		flash_save_data(inputData, 0);
		flash_save_data(inputData2, 4096);
		//save flash memory
		HAL_FLASH_Unlock();
		FLASH_EraseInitTypeDef eraseInit;
		uint32_t pageError = 0;

		eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
		eraseInit.Page = 63;
		eraseInit.NbPages = 1;
		eraseInit.Banks = FLASH_BANK_1;

		while (HAL_FLASHEx_Erase(&eraseInit, &pageError) != HAL_OK) {
			printMsg(1, "error Flash erase\r\n");
		}

		// write soc init
		uint64_t data = 1;
		while (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, 0x0801F800, data)
				!= HAL_OK) {
			// error handling
			printMsg(1, "error flash program \r\n");
		}

		// untuk serial number
		uint64_t data2 = serialnumber;
		while (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, 0x0801F810,
				data2) != HAL_OK) {
			// error handling
			printMsg(1, "error flash program2 \r\n");
		}

		HAL_FLASH_Lock();
	}
}

void BMS_Save_Mem_Flash(){
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef eraseInit;
	uint32_t pageError = 0;

	eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInit.Page = 63;
	eraseInit.NbPages = 1;
	eraseInit.Banks = FLASH_BANK_1;

	while (HAL_FLASHEx_Erase(&eraseInit, &pageError) != HAL_OK) {
		printMsg(1, "error Flash erase\r\n");
	}

	// write soc init
	uint64_t data = socFlashMem;
	while (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, 0x0801F800, data)
			!= HAL_OK) {
		// error handling
		printMsg(1, "error flash program \r\n");
	}

	// untuk serial number
	uint64_t data2 = serialnumber;
	while (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, 0x0801F810,
			data2) != HAL_OK) {
		// error handling
		printMsg(1, "error flash program2 \r\n");
	}

	// untuk count Error
	uint64_t data3 = count_Error_System;
	while (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, 0x0801F820,
			data3) != HAL_OK) {
		// error handling
		printMsg(1, "error flash program3 \r\n");
	}


	HAL_FLASH_Lock();
}

void BMS_UpdateSystem(){
	BQ_Update_Current(&BMS);
	arus = BQ_Get_Current(&BMS);
	BQ_Update_Voltage(&BMS);
	voltage_cell_1 = BQ_Get_Cell_Voltage(&BMS);
	BQ_Update_Voltage(&BMS2);
 	voltage_cell_2 = BQ_Get_Cell_Voltage(&BMS2);
 	voltage_pack = BQ_Get_Pack_Voltage(&BMS) + BQ_Get_Pack_Voltage(&BMS2);
 	BMS_BalancingHandler();

 	BQ_Update_Temperature(&BMS);
	suhu = BQ_Get_Temperature(&BMS);
	BQ_Update_Temperature(&BMS2);
	suhu2 = BQ_Get_Temperature(&BMS2);

	protection_CollectError();
	//return dari fungsi ini buat trigger memori log
	precharge_Loop(arus);
	if (protection_Handler() == 1 && cek_save == 0) {
		cek_save = 1;
	}

	FDCAN_TransmitData(); // CAN data only.
}

void BMS_BalancingHandler() {
	//jika tegangan BQ1 dan BQ2 seimbang
	if (forceControl[1] == 1) {
		BQ_Disable_Balancing(&BMS);
		BQ_Disable_Balancing(&BMS2);
		for (int k = 0; k < 15; k++) {
			BMS.bal_led[k] = 0;
			BMS2.bal_led[k] = 0;
		}
	} else {
		int selisih = 0;
		selisih = abs(BQ_Get_Cell_Voltage_Max(&BMS) - BQ_Get_Cell_Voltage_Max(&BMS2));
		//jika tegangan antara BQ1 atau BQ2 beda jauh
		if (selisih >= 50) {
			if (BQ_Get_Cell_Voltage_Max(&BMS) > BQ_Get_Cell_Voltage_Max(&BMS2)) {
				BQ_Control_Balancing(&BMS, 1);
				for (int k = 0; k < 15; k++) {
					BMS2.bal_led[k] = 0;
				}
			} else {
				BQ_Control_Balancing(&BMS2, 1);
				for (int k = 0; k < 15; k++) {
					BMS.bal_led[k] = 0;
				}
			}
		} else {
			BQ_Control_Balancing(&BMS, arus);
			BQ_Control_Balancing(&BMS2, arus);
		}
	}
}

int BMS_Cell_Max(){
	if(BQ_Get_Cell_Voltage_Max(&BMS) > BQ_Get_Cell_Voltage_Max(&BMS2)) return BQ_Get_Cell_Voltage_Max(&BMS);
	else return BQ_Get_Cell_Voltage_Max(&BMS2);
}

int BMS_Cell_Min(){
	if(BQ_Get_Cell_Voltage_Min(&BMS) < BQ_Get_Cell_Voltage_Min(&BMS2)) return BQ_Get_Cell_Voltage_Min(&BMS);
	else return BQ_Get_Cell_Voltage_Min(&BMS2);
}

int BMS_sleep_Condition(){
	int ret = 0;
	//SLEEP BASED UVP
	if (errorCode.bits.UVP) {
		//mulai hitung selama 5 menit
		if (HAL_GetTick() - tick_UVP > 300000) {
			ret = 1;
		}
	} else
		tick_UVP = HAL_GetTick();

	//SLEEP BASED TINE
//	if (arus == 0) {
		//jika arus 0 selama 3 hari
		// pak rtc aja.
//	}
	return ret;
}

void BMS_Sleep_Handler(int cek) {
	//matikan BMS
	if (cek == 1) {
		inputData.Ah = bat.Ah * 100;
		flash_save_data(inputData, 0);
		BQ_Power_Off(&BMS);
		BQ_Power_Off(&BMS2);
		printMsg(1,"IC BMS DIMATIKAN\r\n");

		if (BQ_Check_Condition(&BMS) && BQ_Check_Condition(&BMS2)) {
			//matikan BMS controller
			printMsg(1,"REGISTER SYS CTRL IS OFF\r\n");
			printMsg(1,"MATIKAN DCDC\r\n");
			pin_Charge(OFF);
			pin_Discharge(OFF);
			pin_Dcdc(OFF);
		} else {
			printMsg(1,"REGISTER BELUM OFF, MATIKAN IC BQ LAGI!! \r\n");
			BQ_Power_Off(&BMS);
			BQ_Power_Off(&BMS2);
		}
	}
}

void BMS_BuzzerHandler(){
	if(errorCode.bits.SC || errorCode.bits.OTP) {
		if(HAL_GetTick() - tick_Buzzer > 2000){
			tick_Buzzer = HAL_GetTick();
		} else if (HAL_GetTick() - tick_Buzzer > 150) {
			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
		} else if (HAL_GetTick() - tick_Buzzer < 150) {
		    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
		    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 1333);  // 50% duty
		}
	}
}

void BMS_BuzzerBeep(){
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 1333);  // 50% duty
	HAL_Delay(200);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
}

void tombol_matikan(int set){
	if (set == 0) {
		errorCode.bits.precharge = 0;
//		printMsg(1, "NYALA \r\n");
	} else {
		errorCode.bits.precharge = 1;
		if(errorCode.bits.SC != 0) terjadiSC = 0; // Reset Flag Short Circuit
	}
}

int protection_Handler() {
	if (errorCode.bits.angsuran == 1) {
		pin_Charge(OFF);
		pin_Discharge(OFF);
	} else {
		if (errorCode.bits.precharge) {
			//kondisi precharge
			delayProteksi = HAL_GetTick();
			pin_Charge(OFF);
			pin_Discharge(OFF);
			cek_log = 0;
			cek_save = 0;
		} else if (errorCode.regByte == 0) {
			delayProteksi = HAL_GetTick();
			pin_Charge(ON);
			pin_Discharge(ON);
			cek_log = 0;
			cek_save = 0;
		} else if (errorCode.bits.SC) {
			delayProteksi = HAL_GetTick();
			pin_Charge(OFF);
			pin_Discharge(OFF);
			cek_log = (cek_log == 0) ? 1 : cek_log; //trigger untuk log error
		} else if (errorCode.bits.OTP) {
			delayProteksi = HAL_GetTick();
			pin_Charge(OFF);
			pin_Discharge(OFF);
			cek_log = (cek_log == 0) ? 1 : cek_log;
		} else if (errorCode.bits.UVP) {
			delayProteksi = HAL_GetTick();
			//kasih skema untuk sleep jika berlangsung lama ==> 5 menit
			pin_Discharge(OFF);
			pin_Charge(ON);
//			if(HAL_GetTick - delayProteksi > 90000) sleepBMS();
		} else if (errorCode.bits.OVP) {
			delayProteksi = HAL_GetTick();
			pin_Charge(OFF);
			pin_Discharge(ON);
		} else if (errorCode.bits.OCC) {
			pin_Charge(OFF);
			pin_Discharge(ON);
			cek_log = (cek_log == 0) ? 1 : cek_log;
		} else if (errorCode.bits.OCD) {
			pin_Discharge(OFF);
			pin_Charge(ON);
			cek_log = (cek_log == 0) ? 1 : cek_log;
		}

		protection_HoldError();
	}
	return cek_log;
}

int time_ZeroCurrent(float arus){
	int ret = 0;
	if (arus == 0){
		if(HAL_GetTick() - tick_zeroCurrent > 300000){ // 5 menit
//			tick_zeroCurrent = HAL_GetTick();
			ret = 1;
		}
	} else {
		tick_zeroCurrent = HAL_GetTick();
	}
	return ret;
}

void protection_CollectError(){
	//tentuin ada UVP atau tidak
	errorCode.bits.UVP = (BMS.errorCode.UVP != 0 || BMS2.errorCode.UVP != 0) ? 1 : 0;
	errorValue[ERR_UVP] = ((BMS.errorCode.UVP & 0xFFFF) < (BMS2.errorCode.UVP & 0xFFFF)) ? BMS.errorCode.UVP : BMS2.errorCode.UVP;
	warningCode.bits.UVP = (BMS.warningCode.UVP != 0 || BMS2.warningCode.UVP != 0) ? 1 : 0;
	//tentuin ada OVP atau tidak
	errorCode.bits.OVP = (BMS.errorCode.OVP != 0 || BMS2.errorCode.OVP != 0) ? 1 : 0;
	errorValue[ERR_OVP] = ((BMS.errorCode.OVP & 0xFFFF) > (BMS2.errorCode.OVP & 0xFFFF)) ? BMS.errorCode.OVP : BMS2.errorCode.OVP;
	warningCode.bits.OVP = (BMS.warningCode.OVP != 0 || BMS2.warningCode.OVP != 0) ? 1 : 0;
	//tentuin ada OTP atau tidak
	errorCode.bits.OTP = (BMS.errorCode.OTP != 0 || BMS2.errorCode.OTP != 0) ? 1 : 0;
	errorValue[ERR_OTP] = (BMS.errorCode.OTP > BMS2.errorCode.OTP) ? BMS.errorCode.OTP : BMS2.errorCode.OTP;
	warningCode.bits.OTP = (BMS.warningCode.OTP != 0 || BMS2.warningCode.OTP != 0) ? 1 : 0;
	//tentuin ada SC atau tidak
	errorCode.bits.SC = terjadiSC;//(BMS.errorCode.SC != 0) ? 1 : 0;
	errorValue[ERR_SC] = BMS.errorCode.SC;
	//tentuin ada OC atau tidak
	if(BMS.errorCode.OCC != 0) {
		errorCode.bits.OCC = 1;
		errorValue[ERR_OCC] = BMS.errorCode.OCC;
	} else {
		errorCode.bits.OCC = 0;
	}
	warningCode.bits.OCC = (BMS.warningCode.OCC != 0 ) ? 1 : 0;
	//tentuin ada OD atau tidak
	errorCode.bits.OCD = (BMS.errorCode.OCD != 0) ? 1 : 0;
	errorValue[ERR_OCD] = BMS.errorCode.OCD;
	warningCode.bits.OCD = (BMS.warningCode.OCD != 0) ? 1 : 0;

	// cek cicilan
	// jika waktu sudah lewat dan arus 0 selama 5 menit.

//	errorCode.bits.angsuran = forceControl[0] * time_ZeroCurrent(arus);
}

void protection_HoldError(){
	if(HAL_GetTick() - delayProteksi > 10000) {
		printMsg(1,"error Release \r\n");
		BMS.errorCode.OCC = 0;
		BMS.errorCode.OCD = 0;
	}
}

void protection_LogError(){
	RTC_TimeDate now;
	pcf8563_GetTime(&hi2c3, &now);
	flash_get_data(&savedData, 0);
	uint32_t time_err; // buffer err
	time_err = (now.year * 10000) + (now.month * 100) + now.day;

	//shift error.
	for (int i = 1; i < 10; i++) {
		inputData.ErrorTime[i - 1] = savedData.ErrorTime[i];
		inputData.ErrorCode[i - 1] = savedData.ErrorCode[i];
		inputData.ErrorValue[i - 1] = savedData.ErrorValue[i];
	}
	inputData.ErrorTime[9] = time_err;
	printMsg_Val(1,"time_err:", "%ld\r\n", inputData.ErrorTime[9]);

	inputData.ErrorCode[9] = errorCode.regByte;

	if (errorCode.regByte & 0b00000001)
		inputData.ErrorValue[9] = errorValue[ERR_UVP];
	if (errorCode.regByte & 0b00000010)
		inputData.ErrorValue[9] = errorValue[ERR_OVP];
	if (errorCode.regByte & 0b00000100)
		inputData.ErrorValue[9] = errorValue[ERR_OTP];
	if (errorCode.regByte & 0b00001000)
		inputData.ErrorValue[9] = errorValue[ERR_OCC];
	if (errorCode.regByte & 0b00010000)
		inputData.ErrorValue[9] = errorValue[ERR_OCD];
	if (errorCode.regByte & 0b00100000)
		inputData.ErrorValue[9] = errorValue[ERR_SC];

	flash_save_data(inputData, 0);
}

void proteksi_ErrorDescription(uint8_t error){
	if (error == 4){
		printMsg(uart_channel_flag,",Temperature-Fault");
	} else if(error == 8){
		printMsg(uart_channel_flag,",Over-Current-Charge");
	} else if (error == 16) {
		printMsg(uart_channel_flag,",Over-Current-Discharge");
	} else if (error == 32) {
		printMsg(uart_channel_flag,",Short-Circuit");
	} else {
		printMsg(uart_channel_flag,",Undeff-Error");
	}
}


void precharge_Loop(float arusPre) {
	if (errorCode.bits.SC != 0 /*|| errorCode.bits.precharge != 0*/) {
		// tunggu 1 detik dulu, biar CD sempet reset.
		if(HAL_GetTick() - tick_SC > 2000){
			CD = precharge_Get_CD() * 10;
			precharge_CheckCondition(CD, 0);
		}
	} else {
		// baca load detection
		// baca charge detection
		tick_SC = HAL_GetTick();
		LD = precharge_Get_LD() * 10;
		CD = precharge_Get_CD() * 10;
		precharge_CheckCondition(CD, LD);
	}

	if (arusPre != 0) tick_Standby_Precharge = HAL_GetTick(); //reset timer

	// jika standby lebih dari 5 menit, kembali ke mode sebelum precharge
	if (HAL_GetTick() - tick_Standby_Precharge > 60000) {
//		errorCode.bits.precharge = 1; //precharge matikan
		tick_Standby_Precharge = HAL_GetTick();
	}
}

void precharge_CheckCondition(int cd, int ld) {
	if (errorCode.bits.precharge != 0 || errorCode.bits.SC != 0) {
		if (CD > CD_limit || LD > LD_limit) {
			//precharge
			if (HAL_GetTick() - tick_Precharge_Valid > 2000) {
//				printMsg_Val(1,"CD: ", "%d\r\n", CD);
//				printMsg_Val(1,"LD: ", "%d\r\n", LD);
				tick_Precharge = HAL_GetTick();
				precharge_Handler();
			}
		} else tick_Precharge_Valid = HAL_GetTick();
	}
}

float precharge_Get_CD(){
	//mulai konversi
	HAL_ADC_Start(&hadc2);
	HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);

	//dapatkan nilai ADC
	uint32_t adc_value = HAL_ADC_GetValue(&hadc2);

	//matikan ADC jika tidak diperlukan terus menerus
	HAL_ADC_Stop(&hadc2);

	//konversi nilai ADC
	float CD = adc_value * (3.3 / 4096);
	return CD;
}

float precharge_Get_LD(){
	// mulai konversi
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);

	// dapatkan nilai ADC
	uint32_t adc_value = HAL_ADC_GetValue(&hadc1);

	//matikan ADC jika tidak diperlukan terus menerus
	HAL_ADC_Stop(&hadc1);

	// konversi nilai ADC
	float LD = adc_value * (3.3 / 4096);
	return LD;
}

void precharge_Handler(){
	pin_Precharge(ON);
	printMsg(1,"precharge Mulai \r\n");
	while (HAL_GetTick() - tick_Precharge < 300) {
		// baca arus
		BQ_Update_Current(&BMS);
		float arus = BQ_Get_Current(&BMS);
		if(arus < -0.4) {
			// cut
			errorCode.bits.precharge = 1;
			terjadiSC = 1;
			printMsg(1,"BYPASS\r\n");
			// do nothing
		} else {
			// lanjut
			errorCode.bits.precharge = 0;
			terjadiSC = 0;
		}
	}
	printMsg(1,"precharge Selesai \r\n");
	pin_Precharge(OFF);
}

/*
 * Fungsi BMS Command via serial
 * dalam keadaan BMS run normal
 * */
void command_SerialHandler(){
//	parse_first_Command(operCommand);
//	int *val = 0;
	if(operCommand[0] != 0){
		if(operCommand[0] == 'm'){
			// pindah task
			get_Saved_Parameter();
			changeMenu = 1;
			BMS_BuzzerBeep();
		} else if (operCommand[0] == 'p') {
//			forceControl[0] = (forceControl[0] == 1) ? 0 : 1; //mati nyala precharge BMS
		} else if (operCommand[0] == 'd') {
			debug_select++;
			if(debug_select > 3) debug_select = 1;
		} else if (operCommand[0] == 'f') {
			debug_select = 2;
		} else if (operCommand[0] == 'v') {
			fdcan_select++;
			if (fdcan_select > 2) {
				fdcan_select = 1;
			}
		} else if (operCommand[0] == 'r') {
			printMsg(1,"Reset SOC: \r\n");
			flag_reset_soc = 1;
			bat.cc = soc_Reset(BMS_Cell_Min());
			bat.Ah = bat.cc * 2.77777778e-7f;
			bat.soc = bat.cc / (bat.cc_max / 1.0e2f);
			HAL_Delay(100);
			inputData.Ah = (int)(bat.Ah * 100);
			flash_save_data(inputData, 0);
			HAL_Delay(50);
			flag_reset_soc = 0;
			soc_init(2);
		} else if (operCommand[0] == 'b') {
			//matikan balancing paksa
			forceControl[1] = (forceControl[1] == 1) ? 0 : 1; //mati nyala baalncing
		} else if (operCommand[0] == 'q') {
		    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
		    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 1333);  // 50% duty
		} else if (operCommand[0] == 'a') {
			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
		} else if (operCommand[0] == 'w') {
			print_SetParameter();
		} else if (operCommand[0] == 'g') {
//			Print_Memory_Log(uart_channel_flag);
			inputData.AhCapacity = (int) (bat.Ah_capacity) * 100;
			inputData.AhMax = (int) (bat.Ah_max) * 100;
			inputData.Ah = (int) (bat.Ah) * 100;

			parameter_SetDefault();

			flash_save_data(inputData, 0);
			flash_save_data(inputData2, 4096);
		} else if (operCommand[0] == 's') {
			// save flash memory
			socFlashMem = -1;
			BMS_Save_Mem_Flash();
			socFlashMem = *(uint32_t*)0x0801F800;
		} else if (operCommand[0] == 'n') {
			// ambil serial number
			char serial_str[20];
			strcpy(serial_str, &operCommand[1]);

			uint32_t serial_num = 0;
			sscanf(serial_str, "%lx", &serial_num);

			// write Serial Number
			HAL_FLASH_Unlock();
			FLASH_EraseInitTypeDef eraseInit;
			uint32_t pageError = 0;

			eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
			eraseInit.Page = 63;
			eraseInit.NbPages = 1;
			eraseInit.Banks = FLASH_BANK_1;

			if (HAL_FLASHEx_Erase(&eraseInit, &pageError) != HAL_OK) {
				printMsg(1,"error Flash erase\r\n");
			}

			uint32_t data = serial_num;
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, 0x0801F810,
					data) != HAL_OK) {
				// error handling
				printMsg(1,"error flash program \r\n");
			}

			uint64_t data2 = socFlashMem;
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, 0x0801F800,
					data2) != HAL_OK) {
				// error handling
				printMsg(1,"error flash program \r\n");
			}

			HAL_FLASH_Lock();

			serialnumber = *(uint32_t*)0x0801F810;
			printMsg_Val(1,"Serial Number: ", "0x%08lX\r\n", serialnumber);

		} else if (operCommand[0] == 'h') {
			print_Memory_Parameter();
		} else if (operCommand[0] == 'j') {
			printMsg(1,"coba save log\r\n");
			protection_LogError();
		} else if (operCommand[0] == 'k') {
			// reset log error
			for(int i = 0; i < 10; i++){
				inputData.ErrorCode[i] = 0;
				inputData.ErrorValue[i] = 0;
				inputData.ErrorTime[i] = 0;
			}
			flash_save_data(inputData, 0);
		} else if (operCommand[0] == '4') {
			//kirim nyalakan led
			soc_Update_Led(100);
		} else if (operCommand[0] == '3') {
			soc_Update_Led(70);
		} else if (operCommand[0] == '2') {
			soc_Update_Led(40);
		} else if (operCommand[0] == '1') {
			soc_Update_Led(20);
		} else if (operCommand[0] == '0') {
			soc_Update_Led(0);
		} else if (operCommand[0] == 'c') {
			//clear ah
			inputData.AhCapacity = 0;
			inputData.AhMax = 0;
			flash_save_data(inputData, 0);
		} else if (operCommand[0] == 'e') {
			get_Saved_Parameter();
		} else if (operCommand[0] == 'y') {
			parse_Edit_Param(operCommand);
			for (int i = 0; i < 5; i++) {
				printMsg_Val(1,"time cut: ", "%d\r\n", time_cut[i]);
			}
		} else if (operCommand[0] == 'u') {
			parse_Edit_Param(operCommand);
			for (int i = 0; i < 5; i++) {
				printMsg_Val(1,"time set: ", "%d\r\n", time_set[i]);
			}
			pcf8563_SetTime(&hi2c3, time_set[0], time_set[1], time_set[2], time_set[3], time_set[4]);
		} else if (operCommand[0] == 'i') {
			// gnati mode antara rs485 dan UART
			uart_channel_flag = (uart_channel_flag == 1) ? 2 : 1;
		} else if (operCommand[0] == '5') {
			printMsg(1, "ERASE MEMORY\r\n");
			// hapus memory
			inputData.Ah = 0;
			inputData.AhCapacity = 0;
			inputData.AhMax = 0;
			inputData.BalancingDiff = 0;
			inputData.BalancingMin = 0;
			inputData.ChargeMax = 0;
			inputData.Cycle = 0;
			inputData.DaySleep = 0;
			inputData.DischargeMax = 0;
			for(int i = 0; i < 10; i++){
				inputData.ErrorCode[i] = 0;
				inputData.ErrorValue[i] = 0;
				inputData.ErrorTime[i] = 0;
			}
			inputData.RShunt = 0;
			inputData.ShortCircuit = 0;
			inputData.SohFlag = 0;
			inputData.TMax = 0;
			inputData.TMosfet = 0;
			inputData.VMax = 0;
			inputData.VMin = 0;
			flash_save_data(inputData, 0);
		} else if (operCommand[0] == '7') {
			// parsing
			parse_Edit_Param(operCommand);
			// save data
			flash_save_data(inputData, 0);
			flash_save_data(inputData2, 4096);

			BMS_BuzzerBeep();
			printMsg(1,"SAVE DATA DULU\r\n");

//			flash_get_data(&savedData, 0);
//			flash_get_data(&savedData2, 4096);

			get_Saved_Parameter();
			inputData = savedData;
			inputData2 = savedData2;

			bat.Ah = (float) (savedData.Ah) / 100.0;
			printMsg_Val(1,"bat.Ah: ", "%.2f\r\n", bat.Ah);

			bat.Ah_max = (float) (savedData.AhMax) / 100.0;
			bat.Ah_capacity = (float) (savedData.AhCapacity) / 100.0;
			bat.sohFlag = savedData.SohFlag;
			bat.cycle = savedData.Cycle;

			BQ_Set_Limit_Current(savedData.ShortCircuit / 100.0,
			savedData.ChargeMax / 100.0, savedData.DischargeMax / 100.0);
			BQ_Set_Limit_Voltage(savedData.VMax, savedData.VMin);
			BQ_Set_Limit_Temperature(savedData.TMax / 100, savedData.TMosfet / 100);
			BQ_Set_Balancing(savedData.BalancingMin, savedData.BalancingDiff);
			BQ_Set_ResShunt(savedData.RShunt / 1000.0);

			BQ_Set_Release_Voltage(savedData2.recovery_ovp, savedData2.recovery_uvp);
			BQ_Set_Release_Temp(savedData2.recovery_OT / 100.0, savedData2.recovery_OTM / 100.0);
			BQ_Set_Limit_Time(savedData2.delayTime_occ, savedData2.delayTime_ocd);

		} else if (operCommand[0] == 'x') {
			terjadiSC++;
			if (terjadiSC > 1) {
				terjadiSC = 0;
			}
		} else if (operCommand[0] == 'z') {
			// ganti CD lD
			// format (z,'cd','ld')
			CD_limit = (uint32_t)strtoul(&operCommand[2], NULL, 10);
			LD_limit = (uint32_t)strtoul(&operCommand[4], NULL, 10);
		} else if (operCommand[0] == '9') {
			errorCode.bits.precharge = (errorCode.bits.precharge == 0) ? 1 : 0;
		}
		memset(operCommand, 0, 100);
	}
}

void debug_SelectionHandler(){
	switch (debug_select) {
	case 1:
		printMsg(1,"<Mode>,bluetooth\r\n");
		bluetooth_TransmitData();
		break;
	case 2:
		// bisa gonta ganti uart
		if (uart_channel_flag == 2) printMsg(1, "<Mode>,RS485\r\n");
		serial_TransmitData(uart_channel_flag);
//		printMsg(1,"<Mode>,bluetooth\r\n");
//		bluetooth_TransmitData();
		break;
	case 3:
		// Generate bluetooth eksternal
		bleData.serialnumber = serialnumber;
		bleData.soc = (int)roundf(soc_Get());
		bleData.voltage = voltage_pack / 1000.0;
		bleData.current = arus;
		bleData.temperature = suhu2[0];
		for (int i = 0; i < 13; i++) {
			bleData.cell[i] = voltage_cell_1[i];
		}
		bleData.cell[13] = voltage_cell_1[14];
		for (int i = 0; i < 10; i++) {
			bleData.cell[i + 14] = voltage_cell_2[i];
		}
		bleData.status = errorCode.regByte;
		// Bluetooth Send Data
		bluetooth_send2(bleData);
		break;
	}
}

void get_Saved_Parameter(){
	flash_get_data(&savedData, 0);
	flash_get_data(&savedData2, 4096);

	for (int i = 0; i < 10; i++) {
		printMsg_Val(uart_channel_flag, "<edit>,14,", "%d\r\n", savedData2.recovery_OT);
		printMsg_Val(uart_channel_flag, "<edit>,15,", "%d\r\n", savedData2.recovery_OTM);
		printMsg_Val(uart_channel_flag, "<edit>,16,", "%d\r\n", savedData2.recovery_ovp);
		printMsg_Val(uart_channel_flag, "<edit>,17,", "%d\r\n", savedData2.recovery_uvp);
		printMsg_Val(uart_channel_flag, "<edit>,18,", "%d\r\n", savedData2.delayTime_occ);
		printMsg_Val(uart_channel_flag, "<edit>,19,", "%d\r\n", savedData2.delayTime_ocd);

		printMsg_Val(uart_channel_flag, "<edit>,20,", "%d\r\n", savedData.AhMax);
		printMsg_Val(uart_channel_flag, "<edit>,21,", "%d\r\n", savedData.VMax);
		printMsg_Val(uart_channel_flag, "<edit>,22,", "%d\r\n", savedData.VMin);
		printMsg_Val(uart_channel_flag, "<edit>,23,", "%d\r\n",
				savedData.ShortCircuit);
		printMsg_Val(uart_channel_flag, "<edit>,24,", "%d\r\n", savedData.ChargeMax);
		printMsg_Val(uart_channel_flag, "<edit>,25,", "%d\r\n",
				savedData.DischargeMax);
		printMsg_Val(uart_channel_flag, "<edit>,26,", "%d\r\n", savedData.TMax);
		printMsg_Val(uart_channel_flag, "<edit>,27,", "%d\r\n", savedData.TMosfet);
		printMsg_Val(uart_channel_flag, "<edit>,28,", "%d\r\n", savedData.AhCapacity);
		printMsg_Val(uart_channel_flag, "<edit>,29,", "%d\r\n",
				savedData.BalancingMin);
		printMsg_Val(uart_channel_flag, "<edit>,30,", "%d\r\n",
				savedData.BalancingDiff);
		printMsg_Val(uart_channel_flag, "<edit>,31,", "%d\r\n", savedData.RShunt);
		printMsg_Val(uart_channel_flag, "<edit>,32,", "%d\r\n", savedData.DaySleep);
		printMsg_Val(uart_channel_flag, "<edit>,33,", "%d\r\n", savedData.SohFlag);
	}
}

void soc_Update_System(unsigned long waktu) {
	soc_Loop(BQ_Get_Current(&BMS), waktu);
	soc_Update_Save(soc_Flag_Save(arus));
	//ini nanti masukin ke fungsi buat cek memori bat sohflag
	int ret = soh_Loop(errorCode.regByte);
	if(ret == 1){
		inputData.SohFlag = bat.sohFlag;
		inputData.AhMax = bat.Ah_max * 100;
		inputData.Cycle = bat.cycle;
		flash_save_data(inputData, 0);
	}
}

//indikator soc LED
void soc_Update_Led(int soc){
	if(soc >= 100 && soc > 75) {
		pin_Led1(ON);
		pin_Led2(ON);
		pin_Led3(ON);
		pin_Led4(ON);
	} else if (soc <= 75 && soc > 50) {
		pin_Led1(ON);
		pin_Led2(ON);
		pin_Led3(ON);
		pin_Led4(OFF);
	} else if (soc <= 50 && soc > 25) {
		pin_Led1(ON);
		pin_Led2(ON);
		pin_Led3(OFF);
		pin_Led4(OFF);
	} else if (soc <= 25 && soc > 1) {
		pin_Led1(ON);
		pin_Led2(OFF);
		pin_Led3(OFF);
		pin_Led4(OFF);
	} else {
		pin_Led1(OFF);
		pin_Led2(OFF);
		pin_Led3(OFF);
		pin_Led4(OFF);
	}
}

void soc_Update_Save(int soc_Flag){
	if(soc_Flag == 1){
		printMsg(1,"masuk Save cek nilai AH\r\n");
		if(bat.Ah != ((float)(savedData.Ah) / 100.0)) {
			printMsg(1,"save DATA\r\n");
			inputData.Ah = (int)(bat.Ah * 100);
			inputData.Cycle = bat.cycle;
			flash_save_data(inputData, 0);
		}
	}
}

void bluetooth_TransmitData(){
	  // Generate bluetooth random dummy data
	  bleData.serialnumber = serialnumber;
	  bleData.soc = (int)roundf(soc_Get());
	  bleData.voltage = voltage_pack / 1000.0;
	  bleData.current = arus;
	  bleData.temperature = suhu2[0];
	  for(int i = 0; i < 13; i++) {
		  bleData.cell[i] = voltage_cell_1[i];
	  }
	  bleData.cell[13] = voltage_cell_1[14];
	  for(int i = 0; i < 10; i++) {
		  bleData.cell[i + 14] = voltage_cell_2[i];
	  }
	  bleData.status = errorCode.regByte;
	  // Bluetooth Send Data
	  bluetooth_send(bleData);
}

void serial_TransmitData(uint8_t uart_channel){
	uint8_t ch = uart_channel;
	uint16_t crc;
	char temp[256];
	char frame[300];
	// ############################################################################################
	snprintf(temp, sizeof(temp), "%d,%d,%d,%d",
			((int)soc_Get())*2,
			soh_Get(),
			(int)(ah_Get()*10),
			cycle_Get());
	crc = crc16_modbus((uint8_t*)temp, strlen(temp));
	snprintf(frame, sizeof(frame), "broad,1,%s,CRC=%04X\r\n", temp, crc);
	printMsg_Val(ch,"","%s", frame);
//	rs485_dataFrame(1);

	// ############################################################################################
	memset(temp, '\0', sizeof(temp));
	memset(frame, '\0', sizeof(frame));
	snprintf(temp, sizeof(temp), "%d,%d,%d,%d,%d,%d",
			(int)(arus * 10),
			(int)voltage_pack,
			(int)(suhu[0]*10),
			(int)(suhu[1]*10),
			(int)(suhu[2]*10),
			(int)(suhu2[0]*10));
	crc = crc16_modbus((uint8_t*)temp, strlen(temp));
	snprintf(frame, sizeof(frame), "broad,2,%s,CRC=%04X\r\n", temp, crc);
	printMsg_Val(ch,"","%s", frame);

	// ############################################################################################
	memset(temp, '\0', sizeof(temp));
	memset(frame, '\0', sizeof(frame));
	int len=0;
	for (int i = 0; i < 15; i++) {
	    len += snprintf(temp + len, sizeof(temp) - len, "%d,", voltage_cell_1[i]);
	    if (len >= sizeof(temp)) break; // biar gak overflow
	}
	for (int i = 0; i < 15; i++) {
	    len += snprintf(temp + len, sizeof(temp) - len, "%d,", voltage_cell_2[i]);
	    if (len >= sizeof(temp)) break; // biar gak overflow
	}
	crc = crc16_modbus((uint8_t*)temp, strlen(temp));
	snprintf(frame, sizeof(frame), "broad,3,%s,CRC=%04X\r\n", temp, crc);
	printMsg_Val(ch,"","%s", frame);

	// ############################################################################################
	memset(temp, '\0', sizeof(temp));
	memset(frame, '\0', sizeof(frame));
	len=0;
	for (int i = 0; i < 15; i++) {
	    len += snprintf(temp + len, sizeof(temp) - len, "%d,", BMS.bal_led[i]);
	    if (len >= sizeof(temp)) break; // biar gak overflow
	}
	for (int i = 0; i < 15; i++) {
	    len += snprintf(temp + len, sizeof(temp) - len, "%d,", BMS2.bal_led[i]);
	    if (len >= sizeof(temp)) break; // biar gak overflow
	}
	crc = crc16_modbus((uint8_t*)temp, strlen(temp));
	snprintf(frame, sizeof(frame), "broad,4,%s,CRC=%04X\r\n", temp, crc);
	printMsg_Val(ch,"","%s", frame);

	// ############################################################################################
	memset(temp, '\0', sizeof(temp));
	memset(frame, '\0', sizeof(frame));
	snprintf(temp, sizeof(temp), "%d,%d,%d,%d,%d,%ld",
			CD,
			LD,
			CAN.status,
			errorCode.regByte,
			warningCode.regByte,
			socFlashMem);
	crc = crc16_modbus((uint8_t*)temp, strlen(temp));
	snprintf(frame, sizeof(frame), "broad,5,%s,CRC=%04X\r\n", temp, crc);
	printMsg_Val(ch,"","%s", frame);
	CAN.status = 0;

	// ############################################################################################
	memset(temp, '\0', sizeof(temp));
	memset(frame, '\0', sizeof(frame));
	RTC_TimeDate now;
	pcf8563_GetTime(&hi2c3, &now);
	snprintf(temp, sizeof(temp), "%02d:%02d,%02d:%02d:%02d,%02d:%02d,%02d:%02d:%02d,0x%08lX,%s",
	         now.hours,
	         now.minutes,
	         now.day,
	         now.month,
	         now.year,
			 23, 59, time_cut[2], time_cut[3], time_cut[4],
			 serialnumber,
			 firmware_ver);
	crc = crc16_modbus((uint8_t*)temp, strlen(temp));
	snprintf(frame, sizeof(frame), "broad,6,%s,CRC=%04X\r\n", temp, crc);
	printMsg_Val(ch,"","%s", frame);
}

void FDCAN_TransmitData(){
	CAN.soc = bat.soc;
	CAN.soh = bat.soh;
	CAN.vpack = voltage_pack/10;
	CAN.arus = arus * 100;
	CAN.errorCode = errorCode.regByte;
	for (int i = 0; i < 15; i++) {
		CAN.cell[i] = voltage_cell_1[i];
	}
	for (int i = 0; i < 10; i++) {
		CAN.cell[i + 15] = voltage_cell_2[i];
	}
	CAN.suhu[0] = suhu[0] * 10;
	CAN.suhu[1] = suhu[1] * 10;
	CAN.suhu[2] = suhu2[0] * 10;
	CAN.suhu[3] = suhu2[1] * 10;
}

void print_Memory_Parameter(){
	flash_get_data(&savedData, 0);
	flash_get_data(&savedData2, 4096);
	printMsg_Val(1,"Ah: ", "%d\r\n", savedData.Ah);
	printMsg_Val(1,"Ah Capacity: ", "%d\r\n", savedData.AhCapacity);
	printMsg_Val(1,"Ah Max: ", "%d\r\n", savedData.AhMax);
	printMsg_Val(1,"Balancing Diff: ", "%d\r\n", savedData.BalancingDiff);
	printMsg_Val(1,"Balancing Min: ", "%d\r\n", savedData.BalancingMin);
	printMsg_Val(1,"Charge Max: ", "%d\r\n", savedData.ChargeMax);
	printMsg_Val(1,"Cycle: ", "%d\r\n", savedData.Cycle);
	printMsg_Val(1,"Day Sleep: ", "%d\r\n", savedData.DaySleep);
	printMsg_Val(1,"Discharge Max: ", "%d\r\n", savedData.DischargeMax);
	printMsg_Val(1,"Resistor Shunt: ", "%d\r\n", savedData.RShunt);
	printMsg_Val(1,"Short Circuit: ", "%d\r\n", savedData.ShortCircuit);
	printMsg_Val(1,"SoH Flag: ", "%d\r\n", savedData.SohFlag);
	printMsg_Val(1,"Temperature Max: ", "%d\r\n", savedData.TMax);
	printMsg_Val(1,"Temperature Mosfet: ", "%d\r\n", savedData.TMosfet);
	printMsg_Val(1,"Volt Max: ", "%d\r\n", savedData.VMax);
	printMsg_Val(1,"Volt Min: ", "%d\r\n", savedData.VMin);
	// memory ke 2
	printMsg_Val(1,"Release OVP: ", "%d\r\n", savedData2.recovery_ovp);
	printMsg_Val(1,"Release UVP: ", "%d\r\n", savedData2.recovery_uvp);
	printMsg_Val(1,"Release OT: ", "%d\r\n", savedData2.recovery_OT);
	printMsg_Val(1,"Release OTM: ", "%d\r\n", savedData2.recovery_OTM);
	printMsg_Val(1,"Delay OCC: ", "%d\r\n", savedData2.delayTime_occ);
	printMsg_Val(1,"Delay OCD: ", "%d\r\n", savedData2.delayTime_ocd);
}

void Print_Memory_Log(uint8_t uart_channel){
	uint8_t ch = uart_channel;
	flash_get_data(&savedData, 0);
	printMsg(1,"LOG ERROR\r\n");
	for(int j = 0; j < 5; j++){
		for(int i = 0; i < 10; i++){
			printMsg_Val(ch,"<error>,", "%ld", savedData.ErrorTime[i]);
			proteksi_ErrorDescription(savedData.ErrorCode[i]);
			printMsg_Val(ch,",=,", "%d\r\n", savedData.ErrorValue[i]);
		}
	}
}

void parameter_SetDefault(){
	inputData.BalancingDiff = BQ_Get_Diff_Balancing();
	inputData.BalancingMin = BQ_Get_Min_Balancing();
	inputData.ChargeMax = ((int)BQ_Get_Limit_Charge()) * 100;
	inputData.Cycle = 0;
	inputData.DaySleep = 7;
	inputData.DischargeMax = ((int)BQ_Get_Limit_Discharge()) * 100;
	for(int i = 0; i < 10; i++){
		inputData.ErrorCode[i] = 0;
		inputData.ErrorValue[i] = 0;
		inputData.ErrorTime[i] = 0;
	}
	inputData.RShunt = (int)(BQ_Get_ResShunt() * 1000);
	inputData.ShortCircuit = ((int)BQ_Get_Limit_ShortCircuit()) * 100;
	inputData.SohFlag = 0;
	inputData.TMax = ((int)BQ_Get_Limit_Temperature()) * 100;
	inputData.TMosfet = ((int)BQ_Get_Limit_Temperature_Mosfet()) * 100;
	inputData.VMax = BQ_Get_Limit_VoltMax();
	inputData.VMin = BQ_Get_Limit_VoltMin();

	inputData2.recovery_ovp = BQ_Get_Release_Volt_Max();
	inputData2.recovery_uvp = BQ_Get_Release_Volt_Min();
	inputData2.recovery_OT = BQ_Get_Release_Temperature() * 100;
	inputData2.recovery_OTM = BQ_Get_Release_Temp_Mosfet() * 100;
	inputData2.delayTime_occ = BQ_Get_Waktu_Over_Charge();
	inputData2.delayTime_ocd = BQ_Get_Waktu_Over_Discharge();

}

void print_SetParameter(){
	printMsg_Val(1,"AH: ", "%.2f\r\n", ah_Get());
	printMsg_Val(1,"AH MAX: ", "%.2f\r\n", bat.Ah_max);
	printMsg_Val(1,"AH CAPACITY: ", "%.2f\r\n", bat.Ah_capacity);
	printMsg_Val(1,"VMAX: ", "%d\r\n", BQ_Get_Limit_VoltMax());
	printMsg_Val(1,"VMIN: ", "%d\r\n", BQ_Get_Limit_VoltMin());
	printMsg_Val(1,"CHARGE: ", "%.2f\r\n", BQ_Get_Limit_Charge());
	printMsg_Val(1,"WAKTU CHARGE: ", "%d\r\n", BQ_Get_Waktu_Over_Charge());
	printMsg_Val(1,"DISCHARGE: ", "%.2f\r\n", BQ_Get_Limit_Discharge());
	printMsg_Val(1,"WAKTU DISCHARGE: ", "%d\r\n", BQ_Get_Waktu_Over_Discharge());
	printMsg_Val(1,"SHUNT: ", "%.2f\r\n", BQ_Get_ResShunt());
	printMsg_Val(1,"TEMPERATURE: ", "%d\r\n", BQ_Get_Limit_Temperature());
	printMsg_Val(1,"BALANCING MIN: ", "%d\r\n", BQ_Get_Min_Balancing());
	printMsg_Val(1,"BALANCING DIFF: ", "%d\r\n", BQ_Get_Diff_Balancing());
}

void parse_first_Command(char *input){
	if (strcmp(first_command, "<ctrl>") == 0){
		printMsg_Val(1,"Control kendali: ", "%s\r\n", operCommand);

	} else if (strcmp(first_command, "<edit>")) {
		printMsg(1, "edit data \r\n");
	}
}

void parse_Edit_Param(char *input){
    char *token;
    int index = 0;
    float values[30];   // buffer tampung hasil parsing (12 cukup, tapi lebihin aja)
    char flag[10];
    printMsg_Val(1,"input raw =", " %s\r\n", input);

    // ---- Ambil flag (sebelum spasi) ----
    token = strtok(input, " ");   // ambil "77"
    if (token != NULL) {
    	strcpy(flag, token);
        printMsg_Val(1,"Flag =", " %c\r\n", flag);
    }

    // ---- Ambil data stream (setelah spasi, pecah dengan koma) ----
    char *subtoken = strtok(NULL,",");   // langsung ambil angka pertama setelah spasi
    printMsg_Val(1,"2nd Token: ", "%s\r\n", subtoken);
    while (subtoken != NULL && index < 30) {
        values[index] = atof(subtoken);  // konversi ke float
        index++;
        subtoken = strtok(NULL, ",");    // lanjut ambil token berikutnya
    }

	if (strcmp(flag, "7") == 0) {
		// contoh: print hasil parsing
		for (int i = 0; i < index; i++) {
			switch (i) {
			case 0:
				bat.Ah_max = values[i];
				inputData.AhMax = values[i] * 100;
				break;
			case 1:
				inputData.VMax = values[i] * 1000;
				BQ_Set_Limit_Voltage(values[i], inputData.VMin);
				break;
			case 2:
				inputData.VMin = values[i] * 1000;
				BQ_Set_Limit_Voltage(inputData.VMax, values[1]);
				break;
			case 3:
				BQ_Set_Limit_Current(inputData.ShortCircuit / 100.0, values[i],
						inputData.DischargeMax / 100.0);
				inputData.ChargeMax = values[i] * 100;
				break;
			case 4:
				BQ_Set_Limit_Current(inputData.ShortCircuit / 100.0,
						inputData.ChargeMax / 100.0, values[i]);
				inputData.DischargeMax = values[i] * 100;
				break;
			case 5:
				BQ_Set_Limit_Current(values[i], inputData.ChargeMax / 100.0,
						inputData.DischargeMax / 100.0);
				inputData.ShortCircuit = values[i] * 100;
				break;
			case 6:
				bat.Ah_capacity = values[i];
				inputData.AhCapacity = values[i] * 100.0;
				break;
			case 7:
				BQ_Set_Limit_Temperature(values[i], inputData.TMosfet);
				inputData.TMax = values[i] * 100;
				break;
			case 8:
				BQ_Set_Limit_Temperature(inputData.TMax, values[i]);
				inputData.TMosfet = values[i] * 100;
				break;
			case 9:
				inputData.BalancingMin = values[i] * 1000;
				BQ_Set_Balancing(values[i], inputData.BalancingDiff);
				break;
			case 10:
				inputData.BalancingDiff = values[i];
				BQ_Set_Balancing(inputData.BalancingMin, values[i]);
				break;
			case 11:
				BQ_Set_ResShunt(values[i]);
				inputData.RShunt = values[i] * 1000;
				break;
			case 12:
				// release ovp limit
				BQ_Set_Release_Voltage(values[i] * 1000, inputData2.recovery_uvp);
				inputData2.recovery_ovp = values[i] * 1000;
				break;
			case 13:
				// release uvp limit
				BQ_Set_Release_Voltage(inputData2.recovery_ovp, values[i] * 1000);
				inputData2.recovery_uvp = values[i] * 1000;
				break;
			case 14:
				// delay time occ
				BQ_Set_Limit_Time(values[i] * 1000, inputData2.delayTime_ocd);
				inputData2.delayTime_occ = values[i] * 1000;
				break;
			case 15:
				// delay time ocd
				BQ_Set_Limit_Time(inputData2.delayTime_occ, values[i] * 1000);
				inputData2.delayTime_ocd = values[i] * 1000;
				break;
			case 16:
				// recovery OT
//				BQ_Set_Release_Temp(values[i], inputData2.TMosfet);
				inputData2.recovery_OT = (int)(values[i] * 100);
				printMsg_Val(1,"input tmax: ", "%d\r\n", inputData2.recovery_OT);
				break;
			case 17:
				// recovery OTM
//				BQ_Set_Release_Temp(inputData2.TMax, values[i]);
				inputData2.recovery_OTM = (int)(values[i] * 100);
				printMsg_Val(1,"input tmosfet: ", "%d\r\n", inputData2.recovery_OTM);
				break;
			}
			printMsg_Val(1,"values = ", "%.2f\r\n", values[i]);
		}
	} else if (strcmp(flag, "76") == 0 || (strcmp(flag, "y")) == 0) {
		time_cut[0] = 23;//(int)values[0]; // jam	// ini nanti di set jam 23
		time_cut[1] = 59;//(int)values[1]; // menit	// ini nanti di set menit 59
		time_cut[2] = (int)values[2];
		time_cut[3] = (int)values[3];
		time_cut[4] = (int)values[4];
	} else if (strcmp(flag, "75") == 0 || (strcmp(flag, "u")) == 0) {
		time_set[0] = (int)values[0];
		time_set[1] = (int)values[1];
		time_set[2] = (int)values[2];
		time_set[3] = (int)values[3];
		time_set[4] = (int)values[4];
	}

}

int* parse_Input(char *input) {
	char buff[100];
	strcpy(buff, input);
	char *token = strtok(buff, " ");

	static int values[10] = { }; // Array untuk menyimpan nilai integer
	int count = 0;
	while (token != NULL) {
		values[count++] = atoi(token); // Konversi token menjadi integer
		token = strtok(NULL, " ");
	}

	return values;
}

void modbus_send(uint8_t frame){
	uint8_t tx[16];
	uint16_t crc;
	if (frame == 1) {

		frame_sel = 0;
//		HAL_UART_Transmit(huart, pData, Size, Timeout)
		tx[0] = 0x99;
		tx[1] = 0x01;
		tx[2] = 0x00;
		tx[3] = 0x00;
		tx[4] = (int)(soc_Get());
		tx[5] = (int )(soh_Get());

		tx[6] = ((int)(voltage_pack / 10) >> 8) & 0xFF;
		tx[7] = ((int)(voltage_pack / 10)) & 0xFF;
		tx[8] = ((int)(abs(arus * 100)) >> 8) & 0xFF;
		tx[9] = (int)(abs(arus * 100)) & 0xFF;
		tx[10] = (arus < 0) ? 1 : 0;

		tx[11] = 0x00;
		tx[12] = 0x00;
		tx[13] = 0x00;

		crc = crc16_modbus(tx, 14); // ID + LEN + payload
		tx[14] = (crc >> 8) & 0xFF; // CRC MSB
		tx[15] = crc & 0xFF;		// CRC LSB
//		tx[15] = (crc >> 8) & 0xFF; // CRC MSB
		send_UART2(tx);

	} else if (frame == 2) {
		frame_sel = 0;

		tx[0] = 0x99;
		tx[1] = 0x01;
		tx[2] = 0x01;
		tx[3] = 0x00;

		for (int i = 0; i < 5; i++){
			tx[4 + (i * 2)] = (voltage_cell_1[i] >> 8) & 0xFF;
			tx[5 + (i * 2)] = voltage_cell_1[i] & 0xFF;
		}

		crc = crc16_modbus(tx, 14); // ID + LEN + payload
		tx[14] = crc & 0xFF;		// CRC LSB
		tx[15] = (crc >> 8) & 0xFF; // CRC MSB
		send_UART2(tx);
	} else if (frame == 3) {
		frame_sel = 0;

		tx[0] = 0x99;
		tx[1] = 0x01;
		tx[2] = 0x01;
		tx[3] = 0x01;

		for (int i = 0; i < 5; i++){
			tx[4 + (i * 2)] = (voltage_cell_1[i + 5] >> 8) & 0xFF;
			tx[5 + (i * 2)] = voltage_cell_1[i + 5] & 0xFF;
		}

		crc = crc16_modbus(tx, 14); // ID + LEN + payload
		tx[14] = crc & 0xFF;		// CRC LSB
		tx[15] = (crc >> 8) & 0xFF; // CRC MSB
		send_UART2(tx);
	} else if (frame == 4) {
		frame_sel = 0;

		tx[0] = 0x99;
		tx[1] = 0x01;
		tx[2] = 0x01;
		tx[3] = 0x02;

		for (int i = 0; i < 5; i++){
			tx[4 + (i * 2)] = (voltage_cell_1[i + 10] >> 8) & 0xFF;
			tx[5 + (i * 2)] = voltage_cell_1[i + 10] & 0xFF;
		}

		crc = crc16_modbus(tx, 14); // ID + LEN + payload
		tx[14] = crc & 0xFF;		// CRC LSB
		tx[15] = (crc >> 8) & 0xFF; // CRC MSB
		send_UART2(tx);
	} else if (frame == 5) {
		frame_sel = 0;

		tx[0] = 0x99;
		tx[1] = 0x01;
		tx[2] = 0x01;
		tx[3] = 0x03;

		for(int i = 0; i < 5; i++)
		{
			tx[4 + (i*2)] = (voltage_cell_2[i] >> 8) & 0xFF;
			tx[5 + (i*2)] = voltage_cell_2[i] & 0xFF;
		}

		crc = crc16_modbus(tx,14);
		tx[14] = crc & 0xFF;
		tx[15] = (crc >> 8) & 0xFF;

		send_UART2(tx);
	} else if (frame == 6) {
		frame_sel = 0;

		tx[0] = 0x99;
		tx[1] = 0x01;
		tx[2] = 0x01;
		tx[3] = 0x04;

		for(int i = 0; i < 5; i++)
		{
			tx[4 + (i*2)] = (voltage_cell_2[i+5] >> 8) & 0xFF;
			tx[5 + (i*2)] = voltage_cell_2[i+5] & 0xFF;
		}

		crc = crc16_modbus(tx,14);
		tx[14] = crc & 0xFF;
		tx[15] = (crc >> 8) & 0xFF;

		send_UART2(tx);
	} else if (frame == 7) {
		frame_sel = 0;

		tx[0] = 0x99;
		tx[1] = 0x01;
		tx[2] = 0x02;
		tx[3] = 0x00;
		tx[4] = (int)(suhu[0]);
		tx[5] = (int)(suhu[1]);
		tx[6] = (int)(suhu[2]);
		tx[7] = (int)(suhu2[0]);

		for (int i = 0; i < 6; i++) {
			tx[8+i] = 0x00;
		}

		crc = crc16_modbus(tx, 14); // ID + LEN + payload
		tx[14] = crc & 0xFF;		// CRC LSB
		tx[15] = (crc >> 8) & 0xFF; // CRC MSB
		send_UART2(tx);
	} else if (frame == 8) {
		frame_sel = 0;

		tx[0] = 0x99;
		tx[1] = 0x01;
		tx[2] = 0x03;      // command Error Status
		tx[3] = 0x00;

		tx[4] = errorCode.regByte;

		for (int i = 5; i < 14; i++)
		{
			tx[i] = 0x00;
		}

		crc = crc16_modbus(tx, 14);

		tx[14] = crc & 0xFF;
		tx[15] = (crc >> 8) & 0xFF;

		send_UART2(tx);
	}
}

void rs485_sendFrame(uint8_t id, const uint8_t *payload, uint8_t len){
    uint8_t buf[256];
    uint16_t crc;
    buf[0] = 0x7E;     // Start
    buf[1] = 0xAA;      // broadcast function
    buf[2] = id;       // Frame ID


    memcpy(&buf[3], payload, len);

    crc = crc16_modbus(&buf[1], len + 2); // ID + LEN + payload

    buf[3 + len] = crc & 0xFF;        // CRC LSB
    buf[4 + len] = (crc >> 8) & 0xFF; // CRC MSB

    buf[5 + len] = 0x7F; //end

	pin_rs485_TX_mode();
    HAL_UART_Transmit(&huart1, buf, 6 + len, HAL_MAX_DELAY);
	pin_rs485_RX_mode();
}

void rs485_dataFrame(uint8_t id){
    uint8_t payload[8];
    switch(id){
    case 1:
        uint16_t soc   = soc_Get() * 2;
        uint8_t  soh   = soh_Get();
        uint16_t ah    = ah_Get() * 10;
        uint16_t cycle = cycle_Get();

        payload[0] = soc & 0xFF;
        payload[1] = soc >> 8;
        payload[2] = soh;
        payload[3] = ah & 0xFF;
        payload[4] = ah >> 8;
        payload[5] = cycle & 0xFF;
        payload[6] = cycle >> 8;
        payload[7] = 0;
        rs485_sendFrame(id, payload, 8);
    	break;
//    case 2:
//
//    	break;
    }
}

// ---------------------------------------------------------------------
// fungsi CRC16 Modbus (polynomial 0xA001)
// ---------------------------------------------------------------------
uint16_t crc16_modbus(const uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)data[pos];
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
