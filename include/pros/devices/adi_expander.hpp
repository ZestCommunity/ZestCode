#pragma once

#include "port.hpp"
#include "pros/devices/port.hpp"

namespace zest {
class AdiExpander {
  public:
    AdiExpander(SmartPort& port)
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
    AdiPort port_invalid;

    SmartPort& smart_port;
};
}; // namespace zest