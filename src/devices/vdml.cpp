#include "vdml/vdml.hpp"

#include "pros/rtos.hpp"
#include "vdml/registry.hpp"

pros::Mutex* create_mutex() {
    // for now just a stub, as don't know what type of mutex will be implemented yet
    return new pros::Mutex();
}

void initialize_vdml() {
    // initialize the device (port) mutexes
    for (int i = 0; i < zest::vdml::MAX_DEVICE_PORTS; i++)
        zest::vdml::device_mutexes[i] = zest::vdml::create_mutex();

    // initialize register
    zest::vdml::initialize_registry();
}

pros::Mutex* smart_port_mutex(uint8_t port) {
    return zest::vdml::device_mutexes.at(port);
}

bool is_valid_port(uint8_t port) {
    return port > 0 && port <= 21;
}