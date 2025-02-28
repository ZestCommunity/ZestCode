#ifndef VDML_HPP
#define VDML_HPP

#include "pros/device.h"

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
} // namespace vdml
} // namespace zest

// STUBS SO OLD VDML CODE COMPILES
#define VALIDATE_PORT_NO(PORT) ((PORT) >= 0 && (PORT) < NUM_V5_PORTS)
#define VALIDATE_PORT_NO_INTERNAL(PORT) ((PORT) >= 0 && (PORT) < V5_MAX_DEVICE_PORTS)
#define claim_port(port, device_type, error_code)                                                  \
    if (registry_validate_binding(port, device_type) != 0) {                                       \
        return error_code;                                                                         \
    }                                                                                              \
    v5_smart_device_s_t* device = registry_get_device(port);                                       \
    if (!port_mutex_take(port)) {                                                                  \
        errno = EACCES;                                                                            \
        return error_code;                                                                         \
    }
#define claim_port_i(port, device_type) claim_port(port, device_type, PROS_ERR)
#define claim_port_f(port, device_type) claim_port(port, device_type, PROS_ERR_F)
int32_t claim_port_try(uint8_t port, pros::c::v5_device_e_t type);
#define return_port(port, rtn)                                                                     \
    port_mutex_give(port);                                                                         \
    return rtn;
extern int32_t port_errors;
void vdml_set_port_error(uint8_t port);
void vdml_unset_port_error(uint8_t port);
bool vdml_get_port_error(uint8_t port);
int port_mutex_take(uint8_t port);
int port_mutex_give(uint8_t port);
void port_mutex_take_all();
void port_mutex_give_all();
int internal_port_mutex_take(uint8_t port);
int internal_port_mutex_give(uint8_t port);
#define V5_PORT_BATTERY 24
#define V5_PORT_CONTROLLER_1 25
#define V5_PORT_CONTROLLER_2 26

#endif // VDML_HPP