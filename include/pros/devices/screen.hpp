#pragma once

#include "pros/rtos.hpp"
#include "stdint.h"

#include <functional>
#include <optional>

namespace zest {

/**
 * @brief A friendlier, documented and safer API wrapper around the Vex SDK's display functions.
 * 
 * @note Heavily inspired by vexide's display module: https://docs.rs/vexide/0.7.0/vexide/devices/display/index.html
 */
struct Screen {
    /** Vertical height taken up by the user program in pixels. */
    static constexpr int16_t HEADER_HEIGHT = 32;
    /** The width of the screen in pixels. */
    static constexpr int16_t WIDTH = 480;
    /** The height of the screen in pixels, excludes the header. */
    static constexpr int16_t HEIGHT = 240;
    /** The framerate of the Brain is 60fps. */
    static constexpr int16_t FRAMERATE = 60;

    /** A touch event on the screen.  */
    struct TouchEvent {
        enum class State {
            /** The screen has been released. */
            Released,
            /** The screen has been touched. */
            Pressed,
            /** The screen has been touched and is still being held. */
            Held
        };

        /** The current state of the touch. */
        State state;
        /** The x coordinate of the touch. */
        int16_t x;
        /** The y coordinate of the touch. */
        int16_t y;

        // TODO: Determine when it starts counting
        /** The number of times the screen has been pressed. */
        int32_t pressCount;
        /** The number of times the screen has been released. */
        int32_t releaseCount;
    };

    /**
     * @brief Gets the most recent touch event.
     *
     * TODO: Determine the starting return value (Before the screen is touched).
     *
     * @return The most recent touch event.
     */
    static TouchEvent get_last_touch();

    /**
     * @brief Subscribes the listener to be called when the screen touch state changes.
     *
     * Spawn a new task to check for events if it is not already running.
     * All listeners are called from this task.
     *
     * TODO: Elaborate on below?
     * @warning If you have multiple listeners, avoid using delays, which can delay other callbacks
     * from being called, and result in lost information.
     * For this reason, it is recommended library developers use get_last_touch() instead of this
     * function.
     *
     * @param listener_cb The function to call when the screen's touch state changes.
     * @param state_filter Optional filter for the touch state. If provided, only events matching
     * this state will trigger the callback. For example, if state_filter ==
     * TouchEvent::State::Pressed, then the callback will only be called when the screen starts
     * being pressed.
     */
    static void on_touched(
        std::function<void(const TouchEvent&)> listener_cb,
        std::optional<TouchEvent::State> state_filter = std::nullopt
    );

    /**
     * @brief Subscribes the listener to be called when the screen begins to be pressed.
     * Wrapper for Screen::on_touched() with state_filter set to TouchEvent::State::Pressed.
     * @see Screen::on_touched()
     */
    static void on_pressed(std::function<void(const TouchEvent&)> listener_cb);

    /**
     * @brief Subscribes the listener to be called when the screen begins to be released.
     * Wrapper for Screen::on_touched() with state_filter set to TouchEvent::State::Released.
     * @see Screen::on_touched()
     */
    static void on_released(std::function<void(const TouchEvent&)> listener_cb);

    /**
     * @brief Subscribes the listener to be called when the screen begins to be held.
     * Wrapper for Screen::on_touched() with state_filter set to TouchEvent::State::Held.
     * @see Screen::on_touched()
     */
    static void on_held(std::function<void(const TouchEvent&)> listener_cb);

    /** @brief Determines where screen operations should be written. Immediate is the default. */
    enum class RenderMode {
        /**
         * Draw operations are immediately applied to the screen without the need to call
         * Screen::render().
         * This is the default mode.
         */
        Immediate,
        /**
         * Draw calls are written to an intermediate display buffer, rather than directly drawn to
         * the screen. This buffer can later be applied using Screen::render().
         *
         * This mode is necessary for preventing screen tearing when drawing at high speeds.
         */
        DoubleBuffered,
    };

    /**
     * @brief Changes the render mode of the screen.
     * @param new_mode The new render mode to set.
     * @see Screen::RenderMode
     */
    static void set_render_mode(RenderMode new_mode);

    /**
     * @brief Gets the current render mode of the screen.
     * @return The current render mode of the screen.
     * @see Screen::RenderMode
     */
    static RenderMode get_render_mode();

    /**
     * @brief Flushes the screen's double buffer it is enabled.
     * This does nothing in the immediate render mode, but is required in the immediate render mode.
     * @see Screen::RenderMode
     */
    static void render();

    static void scroll();
    static void scroll_region();

    // TODO: Should these be a single function? see:
    // https://docs.rs/vexide/latest/vexide/devices/display/struct.Display.html#method.fill
    static void draw_rect();
    static void draw_circle();
    static void draw_line();
    static void draw_pixel();

    static void fill_rect();
    static void fill_circle();

    // Should likely use a text object, like vexide does:
    // https://docs.rs/vexide/latest/vexide/devices/display/struct.Text.html
    static void draw_text();

    static void clear(auto color /* = TODO*/);

    static void draw_buffer();

  private:
    static RenderMode m_render_mode;
    static pros::Mutex m_mutex;
};

} // namespace zest