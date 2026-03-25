#pragma once

#include "config.h"

// Set to true by initBmp180() on success; readable by self_test
extern bool gBmpInitialized;

// Initialise I2C bus, DHT and BMP180 (call once from setup)
void initSensors();

// Scan I2C bus and log all found device addresses
void scanI2CBus();

// (Re-)initialise BMP180; called at boot and on read failures
bool initBmp180();

// Raw DHT read – caller checks isnan(); used by self-test
bool readDHTRaw(float& temperature, float& humidity);

// Raw BMP read – caller checks result; used by self-test diagnostics
bool readBMPRaw(float& temperature, float& pressure);

// Validate and fill readings (including precomputed sea-level pressure); retries on failure
bool readSensors(SensorReadings& readings);
