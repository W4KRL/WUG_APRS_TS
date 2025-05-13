//! @file wifiConnection.h
//! @brief Wi-Fi connection functions
//! @details This file contains the functions to connect to Wi-Fi and
//!          logon to the router.
//! @author Karl Berger
//! @date 2025-05-13

#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <ESP8266WiFi.h> // [manager] v2.0.0 Wi-Fi
// #include <WiFiClientSecure.h> // [manager] v2.0.0 Wi-FiClientSecure	
// #include <HTTPClient.h> // [manager] v2.0.0 HTTPClient
// #include <ArduinoJson.h> // [manager] v6.20.0 ArduinoJson	
#include "credentials.h"
#include "debug.h" // [manager] v1.0.0 Debug

/*
*******************************************************
************** Logon to your Wi-Fi router *************
*******************************************************
*/
void logonToRouter()
{
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // begin WiFi
  WiFi.mode(WIFI_STA);                  // explicitly set mode, esp defaults to STA+AP
  DEBUG_PRINT("\n\nConnecting to " + (String)WIFI_SSID);

  while (WiFi.status() != WL_CONNECTED)
  { // Wait for the Wi-Fi to connect
    delay(500);
    DEBUG_PRINT('.');
  }

  DEBUG_PRINT("\nWi-Fi connected. IP address: ");
  DEBUG_PRINTLN(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer
} // logonToRoutrer()

#endif // WIFI_CONNECTION_H
// End of file