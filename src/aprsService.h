//! @file aprsService.h
//! @brief APRS service header
//! @details This file contains the functions for posting weather data to APRS-IS.
//! @author Karl Berger
//! @date 2025-05-14

#ifndef APRS_SERVICE_H
#define APRS_SERVICE_H

#include <Arduino.h>
// #include "credentials.h" // Wi-Fi and weather station credentials
// #include <WiFiClient.h>
// #include "weatherService.h"	 // weather data
// #include "unitConversions.h" // unit conversion functions

extern int lineCount;		// number of aphorisms in file
extern bool amBulletinSent; // APRS morning bulletin
extern bool pmBulletinSent; // APRS evening bulletin
extern int lineIndex;		// APRS bulletin index

void postToAPRS(String message);
String APRSformatWeather();
void APRSsendWX();
String APRSformatBulletin(String message, String ID);
void APRSsendBulletin(String msg, String ID);
String APRSpadder(float value, int width);
String APRSpadCall(String callSign);
String APRSlocation(float lat, float lon);

#endif // APRS_SERVICE_H
// End of file
