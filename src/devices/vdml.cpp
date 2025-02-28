#include "vdml/vdml.hpp"

#include "vdml/registry.hpp"

#include <cstdlib>
#include <mutex>

using namespace zest::vdml;

// initialize the device mutex array
std::array<std::mutex, 32> device_mutexes;

std::mutex create_mutex() {
    // for now just a stub, as don't know what type of mutex will be implemented yet
    return std::mutex();
}

void initialize_vdml() {
    // initialize the device (port) mutexes
    for (int i = 0; i < MAX_DEVICE_PORTS; i++)
        device_mutexes[i] = create_mutex();

    // initialize register
    initialize_registry();
}

std::mutex& smart_port_mutex(int8_t port) {
    port = abs(port); // prevent negative port

    return device_mutexes[port];
}

bool is_valid_port(uint8_t port) {
    return port > 0 && port <= 21;
}