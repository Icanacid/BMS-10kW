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
    /* 4-byte alignment */
    float voltage;
    float current;
    float temperature;
    float shunt;

    int status;
    uint32_t serialnumber;

    /* 2-byte alignment */
    uint16_t cell[24];

    uint16_t ovp;
    uint16_t ovr;
    uint16_t uvp;
    uint16_t uvr;
    uint16_t occ;
    uint16_t docc;
    uint16_t ocd;
    uint16_t docd;
    uint16_t cap;
    uint16_t bal_min;
    uint16_t bal_dif;

    /* 1-byte alignment */
    uint8_t soc;
    uint8_t otb;
    uint8_t otbr;
    uint8_t otm;
    uint8_t otmr;
    uint8_t sleep;
} bluetooth_data_t;

extern bluetooth_data_t bleData;

extern uint8_t edit_BT;

void bluetooth_begin(UART_HandleTypeDef *huart);
void bluetooth_send(bluetooth_data_t data);
void bluetooth_send2(bluetooth_data_t data);

void bluetooth_Param_Read(char *input, bluetooth_data_t *data);

#endif /* INC_BLE_H_ */
