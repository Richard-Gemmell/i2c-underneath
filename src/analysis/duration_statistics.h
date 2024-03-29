// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#pragma once

#include <cstdint>
#include <common/specifications/i2c_specification.h>

namespace analysis {
class DurationStatistics : public Printable {
public:
    // Adds another duration to the statistics
    void include(uint32_t duration);

    inline uint32_t count() const {
        return count_;
    }

    inline uint32_t min() const {
        return min_;
    }

    inline uint32_t max() const {
        return max_;
    }

    uint32_t average() const;

    bool meets_specification(const common::i2c_specification::TimeRange& timeRange) const;

    size_t printTo(Print& p) const override;

private:
    uint32_t count_ = 0;
    uint32_t min_ = UINT32_MAX;
    uint32_t max_ = 0;
    double total_ = 0;
};
}
