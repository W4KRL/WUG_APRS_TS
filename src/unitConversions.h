//!	@file unitConversions.h
//!	@brief Unit conversion functions		
//!	@details This file contains the functions for unit conversions
//!	@author Karl Berger
//!	@date 2025-05-13

#ifndef UNIT_CONVERSION_H
#define UNIT_CONVERSION_H
#include <Arduino.h> // [builtin] PlatformIO

float CtoF(float tempC) // convert celsius to fahrenheit
{
  return 1.8 * tempC + 32.0;
} // CtoF()

float MStoMPH(float ms) // convert meters per second to miles per hour
{
  return 2.23694 * ms;
} // MStoMPH()

float MMtoIN(float mm) // convert millimeters to inches
{
  return mm * 0.0393701;
} // MMtoIN()

float CMtoIN(float cm)
{ // convert centimeters to inches
  return 0.39370 * cm;
} // CMtoIN()

float KMtoMILES(float km) // convert kilometers to miles or km/h to mph
{
  return km * 0.621371;
} // KMtoMILES()

float DEGtoRAD(float deg) // convert degrees to radians
{
  return 0.0174532925 * deg;
} // DEGtoRAD()

float HPAtoINHG(float hpa) // convert hectoPascal (millibar) to inches of mercury
{
  return 0.0295301 * hpa;
} // HPAtoINHG()

float MtoFT(float meters)
{ // convert meters to feet
  return 3.2808 * meters;
} // MtoFT()

float KMHtoKNOTS(float kmh)
{
  return 0.5399668 * kmh;
} // KMHtoKNOTS()

#endif // UNIT_CONVERSION_H
// End of file
