# IOT
Karmanya Team Deimos Life Sciences IOT

Arduino Mega 2560 sketch for a 10-sensor environmental monitoring rover.

## Sensors Used
BME280
SEN0161
VEML6070
AS7265x
DFR0300
HMC5883L
Capacitive Soil Moisture v1.2
TSL2591
SGP30
SEN0165

## Libraries Required
- Adafruit BME280 Library
- Adafruit VEML6070
- SparkFun AS7265x Arduino Library
- Adafruit HMC5883 Unified
- Adafruit TSL2591 Library
- Adafruit SGP30
- Adafruit Unified Sensor

## BME280 — Temperature / Humidity / Pressure
![BME280](sensors/bme280.jpg)

| BME280 Pin | Arduino Mega Pin |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | Pin 20 |
| SCL | Pin 21 |
| SDO | GND (address 0x76) |
