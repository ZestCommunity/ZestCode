#pragma once

#include "pros/devices/adi_expander.hpp"
#include "pros/devices/screen.hpp"
#include "pros/rtos.hpp"

namespace zest {
/**
 * @brief
 *
 */
class Brain {
  public:
    // physical smart ports
    static constexpr SmartPort PORT_1 = SmartPort(1);
    static constexpr SmartPort PORT_2 = SmartPort(2);
    static constexpr SmartPort PORT_3 = SmartPort(3);
    static constexpr SmartPort PORT_4 = SmartPort(4);
    static constexpr SmartPort PORT_5 = SmartPort(5);
    static constexpr SmartPort PORT_6 = SmartPort(6);
    static constexpr SmartPort PORT_7 = SmartPort(7);
    static constexpr SmartPort PORT_8 = SmartPort(8);
    static constexpr SmartPort PORT_9 = SmartPort(9);
    static constexpr SmartPort PORT_10 = SmartPort(10);
    static constexpr SmartPort PORT_11 = SmartPort(11);
    static constexpr SmartPort PORT_12 = SmartPort(12);
    static constexpr SmartPort PORT_13 = SmartPort(13);
    static constexpr SmartPort PORT_14 = SmartPort(14);
    static constexpr SmartPort PORT_15 = SmartPort(15);
    static constexpr SmartPort PORT_16 = SmartPort(16);
    static constexpr SmartPort PORT_17 = SmartPort(17);
    static constexpr SmartPort PORT_18 = SmartPort(18);
    static constexpr SmartPort PORT_19 = SmartPort(19);
    static constexpr SmartPort PORT_20 = SmartPort(20);
    static constexpr SmartPort PORT_21 = SmartPort(21);

    // virtual smart ports
    static constexpr SmartPort INTEGRATED_ADI_PORT = SmartPort(22);
    static constexpr SmartPort BATTERY_PORT = SmartPort(25);
    // users may use port_invalid as a placeholder or temporary port
    static constexpr SmartPort INVALID_PORT = SmartPort(33);

    // all adi ports on the brain
    static constexpr AdiExpander adi = AdiExpander(INTEGRATED_ADI_PORT);

    // TODO: conditionally declare functions below, to prevent users accidentally calling them
    // TODO: replace lock_all and unlock_all with a single function that returns an std::scoped_lock

    /**
     * @brief Get the mutex of a smart port
     *
     * @warning this function should not be used by typical users
     *
     * @param smart_port the smart port to get the mutex of
     * @return pros::RecursiveMutex&
     */
    static pros::RecursiveMutex& get_smart_port_mutex(SmartPort smart_port);

    /**
     * @brief lock all smart port mutexes
     *
     * uses std::lock to prevent deadlocks
     */
    static void smart_port_mutex_lock_all();

    /**
     * @brief unlock all smart port mutexes
     *
     */
    static void smart_port_mutex_unlock_all();

    using Screen = zest::Screen;

  private:
    static constinit std::array<pros::RecursiveMutex, 33> m_mutexes;
};
}; // namespace zest