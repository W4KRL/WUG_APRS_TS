/**
 * @file analogClock.h
 * @brief Analog clock display header.
 * @details  This file contains the header for functions to render an analog clock
 * on a TFT display, including drawing the clock face, numerals, tick marks,
 * hands (second, minute, hour), and additional information such as timezone,
 * AM/PM indicator, and indoor sensor data (temperature and humidity).
 *
 * @author Karl Berger
 * @date 2025-05-14
 */

#ifndef ANALOG_CLOCK_H
#define ANALOG_CLOCK_H

/** 
 * @param allowHandMovement
 * @brief Controls whether the analog clock hands are allowed to move.
 */
extern bool allowHandMovement; ///< control analog clock update hand

/**
 * @brief Draws and updates the analog clock frame.
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
