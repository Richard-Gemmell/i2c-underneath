# i2c-underneath
A toolkit to monitor, diagnose and validate I2C systems.

## Project Status
This project has only just started. It's nowhere near ready for release.

The design is experimental and likely to change. Do not expect the API
to remain stable until I create a release with a version number.

I haven't organised the documentation properly. It's still more of a set
of working notes.

I haven't made any attempt to make this project friendly to use. You'll
need to download or clone the code and install it in a way that suits
your IDE.

# Installation
* [Arduino Instructions](documentation/installation/arduino_installation.md)
* [PlatformIO Instructions](documentation/installation/platformio_installation.md)

## GitHub Help
If you're wondering how to get code out of GitHub then you're not alone!
Here are some [instructions for downloading or cloning](documentation/installation/github_help.md)
this library. The page also explains how to download a different branch.

# I2C Guides
The [Pullup Resistors and Rise Times](documentation/i2c_setup/pull_up_resistors.md)
page describes how to set and tune your pullups.

There are [links to other I2C resources](#other-i2c-documentation) at the bottom of the page.

# Tools
## Line Tester
Slow rise times are one of the most common problems with I2C.
You can use the [LineTester](src/line_test/line_tester.h) class instead
of an oscilloscope to measure the rise times directly

The [Pullup Resistors and Rise Times](documentation/i2c_setup/pull_up_resistors.md)
page describes how to set and tune your pullups in detail.

## Bus Recorder
The [BusRecorder](documentation/tools/bus_recorder/bus_recorder.md) records electrical
activity on an I2C bus. It's a bit like having an oscilloscope.

It creates a trace which shows each transition from HIGH to LOW and back
again for both SDA and SCL. It also records the interval between these events.

You can use it to:
* record I2C traffic
* measure the bus clock speed
* check for I2C logic errors
* verify that an I2C device conforms to the I2C specification
* detect glitches which may be caused by electrical noise

See [Analysing BusRecorder Traces](documentation/tools/bus_recorder/analysing_traces.md)
for more information about analysing traces.

## Bus Monitor
The `BusMonitor` watches an I2C bus and reports whether it's idle, busy
or stuck. Nothing can use the bus once it becomes stuck. You can free
a stuck bus by sending a set of clock pulses or with a hardware reset.
The [Stuck Bus Example](https://github.com/Richard-Gemmell/i2c-underneath/blob/main/examples/bus_monitor/stuck_bus/stuck_bus.ino)
shows how to use the monitor. The [Monitor Master Example](https://github.com/Richard-Gemmell/i2c-underneath/blob/main/examples/bus_monitor/monitor_master/monitor_master.ino)
contains code to recover a stuck bus. See:

* [The Examples](https://github.com/Richard-Gemmell/i2c-underneath/tree/main/examples/bus_monitor)
* [Bus Monitor Class](https://github.com/Richard-Gemmell/i2c-underneath/tree/main/examples/bus_monitor)

# Other I2C Documentation
## Introductions to the I2C Protocol
* [i2c-bus.org](https://www.i2c-bus.org/)
* [Seed Studio Intro to I2C](https://www.seeedstudio.com/blog/2019/09/26/i2c-communication-interface-and-protocol-with-diagrams/)

## Specifications
* [I2C Specification](https://www.nxp.com/docs/en/user-guide/UM10204.pdf)

## Miscellaneous
* [Hackaday Articles](https://hackaday.com/tag/i2c-bus/)
* [Issues with the I2C Bus and How to Solve Them](https://www.digikey.com/en/articles/issues-with-the-i2c-bus-and-how-to-solve-them)
* [Demystifying Microcontroller GPIO Settings](https://embeddedartistry.com/blog/2018/06/04/demystifying-microcontroller-gpio-settings/)
