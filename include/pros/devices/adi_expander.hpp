#pragma once

#include "port.hpp"
#include "pros/devices/port.hpp"

namespace zest {
class AdiExpander {
  public:
    constexpr AdiExpander(SmartPort port)
        : port_a(port, AdiPort::from_char('A')),
          port_b(port, AdiPort::from_char('B')),
          port_c(port, AdiPort::from_char('C')),
          port_d(port, AdiPort::from_char('D')),
          port_e(port, AdiPort::from_char('E')),
          port_f(port, AdiPort::from_char('F')),
          port_g(port, AdiPort::from_char('G')),
          port_h(port, AdiPort::from_char('H')),
          smart_port(port) {}

    const AdiPort port_a;
    const AdiPort port_b;
    const AdiPort port_c;
    const AdiPort port_d;
    const AdiPort port_e;
    const AdiPort port_f;
    const AdiPort port_g;
    const AdiPort port_h;
    const AdiPort port_invalid = AdiPort::make_invalid();

    const SmartPort smart_port;
};
}; // namespace zest