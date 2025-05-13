#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h> // [builtin] PlatformIO

struct weather {
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

#endif // WEATHER_H
