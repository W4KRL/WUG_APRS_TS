/**
 * @file thingSpeakService.cpp
 * @brief ThingSpeak service implementation
 * @details This file contains the functions for posting weather data to ThingSpeak.
 * @author Karl Berger
 * @date 2025-05-16
 */

#include "thingSpeakService.h"

#include <WiFiClient.h>     // ThingSpeak connection
#include "credentials.h"    // Wi-Fi and weather station credentials
#include "weatherService.h" // weather data
#include "wug_debug.h"          // debug print

//! ************** THINGSPEAK ************************
#define THINGSPEAK_SERVER "api.thingspeak.com" // ThingSpeak Server
String unitStatus = "";                        // ThingSpeak status global

/*
*******************************************************
************** Post to ThingSpeak* ********************
*******************************************************
*/
void postToThingSpeak()
{
  // 12/20/2024
  WiFiClient client;
  // assemble and post the data
  if (client.connect(THINGSPEAK_SERVER, 80))
  {
    DEBUG_PRINT("ThingSpeak Server connected to channel: ");
    DEBUG_PRINTLN(TS_CHANNEL);

    String str = TS_WRITE_KEY;
    str += "&field1=" + String(wx.obsTemp);
    str += "&field2=" + String(wx.obsHumidity);
    str += "&field3=" + String(wx.obsPressure);
    str += "&field4=" + String(wx.obsWindSpeed);
    str += "&field5=" + String(wx.obsWindDir);
    str += "&field6=" + String(wx.obsSolarRadiation);
    str += "&field7=" + String(wx.obsPrecipTotal);
    str += "&field8=" + String(wx.obsPrecipRate);
    if (unitStatus != "")
    {
      str += "&status=" + unitStatus;
    }
    DEBUG_PRINTLN(str); // show ThingSpeak payload on serial monitor

    // post the data to ThingSpeak
    String dataStringLength = String(str.length());
    // consider
    // client.println("Content-Length: " + String(str.length()));
    client.println("POST /update HTTP/1.1");
    // client.println("Host: " + THINGSPEAK_SERVER);
    client.print("Host: ");
    client.println(THINGSPEAK_SERVER);
    client.println("Connection: close");
    client.print("X-THINGSPEAKAPIKEY: ");
    client.println(TS_WRITE_KEY);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + dataStringLength);
    client.println("");
    client.print(str);

    DEBUG_PRINTLN("ThingSpeak data sent.");
  }
  client.stop();
} // postToThingSpeak()