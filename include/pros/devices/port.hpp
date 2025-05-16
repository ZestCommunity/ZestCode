#pragma once

#include <cstdint>

namespace zest {
class SmartPort {
  public:
    static constexpr SmartPort from_number(uint8_t port_number) {
        return SmartPort(port_number - 1);
    }

    static constexpr SmartPort from_index(uint8_t port_index) {
        return SmartPort(port_index);
    }

    constexpr uint8_t as_number() const {
        return m_index + 1;
    }

    constexpr uint8_t as_index() const {
        return m_index;
    }

  private:
    constexpr SmartPort(int port_index)
        : m_index(port_index) {}

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
    static constexpr AdiPort from_letter(char port) {
        // if the index is provided as a character
        if (port >= '0' && port <= '9') {
            return AdiPort(port - '0');
        }
        // convert to uppercase if needed
        if (port >= 'a' && port <= 'z') {
            port -= ('a' - 'A');
        }
        // 'A' has index 0, 'B' has index 1, etc
        return AdiPort(port - 'A');
    }

    static constexpr AdiPort from_index(uint8_t index) {
        return AdiPort(index);
    }

    constexpr char as_letter() const {
        return m_index;
    }

    constexpr uint8_t as_index() const {
        return m_index;
    }

  private:
    uint8_t m_index;

    constexpr AdiPort(uint8_t index)
        : m_index(index) {}
};

namespace ports {
constexpr auto PORT_A = AdiPort::from_letter('A');
constexpr auto PORT_B = AdiPort::from_letter('B');
constexpr auto PORT_C = AdiPort::from_letter('C');
constexpr auto PORT_D = AdiPort::from_letter('D');
constexpr auto PORT_E = AdiPort::from_letter('E');
constexpr auto PORT_F = AdiPort::from_letter('F');
constexpr auto PORT_G = AdiPort::from_letter('G');
constexpr auto PORT_H = AdiPort::from_letter('H');

} // namespace ports
} // namespace zest