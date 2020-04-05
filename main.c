// CONFIG1
#pragma config FEXTOSC = OFF    // External Oscillator mode selection bits (Oscillator not enabled)
#pragma config RSTOSC = HFINTPLL// Power-up default value for COSC bits (HFINTOSC with 2x PLL, with OSCFRQ = 16 MHz and CDIV = 1:1 (FOSC = 32 MHz))
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; i/o or oscillator function on OSC2)
#pragma config CSWEN = OFF      // Clock Switch Enable bit (The NOSC and NDIV bits cannot be changed by user software)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (FSCM timer disabled)

// CONFIG2
#pragma config MCLRE = ON       // Master Clear Enable bit (MCLR pin is Master Clear function)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config LPBOREN = OFF    // Low-Power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = ON       // Brown-out reset enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = HI        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) is set to 2.7V)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = ON     // Peripheral Pin Select one-way control (The PPSLOCK bit can be cleared and set only once in software)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a reset)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled, SWDTEN is ignored)
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4
#pragma config BBSIZE = BB512   // Boot Block Size Selection bits (512 words boot block size)
#pragma config BBEN = OFF       // Boot Block Enable bit (Boot Block disabled)
#pragma config SAFEN = OFF      // SAF Enable bit (SAF disabled)
#pragma config WRTAPP = OFF     // Application Block Write Protection bit (Application Block not write protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block not write protected)
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration Register not write protected)
#pragma config WRTSAF = OFF     // Storage Area Flash Write Protection bit (SAF not write protected)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low Voltage programming enabled. MCLR/Vpp pin function is MCLR.)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (UserNVM code protection disabled)

#include <xc.h>
#include "AC-Dimmer.h"

#define DEBUG

uint16 Ch0Delay;
uint16 Ch1Delay;

uchar Cmd;
uint16 CmdVal;

uchar RcvCnt;
uchar Rx;

//Must be in common RAM for ISR speed
__near uint16 Delay0;
__near uint16 Delay1;
__near uint16 Delay2;

__near flag0 Flags0;
__near flag1 Flags1;

void __interrupt() myIsr(void) {
#ifdef DEBUG
    if (PIR4bits.TMR2IF) {
        PIR4bits.TMR2IF = 0;
#else
    if (PIR2bits.ZCDIF) {
        PIR2bits.ZCDIF = 0;
#endif
        TMR0H = (ZCD_OFFSET_DELAY >> 8);
        TMR0L = (ZCD_OFFSET_DELAY & 0xFF);
        Flags0.delay = 0;
        
        if (Flags1.update) {
            T0CON0bits.T0EN = 1;
            //updateDelays
            Flags0.value = 0;
            Flags1.value = 0;
            Delay0.value = 0xFFFF;
            Delay1.value = 0xFFFF;
            Delay2.value = 0xFFFF;
            
            LATAbits.LATA4 = 0;
            LATAbits.LATA5 = 0;
            
            if (Ch0Delay.value == 0) {
                Flags0.ch0aon = 1;
                LATAbits.LATA4 = 1;
                
                if (Ch1Delay.value == 0) {
                    Flags0.ch1aon = 1;
                    LATAbits.LATA5 = 1;
                    T0CON0bits.T0EN = 0;
                } else {
                    if (Ch1Delay.value == MAX_DELAY) {
                        T0CON0bits.T0EN = 0;
                    } else {
                        Delay0.value = Ch1Delay.value;
                        Flags0.ch1on0 = 1;
                        Delay1.value = MAX_DELAY - Ch1Delay.value;
                        Flags1.off2 = 1;
                    }
                }
            } else if (Ch1Delay.value == 0) {
                Flags0.ch1aon = 1;
                LATAbits.LATA5 = 1;
                
                if (Ch0Delay.value == MAX_DELAY) {
                    T0CON0bits.T0EN = 0;
                } else {
                    Delay0.value = Ch0Delay.value;
                    Flags0.ch0on0 = 1;
                    Delay1.value = MAX_DELAY - Ch0Delay.value;
                    Flags1.off2 = 1;
                }
            } else if (Ch0Delay.value == Ch1Delay.value) {
                if (Ch0Delay.value == MAX_DELAY) {
                    T0CON0bits.T0EN = 0;
                } else {
                    Delay0.value = Ch0Delay.value;
                    Flags0.ch0on0 = 1;
                    Flags0.ch1on0 = 1;
                    Delay1.value = MAX_DELAY - Ch0Delay.value;
                    Flags1.off2 = 1;
                }
            } else if (Ch0Delay.value > Ch1Delay.value) {
                Delay0.value = Ch1Delay.value;
                Flags0.ch1on0 = 1;
                if (Ch0Delay.value == MAX_DELAY) {
                    Delay1.value = MAX_DELAY - Delay0.value;
                    Flags1.off2 = 1;
                } else {
                    Delay1.value = Ch0Delay.value - Ch1Delay.value;
                    Flags0.ch0on1 = 1;
                    Delay2.value = MAX_DELAY - Ch0Delay.value;
                }
            } else {
                Delay0.value = Ch0Delay.value;
                Flags0.ch0on0 = 1;
                if (Ch1Delay.value == MAX_DELAY) {
                    Delay1.value = MAX_DELAY - Delay0.value;
                    Flags1.off2 = 1;
                } else {
                    Delay1.value = Ch1Delay.value - Ch0Delay.value;
                    Flags0.ch1on1 = 1;
                    Delay2.value = MAX_DELAY - Ch1Delay.value;
                }
            }
            
            //complementDelays
            Delay0.value = ~Delay0.value;
            Delay1.value = ~Delay1.value;
            Delay2.value = ~Delay2.value;
        }
    } else {
        PIR0bits.TMR0IF = 0;
        switch(Flags0.delay) {
            case 0:
                TMR0H = Delay0.msb;
                TMR0L = Delay0.lsb;
                break;
            case 1:
                TMR0H = Delay1.msb;
                TMR0L = Delay1.lsb;
                if (Flags0.ch0on0) {
                    LATAbits.LATA4 = 1;
                }
                if (Flags0.ch1on0) {
                    LATAbits.LATA5 = 1;
                }
                break;
            case 2:
                TMR0H = Delay2.msb;
                TMR0L = Delay2.lsb;
                if (Flags0.ch0on1) {
                    LATAbits.LATA4 = 1;
                }
                if (Flags0.ch1on1) {
                    LATAbits.LATA5 = 1;
                }
                if (Flags1.off2) {
                    if (!Flags0.ch0aon) {
                        LATAbits.LATA4 = 0;
                    }
                    if (!Flags0.ch1aon) {
                        LATAbits.LATA5 = 0;
                    }
                }
                break;
            case 3:
                if (!Flags0.ch0aon) {
                    LATAbits.LATA4 = 0;
                }
                if (!Flags0.ch1aon) {
                    LATAbits.LATA5 = 0;
                }
                asm("decf _Flags0, f");
        }
        asm("incf _Flags0, f");
    }
    return;
}

uchar nibbleToHex(uchar i) {
    if (i >= 10) {
        return 'A'+i-10;
    }
    return '0'+i;
}

void sendChar(uchar c) {
    while (!PIR3bits.TX1IF);
    TX1REG = c;
}

void sendInt(uchar i) {
    uchar tmp = i;
    tmp >>= 4;
    sendChar(nibbleToHex(tmp));
    i &= 0xF;
    sendChar(nibbleToHex(i));
}

uchar hexToNibble(uchar c) {
    if (c >= 'A') {
        return c-'A'+10;
    }
    return c-'0';
}

int main(void)
{
    uchar state;
    //Bank 62 ANSEL & PPS
    ANSELA = 0b00000100; //Bit 2 on for ZCD input
#if PACKAGE == PACKAGE_14_PIN
    ANSELC = 0b00000000; //TODO Remove for 8 PIN Build
#endif
    //ESP GPIO13 is RX and is ICSP DAT
    //ICSP DAT is RA0 which should be our TX
    RA0PPS = 0xF;
    
    //Bank 0 LAT, TRIS
    //A0-UART TX, A1-UART RX, A2-ZCD, A4-CH0, A5-CH1
    LATA = 0b00000000;
#if PACKAGE == PACKAGE_14_PIN
    LATC = 0b00000000;
#endif
    TRISA = 0b11001110;
#if PACKAGE == PACKAGE_14_PIN
    TRISC = 0b11111111;
#endif
    
    //Bank 61 More PPS Setup
    TX1CKPPS = 0x00;
    RX1DTPPS = 0x01;
    
    //Bank 2 EUSART Setup
    //SYNC=0, BRGH=1, BRG16=0
    //32MHz Clock, BRG=16 for 115200
    //SP1BRGH = 0; Resets to 0
    SP1BRGL = 16;
    //BAUD1CON = 0; Resets to 0
    TX1STA = 0b00100100;
    RC1STA = 0b10010000;
    
    //Bank 11 Timer 0
    //Clk Fosc/4 8MHz
    //Pre 1:8 1MHz or 1us period
    //16 Bit enabled
    //T0CON0 = 0; Resets to 0
    T0CON0bits.T016BIT = 1;
    T0CON1 = 0b01000011;
    
    //Bank 18 ZCD
    ZCDCON = 0b10000011;
    
#ifdef DEBUG
    //Clock source MFINTOSC = 31.25kHz
    T2CLKCON = 0b00000110;
    //T2HLT = 0; Resets to 0
    PR2 = 133;
    T2CON = 0b10010000;
    PIE4bits.TMR2IE = 1;
#else
    PIE2bits.ZCDIE = 1;
#endif
    
    //Global Bank, turn on interrupts
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    
    //Bank 14 Non-global interrupt setup
    PIE0bits.TMR0IE = 1;
    
    RcvCnt = 0;
    Flags1.value = 0;
    Flags0.value = 0;
    Ch0Delay.value = MAX_DELAY;
    Ch1Delay.value = MAX_DELAY;
    
    while (1) {
        state = STATE_NONE;
        if (PIR3bits.RC1IF) {
            if (RC1STAbits.FERR) {
                RC1REG;
                //This is to handle possible wake on receive
                if (Rx != 0 || RcvCnt != 0) {
                    state = STATE_FRAME_ERROR;
                }
            } else {
                Rx = RC1REG;
                if (Rx == CMD_START) {
                    if (RcvCnt != 0) {
                        sendChar(CMD_NACK);
                    }
                    RcvCnt = 1;
                } else { //Not Start Byte
                    switch (RcvCnt & 7) { //mask first 3 bits just in case
                        case 0:
                            if (Rx != 0) {
                                state = STATE_FAILED;
                            } else {
                                RcvCnt--;
                            }
                            break;
                        case 1:
                            Cmd = hexToNibble(Rx);
                            Cmd <<= 4;
                            break;
                        case 2:
                            Cmd += hexToNibble(Rx);
                            if (Cmd > CMD_MAX) {
                                state = STATE_FAILED;
                            }
                            break;
                        case 3:
                            CmdVal.msb = hexToNibble(Rx);
                            CmdVal.msb <<= 4;
                            break;
                        case 4:
                            CmdVal.msb += hexToNibble(Rx);
                            break;
                        case 5:
                            CmdVal.lsb = hexToNibble(Rx);
                            CmdVal.lsb <<= 4;
                            break;
                        case 6:
                            CmdVal.lsb += hexToNibble(Rx);
                            if (Cmd == CMD_SET_CH0) {
                                if (Flags1.update || CmdVal.value > MAX_DELAY) {
                                    state = STATE_FAILED;
                                } else {
                                    Ch0Delay.value = MAX_DELAY-CmdVal.value;
                                    sendInt(Ch0Delay.msb);
                                    sendInt(Ch0Delay.lsb);
                                    Flags1.update = 1;
                                    state = STATE_PROCESSED;
                                }
                            } else if (Cmd == CMD_SET_CH1) {
                                if (Flags1.update || CmdVal.value > MAX_DELAY) {
                                    state = STATE_FAILED;
                                } else {
                                    Ch1Delay.value = MAX_DELAY-CmdVal.value;
                                    sendInt(Ch1Delay.msb);
                                    sendInt(Ch1Delay.lsb);
                                    Flags1.update = 1;
                                    state = STATE_PROCESSED;
                                }
                            }
                            break;
                    }
                    RcvCnt++;
                }
            }
        } else if (RC1STAbits.OERR) {
            sendChar(CMD_OVERRUN);
            RC1STAbits.CREN = 0;
            RC1STAbits.CREN = 1;
        }
        
        if (state == STATE_PROCESSED) {
            sendChar(CMD_ACK);
            RcvCnt = 0;
        } else if (state == STATE_FAILED) {
            sendChar(CMD_NACK);
            RcvCnt = 0;
        } else if (state == STATE_FRAME_ERROR) {
            sendChar(CMD_FRAME_ERROR);
            RcvCnt = 0;
        }
    }
    
    return 0;
}
