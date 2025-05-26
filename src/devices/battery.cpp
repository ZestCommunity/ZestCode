#include "pros/devices/battery.hpp"

#include "pros/devices/brain.hpp"
#include "v5_api_patched.h"

#include <mutex>

namespace zest {
double Battery::get_capacity() {
    std::lock_guard lock(Brain::get_smart_port_mutex(Brain::BATTERY_PORT));
    return vexBatteryCapacityGet();
}

double Battery::get_current() {
    std::lock_guard lock(Brain::get_smart_port_mutex(Brain::BATTERY_PORT));
    return vexBatteryCurrentGet();
}

double Battery::get_temperature() {
    std::lock_guard lock(Brain::get_smart_port_mutex(Brain::BATTERY_PORT));
    return vexBatteryTemperatureGet();
}

double Battery::get_voltage() {
    std::lock_guard lock(Brain::get_smart_port_mutex(Brain::BATTERY_PORT));
    return vexBatteryVoltageGet();
}
} // namespace zest