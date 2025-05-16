//! @file aphorismGenerator.h
//! @brief Aphorism generator header
//! @details This file contains the functions for generating aphorisms from a file.	
//!          The aphorisms are stored in a file on the LittleFS filesystem.
//! @author Karl Berger
//! @date 2025-05-16

#ifndef APHORISM_GENERATOR_H
#define APHORISM_GENERATOR_H

#include <Arduino.h> // for String

extern int *lineArray; // holds shuffled index to aphorisms
extern int lineCount; // number of aphorisms in file

void cleanupLineArray();
void mountFS();
void shuffleArray(int *array, int size);
String pickAphorism(String aphorismFile, int *lineArray);

#endif // APHORISM_GENERATOR_H
// End of file
