//! @file indoorSensor.h
//! @brief Indoor sensor functions
//! @details This file contains the functions to initialize and read the indoor sensor.
//! @author Karl Berger
//! @date 2025-05-13

#ifndef INDOOR_SENSOR_H
#define INDOOR_SENSOR_H

#include <Arduino.h>		// [builtin] PlatformIO
#include <Wire.h>			// [builtin] for I2C sensor
#include <Adafruit_AHTX0.h> // [manager] v2.0.5 AHT10

//! Sensor type enumeration
enum SensorType
{
	SENSOR_NONE = 0,
	SENSOR_DHT11,
	SENSOR_AHT10,
	SENSOR_HTU21D
};

extern Adafruit_AHTX0 aht; // instantiate indoor Temperature/Humidity Sensor
extern int sensorType;	   // default to no sensor
extern struct sensorTH	   // for indoor sensor
{
	float tempC;
	float humid;
} indoor;

int initSensor();  // detect & initialize indoor sensor
void readSensor(); // read indoor sensor

#endif // INDOOR_SENSOR_H
// End of file
