#pragma once

#include "pros/rtos.hpp"
#include "v5_apitypes_patched.h"

#include <array>

extern std::array<pros::RecursiveMutex, V5_MAX_DEVICE_PORTS> port_mutex_array;

enum class V5Device {
    None,     ///< No device is plugged into the port
    Motor,    ///< A motor is plugged into the port
    Rotation, ///< A rotation sensor is plugged into the port
    Imu,      ///< An inertial sensor is plugged into the port
    Distance, ///< A distance sensor is plugged into the port
    Radio,    ///< A radio is plugged into the port
    Vision,   ///< A vision sensor is plugged into the port
    Adi,      ///< This port is an ADI expander
    Optical,  ///< An optical sensor is plugged into the port
    Gps,      ///< A GPS sensor is plugged into the port
    AiVision, ///< An AI Vision sensor is plugged into the port
    Serial,   ///< A serial device is plugged into the port
};
