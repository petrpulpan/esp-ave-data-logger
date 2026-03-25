#include "i2c_utils.h"

#include <Arduino.h>
#include <Wire.h>

namespace I2CUtils {

void initBus(uint8_t sdaPin, uint8_t sclPin, uint32_t clockHz) {
  Wire.begin(sdaPin, sclPin);
  Wire.setClock(clockHz);
}

bool isDevicePresent(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

bool readRegister(uint8_t address, uint8_t reg, uint8_t& value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(static_cast<int>(address), 1) != 1) return false;
  value = Wire.read();
  return true;
}

bool readBlock(uint8_t address, uint8_t startReg, uint8_t* buffer, size_t length) {
  if (!buffer || length == 0) return false;

  Wire.beginTransmission(address);
  Wire.write(startReg);
  if (Wire.endTransmission(false) != 0) return false;

  const int len = static_cast<int>(length);
  if (Wire.requestFrom(static_cast<int>(address), len) != len) return false;

  for (size_t i = 0; i < length; ++i) {
    if (!Wire.available()) return false;
    buffer[i] = Wire.read();
  }

  return true;
}

void scanBus() {
  Serial.println("[I2C] Scanning bus for devices...");
  uint8_t found = 0;

  for (uint8_t addr = 0x03; addr <= 0x77; ++addr) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      ++found;
      Serial.printf("[I2C] Found device at 0x%02X", addr);
      if (addr == 0x77) {
        Serial.print(" (BMP180/BMP085/BMP280 possible)");
      } else if (addr == 0x76) {
        Serial.print(" (BMP280/BME280 common)");
      }
      Serial.println();
    }
  }

  if (found == 0) {
    Serial.println("[I2C] No devices detected.");
  }
}

}  // namespace I2CUtils
