#pragma once

#include <Adafruit_BMP085.h>
#include <DHT.h>

// Shared driver instances owned by sensor_hw_init layer.
DHT& getDhtInstance();
Adafruit_BMP085& getBmpInstance();
