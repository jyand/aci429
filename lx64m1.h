#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#define DELAY 500U

#define SELECT PINC4 //PINC1
#define MISO PINB0
#define MOSI PINB1
#define SCK PINB7
#define RXFLAG PINB3
#define TXFLAG PINB4
#define HWMR PINC5

#define CWDTPSC 0xF0U
#define SWDTPSC 0x09U // 8s timeout

const unsigned char MASTER_RESET = 0x01 ;
const unsigned char RESET_ALL_LABELS = 0x02 ;
const unsigned char SET_ALL_LABELS  = 0x03 ;
const unsigned char RESET_LABEL = 0x04 ;
const unsigned char SET_LABEL = 0x05 ;
const unsigned char LABEL_PATTERN = 0x06 ;
const unsigned char WRITE_ACLK_DIV = 0x07 ;
const unsigned char RX_DEQUEUE = 0x08 ;
const unsigned char READ_STATUSREG = 0x0A ;
const unsigned char READ_CTLREG = 0x0B ;
const unsigned char READ_ACLK_DIV = 0x0C ;
const unsigned char READ_LABELTABLE = 0x0D ;
const unsigned char TX_ENQUEUE = 0x0E ;
const unsigned char WRITE_CTLREG = 0x10 ;
const unsigned char RESET_TX_FIFO = 0x11  ;
const unsigned char TX_EN = 0x12 ;

struct HI3585_t  {
        unsigned char clkdiv ;
        unsigned char statusreg ;
        unsigned short ctlreg ;
} ;

/*void TimerInit() {
        unsigned long period = 5 ;
        TCNT0 = 0x0 ;
        TIMSK1 = 0x0 ;
        TCC1A = 0x0 ;
        TCCR1B = 0x0 ;
        OCR0A = (unsigned char)(period*(F_CPU/(1024UL*1000UL))+1) ;
        TIMSK0 = 0x2 ;
        TCCR0A = 0x2 ;
        TCCR0B = 0x5 ;
}*/
