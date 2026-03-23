#include "wifi_manager.h"
#include "config.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

bool connectWiFi() {
  Serial.println("[WiFi] Powering on WiFi...");
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(false);
  WiFi.begin(Config::kSsid, Config::kPassword);

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

void shutdownWiFi() {
  Serial.println("[WiFi] Disconnecting and powering down WiFi...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(50);
  Serial.println("[WiFi] WiFi is now OFF.");
}

bool getUnixTime(uint32_t& unixTime) {
  Serial.println("[NTP] Starting NTP client...");

  // Local objects avoid static-initialisation-order issues with Config strings.
  WiFiUDP ntpUdp;
  NTPClient ntpClient(ntpUdp, Config::kNtpServer, 0, 60000);
  ntpClient.begin();

  const uint32_t startMs = millis();
  while ((millis() - startMs) < Config::kNtpTimeoutMs) {
    if (ntpClient.update() || ntpClient.forceUpdate()) {
      unixTime = ntpClient.getEpochTime();
      if (unixTime > 100000) {
        Serial.printf("[NTP] UNIX time synchronized: %u\n",
                      static_cast<unsigned int>(unixTime));
        ntpClient.end();
        return true;
      }
    }
    Serial.printf("[NTP] Waiting for sync... elapsed=%lu ms\n", millis() - startMs);
    delay(500);
  }

  ntpClient.end();
  Serial.println("[NTP] Failed to synchronize time within timeout.");
  return false;
}
