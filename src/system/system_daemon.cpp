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
static pros::Task system_daemon([]() {
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

} // namespace zest