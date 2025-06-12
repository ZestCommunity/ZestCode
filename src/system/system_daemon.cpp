#include "kapi.h"
#include "pros/devices/brain.hpp"
#include "pros/rtos.hpp"

extern "C" {
void vexTasksRun();
void ser_output_flush();
}

namespace zest {

// This task runs for the lifetime of the program.
// It periodically calls vexTasksRun, which copies shared memory to/from vexOS.
// it's wrapped in an optional, because we only want to start it after all constructors are run.
// This means it needs to be initialized by the _start function in startup.cpp
static std::optional<pros::Task> system_daemon;

/**
 * @brief Initialize the system daemon task
 *
 */
void initialize_system_daemon() {
    if (!system_daemon) {
        system_daemon = pros::Task::create([]() {
            while (true) {
                // lock all smart port mutexes
                Brain::smart_port_mutex_lock_all();
                // flush serial output
                ser_output_flush();
                // suspend all tasks
                rtos_suspend_all();
                // copy shared memory
                vexTasksRun();
                // resume all tasks
                rtos_resume_all();
                // unlock all smart port mutexes
                zest::Brain::smart_port_mutex_unlock_all();

                // delay to save resources
                pros::delay(2);
            }
        }, TASK_PRIORITY_MAX);
    }
}

} // namespace zest