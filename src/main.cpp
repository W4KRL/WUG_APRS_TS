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

//! For APRS bulletins
#include <LittleFS.h> // [builtin]

/*
******************************************************
************* FUNCTION PROTOTYPES ********************
******************************************************
*/

void updateWXcurrent();                                // update weather data
String APRSformatBulletin(String message, String ID);  // format APRS bulletin or announcement
String APRSlocation(float lat, float lon);             // format APRS location
String APRSpadder(float value, int width);             // format APRS data
void APRSsendBulletin(String msg, String ID);          // send APRS bulletin or announcement
void APRSsendWX();                                     // send APRS weather data
void postToAPRS(String message);                       // post data to APRS
void mountFS();                                        // mount LittleFS
void shuffleArray(int *array, int size);               // shuffle array
String pickAphorism(const char *fileName, int *array); // pick an aphorism

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
  waitForSync();                   // sync ezTime with a net time server

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
  char paddedCall[10]; // 9 chars + null terminator
  // print at most 9 characters
  snprintf(paddedCall, sizeof(paddedCall), "%-9.9s", callSign.c_str());
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
  lat = constrain(lat, -90, 90);
  lon = constrain(lon, -180, 180);

  const char *latID = (lat < 0) ? "S" : "N";
  const char *lonID = (lon < 0) ? "W" : "E";
  lat = abs(lat);
  lon = abs(lon);
  uint8_t latDeg = (int)lat;              // the characteristic of lat (degrees)
  float latMin = 60 * (lat - latDeg); // the mantissa of lat (minutes)
  uint8_t lonDeg = (int)lon;
  float lonMin = 60 * (lon - lonDeg);

  char buf[20]; // Increased buffer size to safely accommodate the formatted string
  snprintf(buf, sizeof(buf), "%02u%05.2f%.1s/%03u%05.2f%.1s",
           latDeg, latMin, latID, lonDeg, lonMin, lonID);
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