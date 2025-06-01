/**
 * @file sequentialFrames.cpp
 * @author Karl Berger
 * @date 2025-05-31
 * @brief Sequential frames for the TFT display
 * @details This file contains the functions for displaying sequential frames
 *          on the TFT display. The frames include weather data, almanac data,
 *          and clock data.
 */

#include "sequentialFrames.h"

#include <Arduino.h>       // for Arduino functions
#include "tftDisplay.h"    // for TFT display functions
#include "colors.h"        // for colors
#include "credentials.h"   // for ANALOG_CLOCK and DIGITAL_CLOCK
#include "firstWXframe.h"  // for first weather frame
#include "secondWXframe.h" // for second weather frame
#include "almanacFrame.h"  // for almanac frame
#include "analogClock.h"   // for analog clock frame
#include "digitalClock.h"  // for digital clock frame

//! global variables
bool isClockScreen = false; // true if current frame is a clock frame
int maxFrames = 3;          // number of display frames w/o clocks

void initializeSequentialFrames()
{
  maxFrames = 3 + (ANALOG_CLOCK ? 1 : 0) + (DIGITAL_CLOCK ? 1 : 0);
} // initializeFrames()

/*
******************************************************
***************** FRAME UPDATE ***********************
******************************************************
*/
void updateSequentialFrames()
{
  static int currentFrame = 1;        // Tracks which frame is active
  static int framePersistSeconds = 0; // Counts seconds this frame has been shown

  const int DATA_FRAME_DURATION = 5; // Duration each frame is shown in seconds

  // Increment persistence counter
  framePersistSeconds++;

  // Time to switch frames?
  if (framePersistSeconds >= DATA_FRAME_DURATION)
  {
    currentFrame = (currentFrame % maxFrames) + 1; // Cycle through frames
    framePersistSeconds = 0;                       // Reset counter
  }

  // Draw the appropriate frame
  switch (currentFrame)
  {
  case 1:
    if (framePersistSeconds == 0)
      firstWXframe();
    break;
  case 2:
    if (framePersistSeconds == 0)
      secondWXframe();
    break;
  case 3:
    if (framePersistSeconds == 0)
      almanacFrame();
    break;
  case 4:
    if (ANALOG_CLOCK)
    {
      if (framePersistSeconds == 0)
        analogClockFrame(true); // Full draw on entry
      else
        analogClockFrame(false); // Partial update
    }
    else
    {
      if (framePersistSeconds == 0)
        digitalClockFrame(true); // Full draw on entry
      else
        digitalClockFrame(false); // Partial update
    }
    break;
  case 5:
    if (framePersistSeconds == 0)
      digitalClockFrame(true); // Full draw on entry
    else
      digitalClockFrame(false); // Partial update
    break;
  default:
    // Handle unexpected frame numbers, if needed
    break;
  }
}

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
