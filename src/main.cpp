#include "main.h"

void initialize() {
    pros::delay(3000);
    std::cout << "hello world!" << std::endl;
    for (int i = 100; i > 0; i--) {
        pros::c::motor_move(18, i);
        pros::delay(10);
    }
    // std::abort();
}