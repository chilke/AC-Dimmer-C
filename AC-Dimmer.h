/* 
 * File:   AC-Dimmer.h
 * Author: craig
 *
 * Created on February 25, 2020, 8:02 PM
 */

#ifndef AC_DIMMER_H
#define	AC_DIMMER_H

#define PACKAGE_8_PIN 8
#define PACKAGE_14_PIN 14

#define PACKAGE PACKAGE_14_PIN

//Complement of 140
#define ZCD_OFFSET_DELAY 0xFF74

//0x1FFF 13 bits
#define MAX_DELAY 8191

#define CMD_ACK 'K'
#define CMD_NACK 'N'
#define CMD_OVERRUN 'O'
#define CMD_FRAME_ERROR 'Z'
    
#define CMD_START 'S'
#define CMD_STOP 'P'

#define CMD_SET_CH0 0
#define CMD_SET_CH1 1
#define CMD_MAX 1

#define STATE_NONE 0
#define STATE_FAILED 1
#define STATE_PROCESSED 2
#define STATE_FRAME_ERROR 3

typedef unsigned char uchar;

typedef union {
    unsigned int value;
    struct {
        uchar lsb;
        uchar msb;
    };
} uint16;

typedef union {
    uchar value;
    struct {
        unsigned delay  : 2;
        unsigned ch0on0 : 1;
        unsigned ch1on0 : 1;
        unsigned ch0on1 : 1;
        unsigned ch1on1 : 1;
        unsigned ch0aon : 1;
        unsigned ch1aon : 1;
    };
} flag0;

typedef union {
    uchar value;
    struct {
        unsigned off2   : 1;
        unsigned update : 1;
        unsigned unused : 6;
    };
} flag1;

#endif	/* AC_DIMMER_H */

