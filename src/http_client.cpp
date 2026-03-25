#include "http_client.h"
#include "config.h"
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <math.h>

// Builds and sends the HTTPS upload request with corrected pressure values.
bool sendData(const SensorReadings& readings, uint32_t unixTime) {
  // Convert station pressure (Pa) -> sea-level pressure (Pa) -> hPa
  // so backend values are comparable across sites with different elevations.
  const float seaLevelPressurePa =
      readings.pressurePa /
      powf(1.0f - (Config::kAltitudeMeters / 44330.0f), 5.255f);
  const float seaLevelPressureHpa = seaLevelPressurePa / 100.0f;

  Serial.printf("[HTTP] Pressure conversion station=%.2fPa altitude=%.2fm"
                " sea-level=%.2fPa %.2fhPa\n",
                readings.pressurePa, Config::kAltitudeMeters,
                seaLevelPressurePa, seaLevelPressureHpa);

  char requestUrl[320];
  snprintf(requestUrl, sizeof(requestUrl),
           "%s?id=%s&t=%.2f&h=%.2f&p=%.2f&time=%u",
           Config::kBaseUrl, Config::kDeviceId,
           readings.temperatureC, readings.humidityPct, seaLevelPressureHpa,
           static_cast<unsigned int>(unixTime));

  Serial.printf("[HTTP] Request URL: %s\n", requestUrl);

  std::unique_ptr<BearSSL::WiFiClientSecure> client(
      new BearSSL::WiFiClientSecure);
  // Current deployment does not validate CA/certificate chain.
  client->setInsecure();

  HTTPClient http;
  if (!http.begin(*client, requestUrl)) {
    Serial.println("[HTTP] Failed to initialize HTTP client.");
    return false;
  }

  // setUserAgent() overrides the library default better than addHeader("User-Agent", ...).
  http.setUserAgent("SIGFOX");
  Serial.println("[HTTP] User-Agent set to: SIGFOX");
  http.setTimeout(10000);

  const int httpCode = http.GET();
  if (httpCode <= 0) {
    Serial.printf("[HTTP] GET failed: %s\n",
                  http.errorToString(httpCode).c_str());
    http.end();
    return false;
  }

  const String payload = http.getString();
  Serial.printf("[HTTP] Response code: %d\n", httpCode);
  Serial.printf("[HTTP] Payload (%u bytes): %s\n",
                payload.length(), payload.c_str());
  http.end();
  return true;
}
