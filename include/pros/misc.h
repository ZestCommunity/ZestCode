/**
 * \file pros/misc.h
 * \ingroup c-misc
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
 * \defgroup c-misc Miscellaneous C API
 * \note Additional example code for this module can be found in its [Tutorial.](@ref controller)
 */

#ifndef _PROS_MISC_H_
#define _PROS_MISC_H_

#include <stdint.h>

#define NUM_V5_PORTS (22)

/**
 * \ingroup c-misc
 */

/**
 * \addtogroup c-misc
 *  @{
 */

/// \name V5 Competition
//@{

/*#define COMPETITION_DISABLED (1 << 0)
#define COMPETITION_AUTONOMOUS (1 << 1)
#define COMPETITION_CONNECTED (1 << 2)
#define COMPETITION_SYSTEM (1 << 3)*/
typedef enum {
    COMPETITION_DISABLED = 1 << 0,
    COMPETITION_CONNECTED = 1 << 2,
    COMPETITION_AUTONOMOUS = 1 << 1,
    COMPETITION_SYSTEM = 1 << 3,
} competition_status;

#ifdef __cplusplus
extern "C" {
namespace pros {
namespace c {
#endif

/**
 * \fn competition_get_status(void)
 * Get the current status of the competition control.
 *
 * \return The competition control status as a mask of bits with
 * COMPETITION_{ENABLED,AUTONOMOUS,CONNECTED}.
 *
 * \b Example
 * \code
 * void initialize() {
 *   if (competition_get_status() & COMPETITION_CONNECTED == true) {
 *     // Field Control is Connected
 *     // Run LCD Selector code or similar
 *   }
 * }
 * \endcode
 */
uint8_t competition_get_status(void);

/**
 * \fn competition_is_disabled()
 *
 * \return True if the V5 Brain is disabled, false otherwise.
 *
 * \b Example
 * \code
 * void my_task_fn(void* ignore) {
 *   while (!competition_is_disabled()) {
 *   // Run competition tasks (like Lift Control or similar)
 *   }
 * }
 *
 * void initialize() {
 *   task_t my_task = task_create(my_task_fn, NULL, TASK_PRIO_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "My
 * Task");
 * }
 * \endcode
 */
uint8_t competition_is_disabled(void);

/**
 * \return True if the V5 Brain is connected to competition control, false otherwise.
 *
 * \b Example
 * \code
 * void initialize() {
 *   if (competition_is_connected()) {
 *     // Field Control is Connected
 *     // Run LCD Selector code or similar
 *   }
 * }
 * \endcode
 */
uint8_t competition_is_connected(void);

/**
 * \return True if the V5 Brain is in autonomous mode, false otherwise.
 *
 * \b Example
 * \code
 * void my_task_fn(void* ignore) {
 *   while (!competition_is_autonomous()) {
 *     // Wait to do anything until autonomous starts
 *     delay(2);
 *   }
 *   while (competition_is_autonomous()) {
 *     // Run whatever code is desired to just execute in autonomous
 *     }
 * }
 *
 * void initialize() {
 *   task_t my_task = task_create(my_task_fn, NULL, TASK_PRIO_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "My
 * Task");
 * }
 * \endcode
 */
uint8_t competition_is_autonomous(void);

/**
 * \return True if the V5 Brain is connected to VEXnet Field Controller, false otherwise.
 *
 * \b Example
 * \code
 * void initialize() {
 *   if (competition_is_field()) {
 *     // connected to VEXnet Field Controller
 *   }
 * }
 * \endcode
 */
uint8_t competition_is_field(void);

/**
 * \return True if the V5 Brain is connected to VEXnet Competition Switch, false otherwise.
 *
 * \b Example
 * \code
 * void initialize() {
 *   if (competition_is_switch()) {
 *     // connected to VEXnet Competition Switch
 *   }
 * }
 */
uint8_t competition_is_switch(void);

#ifdef __cplusplus
}
}
}
#endif
///@}

/// \name V5 Controller
///@{
#ifdef __cplusplus
extern "C" {
namespace pros {
#endif
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
 *   printf("Battery's Voltage: %d\n", battery_get_voltage());
 * }
 * \endcode
 */
int32_t battery_get_voltage(void);

/**
 * Gets the current current of the battery, as reported by VEXos.
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
 *   printf("Battery Current: %d\n", battery_get_current());
 * }
 * \endcode
 */
int32_t battery_get_current(void);

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
 *   printf("Battery's Temperature: %d\n", battery_get_temperature());
 * }
 * \endcode
 */
double battery_get_temperature(void);

/**
 * Gets the current capacity of the battery, as reported by VEXos.
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
 *   printf("Battery Level: %d\n", battery_get_capacity());
 * }
 * \endcode
 */
double battery_get_capacity(void);

/**
 * Checks if the SD card is installed.
 *
 * \return 1 if the SD card is installed, 0 otherwise
 *
 * \b Example
 * \code
 * void opcontrol() {
 *   printf("%i", usd_is_installed());
 * }
 * \endcode
 */
int32_t usd_is_installed(void);

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
 *	pros::c::usd_list_files("/", test, 128);
 *	pros::delay(200);
 *	printf("%s\n", test); //Prints the file names in the root directory seperated by newlines
 *  pros::delay(100);
 *  pros::c::usd_list_files("/test", test, 128);
 *	pros::delay(200);
 *	printf("%s\n", test); //Prints the names of files in the folder named test seperated by
 *newlines pros::delay(100);
 * }
 * \endcode
 */
int32_t usd_list_files(const char* path, char* buffer, int32_t len);

/******************************************************************************/
/**                              Date and Time                               **/
/******************************************************************************/

extern const char* baked_date;
extern const char* baked_time;

typedef struct {
    uint16_t year; // Year - 1980
    uint8_t day;
    uint8_t month; // 1 = January
} date_s_t;

typedef struct {
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t sec_hund; // hundredths of a second
} time_s_t;

///@}

///@}

#ifdef __cplusplus
}
} // namespace pros
#endif

#endif // _PROS_MISC_H_
