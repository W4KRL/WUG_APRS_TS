/**
 * @file tftDisplay.cpp
 * @brief TFT display implementation
 */

#include "tftDisplay.h"
#include "Roboto_Bold_12.h"
#include "Roboto_Bold_16.h"

// Display configuration
const int FRAME_ORIENTATION = USB_LEFT;
TFT_eSPI tft;

// Font resources
const GFXfont* SmallBold = &Roboto_Bold_12;
const GFXfont* LargeBold = &Roboto_Bold_16;

// Display dimensions (initialized in setup)
int SCREEN_W = 128;
int SCREEN_H = 128;
int SCREEN_W2 = SCREEN_W / 2;
int SCREEN_H2 = SCREEN_H / 2;
int LEFT_COL = 6;
int RIGHT_COL = 125;
int HEADER_RAD = 8;
int HEADER_Y = 37;

void setupTFTDisplay() {
  tft.init();
  tft.setRotation(FRAME_ORIENTATION);
  
  // Update dimensions based on actual display
  SCREEN_W = tft.width();
  SCREEN_H = tft.height();
  SCREEN_W2 = SCREEN_W / 2;
  SCREEN_H2 = SCREEN_H / 2;
}
