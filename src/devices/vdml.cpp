#include "vdml/vdml.hpp"

#include <cstdlib>
#include <mutex>

#include "vdml/registry.hpp"

// initialize the device mutex array
std::array<std::mutex, 32> zest::vdml::device_mutexes;

std::mutex zest::vdml::create_mutex() {
	// for now just a stub, as don't know what type of mutex will be implemented yet
	return std::mutex();
}

void zest::vdml::initialize_vdml() {
	// initialize the device (port) mutexes
	for (int i = 0; i < zest::vdml::MAX_DEVICE_PORTS; i++) zest::vdml::device_mutexes[i] = create_mutex();

	// initialize ADI port mutexes
}

std::mutex& zest::vdml::smart_port_mutex(int8_t port) {
	port = abs(port);  // prevent negative port

	return zest::vdml::device_mutexes[port];
}

bool is_valid_port(uint8_t port) {
	return port > 0 && port <= 21;
}