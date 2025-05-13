//! @file wifiConnection.h
//! @brief Wi-Fi connection functions
//! @details This file contains the functions to connect to Wi-Fi and
//!          logon to the router.
//! @author Karl Berger
//! @date 2025-05-13

#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <ESP8266WiFi.h> // [manager] v2.0.0 Wi-Fi
#include "credentials.h"

void logonToRouter(); // function to connect to Wi-Fi

#endif // WIFI_CONNECTION_H
// End of file