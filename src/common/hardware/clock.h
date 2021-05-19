// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_COMMON_CLOCK_H
#define I2C_UNDERNEATH_COMMON_CLOCK_H

#include <cstdint>

class Clock
{
public:
    virtual ~Clock()

    // Milliseconds since the device started running the current program
//    uint32_t millis();

    // Milliseconds since the device started running the current program
//    uint32_t micros();
};

#endif //I2C_UNDERNEATH_COMMON_CLOCK_H
