/**
 * @file analogClock.h
 * @brief Analog clock display functions.
 * @details This file contains the functions for displaying the analog clock
 *          on the TFT display.
 * @author Karl Berger
 * @date 2025-05-14
 */

/**
 * @param allowHandMovement
 * @brief Controls whether the analog clock hands are allowed to move.
 */

/**
 * @brief Draws or clears the analog clock frame.
 * @details This function is responsible for rendering the frame of an analog clock
 *          on the display. If the parameter @p drawFrame is true, the frame will be drawn;
 *          if false, only the time will be drawn to speed rendering.
 * @param drawFrame Set to true to draw the clock frame on first call, false for subsequent calls.
 */

#ifndef ANALOG_CLOCK_H
#define ANALOG_CLOCK_H

extern bool allowHandMovement; ///< control analog clock update hand

/**
 * @brief Draws or clears the analog clock frame.
 *
 * This function is responsible for rendering the frame of an analog clock
 * on the display. If the parameter drawFrame is true, the frame will be drawn;
 * if false, only the time will be drawn to speed rendering.
 *
 * @param drawFrame Set to true to draw the clock frame on first call, false for subsequent calls.
 */
void analogClockFrame(bool drawFrame);

#endif // ANALOG_CLOCK_H
// End of file
