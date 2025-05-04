#include "pros/devices/battery.hpp"

#include "src/devices/vdml.hpp"
#include "v5_api_patched.h"

#include <mutex>

constexpr uint8_t PORT = 25;

namespace zest {
Result<double, UnknownError> Battery::get_capacity() {
    std::lock_guard lock(port_mutex_array.at(PORT));
    if (auto res = vexBatteryCapacityGet(); res == std::numeric_limits<typeof(res)>::max()) {
        return UnknownError("An unknown error has occurred");
    } else {
        return res;
    }
}

Result<double, UnknownError> Battery::get_current() {
    std::lock_guard lock(port_mutex_array.at(PORT));
    if (auto res = vexBatteryCurrentGet(); res == std::numeric_limits<typeof(res)>::max()) {
        return UnknownError("An unknown error has occurred");
    } else {
        return res;
    }
}

Result<double, UnknownError> Battery::get_temperature() {
    std::lock_guard lock(port_mutex_array.at(PORT));
    if (auto res = vexBatteryTemperatureGet(); res == std::numeric_limits<typeof(res)>::max()) {
        return UnknownError("An unknown error has occurred");
    } else {
        return res;
    }
}

Result<double, UnknownError> Battery::get_voltage() {
    std::lock_guard lock(port_mutex_array.at(PORT));
    if (auto res = vexBatteryVoltageGet(); res == std::numeric_limits<typeof(res)>::max()) {
        return UnknownError("An unknown error has occurred");
    } else {
        return res;
    }
}
} // namespace zest