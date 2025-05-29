/**
 * @file digitalClock.cpp
 * @author Karl Berger
 * @date 2025-05-23
 * @brief Draws and updates the digital clock frame on the TFT display.
 * @details This function displays both UTC and local time in a digital clock format,
 *          including labels and formatted time strings. It draws the initial frame
 *          and updates only the portions of the display that change (minutes and seconds)
 *          to optimize performance. The function uses static variables to track the
 *          previous minute and second, ensuring that only the necessary parts of the
 *          display are refreshed.
 *
 * @param shouldDrawFrame If true, redraws the entire clock frame and labels.
 *                        If false, only updates the time numerals as needed.
 *
 * Dependencies:
 * - Requires global access to `tft` (TFT display object), `UTC` and `myTZ`
 *   (timezone objects), and color constants.
 * - Uses custom fonts and text formatting functions.
 *
 * Display Layout:
 * - Top: "UTC" label and UTC time (HH:MM:SS)
 * - Bottom: Local timezone label and local time (HH:MM:SS)
 * - Colors and fonts are set according to predefined constants.
 */

#include "digitalClock.h"

#include <Arduino.h>
#include "tftDisplay.h"
#include "timezone_globals.h"
#include "colors.h"
#include "indoorSensor.h"

bool allowNumberFlip = false; // digital clock unpdate numerals

/**
 * @brief Draws and updates a digital clock frame on the TFT display.
 *
 * This function displays both UTC and local times in a digital clock format,
 * including hours, minutes, and seconds. It draws a frame, labels, and time values,
 * and efficiently updates only the portions of the display that change (minutes or seconds).
 * The function uses static variables to track the previous minute and second, ensuring
 * that only the necessary parts of the display are refreshed.
 *
 * @param shouldDrawFrame If true, redraws the entire clock frame and all time values.
 *                        If false, only updates the minute and second portions as needed.
 *
 * @note Assumes global objects and constants such as `tft`, `UTC`, `myTZ`, `C_DIGITAL_BG`,
 *       `C_DIGITAL_FRAME_EDGE`, `C_DIGITAL_ALT_TZ`, `C_DIGITAL_LOCAL_TZ`, `SCREEN_W`,
 *       `SCREEN_H`, and `LargeBold` are defined elsewhere.
 */
void digitalClockFrame(bool shouldDrawFrame)
{
  static int prevMinute = -1;
  static int prevSecond = -1;

  // Time format Strings
  const String hhmmFmt = "H~:i~:";        // HH:MM:
  const String hhmmssFmt = hhmmFmt + "s"; // HH:MM:SS

  // Set font prior to using font metrics
  tft.setFreeFont(LargeBold);
  tft.setTextDatum(TL_DATUM);

  const int colLeft = tft.width() / 2 - tft.textWidth("88:88:88") / 2; // Use widest possible string
  const int ssWidth = tft.textWidth("88");                             // Widest possible seconds width
  const int ssCol = colLeft + tft.textWidth(UTC.dateTime(hhmmFmt));    // HH:MM width
  const int textHeight = tft.fontHeight();                             // Initialization permitted within function
  const int lineSpacing = 3;                                           // Pixels between lines
  int row[4];                                                          // number of lines
  row[0] = 10;                                                         // top row
  for (int i = 1; i < 4; i++)
  {
    row[i] = row[i - 1] + textHeight + lineSpacing;
  }

  if (shouldDrawFrame)
  {
    prevMinute = myTZ.minute();
    prevSecond = myTZ.second();

    tft.fillScreen(C_DIGITAL_BG);
    tft.drawRoundRect(0, 0, SCREEN_W, SCREEN_H, 8, C_DIGITAL_FRAME_EDGE);

    // UTC
    tft.setTextColor(C_DIGITAL_ALT_TZ, C_DIGITAL_BG);
    tft.drawString("UTC", colLeft, row[0]);                   // UTC label
    tft.drawString(UTC.dateTime(hhmmssFmt), colLeft, row[1]); // UTC time

    // Local
    tft.setTextColor(C_DIGITAL_LOCAL_TZ, C_DIGITAL_BG);
    tft.drawString(myTZ.getTimezoneName(), colLeft, row[2]);   // local timezone
    tft.drawString(myTZ.dateTime(hhmmssFmt), colLeft, row[3]); // local time
  }

  // Minute update check
  if (myTZ.minute() != prevMinute)
  {
    tft.fillRect(colLeft, row[1], tft.textWidth("88:88"), textHeight, C_DIGITAL_BG);
    tft.setTextColor(C_DIGITAL_ALT_TZ);
    tft.drawString(UTC.dateTime(hhmmFmt), colLeft, row[1]);

    tft.fillRect(colLeft, row[3], tft.textWidth("88:88"), textHeight, C_DIGITAL_BG);
    tft.setTextColor(C_DIGITAL_LOCAL_TZ);
    tft.drawString(myTZ.dateTime(hhmmFmt), colLeft, row[3]);

    prevMinute = myTZ.minute();
  }

  // Second update check
  if (myTZ.second() != prevSecond)
  {
    // UTC Seconds
    tft.fillRect(ssCol, row[1], ssWidth, textHeight, C_DIGITAL_BG);
    tft.setTextColor(C_DIGITAL_ALT_TZ);
    tft.drawString(UTC.dateTime("s"), ssCol, row[1]);

    // Local Seconds
    tft.fillRect(ssCol, row[3], ssWidth, textHeight, C_DIGITAL_BG);
    tft.setTextColor(C_DIGITAL_LOCAL_TZ);
    tft.drawString(myTZ.dateTime("s"), ssCol, row[3]);

    prevSecond = myTZ.second();
  }

  tft.unloadFont();
}

/**
 * @brief Updates the digital clock display if the seconds value has changed.
 *
 * This function checks if the current second (retrieved from myTZ) is different
 * from the previously stored second. If the second has changed, it updates the
 * stored value and, if the digital clock display is enabled (allowNumberFlip is true),
 * it triggers a refresh of the digital clock display without redrawing the frame.
 */
void updateDigitalClock()
{
  static int oldsec = -1;
  if (myTZ.second() != oldsec)
  {
    oldsec = myTZ.second();
    if (allowNumberFlip)
    {                           // user has selected the digital clock
      digitalClockFrame(false); // do not redraw frame
    }
  }
} // updateDigitalClock()