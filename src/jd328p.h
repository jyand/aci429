#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#define DELAY 500U

#define SELECT PINB2
#define MOSI PINB3
#define MISO PINB4
#define SCK PINB5
#define RXFLAG PIND0
#define TXFLAG PIND1
#define HWMR PINC1

#define CWDTPSC 0xF0U
#define SWDTPSC 0x29U // 8s timeout

// TODO: MCUCR?
void Initialize() {
        /* I/O pins direction */
        DDRB |= (1 << SCK) | (1 << MOSI) | (1 << SELECT) ;
        DDRB &= ~(1 << MISO) ;
        DDRC |= (1 << TXFLAG) | (1 << RXFLAG) ;
        DDRC &= ~(1 << HWMR) ;
        /* watchdog timer configuration */
        MCUSR &= ~(1 << WDRF) ;
        WDTCSR |= WDCE ;
        WDTCSR &= CWDTPSC;
        WDTCSR |= SWDTPSC;
        /* SPI settings */
        SPCR |= (1 << SPIE) | (1 << SPE) | (1 << MSTR) | (1 << SPR0) ;
        SPCR &= ~(1 << SPR1) ;
        SPSR |= (1 << SPI2X) ; //with 16MHz ext osc, clear instead to get 1MHz
        SoftMasterReset() ;
}
