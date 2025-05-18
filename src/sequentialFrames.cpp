/**
 * @file sequentialFrames.cpp
 * @brief Sequential frames for the TFT display
 * @details This file contains the functions for displaying sequential frames
 *          on the TFT display. The frames include weather data, almanac data,
 *          and clock data.
 * @author Karl Berger
 * @date 2025-05-16
 */

#include "sequentialFrames.h"

#include <Arduino.h>
#include "tftDisplay.h"	
#include "timezone_globals.h"
#include "colors.h"
#include "indoorSensor.h"
#include "unitConversions.h"
#include "credentials.h"
#include "analogClock.h"
#include "digitalClock.h"
#include "weatherService.h"

struct phaseName
{ //! for moon phase name
  String firstWord;
  String secondWord;
} moonName;

//! function prototypes
String getRainIntensity(float rate);
String getCompassDirection(int degrees);
float moonPhase();
phaseName findPhaseName(float fraction);
void drawMoonFace(int xc, int yc, int r, float fract, int liteColor, int darkColor);

/*
******************************************************
***************** FRAME UPDATE ***********************
******************************************************
*/
void updateFrame()
{
  allowHandMovement = false; // supress hand movement until analog clock is displayed
  allowNumberFlip = false;  // supress numeral change unless digital clock is displayed

  int maxFrames = 3; // number of display frames w/o clocks
  if (ANALOG_CLOCK)
  { // increment number of frames
    maxFrames++;
  }
  if (DIGITAL_CLOCK)
  { // increment number of frames
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
    {                           // process analog clock unless not selected
      analogClockFrame(true);   // draw clock frame
      allowHandMovement = true; // enable update on secondsChanged() in loop()
    }
    else
    {                          // otherwise process digital clock
      digitalClockFrame(true); // draw clock frame
      allowNumberFlip = true; // enable update on secondsChanged() in loop()
    } //
    break;                   //
  case 5:                    // definitely digital
    digitalClockFrame(true); // draw clock frame
    allowNumberFlip = true; // enable update on secondsChanged() in loop()
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
  String dispPrecipRate = getRainIntensity(wx.obsPrecipRate);                                                                    // rate in words
  // https://www.epa.gov/sunsafety/uv-index-scale-0
  String uvLabel = "";
  int uvText = YELLOW;       // text color
  int uvBG = 0;              // background color
  int uvi = round(wx.obsUV); // round and convert to int

  switch (uvi)
  {
  case -1: // No report or unavailable
    uvLabel = "N/A";
    uvBG = BLACK;
    uvText = WHITE;
    break;

  case 0:
  case 1:
  case 2: // Low risk
    uvLabel = "Low";
    uvBG = GREEN;
    uvText = BLACK;
    break;

  case 3:
  case 4:
  case 5: // Moderate risk
    uvLabel = "Moderate";
    uvBG = YELLOW;
    uvText = BLUE;
    break;

  case 6:
  case 7: // High risk
    uvLabel = "High";
    uvBG = OUTRAGEOUS_ORANGE;
    break;

  case 8:
  case 9:
  case 10:                   // Very High risk
    uvLabel = "Very Hi";     // to fit display
    uvBG = ALIZARIN_CRIMSON; // ORANGERED;
    break;

  default: // Extreme (uvi >= 11)
    uvLabel = "Extreme";
    uvBG = MAGENTA;
    break;
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
  unsigned long lunarSecs = lunarDays * 24 * 60 * 60;     // seconds between new moons
  unsigned long new2000 = 947182440;                      // new moon January 6, 2000 18:14 UTC
  unsigned long totalSecs = unixTime - new2000;           // seconds since January 2000 new moon
  unsigned long currentSecs = fmod(totalSecs, lunarSecs); // seconds since most recent new moon
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
    default: // handle unexpected values
      // Optionally log or handle the error
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