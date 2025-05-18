/**
 * @file timezone_globals.h
 * @brief Global variables and declarations for timezone management.
 */

#ifndef TIMEZONE_GLOBALS_H
#define TIMEZONE_GLOBALS_H

#include <Arduino.h>	

#ifdef DEBUG
#undef DEBUG  // Prevent conflicts with other libraries
#endif

#include <ezTime.h> // for Timezone

/**
 * @brief Externally declared Timezone object representing the application's current timezone.
 *
 * This global variable provides access to the configured timezone settings,
 * allowing time conversions and timezone-aware operations throughout the application.
 * 
 * @note The actual definition and initialization of `myTZ` must be provided elsewhere in the codebase.
 */
extern Timezone myTZ;

void setTimeZone(); // Function to set the timezone

#endif
// End of file
