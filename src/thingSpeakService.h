//! @file thingSpeakService.h
//! @brief ThingSpeak service header
//! @details This file contains the functions for posting weather data to ThingSpeak.
//! @author Karl Berger
//! @date 2025-05-14

#ifndef THINGSPEAK_SERVICE_H
#define THINGSPEAK_SERVICE_H

#include <Arduino.h>
#include <WiFiClient.h>	
#include "credentials.h" // Wi-Fi and weather station credentials
#include "weather.h"    // weather data	

extern String unitStatus; // ThingSpeak status

void postToThingSpeak();

#endif // THINGSPEAK_SERVICE_H
// End of file
