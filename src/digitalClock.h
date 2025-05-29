/**
 * @file digitalClock.h
 * @author Karl Berger
 * @date 2025-05-29
 * @brief Digital clock display functions
 * @details This file contains the functions for displaying the digital clock
 *          on the TFT display.
 */

#ifndef DIGITAL_CLOCK_H
#define DIGITAL_CLOCK_H

extern bool allowNumberFlip; // digital clock unpdate numerals

void digitalClockFrame(bool drawFrame);
void updateDigitalClock();

#endif // DIGITAL_CLOCK_H
// End of file

