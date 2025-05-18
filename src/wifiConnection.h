//! @file wifiConnection.h
//! @brief Wi-Fi connection functions
//! @details This file contains the functions to connect to Wi-Fi and
//!          logon to the router.
//! @author Karl Berger
//! @date 2025-05-16

#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

/**
 * @brief Connects the device to a Wi-Fi router.
 *
 * This function attempts to establish a connection to a predefined Wi-Fi network.
 * It handles the authentication and connection process required to access the router.
 * Ensure that the necessary Wi-Fi credentials are configured before calling this function.
 */
void logonToRouter(); // function to connect to Wi-Fi

#endif // WIFI_CONNECTION_H
// End of file