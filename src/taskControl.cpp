#include "taskControl.h" // task control functions

#include <TickTwo.h>		   // v4.4.0 Stefan Staub https://github.com/sstaub/TickTwo
#include "credentials.h"	   // for WX_CURRENT_INTERVAL, WX_FORECAST_INTERVAL, etc.
#include "weatherService.h"	   // weather data from Weather Underground API
#include "thingSpeakService.h" // ThingSpeak posting
#include "sequentialFrames.h"  // sequential weather and almanac frames
#include "aprsService.h"	   // APRS functions

//! Scheduled tasks
TickTwo tmrGetWXcurrent(getWXcurrent, WX_CURRENT_INTERVAL * 60 * 1000, 0, MILLIS);
TickTwo tmrGetWXforecast(getWXforecast, WX_FORECAST_INTERVAL * 60 * 1000, 0, MILLIS);
TickTwo tmrPostWXtoAPRS(postWXtoAPRS, WX_APRS_INTERVAL * 60 * 1000, 0, MILLIS);
TickTwo tmrPostWXtoThingspeak(postWXtoThingspeak, TS_POST_INTERVAL * 60 * 1000, 0, MILLIS);
TickTwo tmrUpdateFrame(updateSequentialFrames, SCREEN_DURATION * 1000, 0, MILLIS);
TickTwo tmrSecondTick(updateClock, 1000, 0, MILLIS);

void startTasks()
{
	tmrGetWXcurrent.start();	   // timer for current weather
	tmrGetWXforecast.start();	   // timer for forecasted weather
	tmrPostWXtoThingspeak.start(); // timer for posting to ThingSpeak
	tmrPostWXtoAPRS.start();	   // timer for posting weather to APRS
	// tmrSecondTick.start();		   // timer for second tick clock updates
	tmrUpdateFrame.start();		   // timer for sequential frames
} // startTasks()

void updateTasks()
{
	// Update the TickTwo timers
	tmrGetWXcurrent.update();		// get current weather
	tmrPostWXtoThingspeak.update(); // post selected current weather to ThingSpeak
	tmrPostWXtoAPRS.update();		// post selected weather data to APRS
	tmrGetWXforecast.update();		// get forecasted weather for display
	tmrUpdateFrame.update();		// update sequential frames
	tmrSecondTick.update();			// update seconds for clock & frame displays
} // updateTasks()