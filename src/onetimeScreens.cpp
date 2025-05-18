/**
 * @file onetimeScreens.cpp
 * @brief Functions to display one-time screens on the TFT display
 * @date 2025-05-18
 */

#include "onetimeScreens.h"

#include <Arduino.h>
#include "tftDisplay.h"
#include "credentials.h"
#include "weatherService.h"
#include "colors.h"
#include "timezone_globals.h"

/*
******************************************************
******************* Splash Screen ********************
******************************************************
*/
void splashScreen()
{
  // 11/24/2024
  tft.setFreeFont(LargeBold);
  tft.fillScreen(BLUE);
  tft.setTextColor(YELLOW);
  int tl = 19;
  tft.setTextDatum(TC_DATUM); // font top, centered
  tft.drawString("D1S-WUG", SCREEN_W2, tl);
  tft.drawString("Display", SCREEN_W2, tl + 20);
  tft.drawString("by", SCREEN_W2, tl + 40);
  tft.drawString("IoT Kits", SCREEN_W2, tl + 60);
  tft.drawString("v" + FW_VERSION + "-M", SCREEN_W2, tl + 80);
  for (int i = 0; i < 4; i++)
  {
    tft.drawRoundRect(12 - 3 * i, 12 - 3 * i, SCREEN_W - 12, SCREEN_H - 12, 8, YELLOW);
  }
  tft.unloadFont();
} // splashScreen()

/*
******************************************************
****************** Data Screen ***********************
******************************************************
*/
void dataScreen()
{
  // 12/15/2024
  tft.fillScreen(BLACK); // clear screen
  String units = (METRIC_DISPLAY) ? "Metric" : "Imperial";
  String clock;
  if (ANALOG_CLOCK && DIGITAL_CLOCK)
  {
    clock = "Analog&Digital";
  }
  else if (ANALOG_CLOCK && !DIGITAL_CLOCK)
  {
    clock = "Analog";
  }
  else if (!ANALOG_CLOCK && DIGITAL_CLOCK)
  {
    clock = "Digital";
  }
  else
  {
    clock = "No";
  }
  clock += " Clock";

  int TR = 0;  // top row
  int LS = 14; // line spacing
  tft.setFreeFont(SmallBold);
  tft.setTextDatum(TC_DATUM); // center text
  tft.setTextColor(YELLOW);
  tft.drawString("Location:", SCREEN_W2, TR);
  tft.drawString(String(wx.obsLat, 2) + "/" + String(wx.obsLon, 2), SCREEN_W2, TR + LS);
  tft.drawString(wx.obsNeighborhood, SCREEN_W2, TR + 2 * LS);
  tft.setTextColor(GREEN);
  tft.drawString("Time Zone:" + getTimezoneName(), SCREEN_W2, TR + 3 * LS);
  tft.drawString(MY_TIMEZONE, SCREEN_W2, TR + 4 * LS); // Olsen timezone
  tft.setTextColor(WHITE);
  tft.drawString("Units & Duration:", SCREEN_W2, TR + 5 * LS);
  tft.drawString(units + " " + SCREEN_DURATION + " seconds", SCREEN_W2, TR + 6 * LS);
  tft.drawString(clock, SCREEN_W2, TR + 7 * LS);
  tft.setTextColor(RED);
  tft.drawString(APHORISM_FILE, SCREEN_W2, TR + 8 * LS);

  tft.unloadFont();
} // dataScreen()