//! @file sequentialFrames.h
//! @brief Sequential frames for the TFT display
//! @details This file contains the functions for displaying sequential frames
//!          on the TFT display. The frames include weather data, almanac data,
//!          and clock data.
//! @author Karl Berger
//! @date 2025-05-14


#ifndef SEQUENTIAL_FRAMES_H
#define SEQUENTIAL_FRAMES_H

#include <Arduino.h>
#include "tftDisplay.h"	
#include "timezone_globals.h"
#include "colors.h"
#include "indoorSensor.h"
#include "unitConversions.h"
#include "credentials.h"
#include "analogClock.h"
#include "digitalClock.h"
#include "weather.h"


void updateFrame();
void drawFramePanels(int top_background, int bottom_background);
void firstWXframe();
void secondWXframe();
void almanacFrame();

#endif // SEQUENTIAL_FRAMES_H
// End of file

