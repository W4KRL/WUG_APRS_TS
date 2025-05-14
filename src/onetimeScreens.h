//! @file onetimeScreens.h
//! @brief One-time screens
//! @details This file contains the functions for the one-time screens
//!          that are displayed on the TFT display.
//! @author Karl Berger
//! @date 2025-05-13

//! @file onetimeScreens.h
#ifndef ONETIME_SCREENS_H
#define ONETIME_SCREENS_H

#include <Arduino.h>
#include "tftDisplay.h"
#include "credentials.h"
#include <ezTime.h>
#include "weather.h"
#include "colors.h"
#include "timezone_globals.h"

void splashScreen();
void dataScreen();

#endif // ONETIME_SCREENS_H

// End of file
