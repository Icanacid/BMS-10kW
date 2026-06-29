/*
 * pcf8563.c
 *
 *  Created on: Oct 3, 2025
 *      Author: faish
 */


#include "pcf8563.h"
#include "i2c.h"
int time_cut[5];
int time_set[5];
uint8_t forceControl[2];

void pcf8563_SetTime(I2C_HandleTypeDef *hi2c, uint8_t hours, uint8_t minute, uint8_t day, uint8_t month, uint8_t year){
	uint8_t buf[7];
	buf[0] = 0x00;             			   // reg 0x02
	buf[1] = dec2bcd(minute);              // reg 0x03
	buf[2] = dec2bcd(hours);               // reg 0x04
	buf[3] = dec2bcd(day);                 // reg 0x05
	buf[4] = 0x00;             			   // reg 0x06
	buf[5] = dec2bcd(month);               // reg 0x07 (tanpa century bit)
	buf[6] = dec2bcd(year);                // reg 0x08 (00–99)

	HAL_I2C_Mem_Write(hi2c, (0x51 << 1), 0x02, I2C_MEMADD_SIZE_8BIT, buf, 7, HAL_MAX_DELAY);
}
void pcf8563_GetTime(I2C_HandleTypeDef *hi2c, RTC_TimeDate *t){
    uint8_t buf[7];
    HAL_I2C_Mem_Read(hi2c, (0x51 << 1), 0x02, I2C_MEMADD_SIZE_8BIT, buf, 7, HAL_MAX_DELAY);

    t->seconds = bcd2dec(buf[0] & 0x7F);  // bit7 = VL
    t->minutes = bcd2dec(buf[1] & 0x7F);
    t->hours   = bcd2dec(buf[2] & 0x3F);
    t->day     = bcd2dec(buf[3] & 0x3F);
    t->weekday = bcd2dec(buf[4] & 0x07);
    t->month   = bcd2dec(buf[5] & 0x1F);  // ignore century bit
    t->year    = bcd2dec(buf[6]);
}

uint8_t bcd2dec(uint8_t val){
	return ((val >> 4) * 10) + (val & 0x0F);
}

uint8_t dec2bcd(uint8_t val){
	return ((val / 10) << 4) | (val % 10);
}

//void rtc_setTime(){
//	RTC_TimeTypeDef sTime = {0};
//	RTC_DateTypeDef sDate = {0};
//
//	sTime.Hours = time_set[0];
//	sTime.Minutes = time_set[1];
//	sTime.Seconds = 0;
//
//	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK){
//		Error_Handler();
//	}
//
//	sDate.Date = time_set[2];
//	sDate.Month = time_set[3];
//	sDate.Year = time_set[4];
//
//	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK){
//		Error_Handler();
//	}
//}
//
//void rtc_cutTime(){
//
//}

void pcf8563_checkTime(){
	RTC_TimeDate now;
	pcf8563_GetTime(&hi2c3, &now);
	// hitung "nilai total" waktu saat ini
	uint32_t timenow = (now.year  * 365 * 24 * 60) +
	               (now.month * 30  * 24 * 60) +
	               (now.day  * 24  * 60) +
	               (now.hours * 60) +
	               (now.minutes);

	// hitung "nilai total" cutoff
	uint32_t timecut = (time_cut[4] * 365 * 24 * 60) +
	               (time_cut[3] * 30  * 24 * 60) +
	               (time_cut[2] * 24  * 60) +
	               (time_cut[0] * 60) +
	               (time_cut[1]);
	if (timenow >= timecut) {
	    forceControl[0] = 1;
	} else {
	    forceControl[0] = 0;
	}
}
