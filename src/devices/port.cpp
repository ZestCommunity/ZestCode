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
}; // namespace zest