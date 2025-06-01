/**
 * @file analogClock.cpp
 * @author Karl Berger
 * @date 2025-06-01
 * @brief Analog clock display functions.
 * @details  This file contains the implementation of functions to render an analog clock
 * on a TFT display, including drawing the clock face, numerals, tick marks,
 * hands (second, minute, hour), and additional information such as timezone,
 * AM/PM indicator, and indoor sensor data (temperature and humidity).
 */

void analogClockFrame(bool drawFrame);

#include "analogClock.h"

#include <Arduino.h>          // Arduino functions
#include "tftDisplay.h"       // for TFT display functions
#include <Adafruit_GFX.h>     // for fillTriangle and other graphics functions
#include "timezone_globals.h" // for getTimezoneName(), to12HourFormat()
#include "colors.h"           // for colors

#define C_ANALOG_PM_NUMERALS RED // Define constant for PM numeral color
#include "indoorSensor.h"        // for indoor sensor data
#include "unitConversions.h"     // for temperature conversions
#include "credentials.h"         // for METRIC_DISPLAY

void analogClockFrame(bool drawFrame)
{
  const int CENTER_X = SCREEN_W2;
  const int CENTER_Y = SCREEN_H2;
  //! scale dimensions from dial radius
  const int DIAL_RADIUS = SCREEN_H2;                 // outer clock dial
  const int AM_NUMERAL_R = DIAL_RADIUS - 6;          // ring for AM numerals
  const int PM_NUMERAL_R = DIAL_RADIUS - 27;         // ring for PM numerals
  const int OUTER_TICK_R = AM_NUMERAL_R - 6;         // outer ring for ticks
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

    //! add minute & hour tick marks
    for (int minTick = 1; minTick < 61; minTick++)
    {
      int tickR = MIN_TICK_R;
      deg = 6 * minTick; // 60 ticks, one every 6 degrees
      if (deg % 30 == 0) // it is an hour tick
      {
        rad = DEGtoRAD(deg);
        tickR = INNER_TICK_R;
        // place numerals centered on AM numeral circle
        x1 = (CENTER_X + (sin(rad) * AM_NUMERAL_R));
        y1 = (CENTER_Y - (cos(rad) * AM_NUMERAL_R));
        tft.setTextColor(C_ANALOG_AM_NUMERALS);
        tft.drawString(String(minTick / 5), x1, y1);

        // place PM numerals centered on PM numeral circle
        tft.setTextColor(C_ANALOG_PM_NUMERALS);
        x1 = (CENTER_X + (sin(rad) * PM_NUMERAL_R));
        y1 = (CENTER_Y - (cos(rad) * PM_NUMERAL_R));
        tft.drawString(String((minTick) / 5 + 12), x1, y1); // add 12 for PM numerals
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

    if (!DIGITAL_CLOCK && indoorSensor) // if indoor sensor exists & no digital clock
    {
      readSensor();
      tft.setTextColor(C_ANALOG_INDOOR, C_ANALOG_DIAL_BG); // print over dial
      String temp = (METRIC_DISPLAY) ? String(indoor.tempC, 1) + " C" : String(CtoF(indoor.tempC), 0) + " F";
      tft.setTextDatum(TC_DATUM);
      tft.drawString(temp, CENTER_X, CENTER_Y - 30);
      tft.drawString(String(indoor.humid, 0) + "%", CENTER_X, CENTER_Y + 15);
    }
  } // if(drawFrame)

  //! update hands. Process second hand, minute hand, hour hand in this order
  //? **** Process second hand ****
  deg = myTZ.second() * 6; // each second advances 6 degrees
  rad = DEGtoRAD(deg);     // Convert degrees to radians
  static float oldSrad = rad;

  //! erase previous second hand
  x3 = (CENTER_X + (sin(oldSrad) * SEC_HAND));
  y3 = (CENTER_Y - (cos(oldSrad) * SEC_HAND));
  tft.drawLine(CENTER_X, CENTER_Y, x3, y3, C_ANALOG_DIAL_BG); // erase second hand line
  tft.fillCircle(x3, y3, SEC_DOT_R, C_ANALOG_DIAL_BG);        // erase second hand pointer
  oldSrad = rad;                                              // save current radians for erase next time

  //! draw new second hand
  x3 = (CENTER_X + (sin(rad) * SEC_HAND));
  y3 = (CENTER_Y - (cos(rad) * SEC_HAND));
  tft.drawLine(CENTER_X, CENTER_Y, x3, y3, C_ANALOG_SEC_HAND); // draw second hand line
  tft.fillCircle(x3, y3, SEC_DOT_R, C_ANALOG_SEC_HAND);        // draw second hand pointer

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
  int dialHour = to12HourFormat(myTZ.hour()); // convert to 12-hour format
  // 30 degree increments + adjust for minutes
  // Increment the minute hand in precise minute steps for better accuracy
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

} // analogClockFrame()

// End of file