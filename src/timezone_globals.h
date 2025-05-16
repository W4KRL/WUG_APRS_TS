//! @file timezone_globals.h
//! @brief Global variables for timezone

#ifndef TIMEZONE_GLOBALS_H
#define TIMEZONE_GLOBALS_H

#include <Arduino.h>	

#include <ezTime.h> // for Timezone

extern Timezone myTZ;

void setTimeZone(); // Function to set the timezone

#endif
// End of file
