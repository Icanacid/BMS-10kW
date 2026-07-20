/*
 * flash.c
 *
 *  Created on: Jul 8, 2025
 *      Author: Bima
 */
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "flash.h"
#include "usart.h"

#define _BUFF_SIZE 512

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
			encodeDataToBuffer(&flashData, bufferData);
			break;
		case 4096:
			printMsg(1, "masuk addres 2 \r\n");
			encodeDataToBuffer2(&flashData, bufferData);
			break;
	}

	flash_sectorErase(addr);
	flash_writeData(addr, bufferData, _BUFF_SIZE);
	flash_waitFinish(FLASH_SAVE_TIMEOUT);
	return strlen((const char*)bufferData);
}

void flash_get_data(flash_data_t *flashData, uint32_t addr){
	uint8_t bufferData[_BUFF_SIZE];

	flash_readData(addr, bufferData, _BUFF_SIZE);
	flash_waitFinish(FLASH_READ_TIMEOUT);

	switch (addr){
		case 0:
			encodeBufferToData(flashData, (char*)bufferData);
			break;
		case 4096:
			printMsg(uart_channel_flag, "masuk addres 2 \r\n");
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
	HAL_SPI_Receive(_hspi, buffer, size, HAL_MAX_DELAY);
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

void flash_sectorErase(uint32_t address){
	sendCommand(FLASH_WRITE_EN);

	uint8_t cmd[4];
	cmd[0] = FLASH_ERASE;
	cmd[1] = (address >> 16) & 0xFF;
	cmd[2] = (address >> 8) & 0xFF;
	cmd[3] = address & 0xFF;

	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(_hspi, cmd, 4, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);

	flash_waitFinish(FLASH_ERASE_TIMEOUT);
}

//void flash_waitFinish(){
//	uint8_t status;
//	do{
//		HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_RESET);
//		HAL_SPI_Transmit(_hspi, (uint8_t[]){0x05}, 1, HAL_MAX_DELAY);
//		HAL_SPI_Receive(_hspi, &status, 1, HAL_MAX_DELAY);
//		HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);
//	} while (status & 0x01);
//}

HAL_StatusTypeDef flash_waitFinish(uint32_t timeout) {
    uint8_t status;
    uint32_t start = HAL_GetTick();

    do {
        if ((HAL_GetTick() - start) >= timeout)
            return HAL_TIMEOUT;

        HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_RESET);

        if (HAL_SPI_Transmit(_hspi, (uint8_t[]){0x05}, 1, 10) != HAL_OK) {
            HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);

            return HAL_ERROR;
        }

        if (HAL_SPI_Receive(_hspi, &status, 1, 10) != HAL_OK) {
            HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);

            return HAL_ERROR;
        }

        HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);

        osDelay(1);

    } while (status & 0x01);

    return HAL_OK;
}

static int safe_append(uint8_t *buf, int len, int cap, const char *fmt, ...) {
    if (len >= cap) {
    	printMsg(1, "Penuh\r\n");
    	return cap;           // sudah penuh, jangan tulis lagi
    }
    va_list ap;
    va_start(ap, fmt);

    int n = vsnprintf((char*)&buf[len], cap - len, fmt, ap);
    va_end(ap);

    if (n < 0) {
		printMsg(1, "Formatting error\r\n");
		return -1;
	}

	if (n >= cap - len) {
		printMsg(1, "Data terpotong\r\n");
		return -1;
	}
    return (n >= cap - len) ? cap : len + n;  // clamp kalau ke-truncate
}

void encodeDataToBuffer(const flash_data_t *flashData, uint8_t* bufferData) {
	memset(bufferData, 0, _BUFF_SIZE);
	int len = 0;

	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->Ah);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->AhMax);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->AhCapacity);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->VMax);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->VMin);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->ChargeMax);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->DischargeMax);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->ShortCircuit);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->TMax);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->TMosfet);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->BalancingMin);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->BalancingDiff);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->RShunt);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->Cycle);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->SohFlag);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%d|", flashData->DaySleep);

//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->Ah);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->AhMax);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->AhCapacity);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->VMax);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->VMin);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->ChargeMax);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->DischargeMax);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->ShortCircuit);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->TMax);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->TMosfet);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->BalancingMin);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->BalancingDiff);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->RShunt);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->Cycle);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->SohFlag);
//	len += snprintf((char*)&bufferData[len], _BUFF_SIZE - len, "%d|", flashData->DaySleep);

	for(int i = 0; i < 10; i++) {
	    len = safe_append(bufferData, len, _BUFF_SIZE, "%d %u %lu", flashData->ErrorCode[i], flashData->ErrorValue[i], flashData->ErrorTime[i]);

	    if(i < 9) {
	        len = safe_append(bufferData, len, _BUFF_SIZE, "|");
	    }
	}
//	printMsg_Val("buffer: ", "%s\r\n", bufferData);
}

void encodeDataToBuffer2(const flash_data_t *flashData, uint8_t* bufferData) {
	memset(bufferData, 0, _BUFF_SIZE);

	int len = 0;

	len = safe_append(bufferData, len, _BUFF_SIZE, "%ld|", flashData->delayTime_occ);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%ld|", flashData->delayTime_ocd);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%u|", flashData->recovery_uvp);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%u|", flashData->recovery_ovp);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%u|", flashData->recovery_OT);
	len = safe_append(bufferData, len, _BUFF_SIZE, "%u|", flashData->recovery_OTM);
}

void encodeBufferToData(flash_data_t *data, const char *bufferData) {
	char *endPointer;

	data->Ah = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->AhMax = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->AhCapacity = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->VMax = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->VMin = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->ChargeMax = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->DischargeMax = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->ShortCircuit = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->TMax = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->TMosfet = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->BalancingMin = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->BalancingDiff = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->RShunt = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->Cycle = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->SohFlag = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->DaySleep = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	for(int i = 0; i < 10; i++) {
		data->ErrorCode[i] =
			strtol(bufferData, &endPointer, 10);

		bufferData = endPointer + 1;

		data->ErrorValue[i] =
			strtol(bufferData, &endPointer, 10);

		bufferData = endPointer + 1;

		data->ErrorTime[i] =
			strtoul(bufferData, &endPointer, 10);

		bufferData = endPointer;

		if(*bufferData == '|')
		{
			bufferData++;
		}
	}
}

void encodeBufferToData2(flash_data_t *data, const char *bufferData) {
	char *endPointer;

	data->delayTime_occ = strtoul(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->delayTime_ocd = strtoul(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->recovery_uvp = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->recovery_ovp = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->recovery_OT = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;

	data->recovery_OTM = strtol(bufferData, &endPointer, 10);
	bufferData = endPointer + 1;
}

void sendCommand(uint8_t cmd){
	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(_hspi, &cmd, 1, 1000);
	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);
}
