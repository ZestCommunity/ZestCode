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
     * @brief Create an invalid Smart Port
     *
     * Users may want to create a device on an invalid port for testing. This function provides an
     * idiomatic way to create an invalid port.
     *
     * @return constexpr SmartPort
     */
    static constexpr SmartPort make_invalid() {
        return SmartPort(std::numeric_limits<uint8_t>::max());
    }

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

    /**
     * @brief whether the smart port represents a physical or virtual smart port
     *
     * @return true the smart port is physical
     * @return false the smart port is virtual or invalid
     */
    constexpr bool is_physical() const {
        return m_index <= 20;
    }

    /**
     * @brief whether the smart port represents a physical or virtual smart port
     *
     * @return true the smart port is virtual
     * @return false the smart port is physical or invalid
     */
    constexpr bool is_virtual() const {
        return m_index >= 21 && is_valid();
    }

    /**
     * @brief whether the smart port is valid or not
     *
     * @return true the smart port is valid
     * @return false the smart port is invalid
     */
    constexpr bool is_valid() const {
        return m_index < MAX_SMART_PORTS;
    }

    /**
     * @brief Get the mutex for the given port
     *
     * @note if the port is not valid (index > 31), a reference to a reserved mutex is returned.
     *
     * @return pros::RecursiveMutex&
     */
    pros::RecursiveMutex& get_mutex() const;

    static constexpr uint8_t MAX_SMART_PORTS = 32; /**< the maximum number of smart ports */

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
    static std::array<pros::RecursiveMutex, MAX_SMART_PORTS + 1> m_mutexes;

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
     * @brief Create an invalid ADI Port
     *
     * Users may want to create a device on an invalid port for testing. This function provides an
     * idiomatic way to create an invalid port.
     *
     * @return constexpr SmartPort
     */
    static constexpr AdiPort make_invalid() {
        return AdiPort(std::numeric_limits<uint8_t>::max());
    }

    /**
     * @brief Construct an ADI Port on an ADI expander
     *
     * @param expander_port the port of the ADI expander
     * @param adi_port the ADI port. Can be lowercase or lowercase
     */
    constexpr AdiPort(SmartPort expander_port, AdiPort adi_port)
        : m_host_port(expander_port),
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
        return m_host_port;
    }

    /**
     * @brief Get the mutex for the given port
     *
     * @note if the expander port or ADI port is not valid, a reference to a reserved mutex is
     * returned.
     *
     * @return pros::RecursiveMutex&
     */
    pros::RecursiveMutex& get_mutex() const {
        if (m_host_port.as_number() > 22 || m_index > 7) {
            // return a reference to the invalid port mutex
            return SmartPort::from_number(33).get_mutex();
        } else {
            return m_host_port.get_mutex();
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
        : m_host_port(INTEGRATED_PORT),
          m_index(index) {}

    SmartPort m_host_port;
    uint8_t m_index;
};
} // namespace zest