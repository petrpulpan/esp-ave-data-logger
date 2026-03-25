#include "self_test.h"
#include "sensors.h"
#include "wifi_manager.h"
#include "ntp_manager.h"

#include <Arduino.h>
#include <math.h>

// Executes startup sensor checks and returns all sensor-side status details.
SensorSelfTestResult runSensorSelfTest() {
  SensorSelfTestResult result = {};

  result.bmpInitStatus = initBmp180();

  for (uint8_t attempt = 1; attempt <= 2; ++attempt) {
    result.dhtSample = readDhtRaw();
    result.bmpSample = readBmpRaw();
    result.dhtOk = result.dhtSample.valid;
    result.bmpOk = result.bmpSample.valid;

    if (result.dhtOk) {
      Serial.printf("[SelfTest] DHT sample OK on attempt %u (T=%.2fC H=%.2f%%)\n",
                    attempt, result.dhtSample.temperatureC, result.dhtSample.humidityPct);
    }

    if (result.bmpOk) {
      Serial.printf("[SelfTest] BMP sample OK on attempt %u (T=%.2fC P=%.2fPa)\n",
                    attempt, result.bmpSample.temperatureC, result.bmpSample.pressurePa);
    }

    if (result.dhtOk && result.bmpOk) {
      // DHT11 vs BMP180 delta helps monitor drift after applying BMP calibration offset.
      const float delta = fabsf(result.bmpSample.temperatureC - result.dhtSample.temperatureC);
      Serial.printf("[SelfTest] Temperature comparison BMP180=%.2fC DHT11=%.2fC Delta=%.2fC\n",
                    result.bmpSample.temperatureC, result.dhtSample.temperatureC, delta);
      break;
    }

    // Wait before retry so DHT11 has time to provide a fresh stable sample.
    delay(1200);
  }

  return result;
}

// Executes startup connectivity checks and returns network-side status details.
ConnectivitySelfTestResult runConnectivitySelfTest() {
  ConnectivitySelfTestResult result = {};
  result.wifiOk = connectWiFi();
  if (result.wifiOk) {
    result.ntpOk = getUnixTime(result.unixTime);
  }
  shutdownWiFi();
  return result;
}

// Executes startup sensor/network checks and prints a consolidated status summary.
StartupSelfTestSummary runStartupSelfTest() {
  Serial.println("[SelfTest] Running startup checks...");

  StartupSelfTestSummary summary = {};
  summary.sensor = runSensorSelfTest();
  summary.connectivity = runConnectivitySelfTest();

  Serial.printf("[SelfTest] Summary WiFi=%s NTP=%s DHT11=%s BMP180=%s TempSource=%s\n",
                summary.connectivity.wifiOk                   ? "OK" : "FAIL",
                summary.connectivity.ntpOk                    ? "OK" : "FAIL",
                summary.sensor.dhtOk                          ? "OK" : "FAIL",
                summary.sensor.bmpInitStatus.bmpLibraryInitialized ? "OK" : "FAIL",
                summary.sensor.bmpOk                          ? "BMP180" : "UNAVAILABLE");

  return summary;
}
