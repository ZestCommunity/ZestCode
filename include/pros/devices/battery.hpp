#pragma once

namespace zest {

class Battery {
  public:
    /**
     * @brief Get the remaining capacity of the robot battery
     *
     * TODO: document how the returned value should be interpreted (e.g 0-1 or 0-100)
     *
     * @return double
     */
    static double get_capacity();
    /**
     * @brief Get the current drawn from the robot battery
     *
     * TODO: document how the returned value should be interpreted (e.g amps or milliamps)
     *
     * @return double
     */
    static double get_current();
    /**
     * @brief Get the temperature of the robot battery
     *
     * TODO: document how the returned value should be interpreted (e.g celsius or fahrenheit)
     *
     * @return double
     */
    static double get_temperature();
    /**
     * @brief Get the voltage of the robot battery
     *
     * TODO: document how the returned value should be interpreted (e.g celsius or fahrenheit)
     *
     * @return double
     */
    static double get_voltage();
};

} // namespace zest