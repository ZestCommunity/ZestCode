#pragma once

#include "pros/rtos.hpp"

#include <cstdint>

namespace zest {

// forward-declare friend classes
class Brain;
class AdiExpander;

/**
 * @brief Smart Port class. Represents a Smart Port on the V5 brain.
 *
 */
class SmartPort {
    // SmartPort instances can only be constructed by the Brain class
    friend class Brain;

  public:
    /**
     * @brief Get the Smart Port as a number (1-indexed)
     *
     * @return constexpr uint8_t
     */
    uint8_t as_number() const {
        return m_number;
    }

    /**
     * @brief Get the Smart Port as an index (0-indexed)
     *
     * @return constexpr uint8_t
     */
    uint8_t as_index() const {
        return m_number - 1;
    }

    pros::RecursiveMutex mutex;

  private:
    /**
     * @brief construct a Smart Port from an index
     *
     */
    SmartPort(uint8_t port_number)
        : m_number(port_number) {}

    uint8_t m_number;
};

/**
 * @brief ADI Port class. Represents an ADI Port on a the brain or on a 3-wire expander.
 *
 * ADI Ports may be represented as a 0-indexed number, or a char (e.g 'a' or 'C'). This class
 * provides an interface so the developer doesn't have to worry about conversions.
 */
class AdiPort {
    // AdiPort instances can only be constructed by the AdiExpander class
    friend AdiExpander;

  public:
    /**
     * @brief Get the ADI Port as a char
     *
     * @note the returned char will be uppercase
     *
     * @return constexpr char
     */
    char as_char() const {
        // convert index to an uppercase letter
        return m_index + 'A';
    }

    /**
     * @brief Get the ADI Port as an index
     *
     * @return constexpr uint8_t
     */
    uint8_t as_index() const {
        return m_index;
    }

    SmartPort& host_port;

  private:
    /**
     * @brief Create an ADI Port using its character
     *
     * @param host_port the smart port of the ADI hub this adi port belongs to
     * @param port must be uppercase, 'A' - 'H'
     * @return constexpr AdiPort
     */
    constexpr AdiPort(SmartPort& host_port, char port)
        : host_port(host_port),
          m_index(port - 'A') {}

    uint8_t m_index;
};
} // namespace zest