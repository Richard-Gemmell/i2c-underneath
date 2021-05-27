// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_FAKES_COMMON_HAL_FAKE_TIMESTAMP_H
#define I2C_UNDERNEATH_FAKES_COMMON_HAL_FAKE_TIMESTAMP_H

#include "common/hal/timestamp.h"

namespace common {
namespace hal {

class FakeTimestamp : public Timestamp {
public:
    ~FakeTimestamp() override = default;

    void set_time_passed(uint32_t elapsed_nanos) {
        elapsed_nanos_ = elapsed_nanos;
    }

    uint32_t get_time_passed() const {
        return elapsed_nanos_;
    }

    void reset() override {
        elapsed_nanos_ = 0;
    }

    bool timed_out_nanos(uint32_t timeout_in_nanos) override {
        return elapsed_nanos_ >= timeout_in_nanos;
    }

private:
    uint32_t elapsed_nanos_ = 0;
};

}
}

#endif //I2C_UNDERNEATH_FAKES_COMMON_HAL_FAKE_TIMESTAMP_H
