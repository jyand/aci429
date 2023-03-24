#include "aci429.h"

unsigned char ExtractLabel(unsigned long word) {
        return word & 0xFF ;
}

unsigned char ExtractSDI(unsigned long word) {
        auto const unsigned char SDI_FIELD_START = 8 ;
        auto const unsigned long SDI_FIELD_MASK = 0xF ;
        return (word >> SDI_FIELD_START) & SDI_FIELD_MASK ;
}

unsigned long ExtractData(unsigned long word) {
        auto const unsigned char DATA_FIELD_START = 10 ;
        auto const unsigned char DATA_FIELD_MASK = 0x7FFFF ;
        return (word >> DATA_FIELD_START) & DATA_FIELD_MASK ;
}

uint8_t CheckParityBit(unsigned long word) {
        if ((int32_t)word < 0) {
                return 1 ;
        } else {
                return 0 ;
        }
}
