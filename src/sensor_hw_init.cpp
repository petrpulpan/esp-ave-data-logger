#include "sensor_hw_init.h"
#include "sensor_instances.h"

#include "config.h"
#include "i2c_utils.h"

#include <Arduino.h>

#define DHTPIN 2
#define DHTTYPE DHT11

static DHT s_dht(DHTPIN, DHTTYPE);
static Adafruit_BMP085 s_bmp;

DHT& getDhtInstance() {
  return s_dht;
}

Adafruit_BMP085& getBmpInstance() {
  return s_bmp;
}

void scanI2CBusLayer() {
  I2CUtils::scanBus();
}

SensorInitStatus reinitBmp180Layer() {
  SensorInitStatus status = {};

  I2CUtils::initBus(Config::kI2cSdaPin, Config::kI2cSclPin);
  status.i2cInitialized = true;

  delay(20);

  status.bmpDetectedOnI2c = I2CUtils::isDevicePresent(Config::kBmp180I2cAddress);
  if (!status.bmpDetectedOnI2c) {
    Serial.printf("[BMP180] I2C device 0x%02X not detected on SDA=%u SCL=%u\n",
                  Config::kBmp180I2cAddress, Config::kI2cSdaPin, Config::kI2cSclPin);
    scanI2CBusLayer();
    return status;
  }

  uint8_t chipId = 0;
  status.bmpChipIdRead = I2CUtils::readRegister(Config::kBmp180I2cAddress, 0xD0, chipId);
  if (status.bmpChipIdRead) {
    status.bmpChipIdValid = (chipId == 0x55);
    Serial.printf("[BMP180] Chip ID register (0xD0): 0x%02X\n", chipId);
    if (!status.bmpChipIdValid) {
      Serial.println("[BMP180] Warning: unexpected chip ID (BMP180/BMP085 should be 0x55).");
    }
  } else {
    Serial.println("[BMP180] Unable to read chip ID register.");
  }

  uint8_t calib[22] = {};
  status.bmpCalibrationReadable = I2CUtils::readBlock(
      Config::kBmp180I2cAddress, 0xAA, calib, sizeof(calib));
  if (status.bmpCalibrationReadable) {
    bool allZero = true;
    bool allFF = true;
    for (size_t i = 0; i < sizeof(calib); ++i) {
      if (calib[i] != 0x00) allZero = false;
      if (calib[i] != 0xFF) allFF = false;
    }
    Serial.println((allZero || allFF)
                       ? "[BMP180] Calibration block content looks invalid."
                       : "[BMP180] Calibration block read looks valid.");
  } else {
    Serial.println("[BMP180] Unable to read calibration block (0xAA..0xBF).");
  }

  status.bmpLibraryInitialized = s_bmp.begin();
  if (!status.bmpLibraryInitialized) {
    Serial.println("[BMP180] Initialization failed after I2C probe.");
    return status;
  }

  Serial.println("[BMP180] Initialization successful.");
  return status;
}

SensorInitStatus initSensorHardwareLayer() {
  SensorInitStatus status = {};

  I2CUtils::initBus(Config::kI2cSdaPin, Config::kI2cSclPin);
  status.i2cInitialized = true;

  Serial.printf("[Boot] I2C initialized (SDA=%u SCL=%u).\n",
                Config::kI2cSdaPin, Config::kI2cSclPin);
  scanI2CBusLayer();

  s_dht.begin();
  status.dhtInitialized = true;
  Serial.printf("[Boot] DHT11 initialized on GPIO %d.\n", DHTPIN);

  const SensorInitStatus bmpStatus = reinitBmp180Layer();
  status.bmpDetectedOnI2c = bmpStatus.bmpDetectedOnI2c;
  status.bmpChipIdRead = bmpStatus.bmpChipIdRead;
  status.bmpChipIdValid = bmpStatus.bmpChipIdValid;
  status.bmpCalibrationReadable = bmpStatus.bmpCalibrationReadable;
  status.bmpLibraryInitialized = bmpStatus.bmpLibraryInitialized;

  if (!status.bmpLibraryInitialized) {
    Serial.println("[Boot] BMP180 initialization failed. Loop will retry reads.");
  }

  return status;
}
