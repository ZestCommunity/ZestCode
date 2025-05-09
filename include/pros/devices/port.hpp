#pragma once

#include "pros/rtos.hpp"

#include <cctype>

namespace zest {

class SmartPort {
  public:
    static constexpr SmartPort from_number(uint8_t number) {
        return SmartPort(number - 1);
    }

    static constexpr SmartPort from_index(uint8_t index) {
        return SmartPort(index);
    }

    constexpr uint8_t as_number() {
        return m_index - 1;
    }

    constexpr uint8_t as_index() {
        return m_index;
    }

  private:
    constexpr SmartPort(uint8_t index)
        : m_index(index) {}

    uint8_t m_index;
};

namespace ports {

constexpr auto PORT_1 = SmartPort::from_number(1);
constexpr auto PORT_2 = SmartPort::from_number(2);
constexpr auto PORT_3 = SmartPort::from_number(3);
constexpr auto PORT_4 = SmartPort::from_number(4);
constexpr auto PORT_5 = SmartPort::from_number(5);
constexpr auto PORT_6 = SmartPort::from_number(6);
constexpr auto PORT_7 = SmartPort::from_number(7);
constexpr auto PORT_8 = SmartPort::from_number(8);
constexpr auto PORT_9 = SmartPort::from_number(9);
constexpr auto PORT_10 = SmartPort::from_number(10);
constexpr auto PORT_11 = SmartPort::from_number(11);
constexpr auto PORT_12 = SmartPort::from_number(12);
constexpr auto PORT_13 = SmartPort::from_number(13);
constexpr auto PORT_14 = SmartPort::from_number(14);
constexpr auto PORT_15 = SmartPort::from_number(15);
constexpr auto PORT_16 = SmartPort::from_number(16);
constexpr auto PORT_17 = SmartPort::from_number(17);
constexpr auto PORT_18 = SmartPort::from_number(18);
constexpr auto PORT_19 = SmartPort::from_number(19);
constexpr auto PORT_20 = SmartPort::from_number(20);
constexpr auto PORT_21 = SmartPort::from_number(21);

} // namespace ports

class AdiPort {
  public:
    static constexpr AdiPort from_char(char port) {
        return AdiPort(port);
    }

    constexpr char as_char() {
        return m_port;
    }

    constexpr uint8_t as_index() {
        return static_cast<uint8_t>(m_port) - 65;
    }

  private:
    constexpr AdiPort(char port) {
        // convert lowercase to uppercase if needed
        if (port >= 97) {
            port -= 32;
        }
        m_port = port;
    }

    char m_port;
};

namespace ports {

constexpr auto PORT_A = AdiPort::from_char('A');
constexpr auto PORT_B = AdiPort::from_char('B');
constexpr auto PORT_C = AdiPort::from_char('C');
constexpr auto PORT_D = AdiPort::from_char('D');
constexpr auto PORT_E = AdiPort::from_char('E');
constexpr auto PORT_F = AdiPort::from_char('F');
constexpr auto PORT_G = AdiPort::from_char('G');
constexpr auto PORT_H = AdiPort::from_char('H');

} // namespace ports

template<typename T>
pros::RecursiveMutex& get_port_mutex(T&&) {
    throw("can't get mutex for non-port object!");
}
} // namespace zest