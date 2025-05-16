//! @file wifiConnection.cpp
//! @brief This file contains the function to connect to the Wi-Fi network.
//! @details The function logonToRouter() connects the ESP8266 to the specified Wi-Fi network.
//! @details The function uses the WiFi library to connect to the network and prints the IP address once connected.
//! @author Karl Berger
//! @date 2025-05-16

#include "wifiConnection.h"

#include <ESP8266WiFi.h> // [manager] v2.0.0 Wi-Fi
#include "credentials.h"
#include "wug_debug.h"

void logonToRouter()
{
	pinMode(LED_BUILTIN, OUTPUT); // Built-in LED

	WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Begin WiFi
	WiFi.mode(WIFI_STA);				  // Explicitly set mode, ESP defaults to STA+AP

	DEBUG_PRINT("\n\nConnecting to " + (String)WIFI_SSID);

	while (WiFi.status() != WL_CONNECTED)
	{ // Wait for Wi-Fi to connect
		delay(500);
		digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Toggle LED
		DEBUG_PRINT('.');
	}
	digitalWrite(LED_BUILTIN, HIGH); // Turn off LED

	DEBUG_PRINT("\nWi-Fi connected. IP address: ");
	DEBUG_PRINTLN(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer
} // logonToRouter()