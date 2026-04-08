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

| BME280 Pin | Arduino Mega Pin |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | Pin 20 |
| SCL | Pin 21 |
| CSB | 3.3V (I2C mode) |
| SDO | GND (address 0x76) |

---

## SEN0161 — pH Level

| SEN0161 Pin | Arduino Mega Pin |
|---|---|
| VCC | 5V |
| GND | GND |
| Signal (Po) | A0 |

---

## VEML6070 — UV Radiation Flux

| VEML6070 Pin | Arduino Mega Pin |
|---|---|
| VDD | 3.3V |
| GND | GND |
| SDA | Pin 20 |
| SCL | Pin 21 |
| ACK | Not connected (optional interrupt) |

---

## AS7265x — Chemical Nutrients

| AS7265x Pin | Arduino Mega Pin |
|---|---|
| 3V3 | 3.3V |
| GND | GND |
| SDA | Pin 20 |
| SCL | Pin 21 |

---

## DFR0300 — Salinity / Electrical Conductivity

| DFR0300 Pin | Arduino Mega Pin |
|---|---|
| VCC | 5V |
| GND | GND |
| Signal | A1 |

---

## HMC5883L — Magnetic Field Strength

| HMC5883L Pin | Arduino Mega Pin |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | Pin 20 |
| SCL | Pin 21 |
| DRDY | Pin 2 (optional interrupt) |

---

## Capacitive Soil Moisture Sensor v1.2 — Soil Porosity and Texture

| Sensor Pin | Arduino Mega Pin |
|---|---|
| VCC (Red) | 3.3V – 5V |
| GND (Black) | GND |
| AOUT (Yellow) | A2 |

---

## TSL2591 — Light Intensity

| TSL2591 Pin | Arduino Mega Pin |
|---|---|
| VIN | 3.3V – 5V |
| GND | GND |
| SDA | Pin 20 |
| SCL | Pin 21 |
| INT | Pin 3 (optional interrupt) |

---

## SGP30 — Dissolved Gas Composition

| SGP30 Pin | Arduino Mega Pin |
|---|---|
| VIN | 3.3V – 5V |
| GND | GND |
| SDA | Pin 20 |
| SCL | Pin 21 |

---

## SEN0165 — Redox / ORP Potential

| SEN0165 Pin | Arduino Mega Pin |
|---|---|
| VCC | 5V |
| GND | GND |
| Signal | A3 |

---
