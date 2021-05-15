# i2c-underneath
A toolkit to monitor, diagnose and validate I2C systems.

## Project Status
This project has only just started. It's nowhere near ready for release.

The design is experimental and likely to change. Do not expect the API
to remain stable until I created a release with a version number. 

## I2C Documentation
### Overview of I2C

### Introductions to the I2C Protocol
https://www.seeedstudio.com/blog/2019/09/26/i2c-communication-interface-and-protocol-with-diagrams/

### Specifications
[I2C Specification](https://www.nxp.com/docs/en/user-guide/UM10204.pdf)

### Miscellaneous
[Demystifying Microcontroller GPIO Settings](https://embeddedartistry.com/blog/2018/06/04/demystifying-microcontroller-gpio-settings/)

# I2C Faults

## Fundamentals

### I2C Pins are Open Drain
I2C bus pins are configured to be "Open Drain". They can pull a line LOW, but they
can't push it HIGH. The line goes HIGH only when none of the devices are pulling it LOW.
It's the pull-up resistors that actually raise the voltage.

#### Consequences
1. The bus becomes "stuck" if any device pulls a line LOW and keeps it there.
1. Devices can tell when they're writing a 1 but another device is writing a 0.
1. Devices cannot tell if they're writing a 0 but another device is trying to write a 1.

### Bit Read Errors

### Arbitration Lost
The master will abort a transaction if it tries to send a 1, but the bus reads 0. 
It will usually report "Arbitration Lost". This is because it thinks that another
master is trying to start a transaction and it gives way to allow the other master
to continue. This is normal and correct behaviour in a multi-master setup.

If there's only one master on the bus then "Arbitration Lost" is really a bit error.

See section 3.1.8 of the I2C Specification 
