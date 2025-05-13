//! @file indoorSensor.cpp
//! @brief Indoor sensor functions	
//! @details This file contains the functions to initialize and read the indoor sensor.
//! @author Karl Berger	
//! @date 2025-05-13

#include "indoorSensor.h"

Adafruit_AHTX0 aht; // Define here
int sensorType = SENSOR_NONE; // Define here
struct sensorTH indoor; // Define here

int initSensor()
{
  // detect sensor type and initialize
  int type = SENSOR_NONE;
  if (aht.begin()) // found AHT10 sensor if true
  {
    type = SENSOR_AHT10;
  }
  return type;
} // initSensor()

void readSensor()
{
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  indoor.tempC = temp.temperature;
  indoor.humid = humidity.relative_humidity;
} // readSensor()