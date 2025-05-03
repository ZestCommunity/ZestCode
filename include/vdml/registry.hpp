/**
 * \file vdml/registry.h
 *
 * This file contains the standard header info for the VDML (Vex Data Management
 * Layer) registry.
 *
 * \copyright Copyright (c) 2017-2024, Purdue University ACM SIGBots.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "pros/apix.h"
#include "v5_api_patched.h"

typedef struct {
    pros::c::V5Device device_type;
    V5_DeviceT device_info;
    uint8_t pad[128]; // 16 bytes in adi_data_s_t times 8 ADI Ports = 128
} v5_smart_device_s_t;

/*
 * Detects the devices that are plugged in.
 *
 * Pulls the type names of plugged-in devices and stores them in the buffer
 * registry_types.
 */
void registry_update_types();

/*
 * Returns the information on the device registered to the port.
 *
 * This function uses the following values of errno when an error state is
 * reached:
 * ENXIO - The given value is not within the range of V5 ports (1-21).
 *
 * \param port
 *        The V5 port number from 1-21
 *
 * \return A struct containing the device type and the info needed for api
 * functions
 */
v5_smart_device_s_t* registry_get_device(uint8_t port);

v5_smart_device_s_t* registry_get_device_internal(uint8_t port);
