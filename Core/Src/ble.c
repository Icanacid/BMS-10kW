/*
 * ble.c
 *
 *  Created on: Jul 9, 2025
 *      Author: Bima
 */

#include "ble.h"
#include <string.h>
#include <stdio.h>
#include "usart.h"
#define BLE_BUFFER_SIZE 512

UART_HandleTypeDef *_bleHuart;

void bluetooth_parse_data(bluetooth_data_t data, uint8_t* buffer);

void bluetooth_begin(UART_HandleTypeDef *huart) {
	_bleHuart = huart;
}

void bluetooth_parse_data(bluetooth_data_t data, uint8_t* buffer) {
	memset(buffer, 0, BLE_BUFFER_SIZE);
	char temp[32];

	sprintf(temp, "{\"serial_number\": \"%08lX\",", data.serialnumber);
	strcat((char*)buffer, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "\"soc\": %d,", data.soc);
	strcat((char*)buffer, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "\"voltage\": %.2f,", data.voltage);
	strcat((char*)buffer, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "\"current\": %.2f,", data.current);
	strcat((char*)buffer, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "\"temperature\": %.2f,", data.temperature);
	strcat((char*)buffer, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "\"error_code\": %d,", data.status);
	strcat((char*)buffer, temp);
	memset(temp, '\0', sizeof(temp));

	sprintf(temp, "\"cell_voltage\": [");
	strcat((char*)buffer, temp);
	memset(temp, '\0', sizeof(temp));

	int cellLength = sizeof(data.cell)/sizeof(data.cell[0]);
	for(int i = 0; i < cellLength; i++){
		sprintf(temp, "%d", data.cell[i]);
		strcat((char*)buffer, temp);
		memset(temp, '\0', sizeof(temp));
		if(i < cellLength - 1){
			strcat((char*)buffer, ",");
		}
	}

	sprintf(temp, "]}");
	strcat((char*)buffer, temp);
	memset(temp, '\0', sizeof(temp));
}

void bluetooth_send(bluetooth_data_t data){
	uint8_t _writeBuffer[BLE_BUFFER_SIZE];
	bluetooth_parse_data(data, _writeBuffer);
	HAL_UART_Transmit(_bleHuart, _writeBuffer, strlen((const char*)_writeBuffer), 500);

}

void bluetooth_send2(bluetooth_data_t data) {
	uint8_t _writeBuffer[BLE_BUFFER_SIZE];
	bluetooth_parse_data(data, _writeBuffer);
	HAL_UART_Transmit(&huart1, _writeBuffer, strlen((const char*)_writeBuffer), 1000);
}
