//! @file timezone_globals.h
//! @brief Global variables for timezone

#ifndef TIMEZONE_GLOBALS_H
#define TIMEZONE_GLOBALS_H

#include <Arduino.h>
#include <ezTime.h>    // This defines the Timezone type

// Do NOT include <ezTime.h> here!
// Only declare the extern variable.
extern Timezone myTZ;

#endif
// End of file
