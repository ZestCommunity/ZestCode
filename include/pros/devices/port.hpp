#pragma once

#include "pros/rtos.hpp"

#include <array>
#include <cstdint>
#include <limits>

namespace zest {

/**
 * @brief Smart Port class. Represents a Smart Port on the V5 brain.
 *
 * Smart Ports may be represented as a 1-indexed number or a 0-indexed number. While the
 * user expects a 1-indexed number (matching the labels on the brain), treating it as an index makes
 * development easier. This class abstracts that away so we don't have to worry about it.
 */
class SmartPort {
  public:
    /**
     * @brief Create a Smart Port using its number
     *
     * @note the smart port labelled "1" on the brain has a port number of 1
     *
     * @param port_number the port number, 1-indexed
     * @return constexpr SmartPort
     */
    static constexpr SmartPort from_number(uint8_t port_number) {
        return SmartPort(port_number - 1);
    }

    /**
     * @brief Create a Smart Port using its index
     *
     * @note the smart port labelled "1" on the brain has a port index of 0
     *
     * @param port_index the port index, 0-indexed
     * @return constexpr SmartPort
     */
    static constexpr SmartPort from_index(uint8_t port_index) {
        return SmartPort(port_index);
    }

    /**
     * @brief Get the Smart Port as a number (1-indexed)
     *
     * @return constexpr uint8_t
     */
    constexpr uint8_t as_number() const {
        return m_index + 1;
    }

    /**
     * @brief Get the Smart Port as an index (0-indexed)
     *
     * @return constexpr uint8_t
     */
    constexpr uint8_t as_index() const {
        return m_index;
    }

    constexpr bool is_physical() const {
        return m_index <= 20;
    }

    constexpr bool is_virtual() const {
        return m_index >= 21 && m_index <= 31;
    }

    constexpr bool is_valid() const {
        return m_index <= 31;
    }

    /**
     * @brief Get the mutex for the given port
     *
     * @note if the port is not valid (index > 31), a reference to a reserved mutex is returned.
     *
     * @return pros::RecursiveMutex&
     */
    pros::RecursiveMutex& get_mutex() const {
        if (this->is_valid()) {
            return m_mutexes.at(m_index);
        } else {
            return m_mutexes.at(32);
        }
    }

  private:
    /**
     * @brief array of recursive mutexes, with 33 elements.
     *
     * The mutexes in this array are used to ensure thread-safety when using devices.
     *
     * The decision to use recursive mutexes was made so device drivers could be made simpler, and
     * performance is not a concern in this scenario.
     *
     * There are 22 physical smart ports, and 10 virtual smart ports, for a total of 32.
     * Virtual smart ports are used by the VEX SDK to represent devices like the battery or a
     * controller.
     *
     * This array has 33 elements instead of 32 as you may expect. The 33rd mutex is used for
     * invalid ports.
     */
    static std::array<pros::RecursiveMutex, 33> m_mutexes;

    /**
     * @brief construct a Smart Port from an index
     *
     * This constructor is private to enforce the use of the `from_number` and `from_index` member
     * functions. Having this construct be public defeats the purpose of this class, which is to
     * prevent bugs by abstracting the port index.
     */
    explicit constexpr SmartPort(uint8_t port_index)
        : m_index(port_index) {}

    uint8_t m_index;
};

namespace ports {
/*
 * Physical smart ports have a number from 1 to 21 (inclusive). While compile-time error checking
 * could prevent an invalid port being constructed, the error messages that would be produced
 * wouldn't be very concise.
 * However, if the user tried constructing a device on the imaginary port 42, the project wouldn't
 * compile since PORT_42 isn't declared. This error message is much clearer.
 */
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
constexpr auto INVALID_SMART_PORT = SmartPort::from_index(std::numeric_limits<uint8_t>::max());
} // namespace ports

/**
 * @brief ADI Port class. Represents an ADI Port on a the brain or on a 3-wire expander.
 *
 * ADI Ports may be represented as a 0-indexed number, or a char (e.g 'a' or 'C'). This class
 * provides an interface so the developer doesn't have to worry about conversions.
 */
class AdiPort {
  public:
    // the default constructor is explicitly deleted to enforce the use of from_char and from_index
    AdiPort(SmartPort, uint8_t) = delete;

    /**
     * @brief Construct an ADI Port on an ADI expander
     *
     * @param expander_port the port of the ADI expander
     * @param adi_port the ADI port. Can be lowercase or lowercase
     */
    constexpr AdiPort(SmartPort expander_port, AdiPort adi_port)
        : m_expander_port(expander_port),
          m_index(adi_port.as_index()) {}

    /**
     * @brief Create an ADI Port using its character
     *
     * @param port can be lowercase, uppercase, or even an index
     * @return constexpr AdiPort
     */
    static constexpr AdiPort from_char(char port) {
        // if the index is provided as a character
        if (port >= '0' && port <= '8') {
            return AdiPort(port - '0');
        }
        // convert to uppercase if needed
        if (port >= 'a' && port <= 'z') {
            port -= ('a' - 'A');
        }
        // 'A' has index 0, 'B' has index 1, etc
        return AdiPort(port - 'A');
    }

    /**
     * @brief Create an ADI Port using its index
     *
     * @param index the port as an index (e.g 'A' has an index of 0)
     * @return constexpr AdiPort
     */
    static constexpr AdiPort from_index(uint8_t index) {
        return AdiPort(index);
    }

    /**
     * @brief Get the ADI Port as a char
     *
     * @note the returned char will be uppercase
     *
     * @return constexpr char
     */
    constexpr char as_char() const {
        // convert index to an uppercase letter
        return m_index + 'A';
    }

    /**
     * @brief Get the ADI Port as an index
     *
     * @return constexpr uint8_t
     */
    constexpr uint8_t as_index() const {
        return m_index;
    }

    /**
     * @brief Get the smart port of the ADI expander
     *
     * @return constexpr SmartPort
     */
    constexpr SmartPort get_expander_port() const {
        return m_expander_port;
    }

    /**
     * @brief Get the mutex for the given port
     *
     * @note if the expander port is not valid, a reference to a reserved mutex is returned.
     *
     * @return pros::RecursiveMutex&
     */
    pros::RecursiveMutex& get_mutex() const {
        if (m_expander_port.as_number() > 22) {
            return ports::INVALID_SMART_PORT.get_mutex();
        } else {
            return m_expander_port.get_mutex();
        }
    }

  private:
    // the integrated ADI ports on the brain are represented by the virtual port 22
    static constexpr auto INTEGRATED_PORT = SmartPort::from_number(22);

    /**
     * @brief construct an ADI Port from an index
     *
     * This constructor is private to enforce the use of the `from_char` and `from_index` member
     * functions. Having this construct be public defeats the purpose of this class, which is to
     * prevent bugs by abstracting the port index.
     */
    explicit constexpr AdiPort(uint8_t index)
        : m_expander_port(INTEGRATED_PORT),
          m_index(index) {}

    SmartPort m_expander_port;
    uint8_t m_index;
};

namespace ports {
/*
 * ADI ports have a char from 'A' to 'H'. While compile-time error checking could prevent an invalid
 * port being constructed, the error messages that would be produced wouldn't be very concise.
 * However, if the user tried constructing a device on the imaginary port I, the project wouldn't
 * compile since PORT_I isn't declared. This error message is much clearer.
 */
constexpr auto PORT_A = AdiPort::from_char('A');
constexpr auto PORT_B = AdiPort::from_char('B');
constexpr auto PORT_C = AdiPort::from_char('C');
constexpr auto PORT_D = AdiPort::from_char('D');
constexpr auto PORT_E = AdiPort::from_char('E');
constexpr auto PORT_F = AdiPort::from_char('F');
constexpr auto PORT_G = AdiPort::from_char('G');
constexpr auto PORT_H = AdiPort::from_char('H');
} // namespace ports
} // namespace zest