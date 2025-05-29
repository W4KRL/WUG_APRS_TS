/**
 * @file sequentialFrames.cpp
 * @author Karl Berger
 * @date 2025-05-29
 * @brief Sequential frames for the TFT display
 * @details This file contains the functions for displaying sequential frames
 *          on the TFT display. The frames include weather data, almanac data,
 *          and clock data.
 */

#include "sequentialFrames.h"

#include <Arduino.h>         // for Arduino functions
#include "tftDisplay.h"      // for TFT display functions
#include "colors.h"          // for colors
#include "credentials.h"     // for ANALOG_CLOCK and DIGITAL_CLOCK
#include "firstWXframe.h"    // for first weather frame
#include "secondWXframe.h"   // for second weather frame
#include "almanacFrame.h"    // for almanac frame
#include "analogClock.h"     // for analog clock frame
#include "digitalClock.h"    // for digital clock frame

/*
******************************************************
***************** FRAME UPDATE ***********************
******************************************************
*/
void updateFrame()
{
  allowHandMovement = false; // supress hand movement until analog clock is displayed
  allowNumberFlip = false;   // supress numeral change unless digital clock is displayed

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
      allowNumberFlip = true;  // enable update on secondsChanged() in loop()
    }
    break;                   //
  case 5:                    // definitely digital
    digitalClockFrame(true); // draw clock frame
    allowNumberFlip = true;  // enable update on secondsChanged() in loop()
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
