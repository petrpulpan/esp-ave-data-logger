#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "sensors.h"
#include "http_client.h"
#include "self_test.h"

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(1200);

  Serial.println();
  Serial.println("[Boot] ESP8266 AVE data logger starting...");
  Serial.printf("[Boot] SDK: %s\n", ESP.getSdkVersion());
  Serial.println("[Boot] I2C wiring: D1=GPIO5=SCL, D2=GPIO4=SDA");

  initSensors();
  runStartupSelfTest();
  shutdownWiFi();
}

void loop() {
  Serial.println("========================================");
  Serial.println("[Loop] Starting new acquisition/upload cycle.");
  scanI2CBus();

  bool cycleOk = true;
  SensorReadings readings = {};
  uint32_t unixTime = 0;

  if (!connectWiFi())                           cycleOk = false;
  if (cycleOk && !getUnixTime(unixTime))        cycleOk = false;
  if (cycleOk && !readSensors(readings))        cycleOk = false;
  if (cycleOk && !sendData(readings, unixTime)) cycleOk = false;

  Serial.println(cycleOk ? "[Loop] Cycle completed successfully."
                         : "[Loop] Cycle completed with errors.");

  shutdownWiFi();
  Serial.printf("[Loop] Sleeping %u ms before next cycle.\n",
                static_cast<unsigned int>(Config::kLoopIntervalMs));
  delay(Config::kLoopIntervalMs);
}
