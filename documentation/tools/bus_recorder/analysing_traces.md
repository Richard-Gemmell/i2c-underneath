# Analysing BusRecorder Traces

## Introduction
The [BusRecorder](../../../src/bus_trace/bus_recorder.h) class records activity
on the I2C bus lines; SDA and SCL. This record forms a
[BusTrace](../../../src/bus_trace/bus_trace.h).

See [BusRecorder](bus_recorder.md) for information about the `BusRecorder`.
This document discusses how to analyse a trace once you've got one.

## BusTrace
A `BusRecorder` adds a [BusEvent](../../../src/bus_trace/bus_event.h) to the
`BusTrace` each time SDA or SCL changes from HIGH to LOW or vice-versa.

The `BusEvent` tells you
* the state of each line after the event
* which lines changed state
* how much time passed since the previous `BusEvent`

## Warnings
Edges that happen more than 200 nanoseconds apart are recorded very
accurately. The trace may be simplified if the edges are closer than
that. See [BusRecorder Limitations](../bus_recorder#limitations) for
full details.

The summary is that:
* glitches are given a width of 0 nanoseconds
* if an event show that both lines changed then the interval
between them is given as 0 nanoseconds. You can't tell which happened
first.
* pulse trains get collapsed into single edges or single glitches

## Timing Analyser
The [I2CTimingAnalyser](../../../src/analysis/i2c_timing_analyser.h) class
is useful for comparing the times between events to the I2C specification.
I created it for tuning the
[teensy4_i2c](https://github.com/Richard-Gemmell/teensy4_i2c) driver.

TODO: Provide full instructions
