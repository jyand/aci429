#include "lx64m1.h"
#include "aci429.h"
#include "ecsctl.h"

const unsigned short CTL_REG_INITIAL = 0xE838U ;//0x603C ;

unsigned long TxQueue[32] ;
unsigned long RxQueue[32] ;

volatile unsigned char LED_1_ = (1 << PINB5) ;
volatile unsigned char LED_2_ = (1 << PINB6) ;

struct ECSdata g_ecsdat ;

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
        SPDR = (char)(halfword >> 8) ;
        SPIWait() ;
        dum = SPDR ;
        SPDR = (char)(halfword & 0xFF) ;
        SPIWait() ;
        dum = SPDR ;
        PORTC |= (1 << SELECT) ;
}

void TestSetTemps() {
        for (uint8_t j = 0 ; j < 16 ; ++j) {
                TxQueue[j+16] = CabinZoneSetPoint(60 + j) ;
        }
}

void TestFanSpeeds() {
        for (uint8_t j = 0 ; j < 16 ; ++j) {
                TxQueue[j] = SetCabinFanSpeed(j) ;
        }
}

void DummyTxData() {
        unsigned long k = 0 ;
        for (uint8_t j = 0 ; j < 32 ; ++j) {
                k++ ;
                if ((k & 0x0F) == 0x0A) {
                        TxQueue[j] = (k << 24) + (k << 16) + (k << 8) + k ;
                }
        }
        eeprom_write_word((uint16_t*)0x20, 0xBEEF) ;
        eeprom_write_block((const void*)0x50, (void*)TxQueue, sizeof(TxQueue)) ;
}

void WriteTxFIFO(uint8_t numwords) {
        if (numwords > 32) {
                numwords = 32 ;
        }
        PORTC &= ~(1 << SELECT) ;
        unsigned char dum = SPSR ;
        SPDR = TX_ENQUEUE ;
        SPIWait() ;
        dum = SPDR ;
        for (uint8_t j = 0 ; j < numwords ; ++j) {
                SPDR = (char)(TxQueue[j] >> 24) ;
                SPIWait() ;
                dum = SPDR ;
                SPDR = (char)((TxQueue[j] >> 16) & 0xFF) ;
                SPIWait() ;
                dum = SPDR ;
                SPDR = (char)((TxQueue[j] >> 8) & 0xFF) ;
                SPIWait() ;
                dum = SPDR ;
                SPDR = (char)(TxQueue[j] & 0xFF) ;
                SPIWait() ;
                dum = SPDR ;
        }
        PORTC |= (1 << SELECT) ;
}

unsigned long ReadRxFIFO() {
        unsigned long word = SPSR ;
        PORTC &= ~(1 << SELECT) ;
        SPDR = RX_DEQUEUE ;
        SPIWait() ;
        SPDR = 0 ;
        SPIWait() ;
        word = (long)SPDR << 24 ;
        SPDR = 0 ;
        SPIWait() ;
        word |= (long)SPDR << 16 ;
        SPDR = 0 ;
        SPIWait() ;
        word |= (long)SPDR << 8 ;
        SPDR = 0 ;
        SPIWait() ;
        word |= (long)SPDR ;
        PORTC |= (1 << SELECT) ;
        return word ;
}

void ProcessRx() {
        unsigned char n = 1 ;
        switch (ReadStatusReg() & 0x7U) {
                case 4:
                        n = 32 ;
                        break ;
                case 2:
                        n = 16 ;
                        break ;
                case 0:
                        n = 1;
                        break ;
                default:
                        break ;
        }
        for (uint8_t j = 0 ; j < n ; ++j) {
               RxQueue[j] = ReadRxFIFO() ;
        }
}

/* decode received 429 word */
void DecodeRx(unsigned long word) {
        unsigned char lbl = ExtractLabel(word) ;
        switch (lbl) {
                case LABEL101:
                        g_ecsdat.cabinfanspeed = (word >> 15) & 0xF ;
                        g_ecsdat.ecsmode = (word >> 8) & 0x7 ;
                        g_ecsdat.compressorstatus = (word >> 20) & 0x1 ;
                        break ;
                case LABEL103:
                        g_ecsdat.temp_setpoint = (word >> 13) & 0xFFFF ;
                        break ;
                case LABEL105:
                        g_ecsdat.temp_setpoint = (word >> 13) & 0xFFFF ;
                        break ;
                default:
                        break ;
        }
}

void Parrot() {
        for (uint8_t j = 0 ; j < sizeof(RxQueue) ; ++j) {
                TxQueue[j] = RxQueue[j] ;
        }
}

void main() __attribute__((noreturn)) ;

int main() {
        asm("cli ; wdr ;") ;
        Initialize() ;
        WriteACLKDiv(1) ;
        WriteCtlReg(CTL_REG_INITIAL) ;
        TestSetTemps() ;
        TestFanSpeeds() ;
        WriteTxFIFO(30) ;
        for(;;) {
                asm("wdr ;") ;
                WriteTxFIFO(sizeof(TxQueue)) ;
                _delay_ms(DELAY) ;
                ProcessRx() ;
                _delay_ms(DELAY) ;
                Parrot() ;
                _delay_ms(DELAY) ;
                sleep_disable() ;
        }
}
