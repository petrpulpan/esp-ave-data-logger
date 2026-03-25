#pragma once

#include "telemetry_types.h"

// Initializes I2C/DHT/BMP hardware and returns detailed init status.
SensorInitStatus initSensors();

// Scans I2C bus and logs detected device addresses.
void scanI2CBus();

// Re-initializes BMP180 path and returns detailed init status.
SensorInitStatus initBmp180();

// Reads one raw DHT sample and returns status + values.
RawDhtSample readDhtRaw();

// Reads one raw BMP sample and returns status + values.
RawBmpSample readBmpRaw();

// Reads validated telemetry values with retry/recovery and optional detail status.
bool readSensors(SensorReadings& readings, SensorReadStatus* status = nullptr);
