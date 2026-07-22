/*
 * soc.c
 *
 *  Created on: Jul 3, 2025
 *      Author: faish
 */

#include <battery_state.h>
#include "main.h"
#include "usart.h"
#include <math.h>
#include <stddef.h>


float ocv_lfp[21] = { 3.490, 3.404, 3.359, 3.308, 3.304, 3.296, 3.283, 3.275,
		3.271, 3.268, 3.265, 3.264, 3.262, 3.252, 3.240, 3.226, 3.213, 3.190,
		3.177, 3.132, 2.833 };

size_t numOcvPoints = sizeof(ocv_lfp) / sizeof(float);
float *ocv = ocv_lfp;
const static int percent = -1;
battery_info bat;
unsigned long soh_Time_Count = 0;
uint8_t socFlagSave = 0;
uint8_t flag_reset_soc = 0;

typedef enum {
    SOH_STATE_IDLE = 0,
    SOH_STATE_LOW_DETECTED = 1,
    SOH_STATE_HIGH_DETECTED = 2
} SOH_State;

void soc_init(int ParameterInit){
	switch (ParameterInit) {
	case 1:
		bat.Ah = 20.00;
		bat.Ah_max = 20.00;
		bat.Ah_capacity = 20.00;
		bat.cc_max = bat.Ah_max * 3.6e6F;
		bat.soh = bat.Ah_max * 100 / bat.Ah_capacity;

		bat.cc_max = bat.Ah_max * 3.6e6F;

		bat.cc = soc_Reset(BMS_Cell_Min());

		bat.Ah = bat.cc * 2.77777778e-7f;
		bat.soc = bat.cc * 100 / bat.cc_max;
		break;
	case 2:
		bat.cc = bat.Ah * 3.6e6F;
		bat.cc_max = bat.Ah_max * 3.6e6F;
		bat.soc = bat.cc * 100 / bat.cc_max;
		break;
	}
}

void soc_Loop(float arus_soc, unsigned long deltaTime){
	if (arus_soc < 0.5 && arus_soc > -0.5) {
		bat.cc += (-40); //power consumption
		bat.cycleCount += 40;
		bat.soc = bat.cc * 100 / bat.cc_max;
	} else {
		bat.cc += (arus_soc * deltaTime);
		bat.cycleCount += (long) (fabs(arus_soc * deltaTime));
		bat.soc = bat.cc * 100 / bat.cc_max;
	}
//	bat.soc = roundf(bat.soc);
	bat.Ah = bat.cc * 2.77777778e-7f;
	cycle_Loop();
}

int soh_Loop(uint8_t error){
	int ret = 0;
	cc_Limit();
	//cek jika undervoltage
	if ((error & 0b00000001) == 1) {
		if(bat.sohFlag == SOH_STATE_IDLE){
			//cek apakah kondisi berlangsung 1 menit
			if(HAL_GetTick() - soh_Time_Count > 60000) {
				bat.sohFlag = SOH_STATE_LOW_DETECTED;
				ret = 1; //kasih return untuk simpan value
			}
		} else if (bat.sohFlag == SOH_STATE_LOW_DETECTED) {
			// habis nyentuh bawah terus nyentuh bawah lagi.
			// reset nilai soc ke 0;
			if(HAL_GetTick() - soh_Time_Count > 30000) {
				bat.Ah = 0;
				bat.cc = 0;
			}
		} else if (bat.sohFlag == SOH_STATE_HIGH_DETECTED) {
			if (HAL_GetTick() - soh_Time_Count > 60000) {
				bat.sohFlag = SOH_STATE_LOW_DETECTED;
				bat.cc_max = bat.cc_max - bat.cc;
				bat.cc = 0;
				bat.Ah_max = bat.Ah_max - bat.Ah;
				bat.Ah = 0;
				//cek simpan
				ret = 1;
			}
		}
	} else if ((error & 0b00000010) == 2) { //cek jika overvoltage
		if(bat.sohFlag == SOH_STATE_IDLE){
			//cek apakah kondisi berlangsung 1 menit
			if(HAL_GetTick() - soh_Time_Count > 60000) {
				bat.Ah = bat.Ah_max;
				bat.cc = bat.cc_max;
				bat.sohFlag = SOH_STATE_HIGH_DETECTED;
				//cek simpan
				ret = 1;
			}
		} else if (bat.sohFlag == SOH_STATE_LOW_DETECTED) {
			// habis nyentuh atas terus nyentuh bawah lagi.
			// reset nilai soh ke AH sekarang;
			if(HAL_GetTick() - soh_Time_Count > 60000) {
				bat.Ah_max = bat.Ah;
				bat.cc_max = bat.cc;
				bat.sohFlag = SOH_STATE_HIGH_DETECTED;
				//cek simpan
				ret = 1;
			}
		} else if (bat.sohFlag == SOH_STATE_HIGH_DETECTED) {
			//reset nilai Ah ke Ah max
			if (HAL_GetTick() - soh_Time_Count > 30000) {
				bat.Ah = bat.Ah_max;
				bat.cc = bat.cc_max;
			}
		}
	} else {
		soh_Time_Count = HAL_GetTick();
	}
	soh_Update();
	return ret;
}

float soc_Reset(int minCell){
	if (percent <= 100 && percent >= 0) {
		bat.cc = bat.Ah_capacity * 3.6e4F * percent;
	} else {
		printMsg(1,"masuk else \r\n");
		printMsg_Val(1,"cell: ","%d\r\n", minCell);
		bat.cc = 0;
		for (unsigned int i = 0; i < numOcvPoints; i++) {
//	    	printMsg_Val("iterasi", "%d\r\n", i);
			if (ocv[i] <= (float)(minCell) / 1000.0) {
				if (i == 0) {
					bat.cc = bat.Ah_capacity * 3.6e6F;
				} else {
					bat.cc = bat.Ah_capacity * 3.6e6F / (numOcvPoints - 1.0)
							* (numOcvPoints - 1.0 - i
									+ (((float)(minCell) / 1000.0) - ocv[i])
											/ (ocv[i - 1] - ocv[i]));
					printMsg_Val(1,"cc_internal: ", "%.f\r\n", bat.cc);
				}
				break;
			}
		}
	}
	;
	return bat.cc;
}

uint8_t soc_Flag_Save(float arus){
	if (arus == 0 && socFlagSave == 0) {
		//cek memory
		socFlagSave = 1;
	} else if (arus != 0) {
		//flag berubah
		socFlagSave = 0;
	} else socFlagSave = 2;
	return socFlagSave;
}

void cycle_Loop(){
	bat.cycleAh = bat.cycleCount * 2.77777778e-7F;
	if (bat.cycleAh >= (bat.Ah_max * 2)) {
		bat.cycle++;
		bat.cycleCount = 0;
	}
}

float soc_Get(){
	return bat.soc;
}

int soh_Update(){
	float soh = (bat.Ah_max / bat.Ah_capacity) * 100;
	bat.soh = round(soh);
	return bat.soh;
}

int soh_Get(){
	return bat.soh;
}

void cc_Limit() {
	if (bat.cc >= bat.cc_max) {
		bat.cc = bat.cc_max;
	}

	if(bat.cc <= 0){
		bat.cc = 0;
	}
}

float ah_Get(){
	bat.Ah = 20.00;
	return bat.Ah;
}

int ahMax_Get() {
	bat.Ah_max = 20.00;
	return bat.Ah_max;
}

int ahCap_Get() {
	bat.Ah_capacity = 20.00;
	return bat.Ah_capacity;
}

int cycle_Get(){
	int get_cycle = (bat.cycle + (bat.cycleAh/(2*bat.Ah_max)))*10;
	return get_cycle;
}
