#include "pros/devices/screen.hpp"

#include "pros/rtos.hpp"
#include "v5_api_patched.h"
#include "v5_apitypes_patched.h"

#include <utility>
#include <vector>

#ifndef ZESTCODE_CONFIG_SCREEN_TOUCH_LISTENER_TASK_INTERVAL
    // Default to 16 ms if not defined
    #define ZESTCODE_CONFIG_SCREEN_TOUCH_LISTENER_TASK_INTERVAL 1000 / zest::Screen::FRAMERATE
#endif

#ifndef ZESTCODE_CONFIG_SCREEN_TOUCH_LISTENER_WARNING_THRESHOLD
    // Default to 5 ms if not defined
    #define ZESTCODE_CONFIG_SCREEN_TOUCH_LISTENER_WARNING_THRESHOLD 5
#endif
namespace zest {

void Screen::on_touched(
    std::function<void(const TouchEvent&)> listener_cb,
    std::optional<Screen::TouchEvent::State> state_filter
) {
    struct Listener {
        std::function<void(const TouchEvent&)> callback;
        std::optional<Screen::TouchEvent::State> filter;
    };

    static std::vector<Listener> listeners;
    listeners.emplace_back(std::move(listener_cb), state_filter);

    // Wait! There's a sdk function that does this for us!: vexTouchUserCallbackSet()
    // However, using this would cause the callback to be called from the same task that calls
    // vexTasksRun(), the system_daemon task. This is potentially quite dangerous, as the user could
    // wait on device data which will never come since the system_daemon task would never reach
    // vexTasksRun() again to update the data.
    // Instead we use a separate task to check for touch events and call the listeners.

    // TODO: Determine thread safety? (What happens if task A calls this, and while touch_task is
    // being constructed, task B preempts it and calls this again?)
    static pros::Task touch_task([] {
        while (true) {
            const auto touch = Screen::get_last_touch();
            const auto state = touch.state;
            static Screen::TouchEvent::State prev_state = state;
            static size_t prev_time = pros::millis();

            if (prev_state != state)
                for (const auto& listener : listeners)
                    if (!listener.filter.has_value() || listener.filter.value() == state) {
                        const size_t start_time = pros::millis();
                        listener.callback(touch);

                        // TODO: Determine the time taken by the actual callback. For example, if
                        // the callback was preempted mid-way through, that time spend being preempt
                        // should not count.
                        const size_t elapsed_time = pros::millis() - start_time;
                        if (listeners.size() > 1
                            && elapsed_time
                                   > ZESTCODE_CONFIG_SCREEN_TOUCH_LISTENER_WARNING_THRESHOLD) {
                            // TODO: Log a warning if the callback took too long
                        }
                    }
            prev_state = state;

            // TODO: I recall there being problems with using plain pros::c::task_delay_until with
            // lemlib. Do those apply here?

            // Use pros::c::task_delay_until() instead of pros::delay() to avoid drift
            pros::c::task_delay_until(
                &prev_time,
                ZESTCODE_CONFIG_SCREEN_TOUCH_LISTENER_TASK_INTERVAL
            );
        }
    });
}

void Screen::on_pressed(std::function<void(const TouchEvent&)> listener_cb) {
    on_touched(std::move(listener_cb), TouchEvent::State::Pressed);
}

void Screen::on_released(std::function<void(const TouchEvent&)> listener_cb) {
    on_touched(std::move(listener_cb), TouchEvent::State::Released);
}

void Screen::on_held(std::function<void(const TouchEvent&)> listener_cb) {
    on_touched(std::move(listener_cb), TouchEvent::State::Held);
}

void Screen::set_render_mode(Screen::RenderMode new_mode) {
    // TODO: Implement
}

Screen::RenderMode Screen::get_render_mode() {
    // TODO: Implement
}

} // namespace zest