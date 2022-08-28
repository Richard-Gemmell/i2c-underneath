# I2C Device Electrical Characteristics


| Part          | 1M  | 400k | 100k | Pullup | Capacitance pF | Default Address | Alternatives     | Type                      | 
|---------------|-----|------|------|--------|----------------|-----------------|------------------|---------------------------|
| Adafruit 4226 | Y   | Y    | Y    | 10k    | ?              | 0x40            | 0x41, 0x44, 0x45 | INA260 Breakout Board     |
| INA260        | Y   | Y    | Y    | N      | 3              | 0x40            | 16 in total      | Current and Power Monitor |
| Adafruit 3387 | ?   | Y    | Y    | 10k    | ?              | 0x6A, 0x1C      | 0x6B, 0x1E       | LSM9DS1 Breakout Board    |
| LSM9DS1       | ?   | Y    | Y    | N      | ?              | 0x6B, 0x1E      | N                | IMU                       |
