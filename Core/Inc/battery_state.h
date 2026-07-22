/*
 * soc.h
 *
 *  Created on: Jul 3, 2025
 *      Author: faish
 */

#ifndef INC_BATTERY_STATE_H_
#define INC_BATTERY_STATE_H_

#include "main.h"

typedef struct {
  float Ah_capacity;
  float Ah_max;
  float Ah;
  float cc;
  float cc_max;
  long cycleCount;
  float cycleAh;
  int cycle;
  int socR;
  float soc;
  int socMap;
  int soh;
  uint8_t sohFlag;  //0 :uninitialized, 1: trigger bawah, 2: trigger atas.
  int countNewSoh;
  uint8_t sohSave;
//  float dod;
} battery_info;

extern battery_info bat;
extern uint8_t flag_reset_soc;
void soc_init(int ParamaeterInit);
void soc_Loop(float arus_soc, unsigned long deltaTime);
int soh_Loop(uint8_t error);
float soc_Reset(int minCell);
uint8_t soc_Flag_Save(float arus);

float soc_Get();
int soh_Update();
int soh_Get();
float ah_Get();
int ahMax_Get();
int ahCap_Get();
void cc_Limit();
float bulatSetengah(float num);
void cycle_Loop();
int cycle_Get();


#endif /* INC_BATTERY_STATE_H_ */
