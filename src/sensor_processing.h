#pragma once

#include "telemetry_types.h"

// Validates raw sensor samples and builds the telemetry payload model.
bool processSensorSamplesLayer(const RawDhtSample& dht,
                               const RawBmpSample& bmp,
                               SensorReadings& readings,
                               bool& dhtValid,
                               bool& bmpValid);
