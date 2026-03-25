#include "ntp_manager.h"
#include "config.h"
#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Obtains current UNIX epoch time from the configured NTP server.
bool getUnixTime(uint32_t& unixTime) {
  Serial.println("[NTP] Starting NTP client...");

  // Local objects avoid static-initialisation-order issues with Config strings.
  // They also keep UDP/NTP resources short-lived between cycles.
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
