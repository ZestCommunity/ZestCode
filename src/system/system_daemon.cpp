/**
 * \file system/system_daemon.c
 *
 * Competition control daemon responsible for invoking the user tasks.
 *
 * \copyright (c) 2017-2024, Purdue University ACM SIGBots.
 * All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "kapi.h"
#include "pros/devices/brain.hpp"
#include "pros/devices/port.hpp"
#include "pros/rtos.hpp"
#include "system/optimizers.h"
#include "system/user_functions.h" // IWYU pragma: keep

#include <mutex>

extern "C" {
void vexTasksRun();
void ser_output_flush();
}

/**
 * @brief lock a sequence of smart ports
 *
 * For whatever reason, std::lock only compiles if you pass the mutexes like this:
 * std::lock(mutex_a, mutex_b, mutex_c);
 * You can't pass it an array or iterator or lambda, so we have to use this helper function instead.
 */
template<std::size_t... Is>
static void lock_ports(std::index_sequence<Is...>) {
    std::lock(zest::Brain::ports[Is].mutex...);
}

/**
 * @brief unlock a sequence of smart ports
 *
 * For whatever reason, std::unlock only compiles if you pass the mutexes like this:
 * std::unlock(mutex_a, mutex_b, mutex_c);
 * You can't pass it an array or iterator or lambda, so we have to use this helper function instead
 */
template<std::size_t... Is>
static void unlock_ports(std::index_sequence<Is...>) {
    constexpr std::size_t N = sizeof...(Is);
    // Unlock in reverse order (RAII best practice)
    (zest::Brain::ports[N - 1 - Is].mutex.unlock(), ...);
}

/**
 * @brief lock all smart port mutexes
 *
 */
static void port_mutex_lock_all() {
    // this looks weird because of how std::lock is implemented. See lock_ports for details.
    constexpr auto num_ports = std::tuple_size<decltype(zest::Brain::ports)>::value;
    lock_ports(std::make_index_sequence<num_ports>{});
}

/**
 * @brief unlock all smart port mutexes
 *
 */
static void port_mutex_unlock_all() {
    // this looks weird because of how std::unlock is implemented. See unlock_ports for details.
    constexpr auto num_ports = std::tuple_size<decltype(zest::Brain::ports)>::value;
    unlock_ports(std::make_index_sequence<num_ports>{});
}

static void _disabled_task(void*);
static void _autonomous_task(void*);
static void _opcontrol_task(void*);
static void _competition_initialize_task(void*);
static void _initialize_task(void*);
static void _system_daemon_task(void*);

static task_stack_t competition_task_stack[TASK_STACK_DEPTH_DEFAULT];
static static_task_s_t competition_task_buffer;
static pros::task_t competition_task;

static task_stack_t system_daemon_task_stack[TASK_STACK_DEPTH_DEFAULT];
static static_task_s_t system_daemon_task_buffer;
static pros::task_t system_daemon_task;

enum state_task {
    E_OPCONTROL_TASK = 0,
    E_AUTON_TASK,
    E_DISABLED_TASK,
    E_COMP_INIT_TASK
};

static const char task_names[4][32] = {
    "User Operator Control (PROS)",
    "User Autonomous (PROS)",
    "User Disabled (PROS)",
    "User Comp. Init. (PROS)"
};

static task_fn_t task_fns[4] =
    {_opcontrol_task, _autonomous_task, _disabled_task, _competition_initialize_task};

// does the basic background operations that need to occur every 2ms
static inline void do_background_operations() {
    port_mutex_lock_all();
    ser_output_flush();
    rtos_suspend_all();
    vexTasksRun();
    rtos_resume_all();
    port_mutex_unlock_all();
}

static void _system_daemon_task(void*) {
    uint32_t time = pros::millis();
    // Initialize status to an invalid state to force an update the first loop
    uint32_t status = (uint32_t)(1 << 8);
    uint32_t task_state;

    // XXX: Delay likely necessary for shared memory to get copied over
    // (discovered b/c VDML would crash and burn)
    // Take all port mutexes to prevent user code from attempting to access VDML during this time.
    // User code could be running if a task is created from a global ctor
    port_mutex_lock_all();
    pros::c::task_delay(2);
    port_mutex_unlock_all();

    // start up user initialize task. once the user initialize function completes,
    // the _initialize_task will notify us and we can go into normal competition
    // monitoring mode
    competition_task = task_create_static(
        _initialize_task,
        NULL,
        TASK_PRIORITY_DEFAULT,
        TASK_STACK_DEPTH_DEFAULT,
        "User Initialization (PROS)",
        competition_task_stack,
        &competition_task_buffer
    );

    time = pros::millis();
    while (!pros::c::task_notify_take(true, 2)) {
        // wait for initialize to finish
        do_background_operations();
    }
    while (1) {
        do_background_operations();

        if (unlikely(status != pros::c::competition_get_status())) {
            // Have a new competition status, need to clean up whatever's running
            uint32_t old_status = status;
            status = pros::c::competition_get_status();
            enum state_task state = E_OPCONTROL_TASK;
            if ((status & COMPETITION_DISABLED) && (old_status & COMPETITION_DISABLED)) {
                // Don't restart the disabled task even if other bits have changed (e.g. auton bit)
                continue;
            }

            // competition initialize runs only when entering disabled and we're
            // connected to competition control
            if ((status ^ old_status) & COMPETITION_CONNECTED
                && (status & (COMPETITION_DISABLED | COMPETITION_CONNECTED))
                       == (COMPETITION_DISABLED | COMPETITION_CONNECTED)) {
                state = E_COMP_INIT_TASK;
            } else if (status & COMPETITION_DISABLED) {
                state = E_DISABLED_TASK;
            } else if (status & COMPETITION_AUTONOMOUS) {
                state = E_AUTON_TASK;
            }

            task_state = pros::c::task_get_state(competition_task);
            // delete the task only if it's in normal operation (e.g. not deleted)
            // The valid task states AREN'T deleted, invalid, or running (running
            // means it's the current task, which will never be the case)
            if (task_state == pros::E_TASK_STATE_READY || task_state == pros::E_TASK_STATE_BLOCKED
                || task_state == pros::E_TASK_STATE_SUSPENDED) {
                pros::c::task_delete(competition_task);
            }

            competition_task = task_create_static(
                task_fns[state],
                NULL,
                TASK_PRIORITY_DEFAULT,
                TASK_STACK_DEPTH_DEFAULT,
                task_names[state],
                competition_task_stack,
                &competition_task_buffer
            );
        }

        pros::c::task_delay_until(&time, 2);
    }
}

void system_daemon_initialize() {
    system_daemon_task = task_create_static(
        _system_daemon_task,
        NULL,
        TASK_PRIORITY_MAX - 2,
        TASK_STACK_DEPTH_DEFAULT,
        "PROS System Daemon",
        system_daemon_task_stack,
        &system_daemon_task_buffer
    );
}

// these functions are what actually get called by the system daemon, which
// attempt to call whatever the user declares
#define FUNC(NAME)                                                                                 \
    static void _##NAME##_task(void* ign) {                                                        \
        user_##NAME();                                                                             \
        pros::c::task_notify(system_daemon_task);                                                  \
    }
#include "system/user_functions/c_list.h"
#undef FUNC
