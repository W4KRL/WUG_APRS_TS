#include "digitalClock.h"

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