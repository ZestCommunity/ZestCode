/**
 * \file devices/competition.cpp
 *
 * Contains competition control functions.
 *
 * \copyright Copyright (c) 2017-2024, Purdue University ACM SIGBots.
 * All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pros/misc.h"
#include "v5_api_patched.h"

// TODO: remove this C API
extern "C" {
uint8_t competition_get_status(void) {
    return vexCompetitionStatus();
}

uint8_t competition_is_disabled(void) {
    return (competition_get_status() & COMPETITION_DISABLED) != 0;
}

uint8_t competition_is_connected(void) {
    return (competition_get_status() & COMPETITION_CONNECTED) != 0;
}

uint8_t competition_is_autonomous(void) {
    return (competition_get_status() & COMPETITION_AUTONOMOUS) != 0;
}

uint8_t competition_is_field(void) {
    return ((competition_get_status() & COMPETITION_SYSTEM) != 0) && competition_is_connected();
}

uint8_t competition_is_switch(void) {
    return ((competition_get_status() & COMPETITION_SYSTEM) == 0) && competition_is_connected();
}
}

namespace pros {
namespace competition {
using namespace pros::c;

uint8_t get_status(void) {
    return competition_get_status();
}

uint8_t is_autonomous(void) {
    return competition_is_autonomous();
}

uint8_t is_connected(void) {
    return competition_is_connected();
}

uint8_t is_disabled(void) {
    return competition_is_disabled();
}

uint8_t is_field_control(void) {
    return competition_is_field();
}

uint8_t is_competition_switch(void) {
    return competition_is_switch();
}

} // namespace competition
} // namespace pros
