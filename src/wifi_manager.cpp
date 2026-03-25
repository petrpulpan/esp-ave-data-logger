#include "wifi_manager.h"
#include "config.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

// Powers on STA mode and connects to configured WiFi with timeout.
bool connectWiFi() {
  Serial.println("[WiFi] Powering on WiFi...");
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(false);
  WiFi.begin(Config::kSsid, Config::kPassword);

  // Bound connect time so a bad AP does not block the full telemetry cycle.
  const uint32_t startMs = millis();
  wl_status_t status = WiFi.status();
  while (status != WL_CONNECTED && (millis() - startMs) < Config::kWifiTimeoutMs) {
    Serial.printf("[WiFi] Connecting... status=%d elapsed=%lu ms\n", status,
                  millis() - startMs);
    delay(500);
    status = WiFi.status();
  }

  if (status != WL_CONNECTED) {
    Serial.printf("[WiFi] Connection failed, final status=%d\n", status);
    return false;
  }

  Serial.printf("[WiFi] Connected. IP=%s RSSI=%d dBm\n",
                WiFi.localIP().toString().c_str(), WiFi.RSSI());
  return true;
}

// Cleanly disconnects and powers down the ESP8266 WiFi radio.
void shutdownWiFi() {
  Serial.println("[WiFi] Disconnecting and powering down WiFi...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(50);
  Serial.println("[WiFi] WiFi is now OFF.");
}

