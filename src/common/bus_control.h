// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_CONTROL_H
#define I2C_UNDERNEATH_BUS_CONTROL_H

#include <cstdint>
#include <functional>
#include "../common/hardware/pin.h"

namespace common {

enum class BusActionTrigger : uint8_t {
    TraceStarted,   // Started recording the trace
    SDA_Rose,
    SDA_Fell,
    SCL_Rose,
    SCL_Fell
};

enum class BusAction : uint8_t {
    None = 0,
    SDA_High,
    SDA_Low,
    SDL_High,
    SCL_Low
};

class BusActionStep {
    // The step waits until a new instance of this event occurs
    // and then starts a timer. When delay_ticks has passed it
    // performs the action.
    // The next step starts as soon as the action is performed
    BusActionTrigger trigger_event;

    // Time to wait after trigger_event before performing the action
    uint32_t delay_ticks;

    // What we should do. Maybe nothing at all.
    BusAction action;
};

/* Sample Master Sequence
    // Do we need a pre-step to check that the bus is free?
        - maybe that's always done for a Master sequence
    Start Bit
        (START) TraceStarted, 10'000, SDA_Low
            - the delay was picked so there's a pause before the transmission begins
            - bus_free_time (tBUF) would be a good time to use by default
        (Start Hold) SDA_Fell, start_hold_time (tHD;STA), SCL_Low
    Data Bit 1
        ( Set SDA) SCL_Fell, data_hold_time (tHD;DAT), SDA_High
            - data_valid_time == data_hold_time + scl_fall_time
        (SCL High) SDA_Rose, (clock_low_time - data_hold_time) (tLOW - tHD;DAT), SCL_HIGH
            - tLOW only has a minimum so we honour the spec although
              the actual clock low time will be a little high (by the SDA fall time)
        (SCL Low) SCL_Rose, clock_high_time (tHIGH), SCL_LOW
    ...
 */
/* Sample Slave Read Sequence
    // Do we need a pre-step to check that the bus is free?
        - maybe that's always done for a Master sequence
    Wait for Start Bit
        (START) SDA_Fell, 0, BusActionTrigger::None
        (Start Hold) SCL_Fell, 0, BusActionTrigger::None
    Wait for Data Bit 1
        (Bit Started) SCL_Rose, 0, BusActionTrigger::None
            - failing to get a Rise and a Fall => stuck clock and we should time out
        (Bit Started) SCL_Fell, 0, BusActionTrigger::None
    ...
    Wait for Data Bit 8
        (Bit Started) SCL_Rose, 0, BusActionTrigger::None
    ACK byte
        (Set SDA) SCL_Fell, data_valid_acknowledge_time (tVD;ACK), SDA_Low
        (Ack Hold) SCL_Rose, 0, BusActionTrigger::None
        (Ack Release) SCL_Fell, data_hold_time (tHD;DAT), SDA_High
            - make this longer to stretch the clock (ACK clock stretch)
 */

// A pattern for an I2C message.
class BusMessagePrototype {
    BusActionStep steps[];
};

// Reads and writes signals on the bus.
// Should baud_rate be a property of BusControl?
//      - in which case setup and hold times should possibly be related to baud rate
class BusControl {
public:
    BusControl(common::hardware::Pin& data_pin, common::hardware::Pin& clock_pin, uint32_t baud_rate);

    // 'set_up_nanos': data set-up time (tSU;STA)
    void start(uint32_t set_up_nanos, uint32_t hold_nanos);

    // 'set_up_nanos': data set-up time (tSU;STO)
    void stop(uint32_t set_up_nanos);

    // Shortest possible pulse is 1/8th of 1/2 of 5 MHz == 0.2 micros / 16 == 125 nanos
    // 'set_up_nanos': data set-up time (tSU;DAT)
    // 'hold_nanos': data hold time (tHD;DAT)
    void write_bit(bool one, uint32_t set_up_nanos, uint32_t hold_nanos);

    // START detected
    void on_start(std::function<void()> callback);

    // STOP or repeated START detected
    void on_stop(std::function<void()> callback);

    // Read a bit value
    void on_bit(std::function<void(bool one)> callback);

    // Attempts to clear the bus by toggling SCL repeatedly
    // This will not work if SCL is stuck low. In that case you'll
    // have to reset each device electronically.
    // See section 3.1.16 of the I2C Specification, revision 6.
    void clear_bus();
};

}

#endif //I2C_UNDERNEATH_BUS_CONTROL_H
