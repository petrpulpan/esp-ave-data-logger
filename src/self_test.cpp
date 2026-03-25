#include "self_test.h"
#include "config.h"
#include "sensors.h"
#include "wifi_manager.h"
#include <Arduino.h>
#include <math.h>

void runStartupSelfTest() {
  Serial.println("[SelfTest] Running startup checks...");

  bool dhtOk = false;
  bool bmpOk = false;
  float dhtTemp = 0.0f, dhtHum = 0.0f;
  float bmpTemp = 0.0f, bmpPressure = 0.0f;

  for (uint8_t attempt = 1; attempt <= 2; ++attempt) {
    dhtOk = readDHTRaw(dhtTemp, dhtHum);
    bmpOk = readBMPRaw(bmpTemp, bmpPressure);

    if (dhtOk) {
      Serial.printf("[SelfTest] DHT sample OK on attempt %u (T=%.2fC H=%.2f%%)\n",
                    attempt, dhtTemp, dhtHum);
    }

    if (bmpOk) {
      Serial.printf("[SelfTest] BMP sample OK on attempt %u (T=%.2fC P=%.2fPa)\n",
                    attempt, bmpTemp, bmpPressure);
    }

    if (dhtOk && bmpOk) {
      // DHT vs BMP delta helps monitor drift after applying BMP calibration offset.
      const float delta = fabsf(bmpTemp - dhtTemp);
      Serial.printf("[SelfTest] Temperature comparison BMP=%.2fC DHT=%.2fC Delta=%.2fC\n",
                    bmpTemp, dhtTemp, delta);
      break;
    }

    delay(1200);
  }

  const bool wifiOk = connectWiFi();
  bool ntpOk = false;
  uint32_t unixTime = 0;
  // Network checks are intentionally separate from sensor checks.
  if (wifiOk) {
    ntpOk = getUnixTime(unixTime);
  }
  shutdownWiFi();

  Serial.printf("[SelfTest] Summary WiFi=%s NTP=%s DHT=%s BMP=%s TempSource=%s\n",
                wifiOk         ? "OK" : "FAIL",
                ntpOk          ? "OK" : "FAIL",
                dhtOk          ? "OK" : "FAIL",
                gBmpInitialized? "OK" : "FAIL",
                bmpOk          ? "BMP180" : "UNAVAILABLE");
}
