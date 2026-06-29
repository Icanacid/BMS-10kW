/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
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
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
extern int terjadiSC;
/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */
void pin_Led_Toggle();
void pin_rs485_TX_mode();
void pin_rs485_RX_mode();
void pin_Led(GPIO_PinState state);
void pin_Charge(GPIO_PinState state);
void pin_Discharge(GPIO_PinState state);
void pin_Precharge(GPIO_PinState state);
void pin_Buzzer(GPIO_PinState state);
void pin_Led1(GPIO_PinState state);
void pin_Led2(GPIO_PinState state);
void pin_Led3(GPIO_PinState state);
void pin_Led4(GPIO_PinState state);
void pin_Dcdc(GPIO_PinState state);
void pin_Boot(GPIO_PinState state);
GPIO_PinState pin_Ble_State();
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

