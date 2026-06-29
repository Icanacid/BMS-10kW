/*
 * register.h
 *
 *  Created on: Jun 30, 2025
 *      Author: faish
 */

// register map

#ifndef REGISTER_H
#define REGISTER_H

#define SYS_STAT 0x00
#define CELLBAL1 0x01
#define CELLBAL2 0x02
#define CELLBAL3 0x03
#define SYS_CTRL1 0x04
#define SYS_CTRL2 0x05
#define PROTECT1 0x06
#define PROTECT2 0x07
#define PROTECT3 0x08
#define OV_TRIP 0x09
#define UV_TRIP 0x0A
#define CC_CFG 0x0B

#define BAT_HI 0x2A
#define BAT_LO 0x2B

#define TS1_HI 0x2C
#define TS1_LO 0x2D
#define TS2_HI 0x2E
#define TS2_LO 0x2F
#define TS3_HI 0x30
#define TS3_LO 0x31

#define CC_HI 0x32
#define CC_LO 0x33

#define ADCGAIN1 0x50
#define ADCOFFSET 0x51
#define ADCGAIN2 0x59

// function from TI reference design
//#define LOW_BYTE(Data) (uint8_t)(0xff & Data)
//#define HIGH_BYTE(Data) (uint8_t)(0xff & (Data >> 8))

// for bit clear operations of the SYS_STAT register
//#define STAT_CC_READY (0x80)
//#define STAT_DEVICE_XREADY (0x20)
//#define STAT_OVRD_ALERT (0x10)
//#define STAT_UV (0x08)
//#define STAT_OV (0x04)
//#define STAT_SCD (0x02)
//#define STAT_OCD (0x01)
//#define STAT_FLAGS (0x3F)


#endif /* INC_REGISTER_H_ */
