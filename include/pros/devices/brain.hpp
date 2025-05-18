#pragma once

#include "pros/devices/adi_expander.hpp"

namespace zest {
class Brain {
  public:
    static constexpr SmartPort port_1 = SmartPort::from_number(1);
    static constexpr SmartPort port_2 = SmartPort::from_number(2);
    static constexpr SmartPort port_3 = SmartPort::from_number(3);
    static constexpr SmartPort port_4 = SmartPort::from_number(4);
    static constexpr SmartPort port_5 = SmartPort::from_number(5);
    static constexpr SmartPort port_6 = SmartPort::from_number(6);
    static constexpr SmartPort port_7 = SmartPort::from_number(7);
    static constexpr SmartPort port_8 = SmartPort::from_number(8);
    static constexpr SmartPort port_9 = SmartPort::from_number(9);
    static constexpr SmartPort port_10 = SmartPort::from_number(10);
    static constexpr SmartPort port_11 = SmartPort::from_number(11);
    static constexpr SmartPort port_12 = SmartPort::from_number(12);
    static constexpr SmartPort port_13 = SmartPort::from_number(13);
    static constexpr SmartPort port_14 = SmartPort::from_number(14);
    static constexpr SmartPort port_15 = SmartPort::from_number(15);
    static constexpr SmartPort port_16 = SmartPort::from_number(16);
    static constexpr SmartPort port_17 = SmartPort::from_number(17);
    static constexpr SmartPort port_18 = SmartPort::from_number(18);
    static constexpr SmartPort port_19 = SmartPort::from_number(19);
    static constexpr SmartPort port_20 = SmartPort::from_number(20);
    static constexpr SmartPort port_21 = SmartPort::from_number(21);

    static constexpr SmartPort integrated_adi_port = SmartPort::from_number(22);
    static constexpr SmartPort battery_port = SmartPort::from_number(25);
    static constexpr SmartPort port_invalid = SmartPort::make_invalid();

    static constexpr AdiExpander adi = AdiExpander(integrated_adi_port);
};
}; // namespace zest