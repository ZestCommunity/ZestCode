#pragma once

#include "port.hpp"
#include "pros/devices/port.hpp"

namespace zest {

/**
 * @brief ADI Expander class. Used to control access to ADI ports
 *
 */
class AdiExpander {
  public:
    /**
     * @brief Construct a new ADI Expander object
     *
     * @param port the smart port the ADI expander is connected to
     */
    constexpr AdiExpander(SmartPort port)
        : port_a(port, 'A'),
          port_b(port, 'B'),
          port_c(port, 'C'),
          port_d(port, 'D'),
          port_e(port, 'E'),
          port_f(port, 'F'),
          port_g(port, 'G'),
          port_h(port, 'H'),
          port_invalid(port, 'J'),
          smart_port(port) {}

    AdiPort port_a;
    AdiPort port_b;
    AdiPort port_c;
    AdiPort port_d;
    AdiPort port_e;
    AdiPort port_f;
    AdiPort port_g;
    AdiPort port_h;
    // users may use port_invalid as a placeholder or temporary port
    AdiPort port_invalid;

    SmartPort smart_port;
};
}; // namespace zest