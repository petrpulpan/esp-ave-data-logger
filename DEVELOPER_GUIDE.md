# ESP AVE Data Logger - Developer Guide

## Purpose

This firmware runs on an ESP8266-class board and performs a repeating telemetry cycle:

1. Bring up WiFi
2. Sync UNIX time from NTP
3. Read humidity from DHT11 and temperature/pressure from BMP180
4. Convert station pressure to sea-level pressure
5. Upload data by HTTPS GET
6. Shut down WiFi and sleep

The codebase is intentionally split into small modules so hardware, networking, and transport logic can be maintained independently.

## Project Layout

- include/config.h
  - Shared constants and project-wide configuration declarations
  - SensorReadings struct (raw pressure + precomputed sea-level pressure)
- src/config.cpp
  - Definitions of runtime string constants (SSID, password, device id, endpoint URL)
- include/pressure_correction.h
  - Pure pressure-correction formulas used by upload code and unit tests
- src/main.cpp
  - High-level orchestration only (setup and loop)
- src/i2c_utils.h + src/i2c_utils.cpp
  - Low-level I2C bus setup, probing, register/block reads, and scan logging
- src/sensors.h + src/sensors.cpp
  - DHT11 and BMP180 initialization/read logic
  - BMP180 diagnostics and validation
  - Read validation, BMP temperature calibration, retry handling
  - Computes and stores sea-level pressure in SensorReadings
- src/wifi_manager.h + src/wifi_manager.cpp
  - WiFi connect/disconnect only
- src/ntp_manager.h + src/ntp_manager.cpp
  - NTP sync and UNIX time retrieval
- src/http_client.h + src/http_client.cpp
  - URL build and HTTPS upload
  - User-Agent handling
  - Uses precomputed sea-level pressure from SensorReadings
- src/self_test.h + src/self_test.cpp
  - Startup health checks and summary logging
- test/test_pressure_correction/test_main.cpp
  - Unit tests for sea-level pressure correction formulas

## Build Environment

PlatformIO environment is defined in platformio.ini.

Current environment:

- env name: nodemcuv2
- platform: espressif8266
- board: esp8285
- framework: arduino
- monitor speed: 115200

Dependencies:

- adafruit/DHT sensor library
- adafruit/Adafruit BMP085 Library
- arduino-libraries/NTPClient

## Hardware Wiring

### Sensors

- DHT11 data pin -> GPIO2
- BMP180 I2C:
  - SDA -> GPIO4 (D2)
  - SCL -> GPIO5 (D1)

### Power

- Use 3.3V and common GND for sensor modules

## Configuration

Most constants are declared in include/config.h.

Runtime string values are defined in src/config.cpp:

- Config::kSsid
- Config::kPassword
- Config::kDeviceId
- Config::kNtpServer
- Config::kBaseUrl

Important tuning values in include/config.h:

- Config::kAltitudeMeters
  - Used for sea-level pressure conversion
- Config::kBmpSensorTemperatureCalibration
  - Additive correction applied to BMP180 temperature
  - Default value: -1.0 (deg C)
- Config::kLoopIntervalMs
- Config::kWifiTimeoutMs
- Config::kNtpTimeoutMs
- Config::kSensorReadRetries
- Config::kBmpMinPressurePa, Config::kBmpMaxPressurePa

Sensor source policy:

- Temperature: BMP180 (with Config::kBmpSensorTemperatureCalibration applied)
- Humidity: DHT11
- Pressure: BMP180

SensorReadings payload fields:

- pressurePa: raw station pressure from BMP180
- seaLevelPressureHpa: corrected pressure used for upload

## Main Runtime Flow

### setup

1. Start Serial at 115200 with debug output
2. Log boot diagnostics
3. Initialize sensors (I2C, DHT11, BMP180)
4. Run startup self-test (WiFi, NTP, DHT, BMP summary + BMP vs DHT temperature comparison)
5. Shutdown WiFi

### loop

1. Log cycle header
2. Optional I2C scan log
3. Connect WiFi
4. Get UNIX time via NTP (ntp_manager)
5. Read validated sensor data
6. Send HTTPS GET upload
7. Shutdown WiFi
8. Delay until next cycle

## Pressure Conversion Model

In include/pressure_correction.h, pressure-correction formulas are defined and reused by sensor processing and unit tests.
In src/sensors.cpp, sea-level pressure is computed and stored in SensorReadings.
In src/http_client.cpp, uploaded pressure value p uses SensorReadings.seaLevelPressureHpa.

Station pressure from BMP180 is in Pa and corrected by altitude:

seaLevelPressurePa = pressurePa / pow(1 - altitudeMeters / 44330, 5.255)
seaLevelPressureHpa = seaLevelPressurePa / 100

BMP180 temperature correction model:

temperatureCorrectedC = temperatureBmpRawC + bmpSensorTemperatureCalibration

The URL currently sends temperature, humidity, and pressure with two decimal places.
Temperature in the URL is the corrected BMP180 value.

## Logging Conventions

Logs are prefixed by subsystem for easy monitor filtering:

- [Boot]
- [I2C]
- [BMP180]
- [SelfTest]
- [WiFi]
- [NTP]
- [Sensors]
- [HTTP]
- [Loop]

## Typical Commands

Build:

```powershell
C:\Users\Petr\.platformio\penv\Scripts\platformio.exe run --environment nodemcuv2
```

Upload:

```powershell
C:\Users\Petr\.platformio\penv\Scripts\platformio.exe run --target upload --environment nodemcuv2
```

Monitor:

```powershell
C:\Users\Petr\.platformio\penv\Scripts\platformio.exe device monitor --baud 115200
```

Unit tests (host/native):

```powershell
C:\Users\Petr\.platformio\penv\Scripts\platformio.exe test --environment native
```

## Operational Notes

- If you do not see early boot logs, open monitor first, then reset board.
- If build exits with Windows file-lock warnings on .pio/build:
  - stop running monitor/upload tasks
  - retry build
- Endpoint uses HTTPS with insecure certificate mode in current implementation.
  - For production hardening, pin server certificate or CA.

## Extension Ideas

- Add offline queue when upload fails
- Add watchdog-based recovery policy
- Add compile-time feature flags (self-test on/off, scan verbosity)
- Add support for BMP280/BME280 fallback driver
