/*
 * resetCause.c
 *
 *  Created on: Sep 30, 2026
 *      Author: icanacid
 */

#include "resetCause.h"
#include "flash.h"
#include "usart.h"
#include <string.h>


static ResetCause_t resetCause = RESET_CAUSE_UNKNOWN;
static ResetCounter_t resetCounter;

static ResetCause_t resetInformation_detectCause(void) {
    uint32_t flags = RCC->CSR;

    if (flags & RCC_CSR_IWDGRSTF) {
        return RESET_CAUSE_IWDG;
    }

    if (flags & RCC_CSR_WWDGRSTF) {
        return RESET_CAUSE_WWDG;
    }

    if (flags & RCC_CSR_SFTRSTF) {
        return RESET_CAUSE_SOFTWARE;
    }

    if (flags & RCC_CSR_PINRSTF) {
        return RESET_CAUSE_PIN;
    }

#ifdef RCC_CSR_BORRSTF
    if (flags & RCC_CSR_BORRSTF) {
        return RESET_CAUSE_BROWN_OUT;
    }
#endif

#ifdef RCC_CSR_PORRSTF
    if (flags & RCC_CSR_PORRSTF) {
        return RESET_CAUSE_POWER_ON;
    }
#endif

    return RESET_CAUSE_UNKNOWN;
}


static uint8_t resetInformation_isFlashEmpty(void) {
    uint8_t buffer[sizeof(ResetCounter_t)];

    flash_readData(RESET_INFORMATION_ADDR, buffer, sizeof(buffer));

    flash_waitFinish();

    for (uint16_t i = 0; i < sizeof(buffer); i++) {
        if (buffer[i] != 0xFF) {
            return 0;
        }
    }

    return 1;
}

static void resetInformation_readCounter(void) {
    flash_readData(RESET_INFORMATION_ADDR, (uint8_t *)&resetCounter, sizeof(ResetCounter_t));
    flash_waitFinish();
}

static void resetInformation_writeCounter(void) {
    flash_sectorErase(RESET_INFORMATION_ADDR);
    flash_waitFinish();

    flash_writeData(RESET_INFORMATION_ADDR, (uint8_t *)&resetCounter, sizeof(ResetCounter_t));
    flash_waitFinish();
}

void resetInformation_init(void) {
    if (resetInformation_isFlashEmpty()) {
        memset(&resetCounter, 0, sizeof(ResetCounter_t));
        printMsg(1, "8192 is Empty\r\n");

        resetInformation_writeCounter();
    } else {
        resetInformation_readCounter();
        printMsg(1, "Read Address 8192\r\n");
    }

    resetCause = resetInformation_detectCause();

    switch (resetCause) {
        case RESET_CAUSE_UNKNOWN:
            resetCounter.unknown++;
            break;

        case RESET_CAUSE_POWER_ON:
            resetCounter.powerOn++;
            break;

        case RESET_CAUSE_BROWN_OUT:
            resetCounter.brownOut++;
            break;

        case RESET_CAUSE_PIN:
            resetCounter.pin++;
            break;

        case RESET_CAUSE_SOFTWARE:
            resetCounter.software++;
            break;

        case RESET_CAUSE_IWDG:
            resetCounter.iwdg++;
            break;

        case RESET_CAUSE_WWDG:
            resetCounter.wwdg++;
            break;

        default:
            resetCounter.unknown++;
            break;
    }

    resetInformation_writeCounter();

    __HAL_RCC_CLEAR_RESET_FLAGS();
}

ResetCause_t resetInformation_getCause(void) {
    return resetCause;
}

uint32_t resetInformation_getCounter(ResetCause_t cause) {
    switch (cause) {
        case RESET_CAUSE_UNKNOWN:
            return resetCounter.unknown;

        case RESET_CAUSE_POWER_ON:
            return resetCounter.powerOn;

        case RESET_CAUSE_BROWN_OUT:
            return resetCounter.brownOut;

        case RESET_CAUSE_PIN:
            return resetCounter.pin;

        case RESET_CAUSE_SOFTWARE:
            return resetCounter.software;

        case RESET_CAUSE_IWDG:
            return resetCounter.iwdg;

        case RESET_CAUSE_WWDG:
            return resetCounter.wwdg;

        default:
            return 0;
    }
}

void resetInformation_printCounter(void) {
    printMsg_Val(1, "Reset UNKNOWN: ", "%lu\r\n", resetInformation_getCounter(RESET_CAUSE_UNKNOWN));
    printMsg_Val(1, "Reset POWER_ON: ", "%lu\r\n", resetInformation_getCounter(RESET_CAUSE_POWER_ON));
    printMsg_Val(1, "Reset BROWN_OUT: ", "%lu\r\n", resetInformation_getCounter(RESET_CAUSE_BROWN_OUT));
    printMsg_Val(1, "Reset PIN: ", "%lu\r\n", resetInformation_getCounter(RESET_CAUSE_PIN));
    printMsg_Val(1, "Reset SOFTWARE: ", "%lu\r\n", resetInformation_getCounter(RESET_CAUSE_SOFTWARE));
    printMsg_Val(1, "Reset IWDG: ", "%lu\r\n", resetInformation_getCounter(RESET_CAUSE_IWDG));
    printMsg_Val(1, "Reset WWDG: ", "%lu\r\n", resetInformation_getCounter(RESET_CAUSE_WWDG));
}
