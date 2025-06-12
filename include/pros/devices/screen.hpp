#pragma once

#include "stdint.h"

#include <functional>

namespace zest {

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
            Release,
            /** The screen has been touched. */
            Press,
            /** The screen has been touched and is still being held. */
            Held
        };

        /** The y coordinate of the touch. */
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
     * @brief Subscribes the listener to be called when the screen begins to be pressed.
     *
     * Spawn a new task to check for events if it is not already running.
     * All listeners are called from this task.
     *
     * @warning If you have multiple listeners, avoid using delays, which can delay other callbacks
     * from being called, and result in lost information.
     * For this reason, it is recommended library developers use get_last_touch() instead of this
     * function.
     *
     * @param listener_cb The function to call when the screen is pressed.
     */
    static void on_pressed(std::function<void(const TouchEvent&)> listener_cb);

    /**
     * @brief Subscribes the listener to be called when the screen begins to be released.
     *
     * Spawn a new task to check for events if it is not already running.
     * All listeners are called from this task.
     *
     * @warning If you have multiple listeners, avoid using delays, which can delay other callbacks
     * from being called, and result in lost information.
     * For this reason, it is recommended library developers use get_last_touch() instead of this
     * function.
     *
     * @param listener_cb The function to call when the screen is pressed.
     */
    static void on_released(std::function<void(const TouchEvent&)> listener_cb);

    // TODO: Should there be a on_held() function?

    static void set_render_mode();
    static void get_render_mode();
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
};

} // namespace zest