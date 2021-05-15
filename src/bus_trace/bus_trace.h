// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_TRACE_H
#define I2C_UNDERNEATH_BUS_TRACE_H

#include <cstdint>
#include <cstddef>

// Events are recorded when either a bus line level changes or a bus
// control pin changes from floating to pull-down. The time delay
// between a pin change and a line change gives the bus rise and fall time.
//
// Pin values and changes are not recorded if we're just spying on the
// bus.
// Bit 7 - SDA Pin Changed
// Bit 6 - SCL Pin Changed
// Bit 5 - SCL Pin Value
// Bit 4 - SDA Pin Value
// Bit 3 - SCL Line Changed
// Bit 2 - SDA Line Changed
// Bit 1 - SCL Line Value
// Bit 0 - SDA Line Value
typedef uint8_t BusEventFlags;

// Records a change to the state of the bus.
class BusEvent {
    // uint32_t timestamp_in_ticks;
    // The time since the recording started in processor ticks.
    // allows absolute ticks. Limits the maximum duration of a single trace to 2^32 ticks.
    // SMBus time out is 35 ms. Which is 35,000,000 nanos. A 3 byte uint gives 16,777,215 ticks.
    // 2 bytes between events gives 65536 ticks which is 100,000 nanos. Enough for a 5 KHz baud rate. SMBus is minimum of 10 kHz
    uint16_t delta_t_in_ticks;  // ticks since previous event

    // Describes the event that happened and the state of the bus lines.
    BusEventFlags flags;

    uint8_t unused; // Pack byte.
};

// A single array of BusEvents contains all level changes
// and moments when a pin is set or cleared by this driver.
// 30 to 60 bytes per byte of I2C message
class BusTrace {
public:
    // Calculates the maximum number of 'BusEvent' objects required
    // to describe an I2C message.
    // Set 'include_pin_events' to true if the system is driving the bus
    // pins. Set it false if the the system is just sniffing a bus.
    static size_t max_events_required(uint32_t bytes_per_message, bool include_pin_events) {
        size_t events_per_bit = 2 * 2; // 2 for SCL. 0-2 for SDA
        if (include_pin_events) {
            events_per_bit *= 2;    // If we're driving the pins then there's an extra event per edge.
        }
        size_t events_per_byte = 9 * events_per_bit;  // 8 bits + the ACK
        return (2 * events_per_bit) +  // Allow for START and STOP bits
               (events_per_byte * (bytes_per_message + 1)); // Allow for address byte
    }

    uint32_t start_millis;  // Absolute time that the trace started
    uint32_t start_ticks;   // Tick count at start

    // The events. Max 4 events per bit
    // TODO: Maybe we should pass this array to the constructor so it can be a compile time object
    // Alternatively we may need to 'new' it.
    BusEvent events[];
};

// Records the electrical activity on I2C bus.
// A trace cannot last longer than 2^32 processor cycles.
// The trace is driven by interrupts so it will continue
// recording while the processor performs other work.
class BusTracer {
    // TODO: how big should `events[]` be?
    // Should the array size be a compile time constant?
    //  - constant is Ok if it's fairly small
    //  - if
    //  - makes it hard for the caller to compare traces unless there's a copy constructor
    // Should the caller provide the array?
    // We could use a std::vector()

    // Starts recording
    // The recording stops automatically when the trace is full
    void start(BusTrace& trace);

    // Returns true if we're recording
    bool is_recording();

    // Stops recording
    void stop();
};

#endif //I2C_UNDERNEATH_BUS_TRACE_H
