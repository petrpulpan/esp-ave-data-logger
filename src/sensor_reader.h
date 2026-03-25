#pragma once

#include "telemetry_types.h"

// Reads raw DHT11 sample values.
RawDhtSample readDhtSample();

// Reads raw BMP180 sample values and applies configured temperature correction.
RawBmpSample readBmpSample();
