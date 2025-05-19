#include "pros/devices/brain.hpp"

namespace zest {
// collection of smart ports
std::array<SmartPort, 33> Brain::ports = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                                          12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
                                          23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33};

// adi ports integrated in the brain
AdiExpander Brain::adi = AdiExpander(battery_port);
// physical smart ports
SmartPort& Brain::port_1 = ports.at(0);
SmartPort& Brain::port_2 = ports.at(1);
SmartPort& Brain::port_3 = ports.at(2);
SmartPort& Brain::port_4 = ports.at(3);
SmartPort& Brain::port_5 = ports.at(4);
SmartPort& Brain::port_6 = ports.at(5);
SmartPort& Brain::port_7 = ports.at(6);
SmartPort& Brain::port_8 = ports.at(7);
SmartPort& Brain::port_9 = ports.at(8);
SmartPort& Brain::port_10 = ports.at(9);
SmartPort& Brain::port_11 = ports.at(10);
SmartPort& Brain::port_12 = ports.at(11);
SmartPort& Brain::port_13 = ports.at(12);
SmartPort& Brain::port_14 = ports.at(13);
SmartPort& Brain::port_15 = ports.at(14);
SmartPort& Brain::port_16 = ports.at(15);
SmartPort& Brain::port_17 = ports.at(16);
SmartPort& Brain::port_18 = ports.at(17);
SmartPort& Brain::port_19 = ports.at(18);
SmartPort& Brain::port_20 = ports.at(19);
SmartPort& Brain::port_21 = ports.at(20);

// virtual smart ports
SmartPort& Brain::integrated_adi_port = ports.at(21);
SmartPort& Brain::battery_port = ports.at(24);
SmartPort& Brain::invalid_port = ports.at(32);
} // namespace zest