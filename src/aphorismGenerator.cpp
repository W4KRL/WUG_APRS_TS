//! @file aphorismGenerator.cpp
//! @brief Aphorism generator implementation
//! @details This file contains the functions for generating aphorisms from a file.	
//!          The aphorisms are stored in a file on the LittleFS filesystem.
//! @author Karl Berger
//! @date 2025-05-14

#include "aphorismGenerator.h"

int* lineArray = nullptr; // holds shuffled index to aphorisms

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
  File file = LittleFS.open(APHORISM_FILE, "r");
  if (!file)
  {
    DEBUG_PRINTLN("FS failed to open file");
    // return -1;  // returns -1 on failure
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
  DEBUG_PRINTLN(" lines in " + (String)APHORISM_FILE);

  // Create and populate the array with sequential integers from 0 to lineCount
  lineArray = new int[lineCount];
  for (int i = 0; i < lineCount; i++)
  {
    lineArray[i] = i;
  }
  randomSeed(analogRead(A0)); // use noise from analog input to randomize seed
  // shuffle the indices in lineArray[]
  shuffleArray(lineArray, lineCount);
} // mountFS()

/***************** Shuffle Array **********************/
void shuffleArray(int *array, int size)
{
  // Fisher-Yates shuffle algorithm
  // random() can be seeded with randomSeed() when called
  for (int i = size - 1; i > 0; i--)
  {
    int j = random(0, i + 1);
    // Swap array[i] with the element at random index
    int temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
} // shuffleArray()

/*************** pickAphorism *******************/
String pickAphorism(String fileName, int *lineArray)
{
  // 12/19/2024
  static int j = 0; // Static variable to retain value between function calls

  while (true)
  {
    File file = LittleFS.open(fileName.c_str(), "r");
    if (!file)
    {
      return ""; // Return nothing on fail
    }

    String line = "";
    int targetLine = lineArray[j];
    int currentLine = 0;

    // Search for the target line
    while (file.available())
    {
      line = file.readStringUntil('\n');
      if (currentLine == targetLine)
      {
        break;
      }
      currentLine++;
    }

    file.close();

    if (currentLine == targetLine)
    {
      j++; // Increment j for the next call
      if (lineArray[j] == -1)
      {        // Assuming the end of the array is marked with -1
        j = 0; // Reset j if it reaches the end of the array
      }
      return line;
    }

    // If the line is not found, reset j and try again
    j = 0;
  }
} // pickAphorism()
