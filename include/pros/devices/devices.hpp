#pragma once

#include "common/result.hpp"
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

/**
 * @brief V5 Port Mismatch Error
 *
 */
class SmartPortError : public ResultError {
  public:
    /**
     * @brief Construct a new V5 Port Mismatch Error object
     *
     * @param expected the device expected to be on the port
     * @param actual the device that is actually on the port
     */
    SmartPortError(DeviceType expected, std::optional<DeviceType> actual = std::nullopt);

    DeviceType expected;
    std::optional<DeviceType> actual;
};
} // namespace zest