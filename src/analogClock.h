//! @file analogClock.h
//! @brief Analog clock display functions
//! @details This file contains the functions for displaying the analog clock	
//!          on the TFT display.
//! @author Karl Berger
//! @date 2025-05-14

#ifndef ANALOG_CLOCK_H
#define ANALOG_CLOCK_H

#include <Arduino.h>
#include "tftDisplay.h"	
#include "timezone_globals.h"
#include "colors.h"
#include "indoorSensor.h"
#include "unitConversions.h"
#include "credentials.h"

extern bool moveHands;   // analog clock update hand

void analogClockFrame(bool drawFrame);

#endif // ANALOG_CLOCK_H
// End of file
