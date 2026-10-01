/*
 * resetCause.h
 *
 *  Created on: Sep 30, 2026
 *      Author: icanacid
 */

#ifndef INC_RESETCAUSE_H_
#define INC_RESETCAUSE_H_

#include "main.h"
#include <stdint.h>

#define RESET_INFORMATION_ADDR    8192U

typedef enum
{
    RESET_CAUSE_UNKNOWN = 0,
    RESET_CAUSE_POWER_ON,
    RESET_CAUSE_BROWN_OUT,
    RESET_CAUSE_PIN,
    RESET_CAUSE_SOFTWARE,
    RESET_CAUSE_IWDG,
    RESET_CAUSE_WWDG,

} ResetCause_t;


typedef struct
{
    uint32_t unknown;
    uint32_t powerOn;
    uint32_t brownOut;
    uint32_t pin;
    uint32_t software;
    uint32_t iwdg;
    uint32_t wwdg;

} ResetCounter_t;


void resetInformation_init(void);

ResetCause_t resetInformation_getCause(void);

uint32_t resetInformation_getCounter(ResetCause_t cause);

void resetInformation_printCounter(void);

#endif /* INC_RESETCAUSE_H_ */
