/**
 * @file main.cpp
 * @brief Main application for the Magnetic Loop Antenna Controller
 * @details This program initializes communication with sensors and processes input data
 *
 * @author Karl Berger
 * @date 2025-05-19
 *
 * @note Ensure the device is connected as described in README.md.
 * 
 * @todo read day/night indicator and choose json data accordingly
 * @todo add APRS bulletin for sunrise/sunset
 * @todo test WUG API responses for absence. Do not update WX data if response missing
 * 
 * @link https://github.com/W4KRL/WUG_APRS_TS
 */

#define WUG_DEBUG //! uncomment this line for serial debug output

/*
******************************************************
******************** INCLUDES ************************
******************************************************
*/
#include <Arduino.h>           // Arduino functions
#include "analogClock.h"       // analog clock functions
#include "aphorismGenerator.h" // aphorism functions
#include "aprsService.h"       // APRS functions
#include "credentials.h"       // account information
#include "digitalClock.h"      // digital clock display
#include "indoorSensor.h"      // indoor sensor functions
#include "onetimeScreens.h"    // splash screen and information screens
#include "sequentialFrames.h"  // sequential weather and almanac frames
#include "tftDisplay.h"        // TFT display functions
#include "thingSpeakService.h" // ThingSpeak posting
#include "timezone_globals.h"  // timezone object
#include "unitConversions.h"   // unit conversions
#include "weatherService.h"    // weather data from Weather Underground API
#include "wifiConnection.h"    // Wi-Fi connection
#include "wug_debug.h"         // debug print macro
#include <TickTwo.h>           // v4.4.0 Stefan Staub https://github.com/sstaub/TickTwo

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
  Serial.begin(115200);  // serial monitor
  initSensor();          // initialize indoor sensor
  setupTFTDisplay();     // initialize TFT display
  splashScreen();        // stays on until logon is complete
  logonToRouter();       // connect to WiFi
  getWXcurrent();        // find latitude & longitude for your weather station
  setTimeZone();         // set timezone
  mountFS();             // mount LittleFS and prepare APRS bulletin file
  dataScreen();          // show configuration data
  getWXforecast();       // initialize weather data - needs lat/lon from getWXcurrent
  APRSsendWX();          // post WXcurrent to APRS weather
  delay(2000);           // delay to show connection info
  ;                      //! Start TickTwo timers
  tmrWXcurrent.start();  //   timer for current weather
  tmrWXforecast.start(); //   timer for forecasted weather
  tmrWXaprs.start();     //   timer for posting weather to APRS
  tmrFrame.start();      //   timer for sequential screens
} // setup()

/*
******************************************************
********************* LOOP ***************************
******************************************************
*/
void loop()
{
  events(); // ezTime events including autoconnect to NTP server
  updateClocks();
  processBulletins();

  //! Update the tasks
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