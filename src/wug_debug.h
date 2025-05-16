//!	@file wug_debug.h
//!	@brief Debugging functions	
//!	@details This file contains the functions for debugging and
//!          displaying messages on the serial monitor.
//!	@author Karl Berger
//!	@date 2025-05-13

#ifndef DEBUG_H
#define DEBUG_H	

#include <Arduino.h>  // Required for String

#ifdef DEBUG
#undef DEBUG  // Prevent conflicts with other libraries
#endif


//! Debug print macro
#ifdef WUG_DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

#endif
// End of file
