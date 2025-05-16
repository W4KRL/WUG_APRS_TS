//! @file timezone_globals.cpp
//!	2025-05-16

#include "timezone_globals.h"

#include "credentials.h"

Timezone myTZ; // This is the single definition

void setTimeZone()
{
	waitForSync(); // sync ezTime with a net time server
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
