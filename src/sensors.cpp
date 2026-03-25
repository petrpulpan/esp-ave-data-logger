#include "sensors.h"

#include "config.h"
#include "sensor_hw_init.h"
#include "sensor_processing.h"
#include "sensor_reader.h"

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Public facade functions (wrapping hw_init/reader/processing layers)
// ---------------------------------------------------------------------------

// Scans the I2C bus and logs detected device addresses.
void scanI2CBus() {
  scanI2CBusLayer();
}

// Probes and initializes BMP180, including diagnostic chip/calibration reads.
SensorInitStatus initBmp180() {
  return reinitBmp180Layer();
}

// Initializes I2C, DHT11, and BMP180 sensors during boot.
SensorInitStatus initSensors() {
  return initSensorHardwareLayer();
}

// Reads raw DHT11 temperature and humidity values.
RawDhtSample readDHTRaw() {
  return readDhtSample();
}

// Reads raw BMP180 temperature/pressure and applies configured temperature correction.
RawBmpSample readBMPRaw() {
  return readBmpSample();
}

// Reads validated telemetry values with retry and BMP recovery logic.
// Also precomputes sea-level pressure in hPa for upload payload usage.
bool readSensors(SensorReadings& readings, SensorReadStatus* status) {
  SensorReadStatus localStatus = {};
  localStatus.ok = false;

  for (uint8_t attempt = 1; attempt <= Config::kSensorReadRetries; ++attempt) {
    Serial.printf("[Sensors] Read attempt %u/%u\n", attempt, Config::kSensorReadRetries);

    RawDhtSample dht = readDhtSample();
    RawBmpSample bmp = readBmpSample();

    if (!bmp.valid) {
      Serial.println("[Sensors] Invalid BMP sample. Re-initializing BMP180...");
      localStatus.bmpInitStatus = reinitBmp180Layer();
      delay(50);
      bmp = readBmpSample();
    }

    bool dhtOk = false;
    bool bmpOk = false;
    const bool processed = processSensorSamples(dht, bmp, readings, dhtOk, bmpOk);

    localStatus.dhtValid = dhtOk;
    localStatus.bmpValid = bmpOk;

    Serial.printf("[Sensors] Raw values T(BMP)=%.2fC H(DHT)=%.2f%% P(BMP)=%.2fPa\n",
                  bmp.temperatureC, dht.humidityPct, bmp.pressurePa);

    if (processed) {
      Serial.printf("[Sensors] Sea-level pressure=%.2fhPa (altitude=%.2fm)\n",
                    readings.seaLevelPressureHpa, Config::kAltitudeMeters);
      Serial.println("[Sensors] Sensor values validated.");
      localStatus.ok = true;
      if (status) {
        *status = localStatus;
      }
      return true;
    }

    Serial.printf("[Sensors] Validation failed (DHT=%s, BMP180=%s). Retrying...\n",
                  dhtOk ? "OK" : "FAIL", bmpOk ? "OK" : "FAIL");

    // Re-init on BMP failures to recover from transient bus/device state issues.
    if (!bmpOk) {
      Serial.println("[Sensors] Re-initializing BMP180 after invalid pressure sample...");
      localStatus.bmpInitStatus = reinitBmp180Layer();
    }

    delay(Config::kSensorRetryDelayMs);
  }

  Serial.println("[Sensors] Failed to read valid sensor values after retries.");
  if (status) {
    *status = localStatus;
  }
  return false;
}
