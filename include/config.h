#pragma once

#include <stdint.h>

// ---------------------------------------------------------------------------
// Sensor reading result passed between modules
// ---------------------------------------------------------------------------
struct SensorReadings {
  float temperatureC;
  float humidityPct;
  float pressurePa;
};

// ---------------------------------------------------------------------------
// Project-wide configuration constants
// ---------------------------------------------------------------------------
namespace Config {

// Credentials / identifiers – defined in src/config.cpp
extern const char* kSsid;
extern const char* kPassword;
extern const char* kDeviceId;
extern const char* kNtpServer;

// I2C pins: D2=GPIO4=SDA, D1=GPIO5=SCL
constexpr uint8_t  kI2cSdaPin           = 4;
constexpr uint8_t  kI2cSclPin           = 5;
constexpr uint8_t  kBmp180I2cAddress    = 0x77;
constexpr float    kBmpSensorTemperatureCalibration = -1.0f;

// Altitude above sea level used for pressure correction (metres)
constexpr float    kAltitudeMeters      = 230.0f;

// Upload endpoint base URL – defined in src/config.cpp
extern const char* kBaseUrl;

// Timing (ms)
constexpr uint32_t kLoopIntervalMs      = 60000;
constexpr uint32_t kWifiTimeoutMs       = 20000;
constexpr uint32_t kNtpTimeoutMs        = 10000;
constexpr uint32_t kSensorRetryDelayMs  = 1500;

// Sensor retry / validation bounds
constexpr uint8_t  kSensorReadRetries   = 3;
constexpr float    kBmpMinPressurePa    = 30000.0f;
constexpr float    kBmpMaxPressurePa    = 120000.0f;

}  // namespace Config
