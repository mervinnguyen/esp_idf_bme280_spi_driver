# ESP32 to BME280 SPI Wiring Diagram

## Pin Connections

| ESP32 Pin | GPIO | Function | BME280 Pin |
|-----------|------|----------|------------|
| GPIO 19   | 19   | MISO     | SDO        |
| GPIO 23   | 23   | MOSI     | SDA/SDI    |
| GPIO 18   | 18   | SCLK     | SCK        |
| GPIO 5    | 5    | CS       | CSB        |
| 3.3V      | -    | Power    | VCC/VIN    |
| GND       | -    | Ground   | GND        |

## Breadboard Layout

```
    ESP32-DevKit                              BME280 Sensor
    ┌──────────┐                              ┌──────────┐
    │          │                              │          │
    │      3.3V├──────────────────────────────┤ VCC      │
    │          │                              │          │
    │      GND ├──────────────────────────────┤ GND      │
    │          │                              │          │
    │   GPIO 5 ├──────────────────────────────┤ CSB (CS) │
    │          │                              │          │
    │  GPIO 18 ├──────────────────────────────┤ SCK      │
    │          │                              │          │
    │  GPIO 19 ├──────────────────────────────┤ SDO      │
    │          │                              │          │
    │  GPIO 23 ├──────────────────────────────┤ SDI/SDA  │
    │          │                              │          │
    └──────────┘                              └──────────┘
```

## Step-by-Step Wiring Instructions

### Power Connections
1. **3.3V to VCC**: Connect ESP32's 3.3V pin to BME280's VCC (or VIN) pin
   - ⚠️ **Important**: Use 3.3V, NOT 5V! BME280 is a 3.3V device
   
2. **GND to GND**: Connect ESP32's GND pin to BME280's GND pin

### SPI Signal Connections
3. **GPIO 18 to SCK**: Connect ESP32 GPIO 18 (SCLK) to BME280 SCK pin
   - This is the SPI clock signal
   
4. **GPIO 23 to SDI**: Connect ESP32 GPIO 23 (MOSI) to BME280 SDI/SDA pin
   - This is the Master Out Slave In data line
   
5. **GPIO 19 to SDO**: Connect ESP32 GPIO 19 (MISO) to BME280 SDO pin
   - This is the Master In Slave Out data line
   
6. **GPIO 5 to CSB**: Connect ESP32 GPIO 5 (CS) to BME280 CSB/CS pin
   - This is the Chip Select signal

## Breadboard Physical Layout

```
                    Breadboard Top View
     ┌────────────────────────────────────────────────┐
     │  - └─┬─┘ - - - - - - - - - - - - - - - - - -  │  Power Rails
     │  + └─┼─┘ - - - - - - - - - - - - - - - - - -  │
     │      │                                          │
     │  a b c d e     f g h i j                      │
     ├──────────────────────────────────────────────┤
  1  │  ┌─────────────────┐                          │
  2  │  │                 │                          │
  3  │  │                 │      BME280              │
  4  │  │                 │    ┌─┬─┬─┬─┐            │
  5  │  │     ESP32       │    │ │ │ │ │            │
  6  │  │   DevKit C      │    └─┴─┴─┴─┘            │
  7  │  │                 │    V G S S              │
  8  │  │                 │    C N C D              │
  9  │  │                 │    C D K O              │
 10  │  │                 │         └──[19]         │
 11  │  │                 │    [18]──┘              │
 12  │  │                 │    [5]───┘              │
 13  │  │     [3.3V]──────┼────┘                    │
 14  │  │     [GND]───────┼────[23]                 │
 15  │  │                 │    |                     │
 16  │  │                 │    +─── to GND rail     │
 17  │  └─────────────────┘                          │
     │                                                │
     └────────────────────────────────────────────────┘

[GPIO Pin] = Wire connection point
```

## BME280 Pin Reference

### Top View of BME280 Module
```
     ┌─────────┐
     │  ●      │  ● = Sensor hole
     │         │
     ├─┬─┬─┬─┬─┤
     │V│G│S│S│S│
     │C│N│C│D│D│
     │C│D│K│I│O│
     └─┴─┴─┴─┴─┘
      1 2 3 4 5
```

1. **VCC** - Power supply (3.3V)
2. **GND** - Ground
3. **SCK** - SPI Clock
4. **SDI** - SPI Data In (MOSI from ESP32)
5. **SDO** - SPI Data Out (MISO to ESP32)
6. **CSB** - Chip Select (some modules have this as 6th pin)

## Notes

- **SPI Mode**: Mode 0 (CPOL=0, CPHA=0)
- **Clock Speed**: 1 MHz (1,000,000 Hz)
- **Logic Level**: 3.3V
- **Communication**: Full-duplex SPI

## Troubleshooting

### If sensor not responding:
1. Verify all 6 connections are secure
2. Check that 3.3V (not 5V) is being used
3. Ensure GPIO 5 (CS) is properly connected
4. Verify BME280 module is SPI-compatible (some modules are I2C only)
5. Use a multimeter to verify power at BME280: VCC=3.3V, GND=0V

### Common Issues:
- **Loose connections**: Press wires firmly into breadboard
- **Wrong voltage**: Double-check using 3.3V output
- **I2C module**: Some BME280 modules only support I2C protocol
- **Cold solder joints**: On some cheap modules, resolder the pins

## Testing

Once wired, the ESP32 should detect the BME280 at startup. Check serial output for:
- SPI bus initialization success
- Device ID read (should be 0x60 for BME280)
- Temperature, pressure, and humidity readings
