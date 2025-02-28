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

// STUBS SO OLD VDML CODE COMPILES

typedef struct {
    pros::c::v5_device_e_t device_type;
    V5_DeviceT device_info;
    uint8_t pad[128]; // 16 bytes in adi_data_s_t times 8 ADI Ports = 128
} v5_smart_device_s_t;

void registry_update_types();
v5_smart_device_s_t* registry_get_device(uint8_t port);
v5_smart_device_s_t* registry_get_device_internal(uint8_t port);
int32_t registry_validate_binding(uint8_t port, pros::c::v5_device_e_t expected_t);

#endif