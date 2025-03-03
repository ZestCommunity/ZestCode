/**
 * \file pros/misc.hpp
 * \ingroup cpp-pros
 *
 * Contains prototypes for miscellaneous functions pertaining to the controller,
 * battery, and competition control.
 *
 * This file should not be modified by users, since it gets replaced whenever
 * a kernel upgrade occurs.
 *
 * \copyright (c) 2017-2024, Purdue University ACM SIGBots.
 * All rights reservered.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \defgroup cpp-misc Miscellaneous C++ API
 * \note Additional example code for this module can be found in its [Tutorial.](@ref controller)
 */

#ifndef _PROS_MISC_HPP_
#define _PROS_MISC_HPP_

#include <cstdint>
#include <string>

#include "pros/misc.h"

namespace pros {
namespace battery {
/**
 * \addtogroup cpp-misc
 * ///@{
 */
/**
 * Gets the current voltage of the battery, as reported by VEXos.
 *
 * This function uses the following values of errno when an error state is
 * reached:
 * EACCES - Another resource is currently trying to access the battery port.
 *
 * \return The current voltage of the battery
 *
 * \b Example
 * \code
 * void initialize() {
 *   printf("Battery Level: %.2f\n", get_capacity());
 * }
 * \endcode
 */
double get_capacity(void);

/**
 * Gets the current current of the battery in milliamps, as reported by VEXos.
 *
 * This function uses the following values of errno when an error state is
 * reached:
 * EACCES - Another resource is currently trying to access the battery port.
 *
 * \return The current current of the battery
 *
 * \b Example
 * \code
 * void initialize() {
 *   printf("Battery Current: %d\n", get_current());
 * }
 * \endcode
 */
int32_t get_current(void);

/**
 * Gets the current temperature of the battery, as reported by VEXos.
 *
 * This function uses the following values of errno when an error state is
 * reached:
 * EACCES - Another resource is currently trying to access the battery port.
 *
 * \return The current temperature of the battery
 *
 * \b Example
 * \code
 * void initialize() {
 *   printf("Battery's Temperature: %.2f\n", get_temperature());
 * }
 * \endcode
 */
double get_temperature(void);

/**
 * Gets the current capacity of the battery in millivolts, as reported by VEXos.
 *
 * This function uses the following values of errno when an error state is
 * reached:
 * EACCES - Another resource is currently trying to access the battery port.
 *
 * \return The current capacity of the battery
 *
 * \b Example
 * \code
 * void initialize() {
 *   printf("Battery's Voltage: %d\n", get_voltage());
 * }
 * \endcode
 */
int32_t get_voltage(void);
///@}
}  // namespace battery

namespace competition {
/**
 * Get the current status of the competition control.
 *
 * \return The competition control status as a mask of bits with
 * COMPETITION_{ENABLED,AUTONOMOUS,CONNECTED}.
 *
 * \b Example
 * \code
 * void status_display_task(){
 * 	if(!is_connected()) {
 * 	 pros::lcd::print(0, "V5 Brain is not connected!");
 *  }
 *  if(is_autonomous()) {
 * 	 pros::lcd::print(0, "V5 Brain is in autonomous mode!");
 *  }
 *  if(!is_disabled()) {
 * 	 pros::lcd::print(0, "V5 Brain is disabled!");
 *  }
 * \endcode
 */
std::uint8_t get_status(void);
std::uint8_t is_autonomous(void);
std::uint8_t is_connected(void);
std::uint8_t is_disabled(void);
std::uint8_t is_field_control(void);
std::uint8_t is_competition_switch(void);
}  // namespace competition

namespace usd {
/**
 * Checks if the SD card is installed.
 *
 * \return 1 if the SD card is installed, 0 otherwise
 *
 * \b Example
 * \code
 * void opcontrol() {
 *   printf("%i", is_installed());
 * }
 * \endcode
 */
std::int32_t is_installed(void);
/**
 * Lists the files in a directory specified by the path
 * Puts the list of file names (NOT DIRECTORIES) into the buffer seperated by newlines
 * 
 * This function uses the following values of errno when an error state is
 * reached:
 * 
 * EIO - Hard error occured in the low level disk I/O layer
 * EINVAL - file or directory is invalid, or length is invalid
 * EBUSY - THe physical drinve cannot work
 * ENOENT - cannot find the path or file
 * EINVAL - the path name format is invalid
 * EACCES - Access denied or directory full
 * EEXIST - Access denied
 * EROFS - SD card is write protected
 * ENXIO - drive number is invalid or not a FAT32 drive
 * ENOBUFS - drive has no work area
 * ENFILE - too many open files
 *
 *
 *
 * \note use a path of "\" to list the files in the main directory NOT "/usd/"
 *  DO NOT PREPEND YOUR PATHS WITH "/usd/"
 *
 * \return 1 on success or PROS_ERR on failure setting errno
 *
 * \b Example
 * \code
 * void opcontrol() {
 * 	char* test = (char*) malloc(128);
 *	pros::usd::list_files("/", test, 128);
 *	pros::delay(200);
 *	printf("%s\n", test); //Prints the file names in the root directory seperated by newlines
 *  pros::delay(100);
 *  pros::list_files("/test", test, 128);
 *	pros::delay(200);
 *	printf("%s\n", test); //Prints the names of files in the folder named test seperated by newlines
 *  pros::delay(100);
 * }
 * \endcode
 */

std::int32_t list_files(const char* path, char* buffer, std::int32_t len);
}  // namespace usd

}  // namespace pros

#endif  // _PROS_MISC_HPP_
