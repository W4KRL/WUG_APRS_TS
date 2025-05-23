/**
 * @file credentials.h
 * @brief Wi-Fi and Weather Underground credentials
 * @details This file contains the Wi-Fi and Weather Underground credentials
 * @author Karl Berger
 * @date 2025-05-23
 * @note This file must reside in the same Arduino directory as the main sketch.
 */

#ifndef CREDENTIALS_H
#define CREDENTIALS_H

const String FW_VERSION = "250523"; // Firmware version

// Wi-Fi Credentials
//! Place all values in quotes " "
#define WIFI_SSID "DCMNET"                   // your Wi-Fi SSID
#define WIFI_PASSWORD "0F1A2D3E4D5G6L7O8R9Y" // your Wi-Fi password

// Weather Underground credentials
//! Place all values in quotes " "
#define WX_STATION_ID "KVACENTR126"               // your Weather Underground Personal Weather Station (PWS) id
#define WX_KEY "c41eb27afef64b6b9eb27afef62b6bed" // your Weather Underground API key
#define MY_TIMEZONE "America/New_York"            // Olson timezone https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
/*
Common Olson Timezones:
EST  America/New_York
CST  America/Chicago
MST  America/Denver
PST  America/Los_Angeles
AKST America/Juneau
HST  Pacific/Honolulu
NST  America/St_Johns
AST  America/Halifax
EST  America/Toronto
CST  America/Winnipeg
MST  America/Edmonton
PST  America/Vancouver
GMT  Europe/London
CET  Europe/Berlin
WAT  Africa/Lagos
JST  Asia/Tokyo
KST  Asia/Seoul
CST  Asia/Shanghai
IST  Asia/Kolkata
*/

// APRS credentials
//! Place all values in quotes " "
const String CALLSIGN = "W4KRL-13"; // call-SSID
const String APRS_PASSCODE = "9092";  // https://aprs.do3sww.de/
const String APHORISM_FILE = "/aphorisms.txt";

// ThingSpeak Credentials
//! Place all values in quotes " "
#define TS_WRITE_KEY "KKJB0YPV19V9KMX0"
#define TS_CHANNEL "88666"
/* ThingSpeak Field Reference:
   Read Key  CRYD7ZD9AY8HHOXP
   Field 1 TempC
   Field 2 Humid
   Field 3 Pressure
   Filed 4 WindSpeed
   Field 5 WindDir
   Field 6 WindGust
   Field 7 RailToday
   Field 8 RainRate
   Status
*/

// Weather update intervals (note minutes)
//! Use integer values. No quote marks
const unsigned int WX_CURRENT_INTERVAL = 7;   // minutes between current weather requests (Should be >= 1)
const unsigned int WX_FORECAST_INTERVAL = 13; // minutes between forecast requests
const unsigned int WX_APRS_INTERVAL = 10;     // minutes between posting weather data to APRS (Must be >= 5)
const unsigned int SCREEN_DURATION = 5;       // display frame interval in !!!seconds!!!

// Display selections
//! Use true or false. No quote marks
const bool DIGITAL_CLOCK = true;   // display digital clock (true/false)
const bool ANALOG_CLOCK = true;   // display analog clock (true/false)
const bool METRIC_DISPLAY = false; // weather display units

#endif // CREDENTIALS_H
// End of file
