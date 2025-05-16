//! @file analogClock.h
//! @brief Analog clock display functions
//! @details This file contains the functions for displaying the analog clock	
//!          on the TFT display.
//! @author Karl Berger
//! @date 2025-05-14

#ifndef ANALOG_CLOCK_H
#define ANALOG_CLOCK_H

extern bool allowHandMovement;   // analog clock update hand

void analogClockFrame(bool drawFrame);

#endif // ANALOG_CLOCK_H
// End of file
