// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_FAKE_SERIAL_H
#define I2C_UNDERNEATH_FAKE_SERIAL_H

#include <Print.h>

class FakeSerial : public Print {
public:
    FakeSerial() = default;

    size_t write(uint8_t b) override {
        buffer.append((char)b);
        return 1;
    }

    int strcmp(const String& other) {
        return buffer.compareTo(other);
    }

    const String& get_string() {
        return buffer;
    }

private:
    String buffer;
};

#endif //I2C_UNDERNEATH_FAKE_SERIAL_H
