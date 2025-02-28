#ifndef VDML_REGISTRY_HPP
#define VDML_REGISTRY_HPP

#include "v5_apitypes.h"
#include "vdml/vdml.hpp"

#include <cstdint>
#include <optional>

namespace zest {
namespace vdml {
enum class DeviceType {
    NONE = 0,
    MOTOR = 2,
    ROTATION = 4,
    IMU = 6,
    DISTANCE = 7,
    RADIO = 8,
    VISION = 11,
    ADI = 12,
    OPTICAL = 16,
    GPS = 20,
    AI_VISION = 29,
    SERIAL = 129,
    UNDEFINED = 255
};

struct DeviceInfo {
    DeviceType type;
    uint32_t data;
};

static std::array<std::optional<DeviceInfo>, MAX_DEVICE_PORTS> device_registry;
static V5_DeviceType registry_types[MAX_DEVICE_PORTS];

void initialize_registry();

void update_registry();
std::optional<DeviceInfo> set_device(uint8_t port, DeviceType type);
std::optional<DeviceInfo> get_device(uint8_t port);
bool validate_device(uint8_t port, DeviceType expected);
} // namespace vdml
} // namespace zest

#endif