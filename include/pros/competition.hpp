#pragma once

#include <functional>

namespace zest::competition {
/**
 * @brief the competition mode (disabled, driver control, autonomous)
 *
 */
enum class Mode {
    Disabled,
    DriverControl,
    Autonomous,
};

/**
 * @brief the competition system being used
 *
 */
enum class System {
    FieldControl,
    CompetitionSwitch,
    None,
};

/**
 * @brief Get the competition mode (disabled, driver control, autonomous)
 *
 * @return Status
 */
Mode get_mode();

/**
 * @brief Get the system type being used (field control, competition switch, or none)
 *
 * @return System
 */
System get_system();

/**
 * @brief Whether field control or a competition switch is connected
 *
 * @return true
 * @return false
 */
bool is_connected();

/**
 * @brief register the callable that will be called when the autonomous period starts.
 *
 * The callable is called whenever the competition state changes to autonomous. The task that runs
 * it is killed as soon as the competition state changes to driver control or disabled.
 *
 * @param callable
 */
void register_autonomous(std::function<void()> callable);

/**
 * @brief register the callable that will be called when the driver control period starts.
 *
 * The callable is called whenever the competition state changes to autonomous. The task that runs
 * it is killed as soon as the competition state changes to driver control or disabled.
 *
 * @param callable
 */
void register_driver_control(std::function<void()> callable);

/**
 * @brief register the callable that will be called when the disabled period starts.
 *
 * The callable is called whenever the competition state changes to autonomous. The task that
 * runs it is killed as soon as the competition state changes to driver control or disabled.
 *
 * @param callable
 */
void register_disabled(std::function<void()> callable);
} // namespace zest::competition