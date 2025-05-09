#include "pros/devices/port.hpp"

#include "pros/rtos.hpp"
#include "v5_apitypes_patched.h"

namespace zest {
template<>
pros::RecursiveMutex& get_port_mutex<SmartPort>(SmartPort&& port) {
    static std::array<pros::RecursiveMutex, V5_MAX_DEVICE_PORTS> array;
    return array.at(port.as_index());
}

template<>
pros::RecursiveMutex& get_port_mutex<AdiPort>(AdiPort&& port) {
    static std::array<pros::RecursiveMutex, 8> array;
    return array.at(port.as_index());
}
} // namespace zest