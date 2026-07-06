# STM32L432KC Smart Locker

Mbed project for a Nucleo-L432KC smart locker with button-based access control, temperature monitoring and tamper detection.

## Features

- Four-step password entered with two buttons
- Green/red LED access feedback
- TMP102 temperature readings over I2C
- ADXL345 motion alarm over SPI
- Serial temperature and acceleration output

## Hardware

- Nucleo-L432KC
- TMP102 temperature sensor
- ADXL345 accelerometer
- Two push buttons, two LEDs and a buzzer

| Component | Nucleo-L432KC pin |
| --- | --- |
| TMP102 SDA / SCL | D1 / D0 |
| ADXL345 MOSI / MISO / SCLK / CS | A6 / A5 / A4 / D12 |
| Button 1 / Button 2 | D3 / D10 |
| Green LED / Red LED | D6 / D9 |
| Buzzer | D2 |

The motion alarm threshold is `3.0g`, configured by `ALERT_THRESHOLD` in `main.cpp`.

## Run

1. Import the repository into Mbed Studio.
2. Resolve the Mbed OS dependency from `mbed-os.lib`.
3. Select `NUCLEO_L432KC`, then build and flash.

The repository also includes the project report and demonstration video.

## Author

Song Zexu
