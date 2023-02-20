#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#define DELAY 500U

#define SELECT PINC4
#define MISO PINB0
#define MOSI PINB1
#define SCK PINB7
#define RXFLAG PINB3
#define TXFLAG PINB4
#define HWMR PINC5

#define CWDTPSC 0xF0U
#define SWDTPSC 0x29U // 8s timeout

// TODO: MCUCR?
void Initialize() {
        //PORTC |= (1 << HWMR) ;
        //_delay_ms(DELAY) ;
        /* I/O pins direction */
        DDRB |= (1 << SCK) | (1 << MOSI) ;
        DDRB &= ~((1 << MISO) | (1 << RXFLAG) | (1 << TXFLAG)) ;
        DDRC |= (0x3U << SELECT) ; // CS and MR
        /* watchdog timer configuration */
        MCUSR &= ~(1 << WDRF) ;
        WDTCSR |= WDCE ;
        WDTCSR &= CWDTPSC;
        WDTCSR |= SWDTPSC;
        /* SPI settings */
        SPCR |= (1 << SPIE) | (1 << SPE) | (1 << MSTR) | (1 << SPR0) ;
        SPCR &= ~(1 << SPR1) ;
        SPSR |= (1 << SPI2X) ;
        SoftMasterReset() ;
        //_delay_ms(DELAY) ;
        //PORTC &= ~(1 << HWMR) ;
}

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
