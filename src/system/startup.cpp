/**
 * \file system/startup.c
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

#include <stdio.h>

#include "kapi.h"
#include "v5_api.h"

extern "C" {

extern void rtos_initialize();
extern void vfs_initialize();
extern void system_daemon_initialize();
extern void graphical_context_daemon_initialize(void);
[[gnu::weak]]
extern void display_initialize(void) {}
extern void rtos_sched_start();
extern void vdml_initialize();
extern void invoke_install_hot_table();

extern uint32_t __bss_start;
extern uint32_t __bss_end;
extern uint32_t __sbss_start;
extern uint32_t __sbss_end;

void __libc_init_array();
void __libc_fini_array();

int main();

void vexSystemExitRequest();
void vexTasksRun();

[[gnu::section(".boot")]] void startup() {
	uint32_t* bss = &__bss_start;
	while (bss < &__bss_end) *bss++ = 0;

	uint32_t* sbss = &__sbss_start;
	while (sbss < &__sbss_end) *sbss++ = 0;

	__libc_init_array();

	{
		main();

		vexSystemExitRequest();

		while (1) {
			vexTasksRun();
		}
	}

	__libc_fini_array();

	while (1) {
		asm volatile("nop");
	}
}

// XXX: pros_init happens inside __libc_init_array, and before any global
// C++ constructors are invoked. This is accomplished by instructing
// GCC to include this function in the __init_array. The 102 argument
// gives the compiler instructions on the priority of the constructor,
// from 0-~65k. The first 0-100 priorities are reserved for language
// implementation. Priority 101 is not used to allow functions such as
// banner_enable to run before PROS initializes.
[[gnu::constructor(102)]]
static void pros_init(void) {
	rtos_initialize();

	vfs_initialize();

	vdml_initialize();

	graphical_context_daemon_initialize();

	display_initialize();

	// NOTE: this function should be called after all other initialize
	// functions. for an example of what could happen if this is not
	// the case, see
	// https://github.com/purduesigbots/pros/pull/144/#issuecomment-496901942
	system_daemon_initialize();

	invoke_install_hot_table();
}

int main() {
	rtos_sched_start();

	vexDisplayPrintf(10, 60, 1, "failed to start scheduler\n");

	printf("Failed to start Scheduler\n");
	for (;;);
}
}