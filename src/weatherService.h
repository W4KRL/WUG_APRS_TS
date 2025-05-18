/**
 * @file weatherService.h
 * @brief Weather service header
 * @details Contains functions for fetching weather data from Weather Underground and posting it to ThingSpeak.
 * @author Karl Berger
 * @date 2025-05-16
 */

#ifndef WEATHER_SERVICE_H
#define WEATHER_SERVICE_H

#include <Arduino.h>     // for struct
#include <ArduinoJson.h> // for fetchData prototype v7.2 Benoit Blanchon https://arduinojson.org/

/**
 * @brief Structure to hold weather forecast and observation data.
 *
 * Members:
 * - forPhraseLong: Long weather forecast phrase (max 32 characters).
 * - forPhraseShort: Short weather forecast phrase (max 12 characters).
 * - forTempMax: Forecasted high temperature in degrees Celsius.
 * - forTempMin: Forecasted low temperature in degrees Celsius.
 * - forCloud: Forecasted average cloud coverage (percentage).
 * - forSunRise: Sunrise time from forecast (Unix time UTC).
 * - forSunSet: Sunset time from forecast (Unix time UTC).
 * - obsLat: Observation station latitude in decimal degrees.
 * - obsLon: Observation station longitude in decimal degrees.
 * - obsNeighborhood: Station neighborhood assigned by Weather Underground.
 * - obsSolarRadiation: Observed solar radiation in W/m^2.
 * - obsUV: Observed UV index.
 * - obsHumidity: Observed relative humidity (percentage).
 * - obsDewPt: Observed dew point in degrees Celsius.
 * - obsTemp: Observed temperature in degrees Celsius.
 * - obsHeatIndex: Observed heat index in degrees Celsius (valid for >18°C).
 * - obsWindChill: Observed wind chill in degrees Celsius (valid for <18°C).
 * - obsWindDir: Observed wind direction in degrees clockwise from north.
 * - obsWindSpeed: Observed wind speed in km/h.
 * - obsWindGust: Observed wind gust speed in km/h.
 * - obsPressure: Observed sea level pressure in millibars (hPa).
 * - obsPrecipRate: Observed instantaneous precipitation rate (mm/h).
 * - obsPrecipTotal: Observed total precipitation since midnight (mm).
 */
struct weather
{
  String forPhraseLong;     // long weather forecast 32 characters max
  String forPhraseShort;    // short weather forecast 12 characters max
  int forTempMax;           // forcasted high (°C)
  int forTempMin;           // forecasted low (°C)
  int forCloud;             // forecasted average cloud coverage (%)
  unsigned long forSunRise; // sunrise (unix time UTC) from forecast
  unsigned long forSunSet;  // sunset (unix time UTC) from forecast
  float obsLat;             // station latitude in decimal degrees
  float obsLon;             // station longitude in decimal degrees
  String obsNeighborhood;   // station neighborhood assigned by Weather Underground
  float obsSolarRadiation;  // luminosity (W/m^2)
  float obsUV;              // UV index
  float obsHumidity;        // relative humidity (%)
  float obsDewPt;           // dewpoint (°C)
  float obsTemp;            // temperature (°C)
  float obsHeatIndex;       // temperature feel in Celsius valid for greater than 65°F (18°C)
  float obsWindChill;       // temperature feel valid below 65°F (18°C)
  float obsWindDir;         // wind direction degrees clockwise from north
  float obsWindSpeed;       // km/h
  float obsWindGust;        // km/h
  float obsPressure;        // sea level pressure millibars (hPa)
  float obsPrecipRate;      // instantaneous rate for an hour (mm/h)
  float obsPrecipTotal;     // total precipitation midnight to present (mm)
};

extern weather wx; // Declaration for use in other files

void getWXforecast(); ///< get forecasted weather
void getWXcurrent();  ///< get current conditions
void fetchDataAndParse(String getQuery, JsonDocument &filter, JsonDocument &doc); ///< http get from Weather Underground API
void updateWXcurrent(); ///< update current conditions and post to thingSpeak

#endif // WEATHER_SERVICE_H
// End of file
