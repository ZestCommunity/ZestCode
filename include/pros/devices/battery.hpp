#pragma once

#include "common/result.hpp"

namespace zest {

class Battery {
  public:
    static Result<double, UnknownError> get_capacity();
    static Result<double, UnknownError> get_current();
    static Result<double, UnknownError> get_temperature();
    static Result<double, UnknownError> get_voltage();
};

} // namespace zest