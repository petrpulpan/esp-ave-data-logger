#pragma once

#include "telemetry_types.h"

struct SensorSelfTestResult {
	bool dhtOk;
	bool bmpOk;
	RawDhtSample dhtSample;
	RawBmpSample bmpSample;
	SensorInitStatus bmpInitStatus;
};

struct ConnectivitySelfTestResult {
	bool wifiOk;
	bool ntpOk;
	uint32_t unixTime;
};

struct StartupSelfTestSummary {
	SensorSelfTestResult sensor;
	ConnectivitySelfTestResult connectivity;
};

// Runs startup sensor checks and returns detailed sensor-side status.
SensorSelfTestResult runSensorSelfTest();

// Runs startup WiFi + NTP checks and returns detailed connectivity status.
ConnectivitySelfTestResult runConnectivitySelfTest();

// Aggregates startup checks and prints one summary line for boot logs.
StartupSelfTestSummary runStartupSelfTest();
