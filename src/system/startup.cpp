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

#include <sys/unistd.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <span>

#include "v5_api.h"

extern "C" {

// Initialization routines provided elsewhere
void rtos_initialize();
void vfs_initialize();
void system_daemon_initialize();
void graphical_context_daemon_initialize();
[[gnu::weak]]
void display_initialize() {}  // weak definition
void rtos_sched_start();
void vdml_initialize();
void invoke_install_hot_table();

// Symbols provided by the linker script
extern uint32_t __bss_start;
extern uint32_t __bss_end;
extern uint32_t __sbss_start;
extern uint32_t __sbss_end;

// libc initialization
void __libc_init_array();

// tick vex tasks
void vexTasksRun();
}  // extern "C"

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
	invoke_install_hot_table();
}

[[gnu::section(".boot")]]
int main() {
	// Calculate the number of uint32_t elements in the BSS and SBSS sections.
	const auto bssCount = static_cast<size_t>(&__bss_end - &__bss_start);
	const auto sbssCount = static_cast<size_t>(&__sbss_end - &__sbss_start);

	// Create spans for the BSS and SBSS memory ranges and zero them out.
	const std::span<uint32_t> bssSpan(&__bss_start, bssCount);
	const std::span<uint32_t> sbssSpan(&__sbss_start, sbssCount);
	std::fill(bssSpan.begin(), bssSpan.end(), 0);
	std::fill(sbssSpan.begin(), sbssSpan.end(), 0);

	// Initialize libc
	__libc_init_array();

	// Start freeRTOS
	rtos_sched_start();

	// If execution reaches here, the scheduler has failed.
	vexDisplayPrintf(10, 60, 1, "failed to start scheduler\n");
	std::printf("Failed to start Scheduler\n");
	_exit(0);  // exit with code 0 to stop spinlock
}
