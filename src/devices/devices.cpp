#include "pros/devices/devices.hpp"

#include "v5_api_patched.h"
#include "v5_apitypes_patched.h"

namespace zest {
DeviceType get_device_type(SmartPort port) {
    // if the port number is greater than 21, return invalid
    if (port.as_number() > 21)
        return DeviceType::Invalid;

    // vexDeviceGetStatus writes a buffer of V5_DeviceType that is V5_MAX_DEVICE_PORTS long.
    // This is the only way to get the device type on a port
    std::array<V5_DeviceType, V5_MAX_DEVICE_PORTS> types;
    vexDeviceGetStatus(types.data());

    // get the device type on the given port
    switch (types.at(port.as_index())) {
        case kDeviceTypeNoSensor:
            return DeviceType::None;
        case kDeviceTypeMotorSensor:
            return DeviceType::Motor;
        case kDeviceTypeAbsEncSensor:
            return DeviceType::Rotation;
        case kDeviceTypeImuSensor:
            return DeviceType::Imu;
        case kDeviceTypeDistanceSensor:
            return DeviceType::Distance;
        case kDeviceTypeRadioSensor:
            return DeviceType::Radio;
        case kDeviceTypeTetherSensor:
            return DeviceType::Controller;
        case kDeviceTypeVisionSensor:
            return DeviceType::Vision;
        case kDeviceTypeAdiSensor:
            return DeviceType::AdiExpander;
        case kDeviceTypeOpticalSensor:
            return DeviceType::Optical;
        case kDeviceTypeGpsSensor:
            return DeviceType::Gps;
        case kDeviceTypeAiVisionSensor:
            return DeviceType::AiVision;
        case kDeviceTypeBumperSensor:
            return DeviceType::Bumper;
        case kDeviceTypeGenericSerial:
            return DeviceType::Serial;
        default:
            return DeviceType::Unknown;
    }
}

SmartPortError::SmartPortError(DeviceType expected, std::optional<DeviceType> actual)
    : expected(expected),
      actual(actual) {}
} // namespace zest