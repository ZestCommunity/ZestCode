#pragma once

#include "pros/devices/adi_expander.hpp"

namespace zest {
/**
 * @brief
 *
 */
class Brain {
  public:
    // port collections
    // there are 21 physical smart ports, 11 virtual smart ports, and 1 reserved virtual smart port
    // for invalid ports. Totals 33.
    static constinit std::array<SmartPort, 33> ports;
    static constinit AdiExpander adi;

    // physical smart ports
    // these references are simply a QoL feature
    static constinit SmartPort& port_1;
    static constinit SmartPort& port_2;
    static constinit SmartPort& port_3;
    static constinit SmartPort& port_4;
    static constinit SmartPort& port_5;
    static constinit SmartPort& port_6;
    static constinit SmartPort& port_7;
    static constinit SmartPort& port_8;
    static constinit SmartPort& port_9;
    static constinit SmartPort& port_10;
    static constinit SmartPort& port_11;
    static constinit SmartPort& port_12;
    static constinit SmartPort& port_13;
    static constinit SmartPort& port_14;
    static constinit SmartPort& port_15;
    static constinit SmartPort& port_16;
    static constinit SmartPort& port_17;
    static constinit SmartPort& port_18;
    static constinit SmartPort& port_19;
    static constinit SmartPort& port_20;
    static constinit SmartPort& port_21;

    // virtual smart ports
    static constinit SmartPort& integrated_adi_port;
    static constinit SmartPort& battery_port;
    // users may use port_invalid as a placeholder or temporary port
    static constinit SmartPort& invalid_port;
};
}; // namespace zest