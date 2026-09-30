/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "string.h"
#include <stdlib.h>
#include "stdio.h"
#include "stdarg.h"
#include "gpio.h"

#define BUFFER_SIZE 200
char user_data;
uint8_t user_data2[8];
char UART_BUFFER[BUFFER_SIZE] = {};
uint8_t buffer_index = 0;
char operCommand[100] = {}; //definisi variable
char *first_command;

uint8_t uart_channel_flag = 1;
uint8_t frame_sel;

volatile uint8_t BT_isReceive = 0;
char operCommand_BT[256];
char user_data_BT;
char UART_BUFFER_BT[256];
uint8_t buffer_index_BT = 0;

volatile uint32_t lastCommTick = 0;
volatile uint8_t commError = 0;

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}
/* USART3 init function */

void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3;
    PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_11);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
// Fungsi serial print khusus teks
void printMsg(uint8_t channel, char *msg) {
	if(channel == 1){
		HAL_UART_Transmit(&huart1, (uint8_t*) msg, strlen(msg), 200);
	} else if (channel == 2){
		pin_rs485_TX_mode();
		if (HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), 200) != HAL_OK) {
			MX_USART2_UART_Init();
			HAL_StatusTypeDef check2 = HAL_UART_Receive_IT(&huart2, user_data2, 8);
			if (check2 == HAL_OK) {
				printMsg(1, "HAL_OK2\r\n");
			} else if (check2 == HAL_BUSY) {
				printMsg(1, "HAL_BUSY2\r\n");
			} else if (check2 == HAL_TIMEOUT) {
				printMsg(1, "HAL_TIMEOUT2\r\n");
			} else if (check2 == HAL_ERROR) {
				printMsg(1, "HAL_ERROR2\r\n");
			}
		}
		pin_rs485_RX_mode();
	}
}

// Fungsi serial print dengan value
void printMsg_Val(uint8_t channel, char *msg, const char *format, ...) {
	char MSG[200] = { '\0' };
	va_list args;

	va_start(args, format);
	vsnprintf(MSG, sizeof(MSG), format, args);
	va_end(args);

	char finalMsg[200] = { '\0' };
	snprintf(finalMsg, sizeof(finalMsg), "%s%s", msg, MSG);

	if (channel == 1){ // UART
		HAL_UART_Transmit(&huart1, (uint8_t*) finalMsg, strlen(finalMsg), HAL_MAX_DELAY);
	} else if(channel == 2){ // RS485
		pin_rs485_TX_mode();
		HAL_UART_Transmit(&huart2, (uint8_t*) finalMsg, strlen(finalMsg),HAL_MAX_DELAY);
		pin_rs485_RX_mode();
	}
}


// fungsi serial print binary
void printMsg_Binary(uint8_t val) {
	// loop untuk mencetak setiap bit dari MSB ke LSB
	for (int i = 7; i >= 0; i--) {
		printMsg_Val(1,"", "%d", (val >> i) & 1); //ambil bit ke-i dan cetak
		if (i % 4 == 0) {	//tambahkan spasi setiap 4 bit
			printMsg(1," ");
		}
	}
}

// fungsi untuk input message
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uart) {
	if (uart->Instance == USART1) {
		if (user_data != '\r' && user_data != '\n') {
			if (buffer_index < BUFFER_SIZE - 1) { // Cegah buffer overflow
				UART_BUFFER[buffer_index++] = user_data;
			}
		} else {
			// Jika 'Enter' diterima, tambahkan null-terminator dan cetak buffer
			UART_BUFFER[buffer_index] = '\0'; // Null-terminate string
			printMsg_Val(1,"<input>,", "%s\r\n", UART_BUFFER);

			memcpy(operCommand, UART_BUFFER, sizeof(operCommand));
			char *input = UART_BUFFER;
			first_command = strtok(input, ",");
			if(input != NULL) {
				printMsg_Val(1, "1st cmd = ", "%s\r\n", first_command);
			}
			// Reset buffer untuk input berikutnya
			memset(UART_BUFFER, 0, BUFFER_SIZE); // Bersihkan buffer
			buffer_index = 0;                   // Reset indeks
		}
		// Enable UART reception in IT mode
		HAL_UART_Receive_IT(&huart1, (uint8_t*) &user_data, sizeof(char));
	} else if (uart->Instance == USART2) {
//		if (user_data != '\r' && user_data != '\n') {
//			if (buffer_index < BUFFER_SIZE - 1) { // Cegah buffer overflow
//				UART_BUFFER[buffer_index++] = user_data;
//			}
//		} else {
//			// beri flag
//			uart_channel_flag = 2;
//
//			// Jika 'Enter' diterima, tambahkan null-terminator dan cetak buffer
//			UART_BUFFER[buffer_index] = '\0'; // Null-terminate string
//			printMsg_Val(1,"input2: ", "%s\r\n", UART_BUFFER);
//			memcpy(operCommand, UART_BUFFER, sizeof(operCommand));
//			// Reset buffer untuk input berikutnya
//			memset(UART_BUFFER, 0, BUFFER_SIZE); // Bersihkan buffer
//			buffer_index = 0;                   // Reset indeks
//		}
		// Enable UART reception in IT mode
		printMsg(1,"REQ\r\n");
		if (user_data2[0] == 0x99 && user_data2[1] == 0x01){
			lastCommTick = HAL_GetTick();
			commError = 0;
			printMsg(1,"Device ID and Funtion ID OK \r\n");
			if (user_data2[2] == 0x00 && user_data2[3] == 0x00) {
				// Frame 1
				frame_sel = 1;
			} else if (user_data2[2] == 0x01 && user_data2[3] == 0x00) {
				// Frame 2
				frame_sel = 2;
			} else if (user_data2[2] == 0x01 && user_data2[3] == 0x01) {
				// Frame 3
				frame_sel = 3;
			} else if (user_data2[2] == 0x01 && user_data2[3] == 0x02) {
				// Frame 4
				frame_sel = 4;
			} else if (user_data2[2] == 0x01 && user_data2[3] == 0x03) {
				// Frame 5
				frame_sel = 5;
			} else if (user_data2[2] == 0x01 && user_data2[3] == 0x04) {
				// Frame 6
				frame_sel = 6;
			} else if (user_data2[2] == 0x02 && user_data2[3] == 0x00) {
				// Frame 7
				frame_sel = 7;
			} else if (user_data2[2] == 0x03 && user_data2[3] == 0x00) {
				// Frame 8 (Error Status)
				frame_sel = 8;
			}
		}
//		else if (user_data2[0] == 0x99 && user_data2[1] == 0x05) {
//			lastCommTick = HAL_GetTick();
//			commError = 0;
//			printMsg(1,"Device ID and Funtion ID Command OK \r\n");
//			if (user_data2[2] == 0x00 && user_data2[3] == 0x10) {
//				uint16_t crcCalc = crc16_modbus(user_data2, 4);
//				uint16_t crcRecv = ((uint16_t)user_data2[5] << 8) | user_data2[4];
//				if (crcCalc == crcRecv) {
//					resetRequest = 1;
//				} else {
//					printMsg(1,"WARN: reset cmd CRC mismatch, diabaikan\r\n");
//				}
//			}
//		}
//		HAL_UART_Receive_IT(&huart2, (uint8_t*) &user_data, sizeof(char));
		HAL_UART_Receive_IT(&huart2, user_data2, 8);
	} else if (uart->Instance == USART3) {
		if (user_data_BT != '}' && user_data_BT != '\n') {
			if (buffer_index_BT < BUFFER_SIZE - 1) { // Cegah buffer overflow
				UART_BUFFER_BT[buffer_index_BT++] = user_data_BT;
			}
		} else {
			// Jika 'Enter' diterima, tambahkan null-terminator dan cetak buffer
			UART_BUFFER_BT[buffer_index_BT++] = '\0'; // Null-terminate string
			printMsg_Val(1, "<input>,%s\r\n", UART_BUFFER_BT);

			memcpy(operCommand_BT, UART_BUFFER_BT, sizeof(operCommand_BT));
			// Reset buffer untuk input berikutnya
			memset(UART_BUFFER_BT, 0, BUFFER_SIZE); // Bersihkan buffer
			buffer_index_BT = 0;                   // Reset indeks

			BT_isReceive = 1;
		}
		// Enable UART reception in IT mode
		HAL_UART_Receive_IT(&huart3, (uint8_t*) &user_data_BT, sizeof(char));
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1)
    {
        uint32_t err = HAL_UART_GetError(huart);

        printMsg_Val(1, "UART Error: ", "%lu\r\n", err);

        HAL_UART_AbortReceive(huart);

        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_FEFLAG(huart);
        __HAL_UART_CLEAR_NEFLAG(huart);

        HAL_UART_Receive_IT(&huart1, (uint8_t*) &user_data, sizeof(char));
    }
}

void send_UART2(uint8_t *data){
	pin_rs485_TX_mode();
	HAL_UART_Transmit(&huart2, data, 16, 100);
	pin_rs485_RX_mode();
}

// fungsi inisiasi untuk input message
void serial_Interrupt_Init() {
	// init interrupt UART1
	HAL_StatusTypeDef check = HAL_UART_Receive_IT(&huart1, (uint8_t*) &user_data, sizeof(char));
	if(check == HAL_OK) {
		printMsg(1,"HAL_OK\r\n");
	} else if (check == HAL_BUSY) {
		printMsg(1,"HAL_BUSY\r\n");
	} else if (check == HAL_TIMEOUT) {
		printMsg(1,"HAL_TIMEOUT\r\n");
	} else if (check == HAL_ERROR) {
		printMsg(1,"HAL_ERROR\r\n");
	}

	// init interrupt UART2
//	HAL_StatusTypeDef check2 = HAL_UART_Receive_IT(&huart2, (uint8_t*) &user_data, sizeof(char));
	HAL_StatusTypeDef check2 = HAL_UART_Receive_IT(&huart2, user_data2, 8);
	if(check2 == HAL_OK) {
		printMsg(1,"HAL_OK2\r\n");
	} else if (check2 == HAL_BUSY) {
		printMsg(1,"HAL_BUSY2\r\n");
	} else if (check2 == HAL_TIMEOUT) {
		printMsg(1,"HAL_TIMEOUT2\r\n");
	} else if (check2 == HAL_ERROR) {
		printMsg(1,"HAL_ERROR2\r\n");
	}

	// init interrupt UART3
	HAL_StatusTypeDef check3 = HAL_UART_Receive_IT(&huart3, (uint8_t*) &user_data, sizeof(char));
	if(check3 == HAL_OK) {
		printMsg(1,"HAL_OK3\r\n");
	} else if (check3 == HAL_BUSY) {
		printMsg(1,"HAL_BUSY3\r\n");
	} else if (check3 == HAL_TIMEOUT) {
		printMsg(1,"HAL_TIMEOUT3\r\n");
	} else if (check3 == HAL_ERROR) {
		printMsg(1,"HAL_ERROR3\r\n");
	}
}

void modbusErrorHandler(void) {
    uint32_t now = HAL_GetTick();

    if ((now - lastCommTick) > 5000) {

		UART_Reset_Transmission();

		printMsg(1, "RS485 TIMEOUT RESET\r\n");
		lastCommTick = now;
    }
}

void UART_Reset_Transmission(void) {
    HAL_UART_Abort(&huart2);

    __HAL_UART_CLEAR_OREFLAG(&huart2);
    __HAL_UART_CLEAR_FEFLAG(&huart2);
    __HAL_UART_CLEAR_NEFLAG(&huart2);
    __HAL_UART_CLEAR_PEFLAG(&huart2);

    HAL_UART_DeInit(&huart2);
    HAL_UART_Init(&huart2);

    pin_rs485_RX_mode();

    // restart receive setelah reset
    HAL_UART_Receive_IT(&huart2, user_data2, 8);
}



/* USER CODE END 1 */
