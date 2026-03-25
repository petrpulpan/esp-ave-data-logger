#include <Arduino.h>
#include <unity.h>

#include "pressure_correction.h"

void test_sea_level_pressure_matches_formula_reference() {
  const float stationPa = 98765.0f;
  const float altitudeM = 230.0f;

  const float expectedPa = stationPa / powf(1.0f - (altitudeM / 44330.0f), 5.255f);
  const float actualPa = PressureCorrection::toSeaLevelPa(stationPa, altitudeM);

  TEST_ASSERT_FLOAT_WITHIN(0.01f, expectedPa, actualPa);
}

void test_sea_level_hpa_is_pa_divided_by_100() {
  const float stationPa = 100123.0f;
  const float altitudeM = 230.0f;

  const float pa = PressureCorrection::toSeaLevelPa(stationPa, altitudeM);
  const float hpa = PressureCorrection::toSeaLevelHpa(stationPa, altitudeM);

  TEST_ASSERT_FLOAT_WITHIN(0.0001f, pa / 100.0f, hpa);
}

void test_zero_altitude_returns_same_pressure_in_pa() {
  const float stationPa = 101325.0f;
  const float seaLevelPa = PressureCorrection::toSeaLevelPa(stationPa, 0.0f);

  TEST_ASSERT_FLOAT_WITHIN(0.001f, stationPa, seaLevelPa);
}

void test_invalid_altitude_returns_nan() {
  const float result = PressureCorrection::toSeaLevelPa(100000.0f, 50000.0f);
  TEST_ASSERT_TRUE(isnan(result));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_sea_level_pressure_matches_formula_reference);
  RUN_TEST(test_sea_level_hpa_is_pa_divided_by_100);
  RUN_TEST(test_zero_altitude_returns_same_pressure_in_pa);
  RUN_TEST(test_invalid_altitude_returns_nan);
  UNITY_END();
}

void loop() {}
