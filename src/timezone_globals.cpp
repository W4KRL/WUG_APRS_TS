/**
 * @file timezone_globals.cpp
 * @author Karl W. Berger
 * @date 2025-06-02
 * @brief Timezone management implementation.
 */

#include "timezone_globals.h"

#include <Arduino.h>	 // Arduino functions
#include <ezTime.h>		 // ezTime library for timezone handling
#include "credentials.h" // for MY_TIMEZONE

Timezone myTZ;

void setTimeZone()
{
	waitForSync();
	/*
	if timezone has not been cached in EEPROM
	or user is asking for a new timezone set the timezone
	*/
	if (!myTZ.setCache(0) || myTZ.getOlson() != MY_TIMEZONE)
	{
		myTZ.setLocation(MY_TIMEZONE);
	}
	myTZ.setDefault(); // set local timezone
}

/**
 * @brief Converts a 24-hour format hour to a 12-hour format hour.
 *
 * @param hour The hour in 24-hour format (0-23).
 * @return int The hour in 12-hour format (1-12).
 */
int to12HourFormat(int hour)
{
	return (hour == 0) ? 12 : (hour > 12) ? (hour - 12)
										  : hour;
}