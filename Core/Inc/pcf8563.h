/*
 * pcf8563.h
 *
 *  Created on: Oct 3, 2025
 *      Author: faish
 */

#ifndef INC_PCF8563_H_
#define INC_PCF8563_H_

#include "main.h"
extern int time_cut[5];
extern int time_set[5];
extern uint8_t forceControl[2];
typedef struct{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t weekday;
    uint8_t month;
    uint8_t year;
} RTC_TimeDate;


void pcf8563_SetTime(I2C_HandleTypeDef *hi2c, uint8_t hours, uint8_t minute, uint8_t day, uint8_t month, uint8_t year);
void pcf8563_GetTime(I2C_HandleTypeDef *hi2c, RTC_TimeDate *t);
void pcf8563_checkTime();

uint8_t bcd2dec(uint8_t val);

uint8_t dec2bcd(uint8_t val);


#endif /* INC_PCF8563_H_ */
