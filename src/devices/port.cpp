#include "pros/devices/port.hpp"

namespace zest {
std::array<pros::RecursiveMutex, SmartPort::MAX_SMART_PORTS + 1> SmartPort::m_mutexes;

pros::RecursiveMutex& SmartPort::get_mutex() const {
    if (this->is_valid()) {
        return m_mutexes.at(m_index);
    } else {
        return m_mutexes.back();
    }
}

pros::RecursiveMutex& AdiPort::get_mutex() const {
    if (m_host_port.as_number() > 22 || m_index > 7) {
        // return a reference to the invalid port mutex
        return SmartPort::from_number(33).get_mutex();
    } else {
        return m_host_port.get_mutex();
    }
}
}; // namespace zest