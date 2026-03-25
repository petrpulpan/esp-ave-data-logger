#pragma once

#include "telemetry_types.h"

// Initializes I2C bus, starts DHT11, and initializes BMP180 with diagnostics.
SensorInitStatus initSensorHardwareLayer();

// Re-initializes only BMP180 path (I2C probe + diagnostics + driver init).
SensorInitStatus reinitBmp180Layer();

// Scans I2C bus and logs detected device addresses.
void scanI2CBusLayer();
