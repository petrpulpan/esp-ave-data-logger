#pragma once

#include "telemetry_types.h"
#include <stdint.h>

// Build and send the HTTP GET upload request.
// Uses precomputed SensorReadings values for payload fields.
bool sendData(const SensorReadings& readings, uint32_t unixTime);
