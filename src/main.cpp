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
*************** INCLUDES & MACROS ********************
******************************************************
*/

//! For general sketch
#include <Arduino.h>           // [builtin] PlatformIO
#include "credentials.h"       // Wi-Fi and weather station credentials
#include "wifiConnection.h"    // Wi-Fi connection functions
#include "timezone_globals.h" // timezone object
#include "indoorSensor.h" // indoor sensor functions
#include "onetimeScreens.h" // splash screen
#include <ESP8266HTTPClient.h> // [builtin] for http and https
#include <WiFiClientSecure.h>  // [builtin] for https
#include <ArduinoJson.h>       // [manager] v7.2 Benoit Blanchon https://arduinojson.org/
#include <TickTwo.h>           // [manager] v4.4.0 Stefan Staub https://github.com/sstaub/TickTwo
#include "unitConversions.h" // unit conversion functions
#include "debug.h"         // [manager] v1.0.0 Debug
#include "tftDisplay.h" // TFT display functions

//! For APRS bulletins
#include <LittleFS.h> // [builtin]

struct phaseName
{ //! for moon phase name
  String firstWord;
  String secondWord;
} moonName;

/*
******************************************************
************* FUNCTION PROTOTYPES ********************
******************************************************
*/

void updateFrame();                                    // display frames for selected durations
void firstWXframe();                                   // current conditions
void secondWXframe();                                  // forecasted conditions
void almanacFrame();                                   // date, sun, and moon data
void digitalClockFrame(bool drawFrame);                // UTC & local time
void analogClockFrame(bool drawFrame);                 // UTC & local time, indoor temp & humid
void getWXforecast();                                  // get forecasted weather
void getWXcurrent();                                   // get current conditions
void updateWXcurrent();                                // update weather data
String APRSformatBulletin(String message, String ID);  // format APRS bulletin or announcement
String APRSlocation(float lat, float lon);             // format APRS location
String APRSpadder(float value, int width);             // format APRS data
void APRSsendBulletin(String msg, String ID);          // send APRS bulletin or announcement
void APRSsendWX();                                     // send APRS weather data
void postToAPRS(String message);                       // post data to APRS
float moonPhase();                                     // moon phase lunation fraction from Unix timestamp
phaseName findPhaseName(float fraction);               // moon phase name two words
String getRainIntensity(float rate);                   // convert rain rate to intensity
String getCompassDirection(int degrees);               // convert degrees to compass direction
void mountFS();                                        // mount LittleFS
void shuffleArray(int *array, int size);               // shuffle array
String pickAphorism(const char *fileName, int *array); // pick an aphorism
void postToThingSpeak();                               // post data to ThingSpeak
void drawMoonFace(int xc, int yc, int r, float fract, int litColor, int darkColor);
void fetchDataAndParse(String getQuery, JsonDocument &filter, JsonDocument &doc);

//! scheduled tasks
TickTwo tmrWXcurrent(updateWXcurrent, WX_CURRENT_INTERVAL * 60 * 1000, 0, MILLIS);
TickTwo tmrWXforecast(getWXforecast, WX_FORECAST_INTERVAL * 60 * 1000, 0, MILLIS);
TickTwo tmrWXaprs(APRSsendWX, WX_APRS_INTERVAL * 60 * 1000, 0, MILLIS);
TickTwo tmrFrame(updateFrame, SCREEN_DURATION * 1000, 0, MILLIS);

/*
******************************************************
******************* DEFAULTS *************************
******************************************************
*/

//! ***** Weather Underground Personal Weather Station (PWS) *****
// !!! DO NOT CHANGE !!!
// documentation: https://docs.google.com/document/d/1eKCnKXI9xnoMGRRzOL1xPCBihNV2rOet08qpE_gArAY/edit?tab=t.0
// WX_KEY is in credentials.h
#define WX_HOST "https://api.weather.com"        // Weather Underground PWS host
#define WX_CURRENT "v2/pws/observations/current" // PWS current observations API
#define WX_FORECAST "v3/wx/forecast/daily/5day"  // PWS 5-day forecast API
#define WX_LANGUAGE "en-US"                      // language for forecast
#define WX_UNITS "m"                             // e = english, m = metric MUST USE METRIC!!!
#define WX_FORMAT "json"                         // data format
#define WX_PRECISION "decimal"                   // null = integer, decimal = decimal

//! ***************** APRS *******************
//            !!! DO NOT CHANGE !!!
// for list of tier 2 servers: http://www.aprs2.net/
// North America: noam.aprs2.net
// South America: soam.aprs2.net
// Europe: euro.aprs2.net
// Asia: asia.aprs2.net
// Africa: africa.aprs2.net
// Oceania: apan.aprs2.net
#define APRS_SERVER "noam.aprs2.net"                   // recommended for North America
#define APRS_DEVICE_NAME "https://w4krl.com/iot-kits/" // link to my website
#define APRS_SOFTWARE_NAME "D1S-VEVOR"                 // unit ID
#define APRS_SOFTWARE_VERS String(FW_VERSION)          // FW version
#define APRS_PORT 14580                                // do not change port
#define APRS_TIMEOUT 2000L                             // milliseconds

//! ************ APRS Bulletin globals ***************
int *lineArray;              // holds shuffled index to aphorisms
int lineCount;               // number of aphorisms in file
bool amBulletinSent = false; // APRS morning bulletin
bool pmBulletinSent = false; // APRS evening bulletin
int lineIndex = 1;           // APRS bulletin index

//! ************** THINGSPEAK ************************
#define THINGSPEAK_SERVER "api.thingspeak.com" // ThingSpeak Server
String unitStatus = "";                        // ThingSpeak status

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

/*
******************************************************
******************* GLOBALS **************************
******************************************************
*/

//! Clock flags
bool moveHands = false;   // analog clock update hand
bool flipNumbers = false; // digital clock unpdate numerals

/*
******************************************************
********************* SETUP **************************
******************************************************
*/
void setup()
{
  Serial.begin(115200);                 // serial monitor
  Wire.begin(SDA, SCL);                 // define I2C pins
  sensorType = initSensor();            // detect & initialize indoor sensor
  setupTFTDisplay();               // initialize TFT display
  pinMode(LED_BUILTIN, OUTPUT);         // built in LED
  digitalWrite(LED_BUILTIN, HIGH);      // turn off LED
  splashScreen();                       // stays on until logon is complete
  logonToRouter();                      // connect to WiFi
  getWXcurrent();                       // find latitude & longitude for your weather station
  waitForSync();                        // sync ezTime with a net time server

  /*
   if timezone has not been cached in EEPROM
   or user is asking for a new timezone set the timezone
   */
  if (!myTZ.setCache(0) || myTZ.getOlson() != MY_TIMEZONE)
  {
    myTZ.setLocation(MY_TIMEZONE);
  }
  myTZ.setDefault(); // set local timezone
  mountFS();         // mount LittleFS and prepare APRS bulletin file
  dataScreen();      // show configuration data
  getWXforecast();   // initialize weather data - needs lat/lon from getWXcurrent
  APRSsendWX();      // post WXcurrent to APRS weather
  delay(2000);       // delay to show connection info

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
void loop()
{
  events(); // ezTime events including autoconnect to NTP server

  //! ezTime secondChanged() is unreliable
  static int oldsec = 0;
  if (second() != oldsec)
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
  if (myTZ.hour() == 0 && myTZ.minute() == 0 && (amBulletinSent || pmBulletinSent))
  {
    amBulletinSent = false;
    pmBulletinSent = false;
  }

  // process TickTwo timers
  tmrWXcurrent.update();  // get current weather
  tmrWXforecast.update(); // get forecasted weather
  tmrWXaprs.update();     // post weather data to APRS
  tmrFrame.update();      // change the display frame
} // loop()

/*
******************************************************
******************* SCREENS & FRAMES *****************
******************************************************
*/

// /*
// ******************************************************
// ******************* Splash Screen ********************
// ******************************************************
// */
// void splashScreen()
// {
//   // 11/24/2024
//   tft.setFreeFont(LargeBold);
//   tft.fillScreen(BLUE);
//   tft.setTextColor(YELLOW);
//   int tl = 19;
//   tft.setTextDatum(TC_DATUM); // font top, centered
//   tft.drawString("D1S-WUG", SCREEN_W2, tl);
//   tft.drawString("Display", SCREEN_W2, tl + 20);
//   tft.drawString("by", SCREEN_W2, tl + 40);
//   tft.drawString("IoT Kits", SCREEN_W2, tl + 60);
//   tft.drawString("v" + String(FW_VERSION) + "-M", SCREEN_W2, tl + 80);
//   for (int i = 0; i < 4; i++)
//   {
//     tft.drawRoundRect(12 - 3 * i, 12 - 3 * i, SCREEN_W - 12, SCREEN_H - 12, 8, YELLOW);
//   }
//   tft.unloadFont();
// } // splashScreen()

// /*
// ******************************************************
// ****************** Data Screen ***********************
// ******************************************************
// */
// void dataScreen()
// {
//   // 12/15/2024
//   tft.fillScreen(BLACK); // clear screen
//   String units = (METRIC_DISPLAY) ? "Metric" : "Imperial";
//   String clock;
//   if (ANALOG_CLOCK && DIGITAL_CLOCK)
//   {
//     clock = "Analog&Digital";
//   }
//   else if (ANALOG_CLOCK && !DIGITAL_CLOCK)
//   {
//     clock = "Analog";
//   }
//   else if (!ANALOG_CLOCK && DIGITAL_CLOCK)
//   {
//     clock = "Digital";
//   }
//   else
//   {
//     clock = "No";
//   }
//   clock += " Clock";

//   int TR = 0;  // top row
//   int LS = 14; // line spacing
//   tft.setFreeFont(SmallBold);
//   tft.setTextDatum(TC_DATUM); // center text
//   tft.setTextColor(YELLOW);
//   tft.drawString("Location:", SCREEN_W2, TR);
//   tft.drawString(String(wx.obsLat, 2) + "/" + String(wx.obsLon, 2), SCREEN_W2, TR + LS);
//   tft.drawString(wx.obsNeighborhood, SCREEN_W2, TR + 2 * LS);
//   tft.setTextColor(GREEN);
//   tft.drawString("Time Zone:" + getTimezoneName(), SCREEN_W2, TR + 3 * LS);
//   tft.drawString(MY_TIMEZONE, SCREEN_W2, TR + 4 * LS); // Olsen timezone
//   tft.setTextColor(WHITE);
//   tft.drawString("Units & Duration:", SCREEN_W2, TR + 5 * LS);
//   tft.drawString(units + " " + SCREEN_DURATION + " seconds", SCREEN_W2, TR + 6 * LS);
//   tft.drawString(clock, SCREEN_W2, TR + 7 * LS);
//   tft.setTextColor(RED);
//   tft.drawString(APHORISM_FILE, SCREEN_W2, TR + 8 * LS);

//   tft.unloadFont();
// } // dataScreen()

/*
******************************************************
***************** FRAME UPDATE ***********************
******************************************************
*/
void updateFrame()
{
  moveHands = false;   // supress hand movement until analog clock is displayed
  flipNumbers = false; // supress numeral change unless digital clock is displayed

  int maxFrames = 3; // number of display frames w/o clocks
  if (ANALOG_CLOCK)
  { // incrment number of frames
    maxFrames++;
  }
  if (DIGITAL_CLOCK)
  { // incrment number of frames
    maxFrames++;
  }

  static int frame = 0; // keep track of last frame between calls
  frame++;              // increment last frame number
  if (frame > maxFrames)
  {
    frame = 1; // roll back to first frame
  }

  switch (frame) // choose frame to display
  {
  case 1: // first weather frame
    firstWXframe();
    break;
  case 2: // second weather frame
    secondWXframe();
    break;
  case 3: // almanac frame
    almanacFrame();
    break;
  case 4: // could be digital clock if not analog
    if (ANALOG_CLOCK)
    {                         // process analog clock unless not selected
      analogClockFrame(true); // draw clock frame
      moveHands = true;       // enable update on secondsChanged() in loop()
    }
    else
    {                          // otherwise process digital clock
      digitalClockFrame(true); // draw clock frame
      flipNumbers = true;      // enable update on secondsChanged() in loop()
    } //
    break;                   //
  case 5:                    // definitely digital
    digitalClockFrame(true); // draw clock frame
    flipNumbers = true;      // enable update on secondsChanged() in loop()
    break;
  default:
    break;
  }
} // updateFrame()

/*
******************************************************
****************** Frame Panels **********************
******************************************************
*/
void drawFramePanels(int top_background, int bottom_background)
{
  // draws an upper and lower panel for the frame
  tft.fillRoundRect(0, 0, SCREEN_W, 2 * HEADER_RAD, HEADER_RAD, top_background);
  tft.fillRect(0, HEADER_RAD, SCREEN_W, HEADER_Y - HEADER_RAD, top_background);
  tft.fillRect(0, HEADER_Y, SCREEN_W, SCREEN_H - HEADER_Y - HEADER_RAD, bottom_background);
  tft.fillRoundRect(0, SCREEN_H - 2 * HEADER_RAD, SCREEN_W, 2 * HEADER_RAD, HEADER_RAD, bottom_background);
} // drawFramePanels()

/*
******************************************************
************* FIRST WEATHER FRAME ********************
******************************************************
*/
void firstWXframe()
{
  // 12/04/2024 fixed UV number display

  // prepare values
  String dispTempNow = (METRIC_DISPLAY) ? String(wx.obsTemp, 1) + " C" : String(CtoF(wx.obsTemp), 0) + " F";
  float feelTempC = (wx.obsTemp > 18) ? wx.obsHeatIndex : wx.obsWindChill;
  String dispTempFeel = (METRIC_DISPLAY) ? String(feelTempC, 0) + " C" : String(CtoF(feelTempC), 0) + " F";
  String dispPrecipType = "Rain";
  String dispPrecipAmt = (METRIC_DISPLAY) ? String(wx.obsPrecipTotal, 0) + " mm" : String(MMtoIN(wx.obsPrecipTotal), 2) + " in"; // total today
  String dispPrecipRate = getRainIntensity(wx.obsPrecipRate);                                                                   // rate in words
  // https://www.epa.gov/sunsafety/uv-index-scale-0
  String uvLabel = "";
  int uvText = YELLOW;       // text color
  int uvBG = 0;              // background color
  int uvi = round(wx.obsUV); // round and convert to int
  if (uvi < 0)
  {
    // No report or not available
    uvLabel = "N/A";
    uvBG = BLACK;
    uvText = WHITE;
  }
  else if (uvi < 3)
  {
    // No protection needed. Safely stay outside using minimal sun protection.
    uvLabel = "Low";
    uvBG = GREEN;
    uvText = BLACK;
  }
  else if (uvi < 6)
  {
    // Protection needed. Seek shade during late morning through mid-afternoon. Use sunscreen.
    uvLabel = "Moderate";
    uvBG = YELLOW;
    uvText = BLUE;
  }
  else if (uvi < 8)
  {
    // Protection needed. Seek shade during late morning through mid-afternoon. Use sunscreen.
    uvLabel = "High";
    uvBG = OUTRAGEOUS_ORANGE;
  }
  else if (uvi < 11)
  {
    // Extra protection needed. Seek shade and wear protective clothing. Use sunscreen.
    uvLabel = "Very High";
    uvBG = ALIZARIN_CRIMSON; // ORANGERED;
  }
  else
  {
    uvLabel = "Extreme";
    uvBG = MAGENTA;
  }

  // print labels, values, abd units
  drawFramePanels(C_WX_TOP_BG, C_WX_BOTTOM_BG);
  int TR = 2;  // top row
  int LS = 18; // line spacing

  tft.setTextColor(C_WX_TOP_TEXT);
  tft.setTextDatum(TC_DATUM);
  tft.setFreeFont(LargeBold);
  tft.drawString("Weather", SCREEN_W2, TR);
  if (tft.textWidth(wx.forPhraseLong) < SCREEN_W)
  {                                                       // Will the long phrase fit on the screen?
    tft.drawString(wx.forPhraseLong, SCREEN_W2, TR + LS); // If yes, print long phrase in large font
  }
  else
  {                             // else
    tft.setFreeFont(SmallBold); // Change to small font
    if (tft.textWidth(wx.forPhraseLong) < SCREEN_W)
    {                                                       // Will the long phrase fit on the screen?
      tft.drawString(wx.forPhraseLong, SCREEN_W2, TR + LS); // If yes, print the long phrase in small font
    }
    else
    {                                                        // else
      tft.setFreeFont(LargeBold);                            // Change to large font - we know short will fit
      tft.drawString(wx.forPhraseShort, SCREEN_W2, TR + LS); // Print the short phrase in the large font
    }
  }

  // print the labels flushed left
  tft.setFreeFont(LargeBold);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(YELLOW);
  tft.drawString("Temp", LEFT_COL, TR + 2 * LS);
  tft.drawString("Feel", LEFT_COL, TR + 3 * LS);
  tft.drawString(dispPrecipType, LEFT_COL, TR + 4 * LS);

  // print the data flushed right
  tft.setTextDatum(TR_DATUM);
  tft.drawString(dispTempNow, RIGHT_COL, TR + 2 * LS);
  tft.drawString(dispTempFeel, RIGHT_COL, TR + 3 * LS);
  tft.drawString(dispPrecipAmt, RIGHT_COL, TR + 4 * LS);
  tft.drawString(dispPrecipRate, RIGHT_COL, TR + 5 * LS);

  // panel for UV index, backgound color varies with UV index
  // top is straight, bottom is rounded
  // overlay a rounded rectangle
  tft.fillRect(0, 109, SCREEN_W, HEADER_RAD, uvBG);
  tft.fillRoundRect(0, SCREEN_H - 2 * HEADER_RAD, SCREEN_W, 2 * HEADER_RAD, HEADER_RAD, uvBG);
  tft.setTextColor(uvText);
  tft.setTextDatum(TL_DATUM); // flush left
  tft.drawString("UV " + (String)uvi, LEFT_COL, TR + 6 * LS);
  tft.setTextDatum(TR_DATUM); // flush right
  tft.drawString(uvLabel, RIGHT_COL, TR + 6 * LS);

  tft.unloadFont();
} // firstWXframe()

/*
******************************************************
*************** SECOND WEATHER FRAME *****************
******************************************************
*/
void secondWXframe()
{
  // 11/24/2024

  // calculate values
  // use 1 decimal for Celsius, none for Fahrenheit
  String tempMax = (METRIC_DISPLAY) ? String(wx.forTempMax, 1) : String(CtoF(wx.forTempMax), 0);
  String tempMin = (METRIC_DISPLAY) ? String(wx.forTempMin, 1) + "°C" : String(CtoF(wx.forTempMin), 0) + "°F";
  String dispWindSpeed = (METRIC_DISPLAY) ? String(wx.obsWindSpeed, 0) + " kph" : String(KMtoMILES(wx.obsWindSpeed), 0) + " mph";
  String dispGust = (METRIC_DISPLAY) ? String(wx.obsWindGust, 0) + " kph" : String(KMtoMILES(wx.obsWindSpeed), 0) + " mph";
  String dispSLP = (METRIC_DISPLAY) ? String(wx.obsPressure, 0) + " mb" : String(HPAtoINHG(wx.obsPressure), 2) + " in";
  String dispCloud = String(wx.forCloud) + "%";
  String dispHumid = String(wx.obsHumidity, 0) + "%";

  // print labels, values, and units
  drawFramePanels(C_WX_TOP_BG, C_WX_BOTTOM_BG);

  int TR = 2;  // top row
  int LS = 18; // line spacing
  tft.setTextColor(C_WX_TOP_TEXT);
  tft.setFreeFont(LargeBold);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Forecast Hi/Lo", SCREEN_W2, TR); // high and low temperature forecasts
  tft.drawString(tempMax + "/" + tempMin, SCREEN_W2, TR + LS);

  // labels flushed left  // labels flushed left
  tft.setTextColor(C_WX_BOTTOM_TEXT);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Gust", LEFT_COL, TR + 3 * LS);
  tft.drawString("Cloud", LEFT_COL, TR + 4 * LS);
  tft.drawString("BP", LEFT_COL, TR + 5 * LS);
  tft.drawString("Humid", LEFT_COL, TR + 6 * LS);

  // values flushed right
  tft.setTextDatum(TR_DATUM);
  tft.drawString(getCompassDirection(wx.obsWindDir) + dispWindSpeed, RIGHT_COL, TR + 2 * LS);
  tft.drawString(dispGust, RIGHT_COL, TR + 3 * LS);
  tft.drawString(dispCloud, RIGHT_COL, TR + 4 * LS);
  tft.drawString(dispSLP, RIGHT_COL, TR + 5 * LS);
  tft.drawString(dispHumid, RIGHT_COL, TR + 6 * LS);

  tft.unloadFont(); // save memory
} // secondWXframe()

/*
******************************************************
**************** ALMANAC FRAME ***********************
******************************************************
*/
void almanacFrame()
{
  // 11/24/2024

  char sunRiseTime[11] = "";
  char sunSetTime[11] = "";
  int hh, mm;

  // process sun data
  hh = ((wx.forSunRise - 60 * getOffset()) % 86400UL) / 3600;
  mm = ((wx.forSunRise - 60 * getOffset()) % 3600UL) / 60;
  snprintf(sunRiseTime, sizeof(sunRiseTime), "Rise%2d:%02da", hh, mm); // no leading zero

  hh = ((wx.forSunSet - 60 * getOffset()) % 86400UL) / 3600;
  mm = ((wx.forSunSet - 60 * getOffset()) % 3600UL) / 60;
  if (hh > 12)
  {
    hh -= 12;
  }
  snprintf(sunSetTime, sizeof(sunSetTime), "Set %2d:%02dp", hh, mm);
  float fraction = moonPhase();                 // find moon phase
  phaseName moonName = findPhaseName(fraction); // returns two Strings

  drawFramePanels(C_ALM_TOP_BG, C_ALM_BOTTOM_BG);

  int TR = 2;  // top row
  int LS = 18; // line spacing
  tft.setTextColor(C_ALM_TOP_TEXT);
  tft.setTextDatum(TC_DATUM); // centered text
  tft.setFreeFont(LargeBold);
  tft.drawString("Almanac", SCREEN_W2, TR);
  tft.drawString(dateTime("D M j"), SCREEN_W2, TR + LS); // "Sat Aug 7"
  tft.setTextColor(C_ALM_BOTTOM_TEXT);
  tft.drawString("SUN", SCREEN_W2, TR + 2 * LS);       // 41
  tft.drawString(sunRiseTime, SCREEN_W2, TR + 3 * LS); // 56
  tft.drawString(sunSetTime, SCREEN_W2, TR + 4 * LS);  // 72

  // panel for moon data
  tft.fillRect(0, 90, SCREEN_W, SCREEN_H - 90, C_ALM_MOON_BG);
  // display moon phase and name
  tft.setTextColor(C_ALM_TOP_TEXT, C_ALM_MOON_BG);
  tft.setTextDatum(TR_DATUM); // flush right
  drawMoonFace(20, SCREEN_H - 19, 15, fraction, C_ALM_MOON_FACE, C_ALM_MOON_BG);
  tft.drawString(moonName.firstWord, SCREEN_W - 6, TR + 5 * LS);  // 93
  tft.drawString(moonName.secondWord, SCREEN_W - 6, TR + 6 * LS); // 110

  tft.unloadFont();
} // almanacFrame()

/*
*****************************************************
*************** Find Moon Phase Name ****************
*****************************************************
*/
phaseName findPhaseName(float fraction)
{
  // process moon phase
  phaseName name;               // struct for two Strings
  float age = fraction * 29.53; // days in synodic month
  String first[9] = {"New", "Waxing", "First", "Waxing",
                     "Full", "Waning", "Last", "Waning", "New"};
  String second[9] = {"Moon", "Crescent", "Quarter", "Gibbous",
                      "Moon", "Gibbous", "Quarter", "Crescent", "Moon"};
  // Start age for each phase. New, First Quarter, Full, and Last Quarter
  // last 1 day, all others last 6.38 days
  float phaseStart[10] = {0, 0.5, 6.88, 8.88,
                          14.26, 15.26, 21.64, 22.64, 29.03, 30};
  int i = 0; // must be declared outside of for loop for scope
  for (i = 0; i < 9; i++)
  {
    if (age >= phaseStart[i] && age < phaseStart[i + 1])
    {
      break; // i is now the index for the phase text array
    }
  }
  name.firstWord = first[i];
  name.secondWord = second[i];
  return name;
} // findPhaseName()

/*
******************************************************
******************* Moon Phase ***********************
******************************************************
*/
float moonPhase()
{
  // adapted by K. W. Berger from
  // https://minkukel.com/en/various/calculating-moon-phase/
  // two decimal precision is sufficient as lunar month is variable
  // https://en.wikipedia.org/wiki/Lunar_phase
  unsigned long unixTime = now();                         // UTC unix time
  float lunarDays = 29.53059;                             // average days between new moons (synodic)
  unsigned int lunarSecs = lunarDays * 24 * 60 * 60;      // seconds between new moons
  unsigned int new2000 = 947182440;                       // new moon January 6, 2000 18:14 UTC
  unsigned int totalSecs = unixTime - new2000;            // seconds since January 2000 new moon
  unsigned int currentSecs = fmod(totalSecs, lunarSecs);  // seconds since most recent new moon
  float fraction = (float)currentSecs / (float)lunarSecs; // fraction of lunation 0=new, 0.25=1Q, 0.5 is full, 1=new

  return fraction;
} // moonPhase()

/*
******************************************************
****************** Draw Moon Face ********************
******************************************************
*/
void drawMoonFace(int xc, int yc, int r, float fract, int liteColor, int darkColor)
{
  // this has been simplified for the dark face having the same
  // color as the background. The code for the more general case
  // has been commented out

  int face; // type of face depending phase fraction
  float f;  // multiplier for ellipse width
            // int lcor, rghtColor; // colors for left and right lines

  if (fract >= 0 && fract <= 0.25)
  {
    face = 1; // waxing crescent, new to first quarter
    f = -4 * fract + 1;
    // lcor = darkColor;
    // rghtColor = liteColor;
  }
  else if (fract > 0.25 && fract <= 0.5)
  {
    face = 2; // waxing gibbous, first quarter to full
    f = 4 * fract - 1;
    // lcor = darkColor;
    // rghtColor = liteColor;
  }
  else if (fract > 0.5 && fract <= 0.75)
  {
    face = 3; // waning gibbous, full to last quarter
    f = -4 * fract + 3;
    // lcor = liteColor;
    // rghtColor = darkColor;
  }
  else
  {
    face = 4; // waning crescent, last quarter to new
    f = 4 * fract - 3;
    // lcor = liteColor;
    // rghtColor = darkColor;
  }

  int hh = r * r;              // height squared
  int wwe = (f * r) * (f * r); // ellipse half-width squared
  int wwm = r * r;             // moon half-width squared
  int hhwwe = hh * wwe;        // for ellipse
  int hhwwm = hh * wwm;        // for moon
  int x0e = f * r;             // half-width of ellipse starting value
  int x0m = r;                 // half width of moon disk starting value
  int dxe = 0;                 // ellipse x value
  int dxm = 0;                 // moon x value

  // calculate x values for circle and ellipse
  for (int y = 0; y <= r; y++)
  {
    // for ellipse
    int x1e = x0e - (dxe - 1);
    while (x1e > 0)
    {
      if (x1e * x1e * hh + y * y * wwe <= hhwwe)
      {
        break;
      }
      x1e--;
    }
    dxe = x0e - x1e;
    x0e = x1e;

    // for circle
    int x1m = x0m - (dxm - 1); // for circle
    while (x1m > 0)
    {
      if (x1m * x1m * hh + y * y * wwm <= hhwwm)
      {
        break;
      }
      x1m--;
    }
    dxm = x0m - x1m;
    x0m = x1m;

    // draw graphic moon
    int xl = xc - x0m; // left end of left line
    int xr = xc + x0m; // right end of right line
    int xp;            // point between left and right lines

    // draw only the lit lines assuming dark face is
    // the same as the background
    switch (face)
    {
    case 1: // waxing crescent - new to first quarter
      xp = xc + x0e;
      tft.drawFastHLine(xp, yc - y, xr - xp, liteColor);
      tft.drawFastHLine(xp, yc + y, xr - xp, liteColor);
      break;
    case 2: // waxing gibbous  - first quarter to full
      xp = xc - x0e;
      tft.drawFastHLine(xp, yc - y, xr - xp, liteColor);
      tft.drawFastHLine(xp, yc + y, xr - xp, liteColor);
      break;
    case 3: // waning gibbous  - full to last quarter
      xp = xc + x0e;
      tft.drawFastHLine(xl, yc - y, xp - xl, liteColor);
      tft.drawFastHLine(xl, yc + y, xp - xl, liteColor);
      break;
    case 4: // waning crescent - last quarter to new
      xp = xc - x0e;
      tft.drawFastHLine(xl, yc - y, xp - xl, liteColor);
      tft.drawFastHLine(xl, yc + y, xp - xl, liteColor);
      break;
    }
    // draw abutting lines above and below x-axis
    // dark face is different than background
    // if the dark color is not the same as the background
    // tft.drawFastHLine(xl, yc - y, xp - xl, lcor);
    // tft.drawFastHLine(xp, yc - y, xr - xp, rghtColor);
    // tft.drawFastHLine(xl, yc + y, xp - xl, lcor);
    // tft.drawFastHLine(xp, yc + y, xr - xp, rghtColor);
  }
} // drawMoonFace()

/*
******************************************************
************** DIGITAL CLOCK FRAME *******************
******************************************************
*/
void digitalClockFrame(bool drawFrame)
{
  // 12/21/2024 clean updates, parameterized line spacing

  int prevMinute = myTZ.minute();

  int tl = 10;                                                            // sets location of top line
  int lc = tft.width() / 2 - tft.textWidth(myTZ.dateTime("H~:i~:s")) / 2; // left text column
  int h = tft.fontHeight();                                               // text height
  int lm = 3;                                                             // pixels
  tft.setFreeFont(LargeBold);
  tft.setTextDatum(TL_DATUM);

  if (drawFrame)
  {
    tft.fillScreen(C_DIGITAL_BG);
    // frame
    tft.drawRoundRect(0, 0, SCREEN_W, SCREEN_H, 8, C_DIGITAL_FRAME_EDGE);

    // Coordinated Universal Time label & HH:MM
    tft.setTextColor(C_DIGITAL_ALT_TZ, C_DIGITAL_BG);
    tft.drawString("UTC", lc, tl);
    tft.drawString(UTC.dateTime("H~:i~:"), lc, tl + h + lm);

    // local time zone label and HH:MM
    tft.setTextColor(C_DIGITAL_LOCAL_TZ, C_DIGITAL_BG);
    tft.drawString(myTZ.getTimezoneName(), lc, tl + 2 * (h + lm));
    tft.drawString(myTZ.dateTime("H~:i~:"), lc, tl + 3 * (h + lm));

    // indoor temperature & humidity
    if (sensorType != SENSOR_NONE)
    {
      tft.setTextColor(C_DIGITAL_INDOOR);
      tft.drawString("Indoor", SCREEN_W2, tl + 80);
      readSensor();
      tft.setTextColor(C_DIGITAL_INDOOR, C_DIGITAL_BG); // print over dial
      // if (METRIC_DISPLAY) {
      //   tft.drawString(String(indoor.tempC, 1) + "C/" + String(indoor.humid, 0) + "%", SCREEN_W2, tl + 96);
      // } else {
      //   float tempF = CtoF(indoor.tempC);
      //   tft.drawString(String(tempF, 0) + "F/" + String(indoor.humid, 0) + "%", SCREEN_W2, tl + 96);
      // }
    }
  } // drawFrame

  int w = 45;

  // hour:minute update if changed during display
  if (myTZ.minute() != prevMinute)
  {
    tft.fillRect(lc, tl + 16, w, h, C_DIGITAL_BG);
    tft.setTextColor(C_DIGITAL_ALT_TZ);
    tft.drawString(UTC.dateTime("H~:i~:"), lc, tl + h + lm);
    tft.fillRect(lc, tl + 3 * (h + lm), w, h, C_DIGITAL_BG);
    tft.setTextColor(C_DIGITAL_LOCAL_TZ);
    tft.drawString(myTZ.dateTime("H~:i~:"), lc, tl + 3 * (h + lm));
    prevMinute = myTZ.minute();
  }

  // second update
  lc += tft.textWidth(myTZ.dateTime("H~:i~:"));
  w = tft.textWidth("00");
  tft.fillRect(lc, tl + h + lm, w, h, C_DIGITAL_BG);
  tft.setTextColor(C_DIGITAL_ALT_TZ);
  tft.drawString(UTC.dateTime("s"), lc, tl + h + lm);

  tft.fillRect(lc, tl + 3 * (h + lm), w, h, C_DIGITAL_BG);
  tft.setTextColor(C_DIGITAL_LOCAL_TZ);
  tft.drawString(myTZ.dateTime("s"), lc, tl + 3 * (h + lm));

  tft.unloadFont();
} // digitalClockFrame()

/*
******************************************************
*************** ANALOG CLOCK FRAME *******************
******************************************************
*/
void analogClockFrame(bool drawFrame)
{
  // 11/27/2024 - adapted for TFT_eSPI
  const int CENTER_X = SCREEN_W2;
  const int CENTER_Y = SCREEN_H2;
  //! scale dimensions from dial radius
  const int DIAL_RADIUS = SCREEN_H2;                 // outer clock dial
  const int NUMERAL_R = DIAL_RADIUS - 6;             // ring for numerals
  const int OUTER_TICK_R = NUMERAL_R - 6;            // outer ring for ticks
  const int MIN_TICK_R = OUTER_TICK_R - 3;           // inner ring for minute ticks
  const int INNER_TICK_R = MIN_TICK_R - 3;           // inner ring for hour ticks
  const int MIN_HAND = INNER_TICK_R;                 // minute hand is the longest
  const int SEC_DOT_R = 3;                           // dot radius at end of second hand
  const int SEC_HAND = INNER_TICK_R - SEC_DOT_R - 1; // dot with radius 2 on end of hand
  const int HOUR_HAND = 3 * MIN_HAND / 4;            // hour hand is 3/4 minute hand length
  const int HUB_R = 4;                               // hub radius
  int x1, x2, x3, y1, y2, y3;                        // various coordinates
  int deg;                                           // degrees
  float rad, rad1, rad2, rad3;                       // radii

  //! draw clock face first time only to speed up graphics
  if (drawFrame)
  {
    tft.setFreeFont(SmallBold);
    tft.setTextDatum(MC_DATUM); // set numeral middle center on numeral circle
    tft.fillScreen(C_ANALOG_FRAME_BG);
    tft.drawRoundRect(0, 0, SCREEN_W, SCREEN_H, 8, C_ANALOG_FRAME_EDGE);
    tft.fillCircle(CENTER_X, CENTER_Y, DIAL_RADIUS, C_ANALOG_DIAL_BG);
    tft.drawCircle(CENTER_X, CENTER_Y, DIAL_RADIUS + 1, C_ANALOG_DIAL_EDGE);
    tft.setTextColor(C_ANALOG_DIAL_NUMERALS);

    //! add minute & hour tick marks
    for (int minTick = 1; minTick < 61; minTick++)
    {
      int tickR = MIN_TICK_R;
      deg = 6 * minTick; // 60 ticks, one every 6 degrees
      if (deg % 30 == 0) // it is an hour tick
      {
        rad = DEGtoRAD(deg);
        tickR = INNER_TICK_R;
        // place numeral centered on numeral circle
        x1 = (CENTER_X + (sin(rad) * NUMERAL_R));
        y1 = (CENTER_Y - (cos(rad) * NUMERAL_R));
        tft.drawString(String(minTick / 5), x1, y1);
      }
      //! draw tick
      rad = DEGtoRAD(deg); // Convert degrees to radians
      x2 = (CENTER_X + (sin(rad) * OUTER_TICK_R));
      y2 = (CENTER_Y - (cos(rad) * OUTER_TICK_R));
      x3 = (CENTER_X + (sin(rad) * tickR));
      y3 = (CENTER_Y - (cos(rad) * tickR));
      tft.drawLine(x2, y2, x3, y3, C_ANALOG_DIAL_TICKS); // tick line
    }
    //! print timezone and AM/PM
    tft.setTextColor(C_ANALOG_TZ, C_ANALOG_FRAME_BG); // print over dial
    tft.setTextDatum(TL_DATUM);                       // flush left
    tft.drawString(getTimezoneName(), 0, 0);
    tft.setTextDatum(TR_DATUM); // flush right
    tft.drawString((myTZ.hour() > 12) ? "PM" : "AM", 126, 0);

    //! print indoor temperature & humidity
    // show only if there is a sensor and no digital clock
    if (!DIGITAL_CLOCK && sensorType != SENSOR_NONE)
    {
      readSensor();
      tft.setTextColor(C_ANALOG_INDOOR, C_ANALOG_DIAL_BG); // print over dial
      String temp = (METRIC_DISPLAY) ? String(indoor.tempC, 1) + " C" : String(CtoF(indoor.tempC), 0) + " F";
      tft.setTextDatum(TC_DATUM);
      tft.drawString(temp, CENTER_X, CENTER_Y - 30);
      tft.drawString(String(indoor.humid, 0) + "%", CENTER_X, CENTER_Y + 15);
    }
  }

  //! update hands. Process second hand, minute hand, hour hand in this order
  //? **** Process second hand ****
  deg = second() * 6;  // each second advances 6 degrees
  rad = DEGtoRAD(deg); // Convert degrees to radians
  static float oldSrad = rad;

  //! erase previous second hand
  x3 = (CENTER_X + (sin(oldSrad) * SEC_HAND));
  y3 = (CENTER_Y - (cos(oldSrad) * SEC_HAND));
  tft.drawLine(CENTER_X, CENTER_Y, x3, y3, C_ANALOG_DIAL_BG);
  tft.fillCircle(x3, y3, SEC_DOT_R, C_ANALOG_DIAL_BG);
  oldSrad = rad; // save current radians for erase next time

  //! draw new second hand
  x3 = (CENTER_X + (sin(rad) * SEC_HAND));
  y3 = (CENTER_Y - (cos(rad) * SEC_HAND));
  tft.drawLine(CENTER_X, CENTER_Y, x3, y3, C_ANALOG_SEC_HAND);
  tft.fillCircle(x3, y3, SEC_DOT_R, C_ANALOG_SEC_HAND);

  //! **** Process minute hand ****
  deg = myTZ.minute() * 6;    // each minute advances 6 degrees
  rad1 = DEGtoRAD(deg + 90);  // base of triangular hand
  rad2 = DEGtoRAD(deg - 90);  // base of triangular hand
  rad3 = DEGtoRAD(deg);       // point of hand
  static float oldMdeg = deg; // save current degrees for triangle

  //! erase previous minute hand
  if (deg != oldMdeg)
  {
    float oldMrad1 = DEGtoRAD(oldMdeg + 90);
    float oldMrad2 = DEGtoRAD(oldMdeg - 90);
    float oldMrad3 = DEGtoRAD(oldMdeg);
    x1 = (CENTER_X + (sin(oldMrad1) * HUB_R));
    y1 = (CENTER_Y - (cos(oldMrad1) * HUB_R));
    x2 = (CENTER_X + (sin(oldMrad2) * HUB_R));
    y2 = (CENTER_Y - (cos(oldMrad2) * HUB_R));
    x3 = (CENTER_X + (sin(oldMrad3) * MIN_HAND));
    y3 = (CENTER_Y - (cos(oldMrad3) * MIN_HAND));
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, C_ANALOG_DIAL_BG);
    oldMdeg = deg;
  }

  //! draw new minute hand
  x1 = (CENTER_X + (sin(rad1) * HUB_R));
  y1 = (CENTER_Y - (cos(rad1) * HUB_R));
  x2 = (CENTER_X + (sin(rad2) * HUB_R));
  y2 = (CENTER_Y - (cos(rad2) * HUB_R));
  x3 = (CENTER_X + (sin(rad3) * MIN_HAND));
  y3 = (CENTER_Y - (cos(rad3) * MIN_HAND));
  tft.fillTriangle(x1, y1, x2, y2, x3, y3, C_ANALOG_MIN_HAND);

  //! **** Process hour hand ****
  //? Must draw hour hand last as it is shortest hand
  int dialHour = (myTZ.hour() > 12) ? myTZ.hour() - 12 : myTZ.hour(); // XXXX check this. it was 13
  // 30 degree increments + adjust for minutes
  // the Swiss prefer incrementing the minute hand in minute steps
  deg = dialHour * 30 + int((myTZ.minute() / 12) * 6);
  static float oldHdeg = deg;
  rad1 = DEGtoRAD(deg + 90);
  rad2 = DEGtoRAD(deg - 90);
  rad3 = DEGtoRAD(deg);

  //? erase previous hour hand
  if (deg != oldHdeg)
  {
    float oldHrad1 = DEGtoRAD(oldHdeg + 90);
    float oldHrad2 = DEGtoRAD(oldHdeg - 90);
    float oldHrad3 = DEGtoRAD(oldHdeg);
    x1 = (CENTER_X + (sin(oldHrad1) * HUB_R));
    y1 = (CENTER_Y - (cos(oldHrad1) * HUB_R));
    x2 = (CENTER_X + (sin(oldHrad2) * HUB_R));
    y2 = (CENTER_Y - (cos(oldHrad2) * HUB_R));
    x3 = (CENTER_X + (sin(oldHrad3) * HOUR_HAND));
    y3 = (CENTER_Y - (cos(oldHrad3) * HOUR_HAND));
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, C_ANALOG_DIAL_BG);
    oldHdeg = deg; // save current degrees for hour hand
  }
  //? draw new hour hand
  x1 = (CENTER_X + (sin(rad1) * HUB_R));
  y1 = (CENTER_Y - (cos(rad1) * HUB_R));
  x2 = (CENTER_X + (sin(rad2) * HUB_R));
  y2 = (CENTER_Y - (cos(rad2) * HUB_R));
  x3 = (CENTER_X + (sin(rad3) * HOUR_HAND));
  y3 = (CENTER_Y - (cos(rad3) * HOUR_HAND));
  tft.fillTriangle(x1, y1, x2, y2, x3, y3, C_ANALOG_HOUR_HAND);

  //? draw hub with little dot in center of hub
  tft.fillCircle(CENTER_X, CENTER_Y, HUB_R, C_ANALOG_HUB);
  tft.fillCircle(CENTER_X, CENTER_Y, 1, C_ANALOG_DIAL_BG);

  tft.unloadFont();
} // analogClockFrame()

/*
******************************************************
********** Get Weather Underground API ***************
******************************************************
*/

/*
******************************************************
************** Get Current Weather *******************
******************************************************
*/
void getWXcurrent()
{
  // Documentation:
  // https://api.weather.com/v2/pws/observations/current?stationId=yourStationID&format=json&units=m&numericPrecision=decimal&apiKey=yourApiKey
  // XXX DELETE BEFORE RELEASE XXX
  // http://api.weather.com/v2/pws/observations/current?stationId=KVACENTR126&format=json&units=e&apiKey=c41eb27afef64b6b9eb27afef62b6bed
  // Uses API stream
  // By Copilot 12/15/2024
  // solves String capacity problem

  const char *getQuery =
      WX_HOST "/" WX_CURRENT
              "?stationId=" WX_STATION_ID
              "&format=" WX_FORMAT
              "&units=" WX_UNITS
              "&numericPrecision=" WX_PRECISION
              "&apiKey=" WX_KEY;

  JsonDocument filter; // filter to reduce size of JsonDocument
  filter["observations"][0]["lat"] = true;
  filter["observations"][0]["lon"] = true;
  filter["observations"][0]["neighborhood"] = true;
  filter["observations"][0]["solarRadiation"] = true;
  filter["observations"][0]["uv"] = true;
  filter["observations"][0]["winddir"] = true;
  filter["observations"][0]["humidity"] = true;
  filter["observations"][0]["metric"] = true; // extends to all items under "metric"
  // includes temp, heatIndex, dewpt, windChill, windSpeed, windGust, pressure, precip rates

  JsonDocument doc; // holds filtered json stream

  fetchDataAndParse(getQuery, filter, doc);
  serializeJsonPretty(doc, Serial);
  Serial.println();

  //! parse the JsonDocument
  JsonObject observations_0 = doc["observations"][0];
  if (observations_0["lat"] != 0)
  {
    wx.obsLat = observations_0["lat"];                           // decimal latitude
    wx.obsLon = observations_0["lon"];                           // decimal longitude
    wx.obsNeighborhood = (String)observations_0["neighborhood"]; // WU area
    wx.obsSolarRadiation = observations_0["solarRadiation"];     // W/m^2
    wx.obsUV = observations_0["uv"];                             // UV index
    wx.obsWindDir = observations_0["winddir"];                   // degrees clockwise from North
    wx.obsHumidity = observations_0["humidity"];                 // relative humidity 0 - 100%
    JsonObject observations_0_metric = observations_0["metric"]; //! all metric values
    wx.obsTemp = observations_0_metric["temp"];                  // Celsius
    wx.obsHeatIndex = observations_0_metric["heatIndex"];        // Celsius valid when temp >+ 18C
    wx.obsDewPt = observations_0_metric["dewpt"];                // Celsius
    wx.obsWindChill = observations_0_metric["windChill"];        // Celsius valid when temp < 18C
    wx.obsWindSpeed = observations_0_metric["windSpeed"];        // km/h
    wx.obsWindGust = observations_0_metric["windGust"];          // km/h
    wx.obsPressure = observations_0_metric["pressure"];          // millibars of mercury
    wx.obsPrecipRate = observations_0_metric["precipRate"];      // mm for rain, cm for snow
    wx.obsPrecipTotal = observations_0_metric["precipTotal"];    // mm/h for rain, {cm/h for snow???} from midnight
  }
  else
  {
    DEBUG_PRINTLN("No data from WU");
  }
} // getWXcurrent()

// TickTwo callback function
void updateWXcurrent()
{
  getWXcurrent();
  postToThingSpeak();
} // updateWXcurrent()

/*
******************************************************
************** Get Forecast Weather ******************
******************************************************
*/
void getWXforecast()
{
  // The PWS Google doc appears to be out of date. Use the IBM document for 5-day.
  // Obsolete Documentation: https://docs.google.com/document/d/1_Zte7-SdOjnzBttb1-Y9e0Wgl0_3tah9dSwXUyEA3-c/edit?tab=t.0
  // Use this Documentation: https://www.ibm.com/docs/en/environmental-intel-suite?topic=fa-daily-forecast-3-day-5-day-7-day-10-day
  // Query: https://api.weather.com/v3/wx/forecast/daily/5day?geocode=38.89,-77.03&format=json&units=m&language=en-US&apiKey=yourApiKey
  // Uses API stream
  // By Copilot 12/15/2024
  // solves String capacity problem

  char getQuery[200];
  snprintf(getQuery, sizeof(getQuery), "%s/%s?geocode=%f,%f&format=%s&units=%s&language=%s&apiKey=%s",
           WX_HOST, WX_FORECAST, wx.obsLat, wx.obsLon, WX_FORMAT, WX_UNITS, WX_LANGUAGE, WX_KEY);

  JsonDocument filter;
  filter["calendarDayTemperatureMax"] = true;   // The midnight to midnight daily maximum temperature.
  filter["calendarDayTemperatureMin"] = true;   // The midnight to midnight daily minimum temperature.
  filter["sunriseTimeUtc"] = true;              // Sunrise time in UNIX epoch value.
  filter["sunsetTimeUtc"] = true;               // Sunset time in UNIX epoch value.
  filter["daypart"][0]["cloudCover"] = true;    // Daytime average cloud cover expressed as a percentage.
  filter["daypart"][0]["wxPhraseLong"] = true;  // Hourly sensible weather phrase up to 32 characters.
  filter["daypart"][0]["wxPhraseShort"] = true; // Hourly sensible weather phrase up to 12 characters.

  // parse the filtered JsonDocument
  JsonDocument doc;
  fetchDataAndParse(getQuery, filter, doc);

  JsonArray calendarDayTemperatureMax = doc["calendarDayTemperatureMax"];
  wx.forTempMax = (calendarDayTemperatureMax[0]) ? calendarDayTemperatureMax[0] : calendarDayTemperatureMax[1];

  JsonArray calendarDayTemperatureMin = doc["calendarDayTemperatureMin"];
  wx.forTempMin = (calendarDayTemperatureMin[0]) ? calendarDayTemperatureMin[0] : calendarDayTemperatureMin[1];

  JsonArray sunriseTimeUtc = doc["sunriseTimeUtc"];
  wx.forSunRise = (sunriseTimeUtc[0]) ? sunriseTimeUtc[0] : sunriseTimeUtc[1]; // 1731412186

  JsonArray sunsetTimeUtc = doc["sunsetTimeUtc"];
  wx.forSunSet = (sunsetTimeUtc[0]) ? sunsetTimeUtc[0] : sunsetTimeUtc[1]; // 1731448679

  JsonObject daypart_0 = doc["daypart"][0];
  JsonArray daypart_0_cloudCover = daypart_0["cloudCover"];
  wx.forCloud = (daypart_0_cloudCover[0]) ? daypart_0_cloudCover[0] : daypart_0_cloudCover[1];

  JsonArray daypart_0_wxPhraseLong = daypart_0["wxPhraseLong"];
  wx.forPhraseLong = (daypart_0_wxPhraseLong[0]) ? (String)daypart_0_wxPhraseLong[0] : (String)daypart_0_wxPhraseLong[1];

  JsonArray daypart_0_wxPhraseShort = daypart_0["wxPhraseShort"];
  wx.forPhraseShort = (daypart_0_wxPhraseShort[0]) ? (String)daypart_0_wxPhraseShort[0] : (String)daypart_0_wxPhraseShort[1];

  // prettified print
  DEBUG_PRINTLN("Forecast filtered:");
  DEBUG_PRINT("\tTemp Max:\t");
  DEBUG_PRINTLN(wx.forTempMax);
  DEBUG_PRINT("\tTemp Min:\t");
  DEBUG_PRINTLN(wx.forTempMin);
  DEBUG_PRINT("\tCloud cover:\t");
  DEBUG_PRINTLN(wx.forCloud);
  DEBUG_PRINT("\tPhrase Long:\t");
  DEBUG_PRINTLN(wx.forPhraseLong);
  DEBUG_PRINT("\tPhrase Short:\t");
  DEBUG_PRINTLN(wx.forPhraseShort);
} // getWXforecast()

/*
******************************************************
************* fetch Data and Parse *******************
******************************************************
*/

void fetchDataAndParse(String getQuery, JsonDocument &filter, JsonDocument &doc)
{
  // HTTP request and parsing logic
  // by Copilot 12/15/2024
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  if (https.begin(client, getQuery))
  {
    int httpCode = https.GET();
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {
        DeserializationError error = deserializeJson(doc, client, DeserializationOption::Filter(filter));
        if (error)
        {
          DEBUG_PRINT("deserialization failed: ");
          DEBUG_PRINTLN(error.c_str());
        }
      }
    }
    else
    {
      DEBUG_PRINT("GET error: ");
      DEBUG_PRINTLN(https.errorToString(httpCode).c_str());
    }
    https.end();   // Ensure connection is closed after each request
    client.stop(); // Explicitly close the client connection
  }
  else
  {
    DEBUG_PRINTLN("https: can't connect");
  }
} // fetchDataAndParse()

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




/*
*******************************************************
************** getCompassDirection ********************
*******************************************************
*/
String getCompassDirection(int degrees)
{
  // translates compass degrees to 16 points with 1 to 3 letter abbreviations
  String compassPoints[] = {
      "N", "NNE", "NE", "ENE",
      "E", "ESE", "SE", "SSE",
      "S", "SSW", "SW", "WSW",
      "W", "WNW", "NW", "NNW"};

  int index = (degrees + 11.25) / 22.5; // Add 11.25 for rounding to the nearest compass point
  index = index % 16;
  return compassPoints[index];
} // getCompassDirection()

/*
*******************************************************
***************** getRainIntensity ********************
*******************************************************
*/
String getRainIntensity(float rate)
{
  // translates rainfall rate in mm/h to meteorlogical name
  // https://en.wikipedia.org/wiki/Rain
  // https://water.usgs.gov/edu/activity-howmuchrain-metric.html

  String intensity = "";
  if (rate == 0)
  {
    intensity = "Rate Nil";
  }
  else if (rate > 0.0 && rate < 2.5)
  {
    intensity = "Light";
  }
  else if (rate >= 2.5 && rate < 7.6)
  {
    intensity = "Moderate";
  }
  else if (rate >= 7.6 && rate < 50)
  {
    intensity = "Heavy";
  }
  else if (rate >= 50)
  {
    intensity = "Violent";
  }
  return intensity;
} // getRainIntensity()

/*
*******************************************************
**************** Post data to APRS-IS *****************
*******************************************************
*/
void postToAPRS(String message)
{
  // 12/20/2024
  // See http://www.aprs-is.net/Connecting.aspx
  // user mycall[-ss] pass passcode[ vers softwarename softwarevers[ UDP udpport][ servercommand]]

  WiFiClient client;
  if (client.connect(APRS_SERVER, APRS_PORT))
  {
    DEBUG_PRINTLN(F("APRS connected"));
  }
  else
  {
    DEBUG_PRINTLN(F("APRS connection failed."));
  }
  if (client.connected())
  {
    String rcvLine = client.readStringUntil('\n');
    DEBUG_PRINTLN("Rcvd: " + rcvLine);
    if (rcvLine.indexOf("full") > 0)
    {
      DEBUG_PRINTLN(F("APRS port full. Retrying."));
      client.stop(); // disconnect from port
      delay(500);
      client.connect(APRS_SERVER, APRS_PORT); // retry
    }
    // send APRS-IS logon info
    String dataString = "user " + (String)CALLSIGN + " pass " + wm_passcode;
    dataString += " vers IoT-Kits " + APRS_SOFTWARE_VERS; // softwarevers
    client.println(dataString);                           // send to APRS-IS
    DEBUG_PRINTLN("APRS logon: " + dataString);
    boolean verified = false;
    unsigned long timeBegin = millis();
    while (millis() - timeBegin < APRS_TIMEOUT)
    {
      String rcvLine = client.readStringUntil('\n');
      DEBUG_PRINTLN("Rcvd: " + rcvLine);
      if (rcvLine.indexOf("verified") != -1 && rcvLine.indexOf("unverified") == -1)
      {
        verified = true;
        break;
      }
      delay(100);
    }
    if (verified)
    {
      DEBUG_PRINTLN("APRS send: " + message);
      client.println(message);
      client.stop(); // disconnect from APRS-IS server
      DEBUG_PRINTLN("APRS done.");
    }
    else
    {
      DEBUG_PRINTLN("APRS user unverified.");
    }
  }
} // postToAPRS()

/*
*******************************************************
************** Format Weather for APRS-IS *************
*******************************************************
*/
String APRSformatWeather()
{
  /* page 65 http://www.aprs.org/doc/APRS101.PDF
     Using Complete Weather Report Format — with Lat/Long position, no Timestamp pg 75
     ________________________________________________________________
     |!|Lat|/|Lon|_|Wind Dir|/|Wind Speed|Weather Data|Software|Unit|
     |1| 8 |1| 9 |1|    3   |1|    3     |      n     |    1   |2-4 |
     |_|___|_|___|_|________|_|__________|____________|________|____|
 */
  int humid = (wx.obsHumidity == 100) ? 0 : wx.obsHumidity; // pg 74

  String dataString = CALLSIGN;
  dataString += ">APRS,TCPIP*:";
  dataString += "!" + APRSlocation(wx.obsLat, wx.obsLon);             // position in DDmm.mmN/DDDmm.mmW
  dataString += "_" + APRSpadder(wx.obsWindDir, 3);                   // degrees clockwise from north
  dataString += "/" + APRSpadder(KMtoMILES(wx.obsWindSpeed), 3);      // speed in mph
  dataString += "g" + APRSpadder(KMtoMILES(wx.obsWindGust), 3);       // speed in mph
  dataString += "t" + APRSpadder(CtoF(wx.obsTemp), 3);                // temperature in Fahrenheit
  dataString += "L" + APRSpadder(wx.obsSolarRadiation, 3);            // luminosity < 999
  dataString += "r" + APRSpadder(100 * MMtoIN(wx.obsPrecipRate), 3);  // rainfall rate in 100th of inches per hour
  dataString += "P" + APRSpadder(100 * MMtoIN(wx.obsPrecipTotal), 3); // rainfall since midnight in 100th of inches
  dataString += "h" + APRSpadder(humid, 2);                           // relative humidity in % 00 = 100%
  dataString += "b" + APRSpadder(10 * wx.obsPressure, 5);             // sea level pressure in 10ths of millibars
  dataString += APRS_DEVICE_NAME;
  // dataString += " ";
  // dataString += APRS_SOFTWARE_VERS;
  DEBUG_PRINTLN("APRS Weather: " + dataString);
  return dataString;
} // APRSformatWeather()

// ********** weather callback ********
void APRSsendWX()
{
  // callback for weather
  postToAPRS(APRSformatWeather());
}

/*
*******************************************************
************** Format Bulletin for APRS-IS ************
*******************************************************
*/
String APRSformatBulletin(String message, String ID)
{
  // format bulletin or announcement
  /* APRS101.pdf pg 83
   * Bulletin ID is a single digit from 0 to 9
   * Announcement ID is a single upper-case letter from A to Z
   * Message may not contain | or ~ or `
   *  ____________________________
   *  |:|BLN|ID|-----|:| Message |
   *  |1| 3 | 1|  5  |1| 0 to 67 |
   *  |_|___|__|_____|_|_________|
   */
  String str = (String)CALLSIGN + ">APRS,TCPIP*:" + ":BLN" + ID + "     :" + message;
  DEBUG_PRINTLN("APRS Bulletin: " + str);
  return str;
} // APRSformatBulletin()

// ********** bulletin callback ********
void APRSsendBulletin(String msg, String ID)
{
  // callback for bulletin
  postToAPRS(APRSformatBulletin(msg, ID));
}

/*
*******************************************************
****************** APRS padder ************************
*******************************************************
*/
String APRSpadder(float value, int width)
{
  // pads APRS rounded data element with leading 0s to the specified width
  int val = round(value);
  char format[6]; // Stores the string format specifier (e.g., "%04d")
  snprintf(format, sizeof(format), "%%0%dd", width);
  char paddedValue[width + 1]; // Buffer to store the formatted string
  snprintf(paddedValue, sizeof(paddedValue), format, val);
  return paddedValue;
} // APRSpadder()

/*
*******************************************************
*********** Format callsign for APRS telemetry ********
*******************************************************
*/
String APRSpadCall(String callSign)
{
  // 12/20/2024
  // pad to 9 characters including the SSID pg 12, 127
  char paddedCall[10];
  snprintf(paddedCall, sizeof(paddedCall), "%-9s", callSign.c_str());
  return String(paddedCall);
} // APRSpadCall()

/*
*******************************************************
*************** Format location for APRS **************
*******************************************************
*/
String APRSlocation(float lat, float lon)
{
  // 12/20/2024
  // convert decimal latitude & longitude to DDmm.mmN/DDDmm.mmW
  const char *latID = (lat < 0) ? "S" : "N";
  const char *lonID = (lon < 0) ? "W" : "E";
  lat = abs(lat);
  lon = abs(lon);
  int latDeg = (int)lat;              // the characteristic of lat (degrees)
  float latMin = 60 * (lat - latDeg); // the mantissa of lat (minutes)
  int lonDeg = (int)lon;
  float lonMin = 60 * (lon - lonDeg);

  char buf[19];
  snprintf(buf, sizeof(buf), "%02d%05.2f%s/%03d%05.2f%s", latDeg, latMin, latID, lonDeg, lonMin, lonID);
  return String(buf);
} // APRSlocation()

/*
*******************************************************
*************** Aphorism File Functions ***************
*******************************************************
*/

void mountFS()
{
  if (!LittleFS.begin())
  {
    DEBUG_PRINTLN("FS error");
    return;
  }
  DEBUG_PRINTLN("FS mounted");
  File file = LittleFS.open(APHORISM_FILE, "r");
  if (!file)
  {
    DEBUG_PRINTLN("FS failed to open file");
    // return -1;  // returns -1 on failure
  }
  /************** Count Lines in File *******************/
  int lineCount = 0;
  while (file.available())
  {
    file.readStringUntil('\n');
    lineCount++;
  }
  file.close();

  DEBUG_PRINT("FS: ");
  DEBUG_PRINT(lineCount);
  DEBUG_PRINTLN(" lines in " + (String)APHORISM_FILE);

  // Create and populate the array with sequential integers from 0 to lineCount
  lineArray = new int[lineCount];
  for (int i = 0; i < lineCount; i++)
  {
    lineArray[i] = i;
  }
  randomSeed(analogRead(A0)); // use noise from analog input to randomize seed
  // shuffle the indices in lineArray[]
  shuffleArray(lineArray, lineCount);
} // mountFS()

/***************** Shuffle Array **********************/
void shuffleArray(int *array, int size)
{
  // Fisher-Yates shuffle algorithm
  // random() can be seeded with randomSeed() when called
  for (int i = size - 1; i > 0; i--)
  {
    int j = random(0, i + 1);
    // Swap array[i] with the element at random index
    int temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
} // shuffleArray()

/*************** pickAphorism *******************/
String pickAphorism(const char *fileName, int *array)
{
  // 12/19/2024
  static int j = 0; // Static variable to retain value between function calls

  while (true)
  {
    File file = LittleFS.open(fileName, "r");
    if (!file)
    {
      return ""; // Return nothing on fail
    }

    String line = "";
    int targetLine = array[j];
    int currentLine = 0;

    // Search for the target line
    while (file.available())
    {
      line = file.readStringUntil('\n');
      if (currentLine == targetLine)
      {
        break;
      }
      currentLine++;
    }

    file.close();

    if (currentLine == targetLine)
    {
      j++; // Increment j for the next call
      if (array[j] == -1)
      {        // Assuming the end of the array is marked with -1
        j = 0; // Reset j if it reaches the end of the array
      }
      return line;
    }

    // If the line is not found, reset j and try again
    j = 0;
  }
} // pickAphorism()

/*
*******************************************************
******************* END OF CODE ***********************
*******************************************************
*/