#include "pros/competition.hpp"
#include "pros/rtos.hpp"

#include <stdint.h>

extern "C" {
void vexDisplayPrintf(int32_t xpos, int32_t ypos, uint32_t bOpaque, const char* format, ...);
void vexDisplayErase();
}

int main() {
    zest::competition::register_driver_control([]() {
        vexDisplayErase();
        vexDisplayPrintf(10, 100, 1, "Driver Control Running\n");
        vexDisplayPrintf(10, 60, 1, "num tasks: %u\n", pros::Task::get_count());
        // infinite loop to test task priorities. If everything is working correctly, the
        // competition task will still switch when the competition state changes
        while (true) {
            asm("nop");
        }
    });
    zest::competition::register_autonomous([]() {
        vexDisplayErase();
        vexDisplayPrintf(10, 100, 1, "Autonomous Running\n");
        vexDisplayPrintf(10, 60, 1, "num tasks: %u\n", pros::Task::get_count());
        // infinite loop to test task priorities. If everything is working correctly, the
        // competition task will still switch when the competition state changes
        while (true) {
            asm("nop");
        }
    });
    zest::competition::register_disabled([]() {
        vexDisplayErase();
        vexDisplayPrintf(10, 100, 1, "Disabled Running\n");
        vexDisplayPrintf(10, 60, 1, "num tasks: %u\n", pros::Task::get_count());
        // infinite loop to test task priorities. If everything is working correctly, the
        // competition task will still switch when the competition state changes
        while (true) {
            asm("nop");
        }
    });

    vexDisplayPrintf(10, 60, 1, "demonstrating that tasks are run after main returns\n");
    pros::delay(3000);
    vexDisplayErase();
}