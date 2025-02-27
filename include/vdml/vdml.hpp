#ifndef VDML_HPP
#define VDML_HPP

#include <array>
#include <mutex>

namespace zest {
namespace vdml {
constexpr uint8_t MAX_DEVICE_PORTS = 32;

extern std::array<std::mutex, MAX_DEVICE_PORTS> device_mutexes;

std::mutex& smart_port_mutex(int port);
std::mutex create_mutex();

void initialize_vdml();

bool is_valid_port(uint8_t port);

}  // namespace vdml
}  // namespace zest

#endif  // VDML_HPP