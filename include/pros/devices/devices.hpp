#pragma once

#include "pros/devices/port.hpp"

namespace zest {

/**
 * @brief device type enum. Contains all the device types compatible with ZestCode
 *
 */
enum class DeviceType {
    AdiExpander,
    AiVision,
    Bumper,
    Controller,
    Distance,
    Gps,
    Imu,
    Motor,
    Optical,
    Radio,
    Rotation,
    Serial,
    Vision,
    Invalid,
    None,
    Unknown,
};

/**
 * @brief Get the type of the device connected to the given smart port
 *
 * @param port
 * @return DeviceType
 */
DeviceType get_device_type(SmartPort port);

} // namespace zest