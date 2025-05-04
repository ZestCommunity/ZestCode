#pragma once

#include "common/result.hpp"

namespace zest {

enum class DeviceType {
    Battery,
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