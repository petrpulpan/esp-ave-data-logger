#include "sensor_processing.h"

#include "config.h"
#include "pressure_correction.h"

#include <math.h>

bool processSensorSamples(const RawDhtSample& dht,
                          const RawBmpSample& bmp,
                          SensorReadings& readings,
                          bool& dhtValid,
                          bool& bmpValid) {
  dhtValid = dht.valid;
  bmpValid = bmp.valid &&
             bmp.pressurePa >= Config::kBmpMinPressurePa &&
             bmp.pressurePa <= Config::kBmpMaxPressurePa;

  if (!dhtValid || !bmpValid) {
    return false;
  }

  const float seaLevelPressureHpa =
      PressureCorrection::toSeaLevelHpa(bmp.pressurePa, Config::kAltitudeMeters);

  if (isnan(seaLevelPressureHpa)) {
    return false;
  }

  readings.temperatureC = bmp.temperatureC;
  readings.humidityPct = dht.humidityPct;
  readings.pressurePa = bmp.pressurePa;
  readings.seaLevelPressureHpa = seaLevelPressureHpa;
  return true;
}
