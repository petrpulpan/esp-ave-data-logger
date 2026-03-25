#include "sensors.h"
#include "config.h"
#include "i2c_utils.h"
#include "pressure_correction.h"
#include <Arduino.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>

#define DHTPIN  2
#define DHTTYPE DHT11

static DHT            s_dht(DHTPIN, DHTTYPE);
static Adafruit_BMP085 s_bmp;

bool gBmpInitialized = false;

// ---------------------------------------------------------------------------
// Public functions
// ---------------------------------------------------------------------------

// Scans the I2C bus and logs detected device addresses.
void scanI2CBus() {
  I2CUtils::scanBus();
}

// Probes and initializes BMP180, including diagnostic chip/calibration reads.
bool initBmp180() {
  I2CUtils::initBus(Config::kI2cSdaPin, Config::kI2cSclPin);
  delay(20);
  gBmpInitialized = false;

  if (!I2CUtils::isDevicePresent(Config::kBmp180I2cAddress)) {
    Serial.printf("[BMP180] I2C device 0x%02X not detected on SDA=%u SCL=%u\n",
                  Config::kBmp180I2cAddress, Config::kI2cSdaPin, Config::kI2cSclPin);
    scanI2CBus();
    return false;
  }

  uint8_t chipId = 0;
  if (I2CUtils::readRegister(Config::kBmp180I2cAddress, 0xD0, chipId)) {
    Serial.printf("[BMP180] Chip ID register (0xD0): 0x%02X\n", chipId);
    if (chipId != 0x55)
      Serial.println("[BMP180] Warning: unexpected chip ID (BMP180/BMP085 should be 0x55).");
  } else {
    Serial.println("[BMP180] Unable to read chip ID register.");
  }

  uint8_t calib[22] = {};
  if (I2CUtils::readBlock(Config::kBmp180I2cAddress, 0xAA, calib, sizeof(calib))) {
    bool allZero = true, allFF = true;
    for (size_t i = 0; i < sizeof(calib); ++i) {
      if (calib[i] != 0x00) allZero = false;
      if (calib[i] != 0xFF) allFF   = false;
    }
    Serial.println((allZero || allFF)
        ? "[BMP180] Calibration block content looks invalid."
        : "[BMP180] Calibration block read looks valid.");
  } else {
    Serial.println("[BMP180] Unable to read calibration block (0xAA..0xBF).");
  }

  if (!s_bmp.begin()) {
    Serial.println("[BMP180] Initialization failed after I2C probe.");
    return false;
  }

  Serial.println("[BMP180] Initialization successful.");
  gBmpInitialized = true;
  return true;
}

// Initializes I2C, DHT11, and BMP180 sensors during boot.
void initSensors() {
  I2CUtils::initBus(Config::kI2cSdaPin, Config::kI2cSclPin);
  Serial.printf("[Boot] I2C initialized (SDA=%u SCL=%u).\n",
                Config::kI2cSdaPin, Config::kI2cSclPin);
  scanI2CBus();

  s_dht.begin();
  Serial.printf("[Boot] DHT11 initialized on GPIO %d.\n", DHTPIN);

  if (!initBmp180()) {
    Serial.println("[Boot] BMP180 initialization failed. Loop will retry reads.");
  }
}

// Reads raw DHT11 temperature and humidity values.
bool readDHTRaw(float& temperature, float& humidity) {
  temperature = s_dht.readTemperature();
  humidity    = s_dht.readHumidity();
  return !isnan(temperature) && !isnan(humidity);
}

// Reads raw BMP180 temperature/pressure and applies configured temperature correction.
bool readBMPRaw(float& temperature, float& pressure) {
  temperature = s_bmp.readTemperature();
  pressure    = s_bmp.readPressure();
  // Calibration offset compensates board/module-specific BMP temperature bias.
  if (!isnan(temperature)) {
    temperature += Config::kBmpSensorTemperatureCalibration;
  }
  // Treat clearly out-of-range pressure as an invalid BMP sample.
  return !isnan(temperature) && !isnan(pressure) &&
         pressure >= Config::kBmpMinPressurePa &&
         pressure <= Config::kBmpMaxPressurePa;
}

// Reads validated telemetry values with retry and BMP recovery logic.
// Also precomputes sea-level pressure in hPa for upload payload usage.
bool readSensors(SensorReadings& readings) {
  for (uint8_t attempt = 1; attempt <= Config::kSensorReadRetries; ++attempt) {
    Serial.printf("[Sensors] Read attempt %u/%u\n", attempt, Config::kSensorReadRetries);

    // Fast I2C presence probe prevents wasting retries on a missing device.
    if (!I2CUtils::isDevicePresent(Config::kBmp180I2cAddress)) {
      Serial.println("[Sensors] BMP180 not responding on I2C. Re-initializing...");
      initBmp180();
      delay(50);
    }

    float temperature = s_bmp.readTemperature();
    const float humidity    = s_dht.readHumidity();
    const float pressure    = s_bmp.readPressure();

    // Apply the same calibration path used in the dedicated BMP helper.
    if (!isnan(temperature)) {
      temperature += Config::kBmpSensorTemperatureCalibration;
    }

    const bool dhtOk = !isnan(humidity);
    const bool bmpOk = !isnan(temperature) && !isnan(pressure) &&
                       pressure >= Config::kBmpMinPressurePa &&
                       pressure <= Config::kBmpMaxPressurePa;

    Serial.printf("[Sensors] Raw values T(BMP)=%.2fC H(DHT)=%.2f%% P(BMP)=%.2fPa\n",
                  temperature, humidity, pressure);

    if (dhtOk && bmpOk) {
      const float seaLevelPressureHpa =
          PressureCorrection::toSeaLevelHpa(pressure, Config::kAltitudeMeters);

      readings.temperatureC = temperature;
      readings.humidityPct  = humidity;
      readings.pressurePa   = pressure;
      readings.seaLevelPressureHpa = seaLevelPressureHpa;

      Serial.printf("[Sensors] Sea-level pressure=%.2fhPa (altitude=%.2fm)\n",
                    seaLevelPressureHpa, Config::kAltitudeMeters);
      Serial.println("[Sensors] Sensor values validated.");
      return true;
    }

    Serial.printf("[Sensors] Validation failed (DHT=%s, BMP180=%s). Retrying...\n",
                  dhtOk ? "OK" : "FAIL", bmpOk ? "OK" : "FAIL");

    // Re-init on BMP failures to recover from transient bus/device state issues.
    if (!bmpOk) {
      Serial.println("[Sensors] Re-initializing BMP180 after invalid pressure sample...");
      initBmp180();
    }

    delay(Config::kSensorRetryDelayMs);
  }

  Serial.println("[Sensors] Failed to read valid sensor values after retries.");
  return false;
}
