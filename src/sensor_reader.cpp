#include "sensor_reader.h"

#include "config.h"
#include "sensor_instances.h"

#include <math.h>

RawDhtSample readDhtSampleLayer() {
  RawDhtSample sample = {};
  sample.temperatureC = getDhtInstance().readTemperature();
  sample.humidityPct = getDhtInstance().readHumidity();
  sample.valid = !isnan(sample.temperatureC) && !isnan(sample.humidityPct);
  return sample;
}

RawBmpSample readBmpSampleLayer() {
  RawBmpSample sample = {};
  sample.temperatureC = getBmpInstance().readTemperature();
  sample.pressurePa = getBmpInstance().readPressure();

  if (!isnan(sample.temperatureC)) {
    sample.temperatureC += Config::kBmpSensorTemperatureCalibration;
  }

  sample.valid = !isnan(sample.temperatureC) && !isnan(sample.pressurePa);
  return sample;
}
