// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_RECORDER_H
#define I2C_UNDERNEATH_BUS_RECORDER_H

#include "bus_trace.h"
#include <common/hal/pin.h>
#include <common/hal/clock.h>

namespace bus_trace {

// Records the electrical activity on I2C bus.
// A trace cannot last longer than 2^32 processor cycles.
// The trace is driven by interrupts so it will continue
// recording while the processor performs other work.
//
// WARNING It takes the pin interrupt approximately 170 nanoseconds to
// add an event to the trace. Only one interrupt can be handled
// at once. This means that the recorder will not record the
// correct time for edges that are less than 200 nanos apart.
// They will appear as if one followed the other by 200 nanos.
// In some cases, this can make events appear to have happened
// in the wrong order. e.g. when SDA drops 5 ns after SCL. You can
// solve this by swapping the pin assignments round.
//
// Other interrupts can also affect the results. It's best to avoid
// having any when recording.
class BusRecorder {
public:
    BusRecorder(common::hal::Pin& sda, common::hal::Pin& scl, const common::hal::Clock& clock);

    // Stops any recording that's in progress and then
    // starts recording. Bus events are added to 'trace'.
    // The recording stops automatically when the trace is full.
    void start(BusTrace& trace);

    // Stops recording
    void stop();

    // Returns true if we're recording
    bool is_recording() const;

private:
    common::hal::Pin& sda;
    common::hal::Pin& scl;
    const common::hal::Clock& clock;    // Provides system time
    uint32_t ticks_at_latest_event = 0;
    BusTrace* current_trace = nullptr;
    bool sda_high = false;
    bool scl_high = false;

    void on_change(BusEventFlags edges);
};

} // bus_trace

#endif //I2C_UNDERNEATH_BUS_RECORDER_H
