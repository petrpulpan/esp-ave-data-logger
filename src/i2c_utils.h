#pragma once

#include <stddef.h>
#include <stdint.h>

namespace I2CUtils {

// Initializes I2C bus pins and clock speed.
void initBus(uint8_t sdaPin, uint8_t sclPin, uint32_t clockHz = 100000);

// Returns true when an I2C device acknowledges on the specified address.
bool isDevicePresent(uint8_t address);

// Reads a single byte from an I2C register.
bool readRegister(uint8_t address, uint8_t reg, uint8_t& value);

// Reads a contiguous register block from an I2C device.
bool readBlock(uint8_t address, uint8_t startReg, uint8_t* buffer, size_t length);

// Scans the I2C bus and logs detected device addresses.
void scanBus();

}  // namespace I2CUtils
