/*
 * flash.c
 *
 *  Created on: Jul 8, 2025
 *      Author: Bima
 */

#include "flash.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "usart.h"

#define _BUFF_SIZE 256

SPI_HandleTypeDef *_hspi;
GPIO_TypeDef *_csGpio;
uint16_t _csPin;

uint32_t _addr = 0x000000;
uint32_t _addr2 = 0x000100;

void flash_begin(SPI_HandleTypeDef *hspi){
	_hspi = hspi;
}

void flash_setCsPin(GPIO_TypeDef *gpio, uint16_t pin){
	_csGpio = gpio;
	_csPin = pin;
}

uint32_t flash_readId(){
	uint8_t cmd = FLASH_ID;
	uint8_t id[3] = {0};
	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(_hspi, &cmd, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(_hspi, id, 3, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);
	return ((id[0] << 16) | (id[1] << 8) | id[2]);
}

uint32_t flash_save_data(flash_data_t flashData, uint32_t addr){
	uint8_t bufferData[_BUFF_SIZE] = {0};
	switch (addr){
		case 0:
			encodeDataToBuffer(flashData, bufferData);
			break;
		case 4096:
			printMsg(1, "masuk addres 2 \r\n");
			encodeDataToBuffer2(flashData, bufferData);
			break;
	}

	flash_sectorErase(addr);
	flash_writeData(addr, bufferData, _BUFF_SIZE);
	flash_waitFinish();
	return strlen((const char*)bufferData);
}

void flash_get_data(flash_data_t *flashData, uint32_t addr){
	uint8_t bufferData[_BUFF_SIZE];

	flash_readData(addr, bufferData, _BUFF_SIZE);
	flash_waitFinish();

	switch (addr){
		case 0:
			encodeBufferToData(flashData, (char*)bufferData);
			break;
		case 4096:
			printMsg(1, "masuk addres 2 \r\n");
			encodeBufferToData2(flashData, (char*)bufferData);
			break;
	}
//	printf("%s\r\n", flashData->ErrorTime[0]);
}

void flash_readData(uint32_t address, uint8_t* buffer, uint16_t size){
	uint8_t cmd[4];
	cmd[0] = FLASH_READ;
	cmd[1] = (address >> 16) & 0xFF;
	cmd[2] = (address >> 8) & 0xFF;
	cmd[3] = address & 0xFF;

	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(_hspi, cmd, 4, HAL_MAX_DELAY);
	HAL_SPI_Receive(_hspi, buffer, size, HAL_MAX_DELAY);;
	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);
}

void flash_writeData(uint32_t address, uint8_t* data, uint16_t size){
	sendCommand(FLASH_WRITE_EN);

	uint8_t cmd[4];
	cmd[0] = FLASH_PROGRAM;
	cmd[1] = (address >> 16) & 0xFF;
	cmd[2] = (address >> 8) & 0xFF;
	cmd[3] = address & 0xFF;

	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(_hspi, cmd, 4, HAL_MAX_DELAY);
	HAL_SPI_Transmit(_hspi, data, size, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);
}

uint8_t flash_sectorErase(uint32_t address){
	HAL_StatusTypeDef status;
	uint8_t cmd[4];

	flash_waitFinish();

	sendCommand(FLASH_WRITE_EN);

	if ((flash_readStatusRegister() & WEL_STATUS) == 0) {
		return 1;
	}

	cmd[0] = FLASH_ERASE;
	cmd[1] = (address >> 16) & 0xFF;
	cmd[2] = (address >> 8) & 0xFF;
	cmd[3] = address & 0xFF;

	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_RESET);
	status = HAL_SPI_Transmit(_hspi, cmd, 4, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);

	if (status != HAL_OK) {
		return 1;
	}

	flash_waitFinish();

	return 0;
}

void flash_waitFinish(void){
	uint8_t status;
	do{
		HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_RESET);
		HAL_SPI_Transmit(_hspi, (uint8_t[]){0x05}, 1, HAL_MAX_DELAY);
		HAL_SPI_Receive(_hspi, &status, 1, HAL_MAX_DELAY);
		HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);
	} while (status & 0x01);
}

uint8_t flash_readStatusRegister(void) {
	uint8_t resp;

	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(_hspi, (uint8_t[]){0x05}, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(_hspi, &resp, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);

	return resp;
}

void sendCommand(uint8_t cmd){
	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(_hspi, &cmd, 1, 1000);
	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);
}

void encodeDataToBuffer(flash_data_t flashData, uint8_t* bufferData) {
	memset(bufferData, 0, _BUFF_SIZE);
	char temp[32];
	sprintf(temp, "%d|", flashData.Ah);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.AhMax);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.AhCapacity);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.VMax);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.VMin);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.ChargeMax);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.DischargeMax);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.ShortCircuit);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.TMax);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.TMosfet);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.BalancingMin);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.BalancingDiff);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.RShunt);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.Cycle);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.SohFlag);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.DaySleep);
	strcat((char*)bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	for(int i = 0; i < 10; i++) {
		char tempErr[128];
//		flashData.ErrorTime[i] = *flashData.ErrorTime[i] != '\0' ? flashData.ErrorTime[i] : "0";
		sprintf(tempErr, "%d %d %ld",
				flashData.ErrorCode[i],
				flashData.ErrorValue[i],
				flashData.ErrorTime[i]);
		strcat((char*)bufferData, tempErr);
		if(i < 9) {
			strcat((char*)bufferData, "|");
		}
	}
//	printMsg_Val("buffer: ", "%s\r\n", bufferData);
}

void encodeDataToBuffer2(flash_data_t flashData, uint8_t* bufferData) {
	memset(bufferData, 0, _BUFF_SIZE);
	char temp[32];
	sprintf(temp, "%ld|", flashData.delayTime_occ);
	strcat((char*) bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%ld|", flashData.delayTime_ocd);
	strcat((char*) bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.recovery_uvp);
	strcat((char*) bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.recovery_ovp);
	strcat((char*) bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.recovery_OT);
//	printMsg_Val(1,"saved tmax: ", "%.2f\r\n", savedData2.recovery_OT);
	strcat((char*) bufferData, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "%d|", flashData.recovery_OTM);
//	printMsg_Val(1,"saved tmosfet: ", "%.2f\r\n", savedData2.recovery_OTM);
	strcat((char*) bufferData, temp);
	memset(temp, '\0', sizeof(temp));
}

void encodeBufferToData(flash_data_t *flashData, char *bufferData) {
	char* _errorCode[10];
//	printMsg_Val("buffer: ", "%s\r\n", bufferData);
	char* token = strtok(bufferData, "|");
	int idx = 1;
	while(token != NULL){
		switch(idx){
		case 1:
			flashData->Ah = atoi(token);
			break;
		case 2:
			flashData->AhMax = atoi(token);
			break;
		case 3:
			flashData->AhCapacity = atoi(token);
			break;
		case 4:
			flashData->VMax = atoi(token);
			break;
		case 5:
			flashData->VMin = atoi(token);
			break;
		case 6:
			flashData->ChargeMax = atoi(token);
			break;
		case 7:
			flashData->DischargeMax = atoi(token);
			break;
		case 8:
			flashData->ShortCircuit = atoi(token);
			break;
		case 9:
			flashData->TMax = atoi(token);
			break;
		case 10:
			flashData->TMosfet = atoi(token);
			break;
		case 11:
			flashData->BalancingMin = atoi(token);
			break;
		case 12:
			flashData->BalancingDiff = atoi(token);
			break;
		case 13:
			flashData->RShunt = atoi(token);
			break;
		case 14:
			flashData->Cycle = atoi(token);
			break;
		case 15:
			flashData->SohFlag = atoi(token);
			break;
		case 16:
			flashData->DaySleep = atoi(token);
			break;
		default:
			_errorCode[idx-17] = token;
//			flashData->ErrorTime[idx-17] = atoi(token);
//			printMsg_Val("errorcode: ", "%s\r\n", _errorCode[idx-17]);
			break;
		}
		idx++;
		token = strtok(NULL, "|");
	}

	for(int i = 0; i < 10; i++) {
		token = strtok(_errorCode[i], " ");
		idx = 1;
		while(token != NULL){
			switch(idx){
			case 1:
				flashData->ErrorCode[i] = atoi(token);
//				printMsg_Val("code: ", "%d\r\n", flashData->ErrorCode[i]);
				break;
			case 2:
				flashData->ErrorValue[i] = atoi(token);
//				printMsg_Val("errVal: ", "%d\r\n", flashData->ErrorValue[i]);
				break;
			case 3:
				flashData->ErrorTime[i] = atoi(token);
//				printMsg_Val("errTime: ", "%ld\r\n", flashData->ErrorTime[i]);
				break;
			}
			idx++;
			token = strtok(NULL, " ");
		}
	}
	printMsg(1, "Selesai encode 1\r\n");
}

void encodeBufferToData2(flash_data_t *flashData, char *bufferData) {
//	printMsg_Val("buffer: ", "%s\r\n", bufferData);
	char* token = strtok(bufferData, "|");
	int idx = 1;
	while(token != NULL){
		switch(idx){
		case 1:
			flashData->delayTime_occ = atoi(token);
			break;
		case 2:
			flashData->delayTime_ocd = atoi(token);
			break;
		case 3:
			flashData->recovery_uvp = atoi(token);
			break;
		case 4:
			flashData->recovery_ovp = atoi(token);
			break;
		case 5:
			flashData->recovery_OT = atoi(token);
			break;
		case 6:
			flashData->recovery_OTM = atoi(token);
			break;

//		default:
//			_errorCode[idx-17] = token;
//			break;
		}
		idx++;
		token = strtok(NULL, "|");
	}

	printMsg(1, "Selesai encode 2\r\n");

}

