/**
 * @file sequentialFrames.h
 * @brief Sequential frames for the TFT display
 * @details This file contains the functions for displaying sequential frames
 *          on the TFT display. The frames include weather data, almanac data,
 *          and clock data.
 * @author Karl Berger
 * @date 2025-05-16
 */


#ifndef SEQUENTIAL_FRAMES_H
#define SEQUENTIAL_FRAMES_H

void updateFrame();
void updateClocks();
void drawFramePanels(int top_background, int bottom_background);
void firstWXframe();
void secondWXframe();
void almanacFrame();

#endif // SEQUENTIAL_FRAMES_H
// End of file

