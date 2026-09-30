/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "gpio.h"
#include "fdcan.h"
#include "battery_state.h"
#include "iwdg.h"
#include "pcf8563.h"
#include "ble.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
unsigned long prevTick = 0;
unsigned long tick = 0;
int deltaTick = 0;



/*
 * buzz = 0x01
 * led	= 0x02
 * */
uint8_t flag;

unsigned long time_led = 0;

/* USER CODE END Variables */
/* Definitions for normalTask */
osThreadId_t normalTaskHandle;
const osThreadAttr_t normalTask_attributes = {
  .name = "normalTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1024 * 4
};
/* Definitions for highTask */
osThreadId_t highTaskHandle;
const osThreadAttr_t highTask_attributes = {
  .name = "highTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 896 * 4
};
/* Definitions for aboveNormalTask */
osThreadId_t aboveNormalTaskHandle;
const osThreadAttr_t aboveNormalTask_attributes = {
  .name = "aboveNormalTask",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 1024 * 4
};
/* Definitions for lowTask */
osThreadId_t lowTaskHandle;
const osThreadAttr_t lowTask_attributes = {
  .name = "lowTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 1024 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
/* USER CODE END FunctionPrototypes */

void NromalTask(void *argument);
void HighTask(void *argument);
void AboveNormalTask(void *argument);
void LowTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of normalTask */
  normalTaskHandle = osThreadNew(NromalTask, NULL, &normalTask_attributes);

  /* creation of highTask */
  highTaskHandle = osThreadNew(HighTask, NULL, &highTask_attributes);

  /* creation of aboveNormalTask */
  aboveNormalTaskHandle = osThreadNew(AboveNormalTask, NULL, &aboveNormalTask_attributes);

  /* creation of lowTask */
  lowTaskHandle = osThreadNew(LowTask, NULL, &lowTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */

  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_NromalTask */
/**
  * @brief  Function implementing the normalTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_NromalTask */
void NromalTask(void *argument)
{
  /* USER CODE BEGIN NromalTask */
  /* Infinite loop */
  for(;;)
  {
		osDelay(100);
//		printMsg(1,"Masuk Normal Task\r\n");
		command_SerialHandler();
		fdcanHandler();
		FDCAN_ErrorHandler();

		// Buzzer Tone Connect Bluetooth
//		if(pin_Ble_State() == GPIO_PIN_RESET) {
//			if((flag & 0x00) || (flag & 0x02)) {
//				debug_select = 2;
//			} else debug_select = debug_select;
//			flag |= 0x01;
//		} else {
//			debug_select = 1;
//			if (flag & 0x01) {
//				BMS_BuzzerBeep();
//				flag &= 0xFE;
//			}
//		}

		// Lampu LED Indikator SOC
		if (HAL_GPIO_ReadPin(led_button_GPIO_Port, led_button_Pin)
				== GPIO_PIN_RESET) {
			flag |= 0x02;
			tombol_matikan(0); //nyala
			time_led = HAL_GetTick();
		}
		else tombol_matikan(1);

		if((flag & 0x02) && HAL_GetTick() - time_led < 5000){
			soc_Update_Led(soc_Get());
		} else {
			soc_Update_Led(0);
			flag &= 0xFD; // set ke 0
		}

		if(BT_isReceive) {
			BT_isReceive = 0;
			bluetooth_Param_Read(operCommand_BT, &bleData);
			save_BT();
		}

		modbus_send(frame_sel);
		modbusErrorHandler();
	    BMS_BuzzerHandler();

//	    if (resetRequest && (arus < 0.5 && arus > -0.5)) {
//	    	safeReset();
//	    } else if (resetRequest && (arus > 0.5 || arus < -0.5)) {
//	    	resetRequest = 0;
//	    	printMsg(1, "Gagal melakukan Reset, Baterai tidak dalam kondisi Idle\r\n");
//	    }
	}
  /* USER CODE END NromalTask */
}

/* USER CODE BEGIN Header_HighTask */
/**
* @brief Function implementing the highTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_HighTask */
void HighTask(void *argument)
{
  /* USER CODE BEGIN HighTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(250);
//    printMsg(1,"Masuk High Task\r\n");
    BMS_UpdateSystem();
  }
  /* USER CODE END HighTask */
}

/* USER CODE BEGIN Header_AboveNormalTask */
/**
* @brief Function implementing the aboveNormalTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_AboveNormalTask */
void AboveNormalTask(void *argument)
{
  /* USER CODE BEGIN AboveNormalTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
//    printMsg(1,"Masuk Above Normal Task\r\n");
    tick = HAL_GetTick();

    deltaTick = tick - prevTick;
		if (deltaTick != 0.00 && flag_reset_soc == 0) {
			soc_Update_System(deltaTick);
			prevTick = tick;
		}
		if(cek_save == 1) {
			printMsg(1,"SAVE log SHIFT\r\n");
			protection_LogError();
			cek_save = 2;
		}
  }
  /* USER CODE END AboveNormalTask */
}

/* USER CODE BEGIN Header_LowTask */
/**
* @brief Function implementing the lowTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LowTask */
void LowTask(void *argument)
{
  /* USER CODE BEGIN LowTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(950);
//    printMsg(1,"Masuk Low Task\r\n");
//    pcf8563_checkTime();
    pin_Led_Toggle();
    debug_SelectionHandler();
    BMS_Sleep_Handler(BMS_sleep_Condition());
    iwdg_Reload();

  }

  /* USER CODE END LowTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

