/*
 * flash.h
 *
 *  Created on: Jul 8, 2025
 *      Author: Bima
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "main.h"

#define FLASH_WRITE_EN 	0x06
#define FLASH_WRITE_DS 	0x04
#define FLASH_READ 		0x03
#define FLASH_PROGRAM 	0x02
#define FLASH_ERASE 	0x20
#define FLASH_ID 		0x9F

#define FLASH_SAVE_TIMEOUT 1000
#define FLASH_READ_TIMEOUT 100
#define FLASH_ERASE_TIMEOUT 300

typedef struct FlashData {
	uint16_t Ah;
	uint16_t AhMax;
	uint16_t AhCapacity;
	uint16_t VMax;
	uint16_t VMin;
	int ChargeMax;
	int DischargeMax;
	int ShortCircuit;
	int TMax;
	int TMosfet;
	uint16_t BalancingMin;
	uint8_t BalancingDiff;
	uint16_t RShunt;
	uint16_t Cycle;
	uint8_t SohFlag;
	uint8_t DaySleep;
	uint8_t ErrorCode[10];
	uint16_t ErrorValue[10];
	uint32_t ErrorTime[10];

	uint32_t delayTime_occ;
	uint32_t delayTime_ocd;
	uint16_t recovery_uvp;
	uint16_t recovery_ovp;
	uint16_t recovery_OT;
	uint16_t recovery_OTM;

}flash_data_t;

void flash_begin(SPI_HandleTypeDef *hspi);
void flash_setCsPin(GPIO_TypeDef *gpio, uint16_t pin);
uint32_t flash_readId();

void encodeDataToBuffer(const flash_data_t *flashData, uint8_t* bufferData);
void encodeDataToBuffer2(const flash_data_t *flashData, uint8_t* bufferData);
void encodeBufferToData(flash_data_t *flashData, const char *bufferData);
void encodeBufferToData2(flash_data_t *flashData, const char *bufferData);
void sendCommand(uint8_t cmd);
void mapErrorCode(int index, flash_data_t *flashData ,char* token);
uint32_t flash_save_data(flash_data_t flashData, uint32_t addr);
void flash_get_data(flash_data_t *flashData, uint32_t addr);

void flash_readData(uint32_t address, uint8_t* buffer, uint16_t size);
void flash_writeData(uint32_t address, uint8_t* data, uint16_t size);
void flash_sectorErase(uint32_t address);
//void flash_waitFinish();
HAL_StatusTypeDef flash_waitFinish(uint32_t timeout);

#endif /* INC_FLASH_H_ */
