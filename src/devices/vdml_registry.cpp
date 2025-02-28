#include "v5_api.h"
#include "vdml/registry.hpp"
#include "vdml/vdml.hpp"

#include <optional>

using namespace zest::vdml;

void initialize_registry() {
    // update registry types
    update_registry();
    for (int i = 0; i < 22; i++) {
        device_registry[i]->type = (DeviceType)registry_types[i];
        device_registry[i]->data = (int)vexDeviceGetByIndex(i);
    }
}

void update_registry() {
    vexDeviceGetStatus(registry_types);
}

std::optional<DeviceInfo> set_device(uint8_t port, DeviceType type) {
    // TODO: add error messages
    // invalid port
    if (!is_valid_port(port))
        return std::nullopt;
    // in use
    if (device_registry[port]->type != DeviceType::NONE)
        return std::nullopt;
    // device type mismatch
    if ((DeviceType)registry_types[port] != type
        && (DeviceType)registry_types[port] != DeviceType::NONE)
        return std::nullopt;

    DeviceInfo device = {.type = type, .data = (uint32_t)vexDeviceGetByIndex(port)};

    device_registry[port] = device;

    return device;
}

std::optional<DeviceInfo> get_device(uint8_t port) {
    if (!is_valid_port(port))
        return std::nullopt;

    return device_registry[port];
}

bool validate_device(uint8_t port, DeviceType expected) {
    if (!is_valid_port(port))
        return false;

    DeviceType registered = device_registry[port]->type;
    DeviceType actual = (DeviceType)registry_types[port];

    // automatically register the port, if needed
    if (registered == DeviceType::NONE && actual != DeviceType::NONE) {
        zest::vdml::set_device(port, actual);
        registered = device_registry[port]->type;
    }

    // TODO: add error / warn messages

    if ((expected == registered || expected == zest::vdml::DeviceType::NONE)
        && registered == actual) {
        // all good
        return true;
    } else if (actual == zest::vdml::DeviceType::NONE) {
        // no device
        return false;
    } else {
        // device type mismatch
        return false;
    }
}
