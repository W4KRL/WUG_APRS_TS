/**
 * @file aphorismGenerator.h
 * @brief Aphorism generator header.
 * @details This file contains the declarations for functions used to generate aphorisms from a file.
 *          The aphorisms are stored in a file on the LittleFS filesystem.
 * @author Karl Berger
 * @date 2025-05-16
 *
 * @param lineCount
 *      Number of aphorisms in the file.
 *      Cleans up and frees memory used by the line array.
 * @fn void mountFS()
 *      Mounts the LittleFS filesystem.
 * @fn void shuffleArray(int *array, int size)
 *      Shuffles the given array of indices.
 *      @param array Pointer to the array to shuffle.
 *      @param size Size of the array.
 * @fn String pickAphorism(String aphorismFile, int *lineArray)
 *      Picks an aphorism from the file using the shuffled line array.
 */

#ifndef APHORISM_GENERATOR_H
#define APHORISM_GENERATOR_H

#include <Arduino.h>

extern int *lineArray;
extern int lineCount;

/**
 * @brief Cleans up the dynamically allocated line array.
 * @details
 * Deletes the memory allocated for the shuffled index array of aphorisms
 * and resets the pointer to nullptr.
 */
void cleanupLineArray();

/**
 * @brief Mounts the LittleFS filesystem and initializes the aphorism index array.
 * @details
 * - Mounts the filesystem.
 * - Counts the number of lines (aphorisms) in the aphorism file.
 * - Allocates and shuffles an array of indices for random aphorism selection.
 * @param lineCount
 *      Number of aphorisms in the file.
 */
void mountFS();

/**
 * @brief Shuffles an integer array using the Fisher-Yates algorithm.
 * @param array Pointer to the integer array to shuffle.
 * @param size Number of elements in the array.
 * @details
 * Randomly permutes the elements of the array to ensure uniform distribution.
 */
void shuffleArray(int *array, int size);

/**
 * @brief Picks a random aphorism from the specified file.
 * @param fileName Name of the file containing aphorisms.
 * @param lineArray Pointer to the shuffled array of line indices.
 *      Holds shuffled indices to aphorisms.
 * @return String containing the selected aphorism, or an empty string on failure.
 * @details
 * - Uses a static index to iterate through the shuffled line array.
 * - Opens the file and retrieves the aphorism at the target line.
 * - Resets the index if the end of the array is reached.
 * - Limits the number of attempts to prevent infinite loops.
 * - Limits the number of attempts to prevent infinite loops by using a predefined maximum attempt count.
String pickAphorism(String fileName, int *lineArray);
 */
String pickAphorism(String aphorismFile, int *lineArray);

#endif // APHORISM_GENERATOR_H
// End of file
