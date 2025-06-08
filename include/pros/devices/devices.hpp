#pragma once

#include "common/result.hpp"

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
    None,
    Unknown,
};

/**
 * @brief V5 Port Mismatch Error
 *
 */
class V5PortMismatchError : public ResultError {
  public:
    /**
     * @brief Construct a new V5 Port Mismatch Error object
     *
     * @param expected the device expected to be on the port
     * @param actual the device that is actually on the port
     */
    V5PortMismatchError(DeviceType expected, DeviceType actual)
        : expected(expected),
          actual(actual) {}

    DeviceType expected;
    DeviceType actual;
};
} // namespace zest