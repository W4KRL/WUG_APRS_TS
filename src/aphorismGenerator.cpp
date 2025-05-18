/**
 * @file aphorismGenerator.cpp
 * @brief Implementation of aphorism generator functions.
 * @details
 * This file provides functions to generate and retrieve aphorisms stored in a file
 * on the LittleFS filesystem. It includes logic for mounting the filesystem, counting
 * lines in the aphorism file, shuffling the order of aphorisms, and selecting a random
 * aphorism for use.
 * 
 * - Uses LittleFS for file storage.
 * - Shuffles aphorism indices for random selection.
 * - Provides cleanup for dynamically allocated memory.
 * 
 * @author Karl Berger
 * @date 2025-05-16
 */

#include "aphorismGenerator.h"

#include <LittleFS.h>    // [builtin]
#include "credentials.h" // Wi-Fi and weather station credentials
#include "wug_debug.h"       // for debug print

int *lineArray = nullptr; // holds shuffled index to aphorisms

void cleanupLineArray()
{
  delete[] lineArray;
  lineArray = nullptr;
}

int lineArraySize = 0; // tracks the size of the lineArray

/*
*******************************************************
*************** Aphorism File Functions ***************
*******************************************************
*/

void mountFS()
{
  if (!LittleFS.begin())
  {
    DEBUG_PRINTLN("FS error");
    return;
  }
  DEBUG_PRINTLN("FS mounted");

  // Ensure previous memory is cleaned up before allocating new memory
  // cleanupLineArray();
  File file = LittleFS.open(APHORISM_FILE, "r");
  if (!file)
  {
    DEBUG_PRINTLN("FS failed to open file");
  }
  /************** Count Lines in File *******************/
  int lineCount = 0;
  while (file.available())
  {
    file.readStringUntil('\n');
    lineCount++;
  }
  file.close();

  DEBUG_PRINT("FS: ");
  DEBUG_PRINT(lineCount);
  DEBUG_PRINTLN(" lines in " + APHORISM_FILE);

  // Create and shuffle the array directly during initialization
  lineArraySize = lineCount;
  lineArray = new int[lineArraySize];
  randomSeed(analogRead(0)); // Seed with analog noise from pin 0 for better entropy
  for (int i = 0; i < lineCount; i++)
  {
    int j = random(0, i + 1);                   // Generate a random index
    lineArray[i] = (j == i) ? i : lineArray[j]; // Place the new index or swap with an existing one
    lineArray[j] = i;
  }
} // mountFS()

/***************** Shuffle Array **********************/
void shuffleArray(int *array, int size)
{
  // Fisher-Yates shuffle algorithm
  // This algorithm iterates through the array from the last element to the second.
  // For each element, it selects a random index (j) between 0 and the current index (i).
  // The elements at indices i and j are then swapped, ensuring a uniform shuffle.
  // random() is used to generate the random index, and it can be seeded with randomSeed() for better randomness.

  for (int i = size - 1; i > 0; i--)
  {
    int j = random(0, i + 1); // Generate a random index between 0 and i (inclusive)
    // Swap array[i] with the element at random index j
    int temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
} // shuffleArray()

/*************** pickAphorism *******************/
String pickAphorism(String fileName, int *lineArray)
{
  static int j = 0; // Static variable to retain value between function calls

  if (lineArray == nullptr)
  {
    return ""; // Return an empty string if lineArray is not initialized
  }

  for (int attempt = 0; attempt < 10; attempt++)
  { // Limit attempts to prevent infinite loop
    File file = LittleFS.open(fileName.c_str(), "r");
    if (!file)
    {
      DEBUG_PRINTLN("FS failed to open file");
      return ""; // Return empty string on failure
    }

    String aphorism = "";
    int targetLine = lineArray[j];
    int currentLine = 0;

    // Search for the target line
    while (file.available())
    {
      String line = file.readStringUntil('\n');
      if (currentLine == targetLine)
      {
        aphorism = line;
        break;
      }
      currentLine++;
    }

    file.close(); // Ensure file is closed

    if (currentLine == targetLine)
    {
      j++; // Increment j for the next call
      if (lineArray[j] == -1)
      {        // Assuming the end of the array is marked with -1
        j = 0; // Reset j if it reaches the end of the array
      }
      return aphorism; // Return found aphorism
    }
  }

  j = 0;     // Reset j if no valid aphorism is found
  return ""; // Return empty string if unsuccessful
}
