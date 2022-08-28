// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include <cmath>
#include <algorithm>
#include <Print.h>
#include "duration_statistics.h"

void analysis::DurationStatistics::include(uint32_t duration) {
    total_ += duration;
    count_ += 1;
    min_ = std::min(min_, duration);
    max_ = std::max(max_, duration);
}

uint32_t analysis::DurationStatistics::average() const {
    return std::llround(total_ / count_);
}

bool analysis::DurationStatistics::meets_specification(const common::i2c_specification::TimeRange& timeRange) const {
    return (timeRange.min <= min()) && (max() <= timeRange.max);
}

size_t analysis::DurationStatistics::printTo(Print& p) const {
    size_t count =  p.print("Avg ");
    count += p.print(average());
    count += p.print(" (");
    count += p.print(min());
    count += p.print(" - ");
    count += p.print(max());
    count += p.println(")");
    return count;
}
