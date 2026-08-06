/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef union allERROR {
  struct
  {
    uint8_t UVP : 1;
    uint8_t OVP : 1;
    uint8_t OTP : 1;
    uint8_t OCC : 1;
    uint8_t OCD : 1;
    uint8_t SC : 1;
    uint8_t angsuran : 1; //buat proteksi waktu angsuran
    uint8_t precharge : 1; //buat precharge
  } bits;
  uint8_t regByte;
} allERROR_t;

typedef union allWARNING {
  struct
  {
    uint8_t UVP : 1;
    uint8_t OVP : 1;
    uint8_t OTP : 1;
    uint8_t OCC : 1;
    uint8_t OCD : 1;
    uint8_t SC : 1;
    uint8_t angsuran : 1; //buat proteksi angsuran
    uint8_t precharge : 1; //buat precharge
  } bits;
  uint8_t regByte;
} allWARNING_t;

typedef enum {
  ERR_UVP = 0,
  ERR_OVP,
  ERR_OTP,
  ERR_OCC,
  ERR_OCD,
  ERR_SC,
  ERROR_COUNT
} ErrorIndex_t;

//uint8_t cek_log;
extern uint8_t cek_save;

extern uint8_t changeMenu;
extern uint8_t debug_select;
extern uint8_t bluetooth_mode;

//variabel sensor
extern long voltage_pack;
extern uint16_t *voltage_cell_1, *voltage_cell_2;
extern float arus;
extern float *suhu, *suhu2;

extern unsigned long tick_Buzzer;
extern unsigned long tick_SC;

extern uint8_t fdcan_select;

extern volatile uint8_t resetRequest;
//extern uint8_t cek2Save;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void BMS_SoC_Mem_Init(uint32_t memFlag);
void BMS_Save_Mem_Flash();
void BMS_UpdateSystem();
void BMS_BalancingHandler();
int BMS_Cell_Max();
int BMS_Cell_Min();
int BMS_sleep_Condition();
void BMS_Sleep_Handler(int sleepCondition);
void BMS_BuzzerHandler();
void BMS_BuzzerBeep();

int protection_Handler();
void protection_CollectError();
void protection_HoldError();
void protection_LogError();
void proteksi_ErrorDescription(uint8_t error);
void tombol_matikan(int set);
void precharge_CheckCondition(int cd, int ld);
void precharge_Loop(float arusPre);
void precharge_Handler();
float precharge_Get_CD();
float precharge_Get_LD();

int time_ZeroCurrent(float arus);

void command_SerialHandler();
void command_EditHandler();

void debug_SelectionHandler();

void bluetooth_TransmitData();
void save_BT(void);
void FDCAN_TransmitData();
void serial_TransmitData(uint8_t uart_channel);
void serial2_TransmitData();

void soc_Update_System(unsigned long waktu);
void soc_Update_Led(int soc);
void soc_Update_Save(int soc_Flag);
void print_SetParameter();
void get_Saved_Parameter();
void print_Memory_Parameter();
void Print_Memory_Log(uint8_t uart_channel);
void parameter_SetDefault();
void rs485_sendFrame(uint8_t id, const uint8_t *payload, uint8_t len);
void rs485_dataFrame(uint8_t id);
void parse_first_Command(char *input);
void parse_Edit_Param(char *input);
int* parse_Input(char *input);
void modbus_send(uint8_t frame);
void safeReset(void);
uint16_t crc16_modbus(const uint8_t *data, uint16_t len);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define led_Pin GPIO_PIN_2
#define led_GPIO_Port GPIOC
#define buzzer_Pin GPIO_PIN_3
#define buzzer_GPIO_Port GPIOC
#define de_re_Pin GPIO_PIN_4
#define de_re_GPIO_Port GPIOA
#define SPI_CS_Pin GPIO_PIN_4
#define SPI_CS_GPIO_Port GPIOC
#define M_FLASH_RST_Pin GPIO_PIN_5
#define M_FLASH_RST_GPIO_Port GPIOC
#define precharge_Pin GPIO_PIN_2
#define precharge_GPIO_Port GPIOB
#define discharge_Pin GPIO_PIN_12
#define discharge_GPIO_Port GPIOB
#define charge_Pin GPIO_PIN_13
#define charge_GPIO_Port GPIOB
#define load_trigger_Pin GPIO_PIN_14
#define load_trigger_GPIO_Port GPIOB
#define charge_trigger_Pin GPIO_PIN_15
#define charge_trigger_GPIO_Port GPIOB
#define dcdc_Pin GPIO_PIN_6
#define dcdc_GPIO_Port GPIOC
#define boot_Pin GPIO_PIN_7
#define boot_GPIO_Port GPIOC
#define alert_Pin GPIO_PIN_15
#define alert_GPIO_Port GPIOA
#define led_button_Pin GPIO_PIN_3
#define led_button_GPIO_Port GPIOB
#define led4_Pin GPIO_PIN_4
#define led4_GPIO_Port GPIOB
#define led3_Pin GPIO_PIN_5
#define led3_GPIO_Port GPIOB
#define led2_Pin GPIO_PIN_6
#define led2_GPIO_Port GPIOB
#define led1_Pin GPIO_PIN_7
#define led1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
