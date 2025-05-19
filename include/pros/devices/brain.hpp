#pragma once

#include "pros/devices/adi_expander.hpp"

namespace zest {
class Brain {
  public:
    // port collections
    static std::array<SmartPort, 33> ports;
    static AdiExpander adi;

    // physical smart ports
    static SmartPort& port_1;
    static SmartPort& port_2;
    static SmartPort& port_3;
    static SmartPort& port_4;
    static SmartPort& port_5;
    static SmartPort& port_6;
    static SmartPort& port_7;
    static SmartPort& port_8;
    static SmartPort& port_9;
    static SmartPort& port_10;
    static SmartPort& port_11;
    static SmartPort& port_12;
    static SmartPort& port_13;
    static SmartPort& port_14;
    static SmartPort& port_15;
    static SmartPort& port_16;
    static SmartPort& port_17;
    static SmartPort& port_18;
    static SmartPort& port_19;
    static SmartPort& port_20;
    static SmartPort& port_21;

    // virtual smart ports
    static SmartPort& integrated_adi_port;
    static SmartPort& battery_port;
    static SmartPort& invalid_port;
};
}; // namespace zest