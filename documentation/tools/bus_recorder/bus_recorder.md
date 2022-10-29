# Bus Recorder

## Introduction
The [BusRecorder](../../../src/bus_trace/bus_recorder.h) class records activity
on the I2C bus lines; SDA and SCL. This record forms a
[BusTrace](../../../src/bus_trace/bus_trace.h).

The `BusRecorder` adds a [BusEvent](../../../src/bus_trace/bus_event.h) to the
`BusTrace` each time one of the lines changes from high to low or vice-versa.

The `BusEvent` tells you whether the lines ended up high or low and which line
changed. It also records how much time passed since the previous event.

Once you've got a trace, you can analyse it to find out what happened.
See [Analysing BusRecorder Traces](analysing_traces.md) for details.

## Usage
`BusRecorder` works with I2C speeds of 1 MHz or slower. (Standard-mode,
Fast-mode and Fast-mode Plus)

You can record a bus in the background while your Teensy does other things.
You can even record a bus that's being driven by the same device. You can't
use the same 2 pins to drive the I2C bus and record it at the same time though.
You'll need 4 pins in this case; 2 for I2C and 2 for recording.

You can record the bus for as long as you like. The only limitation is
the amount of memory required by the [BusTrace](../../../src/bus_trace/bus_trace.h).
Events are discarded once the trace is full. You can stop and start
recording as often as you like.

### Choosing the Pins
'BusRecorder' requires a matched pair of pins to watch the I2C bus.
`start()` will return an error code if the combination is not valid.

You can take any pair of pins from one of these lists:
* 0, 1, 24, 25
* 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 26, 27, 38, 39, 40, 41
* 6, 9, 10, 11, 12, 13, 32
* 7, 8, 34, 35, 36, 37
* 42, 43, 44, 45
* 28, 30, 31, 46, 47
* 2, 3, 4, 5, 33
* 29, 48, 49, 50, 51, 52, 53, 54

So pins 6 and 11 will be Ok but pins 6 and 7 won't work.

You can check if a pair of pins will work without a BusRecorder.
The pins are valid if and only if:

`getSlowIRQ(pin_sda) == getSlowIRQ(pin_scl)`

`getSlowIRQ()` is defined in `super_fast_io.h`.

This is because the pins in each list use the same IRQ.

### Recording Multiple Buses
You can record different I2C buses at the same time.
* create a separate `BusRecorder` for each bus
* the pins for each recorder MUST come from different lists

e.g. Lets say you want to record 2 buses, and you decide to use pins 20 and 21
for the first bus. You can't use pins 15 and 16 for the second bus because
they're in the same list. You can use pins 2 and 4 though.

Recording multiple buses at the same time will affect the timing accuracy.
It's unlikely to work well at 1 MHz. It should be fine at 100k or 400k though.
See [Specification](#specification) for more information.

### Limitations
Edges that are very close (< 100 nanosecond apart) are treated as if
they happened simultaneously.

Trains of very close edges are recorded as 1 edge if the final state
is different to the starting state. For example, if SDA goes from HIGH
to LOW to HIGH to LOW then the recorder will record a single HIGH to LOW
event.

Likewise, trains of glitches are recorded as a single glitch.
See [Glitches](#glitches).

A glitch on one line is ignored if the other line changed state at the same
time. e.g. if SDA changes from HIGH to LOW to HIGH immediately before
SCL changes from LOW to HIGH then the recorder discards the change to SDA.
It just records that SCL rose.

Timings are affected by high priority interrupts including from other
`BusRecorder` instances.

Edges that are more than 200 nanoseconds apart will be recorded faithfully.

## Specification

### CPU Budget
It takes approximately 130 nanoseconds for `BusRecorder` to record a rising
or falling edge on one of the I2C bus lines. Each I2C bit requires 2 edges
for the SCL clock pulse and 1 or 2 more to change SDA.

Events are recorded with an interrupt service routine (ISR). Your application
code and other interrupts are suspended while it runs.

This table shows the proportion of the Teensy's CPU time need to record
an I2C bus. As you can see, it's negligible for Standard-mode I2C running
at 100 kHz but quite significant for Fast-mode Plus at 1 MHz.

| I2C Clock | Min CPU Use | Max CPU Use |
|-----------|-------------|-------------|
| 100 kHz   | 3%          | 5%          |
| 400 kHz   | 9%          | 18%         |
| 1 MHz     | 23%         | 45%         |

This only applies when you're recording and there's an I2C message is in flight.
There's no CPU cost in the gap between I2C messages. There's no CPU load
if you stop recording.

### Timing Accuracy
The [BusTrace](../../../src/bus_trace/bus_trace.h) gives you the time between
each event. See `nanos_to_previous()` etc. These durations are usually accurate
to within a few nanoseconds.

The times are not accurate in some circumstances.

#### "Simultaneous" Edges
If both I2C lines change within a very short interval then they appear
to have changed simultaneously to the `BusRecorder`. It records a single
event which shows changes to both lines. It's not possible to say which
edge happened first.

This definitely happens with edges less than 20 nanoseconds apart. Edges
separated by more than 65 nanoseconds should be recorded as separate events.

Unfortunately, it's quite common for I2C slave devices to drop the SDA line
as soon as they see SCL go low. This can happen in less than 10 nanoseconds.
Analysis tools can compensate for this by assuming that simultaneous events
happened in the correct order. They should warn the user though as they
can't guarantee that the I2C protocol was followed correctly. (The order
of edges is critical.)

#### Glitches
A "glitch" is a short spike or dip on one of the lines. For example, SDA might
drop from HIGH to LOW and back again in 10 nanoseconds.

Glitches can happen on SDA when control passes from one device to the other.
For example, if the master has pulled SDA LOW and wants to pass control to
the slave then it'll release SDA causing it to rise. If the slave takes
control 20 nanoseconds later and decides to pull SDA LOW again then you get
a 20 nanosecond pulse from LOW to HIGH and back again.

Glitches are often caused by electrical noise. This is especially true about
SCL as control of SCL remains with the master device throughout the I2C
transaction.

The `BusRecorder` records most glitches. It takes about 200 nanoseconds each
one. This includes both edges. There's no way for the recorder to know
how time passed between the 2 edges of the glitch, so it records the interal
as 0 nanoseconds.

A train of glitches in rapid succession will be recorded as a single glitch.
(Think of a 20 nanosecond square wave.)

The recorder will ignore a glitch on one line when the other line changes
state at the same time. For example, if SDA toggles from LOW to HIGH and back
to LOW 20 nanoseconds before SCL goes from LOW to HIGH then the recorder
will ignore the glitch. It'll just record that SCL rose.

#### Edges During an Interrupt
An I2C line may change state whilst the BusRecorder is handling the interrupt
for the previous edge. If it doesn't see the edges as being
[simultaneous](#simultaneous-edges), then it'll fire the ISR again to record
the second edge as soon possible. This makes the recorded interval between
the edges a bit too high. The error will be less than 130 nanoseconds and is
likely to be less than 80 nanos.

This effect ripples on to the next edge. Its interval will appear slightly
too low by the same amount.

#### Delayed Interrupt Service Routines
Anything that delays or interrupts the BusRecorder's ISR will affect the
timing accuracy. The error will appear as jitter as described
[above](#edges-during-an-interrupt).

This may happen if your application has interrupts with a higher priority
than the BusRecorder. IRQs are usually set with the default priority.
The BusRecorder's IRQ is boosted slightly so this problem should be unusual.

You'll definitely encounter this problem if you run more than 1 BusRecorder
on the same Teensy. It won't matter much if you're recording a 100 kHz bus
but the faster the bus speed becomes the worse the timings will get.

Note that it's only the interval between edges that's affected. The order
of the events for a particular trace will still be correct.

## Appendix A - Interrupt Service Routine Timings
This table breaks down the time taken for the system to detect an edge
on an I2C line and to record the edge with the `BusRecorder`.

The Duration column represents the time taken in nanoseconds to complete that
line. For example, it takes 27 nanoseconds to create and save the `BusEvent`. 

| Event                         | Executing                                        | Duration |
|-------------------------------|--------------------------------------------------|----------|
| GPIO detects edge on I2C line | Application code                                 |          |
| Preparing to call interrupt   | Application code                                 | 16       |
| Application interrupted       | Calling interrupt service routine (ISR)          | 27       |
| ISR starts executing          | Reading line states and clearing interrupt flags | 29       |
|                               | Creating and saving `BusEvent` record            | 27       |
| ISR finished                  | Exiting interrupt                                | 33       |
| Returned to application       | Application code                                 |          |

* total elapsed time is 130 nanoseconds
* 114 nanoseconds to run ISR
* it takes 200 nanoseconds to record a "glitch"

The BusRecorder itself only takes about 27 nanoseconds to record the event.
The vast majority of the time is taken by the business of calling an interrupt.
