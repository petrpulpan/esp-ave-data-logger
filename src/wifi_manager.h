#pragma once

#include <stdint.h>

bool connectWiFi();
void shutdownWiFi();
bool getUnixTime(uint32_t& unixTime);
