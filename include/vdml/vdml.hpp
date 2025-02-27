#ifndef VDML_HPP
#define VDML_HPP

#include <array>
#include <mutex>

namespace vdml {
extern std::array<std::mutex, 21> port_mutexes;

std::mutex& smart_port_mutex(int port);

}  // namespace vdml

#endif  // VDML_HPP