//! @file onetimeScreens.h
//! @brief One-time screens
//! @details This file contains the functions for the one-time screens
//!          that are displayed on the TFT display.
//! @author Karl Berger
//! @date 2025-05-13

#ifndef ONETIME_SCREENS_H
#define ONETIME_SCREENS_H

#include <Arduino.h>	 // [builtin] PlatformIO
#include <TFT_eSPI.h>	 // [manager] v2.4.0 TFT_eSPI
#include "credentials.h" // Wi-Fi and weather station credentials
#include <ezTime.h>
#include <TFT_eSPI.h>		  // Setup 7 with CS = D0, GREENTAB
extern TFT_eSPI tft;		  // instantiate fast display driver
#include "weather.h"		  // weather station object
#include "colors.h"			  // custom frame colors
#include "timezone_globals.h" // timezone object

//! Display orientation definitions
#define USB_LEFT 0;						// Definitions for the
#define USB_DOWN 1;						// location of USB port
#define USB_RIGHT 2;					// as viewed from
#define USB_UP 3;						// the front of the display
const int FRAME_ORIENTATION = USB_LEFT; // for IoT Kits case
//! *********** WEMOS TFT 1.4 SETTINGS ******************
const int SCREEN_W = tft.width();	// width in pixels = 128
const int SCREEN_H = tft.height();	// height in pixels = 128
const int SCREEN_W2 = SCREEN_W / 2; // half width
const int SCREEN_H2 = SCREEN_H / 2; // half height
const int LEFT_COL = 6;				// left margin
const int RIGHT_COL = 125;			// right margin
const int HEADER_RAD = 8;			// radius of frame header corners
const int HEADER_Y = 37;			// bottom row of frame header

// #include "Roboto_Bold_12.h"					// small font
// #include "Roboto_Bold_16.h"					// large font
extern const GFXfont *SmallBold; // pointer to small font file
extern const GFXfont *LargeBold; // pointer to large font file

void splashScreen(); // splash screen
void dataScreen();	 // show configured parameters screen

#endif // ONETIME_SCREENS_H
// End of file
