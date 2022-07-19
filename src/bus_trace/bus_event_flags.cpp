// Copyright © 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include <type_traits>
#include "bus_event.h"

namespace bus_trace {

BusEventFlags operator|(BusEventFlags lhs, BusEventFlags rhs) {
    return static_cast<BusEventFlags>(
            static_cast<std::underlying_type_t<BusEventFlags>>(lhs) |
            static_cast<std::underlying_type_t<BusEventFlags>>(rhs)
            );
}

BusEventFlags operator&(BusEventFlags lhs, BusEventFlags rhs) {
    return static_cast<BusEventFlags>(
            static_cast<std::underlying_type_t<BusEventFlags>>(lhs) &
            static_cast<std::underlying_type_t<BusEventFlags>>(rhs)
    );
}
}