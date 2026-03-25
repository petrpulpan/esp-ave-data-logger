#include "http_client.h"
#include "config.h"
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>

// Builds and sends the HTTPS upload request using precomputed sensor payload values.
bool sendData(const SensorReadings& readings, uint32_t unixTime) {
  Serial.printf("[HTTP] Upload values T=%.2fC H=%.2f%% P(sea-level)=%.2fhPa\n",
                readings.temperatureC, readings.humidityPct, readings.seaLevelPressureHpa);

  char requestUrl[320];
  snprintf(requestUrl, sizeof(requestUrl),
           "%s?id=%s&t=%.2f&h=%.2f&p=%.2f&time=%u",
           Config::kBaseUrl, Config::kDeviceId,
           readings.temperatureC, readings.humidityPct, readings.seaLevelPressureHpa,
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
