#pragma once

#include <math.h>

namespace PressureCorrection {

// Converts station pressure (Pa) to sea-level pressure (Pa) using barometric formula.
inline float toSeaLevelPa(float stationPressurePa, float altitudeMeters) {
  const float base = 1.0f - (altitudeMeters / 44330.0f);
  if (base <= 0.0f) {
    return NAN;
  }
  return stationPressurePa / powf(base, 5.255f);
}

// Converts station pressure (Pa) to sea-level pressure (hPa).
inline float toSeaLevelHpa(float stationPressurePa, float altitudeMeters) {
  return toSeaLevelPa(stationPressurePa, altitudeMeters) / 100.0f;
}

}  // namespace PressureCorrection
