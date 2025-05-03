#include "src/devices/vdml.hpp"

std::array<pros::RecursiveMutex, V5_MAX_DEVICE_PORTS> port_mutex_array;