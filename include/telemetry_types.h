#pragma once

#include <stdint.h>

struct SensorReadings {
  float temperatureC;
  float humidityPct;
  float pressurePa;
  float seaLevelPressureHpa;
};

struct SensorInitStatus {
  bool i2cInitialized;
  bool dhtInitialized;
  bool bmpDetectedOnI2c;
  bool bmpChipIdRead;
  bool bmpChipIdValid;
  bool bmpCalibrationReadable;
  bool bmpLibraryInitialized;
};

struct RawDhtSample {
  float temperatureC;
  float humidityPct;
  bool valid;
};

struct RawBmpSample {
  float temperatureC;
  float pressurePa;
  bool valid;
};

struct SensorReadStatus {
  bool ok;
  bool dhtValid;
  bool bmpValid;
  SensorInitStatus bmpInitStatus;
};
