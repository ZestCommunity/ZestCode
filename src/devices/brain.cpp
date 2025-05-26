#include "pros/devices/brain.hpp"

#include <mutex>

namespace zest {

pros::RecursiveMutex& Brain::get_smart_port_mutex(SmartPort smart_port) {
    // If the port is invalid, return the invalid port mutex.
    // Otherwise, return the respective mutex
    if (smart_port.as_number() > 32) {
        return m_mutexes.at(Brain::INVALID_PORT.as_index());
    } else {
        return m_mutexes.at(smart_port.as_index());
    }
}

void Brain::smart_port_mutex_lock_all() {
    std::apply([&](auto&... args) {
        std::lock(args...);
    }, Brain::m_mutexes);
}

void Brain::smart_port_mutex_unlock_all() {
    for (auto& mutex : Brain::m_mutexes) {
        mutex.unlock();
    }
}

// The VEX SDK has 32 virtual smart ports.
// 21 of these ports represent physical smart ports,
// the rest represent things like the battery, the controller, etc.
// This array has 33 mutexes. The extra mutex is used for invalid ports.
constinit std::array<pros::RecursiveMutex, 33> Brain::m_mutexes;

} // namespace zest