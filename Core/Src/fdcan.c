/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.c
  * @brief   This file provides code for the configuration
  *          of the FDCAN instances.
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
#include "fdcan.h"

/* USER CODE BEGIN 0 */
#include "usart.h"
#include <stdlib.h>
#include "i2c.h"
/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;

volatile uint32_t lastRxTick = 0;
volatile uint32_t lastTxTick = 0;

volatile uint8_t inverterOnline = 1;

typedef enum {
    CAN_STATE_OK,
    CAN_STATE_WARNING,
    CAN_STATE_ERROR,
    CAN_STATE_BUS_OFF
} CAN_State_t;

volatile CAN_State_t canState = CAN_STATE_OK;

uint8_t id_inverter_Deye = 0;

/* FDCAN1 init function */
void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */
	__HAL_FDCAN_CLEAR_FLAG(&hfdcan1, FDCAN_FLAG_RX_FIFO0_NEW_MESSAGE);
	__HAL_FDCAN_CLEAR_FLAG(&hfdcan1, FDCAN_FLAG_RX_FIFO0_FULL);
	__HAL_FDCAN_CLEAR_FLAG(&hfdcan1, FDCAN_FLAG_RX_FIFO0_MESSAGE_LOST);
  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
//  hfdcan1.Init.NominalPrescaler = 16; // Bitrate 250000
  hfdcan1.Init.NominalPrescaler = 8; // Bitrate 500000
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 1;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  FDCAN_FilterTypeDef filterConfig;
  filterConfig.IdType = FDCAN_STANDARD_ID;
  filterConfig.FilterType = FDCAN_FILTER_RANGE;
  filterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  filterConfig.FilterID1 = 0x200;
  filterConfig.FilterID2 = 0x210;
  filterConfig.FilterIndex = 0;


  if(HAL_FDCAN_ConfigFilter(&hfdcan1, &filterConfig) != HAL_OK){
	  printMsg(1,"Config Filter Fail \r\n");
  }

  if(HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK){
	  printMsg(1,"Config Global Filter Fail \r\n");
  }

  /* USER CODE END FDCAN1_Init 2 */

}

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspInit 0 */

  /* USER CODE END FDCAN1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN1 clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* FDCAN1 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
  /* USER CODE BEGIN FDCAN1_MspInit 1 */

  /* USER CODE END FDCAN1_MspInit 1 */
  }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

  /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_FDCAN_CLK_DISABLE();

    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* FDCAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
  /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

  /* USER CODE END FDCAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs){
	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t rxData[8];

	HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, rxData);
	lastRxTick = HAL_GetTick();
	inverterOnline = 1;

	if(RxHeader.Identifier == 0x202) CAN_Data_Select(1);
	if(RxHeader.Identifier == 0x203) CAN_Data_Select(2);
	if(RxHeader.Identifier == 0x204) CAN_Data_Select(3);
	if(RxHeader.Identifier == 0x205) CAN_Data_Select(4);
	if(RxHeader.Identifier == 0x206) CAN_Data_Select(5);
	if(RxHeader.Identifier == 0x207) CAN_Data_Select(6);
	if(RxHeader.Identifier == 0x208) CAN_Data_Select(7);
	if(RxHeader.Identifier == 0x209) CAN_Data_Select(8);
	if(RxHeader.Identifier == 0x210) CAN_Data_Select(9);

	CAN.status = RxHeader.Identifier;

	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_TX_COMPLETE | FDCAN_IT_TX_ABORT_COMPLETE, 0);
}

void FDCAN_Deye(int ID) {
	HAL_StatusTypeDef status;
	FDCAN_TxHeaderTypeDef TxHeader;
	uint8_t TxData[8];
	TxHeader.IdType = FDCAN_STANDARD_ID;

	TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;

	switch (ID){
	case 1:
		TxHeader.Identifier = 0x359;
		TxHeader.DataLength = FDCAN_DLC_BYTES_8;
		TxData[0] = 0;
		TxData[1] = 0;
		TxData[2] = 0;
		TxData[3] = 0;
		TxData[4] = 1;
		TxData[5] = 'P';
		TxData[6] = 'N';
		TxData[7] = 0;
		break;
	case 2:
		TxHeader.Identifier = 0x351;
		TxHeader.DataLength = FDCAN_DLC_BYTES_8;
		TxData[0] = 584 & 0xFF;
		TxData[1] = 584 >> 8;
		TxData[2] = ((int)BQ_Get_Limit_Charge() * 10) & 0xFF; //charge
		TxData[3] = ((int)BQ_Get_Limit_Charge() * 10) >> 8;   //charge
		TxData[4] = ((int)BQ_Get_Limit_Discharge() * 10) & 0xFF; // discharge
		TxData[5] = ((int)BQ_Get_Limit_Discharge() * 10) >> 8;   // discharge
		TxData[6] = 448 & 0xFF;
		TxData[7] = 448 >> 8;
		break;
	case 3:
		TxHeader.Identifier = 0x355;
		TxHeader.DataLength = FDCAN_DLC_BYTES_4;
		TxData[0] = CAN.soc & 0xFF;
		TxData[1] = CAN.soc >> 8;
		TxData[2] = CAN.soh & 0xFF;
		TxData[3] = CAN.soh >> 8;
		break;
	case 4:
		TxHeader.Identifier = 0x356;
		TxHeader.DataLength = FDCAN_DLC_BYTES_6;
		TxData[0] = CAN.vpack & 0xFF;
		TxData[1] = CAN.vpack >> 8;
		TxData[2] = abs(CAN.arus) & 0xFF;
		TxData[3] = abs(CAN.arus) >> 8;
		TxData[4] = CAN.suhu[0] & 0xFF;
		TxData[5] = CAN.suhu[0] >> 8;
		break;
	case 5:
		TxHeader.Identifier = 0x35C;
		TxHeader.DataLength = FDCAN_DLC_BYTES_1;
		TxData[0] = 0b11000000;
		break;
	case 6:
		TxHeader.Identifier = 0x35E;
		TxHeader.DataLength = FDCAN_DLC_BYTES_8;
		TxData[0] = 'P';
		TxData[1] = 'Y';
		TxData[2] = 'L';
		TxData[3] = 'O';
		TxData[4] = 'N';
		TxData[5] = ' ';
		TxData[6] = ' ';
		TxData[7] = ' ';
		break;
	}

	lastTxTick = HAL_GetTick();
	status = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData);
//	printMsg_Val(1,"status: ", "%d\r\n", status);

//	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) != HAL_OK) {
//		printMsg(1,"CAN SEND ERROR \r\n");
//	  	//start FDCAN
//	  	if(HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
//	  		printMsg(1,"FDCAN START FAIL \r\n");
//	  	}
//	}
}

void CAN_Data_Select(int id){
	FDCAN_TxHeaderTypeDef TxHeader;
	uint8_t TxData[8];

	TxHeader.IdType = FDCAN_STANDARD_ID;

	TxHeader.DataLength = FDCAN_DLC_BYTES_8;
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;

	switch(id){
	case 1:
		TxHeader.Identifier = 0x002;
		TxData[0] = CAN.soc;
		TxData[1] = CAN.soh;
		TxData[2] = CAN.vpack >> 8;
		TxData[3] = CAN.vpack & 0xFF;
		TxData[4] = CAN.errorCode;
		TxData[5] = abs(CAN.arus) >> 8;
		TxData[6] = abs(CAN.arus) & 0xFF;
		TxData[7] = (CAN.arus > 0) ? 0 : 1;
		break;
	case 2:
		TxHeader.Identifier = 0x104;
		TxData[0] = CAN.suhu[0] >> 8;
		TxData[1] = CAN.suhu[0] & 0xFF;
		TxData[2] = CAN.suhu[1] >> 8;
		TxData[3] = CAN.suhu[1] & 0xFF;
		TxData[4] = CAN.suhu[2] >> 8;
		TxData[5] = CAN.suhu[2] & 0xFF;
		TxData[6] = CAN.suhu[3] >> 8;
		TxData[7] = CAN.suhu[3] & 0xFF;
		break;
	case 3:
		TxHeader.Identifier = 0x208;
		TxData[0] = CAN.cell[0] >> 8;
		TxData[1] = CAN.cell[0] & 0xFF;
		TxData[2] = CAN.cell[1] >> 8;
		TxData[3] = CAN.cell[1] & 0xFF;
		TxData[4] = CAN.cell[2] >> 8;
		TxData[5] = CAN.cell[2] & 0xFF;
		TxData[6] = CAN.cell[3] >> 8;
		TxData[7] = CAN.cell[3] & 0xFF;
		break;
	case 4:
		TxHeader.Identifier = 0x302;
		TxData[0] = CAN.cell[4] >> 8;
		TxData[1] = CAN.cell[4] & 0xFF;
		TxData[2] = CAN.cell[5] >> 8;
		TxData[3] = CAN.cell[5] & 0xFF;
		TxData[4] = CAN.cell[6] >> 8;
		TxData[5] = CAN.cell[6] & 0xFF;
		TxData[6] = CAN.cell[7] >> 8;
		TxData[7] = CAN.cell[7] & 0xFF;
		break;
	case 5:
		TxHeader.Identifier = 0x404;
		TxData[0] = CAN.cell[8] >> 8;
		TxData[1] = CAN.cell[8] & 0xFF;
		TxData[2] = CAN.cell[9] >> 8;
		TxData[3] = CAN.cell[9] & 0xFF;
		TxData[4] = CAN.cell[10] >> 8;
		TxData[5] = CAN.cell[10] & 0xFF;
		TxData[6] = CAN.cell[11] >> 8;
		TxData[7] = CAN.cell[11] & 0xFF;
		break;
	case 6:
		TxHeader.Identifier = 0x508;
		TxData[0] = CAN.cell[12] >> 8;
		TxData[1] = CAN.cell[12] & 0xFF;
		TxData[2] = CAN.cell[13] >> 8;
		TxData[3] = CAN.cell[13] & 0xFF;
		TxData[4] = CAN.cell[14] >> 8;
		TxData[5] = CAN.cell[14] & 0xFF;
		TxData[6] = CAN.cell[15] >> 8;
		TxData[7] = CAN.cell[15] & 0xFF;
		break;
	case 7:
		TxHeader.Identifier = 0x602;
		TxData[0] = CAN.cell[16] >> 8;
		TxData[1] = CAN.cell[16] & 0xFF;
		TxData[2] = CAN.cell[17] >> 8;
		TxData[3] = CAN.cell[17] & 0xFF;
		TxData[4] = CAN.cell[18] >> 8;
		TxData[5] = CAN.cell[18] & 0xFF;
		TxData[6] = CAN.cell[19] >> 8;
		TxData[7] = CAN.cell[19] & 0xFF;
		break;
	case 8:
		TxHeader.Identifier = 0x704;
		TxData[0] = CAN.cell[20] >> 8;
		TxData[1] = CAN.cell[20] & 0xFF;
		TxData[2] = CAN.cell[21] >> 8;
		TxData[3] = CAN.cell[21] & 0xFF;
		TxData[4] = CAN.cell[22] >> 8;
		TxData[5] = CAN.cell[22] & 0xFF;
		TxData[6] = CAN.cell[23] >> 8;
		TxData[7] = CAN.cell[23] & 0xFF;
		break;
	case 9:
		TxHeader.Identifier = 0x705;
		TxData[0] = CAN.cell[24] >> 8;
		TxData[1] = CAN.cell[24] & 0xFF;
		TxData[2] = 2;
		TxData[3] = 3;
		TxData[4] = 4;
		TxData[5] = 5;
		TxData[6] = 6;
		TxData[7] = 7;
		break;
	}

	lastTxTick = HAL_GetTick();

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) != HAL_OK) {
		printMsg(1,"CAN SEND ERROR \r\n");
		if(HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
			printMsg(1,"FDCAN START FAIL \r\n");
		}
//
//		if(HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
//			printMsg(1,"FDCAN INTERRUPT RX FAIL \r\n");
//		}
	}

}

void fdcanHandler(void) {
//	printMsg(1,"Masuk FDCAN Handler\r\n");
	if (fdcan_select == 1) {
//		printMsg(1,"FDCAN Select 1\r\n");
		FDCAN_Deye(id_inverter_Deye);
		(id_inverter_Deye == 6) ? (id_inverter_Deye = 1) : (id_inverter_Deye++);
//		printMsg(1,"Using CAN Data Select \r\n");
	} else {
		CAN_Data_Select(1);
//		printMsg_Val(1,"ID: ", "%d\r\n", id_inverter_Deye);
	}
}

void FDCAN_ErrorHandler(void) {
	uint8_t errorDetected = 0;
//    uint32_t error = HAL_FDCAN_GetError(&hfdcan1);
    /* ===== HARDWARE FAULT ONLY ===== */
    if (__HAL_FDCAN_GET_FLAG(&hfdcan1, FDCAN_FLAG_BUS_OFF)) {
        canState = CAN_STATE_BUS_OFF;

        HAL_FDCAN_Stop(&hfdcan1);
        HAL_FDCAN_Start(&hfdcan1);
        return;
    }

    if (__HAL_FDCAN_GET_FLAG(&hfdcan1, FDCAN_FLAG_ERROR_PASSIVE)) {
    	canState = CAN_STATE_ERROR;
        errorDetected = 1;
    }

    if (__HAL_FDCAN_GET_FLAG(&hfdcan1, FDCAN_FLAG_RX_FIFO0_MESSAGE_LOST)) {
    	canState = CAN_STATE_WARNING;
        errorDetected = 1;
    }

    /* ===== FINAL STATE DECISION ===== */
    if (errorDetected) {
    	__HAL_FDCAN_CLEAR_FLAG(&hfdcan1, FDCAN_FLAG_ERROR_PASSIVE);
    	__HAL_FDCAN_CLEAR_FLAG(&hfdcan1, FDCAN_FLAG_BUS_OFF);
    	__HAL_FDCAN_CLEAR_FLAG(&hfdcan1, FDCAN_FLAG_RX_FIFO0_MESSAGE_LOST);
    	HAL_FDCAN_Stop(&hfdcan1);
		HAL_FDCAN_Start(&hfdcan1);
//		printMsg(1, "Reset CAN\r\n");
    } else {
    	canState = CAN_STATE_OK;
    }

//    printMsg_Val(1,"error: ", "%d\r\n", error);
//	printMsg_Val(1, "CAN state: ", "%d\r\n", canState);
}


/* USER CODE END 1 */
