//! @file thingSpeakService.h
//! @brief ThingSpeak service header
//! @details This file contains the functions for posting weather data to ThingSpeak.
//! @author Karl Berger
//! @date 2025-05-16

#ifndef THINGSPEAK_SERVICE_H
#define THINGSPEAK_SERVICE_H

#include <Arduino.h> // for String

extern String unitStatus; // ThingSpeak status

void postToThingSpeak();

#endif // THINGSPEAK_SERVICE_H
// End of file
