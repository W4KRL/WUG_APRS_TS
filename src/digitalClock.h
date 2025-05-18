/**
 * @file digitalClock.h
 * @brief Digital clock display functions
 * @details This file contains the functions for displaying the digital clock
 *          on the TFT display.
 * @author Karl Berger
 * @date 2025-05-14
 */

#ifndef DIGITAL_CLOCK_H
#define DIGITAL_CLOCK_H

extern bool allowNumberFlip; // digital clock unpdate numerals

void digitalClockFrame(bool drawFrame);

#endif // DIGITAL_CLOCK_H
// End of file

