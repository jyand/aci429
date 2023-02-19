#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "lx64m1.h"

const unsigned long MASK = 0xFF ;
const unsigned short CTL_REG_INITIAL = 0xE838U ;//0x603C ;
const unsigned char PBIO_DIR = 0xE5 ;

unsigned long TxQueue[32] ;
unsigned long RxQueue[32] ;

volatile unsigned char LED_1_ = (1 << PINB5) ;
volatile unsigned char LED_2_ = (1 << PINB6) ;

void SPIWait() {
        while(!(SPSR & (1 << SPIF))) {}
}

void SoftMasterReset() {
        unsigned char dum = SPSR ;
        PORTC &= ~(1 << SELECT) ;
        SPDR = MASTER_RESET ;
        SPIWait() ;
        dum = SPDR ;
        PORTC |= (1 << SELECT) ;
}

unsigned char ReadACLKDiv() {
        unsigned char byte = SPSR ;
        PORTC &= ~(1 << SELECT) ;
        SPDR = READ_ACLK_DIV ;
        SPIWait() ;
        SPDR = 0 ;
        SPIWait() ;
        byte = SPDR ;
        PORTC |= (1 << SELECT) ;
        return byte ;
}

void WriteACLKDiv(unsigned char divisor) {
        PORTC &= ~(1 << SELECT) ;
        unsigned char dum = SPSR ;
        SPDR = WRITE_ACLK_DIV ;
        SPIWait() ;
        dum = SPDR ;
        SPDR = divisor ;
        SPIWait() ;
        dum = SPDR ;
        PORTC |= (1 << SELECT) ;
}

unsigned char ReadStatusReg() {
        unsigned char byte = SPSR ;
        PORTC &= ~(1 << SELECT) ;
        SPDR = READ_STATUSREG ;
        SPIWait() ;
        SPDR = 0 ;
        SPIWait() ;
        byte = SPDR ;
        PORTC |= (1 << SELECT) ;
        return byte ;
}

unsigned short ReadCtlReg() {
        unsigned char upper, lower ;
        PORTC &= ~(1 << SELECT) ;
        unsigned char dum = SPSR ;
        SPDR = READ_CTLREG ;
        SPIWait() ;
        dum = SPDR ;
        SPDR = 0 ;
        SPIWait() ;
        upper = SPDR ;
        SPDR = 0 ;
        SPIWait() ;
        lower = SPDR ;
        PORTC |= (1 << SELECT) ;
        return (((short)upper) << 8) | (short)lower ;
}

void WriteCtlReg (unsigned short halfword) {
        PORTC &= ~(1 << SELECT) ;
        unsigned char dum = SPSR ;
        SPDR = WRITE_CTLREG  ;
        SPIWait() ;
        dum = SPDR ;
        //SPDR = 0xE8U ;
        SPDR = (char)(halfword >> 8) ;
        SPIWait() ;
        dum = SPDR ;
        //SPDR = 0x38U ;
        SPDR = (char)(halfword & 0xFF) ;
        SPIWait() ;
        dum = SPDR ;
        PORTC |= (1 << SELECT) ;
}

void WriteTxFIFO(uint8_t numwords) {
        if (numwords > 32) {
                numwords = 32 ;
        }
        unsigned char byte = SPSR ;
        PORTC &= ~(1 << SELECT) ;
        SPDR = TX_ENQUEUE ;
        SPIWait() ;
        for (uint8_t j = 0 ; j < numwords ; ++j) {
                SPDR = (char)(TxQueue[j] >> 24) ;
                SPIWait() ;
                SPDR = (char)((TxQueue[j] >> 16) & MASK) ;
                SPIWait() ;
                SPDR = (char)((TxQueue[j] >> 8) & MASK) ;
                SPIWait() ;
                SPDR = (char)(TxQueue[j] & MASK) ;
                SPIWait() ;
        }
        PORTC |= (1 << SELECT) ;
}

unsigned long ReadRxFIFO() {
        unsigned long word = SPSR ;
        PORTC &= ~(1 << SELECT) ;
        SPDR = TX_ENQUEUE ;
        SPIWait() ;
        SPDR = 0 ;
        SPIWait() ;
        word = SPDR << 24 ;
        SPDR = 0 ;
        SPIWait() ;
        word |= SPDR << 16 ;
        SPDR = 0 ;
        SPIWait() ;
        word |= SPDR << 8 ;
        SPDR = 0 ;
        SPIWait() ;
        word |= SPDR ;
        PORTC |= (1 << SELECT) ;
        return word ;
}

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
        SPSR |= (1 << SPI2X) ;
        SoftMasterReset() ;
        //_delay_ms(DELAY) ;
        //PORTC &= ~(1 << HWMR) ;
}

int main() {
        asm("cli ; wdr ;") ;
        struct HI3585_t hi = {} ;
        DDRB |= LED_1_ ;
        PORTB |= LED_1_ ;
        Initialize() ;
        asm("sei ;") ;
        WriteACLKDiv(1) ;
        WriteCtlReg(0xE838) ;
        hi.clkdiv = ReadACLKDiv() ;
        hi.statusreg = ReadStatusReg() ;
        hi.ctlreg = ReadCtlReg() ;
        DDRB |= LED_2_ ;
        PORTB |= LED_2_ ;
        _delay_ms(DELAY) ;
        for(;;) {
                asm("wdr ;") ;
                _delay_ms(DELAY) ;
                sleep_disable() ;
        }
}
