/**
 * \file system/startup.cpp
 *
 * Contains the main startup code for PROS 3.0. main is called from vexStartup
 * code. Our main() initializes data structures and starts the FreeRTOS
 * scheduler.
 *
 * \copyright Copyright (c) 2017-2024, Purdue University ACM SIGBots.
 * All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pros/competition.hpp"
#include "pros/rtos.hpp"
#include "v5_api_patched.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sys/unistd.h>

extern "C" {
// Initialization routines provided elsewhere
void rtos_initialize();
void vfs_initialize();
void rtos_sched_start();
void __libc_init_array();
}

// this goes in the first 32-byte chunk of the user program
// which is why the entrypoint is offset from 0x3800000 by 0x20
// only the first 16 bytes of this chunk is used however
// see the vcodesig definition in the SDK for more details
[[gnu::section(".boot_data")]]
vcodesig boot_data = {
    .magic = V5_SIG_MAGIC,
    .type = V5_SIG_TYPE_USER,
    .owner = V5_SIG_OWNER_PARTNER,
    .options = V5_SIG_OPTIONS_NONE,
};

// The pros_init function is executed early (via constructor attribute)
[[gnu::constructor(101)]]
static void pros_init() {
    rtos_initialize();
    vfs_initialize();
}

// forward-declare system daemon initialization function
void initialize_system_daemon();
// forward-declare main function
int main();

// Program entrypoint. This is the first function that is run.
// It sets up memory, calls constructors, and starts the scheduler
extern "C" [[gnu::section(".boot")]]
void _start() {
    // Symbols provided by the linker script
    extern uint32_t __bss_start;
    extern uint32_t __bss_end;
    extern uint32_t __sbss_start;
    extern uint32_t __sbss_end;
    // don't try refactoring this code with stuff like std::fill or std::span.
    // It's been tried before, and it causes UB.
    // It's suspected that this is due to libc not being initialized yet.
    for (uint32_t* bss = &__bss_start; bss < &__bss_end; bss++)
        *bss = 0;
    for (uint32_t* sbss = &__sbss_start; sbss < &__sbss_end; sbss++)
        *sbss = 0;

    // call global constructors
    __libc_init_array();

    // initialize the system daemon
    initialize_system_daemon();

    // start main task
    // these pragmas are needed to silence the same warning on clang and gcc
    // normally you aren't supposed to reference the main function
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmain"
    pros::Task main_task([]() {
        // run the main function
        main();
        // initialize the competition control task
        zest::competition::initialize();
    });
#pragma clang diagnostic pop
#pragma GCC diagnostic pop

    // start the scheduler
    rtos_sched_start();

    // If execution reaches here, the scheduler has failed.
    vexDisplayPrintf(10, 60, 1, "failed to start scheduler\n");
    std::printf("Failed to start Scheduler\n");
    _exit(0); // exit with code 0 to stop spinlock
}
