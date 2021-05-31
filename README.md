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

# Tools
## Bus Monitor
The BusMonitor watches an I2C bus and reports whether it's idle, busy
or stuck. Nothing can use the bus once it becomes stuck. You can free
a stuck bus by sending a set of clock pulses or with a hardware reset.
The [Stuck Bus Example](https://github.com/Richard-Gemmell/i2c-underneath/blob/main/examples/bus_monitor/stuck_bus/stuck_bus.ino)
shows how to use the monitor. The [Monitor Master Example](https://github.com/Richard-Gemmell/i2c-underneath/blob/main/examples/bus_monitor/monitor_master/monitor_master.ino)
contains code to recover a stuck bus. See:

* [The Examples](https://github.com/Richard-Gemmell/i2c-underneath/tree/main/examples/bus_monitor)
* [Bus Monitor Class](https://github.com/Richard-Gemmell/i2c-underneath/tree/main/examples/bus_monitor)

# I2C Documentation
## Introductions to the I2C Protocol
* [Seed Studio Intro to I2C](https://www.seeedstudio.com/blog/2019/09/26/i2c-communication-interface-and-protocol-with-diagrams/)

## Specifications
* [I2C Specification](https://www.nxp.com/docs/en/user-guide/UM10204.pdf)

## Miscellaneous
* [Issues with the I2C Bus and How to Solve Them](https://www.digikey.com/en/articles/issues-with-the-i2c-bus-and-how-to-solve-them)
* [Demystifying Microcontroller GPIO Settings](https://embeddedartistry.com/blog/2018/06/04/demystifying-microcontroller-gpio-settings/)
