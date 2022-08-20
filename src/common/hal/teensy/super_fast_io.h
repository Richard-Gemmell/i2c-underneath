// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)
// Elements of this file are based on Teensyduino Core Library core_pins.h.

#pragma once
#include <core_pins.h>

// Write the correct bitmask to the returned port to set or clear the digital pin.
// This is slightly faster than digitalWriteFast() but is rarely worth the bother.
static inline volatile uint32_t* getDigitalWritePort(uint8_t pin, bool set) {
    if (__builtin_constant_p(pin)) {
        if (pin == 0) return set ? &CORE_PIN0_PORTSET : &CORE_PIN0_PORTCLEAR;
        if (pin == 1) return set ? &CORE_PIN1_PORTSET : &CORE_PIN1_PORTCLEAR;
        if (pin == 2) return set ? &CORE_PIN2_PORTSET : &CORE_PIN2_PORTCLEAR;
        if (pin == 3) return set ? &CORE_PIN3_PORTSET : &CORE_PIN3_PORTCLEAR;
        if (pin == 4) return set ? &CORE_PIN4_PORTSET : &CORE_PIN4_PORTCLEAR;
        if (pin == 5) return set ? &CORE_PIN5_PORTSET : &CORE_PIN5_PORTCLEAR;
        if (pin == 6) return set ? &CORE_PIN6_PORTSET : &CORE_PIN6_PORTCLEAR;
        if (pin == 7) return set ? &CORE_PIN7_PORTSET : &CORE_PIN7_PORTCLEAR;
        if (pin == 8) return set ? &CORE_PIN8_PORTSET : &CORE_PIN8_PORTCLEAR;
        if (pin == 9) return set ? &CORE_PIN9_PORTSET : &CORE_PIN9_PORTCLEAR;
        if (pin == 10) return set ? &CORE_PIN10_PORTSET : &CORE_PIN10_PORTCLEAR;
        if (pin == 11) return set ? &CORE_PIN11_PORTSET : &CORE_PIN11_PORTCLEAR;
        if (pin == 12) return set ? &CORE_PIN12_PORTSET : &CORE_PIN12_PORTCLEAR;
        if (pin == 13) return set ? &CORE_PIN13_PORTSET : &CORE_PIN13_PORTCLEAR;
        if (pin == 14) return set ? &CORE_PIN14_PORTSET : &CORE_PIN14_PORTCLEAR;
        if (pin == 15) return set ? &CORE_PIN15_PORTSET : &CORE_PIN15_PORTCLEAR;
        if (pin == 16) return set ? &CORE_PIN16_PORTSET : &CORE_PIN16_PORTCLEAR;
        if (pin == 17) return set ? &CORE_PIN17_PORTSET : &CORE_PIN17_PORTCLEAR;
        if (pin == 18) return set ? &CORE_PIN18_PORTSET : &CORE_PIN18_PORTCLEAR;
        if (pin == 19) return set ? &CORE_PIN19_PORTSET : &CORE_PIN19_PORTCLEAR;
        if (pin == 20) return set ? &CORE_PIN20_PORTSET : &CORE_PIN20_PORTCLEAR;
        if (pin == 21) return set ? &CORE_PIN21_PORTSET : &CORE_PIN21_PORTCLEAR;
        if (pin == 22) return set ? &CORE_PIN22_PORTSET : &CORE_PIN22_PORTCLEAR;
        if (pin == 23) return set ? &CORE_PIN23_PORTSET : &CORE_PIN23_PORTCLEAR;
        if (pin == 24) return set ? &CORE_PIN24_PORTSET : &CORE_PIN24_PORTCLEAR;
        if (pin == 25) return set ? &CORE_PIN25_PORTSET : &CORE_PIN25_PORTCLEAR;
        if (pin == 26) return set ? &CORE_PIN26_PORTSET : &CORE_PIN26_PORTCLEAR;
        if (pin == 27) return set ? &CORE_PIN27_PORTSET : &CORE_PIN27_PORTCLEAR;
        if (pin == 28) return set ? &CORE_PIN28_PORTSET : &CORE_PIN28_PORTCLEAR;
        if (pin == 29) return set ? &CORE_PIN29_PORTSET : &CORE_PIN29_PORTCLEAR;
        if (pin == 30) return set ? &CORE_PIN30_PORTSET : &CORE_PIN30_PORTCLEAR;
        if (pin == 31) return set ? &CORE_PIN31_PORTSET : &CORE_PIN31_PORTCLEAR;
        if (pin == 32) return set ? &CORE_PIN32_PORTSET : &CORE_PIN32_PORTCLEAR;
        if (pin == 33) return set ? &CORE_PIN33_PORTSET : &CORE_PIN33_PORTCLEAR;
        if (pin == 34) return set ? &CORE_PIN34_PORTSET : &CORE_PIN34_PORTCLEAR;
        if (pin == 35) return set ? &CORE_PIN35_PORTSET : &CORE_PIN35_PORTCLEAR;
        if (pin == 36) return set ? &CORE_PIN36_PORTSET : &CORE_PIN36_PORTCLEAR;
        if (pin == 37) return set ? &CORE_PIN37_PORTSET : &CORE_PIN37_PORTCLEAR;
        if (pin == 38) return set ? &CORE_PIN38_PORTSET : &CORE_PIN38_PORTCLEAR;
        if (pin == 39) return set ? &CORE_PIN39_PORTSET : &CORE_PIN39_PORTCLEAR;
#if CORE_NUM_DIGITAL >= 55
        if (pin == 40) return set ? &CORE_PIN40_PORTSET : &CORE_PIN40_PORTCLEAR;
        if (pin == 41) return set ? &CORE_PIN41_PORTSET : &CORE_PIN41_PORTCLEAR;
        if (pin == 42) return set ? &CORE_PIN42_PORTSET : &CORE_PIN42_PORTCLEAR;
        if (pin == 43) return set ? &CORE_PIN43_PORTSET : &CORE_PIN43_PORTCLEAR;
        if (pin == 44) return set ? &CORE_PIN44_PORTSET : &CORE_PIN44_PORTCLEAR;
        if (pin == 45) return set ? &CORE_PIN45_PORTSET : &CORE_PIN45_PORTCLEAR;
        if (pin == 46) return set ? &CORE_PIN46_PORTSET : &CORE_PIN46_PORTCLEAR;
        if (pin == 47) return set ? &CORE_PIN47_PORTSET : &CORE_PIN47_PORTCLEAR;
        if (pin == 48) return set ? &CORE_PIN48_PORTSET : &CORE_PIN48_PORTCLEAR;
        if (pin == 49) return set ? &CORE_PIN49_PORTSET : &CORE_PIN49_PORTCLEAR;
        if (pin == 50) return set ? &CORE_PIN50_PORTSET : &CORE_PIN50_PORTCLEAR;
        if (pin == 51) return set ? &CORE_PIN51_PORTSET : &CORE_PIN51_PORTCLEAR;
        if (pin == 52) return set ? &CORE_PIN52_PORTSET : &CORE_PIN52_PORTCLEAR;
        if (pin == 53) return set ? &CORE_PIN53_PORTSET : &CORE_PIN53_PORTCLEAR;
        if (pin == 54) return set ? &CORE_PIN54_PORTSET : &CORE_PIN54_PORTCLEAR;
#endif
    }
    return set ? portSetRegister(pin) : portClearRegister(pin);
}

// Read the port and bitwise or the result with the pin's bitmask to see if it's
// set of not.
// This is slightly faster than digitalReadFast() but is rarely worth the bother.
static inline volatile uint32_t* getDigitalReadPort(uint8_t pin) {
    if (__builtin_constant_p(pin)) {
        if (pin == 0) return &CORE_PIN0_PINREG;
        if (pin == 1) return &CORE_PIN1_PINREG;
        if (pin == 2) return &CORE_PIN2_PINREG;
        if (pin == 3) return &CORE_PIN3_PINREG;
        if (pin == 4) return &CORE_PIN4_PINREG;
        if (pin == 5) return &CORE_PIN5_PINREG;
        if (pin == 6) return &CORE_PIN6_PINREG;
        if (pin == 7) return &CORE_PIN7_PINREG;
        if (pin == 8) return &CORE_PIN8_PINREG;
        if (pin == 9) return &CORE_PIN9_PINREG;
        if (pin == 10) return &CORE_PIN10_PINREG;
        if (pin == 11) return &CORE_PIN11_PINREG;
        if (pin == 12) return &CORE_PIN12_PINREG;
        if (pin == 13) return &CORE_PIN13_PINREG;
        if (pin == 14) return &CORE_PIN14_PINREG;
        if (pin == 15) return &CORE_PIN15_PINREG;
        if (pin == 16) return &CORE_PIN16_PINREG;
        if (pin == 17) return &CORE_PIN17_PINREG;
        if (pin == 18) return &CORE_PIN18_PINREG;
        if (pin == 19) return &CORE_PIN19_PINREG;
        if (pin == 20) return &CORE_PIN20_PINREG;
        if (pin == 21) return &CORE_PIN21_PINREG;
        if (pin == 22) return &CORE_PIN22_PINREG;
        if (pin == 23) return &CORE_PIN23_PINREG;
        if (pin == 24) return &CORE_PIN24_PINREG;
        if (pin == 25) return &CORE_PIN25_PINREG;
        if (pin == 26) return &CORE_PIN26_PINREG;
        if (pin == 27) return &CORE_PIN27_PINREG;
        if (pin == 28) return &CORE_PIN28_PINREG;
        if (pin == 29) return &CORE_PIN29_PINREG;
        if (pin == 30) return &CORE_PIN30_PINREG;
        if (pin == 31) return &CORE_PIN31_PINREG;
        if (pin == 32) return &CORE_PIN32_PINREG;
        if (pin == 33) return &CORE_PIN33_PINREG;
        if (pin == 34) return &CORE_PIN34_PINREG;
        if (pin == 35) return &CORE_PIN35_PINREG;
        if (pin == 36) return &CORE_PIN36_PINREG;
        if (pin == 37) return &CORE_PIN37_PINREG;
        if (pin == 38) return &CORE_PIN38_PINREG;
        if (pin == 39) return &CORE_PIN39_PINREG;
#if CORE_NUM_DIGITAL >= 55
        if (pin == 40) return &CORE_PIN40_PINREG;
        if (pin == 41) return &CORE_PIN41_PINREG;
        if (pin == 42) return &CORE_PIN42_PINREG;
        if (pin == 43) return &CORE_PIN43_PINREG;
        if (pin == 44) return &CORE_PIN44_PINREG;
        if (pin == 45) return &CORE_PIN45_PINREG;
        if (pin == 46) return &CORE_PIN46_PINREG;
        if (pin == 47) return &CORE_PIN47_PINREG;
        if (pin == 48) return &CORE_PIN48_PINREG;
        if (pin == 49) return &CORE_PIN49_PINREG;
        if (pin == 50) return &CORE_PIN50_PINREG;
        if (pin == 51) return &CORE_PIN51_PINREG;
        if (pin == 52) return &CORE_PIN52_PINREG;
        if (pin == 53) return &CORE_PIN53_PINREG;
        if (pin == 54) return &CORE_PIN54_PINREG;
#endif
    }
    return portInputRegister(pin);
}

// Write to a digital port to change it.
// Compare to the result of read port to see if the pin is set or clear.
static inline uint32_t getPortBitmask(uint8_t pin) {
    if (__builtin_constant_p(pin)) {
        if (pin == 0) return CORE_PIN0_BITMASK;
        if (pin == 1) return CORE_PIN1_BITMASK;
        if (pin == 2) return CORE_PIN2_BITMASK;
        if (pin == 3) return CORE_PIN3_BITMASK;
        if (pin == 4) return CORE_PIN4_BITMASK;
        if (pin == 5) return CORE_PIN5_BITMASK;
        if (pin == 6) return CORE_PIN6_BITMASK;
        if (pin == 7) return CORE_PIN7_BITMASK;
        if (pin == 8) return CORE_PIN8_BITMASK;
        if (pin == 9) return CORE_PIN9_BITMASK;
        if (pin == 10) return CORE_PIN10_BITMASK;
        if (pin == 11) return CORE_PIN11_BITMASK;
        if (pin == 12) return CORE_PIN12_BITMASK;
        if (pin == 13) return CORE_PIN13_BITMASK;
        if (pin == 14) return CORE_PIN14_BITMASK;
        if (pin == 15) return CORE_PIN15_BITMASK;
        if (pin == 16) return CORE_PIN16_BITMASK;
        if (pin == 17) return CORE_PIN17_BITMASK;
        if (pin == 18) return CORE_PIN18_BITMASK;
        if (pin == 19) return CORE_PIN19_BITMASK;
        if (pin == 20) return CORE_PIN20_BITMASK;
        if (pin == 21) return CORE_PIN21_BITMASK;
        if (pin == 22) return CORE_PIN22_BITMASK;
        if (pin == 23) return CORE_PIN23_BITMASK;
        if (pin == 24) return CORE_PIN24_BITMASK;
        if (pin == 25) return CORE_PIN25_BITMASK;
        if (pin == 26) return CORE_PIN26_BITMASK;
        if (pin == 27) return CORE_PIN27_BITMASK;
        if (pin == 28) return CORE_PIN28_BITMASK;
        if (pin == 29) return CORE_PIN29_BITMASK;
        if (pin == 30) return CORE_PIN30_BITMASK;
        if (pin == 31) return CORE_PIN31_BITMASK;
        if (pin == 32) return CORE_PIN32_BITMASK;
        if (pin == 33) return CORE_PIN33_BITMASK;
        if (pin == 34) return CORE_PIN34_BITMASK;
        if (pin == 35) return CORE_PIN35_BITMASK;
        if (pin == 36) return CORE_PIN36_BITMASK;
        if (pin == 37) return CORE_PIN37_BITMASK;
        if (pin == 38) return CORE_PIN38_BITMASK;
        if (pin == 39) return CORE_PIN39_BITMASK;
#if CORE_NUM_DIGITAL >= 55
        if (pin == 40) return CORE_PIN40_BITMASK;
        if (pin == 41) return CORE_PIN41_BITMASK;
        if (pin == 42) return CORE_PIN42_BITMASK;
        if (pin == 43) return CORE_PIN43_BITMASK;
        if (pin == 44) return CORE_PIN44_BITMASK;
        if (pin == 45) return CORE_PIN45_BITMASK;
        if (pin == 46) return CORE_PIN46_BITMASK;
        if (pin == 47) return CORE_PIN47_BITMASK;
        if (pin == 48) return CORE_PIN48_BITMASK;
        if (pin == 49) return CORE_PIN49_BITMASK;
        if (pin == 50) return CORE_PIN50_BITMASK;
        if (pin == 51) return CORE_PIN51_BITMASK;
        if (pin == 52) return CORE_PIN52_BITMASK;
        if (pin == 53) return CORE_PIN53_BITMASK;
        if (pin == 54) return CORE_PIN54_BITMASK;
#endif
    }
    return digitalPinToBitMask(pin);
}