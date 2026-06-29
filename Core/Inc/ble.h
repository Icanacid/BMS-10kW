/*
 * ble.h
 *
 *  Created on: Jul 9, 2025
 *      Author: Bima
 */

#ifndef INC_BLE_H_
#define INC_BLE_H_

#include "main.h"

typedef struct BluetoothData {
	uint8_t soc;
	float voltage;
	float current;
	float temperature;
	int status;
	uint16_t cell[24];
	uint32_t serialnumber;
}bluetooth_data_t;

void bluetooth_begin(UART_HandleTypeDef *huart);
void bluetooth_send(bluetooth_data_t data);
void bluetooth_send2(bluetooth_data_t data);

#endif /* INC_BLE_H_ */
