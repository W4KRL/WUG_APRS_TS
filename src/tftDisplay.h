//! @file tftDisplay.h
//! @brief TFT display configuration and dimensions
//! @author Karl Berger
//! @date 2025-05-14

#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

#include <TFT_eSPI.h> // [manager] v2.4.0 TFT_eSPI

// Display orientation definitions
#define USB_LEFT 0
#define USB_DOWN 1
#define USB_RIGHT 2
#define USB_UP 3

// Display constants
extern const int FRAME_ORIENTATION;
extern int SCREEN_W;    // width in pixels
extern int SCREEN_H;    // height in pixels
extern int SCREEN_W2;   // half width
extern int SCREEN_H2;   // half height
extern int LEFT_COL;    // left margin
extern int RIGHT_COL;   // right margin
extern int HEADER_RAD;  // header corner radius
extern int HEADER_Y;    // header bottom row

// Font pointers
extern const GFXfont* SmallBold;
extern const GFXfont* LargeBold;

// TFT instance
extern TFT_eSPI tft;

void setupTFTDisplay();

#endif // TFT_DISPLAY_H
