#include "pros/competition.hpp"

#include "pros/rtos.h"
#include "pros/rtos.hpp"
#include "v5_api_patched.h"

#include <optional>

namespace zest::competition {

// bitmask constants
constexpr uint32_t DISABLED_MASK = 1U << 0;
constexpr uint32_t AUTONOMOUS_MASK = 1U << 1;
constexpr uint32_t CONNECTED_MASK = 1U << 2;
constexpr uint32_t SYSTEM_MASK = 1U << 3;

// competition task
static std::optional<pros::Task> competition_task;

// callbacks
static std::optional<std::function<void()>> autonomous_func;
static std::optional<std::function<void()>> driver_control_func;
static std::optional<std::function<void()>> disabled_func;

/**
 * @brief kill the current competition task, and spawn a new one that calls the given function
 *
 * @param f the new function that should be run
 */
static void switch_task(std::optional<std::function<void()>> f) {
    // kill the current competition task if it's still running
    if (competition_task) {
        const uint32_t competition_task_state = competition_task->get_state();
        if (competition_task_state != pros::E_TASK_STATE_DELETED
            && competition_task_state != pros::E_TASK_STATE_INVALID) {
            competition_task->remove();
        }
    }

    // spawn the new task, if the given function is valid
    if (f) {
        competition_task = pros::Task::create(*f);
    }
}

// This task controls the competition task. Runs for the lifetime of the program.
// Runs at second highest priority, updates every 2 milliseconds.
static std::optional<pros::Task> control_task;

void initialize() {
    control_task = pros::Task::create([]() {
        std::optional<Mode> prev_mode;

        // only run if the mutex is available
        while (true) {
            const Mode mode = get_mode();

            // if the competition state changed, or the control task has been notified,
            // the competition task should be changed
            if (mode != prev_mode) {
                prev_mode = mode;

                switch (mode) {
                    case Mode::Disabled: {
                        switch_task(disabled_func);
                        break;
                    }
                    case Mode::Autonomous: {
                        switch_task(autonomous_func);
                        break;
                    }
                    case Mode::DriverControl: {
                        switch_task(driver_control_func);
                        break;
                    }
                }
            }

            // delay to save resources
            pros::delay(2);
        }
    }, TASK_PRIORITY_MAX - 1);
}

Mode get_mode() {
    const uint32_t status = vexCompetitionStatus();

    if ((status & DISABLED_MASK) != 0) {
        return Mode::Disabled;
    } else if ((status & AUTONOMOUS_MASK) != 0) {
        return Mode::Autonomous;
    } else {
        return Mode::DriverControl;
    }
}

System get_system() {
    if (!is_connected()) {
        return System::None;
    } else if ((vexCompetitionStatus() & SYSTEM_MASK) != 0) {
        return System::FieldControl;
    } else {
        return System::CompetitionSwitch;
    }
};

bool is_connected() {
    return (vexCompetitionStatus() & CONNECTED_MASK) != 0;
}

void register_autonomous(std::function<void()> callable) {
    autonomous_func = callable;
}

void register_driver_control(std::function<void()> callable) {
    driver_control_func = callable;
}

void register_disabled(std::function<void()> callable) {
    disabled_func = callable;
}
}; // namespace zest::competition