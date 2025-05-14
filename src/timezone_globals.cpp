#include <Arduino.h>
#include <ezTime.h>
#include "timezone_globals.h"

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
