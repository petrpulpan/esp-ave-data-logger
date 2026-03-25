#pragma once

#include <stdint.h>

// Obtains current UNIX epoch time from the configured NTP server.
bool getUnixTime(uint32_t& unixTime);
