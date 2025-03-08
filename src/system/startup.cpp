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

#include "v5_api.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sys/unistd.h>

extern "C" {

// Initialization routines provided elsewhere
void rtos_initialize();
void vfs_initialize();
void system_daemon_initialize();
void graphical_context_daemon_initialize();

[[gnu::weak]]
void display_initialize() {}

void rtos_sched_start();
void vdml_initialize();

// libc initialization
void __libc_init_array();

// tick vex tasks
void vexTasksRun();
} // extern "C"

// this goes in the first 32-byte chunk of the user program
// which is why the entrypoint is offset from 0x3800000 by 0x20
// only the first 16 bytes of this chunk is used however
// see the vcodesig definition in the SDK for more details
[[gnu::section(".boot_data")]]
vcodesig vexCodeSig = {V5_SIG_MAGIC, V5_SIG_TYPE_USER, V5_SIG_OWNER_PARTNER, V5_SIG_OPTIONS_NONE};

// The pros_init function is executed early (via constructor attribute)
// before most global C++ constructors are run.
[[gnu::constructor(102)]]
static void pros_init() {
    rtos_initialize();
    vfs_initialize();
    vdml_initialize();
    graphical_context_daemon_initialize();
    display_initialize();
    // Note: system_daemon_initialize must be called last, per design requirements.
    system_daemon_initialize();
}

// the main function, starts the scheduler and ensures the program exits gracefully if it fails
int main() {
    // Start freeRTOS
    rtos_sched_start();

    // If execution reaches here, the scheduler has failed.
    vexDisplayPrintf(10, 60, 1, "failed to start scheduler\n");
    std::printf("Failed to start Scheduler\n");
    _exit(0); // exit with code 0 to stop spinlock
}

// program entrypoint. This is the first function that is run
// it sets up memory, calls constructors, and then calls main
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

    // call the main function
    // This GCC warning is a nuisance.
    // This is the industry standard
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    main();
#pragma GCC diagnostic pop
}
