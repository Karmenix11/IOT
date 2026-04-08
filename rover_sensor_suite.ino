// =============================================================
//  ROVER SENSOR SUITE — Complete Integrated Sketch
//  Sensors: BME280, SEN0161, VEML6070, AS7265x, DFR0300,
//           HMC5883L, Capacitive Soil Moisture v1.2,
//           TSL2591, SGP30, SEN0165
//  Target:  Arduino Mega 2560 (recommended — more SRAM & pins)
//  Author:  Generated Reference Code
// =============================================================

// ── Libraries ─────────────────────────────────────────────────
#include <Wire.h>
#include <Adafruit_BME280.h>        // Install: Adafruit BME280 Library
#include <Adafruit_VEML6070.h>      // Install: Adafruit VEML6070
#include <SparkFun_AS7265X.h>       // Install: SparkFun AS7265x Arduino Library
#include <Adafruit_HMC5883_U.h>     // Install: Adafruit HMC5883 Unified
#include <Adafruit_Sensor.h>        // Install: Adafruit Unified Sensor
#include <Adafruit_TSL2591.h>       // Install: Adafruit TSL2591 Library
#include "Adafruit_SGP30.h"         // Install: Adafruit SGP30 Library

// ── I2C Sensor Objects ─────────────────────────────────────────
Adafruit_BME280       bme;
Adafruit_VEML6070     uv    = Adafruit_VEML6070();
AS7265X               spec;                           // Chemical Nutrients (18-channel)
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
Adafruit_TSL2591      tsl   = Adafruit_TSL2591(2591);
Adafruit_SGP30        sgp;

// ── Analog Pin Assignments ─────────────────────────────────────
const int PH_PIN       = A0;   // SEN0161  — pH Level
const int EC_PIN       = A1;   // DFR0300  — Salinity / Electrical Conductivity
const int SOIL_PIN     = A2;   // Cap. Soil Moisture v1.2 — Soil Porosity & Texture
const int ORP_PIN      = A3;   // SEN0165  — Redox / ORP Potential

// ── Calibration Constants ──────────────────────────────────────
// pH  (SEN0161) — calibrate with pH 4.0 and pH 7.0 buffer solutions
const float PH_OFFSET     =  0.0;   // Adjust after calibration

// ORP (SEN0165) — calibrate against 225mV or 470mV ORP standard
const float ORP_OFFSET    =  0.0;   // Adjust after calibration

// Soil Moisture — calibrate: sensor dry in air vs. submerged in water
const int   SOIL_DRY      = 620;    // Raw ADC value in dry air
const int   SOIL_WET      = 310;    // Raw ADC value fully submerged

// EC  (DFR0300) — voltage divider + temperature compensation
const float EC_RES2       = 820.0;  // Voltage divider resistor (Ω)
const float EC_REF        = 200.0;  // Calibration coefficient

// Reference voltage for Arduino Mega
const float VREF          = 5.0;

// ── Sensor Status Flags ────────────────────────────────────────
bool bme_ok  = false;
bool mag_ok  = false;
bool tsl_ok  = false;
bool sgp_ok  = false;
bool spec_ok = false;

// =============================================================
//  SETUP
// =============================================================
void setup() {
  Serial.begin(9600);
  Wire.begin();
  Serial.println(F("=== Rover Sensor Suite Initializing ==="));

  // ── 1. BME280 (Temperature, Humidity/Water Activity, Pressure) ─
  if (bme.begin(0x76)) {
    bme_ok = true;
    Serial.println(F("[OK] BME280 found at 0x76"));
  } else if (bme.begin(0x77)) {
    bme_ok = true;
    Serial.println(F("[OK] BME280 found at 0x77"));
  } else {
    Serial.println(F("[FAIL] BME280 not found — check wiring"));
  }

  // ── 2. VEML6070 (UV Radiation Flux) ───────────────────────────
  // NOTE: VEML6070 measures UV light intensity, NOT ionizing radiation.
  // Do NOT substitute with a Geiger counter — different phenomenon entirely.
  uv.begin(VEML6070_1_T);   // 1T integration (~125ms)
  Serial.println(F("[OK] VEML6070 UV sensor initialized"));

  // ── 3. AS7265x (Chemical Nutrients — 18-channel spectroscopy) ──
  if (spec.begin()) {
    spec_ok = true;
    spec.disableIndicator();
    Serial.println(F("[OK] AS7265x spectral sensor found"));
  } else {
    Serial.println(F("[FAIL] AS7265x not found — check I2C address/wiring"));
  }

  // ── 4. HMC5883L (Magnetic Field Strength) ─────────────────────
  if (mag.begin()) {
    mag_ok = true;
    Serial.println(F("[OK] HMC5883L magnetometer found"));
  } else {
    Serial.println(F("[FAIL] HMC5883L not found"));
  }

  // ── 5. TSL2591 (Light Intensity) ──────────────────────────────
  if (tsl.begin()) {
    tsl_ok = true;
    tsl.setGain(TSL2591_GAIN_MED);
    tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
    Serial.println(F("[OK] TSL2591 light sensor found"));
  } else {
    Serial.println(F("[FAIL] TSL2591 not found"));
  }

  // ── 6. SGP30 (Dissolved Gas Composition) ──────────────────────
  if (sgp.begin()) {
    sgp_ok = true;
    Serial.print(F("[OK] SGP30 gas sensor — Serial #"));
    Serial.println(sgp.serialnumber[0], HEX);
    // NOTE: SGP30 needs ~15 min warm-up for stable baseline readings.
    // For persistent use, save and restore baseline from EEPROM.
  } else {
    Serial.println(F("[FAIL] SGP30 not found"));
  }

  // ── 7. Analog Sensors — no init needed, set input mode ────────
  pinMode(PH_PIN,   INPUT);
  pinMode(EC_PIN,   INPUT);
  pinMode(SOIL_PIN, INPUT);
  pinMode(ORP_PIN,  INPUT);
  Serial.println(F("[OK] Analog pins configured (A0–A3)"));

  Serial.println(F("=== Initialization Complete ===\n"));
  delay(1000);
}

// =============================================================
//  HELPER: Read averaged analog value (reduces noise)
// =============================================================
float readAnalogAvg(int pin, int samples = 10) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  return (float)sum / samples;
}

// =============================================================
//  MAIN LOOP
// =============================================================
void loop() {
  Serial.println(F("\n========================================"));
  Serial.println(F("  ENVIRONMENTAL SENSOR READINGS"));
  Serial.println(F("========================================"));

  float currentTemp = 25.0;   // Default; will be overwritten by BME280 reading

  // ── [1] BME280 — Temperature, Water Activity (Humidity), Pressure ─
  Serial.println(F("\n[BME280] Temp / Water Activity / Pressure"));
  if (bme_ok) {
    currentTemp        = bme.readTemperature();       // °C
    float humidity     = bme.readHumidity();          // % Relative Humidity (Water Activity proxy)
    float pressure     = bme.readPressure() / 100.0F; // hPa
    Serial.print(F("  Temperature    : ")); Serial.print(currentTemp, 2); Serial.println(F(" °C"));
    Serial.print(F("  Water Activity : ")); Serial.print(humidity, 2);    Serial.println(F(" % RH"));
    Serial.print(F("  Atm. Pressure  : ")); Serial.print(pressure, 2);    Serial.println(F(" hPa"));
  } else {
    Serial.println(F("  Sensor offline."));
  }

  // ── [2] SEN0161 — pH Level ─────────────────────────────────────
  Serial.println(F("\n[SEN0161] pH Level"));
  {
    float rawADC    = readAnalogAvg(PH_PIN);
    float voltage   = rawADC * (VREF / 1023.0);
    // Standard DFRobot pH formula (adjust PH_OFFSET after buffer calibration)
    float pHValue   = 7.0 + ((2.5 - voltage) / 0.18) + PH_OFFSET;
    Serial.print(F("  Voltage : ")); Serial.print(voltage, 3); Serial.println(F(" V"));
    Serial.print(F("  pH      : ")); Serial.println(pHValue, 2);
  }

  // ── [3] VEML6070 — UV Radiation Flux ──────────────────────────
  // IMPORTANT: This is UV optical radiation (280–400nm), NOT nuclear/ionizing radiation.
  // VEML6070 is the correct sensor for the "Radiation Flux" parameter in Question 2.
  Serial.println(F("\n[VEML6070] UV Radiation Flux"));
  {
    uint16_t uvRaw = uv.readUV();
    String   risk  = "Low";
    if (uvRaw > 1000) risk = "Moderate";
    if (uvRaw > 2250) risk = "High";
    if (uvRaw > 3000) risk = "Very High";
    Serial.print(F("  UV Count : ")); Serial.println(uvRaw);
    Serial.print(F("  UV Risk  : ")); Serial.println(risk);
  }

  // ── [4] AS7265x — Chemical Nutrients (Spectral Proxy) ─────────
  Serial.println(F("\n[AS7265x] Chemical Nutrients (18-channel Spectroscopy)"));
  if (spec_ok) {
    spec.takeMeasurementsWithBulb();  // Uses internal illumination LEDs
    // Key NIR/Visible bands used for nutrient index estimation (NDVI-style)
    float ch610 = spec.getCalibratedA();  // 610nm — Chlorophyll / Red
    float ch680 = spec.getCalibratedB();  // 680nm — Chlorophyll absorption
    float ch730 = spec.getCalibratedC();  // 730nm — Red edge (plant stress)
    float ch760 = spec.getCalibratedD();  // 760nm — NIR reflectance
    float ch810 = spec.getCalibratedE();  // 810nm — NIR (water content)
    float ch860 = spec.getCalibratedF();  // 860nm — Dry matter index
    Serial.print(F("  610nm (Red)    : ")); Serial.print(ch610, 2); Serial.println(F(" nW/cm2"));
    Serial.print(F("  680nm (Chl)    : ")); Serial.print(ch680, 2); Serial.println(F(" nW/cm2"));
    Serial.print(F("  730nm (RedEdge): ")); Serial.print(ch730, 2); Serial.println(F(" nW/cm2"));
    Serial.print(F("  760nm (NIR)    : ")); Serial.print(ch760, 2); Serial.println(F(" nW/cm2"));
    Serial.print(F("  810nm (NIR)    : ")); Serial.print(ch810, 2); Serial.println(F(" nW/cm2"));
    Serial.print(F("  860nm (NIR)    : ")); Serial.print(ch860, 2); Serial.println(F(" nW/cm2"));
    // Simple NDVI proxy (Normalized Difference Vegetation Index)
    if ((ch760 + ch680) > 0) {
      float ndvi = (ch760 - ch680) / (ch760 + ch680);
      Serial.print(F("  NDVI proxy     : ")); Serial.println(ndvi, 3);
    }
  } else {
    Serial.println(F("  Sensor offline."));
  }

  // ── [5] DFR0300 — Salinity (Electrical Conductivity) ──────────
  Serial.println(F("\n[DFR0300] Salinity / Electrical Conductivity"));
  {
    float rawADC   = readAnalogAvg(EC_PIN);
    float voltage  = rawADC * (VREF / 1023.0);
    // EC from voltage divider (µS/cm), then temperature-compensated
    float ecRaw    = (voltage > 0) ? (voltage * EC_RES2 * EC_REF) / (VREF - voltage) : 0;
    float ecComp   = ecRaw / (1.0 + 0.0185 * (currentTemp - 25.0));
    // Approximate salinity (PSU) from EC (rough conversion)
    float salinity = ecComp * 0.00064;   // Approx. for seawater — recalibrate for soil
    Serial.print(F("  Voltage  : ")); Serial.print(voltage, 3); Serial.println(F(" V"));
    Serial.print(F("  EC       : ")); Serial.print(ecComp, 2);  Serial.println(F(" µS/cm"));
    Serial.print(F("  Salinity : ")); Serial.print(salinity, 4); Serial.println(F(" PSU (approx)"));
  }

  // ── [6] HMC5883L — Magnetic Field Strength ────────────────────
  Serial.println(F("\n[HMC5883L] Magnetic Field Strength"));
  if (mag_ok) {
    sensors_event_t event;
    mag.getEvent(&event);
    float x = event.magnetic.x;
    float y = event.magnetic.y;
    float z = event.magnetic.z;
    float heading = atan2(y, x) * (180.0 / PI);
    if (heading < 0) heading += 360.0;
    Serial.print(F("  X : ")); Serial.print(x, 3); Serial.println(F(" µT"));
    Serial.print(F("  Y : ")); Serial.print(y, 3); Serial.println(F(" µT"));
    Serial.print(F("  Z : ")); Serial.print(z, 3); Serial.println(F(" µT"));
    Serial.print(F("  Heading : ")); Serial.print(heading, 1); Serial.println(F("°"));
  } else {
    Serial.println(F("  Sensor offline."));
  }

  // ── [7] Capacitive Soil Moisture v1.2 — Porosity & Texture ────
  Serial.println(F("\n[Soil Moisture v1.2] Porosity & Texture"));
  {
    int   raw      = (int)readAnalogAvg(SOIL_PIN);
    float moisture = constrain(map(raw, SOIL_DRY, SOIL_WET, 0, 100), 0, 100);
    Serial.print(F("  Raw ADC  : ")); Serial.println(raw);
    Serial.print(F("  Moisture : ")); Serial.print(moisture, 1); Serial.println(F(" %"));
    // Rough texture inference from moisture retention
    if      (moisture < 20) Serial.println(F("  Texture  : Sandy / Low porosity"));
    else if (moisture < 50) Serial.println(F("  Texture  : Loamy / Medium porosity"));
    else                    Serial.println(F("  Texture  : Clay / High porosity"));
  }

  // ── [8] TSL2591 — Light Intensity ─────────────────────────────
  Serial.println(F("\n[TSL2591] Light Intensity"));
  if (tsl_ok) {
    sensors_event_t event;
    tsl.getEvent(&event);
    uint32_t lum     = tsl.getFullLuminosity();
    uint16_t ir      = lum >> 16;
    uint16_t full    = lum & 0xFFFF;
    uint16_t visible = full - ir;
    Serial.print(F("  Lux      : ")); Serial.println(event.light, 2);
    Serial.print(F("  Visible  : ")); Serial.println(visible);
    Serial.print(F("  IR       : ")); Serial.println(ir);
  } else {
    Serial.println(F("  Sensor offline."));
  }

  // ── [9] SGP30 — Dissolved Gas Composition ─────────────────────
  Serial.println(F("\n[SGP30] Dissolved / Ambient Gas Composition"));
  if (sgp_ok) {
    // Set humidity compensation using BME280 values (improves accuracy)
    if (bme_ok) {
      float absHum = 216.7f * ((bme.readHumidity() / 100.0f) * 6.112f *
                    exp((17.62f * currentTemp) / (243.12f + currentTemp)) /
                    (273.15f + currentTemp));
      uint32_t absHumScaled = (uint32_t)(1000.0f * absHum);
      sgp.setHumidity(((uint16_t)absHumScaled << 8) | (uint8_t)(absHumScaled % 256));
    }
    if (sgp.IAQmeasure()) {
      Serial.print(F("  eCO2   : ")); Serial.print(sgp.eCO2);  Serial.println(F(" ppm"));
      Serial.print(F("  TVOC   : ")); Serial.print(sgp.TVOC);  Serial.println(F(" ppb"));
    }
    if (sgp.IAQmeasureRaw()) {
      Serial.print(F("  H2 raw     : ")); Serial.println(sgp.rawH2);
      Serial.print(F("  Ethanol raw: ")); Serial.println(sgp.rawEthanol);
    }
  } else {
    Serial.println(F("  Sensor offline."));
  }

  // ── [10] SEN0165 — Redox / ORP Potential ──────────────────────
  Serial.println(F("\n[SEN0165] Redox (ORP) Potential"));
  {
    float rawADC   = readAnalogAvg(ORP_PIN, 40);   // More samples for ORP stability
    float voltage  = rawADC * (VREF / 1023.0);
    // DFRobot ORP formula — temperature compensated (Nernst)
    float orp      = ((30.0 * VREF) - (75.0 * voltage)) / 75.0 * 1000.0 + ORP_OFFSET;
    orp           += 0.6 * (currentTemp - 25.0);   // Nernst temperature correction
    Serial.print(F("  Voltage : ")); Serial.print(voltage, 3); Serial.println(F(" V"));
    Serial.print(F("  ORP     : ")); Serial.print(orp, 1);     Serial.println(F(" mV"));
    // Interpretation
    if      (orp > 200)  Serial.println(F("  Status  : Oxidizing environment"));
    else if (orp > 0)    Serial.println(F("  Status  : Mildly oxidizing"));
    else                 Serial.println(F("  Status  : Reducing environment"));
  }

  Serial.println(F("\n========================================"));
  Serial.println(F("  Next sweep in 5 seconds..."));
  delay(5000);
}
