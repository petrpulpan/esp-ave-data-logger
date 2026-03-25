#pragma once

#include "telemetry_types.h"

// Reads raw DHT11 sample values.
RawDhtSample readDhtSampleLayer();

// Reads raw BMP180 sample values and applies configured temperature correction.
RawBmpSample readBmpSampleLayer();
