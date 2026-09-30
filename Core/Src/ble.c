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
#include <stdint.h>
#include <stdlib.h>

#define BLE_BUFFER_SIZE 512
uint8_t _writeBuffer[BLE_BUFFER_SIZE];
UART_HandleTypeDef *_bleHuart;
uint8_t edit_BT = 0;

static uint8_t jsonGetUint16(char *json, const char *key, uint16_t *value);
static uint8_t jsonGetUint8(char *json, const char *key, uint8_t *value);
static uint8_t jsonGetFloat(char *json, const char *key, float *value);

void bluetooth_parse_data(bluetooth_data_t data, uint8_t* buffer);

void bluetooth_begin(UART_HandleTypeDef *huart) {
	_bleHuart = huart;
}

void bluetooth_parse_data(bluetooth_data_t data, uint8_t* buffer) {
	memset(buffer, 0, BLE_BUFFER_SIZE);
	char temp[64];

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

	snprintf(temp, sizeof(temp), "],");
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	/* batas */
	snprintf(temp, sizeof(temp), "\"settings\":{ \"ovp\": %d,", data.ovp);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"ovr\": %d,", data.ovr);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"uvp\": %d,", data.uvp);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"uvr\": %d,", data.uvr);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"occ\": %d,", data.occ);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"docc\": %d,", data.docc);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"ocd\": %d,", data.ocd);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"docd\": %d,", data.docd);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"otb\": %d,", data.otb);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"otbr\": %d,", data.otbr);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"otm\": %d,", data.otm);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"otmr\": %d,", data.otmr);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"cap\": %d,", data.cap);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"shunt\": %.2f,", data.shunt);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"bal_min\": %d,", data.bal_min);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"bal_dif\": %d,", data.bal_dif);
	strcat((char*) buffer, temp);
	memset(temp, '\0', sizeof(temp));

	snprintf(temp, sizeof(temp), "\"sleep\": %d}}", data.sleep);
	strcat((char*) buffer, temp);
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

void bluetooth_Param_Read(char *input, bluetooth_data_t *data) {
	if(operCommand_BT[0] != 0){
		edit_BT = 1;
		printMsg(1, "parsing data\r\n");
	    jsonGetUint16(input, "\"ovp\"", &data->ovp);
	    jsonGetUint16(input, "\"ovr\"", &data->ovr);
	    jsonGetUint16(input, "\"uvp\"", &data->uvp);
	    jsonGetUint16(input, "\"uvr\"", &data->uvr);
	    jsonGetUint16(input, "\"occ\"", &data->occ);
	    jsonGetUint16(input, "\"docc\"", &data->docc);
	    jsonGetUint16(input, "\"ocd\"", &data->ocd);
	    jsonGetUint16(input, "\"docd\"", &data->docd);
	    jsonGetUint8(input, "\"otb\"", &data->otb);
	    jsonGetUint8(input, "\"otbr\"", &data->otbr);
	    jsonGetUint8(input, "\"otm\"", &data->otm);
	    jsonGetUint8(input, "\"otmr\"", &data->otmr);
	    jsonGetUint16(input, "\"cap\"", &data->cap);
	    jsonGetFloat(input, "\"shunt\"", &data->shunt);
	    jsonGetUint16(input, "\"bal_min\"", &data->bal_min);
	    jsonGetUint16(input, "\"bal_dif\"", &data->bal_dif);

	    jsonGetUint8(input, "\"sleep\"", &data->sleep);

	    /* proteksi nilai invalid kembali ke dafult*/
	    if(data->ovp < 2800 || data->ovp > 3650) data->ovp = 3650;
	    if(data->ovr < 2800 || data->ovr > 3650) data->ovr = 3450;
	    if(data->uvp < 2800 || data->uvp > 3650) data->uvp = 2800;
	    if(data->uvr < 2800 || data->uvr > 3650) data->uvr = 2900;
	    if(data->occ == 0) data->occ = 50;
	    if(data->ocd == 0) data->ocd = 100;
	    if(data->otb == 0) data->otb = 47;
	    if(data->otbr == 0) data->otbr = 42;
	    if(data->otm == 0) data->otm = 90;
	    if(data->otmr == 0) data->otmr = 80;
	    if(data->cap == 0) data->cap = 100;
	    if(data->shunt == 0) data->shunt = 0.5;
	    if(data->bal_min == 0) data->bal_min = 3300;
	    if(data->bal_dif == 0) data->bal_dif = 10;
	    if(data->sleep == 0) data->sleep = 7;


	    /* print data */
	    printMsg_Val(1, "val ", "%d\r\n", data->ovp);
		printMsg_Val(1, "val ", "%d\r\n", data->ovr);
		printMsg_Val(1, "val ", "%d\r\n", data->uvp);
		printMsg_Val(1, "val ", "%d\r\n", data->uvr);
		printMsg_Val(1, "val ", "%d\r\n", data->occ);
		printMsg_Val(1, "val ", "%d\r\n", data->docc);
		printMsg_Val(1, "val ", "%d\r\n", data->ocd);
		printMsg_Val(1, "val ", "%d\r\n", data->docd);
		printMsg_Val(1, "val ", "%d\r\n", data->otb);
		printMsg_Val(1, "val ", "%d\r\n", data->otbr);
		printMsg_Val(1, "val ", "%d\r\n", data->otm);
		printMsg_Val(1, "val ", "%d\r\n", data->otmr);

		printMsg_Val(1, "val ", "%d\r\n", data->cap);
		printMsg_Val(1, "val ", "%.2f\r\n", data->shunt);
		printMsg_Val(1, "val ", "%d\r\n", data->bal_min);
		printMsg_Val(1, "val ", "%d\r\n", data->bal_dif);
		printMsg_Val(1, "val ", "%d\r\n", data->sleep);
	}
}

/*
 * @brief Helper function
 */
static uint8_t jsonGetUint16(char *json, const char *key, uint16_t *value) {
	char *p = strstr(json, key);

	if (p == NULL)
		return 0;

	p = strchr(p, ':');

	if (p == NULL)
		return 0;

	p++;        // lewati ':'
	return (sscanf(p, "%hu", value) == 1);
//	return atoi(p);
}

/*
 * @brief Helper Function
 */
static uint8_t jsonGetUint8(char *json, const char *key, uint8_t *value) {
	uint16_t temp;

	if (jsonGetUint16(json, key, &temp)) {
		*value = (uint8_t) temp;
		return 1;
	}

	return 0;
}

/*
 * @brief Helper Function
 */
static uint8_t jsonGetFloat(char *json, const char *key, float *value) {
	char *p = strstr(json, key);

	if (p == NULL)
		return 0;

	p = strchr(p, ':');

	if (p == NULL)
		return 0;

	p++;
	return (sscanf(p, "%f", value) == 1);
//	return atof(p);
}
