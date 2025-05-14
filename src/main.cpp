//! In folder WUG_APRS_TS
//! 12/26/2024 imported to Platformio from D1S-Display-WUG-VEVOR-APRS_24_12_22_WORKING.ino
//! Synced with https://github.com/W4KRL/WUG_APRS_TS

// this version accomodates modified and unmodified TFT screens
// it must be compiled as one or the other
// unmarked version has CS = D4
// use version v2 with TFT_MOD CS = D0
// expects AHT10 sensor

//! Weather code from D1S-Display-WBIT-Mod.ino
//! APRS code from D1M-WX2-APRS-TS.ino
//! ThingSpeak code from XXXX

// TODO read day/night indicator and choose json data accordingly
// TODO add APRS bulletin for sunrise/sunset
// TODO test WUG API responses for absence. Do not update WX data if response missing

#define DEBUG //! uncomment this line for serial debug output

// Displays local weather from VEVOR 7-in-1 Weather Station
// through Weather Underground PWS account
/* Configured in credentials.h
 ? Data Needed:
 *   Wi-Fi credentials (SSID & Password)
 *   WeatherUnderground.com API key
 *   Olson timezone
 *   APRS callsign & SSID
 *   APRS passcode
 *   ThingSpeak channel ID
 */

/*_____________________________________________________________________________
   Copyright(c) 2018 - 2025 Karl Berger dba IoT Kits https://w4krl.com

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   **The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.**

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   _____________________________________________________________________________
*/

/*
******************************************************
******************** INCLUDES ************************
******************************************************
*/

//! For general sketch
#include <Arduino.h>           // [builtin] PlatformIO
#include "wifiConnection.h"    // Wi-Fi connection functions
#include "timezone_globals.h"  // timezone object
#include "indoorSensor.h"      // indoor sensor functions
#include "onetimeScreens.h"    // splash screen
#include <TickTwo.h>           // [manager] v4.4.0 Stefan Staub https://github.com/sstaub/TickTwo
#include "unitConversions.h"   // unit conversion functions
#include "debug.h"             // [manager] v1.0.0 Debug
#include "tftDisplay.h"        // TFT display functions
#include "digitalClock.h"      // digital clock functions
#include "analogClock.h"       // analog clock functions
#include "sequentialFrames.h"  // sequential frames functions
#include "thingSpeakService.h" // ThingSpeak functions
#include "aprsService.h"       // APRS functions
#include "aphorismGenerator.h" // aphorism functions

/*
******************************************************
**************** DEVICE CONNECTIONS ******************
******************************************************
*/
/*
!            WEMOS D1 MINI PINOUTS
 /RST - TFT RST          TX - GPIO 1 not used
   A0 - ADC not used     RX - GPIO 3 not used
   D0 - GPIO 16 CS       D1 - GPIO 5 I2C SCL
   D5 - GPIO 14 SCK      D2 - GPIO 4 I2C SDA
   D6 - GPIO 12 MISO     D3 - GPIO 0 TFT_DC
   D7 - GPIO 13 MOSI     D4 - GPIO 2 LED_BUILTIN
   D8 - GPIO 15 SS      GND - return for sensor
  3V3 - power to sensor  5V - not used
                  |USB|

 ! TFT = WEMOS 1.4 shield connections
 ! Mod to screen moves CS from D4 to D0 to avoid LED flash
 ! Used on kits starting August 2022
  #define TFT_RST -1  // use -1 according to Wemos
  #define TFT_DC D3        // GPIO 0
  #define MOSI D7          // GPIO 13
  #define SCK D5           // GPIO 14
  #define TFT_LED NC       // no connection
*/

//! Scheduled tasks
TickTwo tmrWXcurrent(updateWXcurrent, WX_CURRENT_INTERVAL * 60 * 1000, 0, MILLIS);
TickTwo tmrWXforecast(getWXforecast, WX_FORECAST_INTERVAL * 60 * 1000, 0, MILLIS);
TickTwo tmrWXaprs(APRSsendWX, WX_APRS_INTERVAL * 60 * 1000, 0, MILLIS);
TickTwo tmrFrame(updateFrame, SCREEN_DURATION * 1000, 0, MILLIS);

/*
******************************************************
********************* SETUP **************************
******************************************************
*/
void setup()
{
  Serial.begin(115200);            // serial monitor
  Wire.begin(SDA, SCL);            // define I2C pins
  sensorType = initSensor();       // detect & initialize indoor sensor
  setupTFTDisplay();               // initialize TFT display
  pinMode(LED_BUILTIN, OUTPUT);    // built in LED
  digitalWrite(LED_BUILTIN, HIGH); // turn off LED
  splashScreen();                  // stays on until logon is complete
  logonToRouter();                 // connect to WiFi
  getWXcurrent();                  // find latitude & longitude for your weather station
  setTimeZone();                   // set timezone
  mountFS();                       // mount LittleFS and prepare APRS bulletin file
  dataScreen();                    // show configuration data
  getWXforecast();                 // initialize weather data - needs lat/lon from getWXcurrent
  APRSsendWX();                    // post WXcurrent to APRS weather
  delay(2000);                     // delay to show connection info

  // start TickTwo timers
  tmrWXcurrent.start();
  tmrWXforecast.start();
  tmrWXaprs.start();
  tmrFrame.start();
} // setup()

/*
******************************************************
********************* LOOP ***************************
******************************************************
*/
// Main loop function that handles periodic tasks, updates clocks, processes APRS bulletins,
// and manages scheduled timers for weather updates and display frames.
void loop()
{
  events(); // ezTime events including autoconnect to NTP server

  //! ezTime secondChanged() is unreliable
  static int oldsec = 0;
  if (myTZ.second() != oldsec)
  {
    oldsec = second();
    if (flipNumbers)
    {                           // user has selected the digital clock
      digitalClockFrame(false); // do not redraw frame
    }
    if (moveHands)
    {                          // user has selected the analog clock
      analogClockFrame(false); // do not redraw frame
    }
  }

  // static int lastHour = -1;
  // if (lastHour != myTZ.hour()) {
  //   lastHour = myTZ.hour();
  //   if (myTZ.minute() == 0) {
  //     bulletinText = pickAphorism(APHORISM_FILE, lineArray);
  //     APRSsendBulletin(bulletinText, "Q");
  //     unitStatus = myTZ.dateTime("d M ~A~M ") + bulletinText;
  //     postToThingSpeak();
  //     unitStatus = "";
  //   }
  // }

  //! process APRS bulletins
  //? Check if it is 0800 EST and the morning bulletin has not been sent
  String bulletinText = "";
  if (myTZ.hour() == 8 && myTZ.minute() == 0 && !amBulletinSent)
  {
    bulletinText = pickAphorism(APHORISM_FILE, lineArray);
    APRSsendBulletin(bulletinText, "M"); // send morning bulletin
    amBulletinSent = true;               // mark it sent

    unitStatus = myTZ.dateTime("d M ~A~M ") + bulletinText;
    postToThingSpeak();
    unitStatus = "";
  }
  //? Check if it is 2000 EST and the evening bulletin has not been sent
  if (myTZ.hour() == 20 && myTZ.minute() == 0 && !pmBulletinSent)
  {
    bulletinText = pickAphorism(APHORISM_FILE, lineArray);
    APRSsendBulletin(bulletinText, "E"); // send evening bulletin
    pmBulletinSent = true;               // mark it sent

    unitStatus = myTZ.dateTime("d M ~P~M ") + bulletinText;
    postToThingSpeak();
    unitStatus = "";
  }
  //? Reset the bulletin flags at midnight if either is true
  static int lastDay = -1;
  int currentDay = myTZ.day();
  if (currentDay != lastDay)
  {
    lastDay = currentDay;
    amBulletinSent = false;
    pmBulletinSent = false;
  }

  //! update the tasks
  tmrWXcurrent.update();  // get current weather
  tmrWXforecast.update(); // get forecasted weather
  tmrWXaprs.update();     // post weather data to APRS
  tmrFrame.update();      // change the display frame
} // loop()

/*
*******************************************************
******************* END OF CODE ***********************
*******************************************************
*/