#pragma once

#include "config.h"
#include <stdint.h>

// Build and send the HTTP GET upload request.
// Converts pressurePa to sea-level hPa before sending.
bool sendData(const SensorReadings& readings, uint32_t unixTime);
