/**
 * @file aprsService.h
 * @brief APRS service header
 * @details Contains functions for posting weather data and bulletins to APRS-IS.
 * 
 * @author Karl Berger
 * @date 2025-05-14
 */

/**
 * @param int lineCount
 * @brief Number of aphorisms in file.
 */

/**
 * @param bool amBulletinSent
 * @brief Flag indicating if the APRS morning bulletin has been sent.
 */

/**
 * @param bool pmBulletinSent
 * @brief Flag indicating if the APRS evening bulletin has been sent.
 */

/**
 * @param int lineIndex
 * @brief Index for the APRS bulletin.
 */

/**
 * @brief Posts a message to APRS-IS.
 * @param message The message to post.
 */
void postToAPRS(String message);

/**
 * @brief Formats weather data for APRS.
 * @return Formatted weather string.
 */
String APRSformatWeather();

/**
 * @brief Sends formatted weather data to APRS-IS.
 */
void APRSsendWX();

/**
 * @brief Formats a bulletin message for APRS.
 * @param message The bulletin message.
 * @param ID The bulletin ID.
 * @return Formatted bulletin string.
 */
String APRSformatBulletin(String message, String ID);

/**
 * @brief Sends a bulletin message to APRS-IS.
 * @param msg The bulletin message.
 * @param ID The bulletin ID.
 */
void APRSsendBulletin(String msg, String ID);

/**
 * @brief Pads a float value to a specified width for APRS formatting.
 * @param value The value to pad.
 * @param width The width to pad to.
 * @return Padded string.
 */
String APRSpadder(float value, int width);

/**
 * @brief Pads a callsign for APRS formatting.
 * @param callSign The callsign to pad.
 * @return Padded callsign string.
 */
String APRSpadCall(String callSign);

/**
 * @brief Formats latitude and longitude for APRS location.
 * @param lat Latitude.
 * @param lon Longitude.
 * @return Formatted location string.
 */
String APRSlocation(float lat, float lon);