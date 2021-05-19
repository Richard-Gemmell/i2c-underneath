# i2c-underneath
A toolkit to monitor, diagnose and validate I2C systems.

## Project Status
This project has only just started. It's nowhere near ready for release.

The design is experimental and likely to change. Do not expect the API
to remain stable until I create a release with a version number.

## Introduction
TODO

# Simple Stuff
## Electrical
* no loose wires
* common ground
* pull up resistors neither too high nor too low
* total bus capacitance is acceptable

## Logic & Compatibility
* each slave has a unique address
* devices have compatible bus voltages (ideally the same voltage)
* master clock speed is no faster than the slowest slave (if in doubt start at 100 kHz)
* master supports clock-stretching if a slave requires it
* all masters have multi-master support if you've got more than 1 master

## I2C Documentation
### Overview of I2C

### Introductions to the I2C Protocol
* https://www.seeedstudio.com/blog/2019/09/26/i2c-communication-interface-and-protocol-with-diagrams/

### Specifications
* [I2C I2CParameters](https://www.nxp.com/docs/en/user-guide/UM10204.pdf)

### Miscellaneous
* [Issues with the I2C Bus and How to Solve Them](https://www.digikey.com/en/articles/issues-with-the-i2c-bus-and-how-to-solve-them)
* [Demystifying Microcontroller GPIO Settings](https://embeddedartistry.com/blog/2018/06/04/demystifying-microcontroller-gpio-settings/)

# I2C I2CParameters Notes
This section points out some key features of I2C and how they might affect
the reliability of an I2C connection.

## I2C Pins are Open-Drain
I2C bus pins are configured to be "Open Drain". They can pull a line LOW, but they
can't push it HIGH. The line goes HIGH only when none of the devices are pulling it
LOW. It's the pull-up resistors that actually raise the voltage.

(The spec does allow a master device to use a push-pull driver on the clock line
(SCL) but this is risky. It causes a short circuit if any slave on the bus tries
to stretch the clock. See sections 3.1.1 and 3.1.9 of the I2C I2CParameters.)

### Consequences
* If any device pulls a line LOW and keeps it there then nothing can use the
  bus. This is known as a "stuck bus".
* Devices can tell when they're writing a 1 but another device is writing a 0.
* Devices **cannot** tell if they're writing a 0 but another device is trying
  to write a 1.

## Multi-Master Support
The I2C protocol allows a bus to have more than one master at the same time.
The "synchronisation" and "arbitration" mechanisms enable competing masters to
decide which one should continue if they try to use the bus at the same time.
See sections 3.1.7 and 3.1.8 of the I2C specification.

Master devices are not required to support multi-master configurations.
(from section 3.1 of the I2C I2CParameters)

### Consequences
* a device which supports multi-master configurations may report
  "Arbitration Lost" when it detects a bit error even though it's
  the only master on the bus.
* you can connect another master to the bus without stopping the existing
  master as long as both support multi-master configuration. This may be
  useful for testing the bus if you're unable to disable the first master.

## Clock Stretching
I2C slave devices are allowed to slow the bus down if they need to. This allows
slow and fast devices to communicate successfully. The slave slows the bus by
holding the clock line (SCL) LOW until it's ready to continue. The master is
forced to wait until the slave is ready. There's no time limit in the I2C
specification. The slave can wait as long as it likes.

Devices are not required to support clock stretching.

### Consequences
* if a slave pulls SCL LOW and then locks up it'll break the bus.
  (See "stuck bus") There's no way to fix this without rebooting the slave.
* if a slave pulls SCL LOW for too long, a master might think the bus is
  stuck even if the slave eventually releases SCL.
* if the master doesn't support clock-stretching but the slave does, then
  the master will keep thinking that the bus is stuck.

# I2C Faults
## Timeouts
TODO

* slave stretches the clock for too long
* ?? slave is waiting for master to pulse SCL but the master isn't responding

## Stuck Bus
The bus is said to be "stuck" if a device pulls a lines down and holds it there.
This is relatively common in I2C systems.

This can happen if:
* a device pulls a pin LOW and either hangs or forgets to release the pin
* (confirmation required) a slave pulls SDA LOW to write a 0 but the master does
  not pulse SCL as expected. A well-behaved slave will timeout after a while and
  abort the transaction to avoid a stuck bus. The specification does not require
  this. A master can attempt to clear this error by sending 9 clock pulses on SCL.
  Many master implementations do this automatically.

A master may think the bus is stuck when it isn't. This can happen if:
* a slave stretches the clock but the master doesn't support clock stretching
* the slave stretches the clock for a very long time

A device cannot break the bus by setting a line HIGH and then failing because
this won't stop other devices from changing the line level.

See section 3.1.16 of the I2C I2CParameters for more information. 

## Bit Errors
A device reports a bit error of some kind when it tries to write a 1
to the bus but, the data line (SDA) remains LOW. Note that line can
never be unexpectedly HIGH because a device can always pull the line
LOW whenever it wants.

### Arbitration Lost
The master will abort a transaction if it tries to send a 1, but the bus reads 0. 
It will usually report "Arbitration Lost". This is because it thinks that another
master is trying to start a transaction. It gives way to allow the other master
to continue. This is normal and correct behaviour in a multi-master setup.

If there's only one master on the bus then "Arbitration Lost" is just a
bit error with a confusing error message.

See section 3.1.8 of the I2C I2CParameters for a complete description of arbitration. 

## Timing Problems
### Missed Start
If a device needs to poll the bus then it may miss a START condition.
This shouldn't happen if the device has hardware support but can be a
problem if it bit-bangs I2C. See section 3.1.15 of the I2C I2CParameters.

#### Fixes
* configure the master to send a **START byte** instead of a normal start.
  This gives the slave more time to notice the transfer.

### Incorrect Timing
Section 6 of the I2C I2CParameters defines various timing parameters that
determine how long the lines are held high or low, how quickly they must
change level and when changes are allowed.

An I2C transfer is likely to fail in odd ways if a device gets the timing
wrong. Issues can include

### Fixes
* add or increase the strength of the pull up resistors if the rise time
  for a line is too slow
* reduce the strength of the pull-up resistors if the fall time is too slow
  (Need confirmation for this. RJG May 2021)
* reduce the capacitance of the bus if the rise or fall times are too slow

## Electrical Problems

### Incorrect Logic Levels
* lack of common GND
* GND varies too much due to resistance between devices
* incompatible V<sub>DD</sub> voltages (e.g. 5V and 2.5V)
* pins set to trigger at incorrect voltages (should be 0.3 V<sub>DD</sub>
  and 0.7 V<sub>DD</sub>)