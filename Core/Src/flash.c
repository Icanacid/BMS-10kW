/*
 * flash.c
 *
 *  Created on: Jul 8, 2025
 *      Author: Bima
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include "flash.h"
#include "usart.h"

#define _BUFF_SIZE 			256
#define ERROR_LOG_COUNT   	10
#define ADDR1_FIELD_COUNT 19
#define ADDR2_FIELD_COUNT 13

static uint8_t flashInit[2] = {0};

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

uint32_t flash_save_data(flash_data_t flashData, uint32_t addr) {
    uint8_t bufferData[_BUFF_SIZE] = {0};
    uint8_t result;

    switch(addr) {
    case 0:
        result = encodeDataToBuffer(&flashData, bufferData, _BUFF_SIZE);
        break;
    case 4096:
        result = encodeDataToBuffer2(&flashData, bufferData, _BUFF_SIZE);
        break;
    default:
        return 0;
    }

    if (!result) {
        printMsg_Val(1, "Gagal Encode Address:", "%u\r\n", addr);
        return 0;
    }

    for(uint8_t retry = 0; retry < MAX_RETRY_SAVE; retry++){
        flash_sectorErase(addr);
        flash_writeData(addr, bufferData, _BUFF_SIZE);
        flash_waitFinish();

        flash_data_t tempData = {0};

        flash_get_data(&tempData, addr);

        if(compareFlashData(&tempData, &flashData, addr)) {
        	printMsg(1, "Berhasil menyimpan flash\r\n");
            return strlen((char*)bufferData);
        }

        printMsg_Val(1, "Verifikasi gagal retry:", "%u\r\n", retry + 1);
    }

    printMsg(1, "Gagal menyimpan flash\r\n");

    return 0;
}

void flash_get_data(flash_data_t *flashData, uint32_t addr){
	uint8_t bufferData[_BUFF_SIZE] = {0};

	if (flashData == NULL) {
	    return;
	}

	flash_data_t tempData = *flashData;

	flash_readData(addr, bufferData, _BUFF_SIZE);
	flash_waitFinish();

	uint8_t index;

	switch (addr) {
		case 0:
			index = 0;

			if (encodeBufferToData(&tempData, (char *)bufferData)) {
				*flashData = tempData;
				flashInit[index] = 1;
			} else {
				printMsg_Val(1, "Gagal Parsing Data Address: ", "%u\r\n", addr);

				if (!flashInit[index]) {
					parameter_SetDefault(flashData);
					flashInit[index] = 1;
				}
			}
			break;

		case 4096:
			index = 1;

			if (encodeBufferToData2(&tempData, (char *)bufferData)) {
				*flashData = tempData;
				flashInit[index] = 1;
			} else {
				printMsg_Val(1, "Gagal Parsing Data Address: ", "%u\r\n", addr);

				if (!flashInit[index]) {
					parameter_SetDefault(flashData);
					flashInit[index] = 1;
				}
			}
			break;
	}
}

void flash_readData(uint32_t address, uint8_t* buffer, uint16_t size){
	uint8_t cmd[4];
	cmd[0] = FLASH_READ;
	cmd[1] = (address >> 16) & 0xFF;
	cmd[2] = (address >> 8) & 0xFF;
	cmd[3] = address & 0xFF;

	flash_waitFinish();

	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(_hspi, cmd, 4, HAL_MAX_DELAY);
	HAL_SPI_Receive(_hspi, buffer, size, HAL_MAX_DELAY);;
	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);

	printMsg_Val(1, "Data: ", "%s\r\n", buffer);
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

uint8_t compareFlashData(const flash_data_t *data1, const flash_data_t *data2, uint32_t addr) {
    if (data1 == NULL || data2 == NULL) {
        return 0;
    }

    switch (addr) {
		case 0:
			if (data1->AhMax != data2->AhMax)
				return 0;

			if (data1->AhCapacity != data2->AhCapacity)
				return 0;

			if (data1->VMax != data2->VMax)
				return 0;

			if (data1->VMin != data2->VMin)
				return 0;

			if (data1->ChargeMax != data2->ChargeMax)
				return 0;

			if (data1->DischargeMax != data2->DischargeMax)
				return 0;

			if (data1->ShortCircuit != data2->ShortCircuit)
				return 0;

			if (data1->TMax != data2->TMax)
				return 0;

			if (data1->TMosfet != data2->TMosfet)
				return 0;

			if (data1->BalancingMin != data2->BalancingMin)
				return 0;

			if (data1->BalancingDiff != data2->BalancingDiff)
				return 0;

			if (data1->RShunt != data2->RShunt)
				return 0;

			if (data1->DaySleep != data2->DaySleep)
				return 0;

			if (data1->delayTime_occ != data2->delayTime_occ)
				return 0;

			if (data1->delayTime_ocd != data2->delayTime_ocd)
				return 0;

			if (data1->recovery_uvp != data2->recovery_uvp)
				return 0;

			if (data1->recovery_ovp != data2->recovery_ovp)
				return 0;

			if (data1->recovery_OT != data2->recovery_OT)
				return 0;

			if (data1->recovery_OTM != data2->recovery_OTM)
				return 0;
			break;
		case 4096:
			if (data1->Ah != data2->Ah)
				return 0;
			if (data1->Cycle != data2->Cycle)
				return 0;
			if (data1->SohFlag != data2->SohFlag)
				return 0;

			for (uint8_t i = 0; i < 10; i++) {
				if (data1->ErrorCode[i] != data2->ErrorCode[i])
					return 0;

				if (data1->ErrorValue[i] != data2->ErrorValue[i])
					return 0;

				if (data1->ErrorTime[i] != data2->ErrorTime[i])
					return 0;
			}
			break;

		default:
			return 0;
    }

    return 1;
}

void sendCommand(uint8_t cmd){
	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(_hspi, &cmd, 1, 1000);
	HAL_GPIO_WritePin(_csGpio, _csPin, GPIO_PIN_SET);
}

static uint8_t append_field(char *buf, size_t bufferSize, size_t *used, const char *format, ...) {
    if (buf == NULL || used == NULL || format == NULL) {
        return 0;
    }

    if (*used >= bufferSize) {
        return 0;
    }

    va_list args;
    va_start(args, format);

    int n = vsnprintf(buf + *used, bufferSize - *used, format, args);

    va_end(args);

    if (n < 0 || (size_t)n >= (bufferSize - *used)) {
        buf[bufferSize - 1] = '\0';
        return 0;
    }

    *used += (size_t)n;

    return 1;
}

static uint8_t parseLong(const char *token, long *value) {
    if (token == NULL || value == NULL) {
        return 0;
    }

    if (token[0] == '\0') {
        return 0;
    }

    char *endptr = NULL;

    long parsedValue = strtol(token, &endptr, 10);

    if (*endptr != '\0') {
        return 0;
    }

    *value = parsedValue;

    return 1;
}

static uint8_t parseUint8(const char *token, uint8_t *value) {
    long parsedValue;

    if (!parseLong(token, &parsedValue)) {
        return 0;
    }

    if (parsedValue < 0 || parsedValue > UINT8_MAX) {
        return 0;
    }

    *value = (uint8_t)parsedValue;

    return 1;
}

static uint8_t parseUint16(const char *token, uint16_t *value) {
    long parsedValue;

    if (!parseLong(token, &parsedValue)) {
        return 0;
    }

    if (parsedValue < 0 || parsedValue > UINT16_MAX) {
        return 0;
    }

    *value = (uint16_t)parsedValue;

    return 1;
}

static uint8_t parseUint32(const char *token, uint32_t *value) {
    unsigned long parsedValue;

    if (token == NULL || value == NULL || token[0] == '\0') {
        return 0;
    }

    char *endptr = NULL;

    parsedValue = strtoul(token, &endptr, 10);

    if (*endptr != '\0') {
        return 0;
    }

    *value = (uint32_t)parsedValue;

    return 1;
}

static uint8_t parseInt(const char *token, int *value) {
    long parsedValue;

    if (!parseLong(token, &parsedValue)) {
        return 0;
    }

    if (parsedValue < INT_MIN || parsedValue > INT_MAX) {
        return 0;
    }

    *value = (int)parsedValue;

    return 1;
}

uint8_t isStructEmpty(flash_data_t *data) {
    flash_data_t emptyData = {0};

    return (memcmp(data, &emptyData, sizeof(flash_data_t)) == 0);
}

uint8_t encodeDataToBuffer(const flash_data_t *flashData, uint8_t *bufferData, size_t bufferSize) {
    if (flashData == NULL || bufferData == NULL || bufferSize == 0) {
        return 0;
    }

    char *buf = (char *)bufferData;
    size_t used = 0;

    buf[0] = '\0';

    if (!append_field(buf, bufferSize, &used, "%d|", flashData->AhMax)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->AhCapacity)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->VMax)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->VMin)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->ChargeMax)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->DischargeMax)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->ShortCircuit)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->TMax)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->TMosfet)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->BalancingMin)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->BalancingDiff)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->RShunt)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->DaySleep)) return 0;

    if (!append_field(buf, bufferSize, &used, "%d|", flashData->delayTime_occ)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->delayTime_ocd)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->recovery_uvp)) return 0;
    if (!append_field(buf, bufferSize, &used, "%d|", flashData->recovery_ovp)) return 0;
	if (!append_field(buf, bufferSize, &used, "%d|", flashData->recovery_OT)) return 0;
	if (!append_field(buf, bufferSize, &used, "%d|", flashData->recovery_OTM)) return 0;

    return 1;
}

uint8_t encodeDataToBuffer2(const flash_data_t *flashData, uint8_t *bufferData, size_t bufferSize) {
    if (flashData == NULL || bufferData == NULL || bufferSize == 0) {
        return 0;
    }

    char *buf = (char *)bufferData;
    size_t used = 0;

    buf[0] = '\0';

    if (!append_field(buf, bufferSize, &used, "%ld|", flashData->Ah)) return 0;

    if (!append_field(buf, bufferSize, &used,"%ld|", flashData->Cycle)) return 0;

    if (!append_field(buf, bufferSize, &used, "%d|", flashData->SohFlag)) return 0;

    for (int i = 0; i < ERROR_LOG_COUNT; i++) {
		const char *sep = (i < ERROR_LOG_COUNT - 1) ? "|" : "";

		if (!append_field(buf, bufferSize, &used, "%d %d %ld%s", flashData->ErrorCode[i], flashData->ErrorValue[i], flashData->ErrorTime[i], sep)) {
			return 0;
		}
	}

    return 1;
}

uint8_t encodeBufferToData(flash_data_t *flashData, char *bufferData) {
    if (flashData == NULL || bufferData == NULL) {
        return 0;
    }

    flash_data_t tmp = *flashData;

    char *saveptr = NULL;

    char *token = strtok_r(bufferData, "|", &saveptr);

    int idx = 0;

    while (token != NULL) {
        uint8_t result = 0;

        switch(idx) {
        case 0:
            result = parseUint16(token, &tmp.AhMax);
            break;

        case 1:
            result = parseUint16(token, &tmp.AhCapacity);
            break;

        case 2:
            result = parseUint16(token, &tmp.VMax);
            break;

        case 3:
            result = parseUint16(token, &tmp.VMin);
            break;

        case 4:
            result = parseInt(token, &tmp.ChargeMax);
            break;

        case 5:
            result = parseInt(token, &tmp.DischargeMax);
            break;

        case 6:
            result = parseInt(token, &tmp.ShortCircuit);
            break;

        case 7:
            result = parseInt(token, &tmp.TMax);
            break;

        case 8:
            result = parseInt(token, &tmp.TMosfet);
            break;

        case 9:
            result = parseUint16(token, &tmp.BalancingMin);
            break;

        case 10:
            result = parseUint8(token, &tmp.BalancingDiff);
            break;

        case 11:
            result = parseUint16(token, &tmp.RShunt);
            break;

        case 12:
            result = parseUint8(token, &tmp.DaySleep);
            break;


        case 13:
            result = parseUint32(token, &tmp.delayTime_occ);
            break;

        case 14:
            result = parseUint32(token, &tmp.delayTime_ocd);
            break;

        case 15:
            result = parseUint16(token, &tmp.recovery_uvp);
            break;

        case 16:
            result = parseUint16(token, &tmp.recovery_ovp);
            break;

        case 17:
            result = parseUint16(token, &tmp.recovery_OT);
            break;

        case 18:
            result = parseUint16(token, &tmp.recovery_OTM);
            break;

        default:
            printMsg(1, "Decode gagal Addr 1: token terlalu banyak\r\n");
            return 0;
        }


        if (!result){
            printMsg(1, "Decode gagal Addr 1: data tidak valid\r\n");
            return 0;
        }

        idx++;

        token = strtok_r(NULL,"|", &saveptr);
    }

    if (idx != ADDR1_FIELD_COUNT) {
        printMsg(1, "Decode gagal Addr 1: jumlah token salah\r\n");

        return 0;
    }

    *flashData = tmp;

    printMsg(1, "Selesai decode Addr 1\r\n");

    return 1;
}

uint8_t encodeBufferToData2(flash_data_t *flashData, char *bufferData) {
    if (flashData == NULL || bufferData == NULL) {
        return 0;
    }

    flash_data_t tmp = *flashData;

    char *saveptr = NULL;

    char *token = strtok_r(bufferData, "|", &saveptr);

    int idx = 0;

    while(token != NULL) {

        uint8_t result = 0;

        switch(idx) {

        case 0:
            result = parseUint16(token, &tmp.Ah);
            break;

        case 1:
            result = parseUint16(token, &tmp.Cycle);
            break;

        case 2:
            result = parseUint8(token, &tmp.SohFlag);
            break;

        default: {
                int errorIdx = idx - 3;

                unsigned int errorCode;
                unsigned int errorValue;
                unsigned long errorTime;

                int ret = sscanf(token, "%u %u %lu", &errorCode, &errorValue, &errorTime);

                if(ret != 3) {
                    printMsg(1, "Decode gagal Addr 2: format error\r\n");

                    return 0;
                }


                tmp.ErrorCode[errorIdx] =(uint8_t)errorCode;
                tmp.ErrorValue[errorIdx] = (uint16_t)errorValue;
                tmp.ErrorTime[errorIdx] = (uint32_t)errorTime;

                result = 1;
            }

            break;
        }

        if(!result) {
            printMsg(1, "Decode gagal Addr 2: data tidak valid\r\n"
            );

            return 0;
        }

        idx++;

        token = strtok_r(NULL, "|", &saveptr);
    }

    if(idx != ADDR2_FIELD_COUNT) {
        printMsg(1, "Decode gagal Addr 2: jumlah token salah\r\n");

        return 0;
    }

    *flashData = tmp;

    printMsg(1,"Selesai decode Addr 2\r\n");

    return 1;
}
